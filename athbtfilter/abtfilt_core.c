//------------------------------------------------------------------------------
// <copyright file="abtfilt_core.c" company="Atheros">
//    Copyright (c) 2009 Atheros Corporation.  All rights reserved.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation;
//
// Software distributed under the License is distributed on an "AS
// IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
// implied. See the License for the specific language governing
// rights and limitations under the License.
// 
//
//------------------------------------------------------------------------------
//==============================================================================
// Author(s): ="Atheros"
//==============================================================================
/*
 * Bluetooth Filter Front End
 *
 */
static const char athId[] __attribute__ ((unused)) = "$Id: //depot/sw/releases/olca2.2/host/tools/athbtfilter/bluez/abtfilt_core.c#9 $";

#include "abtfilt_int.h"

/* Defines */
#define MAX_VAL_DATA_LENGTH     128

#ifdef ANDROID
#define BUFFER_LEN  (4 + sizeof(WMI_SET_BT_PARAMS_CMD))
#endif

const A_CHAR *g_IndicationStrings[ATH_BT_MAX_STATE_INDICATION] = 
{
   "NOP",
   "INQUIRY",
   "CONNECT",
   "SCO",
   "ACL",
   "A2DP",
   "ESCO",
};

extern A_FILE_HANDLE gConfigFile;

/* Function Prototypes */
static void BtStateActionProper(ATHBT_FILTER_INFO *pInfo, 
                                ATHBT_STATE_INDICATION Indication, 
                                ATHBT_STATE State);
static void DoBtStateAction(ATHBT_FILTER_INFO *pInfo, 
                            ATHBT_STATE_INDICATION Indication, 
                            ATHBT_STATE State);
static void AthFilterCmdEventsCallback(void *pContext, 
                                       ATHBT_HCI_CTRL_TYPE Type, 
                                       unsigned char *pBuffer, int Length);
static void AthFilterIndicateStateCallback(void *pContext, 
                                           ATHBT_STATE_INDICATION Indication, 
                                           ATHBT_STATE State, 
                                           unsigned char LMPVersion);
static void AthFilterAclDataOutCallback(void *pContext,
                                        unsigned char *pBuffer, int Length);
static void AthFilterAclDataInCallback(void *pContext, 
                                       unsigned char *pBuffer, int Length);
static void ProcessBTActionMessages(ATHBT_FILTER_INFO      *pInfo, 
                                    BTACTION_QUEUE_PROC    Process,
                                    ATHBT_STATE_INDICATION StateToFlush);
#ifdef ANDROID
static void SetAntennaConfig(ATHBT_FILTER_INFO *pInfo);
#endif

static void ExecuteBtAction(ATHBT_FILTER_INFO *pInfo, 
                            BT_ACTION_MSG *pBtActionMsg);
static ATHBT_STATE_INDICATION IndicateA2DP(ATHBT_FILTER_INFO *pInfo , 
                                           ATHBT_STATE_INDICATION Indication,
                                           ATHBT_STATE State,
                                           unsigned char *pACLBuffer);
static void *FilterThread(void *arg);
static void SyncBTState(ATHBT_FILTER_INFO *pInfo);
static void ProcessActionOverride(ATHBT_FILTER_INFO *pInfo, 
                                  A_CHAR            *pIndicationStr, 
                                  A_CHAR            *pModifyAction, 
                                  A_CHAR            *pAction);
static void GetActionStringOverrides(ATHBT_FILTER_INFO *pInfo);

/* APIs exported to other modules */
int
AthBtFilter_Attach(ATH_BT_FILTER_INSTANCE *pInstance)
{
    int                             i;
    int                             retVal = -1;
    ATHBT_FILTER_INFO              *pInfo = NULL;
    BT_ACTION_MSG                  *pBTActionMsg;
    A_UINT32                        maxBTActionMsgs = MAX_BT_ACTION_MESSAGES;
    A_STATUS                        status;

    do {
        pInfo = (ATHBT_FILTER_INFO *)A_MALLOC(sizeof(ATHBT_FILTER_INFO) + 
                                   maxBTActionMsgs * (sizeof(BT_ACTION_MSG)));
        if (NULL == pInfo) {
            A_ERR("[%s] Failed to allocate BT filter info\n", __FUNCTION__);
            break;    
        }       
        A_MEMZERO(pInfo, sizeof(ATHBT_FILTER_INFO));

        pInstance->pContext = pInfo;
        pInfo->pInstance = pInstance;
        pInfo->MaxBtActionMessages = (int)maxBTActionMsgs;
        pInfo->AdapterAvailable = FALSE;
        pInfo->Shutdown = FALSE;

        status = A_MUTEX_INIT(&pInfo->CritSection);
        if (A_FAILED(status)) {
            A_ERR("[%s] Failed to initialize critical section\n",
                  __FUNCTION__);
            break;
        }

        DL_LIST_INIT(&pInfo->BTActionMsgList);
        DL_LIST_INIT(&pInfo->FreeActionMsgList);

        if (!FCore_Init(&pInfo->FilterCore)) {
            break;    
        }

        GetActionStringOverrides(pInfo);

        status = FCore_RefreshActionList(&pInfo->FilterCore);
        if (A_FAILED(status)) {
            A_ERR("[%s] Failed refresh action list (status:%d)\n", 
                  __FUNCTION__, status);
            break;    
        }

        /* message buffers are at the end of our context blob */
        pBTActionMsg = (BT_ACTION_MSG *)((A_UCHAR *)pInfo + 
                                         sizeof(ATHBT_FILTER_INFO));  
         
        for (i = 0; i < pInfo->MaxBtActionMessages; i++, pBTActionMsg++) {
            /* create the event for blocking requests */
            status = A_COND_INIT(&pBTActionMsg->hWaitEvent);
            if (A_FAILED(status)) {
                A_ERR("[%s] Failed to allocate BT action event wait object\n",
                      __FUNCTION__);
                /* if we are running out of memory we'll fail farther down */
                break;    
            }

            status = A_MUTEX_INIT(&pBTActionMsg->hWaitEventLock);
            if (A_FAILED(status)) {
                A_ERR("[%s] Failed to initialize the mutex\n", __FUNCTION__);
                /* if we are running out of memory we'll fail farther down */
                break;    
            }

            /* free to list */
            FREE_BT_ACTION_MSG(pInfo, pBTActionMsg);
        }        

        /* create the wake event for our dispatcher thread */        
        status = A_COND_INIT(&pInfo->hWakeEvent);
        if (A_FAILED(status)) {
            A_ERR("[%s] Failed to allocate wakeup event\n", __FUNCTION__);
            break;    
        }

        status = A_MUTEX_INIT(&pInfo->hWakeEventLock);
        if (A_FAILED(status)) {
            A_ERR("[%s] Failed to initialize critical section\n", 
                  __FUNCTION__);
            break;
        }

        /* 
         * get the event types that the filter core can ignore. The BT 
         * notification side can handle them TODO 
         */
        pInfo->FilterCore.StateFilterIgnore = 0;

        pInstance->pFilterCmdEvents = AthFilterCmdEventsCallback;
        pInstance->pIndicateState = AthFilterIndicateStateCallback;
        pInstance->pFilterAclDataOut = AthFilterAclDataOutCallback;
        pInstance->pFilterAclDataIn = AthFilterAclDataInCallback;

        /* 
         * We are fully initialized and ready to filter. The filter core 
         * needs to stay in sync with the BT radio state until the WLAN 
         * driver comes up, when the WLAN driver comes on-line the filter 
         * will issue operating parameters for the current BT radio state 
         * (see HandleAdapterEvent) 
         */
        pInstance->FilterEnabled = TRUE;

        status = A_TASK_CREATE(&pInfo->hFilterThread, FilterThread, pInfo);
        if (A_FAILED(status)) {
            A_ERR("[%s] Failed to create filter thread\n", __FUNCTION__);
            break;   
        }

        pInfo->FilterThreadValid = TRUE;
        
        retVal = 0;
        A_INFO("BT Filter Core init complete\n");
    } while (FALSE);
    
    if (retVal < 0) {
        AthBtFilter_Detach(pInstance);
    }
    
    return retVal;
}

