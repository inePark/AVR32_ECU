/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Adjustment of the Socket Connection Manager Wrapper (WSCM)
File:           wscm_ad.h
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
  * \brief      Adjustment of the Socket Connection Manager Wrapper (WSCM)
  */

#ifndef _WSCM_AD_H
#define _WSCM_AD_H

#include "rules_ad.h"

#ifdef SCM_MIN

    #define SCM_0  ((word)  0) /* ScmInit                           */
    #define SCM_1  ((word)  1) /* ScmService                        */
    #define SCM_2  ((word)  2) /* ScmSetPendingEvent                */
    #define SCM_3  ((word)  3) /* ScmGetNextEventToHandle           */
    #define SCM_4  ((word)  4) /* ScmGoProtected                    */
    #define SCM_5  ((word)  5) /* ScmSendHandleMsg                  */
    #define SCM_6  ((word)  6) /* ScmSetByteByMsg                   */
    #define SCM_7  ((word)  7) /* ScmOpenPort                       */
    #define SCM_8  ((word)  8) /* ScmCreateSocket                   */
    #define SCM_10 ((word) 10) /* ScmGetMuteMode                    */
    #define SCM_11 ((word) 11) /* ScmGetBoundary                    */
    #define SCM_15 ((word) 15) /* ScmAllocOnlyMlb                   */
    #define SCM_16 ((word) 16) /* ScmDeallocOnlyMlb                 */
    #define SCM_18 ((word) 18) /* ScmHandleResult                   */
    #define SCM_19 ((word) 19) /* ScmHandleCreateSocketResult       */
    #define SCM_20 ((word) 20) /* ScmHandleConnectSocketsResult     */
    #define SCM_21 ((word) 21) /* ScmHandleMuteModeStatus           */
    #define SCM_22 ((word) 22) /* ScmHandleMlbAllocOnlyResult       */
    #define SCM_24 ((word) 24) /* ScmHandleBandwidthStatus          */
    #define SCM_26 ((word) 26) /* ScmHandleError                    */
    #define SCM_27 ((word) 27) /* ScmClosePort                      */
    #define SCM_28 ((word) 28) /* ScmHandleSourceDropStatus         */
    #define SCM_29 ((word) 29) /* ScmHandleSCErrorStatus            */
    #define SCM_30 ((word) 30) /* ScmGetBoundaryExt                 */
    #define SCM_31 ((word) 31) /* ScmDestroySocketExt               */
    #define SCM_32 ((word) 32) /* ScmConnectSocketsExt              */
    #define SCM_35 ((word) 35) /* ScmGetSource                      */
    #define SCM_36 ((word) 36) /* ScmAssembleErrResList             */
    #define SCM_37 ((word) 37) /* ScmHandleRemoteGetSourceStatus    */
    #define SCM_38 ((word) 38) /* ScmPMCheck                        */
    #define SCM_39 ((word) 39) /* ScmPMService                      */
    #define SCM_40 ((word) 40) /* ScmPMCreateResult                 */
    #define SCM_41 ((word) 41) /* ScmPMDestroyResult                */
    #define SCM_42 ((word) 42) /* ScmPMComplete                     */
    #define SCM_43 ((word) 43) /* ScmPMRecheck                      */
    #define SCM_44 ((word) 44) /* ScmHandleSCDemuteStatus           */
    #define SCM_45 ((word) 45) /* ScmPMOpenPortResult               */

#endif /* #ifdef SCM_MIN */

#endif /* _WSCM_AD_H */
