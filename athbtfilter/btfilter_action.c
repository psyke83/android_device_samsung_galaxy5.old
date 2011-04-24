//------------------------------------------------------------------------------
// <copyright file="btfilter_action.c" company="Atheros">
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
// Bluetooth filter core action tables and action lookup
//
// Author(s): ="Atheros"
//==============================================================================

#include <stdlib.h>
#include <string.h>
#include "athdefs.h"
//#include "a_debug.h"
//#include "a_types.h"
//#include "a_osapi.h"
#include "wmi.h"
#include "athbtfilter.h"
#include "btfilter_core.h"

#define ACTION_NOOP          NULL
#define ACTION_TODO_TBD      NULL

static void CleanupModifiedControlActionDescChain(BT_CONTROL_ACTION_DESC  *pDesc);

/* default actions descriptor table
 *
 * NOTE: the pNext member of each entry must be set to NULL, when this table is copied
 *  to a RAM table so that the pNext field can be altered at run time in case the entry must be
 *  extended or overridden by the user */
static const BT_CONTROL_ACTION_DESC_STATE g_ActionDefaults[ATH_BT_MAX_STATE_INDICATION] =
{
                                        /* STATE OFF */                     /* STATE ON */
    /* ATH_BT_NOOP    */      {{{ ACTION_NOOP,          0, NULL }, { ACTION_NOOP,          0, NULL }}},
    /* ATH_BT_INQUIRY */      {{{ "-s 3 2",             0, NULL }, { "-s 3 1",             0, NULL }}},
    /* ATH_BT_CONNECT */      {{{ ACTION_TODO_TBD,      0, NULL }, { ACTION_TODO_TBD,      0, NULL }}},
    /* ATH_BT_SCO     */      {{{ "-s 1 2",             0, NULL }, { "-pSCO 10 20 2 0 0 0 2 6 1 2 4 0; -s 1 1", 0, NULL }}},
    /* ATH_BT_ACL     */      {{{ ACTION_TODO_TBD,      0, NULL }, { ACTION_TODO_TBD,      0, NULL }}},
    /* ATH_BT_A2DP    */      {{{ "-s 2 2",             0, NULL }, { "-pA2DP 30 3 20 0 0 0 0 0 0; -s 2 1", 0, NULL }}},
    /* ATH_BT_ESCO    */      {{{ "-s 4 2",             0, NULL }, { "-pSCO 1 20 2 0 0 0 2 6 3 2 10 0 ; -s 4 1",0, NULL }}},
};

typedef enum _BT_COMMAND_TAG {
    BT_STATUS_TAG               = 0,
    BT_PARAM_SCO_TAG            = 1,
    BT_PARAM_A2DP_TAG,
    /* add new tags here */
    BT_COMMAND_TAGS_MAX
} BT_COMMAND_TAG;


static const A_CHAR *g_TagStrings[BT_COMMAND_TAGS_MAX] = {
    "-s",
    "-pSCO",
    "-pA2DP",
};

static int FindActionString(A_CHAR *pString, A_CHAR **ppStart)
{
    int     count = 0;
    A_BOOL    found = FALSE;

    if (*pString == ';') {
            /* skip if this is the first one we hit */
        pString++;
    }

    while ((*pString != '\0') && (*pString != ';')) {
        if (!found) {
            if (*pString == '-') {
                    /* found start of one action string */
                *ppStart = pString;
                found = TRUE;
                count++;
            }
        } else {
            count++;
        }
        pString++;
    }

    return count;
}

#define ACTION_STRING_DELIMIT  ' '

    /* assemble an array of pointers to each argument in the string */