void 
AthBtFilter_Detach(ATH_BT_FILTER_INSTANCE *pInstance)
{
    A_STATUS           status;
    ATHBT_FILTER_INFO  *pInfo = (ATHBT_FILTER_INFO  *)pInstance->pContext;
    BT_ACTION_MSG      *pBTActionMsg;
    int                i;

    /* make sure filter is disabled */
    pInstance->FilterEnabled = FALSE;

    if (NULL == pInfo) {
        return;    
    }

    pInfo->Shutdown = TRUE;

        /* wake filter thread */
    A_MUTEX_LOCK(&pInfo->hWakeEventLock);
    A_COND_SIGNAL(&pInfo->hWakeEvent);
    A_MUTEX_UNLOCK(&pInfo->hWakeEventLock);
   
    if (pInfo->FilterThreadValid) {
        pInfo->FilterThreadValid = FALSE;
            /* wait for thread to exit */
        status = A_TASK_JOIN(&pInfo->hFilterThread);
        if (A_FAILED(status)) {
            A_ERR("[%s] Failed to JOIN filter thread \n", 
                  __FUNCTION__);
        }
    }
    
    A_COND_DEINIT(&pInfo->hWakeEvent);
    A_MUTEX_DEINIT(&pInfo->hWakeEventLock);

    pBTActionMsg = (BT_ACTION_MSG *)((A_UCHAR *)pInfo + 
                                     sizeof(ATHBT_FILTER_INFO));  
    /* close all action message wait objects */
    for (i = 0; i < pInfo->MaxBtActionMessages; i++, pBTActionMsg++) {
        A_COND_DEINIT(&pBTActionMsg->hWaitEvent);
        A_MUTEX_DEINIT(&pBTActionMsg->hWaitEventLock);
    }        

    pInstance->pContext = NULL;
    A_MUTEX_DEINIT(&pInfo->CritSection);
    FCore_Cleanup(&pInfo->FilterCore);
    A_FREE(pInfo);

    A_INFO("BT Filter Core de-init complete\n");
}

