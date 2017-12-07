/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Adjustment of the Virtual MOST Supervisor (VMSV)
File:           vmsv_ad.h
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
  * \brief      Adjustment of the Virtual MOST Supervisor (VMSV)
  */

#ifndef _VMSV_AD_H
#define _VMSV_AD_H

#include "rules_ad.h"


#ifdef VMSV_MIN

    #define VMSV_0      ((word)   0)    /* VmsvInit                         */
    #define VMSV_1      ((word)   1)    /* VmsvService                      */
    #define VMSV_2      ((word)   2)    /* VmsvSetPendingEvent              */
    #define VMSV_3      ((word)   3)    /* VmsvGetNextEventToHandle         */
    #define VMSV_4      ((word)   4)    /* MostStartUp                      */
    #define VMSV_5      ((word)   5)    /* VmsvNWStartUpRetry               */
    #define VMSV_6      ((word)   6)    /* MostShutDown                     */
    #define VMSV_7      ((word)   7)    /* MostGetState                     */
    #define VMSV_8      ((word)   8)    /* MostGetDevMode                   */
    #define VMSV_9      ((word)   9)    /* MostGetNCState                   */
    #define VMSV_11     ((word)  11)    /* VmsvEHCIGoProtected              */
    #define VMSV_12     ((word)  12)    /* VmsvEHCIGoAttached               */
    #define VMSV_13     ((word)  13)    /* VmsvHandleDeviceModeStatus       */
    #define VMSV_14     ((word)  14)    /* VmsvHandleLockStateStatus        */
    #define VMSV_16     ((word)  16)    /* VmsvHandleNIEventStatus          */
    #define VMSV_17     ((word)  17)    /* VmsvHandleNIStateStatus          */
    #define VMSV_18     ((word)  18)    /* VmsvHandleNCEDelayedStatus       */
    #define VMSV_20     ((word)  20)    /* VmsvHandleNCStateStatus          */
    #define VMSV_22     ((word)  22)    /* MostSetDevMode                   */
    #define VMSV_23     ((word)  23)    /* VmsvHandleRBDResultStatus        */
    #define VMSV_25     ((word)  25)    /* VmsvFireCallbacks                */
    #define VMSV_26     ((word)  26)    /* VmsvHandleNIStateChange          */
    #define VMSV_27     ((word)  27)    /* VmsvHandleNIEventChange          */
    #define VMSV_28     ((word)  28)    /* VmsvHandleNCEDelayedChange       */
    #define VMSV_29     ((word)  29)    /* VmsvHandleLockStateChange        */
    #define VMSV_30     ((word)  30)    /* VmsvHandleRBDResultChange        */
    #define VMSV_31     ((word)  31)    /* MostGetMaxPos                    */
    #define VMSV_32     ((word)  32)    /* VmsvRegisterInternalHooks        */
    #define VMSV_33     ((word)  33)    /* VmsvGetNCStateShutdown           */
    #define VMSV_34     ((word)  34)    /* VmsvHandleConfigStatus           */
    #define VMSV_35     ((word)  35)    /* VmsvHandleNCStateChange          */
    #define VMSV_36     ((word)  36)    /* VmsvHandleFBlockIDsGet           */
    #define VMSV_37     ((word)  37)    /* VmsvInjectFBlockIDsGet           */
    #define VMSV_38     ((word)  38)    /* VmsvRetry                        */
    #define VMSV_39     ((word)  39)    /* VmsvTxFilter                     */
    #define VMSV_40     ((word)  40)    /* VmsvRxFilter                     */
    #define VMSV_41     ((word)  41)    /* VmsvHandleNWStartupError         */
    #define VMSV_42     ((word)  42)    /* VmsvHandleNWShutdownError        */
    #define VMSV_45     ((word)  45)    /* VmsvCheckForNpr                  */
    #define VMSV_46     ((word)  46)    /* VmsvReportNetOn                  */
    #define VMSV_47     ((word)  47)    /* VmsvHandlePMIStateStatus         */
    #define VMSV_48     ((word)  48)    /* VmsvHandlePMIStateChange         */
    #define VMSV_49     ((word)  49)    /* MostGetPMIState                  */
    #define VMSV_54     ((word)  54)    /* MostGetSysErrMonitor             */
    #define VMSV_55     ((word)  55)    /* VmsvRequestSysErrMonitor         */
    #define VMSV_56     ((word)  56)    /* VmsvHandleSysErrMonitorStatus    */
    #define VMSV_57     ((word)  57)    /* VmsvReportSysErrMonitor          */
    #define VMSV_58     ((word)  58)    /* VmsvRefreshNodePos               */
    #define VMSV_59     ((word)  59)    /* VmsvInjectCfgState               */
    #define VMSV_60     ((word)  60)    /* VmsvEHCIGoSemiProtected          */

  #ifdef MSV_DIAG_RESULT_MSG
    #define VMSV_66     ((word)  66)    /* VmsvCommRBDResult                */
    #define VMSV_67     ((word)  67)    /* VmsvCommRBDResultPos0            */
    #define VMSV_68     ((word)  68)    /* VmsvCommRBDResultPosX            */
    #define VMSV_69     ((word)  69)    /* VmsvBroadcastRBDResult           */
    #define VMSV_70     ((word)  70)    /* VmsvHandleDiagResult             */
  #endif

    #define VMSV_71     ((word)  71)    /* MostGetNetOnTime                 */
    #define VMSV_72     ((word)  72)    /* VmsvRequestNetOnTime             */
    #define VMSV_73     ((word)  73)    /* VmsvHandleNetOnTimerStatus       */
    #define VMSV_74     ((word)  74)    /* VmsvHandleNetOnTimerError        */
    #define VMSV_81     ((word)  81)    /* MostGetLockState                 */

  #ifdef _OS81110_SSO
    #define VMSV_85     ((word)  85)    /* VmsvHandleSSOResultStatus        */
    #define VMSV_86     ((word)  86)    /* VmsvHandleSSOResultError         */
    #define VMSV_87     ((word)  87)    /* VmsvResetSSOResult               */
    #define VMSV_88     ((word)  88)    /* VmsvGetSSOResult                 */
    #define VMSV_99     ((word)  99)    /* VmsvResetINICSSOResult           */
  #endif

  #ifdef MSV_DIAG_RESULT_MSG
    #define VMSV_90     ((word)  90)    /* VmsvHandleNIWakeupModeStatus     */
    #define VMSV_91     ((word)  91)    /* VmsvHandleNIWakeupModeError      */
    #define VMSV_92     ((word)  92)    /* VmsvRequestNIWakeUpMode          */
    #define VMSV_93     ((word)  93)    /* VmsvSetNIWakeUpMode              */
  #endif

  #ifdef NS_MSV_ET
    #define VMSV_94     ((word)  94)    /* VmsvPhysicalLayerTestStart       */
    #define VMSV_95     ((word)  95)    /* VmsvPhysicalLayerTestProcedure   */
    #define VMSV_96     ((word)  96)    /* VmsvPhysicalLayerTestResult      */
    #define VMSV_97     ((word)  97)    /* VmsvHandleCodingErrorsStatus     */
    #define VMSV_98     ((word)  98)    /* VmsvPhysicalLayerTestTxFinal     */
    #define VMSV_102    ((word) 102)    /* VmsvPhysicalLayerTestActive      */
  #endif

    #define VMSV_100    ((word) 100)    /* MostEmergencyCondition           */




    #define VMSV_NWM_STD_INST_ID ((byte) 0x01)
    #define VMSV_NWM_INST_ID     ((byte) 0x00)

  #ifdef MSV_DIAG_RESULT_MSG
    #define VMSV_DC_T_PREPARE_RBD   ((word)  200)
    #define VMSV_DC_T_RBD           ((word) 1000)
    #define VMSV_DC_T_WAIT_RBD      ((word) 2000)
  #endif


    #define MSV_ADD9_OPT1

  #ifdef MDM_MSV2_NWM_CONF_STATE_NOK
    #define VMSV_101     ((word) 101)   /* VmsvSendDbgMsv2NwmConfStateNok   */
  #endif

#endif /* #ifdef VMSV_MIN */

#endif /* _VMSV_AD_H */