static int GetArgStrings(A_CHAR *pString, 
                         A_CHAR  Delimit,
                         A_CHAR  *pArgArray[],
                         int     MaxArgs)
{
    int     totalArgs = 0;
    int     length;
    A_BOOL  done = FALSE;
    
    while (!done && (totalArgs < MaxArgs)) {
        
        pArgArray[totalArgs] = pString;
        length = 0;
        
            /* walk through the string and assemble substrings */
        while (1) {     
            
            if ((*pString == '\0') || (*pString == Delimit)) {
                               
                if (*pString == '\0') {
                        /* reached the end of the string */
                    done = TRUE;             
                } else {            
                        /* terminate */
                    *pString = (A_CHAR)0;
                }
                    /* advance pass NULL */
                pString++;
                
                if (length) {
                        /* only increment arg if we found a non-zero length string */
                    totalArgs++;
                }
                
                break;
            }
            
            pString++;
            length++;
        }
        
    }
     
    return totalArgs;
}

static int GetArguments(A_CHAR *pString, A_INT32 *pArgArray, int MaxArgs)
{
    int     i;
    int     totalArgs;
    int     argsFound;
    A_CHAR  *argStrings[BT_ACTION_MAX_ARGS];

    
    totalArgs = GetArgStrings(pString,
                              ACTION_STRING_DELIMIT,
                              argStrings,
                              BT_ACTION_MAX_ARGS);
    
    argsFound = 0;
                                                                    
    for (i = 0; (i < totalArgs); i++) {
        
        if (argStrings[i][0] == '-') {
            /* skip the action tag */
            continue;
        }
        
        if (argsFound < MaxArgs) {
                /* got an arg */
            pArgArray[argsFound] = atol(argStrings[i]);
            argsFound++;
        }
    }

    return argsFound;
}