/* Internal functions */
static void 
AdjustBtControlAction(ATHBT_FILTER_INFO      *pInfo, 
                      BT_ACTION_MSG          *pActionMsg)
{
    WMI_SET_BT_PARAMS_CMD   *pParamsCmd;
    
    do {    
        /* adjust control action for BT_PARAMS_A2DP control action */
        if (pActionMsg->IndicationForControlAction == ATH_BT_A2DP) {
	  A_UINT32 a2dpOptFlags=0;
            if (pActionMsg->StateForControlAction != STATE_ON) {
                /* nothing to adjust in OFF case */
                break;
            }

            if (pActionMsg->ControlAction.Type != BT_CONTROL_ACTION_PARAMS) {
                /* only modify action that issues a PARAMS control action */
                break;
            }

            pParamsCmd = 
               (WMI_SET_BT_PARAMS_CMD *)(pActionMsg->ControlAction.Buffer);

            if (pParamsCmd->paramType != BT_PARAM_A2DP) {
                /* only modify A2DP params */
                break;
            }
             /*Role =0 is Master, Role =1, is slave */
#ifndef DISABLE_MASTER_MODE
            if(pInfo->A2DPConnection_Role == 0) {
                pParamsCmd->info.a2dpParams.isCoLocatedBtRoleMaster = 1;
            }else {
#endif
                pParamsCmd->info.a2dpParams.isCoLocatedBtRoleMaster = 0;
                /* workaround for local BT radio that disables EDR
                 * rates when operating as a slave. We downgrade
                 * the remote lmp version to protect A2DP as if the radio was 1.2 */
                 if(btWarDowngradeLmp()){
                    pInfo->A2DPConnection_LMPVersion = 2;
                 }
#ifndef DISABLE_MASTER_MODE
            }
#endif

            switch (pInfo->A2DPConnection_LMPVersion) {
                case 0: // 1.0
                case 1: // 1.1
                case 2: // 1.2
                    pParamsCmd->info.a2dpParams.a2dpWlanUsageLimit = 30;
                    pParamsCmd->info.a2dpParams.a2dpBurstCntMin = 3;
                    pParamsCmd->info.a2dpParams.a2dpDataRespTimeout =20;
                    break;
                case 3: // 2.0
                case 4: // 2.1
                default:
                    if( pParamsCmd->info.a2dpParams.isCoLocatedBtRoleMaster) {
                        /* allow close range optimization for newer BT radios */
                        a2dpOptFlags |= BT_A2DP_ALLOW_CLOSE_RANGE_OPT;
                        a2dpOptFlags |= BT_A2DP_FORCE_AWAKE_OPT;
                    }   
                    pParamsCmd->info.a2dpParams.a2dpWlanUsageLimit = 30;
                    pParamsCmd->info.a2dpParams.a2dpBurstCntMin = 3;
                    pParamsCmd->info.a2dpParams.a2dpDataRespTimeout =20;
                 /* pParamsCmd->info.a2dpParams.p2lrpOptModeBound=7;
                    pParamsCmd->info.a2dpParams.p2lrpNonOptModeBound=9;
                    BT_A2DP_SET_DEFAULT_OVERRIDE(a2dpOptFlags);*/
                    /*uncomment the above if need to modify the 
                    Atheros recommended default values. It is expected
                    that if you are modifying this, you know what you
                    know the details*/ 
                    break;
            }
            pParamsCmd->info.a2dpParams.a2dpOptFlags = a2dpOptFlags;
            
            A_DEBUG(("ATHBT: BT PARAMS A2DP Adjustments :\r\n"));
            A_DEBUG(("    a2dpWlanUsageLimit  : %d\r\n"),
                    pParamsCmd->info.a2dpParams.a2dpWlanUsageLimit);
            A_DEBUG(("    a2dpBurstCntMin     : %d\r\n"),
                    pParamsCmd->info.a2dpParams.a2dpBurstCntMin);
            A_DEBUG(("    a2dpDataRespTimeout : %d\r\n"),
                    pParamsCmd->info.a2dpParams.a2dpDataRespTimeout);
	        A_DEBUG(("      a2dpOptFlags : %d\r\n"),
                    pParamsCmd->info.a2dpParams.a2dpOptFlags );
	        A_DEBUG(("    isCoLocatedBtRoleMaster : %d\r\n"),
                    pParamsCmd->info.a2dpParams.isCoLocatedBtRoleMaster);
            A_DEBUG(("    a2dpOptFlags    : 0x%X\r\n"),
                    pParamsCmd->info.a2dpParams.a2dpOptFlags);
        }

        /* adjust control action for BT_PARAMS_SCO control action  */
        if ((pActionMsg->IndicationForControlAction == ATH_BT_SCO) || 
            (pActionMsg->IndicationForControlAction == ATH_BT_ESCO))
        {
	    A_UINT32 scoOptFlags = 0;
            if (pActionMsg->StateForControlAction != STATE_ON) {
                /* nothing to adjust in OFF case */
                break;     
            }           

            if (pActionMsg->ControlAction.Type != BT_CONTROL_ACTION_PARAMS) {
                /* only modify action that issues a PARAMS control action */
                break;                
            }        
            
            pParamsCmd = 
                (WMI_SET_BT_PARAMS_CMD *)(pActionMsg->ControlAction.Buffer);
            
            if (pParamsCmd->paramType != BT_PARAM_SCO) {
                /* only modify SCO params */
                break;    
            }  

            if ((pInfo->SCOConnectInfo.LinkType == BT_LINK_TYPE_ESCO) && 
                (pInfo->SCOConnectInfo.Valid)) {
                    
                A_UCHAR scoSlots;
                
                pInfo->SCOConnectInfo.Valid = FALSE;
                
                /* decode packet length to get packet type */
                if (pInfo->SCOConnectInfo.TxPacketLength <= 30) {
                    /* EV3 */
                    scoSlots = 1;
                } else if (pInfo->SCOConnectInfo.TxPacketLength <= 60) {
                    /* 2-EV3 */
                    scoSlots = 1;
                    scoOptFlags |= BT_SCO_ALLOW_CLOSE_RANGE_OPT;
                    A_DEBUG(("     eSCO - 2-EV3\r\n"));
                } else if (pInfo->SCOConnectInfo.TxPacketLength <= 90) {
                    /*3-EV3 */
                    scoSlots = 1;            
                    scoOptFlags |= BT_SCO_ALLOW_CLOSE_RANGE_OPT;
                    A_DEBUG(("     eSCO - 3-EV3\r\n"));
                } else if (pInfo->SCOConnectInfo.TxPacketLength <= 120) {
                    /* EV4: */  
                    scoSlots = 3;             
                    A_DEBUG(("     eSCO - EV4\r\n"));
                } else if (pInfo->SCOConnectInfo.TxPacketLength <= 180) {
                    /* EV5: */  
                    scoSlots = 3;   
                    A_DEBUG(("     eSCO - EV5\r\n"));
                } else if (pInfo->SCOConnectInfo.TxPacketLength <= 360) {
                    /* 2-EV5 */
                    scoSlots = 3;
                    scoOptFlags |= BT_SCO_ALLOW_CLOSE_RANGE_OPT;
                    A_DEBUG(("     eSCO - 2-EV5\r\n"));
                } else {
                    /* 3-EV5 */
                    scoSlots = 3;
                    scoOptFlags |= BT_SCO_ALLOW_CLOSE_RANGE_OPT;
                    A_DEBUG(("     eSCO - 3-EV5\r\n"));
                }
                
                scoOptFlags = 0;

                /* account for RX/TX */
                scoSlots *= 2;
            /*  pParamsCmd->info.scoParams.p2lrpOptModeBound=7;
                pParamsCmd->info.scoParams.p2lrpNonOptModeBound=9;
                BT_SCO_SET_DEFAULT_OVERRIDE(scoOptFlags);*/
                /*uncomment the above if need to modify the 
                  Atheros recommended default values. It is expected
                  that if you are modifying this, you know what you
                  know the details*/              
                pParamsCmd->info.scoParams.noSCOSlots =  scoSlots;
            	pParamsCmd->info.scoParams.scoOptFlags = scoOptFlags;
                
                if (pInfo->SCOConnectInfo.TransmissionInterval >= scoSlots) {
                    pParamsCmd->info.scoParams.noIdleSlots = 
                        pInfo->SCOConnectInfo.TransmissionInterval - scoSlots;
                } else {
                    A_DEBUG(("Invalid scoSlot,  got:%d, transInt: %d\n"), 
                            scoSlots, 
                            pInfo->SCOConnectInfo.TransmissionInterval);
                }
            } else {
                /* legacy SCO */
                pParamsCmd->info.scoParams.noSCOSlots = 2;
                pParamsCmd->info.scoParams.noIdleSlots = 4;
                /*dont enable optimization for basic rate sco at all*/
            	pParamsCmd->info.scoParams.scoOptFlags = 0;
            }
                /* provide SCO packet type hint */             
            A_DEBUG(("ATHBT: BT PARAMS SCO adjustment (%s) \n"), 
                    pInfo->SCOConnectInfo.LinkType == BT_LINK_TYPE_ESCO ? "eSCO":"SCO");
            A_DEBUG(("    numScoCyclesForceTrigger : %d \n"),
                    pParamsCmd->info.scoParams.numScoCyclesForceTrigger);
            A_DEBUG(("    dataResponseTimeout      : %d \n"),
                    pParamsCmd->info.scoParams.dataResponseTimeout);                
            A_DEBUG(("    stompScoRules            : %d \n"),
                    pParamsCmd->info.scoParams.stompScoRules);      
	    A_DEBUG(("     scoOptFlags             : %d \n"),
                    pParamsCmd->info.scoParams.scoOptFlags);            
            A_DEBUG(("    stompDutyCyleVal         : %d \n"),
                    pParamsCmd->info.scoParams.stompDutyCyleVal);
            A_DEBUG(("    stompDutyCyleMaxVal         : %d \n"),
                    pParamsCmd->info.scoParams.stompDutyCyleMaxVal);
            A_DEBUG(("    psPollLatencyFraction    : %d \n"),
                    pParamsCmd->info.scoParams.psPollLatencyFraction);
            A_DEBUG(("    noSCOSlots               : %d \n"),
                    pParamsCmd->info.scoParams.noSCOSlots);
            A_DEBUG(("    noIdleSlots              : %d \n"),
                    pParamsCmd->info.scoParams.noIdleSlots);                
        }  
    } while (FALSE); 
}

