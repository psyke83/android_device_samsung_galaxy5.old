//------------------------------------------------------------------------------
// <copyright file="abtfilt_bluez_dbus.h" company="Atheros">
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

#ifndef ABTFILT_BTSTACK_DBUS_H_
#define ABTFILT_BTSTACK_DBUS_H_
#include "abtfilt_int.h"
#include <dbus/dbus.h>

/*-----------------------------------------------------------------------*/
/* BT Section */
#define STRING_SIZE_MAX             128
#define BD_ADDR_SIZE                6


typedef void (* BT_EVENT_HANDLER)(void *,void *);

typedef enum {
    BT_ADAPTER_ADDED = 0,
    BT_ADAPTER_REMOVED,
    DEVICE_DISCOVERY_STARTED,
    DEVICE_DISCOVERY_FINISHED,
    REMOTE_DEVICE_CONNECTED,
    REMOTE_DEVICE_DISCONNECTED,
    AUDIO_DEVICE_ADDED,
    AUDIO_DEVICE_REMOVED,
    AUDIO_HEADSET_CONNECTED,
    AUDIO_HEADSET_DISCONNECTED,
    AUDIO_HEADSET_STREAM_STARTED,
    AUDIO_HEADSET_STREAM_STOPPED,
    AUDIO_GATEWAY_CONNECTED, /* Not Implemented */
    AUDIO_GATEWAY_DISCONNECTED, /* Not Implemented */
    AUDIO_SINK_CONNECTED,
    AUDIO_SINK_DISCONNECTED,
    AUDIO_SINK_STREAM_STARTED,
    AUDIO_SINK_STREAM_STOPPED,
    AUDIO_SOURCE_CONNECTED, /* Not Implemented */
    AUDIO_SOURCE_DISCONNECTED, /* Not Implemented */
    BT_EVENTS_NUM_MAX,
} BT_STACK_EVENT;

typedef struct _ABF_BT_INFO {
    ATHBT_FILTER_INFO              *pInfo;
    A_MUTEX_OBJECT                  hWaitEventLock;
    A_BOOL                          AdapterAvailable;
    DBusConnection                 *Bus;
    A_UINT8                         HCIVersion;
    A_UINT16                        HCIRevision;
    A_UINT8                         HCI_LMPVersion;
    A_UINT16                        HCI_LMPSubVersion;
    A_UINT8                         RemoteDevice[BD_ADDR_SIZE];
    A_UINT8                         HCI_DeviceAddress[BD_ADDR_SIZE];
    A_CHAR                          HCI_AdapterName[STRING_SIZE_MAX];
    A_CHAR                          HCI_DeviceName[STRING_SIZE_MAX];
    A_CHAR                          HCI_ManufacturerName[STRING_SIZE_MAX];
    A_CHAR                          HCI_ProtocolVersion[STRING_SIZE_MAX];
    A_BOOL                          AdapterCbRegistered;
    A_CHAR                          DefaultAudioDeviceName[STRING_SIZE_MAX];
    A_CHAR                          DefaultRemoteAudioDeviceAddress[32];
    A_CHAR                          DefaultRemoteAudioDeviceVersion[32];
    A_UINT8                         DefaultAudioDeviceLmpVersion;
    A_BOOL                          DefaultAudioDeviceAvailable;
    A_BOOL                          AudioCbRegistered;
    A_UCHAR                         CurrentSCOLinkType;
    int                             AdapterId;
    int                             HCIEventListenerSocket;
    A_TASK_HANDLE                   hBtHCIFilterThread;
    A_BOOL                          HCIFilterThreadCreated;
    A_BOOL                          HCIFilterThreadShutdown;
    BT_EVENT_HANDLER                SignalHandlers[BT_EVENTS_NUM_MAX];
    A_BOOL                          DefaultRemoteAudioDevicePropsValid;
    A_BOOL                          ThreadCreated;
} ABF_BT_INFO;

#endif /*ABTFILT_BTSTACK_DBUS_H_*/