static A_STATUS BuildActionFromString(A_CHAR *pActionString, int Length, BT_CONTROL_ACTION_ITEM **pControlItem)
{
    A_CHAR                  stringBuf[BT_ACTION_STRING_MAX_LENGTH + 1];
    int                     stringLength = min(Length,BT_ACTION_STRING_MAX_LENGTH);
    BT_COMMAND_TAG          tag = BT_COMMAND_TAGS_MAX;
    BT_CONTROL_ACTION_ITEM  *pItem = NULL;
    A_STATUS                status = A_OK;
    A_INT32                 args[BT_ACTION_MAX_ARGS];
    int                     argCount;
    WMI_SET_BT_STATUS_CMD   *pStatusCmd = NULL;
    WMI_SET_BT_PARAMS_CMD   *pParamsCmd = NULL;


    pItem = (BT_CONTROL_ACTION_ITEM *)A_MALLOC(sizeof(BT_CONTROL_ACTION_ITEM));

    if (NULL == pItem) {
        return A_NO_MEMORY;
    }

    A_MEMZERO(pItem,sizeof(BT_CONTROL_ACTION_ITEM));
        /* copy string to temp buffer */
    A_MEMCPY(stringBuf, pActionString, stringLength);
    stringBuf[stringLength] = '\0';

        /* Parse string for command type */
    for (tag = 0; tag < BT_COMMAND_TAGS_MAX; tag++) {
        if (strstr(stringBuf, g_TagStrings[tag]) != NULL) {
            argCount = GetArguments(stringBuf,args,BT_ACTION_MAX_ARGS);
            break;
        }
    }

/*
 * BT STATUS:
 *      "-s <streamtype> <status>"
 *
 * BT PARAM SCO:
 *      "-pSCO <numScoCyclesForceTrigger> <dataResponseTimeout> <stompDutyCyleVal> <psPollLatencyFraction>
 *                  <stompScoRules>  <noSCOSlots> <noIdleSlots> "
 *
 * BT PARAM A2DP
 *      "-pA2DP <a2dpWlanUsageLimit> <a2dpBurstCntMin> <a2dpDataRespTimeout "
 *
 */

    switch (tag) {
        case BT_STATUS_TAG:
                /* set up item and struct pointer */
            pItem->ControlAction.Type = BT_CONTROL_ACTION_STATUS;
            pItem->ControlAction.Length = sizeof(WMI_SET_BT_STATUS_CMD);
            pStatusCmd = (WMI_SET_BT_STATUS_CMD *)pItem->ControlAction.Buffer;
            break;

        case BT_PARAM_SCO_TAG:
        case BT_PARAM_A2DP_TAG:
                /* these all use the same WMI message */
            pItem->ControlAction.Type = BT_CONTROL_ACTION_PARAMS;
            pItem->ControlAction.Length = sizeof(WMI_SET_BT_PARAMS_CMD);
            pParamsCmd = (WMI_SET_BT_PARAMS_CMD *)pItem->ControlAction.Buffer;
            break;

        default:
            break;

    }

    status = A_OK;

    switch (tag) {

        case BT_STATUS_TAG:
            if (argCount != 2) {
                status = A_EINVAL;
                break;
            }
            pStatusCmd->streamType = (A_UINT8)args[0];
            pStatusCmd->status =     (A_UINT8)args[1];
            break;

        case BT_PARAM_SCO_TAG:
            if (argCount != 12) {
                status = A_EINVAL;
                break;
            }
            pParamsCmd->paramType = BT_PARAM_SCO;
            pParamsCmd->info.scoParams.numScoCyclesForceTrigger =
                                                    (A_UINT8)args[0];
            pParamsCmd->info.scoParams.dataResponseTimeout =
                                                     (A_UINT8)args[1];
            pParamsCmd->info.scoParams.stompScoRules =
                                                    (A_UINT8)args[2];
            pParamsCmd->info.scoParams.scoOptFlags =  (A_UINT8)args[3];
            pParamsCmd->info.scoParams.p2lrpOptModeBound =
                                                    (A_UINT8)args[4];
            pParamsCmd->info.scoParams.p2lrpNonOptModeBound =
                                                    (A_UINT8)args[5];
            pParamsCmd->info.scoParams.stompDutyCyleVal =
                                                    (A_UINT8)args[6];
            pParamsCmd->info.scoParams.stompDutyCyleMaxVal =  
                                                    (A_UINT8)args[7];
            pParamsCmd->info.scoParams.psPollLatencyFraction =
                                                    (A_UINT8)args[8];
            pParamsCmd->info.scoParams.noSCOSlots = (A_UINT8)args[9];
            pParamsCmd->info.scoParams.noIdleSlots = (A_UINT8)args[10];
            pParamsCmd->info.scoParams.reserved8 =(A_UINT8)args[11];
            break;

        case BT_PARAM_A2DP_TAG:
            if (argCount != 9) {
                status = A_EINVAL;
                break;
            }
            pParamsCmd->paramType = BT_PARAM_A2DP;
            pParamsCmd->info.a2dpParams.a2dpWlanUsageLimit = (A_UINT8)args[0];
            pParamsCmd->info.a2dpParams.a2dpBurstCntMin = (A_UINT8)args[1];
            pParamsCmd->info.a2dpParams.a2dpDataRespTimeout = (A_UINT8)args[2];
            pParamsCmd->info.a2dpParams.a2dpOptFlags = (A_UINT8)args[3];
            pParamsCmd->info.a2dpParams.p2lrpOptModeBound = (A_UINT8)args[4];
            pParamsCmd->info.a2dpParams.p2lrpNonOptModeBound = (A_UINT8)args[5];
            pParamsCmd->info.a2dpParams.reserved16 = (A_UINT8)args[6];
            pParamsCmd->info.a2dpParams.isCoLocatedBtRoleMaster = (A_UINT8)args[7];
            pParamsCmd->info.a2dpParams.reserved8 = (A_UINT8)args[8];
            break;

        default:
            break;
    }

    if (A_FAILED(status)) {
        A_FREE(pItem);
    } else {
        *pControlItem = pItem;
    }

    return status;
}