static void 
BtStateActionProper(ATHBT_FILTER_INFO *pInfo, 
                    ATHBT_STATE_INDICATION Indication, ATHBT_STATE State)
{
    A_COND_OBJECT           *hWait = NULL;
    BT_ACTION_MSG           *pActionMsg;
    DL_LIST                 *pListEntry;
    BT_CONTROL_ACTION_ITEM  *pBtControlAction;
    int                     queued = 0;

    A_LOCK_FILTER(pInfo);

    pBtControlAction = FCore_GetControlAction(&pInfo->FilterCore, 
                                              Indication,
                                              State,
                                              NULL);

    while (pBtControlAction != NULL) { 
        /* allocate an action message */
        pListEntry = DL_ListRemoveItemFromHead(&pInfo->FreeActionMsgList);
    
        if (NULL == pListEntry) {
            A_DEBUG("action messages exhausted\n");
            break;
        }

        pActionMsg = A_CONTAINING_STRUCT(pListEntry, BT_ACTION_MSG, 
                                         ListEntry);

        /* save state for later flushing */
        pActionMsg->StateForControlAction = State;
        pActionMsg->IndicationForControlAction = Indication;

        /* we need to buffer the control actions */       
        A_MEMCPY(&pActionMsg->ControlAction, 
                 &pBtControlAction->ControlAction, 
                 sizeof(pActionMsg->ControlAction));
 
        /* When is it ever set to blocking TODO */
        if (pActionMsg->Blocking) {
            /* this is the action to wait on */
            hWait = &pActionMsg->hWaitEvent;
            A_COND_RESET(hWait);
        }
        
        /* allow for adjustments to the control action beyond the defaults */
        AdjustBtControlAction(pInfo, pActionMsg);
                
        /* queue action */
        QUEUE_BT_ACTION_MSG(pInfo, pActionMsg);
        queued++;
        
        /* get the next action using the current one as a starter */
        pBtControlAction = FCore_GetControlAction(&pInfo->FilterCore, 
                                                  Indication,
                                                  State,
                                                  pBtControlAction);
    } 
    
    A_UNLOCK_FILTER(pInfo);

    if (queued > 0) {
        /* wake thread to process all the queued up actions */
        A_MUTEX_LOCK(&pInfo->hWakeEventLock);
        A_COND_SIGNAL(&pInfo->hWakeEvent);
        A_MUTEX_UNLOCK(&pInfo->hWakeEventLock);
    }

    /* check if we need to block until the dispatch thread issued the 
     * last action if the adapter becomes unavailable we cannot block 
     * the thread (queue will stall), so only block if the adapter is 
     * available and use a reasonable timeout 
     */
    if (hWait) {
        A_COND_WAIT(hWait, &pInfo->CritSection, ACTION_WAIT_TIMEOUT);
    }
}

