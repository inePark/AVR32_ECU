/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Internal API of Application Message Service (WAMS)
File:           ams.h
Version:        3.0.x-SR-1  
Language:       C
Author(s):      S.Kerber, S.Semmler
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
  * \brief      Internal API of the Application Message Service Wrapper (WAMS)
  */

#ifndef _AMS_H
#define _AMS_H

/*lint -e(537) repeated include file */
#include "mostns1.h"

#ifdef AMS_0
    void MsgInit(struct TMsgConfig *cfg_ptr);
#endif

#ifdef AMS_1
    void MsgService(void);
#endif

#ifdef AMS_2
    void MsgSetPendingEvent(word event_flag);
#endif

#if (defined AMS_7) && (!defined MNS_AVOID_ATTACH)
    void MsgSend3(TMsgTx *msg_ptr);
#endif

#ifdef AMS_8
    byte MsgTxFinal(HMBMBUF handle, byte status);
#endif

#ifdef AMS_10
    void MsgRxTrigger(TMsgRx *msg_ptr);
#endif
#ifdef AMS_12
    void MsgRxError(byte error, TMsgRx *msg_ptr);
#endif
#ifdef AMS_29
    void MsgHandleAbilityToSegmentStatus(TMsgRx *msg_ptr);
#endif

#ifdef AMS_30
    void MsgHandleAbilityToSegmentError(TMsgRx *msg_ptr);
#endif
#ifdef AMS_37
    void MsgHandleMidLevelRetriesStatus(TMsgRx *msg_ptr);
#endif

#ifdef AMS_38
    void MsgNIStateNetOn(bool on);
#endif

#ifdef AMS_39
    void MsgPrepareReInit(void);
#endif

#ifdef AMS_42
    bool MsgDiscardRx(void);
#endif

#ifdef AMS_47
    void MsgHandleRetryOptionsStatus(TMsgRx *msg_ptr);
#endif

#ifdef AMS_48
    void MsgHandleRetryParamsStatus(TMsgRx *msg_ptr);
#endif

#ifdef AMS_49
    void MsgHandleRetryError(TMsgRx *msg_ptr);
#endif

#ifdef AMS_53
    void MsgNIStateNetInit(void);
#endif

#endif /* _AMS_H */