/* assemble the action list based on the descriptor list */
static A_STATUS AssembleActionList(BT_CONTROL_ACTION_DESC *pDesc, DL_LIST *pActionListHead)
{
    BT_CONTROL_ACTION_ITEM  *pControlItem;
    A_CHAR                  *pActionString;
    A_CHAR                  *pStringToScan;
    int                     length;
    A_STATUS                status = A_OK;
    
    for ( ;((pDesc != NULL) && A_SUCCESS(status)); pDesc = pDesc->pNext) {
        if (NULL == pDesc->pActionString) {
            continue;
        }
            /* scan the action string and assemble all actions */
        pStringToScan = pDesc->pActionString;

        while (1) {

            length = FindActionString(pStringToScan, &pActionString);

            if (0 == length) {
                break;
            }
                /* found the string, now build the action */
            status = BuildActionFromString(pActionString, length , &pControlItem);

            if (A_FAILED(status)) {
                break;
            }

                /* insert into the list head in FIFO order */
            DL_ListInsertTail(pActionListHead,&pControlItem->ListEntry);

                /* scan the rest of the string */
            pStringToScan = pActionString + length;
        }
    }

    return status;
}


static void CleanListItems(DL_LIST  *pListHead)
{
    DL_LIST                 *pListItem;
    BT_CONTROL_ACTION_ITEM  *pControlItem;

    while (1) {
        pListItem = DL_ListRemoveItemFromHead(pListHead);
        if (NULL == pListItem) {
            break;
        }
        pControlItem = A_CONTAINING_STRUCT(pListItem, BT_CONTROL_ACTION_ITEM, ListEntry);
            /* free it */
        A_FREE(pControlItem);
    }
}


static void CleanupActionLists(BT_FILTER_CORE_INFO *pCore)
{
    int  indication,state;

    for (state = 0; state < STATE_MAX; state++) {
            /* free all action list items */
        for (indication = 0; indication < ATH_BT_MAX_STATE_INDICATION; indication++) {
            CleanListItems(&pCore->ActionListsState[indication][state]);
        }
    }
}

/* API implementation */
A_BOOL FCore_Init(BT_FILTER_CORE_INFO *pCore)
{
    int indication,state;

    for (state = 0; state < STATE_MAX; state++) {
            /* initialize all action list heads */
        for (indication = 0; indication < ATH_BT_MAX_STATE_INDICATION; indication++) {
            DL_LIST_INIT(&pCore->ActionListsState[indication][state]);
        }
    }
        /* load up the default action descriptors */
    memcpy(pCore->ActionDescriptors, g_ActionDefaults, sizeof(pCore->ActionDescriptors));

    pCore->FilterState.ACL_ConnectionHandle = INVALID_BT_CONN_HANDLE;
    pCore->FilterState.eSCO_ConnectionHandle = INVALID_BT_CONN_HANDLE;
    pCore->FilterState.SCO_ConnectionHandle = INVALID_BT_CONN_HANDLE;

    return TRUE;
}

/* assembly action lists from the action table, this is called after the OS-porting layer
 * has had a chance to alter the action table */
A_STATUS FCore_RefreshActionList(BT_FILTER_CORE_INFO *pCore)
{
    int         indication,state;
    A_STATUS    status = A_OK;

        /* clean up previous ones */
    CleanupActionLists(pCore);

    for (state = 0; (state < STATE_MAX) && A_SUCCESS(status); state++) {
        for (indication = 0; (indication < ATH_BT_MAX_STATE_INDICATION) && A_SUCCESS(status); indication++) {
            status = AssembleActionList(&pCore->ActionDescriptors[indication].Action[state],
                                        &pCore->ActionListsState[indication][state]);
        }
    }

    if (A_FAILED(status)) {
        FCore_Cleanup(pCore);
    }

    return status;
}

void FCore_Cleanup(BT_FILTER_CORE_INFO *pCore)
{
    int indication,state;

    for (state = 0; state < STATE_MAX; state++) {
        for (indication = 0; indication < ATH_BT_MAX_STATE_INDICATION; indication++) {
            CleanupModifiedControlActionDescChain(&pCore->ActionDescriptors[indication].Action[state]);
        }
    }

    CleanupActionLists(pCore);
}