static void DoBtStateAction(ATHBT_FILTER_INFO *pInfo, ATHBT_STATE_INDICATION Indication, ATHBT_STATE State)
{
    A_UINT32 bitmap = FCore_GetCurrentBTStateBitMap(&pInfo->FilterCore);

    if ((Indication == ATH_BT_INQUIRY) && (State == STATE_ON)) {
        int i;
        for (i=0; i<ATH_BT_MAX_STATE_INDICATION; ++i) {
            if ((i!=Indication) && (bitmap & (1<<i))) {
                BtStateActionProper(pInfo,i,STATE_OFF);
            }
        }
    }

    if ((Indication == ATH_BT_A2DP) && ((bitmap & (1<<ATH_BT_SCO)) || (bitmap & (1<<ATH_BT_ESCO)))) {
        /* SCO protection is running, don't take any actions */
        return;
    }
    
    if (((Indication == ATH_BT_SCO) || (Indication == ATH_BT_ESCO)) && (State == STATE_ON)) {
        if (bitmap & (1<<ATH_BT_A2DP)) {
            BtStateActionProper(pInfo,ATH_BT_A2DP,STATE_OFF);
        }
    }
    
    BtStateActionProper(pInfo, Indication, State);
    
    if ((Indication == ATH_BT_INQUIRY) && (State == STATE_OFF)) {
        SyncBTState(pInfo);
    }

    if (((Indication == ATH_BT_SCO) || (Indication == ATH_BT_ESCO)) && (State == STATE_OFF)) {
        if (bitmap & (1<<ATH_BT_A2DP)) {
            BtStateActionProper(pInfo,ATH_BT_A2DP,STATE_ON);
        }
    }
}


static void 
AthFilterCmdEventsCallback(void *pContext, ATHBT_HCI_CTRL_TYPE Type, 
                           unsigned char *pBuffer, int Length)
{
    ATHBT_STATE_INDICATION  indication;
    ATHBT_FILTER_INFO       *pInfo = (ATHBT_FILTER_INFO *)pContext;
    ATHBT_STATE             state;
    
    if (Type == ATHBT_HCI_EVENT) {
        if (HCI_GET_EVENT_CODE(pBuffer) == HCI_EVT_NUM_COMPLETED_PKTS) {
            /* don't delays these packets, we don't act on them anyways */
            return;    
        }            
    }
    
    if (pInfo->Shutdown) {
        return;    
    }
    
   /* 
    * the filter state machine needs to be protected in case the HCI layer
    * can process commands and events in an unserialize manner 
    */
    A_LOCK_FILTER(pInfo);
    
    if (Type == ATHBT_HCI_COMMAND) {
        A_DUMP_BUFFER(pBuffer, Length, "BT HCI Command");
        indication = FCore_FilterBTCommand(&pInfo->FilterCore, pBuffer, 
                                           Length, &state);
    } else {
        A_DUMP_BUFFER(pBuffer, Length, "BT HCI Event");
        indication = FCore_FilterBTEvent(&pInfo->FilterCore, pBuffer, 
                                         Length, &state);
        /* check SCO and ESCO connection events */
        if ((indication == ATH_BT_SCO) || (indication == ATH_BT_ESCO)) {
            if (HCI_GET_EVENT_CODE(pBuffer) == HCI_EVT_SCO_CONNECT_COMPLETE) { 
                A_DEBUG(("SCO_CONNECT_COMPLETE (%s)\n"), 
                        (state == STATE_ON) ? "ON" : "OFF");
                if (state == STATE_ON) {
                    /* save these off for the BT Action adjustment */
                    pInfo->SCOConnectInfo.LinkType = 
                        GET_BT_CONN_LINK_TYPE(pBuffer);
                    pInfo->SCOConnectInfo.TransmissionInterval = 
                        GET_TRANS_INTERVAL(pBuffer);
                    pInfo->SCOConnectInfo.RetransmissionInterval = 
                        GET_RETRANS_INTERVAL(pBuffer);
                    pInfo->SCOConnectInfo.RxPacketLength = 
                        GET_RX_PKT_LEN(pBuffer);
                    pInfo->SCOConnectInfo.TxPacketLength = 
                        GET_TX_PKT_LEN(pBuffer);
                    A_DEBUG(("ATHBT: SCO conn info (%d, %d, %d, %d, %d))\n"), 
                            pInfo->SCOConnectInfo.LinkType,
                            pInfo->SCOConnectInfo.TransmissionInterval,
                            pInfo->SCOConnectInfo.RetransmissionInterval,
                            pInfo->SCOConnectInfo.RxPacketLength,
                            pInfo->SCOConnectInfo.TxPacketLength);
                    /* now valid */
                    pInfo->SCOConnectInfo.Valid = TRUE;
                } else {
                    /* disconnected, invalidate */
                    pInfo->SCOConnectInfo.Valid = FALSE; 
                }             
            }
        }
    }
    
    A_UNLOCK_FILTER(pInfo);
    
    if (indication == ATH_BT_NOOP) {
        return;    
    }

    A_DEBUG(("New Indication :%d State:%s (map:0x%X)\n"), 
            indication, (state == STATE_ON) ? "ON" : "OFF", 
            FCore_GetCurrentBTStateBitMap(&pInfo->FilterCore)); 

    if (pInfo->AdapterAvailable) {
        DoBtStateAction(pInfo, indication, state);
    }
}

static ATHBT_STATE_INDICATION 
IndicateA2DP(ATHBT_FILTER_INFO        *pInfo, 
             ATHBT_STATE_INDICATION   Indication,
             ATHBT_STATE              State,
             unsigned char            *pACLBuffer)
{
    A_ERR("[%s] Not yet implemented\n", __FUNCTION__);

    return ATH_BT_NOOP; /* TODO */
}

static void 
AthFilterAclDataOutCallback(void *pContext, unsigned char *pBuffer, int Length)
{
    ATHBT_STATE_INDICATION  indication;
    ATHBT_FILTER_INFO       *pInfo = (ATHBT_FILTER_INFO *)pContext;
    ATHBT_STATE             state;
    
    if (pInfo->Shutdown) {
        return;    
    }
    
    A_LOCK_FILTER(pInfo);
    
    indication = FCore_FilterACLDataOut(&pInfo->FilterCore,
                                        pBuffer, 
                                        Length, 
                                        &state);
                                        
    if (indication == ATH_BT_A2DP) {
        indication = IndicateA2DP(pInfo, 
                                  ATH_BT_A2DP,
                                  state,
                                  pBuffer);   
    }
    
    A_UNLOCK_FILTER(pInfo);
    
    if (indication == ATH_BT_NOOP) {
        return;    
    }

    A_DEBUG(("New Indication :%d State:%s (map:0x%X)\n"), 
            indication, (state == STATE_ON) ? "ON" : "OFF", 
            FCore_GetCurrentBTStateBitMap(&pInfo->FilterCore));

    if (pInfo->AdapterAvailable) {
        DoBtStateAction(pInfo, indication, state);
    }
    
}

