/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Private declarations and definitions of Application Message 
                Service Wrapper (WAMS)
File:           ams_pv.h
Version:        3.0.x-SR-1  
Language:       C
Author(s):      T.Jahnke
Date:           05.January.2011

FileGroup:      Layer I
Customer ID:    0018FF2A0300xx.N.KETI
FeatureCode:    FCR1
------------------------------------------------------------------------------

                (c) Copyright 1998-2011
                SMSC
                All Rights Reserved

------------------------------------------------------------------------------



Modifications
~~~~~~~~~~~~~
Date            By      Description

==============================================================================
*/

/*! \file
  * \brief      Private declarations and definitions of Application Message 
  *             Service Wrapper (WAMS)
  */

#ifndef _AMS_PV_H
#define _AMS_PV_H


/*
==============================================================================
    Includes
==============================================================================
*/

#include "ams.h"


/*
==============================================================================
    Module Internal Definitions
==============================================================================
*/


/*
==============================================================================
    Module Internal Macros
==============================================================================
*/
#define AMS_RETRY_MIDLEVEL_RETRIES      ((word)0x0001)
#define MSG_RETRY_SEGMODE               ((word)0x0002)
#define MSG_RETRY_REQ_SEGMODE           ((word)0x0004)
#define MSG_RETRY_REQ_RETRYPARAMS       ((word)0x0008)
#define MSG_RETRY_RETRYPARAMS           ((word)0x0010)

#define MSG_RX_F_NOT_FINISHED   ((byte) 0x80)   /* Flag in field "UsageCnt" (RX section). It signals, that the message      */
                                                /* must not be deleted (by another application), due to a retrigger request */
                                                /* from one application.                                                    */

/*
==============================================================================
    Module Internal Structures
==============================================================================
*/

#ifdef AMS_MIN

struct TMsgSegATS
{
    byte value;

    byte shadow;
};

struct TMsgSegConfig
{
    struct TMsgSegATS ats;

    byte mode;
};

struct TMsgRetryParamsShadow
{
    byte time;
    byte total_attempts;
};

struct TMsgRetryParams
{
    byte time;
    byte total_attempts;
    bool busy;
    struct TMsgRetryParamsShadow shadow;
};

typedef struct MsgData
{
    word pending_events;
    word latest_handled_event;
    bool net_on;

    MbmQueue rx_queue;
    MbmQueue tx_queue;

    word retry;

    struct TMsgConfig *cfg_ptr;

    struct TMsgSegConfig seg;

    struct TMsgRetryParams retry_cfg;

    byte mid_level_retries;

    #ifdef NS_AMS_MSV2
    void (*cbTxMsgFlushed_fptr)(TMsgTx *msg_ptr);
    #endif
} TMsgData;

#endif  /* #ifdef AMS_MIN */


/*
==============================================================================
    Module Internal Function Prototypes
==============================================================================
*/

#ifdef AMS_3
    static word MsgGetNextEventToHandle(void);
#endif

#ifdef AMS_11
    static void MsgRxOutTrigger(TMsgRx *msg_ptr);
#endif

#ifdef AMS_26
    static void MsgEHCIGoProtected(void);
#endif
#ifdef AMS_27
    static void MsgEHCIGoSemiProtected(void);
#endif

#ifdef AMS_28
    static byte MsgRetry(void);
#endif

#ifdef AMS_33
    static byte MsgForceSegMode(void);
#endif

#ifdef AMS_34
    static byte MsgRequestSegMode(void);
#endif

#ifdef AMS_34
    static byte MsgForceMidLevelRetries(void);
#endif
#ifdef AMS_45
    static byte MsgRequestRetryParams(void);
#endif

#ifdef AMS_46
    static byte MsgSendRetryParams(void);
#endif

#ifdef MSG_TX_USER_PAYLOAD
    #ifdef NS_AMS_AH
    void AddrHClearTasks(void);
    #endif
#endif




#endif /* #ifndef _AMS_PV_H */

/*
==============================================================================
    End Of File
==============================================================================
*/
