/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Internal API of the Socket Connection Manager Wrapper (WSCM)
File:           wscm.h
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
  * \brief      Internal API of the Socket Connection Manager Wrapper (WSCM)
  */

#ifndef _WSCM_H
#define _WSCM_H

/*lint -e(537) repeated include file */
#include "mostns1.h"

#ifdef SCM_0
    void ScmInit(struct TScmConfig *cfg_ptr);
#endif

#ifdef SCM_1
    void ScmService(void);
#endif

#ifdef SCM_2
    void ScmSetPendingEvent(word event_flag);
#endif

#ifdef SCM_18
    void ScmHandleResult(TMsgRx *msg_ptr);
#endif

#ifdef SCM_19
    void ScmHandleCreateSocketResult(TMsgRx *msg_ptr);
#endif

#ifdef SCM_20
    void ScmHandleConnectSocketsResult(TMsgRx *msg_ptr);
#endif

#ifdef SCM_21
    void ScmHandleMuteModeStatus(TMsgRx *msg_ptr);
#endif

#ifdef SCM_22
    void ScmHandleMlbAllocOnlyResult(TMsgRx *msg_ptr);
#endif

#ifdef SCM_24
    void ScmHandleBandwidthStatus(TMsgRx *msg_ptr);
#endif

#ifdef SCM_26
    void ScmHandleError(TMsgRx *msg_ptr);
#endif

#ifdef SCM_28
    void ScmHandleSourceDropStatus(TMsgRx *msg_ptr);
#endif

#ifdef SCM_29
    void ScmHandleSCErrorStatus(TMsgRx *msg_ptr);
#endif

#ifdef SCM_37
    void ScmHandleRemoteGetSourceStatus(TMsgRx *msg_ptr);
#endif

#ifdef SCM_42
    bool ScmPMComplete(void);
#endif

#ifdef SCM_44
    void ScmHandleSCDemuteStatus(TMsgRx  *msg_ptr);
#endif


#endif  /* _WSCM_H */