static void 
AthFilterAclDataInCallback(void *pContext, unsigned char *pBuffer, int Length)
{   
    ATHBT_STATE_INDICATION  indication;
    ATHBT_FILTER_INFO       *pInfo = (ATHBT_FILTER_INFO *)pContext;
    ATHBT_STATE             state;
    
    if (pInfo->Shutdown) {
        return;    
    }
    
    A_LOCK_FILTER(pInfo);
    
    indication = FCore_FilterACLDataIn(&pInfo->FilterCore,
                                        pBuffer, 
                                        Length, 
                                        &state);
    
    if (indication == ATH_BT_A2DP) {
        indication = IndicateA2DP(pInfo, 
                                  ATH_BT_A2DP,
                                  state,
                                  pBuffer);   
    }
                                                                 
    A_UNLOCK_FILTER(pInfo);
    
    if (indication == ATH_BT_NOOP) {
        return;    
    }

    A_DEBUG(("New Indication :%d State:%s (map:0x%X)\n"), 
            indication, (state == STATE_ON) ? "ON" : "OFF", 
            FCore_GetCurrentBTStateBitMap(&pInfo->FilterCore));

    if (pInfo->AdapterAvailable) {
        DoBtStateAction(pInfo, indication, state);
    }
}

static void 
AthFilterIndicateStateCallback(void *pContext, 
                               ATHBT_STATE_INDICATION Indication, 
                               ATHBT_STATE State, unsigned char LMPVersion)
{
    ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pContext;
    ATHBT_STATE             newState;
    
    if (pInfo->Shutdown) {
        return;    
    }
    
    A_LOCK_FILTER(pInfo);
    Indication = FCore_FilterIndicatePreciseState(&pInfo->FilterCore, 
                                            Indication, State, &newState);
    A_UNLOCK_FILTER(pInfo);
    
    if (Indication == ATH_BT_NOOP) {
        return;    
    }
    
    A_DEBUG(("New Indication :%d State:%s (map:0x%X) \r\n"), 
            Indication, (newState == STATE_ON) ? "ON" : "OFF", 
            FCore_GetCurrentBTStateBitMap(&pInfo->FilterCore));

    if ((newState == STATE_ON) && (LMPVersion < 5)) {
        pInfo->LMPVersion = LMPVersion;
    }

    if (pInfo->AdapterAvailable) {
        DoBtStateAction(pInfo, Indication, newState);
    }
}

static void *
FilterThread(void *pContext)
{
    ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pContext;

    A_INFO("Starting the BT Filter task\n");
    while (1) {
        /* Wait to be woken up the BT thread */
        A_MUTEX_LOCK(&pInfo->hWakeEventLock);
        A_COND_WAIT(&pInfo->hWakeEvent, &pInfo->hWakeEventLock, WAITFOREVER);
        A_MUTEX_UNLOCK(&pInfo->hWakeEventLock);

        if (pInfo->AdapterAvailable) { 
            ProcessBTActionMessages(pInfo, BTACTION_QUEUE_NORMAL, ATH_BT_NOOP);
        }

        if (pInfo->Shutdown) {
            /* 
             * before we exit we need to counter-act the coexistence 
             * settings. Currently we just indicate that each state is now 
             * OFF (if they are ON). This state synchronization is typically 
             * required on HOT-removable BT adapters or where the low level 
             * adapter can be surprise removed before the BT stack can clean 
             * up HCI connections and states 
             */
            if (pInfo->AdapterAvailable) {
                int         indication, newIndication;
                ATHBT_STATE newState;
                
                /* 
                 * the BT adapter is going away, indicate that all indications 
                 * are now in the OFF state, this may queue up control action 
                 * messages, which is okay 
                 */
                for (indication = 0; indication < ATH_BT_MAX_STATE_INDICATION;
                     indication++) 
                {
                    A_LOCK_FILTER(pInfo);
                    newIndication = 
                        FCore_FilterIndicatePreciseState(&pInfo->FilterCore, 
                                          indication, STATE_OFF, &newState);
                    A_UNLOCK_FILTER(pInfo);

                    if (newIndication == ATH_BT_NOOP) {
                        continue;
                    }
                    
                    DoBtStateAction(pInfo, indication, newState);
                }
                
                /* issue control actions */
                ProcessBTActionMessages(pInfo, BTACTION_QUEUE_SYNC_STATE, 
                                        ATH_BT_NOOP);
            }

            break;    
        } 
    }

    A_INFO("Terminating the BT Filter task\n");
    return NULL;
}

