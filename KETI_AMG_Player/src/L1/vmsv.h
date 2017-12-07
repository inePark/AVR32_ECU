/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Internal API of the Virtual MOST Supervisor (VMSV)
File:           vmsv.h
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
  * \brief      Internal API of the Virtual MOST Supervisor (VMSV)
  */


#ifndef _VMSV_H
#define _VMSV_H
/*lint -e(537) repeated include file */
#include "mostns1.h"

#ifdef VMSV_0
    void VmsvInit(struct TVmsvConfig *cfg_ptr);
#endif

#ifdef VMSV_1
    void VmsvService(void);
#endif

#ifdef VMSV_2
    void VmsvSetPendingEvent(word event_flag);
#endif

#ifdef VMSV_13
    void VmsvHandleDeviceModeStatus(TMsgRx *msg_ptr);
#endif

#ifdef VMSV_14
    void VmsvHandleLockStateStatus(TMsgRx *msg_ptr);
#endif

#ifdef VMSV_16
    void VmsvHandleNIEventStatus(TMsgRx *msg_ptr);
#endif

#ifdef VMSV_17
    void VmsvHandleNIStateStatus(TMsgRx *msg_ptr);
#endif

#ifdef VMSV_18
    void VmsvHandleNCEDelayedStatus(TMsgRx *msg_ptr);
#endif

#ifdef VMSV_58
    void VmsvRefreshNodePos(byte pos);
#endif

#ifdef VMSV_20
    void VmsvHandleNCStateStatus(TMsgRx *msg_ptr);
#endif

#ifdef VMSV_23
    void VmsvHandleRBDResultStatus(TMsgRx *msg_ptr);
#endif

#ifdef VMSV_33
    bool VmsvGetNCStateShutdown(void);
#endif

#ifdef VMSV_34
    void VmsvHandleConfigStatus(TMsgRx *msg_ptr);
#endif

#ifdef VMSV_39
    byte VmsvTxFilter(TMsgTx *msg_ptr);
#endif

#ifdef VMSV_40
    void VmsvRxFilter(TMsgRx *msg_ptr);
#endif

#ifdef VMSV_41
    void VmsvHandleNWStartupError(TMsgRx *msg_ptr);
#endif

#ifdef VMSV_42
    void VmsvHandleNWShutdownError(TMsgRx *msg_ptr);
#endif

#ifdef VMSV_47
    void VmsvHandlePMIStateStatus(TMsgRx *msg_ptr);
#endif

#ifdef VMSV_56
    void VmsvHandleSysErrMonitorStatus(TMsgRx *msg_ptr);
#endif

#ifdef VMSV_70
    void VmsvHandleDiagResult(TMsgRx *msg_ptr);
#endif

#ifdef VMSV_73
    void VmsvHandleNetOnTimerStatus(TMsgRx *msg_ptr);
#endif

#ifdef VMSV_74
    void VmsvHandleNetOnTimerError(TMsgRx *msg_ptr);
#endif

#ifdef VMSV_85
    void VmsvHandleSSOResultStatus(TMsgRx *msg_ptr);
#endif

#ifdef VMSV_86
    void VmsvHandleSSOResultError(TMsgRx *msg_ptr);
#endif

#ifdef VMSV_90
    void VmsvHandleNIWakeupModeStatus(TMsgRx *msg_ptr);
#endif

#ifdef VMSV_91
    void VmsvHandleNIWakeupModeError(TMsgRx *msg_ptr);
#endif

#ifdef VMSV_97
    void VmsvHandleCodingErrorsStatus(TMsgRx *msg_ptr);
#endif

#ifdef VMSV_102
    bool VmsvPhysicalLayerTestActive(void);
#endif



#endif /* _VMSV_H */