BT_CONTROL_ACTION_ITEM *FCore_GetControlAction(BT_FILTER_CORE_INFO       *pCore,
                                               ATHBT_STATE_INDICATION    Indication,
                                               ATHBT_STATE               State,
                                               BT_CONTROL_ACTION_ITEM    *pStart)
{
    DL_LIST  *pListHead;

    if (State >= STATE_MAX) {
        return NULL;
    }

    pListHead = &pCore->ActionListsState[Indication][State];

    if (NULL == pStart) {
            /* caller wants the first item in the list */
        if (pListHead->pNext == pListHead) {
                /* list is empty */
            return NULL;
        }
        return A_CONTAINING_STRUCT(pListHead->pNext, BT_CONTROL_ACTION_ITEM, ListEntry);
    } else {
            /* caller wants the next list item */
        if (pStart->ListEntry.pNext == pListHead) {
                /* reached the end */
            return NULL;
        }
        return A_CONTAINING_STRUCT(pStart->ListEntry.pNext, BT_CONTROL_ACTION_ITEM, ListEntry);
    }
}

static void CleanupModifiedControlActionDescChain(BT_CONTROL_ACTION_DESC  *pDesc)
{
    BT_CONTROL_ACTION_DESC  *pNextDesc;

    while (pDesc) {
        pNextDesc = pDesc->pNext;
        if (pDesc->Flags & BT_CA_DESC_FLAGS_ALLOCATED) {
            A_FREE(pDesc);
        }
        pDesc = pNextDesc;
    }

}

A_STATUS FCore_ModifyControlActionString(BT_FILTER_CORE_INFO          *pCore,
                                         ATHBT_STATE_INDICATION       Indication,
                                         ATHBT_STATE                  State,
                                         A_CHAR                       *pAction,
                                         int                          StringLength,
                                         ATHBT_MODIFY_CONTROL_ACTION  ModifyAction)
{
    BT_CONTROL_ACTION_DESC  *pNewDesc;
    BT_CONTROL_ACTION_DESC  *pDescHead;
    A_CHAR                  *pString;

        /* get first entry at the head */
    pDescHead = &pCore->ActionDescriptors[Indication].Action[State];

        /* allocate and assemble an entry for this action */
    pNewDesc = (BT_CONTROL_ACTION_DESC  *)A_MALLOC(sizeof(BT_CONTROL_ACTION_DESC) + StringLength + 1);

    if (NULL == pNewDesc) {
        return A_NO_MEMORY;
    }

    A_MEMZERO(pNewDesc,sizeof(BT_CONTROL_ACTION_DESC));

        /* setup and copy string */
    pString = (A_CHAR *)((A_UINT8 *)pNewDesc + sizeof(BT_CONTROL_ACTION_DESC));
    A_MEMCPY(pString,pAction,StringLength);
    pString[StringLength] = 0;
    pNewDesc->pActionString = pString;
        /* mark that it was allocated so we can clean it up later */
    pNewDesc->Flags = BT_CA_DESC_FLAGS_ALLOCATED;

    switch (ModifyAction) {
        case ATHBT_MODIFY_CONTROL_ACTION_APPEND:
                /* append to the end of the list */
            while (pDescHead->pNext != NULL) {
                pDescHead = pDescHead->pNext;
            }
            pDescHead->pNext = pNewDesc;
            break;

        case ATHBT_MODIFY_CONTROL_ACTION_REPLACE:
                /* remove any existing replacements or append operations */
            CleanupModifiedControlActionDescChain(pDescHead);
                /* ignore the first entry's action string */
            pDescHead->pActionString = NULL;
                /* add new replacement */
            pDescHead->pNext = pNewDesc;
            break;

        default:
            A_FREE(pNewDesc);
            break;
    }


    return A_OK;
}