static void 
ProcessBTActionMessages(ATHBT_FILTER_INFO      *pInfo, 
                        BTACTION_QUEUE_PROC    Process,
                        ATHBT_STATE_INDICATION StateToFlush)
{
    BT_ACTION_MSG *pActionMsg = NULL;
    DL_LIST       *pListEntry = NULL;

    A_LOCK_FILTER(pInfo);

    while (1) {
        /* determine how we want to pull the message out */
        if (BTACTION_QUEUE_FLUSH_STATE == Process) {
            if (NULL == pListEntry) {
                /* first time through */
                if (!DL_LIST_IS_EMPTY(&pInfo->BTActionMsgList)) {
                    /* get the entry at the head of the list , don't remove */ 
                    pListEntry = 
                        DL_LIST_GET_ITEM_AT_HEAD(&pInfo->BTActionMsgList); 
                }
            } else {
                /* move onto the next one */
                pListEntry = pListEntry->pNext;
                if (pListEntry == &pInfo->BTActionMsgList) {
                    /* reached the end */
                    pListEntry = NULL; 
                } 
            } 
        } else {
            /* 
             * for all others we are removing items from the head of the
             * queue 
             */
            pListEntry = DL_ListRemoveItemFromHead(&pInfo->BTActionMsgList);            
        }

        if (NULL == pListEntry) {
            /* queue was empty */
            break;
        }

        pActionMsg = A_CONTAINING_STRUCT(pListEntry, BT_ACTION_MSG, ListEntry);

        /* now determine what to do with the message that was found */
        switch (Process) {
            case BTACTION_QUEUE_FLUSH_STATE:
                /* 
                 * caller wants to just flush action messages matching a 
                 * state 
                 */
                if (pActionMsg->StateForControlAction == StateToFlush) {
                    A_DEBUG(("Removed action for state=%d from queue\n"),
                            StateToFlush);

                    /* remove from list, it will get freed below */
                    DL_ListRemove(&pActionMsg->ListEntry);

                    /* 
                     * this would re-start the scan to the head of the list 
                     * each time we found one.  This type of flush doesn't 
                     * happen very often so restarting from the head of the 
                     * list and rescanning isn't time consuming 
                     */
                    pListEntry = NULL;
                } else {
                    /* not the one we are interested in */
                    pActionMsg = NULL;
                }
                break;

            case BTACTION_QUEUE_NORMAL:
            case BTACTION_QUEUE_SYNC_STATE:
                /* issue/execute actions */
                A_UNLOCK_FILTER(pInfo);
                A_DEBUG(("Processing action for indication=%d (%s) (%s)\n"),
                        pActionMsg->IndicationForControlAction,
                        (pActionMsg->StateForControlAction == STATE_ON) ?
                        "ON" : "OFF", (BTACTION_QUEUE_SYNC_STATE == Process) ?
                        "Sync State" : "Normal");

                if (BTACTION_QUEUE_SYNC_STATE == Process) {
                    /* let's not issue these too fast ... */
                    sleep(10); 
                }

                ExecuteBtAction(pInfo, pActionMsg); 

                if (pActionMsg->Blocking) {
                    pActionMsg->Blocking = FALSE;

                    /* set the event to unblock the caller */
                    A_MUTEX_LOCK(&pActionMsg->hWaitEventLock);
                    A_COND_SIGNAL(&pActionMsg->hWaitEvent);
                    A_MUTEX_UNLOCK(&pActionMsg->hWaitEventLock);
                }
                A_LOCK_FILTER(pInfo); 
                break;

            case BTACTION_QUEUE_FLUSH_ALL:
                A_DEBUG(("Flushed action for state=%d from queue\n"),
                        pActionMsg->StateForControlAction);
                /* 
                 * nothing to do here, the action message will get 
                 * recycled below 
                 */
                break;

            default:
                break;
        }

        if (pActionMsg) {
            /* recycle message */        
            FREE_BT_ACTION_MSG(pInfo, pActionMsg);
        }
    }

    A_UNLOCK_FILTER(pInfo);
}
    
static void 
SyncBTState(ATHBT_FILTER_INFO *pInfo)
{
    int      stateIndication;
    A_UINT32 stateBitMap;

    A_LOCK_FILTER(pInfo);
    stateBitMap = FCore_GetCurrentBTStateBitMap(&pInfo->FilterCore);
    A_UNLOCK_FILTER(pInfo);
    
    /* 
     * the state bit map is a simple STATE ON/OFF bit map, if we detect 
     * that one of the states is ON we process the BT action to synchronize 
     * the WLAN side with the BT radio state 
     */
    for (stateIndication = 0; stateIndication < ATH_BT_MAX_STATE_INDICATION; 
         stateIndication++)
    {
        if (stateBitMap & (1 << stateIndication)) {
            /* this state is ON */
            DoBtStateAction(pInfo, stateIndication, STATE_ON);
        }
    }   
    
}

void 
HandleAdapterEvent(ATHBT_FILTER_INFO *pInfo, ATH_ADAPTER_EVENT Event)
{
    switch (Event) {
        case ATH_ADAPTER_ARRIVED:       
            A_INFO("BT Filter Core : WLAN Arrived \n");
            pInfo->AdapterAvailable = TRUE;

            /* sync BT state */
            SyncBTState(pInfo);
#ifdef ANDROID
	    A_LOCK_FILTER(pInfo);
    	    if (pInfo->AdapterAvailable){
    		SetAntennaConfig(pInfo);
		A_INFO("set antenna configuration\n");
		}
    	    A_UNLOCK_FILTER(pInfo);
#endif
            /* 
             * the call to sync BT state may queue a bunch of actions to 
             * the action queue, we want to issues these differently 
             */
            ProcessBTActionMessages(pInfo, BTACTION_QUEUE_SYNC_STATE, 
                                    ATH_BT_NOOP); 
            break; 
        
        case ATH_ADAPTER_REMOVED:
            A_INFO("BT Filter Core : WLAN removed \n");
            pInfo->AdapterAvailable = FALSE;

            /* flush messages */
            ProcessBTActionMessages(pInfo, BTACTION_QUEUE_FLUSH_ALL, 
                                    ATH_BT_NOOP);  
            break;
            
        default:
            break;
    }
}

#ifdef ANDROID
static void
SetAntennaConfig(ATHBT_FILTER_INFO *pInfo)
{
    A_UCHAR buf[BUFFER_LEN];
    WMI_SET_BT_PARAMS_CMD cmd;
    A_MEMZERO(buf, sizeof(buf));
    A_INFO("========>SetAntennaConfig\n");
    cmd.paramType = BT_PARAM_ANTENNA_CONFIG;
    cmd.info.antType = 3;
    
    
    (buf)[0] = AR6000_XIOCTL_WMI_SET_BT_PARAMS;
    A_MEMCPY((buf + 4), (void*)&cmd, 
              sizeof(cmd));
    Abf_WlanDispatchIO(pInfo, AR6000_IOCTL_EXTENDED, (void *)buf, 
                                sizeof(cmd) + 4);
    A_INFO("<========SetAntennaConfig\n");
    return;
}
#endif

#ifdef ANDROID
/* execute the BT action
 * this function is called by the single dispatcher thread
 */

static void 
ExecuteBtAction(ATHBT_FILTER_INFO *pInfo, BT_ACTION_MSG *pBtActionMsg)
{
    A_STATUS status;
    A_UCHAR buf[BUFFER_LEN]; //larger of the two

    A_MEMZERO(buf, sizeof(buf));

    if (pBtActionMsg->ControlAction.Type == BT_CONTROL_ACTION_STATUS) {

	(buf)[0] = AR6000_XIOCTL_WMI_SET_BT_STATUS;
    } else {

	(buf)[0] = AR6000_XIOCTL_WMI_SET_BT_PARAMS;
    }

	
     A_MEMCPY((buf + 4), /*(void *)*/pBtActionMsg->ControlAction.Buffer, 
              pBtActionMsg->ControlAction.Length);

 
    status = Abf_WlanDispatchIO(pInfo, AR6000_IOCTL_EXTENDED, (void *)buf, 
                                pBtActionMsg->ControlAction.Length + 4);

    if (A_FAILED(status)) {
        /* these can actually fail if the device powers down */
	A_DEBUG("DISPATCH IO FAIL\n");

    }

	return;
}
#else

/* execute the BT action
 * this function is called by the single dispatcher thread
 */
static void 
ExecuteBtAction(ATHBT_FILTER_INFO *pInfo, BT_ACTION_MSG *pBtActionMsg)
{
    A_STATUS status;
    A_UCHAR  buf[sizeof(A_UINT32) + sizeof(WMI_SET_BT_PARAMS_CMD)]; 
    A_UINT32 controlCode;
    
    A_MEMZERO(buf, sizeof(buf));
    
    if (pBtActionMsg->ControlAction.Type == BT_CONTROL_ACTION_STATUS) {
        /* this action issues a STATUS OID command */
        controlCode = AR6000_XIOCTL_WMI_SET_BT_STATUS;
    } else {
        /* this action issues a PARAMS OID command */
        controlCode = AR6000_XIOCTL_WMI_SET_BT_PARAMS;
    }

    A_MEMCPY(buf, &controlCode, sizeof(A_UINT32));
    
    if (pBtActionMsg->ControlAction.Length > sizeof(WMI_SET_BT_PARAMS_CMD)) {
        A_ERR("Bad control action length : %d \n", pBtActionMsg->ControlAction.Length);
        return;    
    }
    
    A_MEMCPY((buf + sizeof(A_UINT32)), (void *)pBtActionMsg->ControlAction.Buffer, 
              pBtActionMsg->ControlAction.Length);
    status = Abf_WlanDispatchIO(pInfo, AR6000_IOCTL_EXTENDED, (void *)buf, 
                                pBtActionMsg->ControlAction.Length + sizeof(A_UINT32));

    if (A_FAILED(status)) {
        /* these can actually fail if the device powers down */
        A_ERR("[%s] BT Action issue failed, Ioctl: 0x%x, Len: %d\n", 
              __FUNCTION__, ((int *)buf)[0], 
              pBtActionMsg->ControlAction.Length);
    } else {
        A_DEBUG("BT Action issued to WLAN Adapter, Ioctl: 0x%x, Len: %d\n", 
                ((int *)buf)[0], pBtActionMsg->ControlAction.Length);
        A_DUMP_BUFFER(pBtActionMsg->ControlAction.Buffer,
                      pBtActionMsg->ControlAction.Length,
                      "BT Control Action");
    }
}
#endif

static void 
ProcessActionOverride(ATHBT_FILTER_INFO *pInfo, 
                      A_CHAR            *pIndicationStr, 
                      A_CHAR            *pModifyAction, 
                      A_CHAR            *pAction)
{
    int                          i;
    ATHBT_STATE_INDICATION       indication;
    ATHBT_STATE                  state = STATE_MAX;
    ATHBT_MODIFY_CONTROL_ACTION  modifyAction = 
                                 ATHBT_MODIFY_CONTROL_ACTION_NOOP;
    char                         charBuffer[MAX_VAL_DATA_LENGTH];

    /* 
     * parse the indication string to figure which indication and state 
     * to change i.e.  <indication>-ON or <indication>-OFF 
     */
    for (indication = 0; indication < ATH_BT_MAX_STATE_INDICATION; 
         indication++)
    {
        if (strstr(pIndicationStr, g_IndicationStrings[indication]) != NULL) {
            /* found one */
            if (strstr(pIndicationStr, "-ON") != NULL) {
                state = STATE_ON;    
            }    

            if (strstr(pIndicationStr, "-OFF") != NULL) {
                state = STATE_OFF;
            }  

            if (strstr(pModifyAction, "REPLACE") != NULL) {
                modifyAction = ATHBT_MODIFY_CONTROL_ACTION_REPLACE;
            }     

            if (strstr(pModifyAction, "APPEND") != NULL) {
                modifyAction = ATHBT_MODIFY_CONTROL_ACTION_APPEND;
            } 

            break;    
        }
    }    

    if ((indication == ATH_BT_MAX_STATE_INDICATION) || 
        (state == STATE_MAX)               || 
        (modifyAction == ATHBT_MODIFY_CONTROL_ACTION_NOOP)) 
    {
        return;    
    }

    A_DEBUG("Found Action override : %s (%s) (%s)\n", 
            pIndicationStr, pModifyAction, pAction);

    A_MEMZERO(charBuffer, sizeof(charBuffer));

    for (i = 0; (i < (int)strlen(pAction)) && (i < (MAX_VAL_DATA_LENGTH - 1)); 
         i++)
    {
        charBuffer[i] = (char)pAction[i];    
    }

    FCore_ModifyControlActionString(&pInfo->FilterCore, 
                                    indication,
                                    state,
                                    charBuffer,
                                    i,
                                    modifyAction);
}

static void 
GetActionStringOverrides(ATHBT_FILTER_INFO *pInfo)
{
    A_CHAR *ptr, *indication, *modify, *action;
    A_CHAR *string = (A_CHAR *)A_MALLOC(MAX_VAL_DATA_LENGTH);

    if (!(gConfigFile)) return;

    fgets(string, MAX_VAL_DATA_LENGTH, gConfigFile);
    while (!(feof(gConfigFile))) {
        ptr = string;
        indication = strsep(&string, ":");
        modify = strsep(&string, ":");
        action = string;
        ProcessActionOverride(pInfo, indication, modify, action);
        string = ptr;
        fgets(string, MAX_VAL_DATA_LENGTH, gConfigFile);
    }

    A_FREE(string);
}
