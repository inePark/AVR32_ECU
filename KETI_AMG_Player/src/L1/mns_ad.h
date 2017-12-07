/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Adjustment of the MOST NetServices Kernel (Basic Layer)
File:           mns_ad.h
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
  * \brief      Adjustment of the MOST NetServices Kernel (Basic Layer)
  */

#ifndef _MNS_AD_H
#define _MNS_AD_H

#include "rules_ad.h"

#ifdef MNS_MIN


    #define MNS_0       ((word)  0)             /* InitNetServices              */
    #define MNS_1       ((word)  1)             /* MostService                  */
    #define MNS_2       ((word)  2)             /* MnsSetPendingService         */
    #define MNS_3       ((word)  3)             /* MnsSetPendingEventFlag       */
    #define MNS_4       ((word)  4)             /* MnsGetNextEventFlagToCall    */
    #define MNS_5       ((word)  5)             /* MnsGetNextServiceToCall      */
    #define MNS_6       ((word)  6)             /* MnsService                   */
    #define MNS_7       ((word)  7)             /* MnsSetPendingEvent           */
    #define MNS_8       ((word)  8)             /* MnsGetNextEventToHandle      */
    #define MNS_9       ((word)  9)             /* MnsDistribEvent              */
    #define MNS_10      ((word) 10)             /* MnsInit                      */
    #define MNS_11      ((word) 11)             /* MnsServiceInitComplete       */
    #define MNS_12      ((word) 12)             /* MnsIsNSInitComplete          */
    #define MNS_15      ((word) 15)             /* MostClearTimer               */
    #define MNS_16      ((word) 16)             /* MostSetTimer                 */
    #define MNS_17      ((word) 17)             /* MostGetTimer                 */
    #define MNS_20      ((word) 20)             /* MnsReportError               */
    #define MNS_21      ((word) 21)             /* MnsReportInitComplete        */
    #define MNS_22      ((word) 22)             /* MnsEHCIGoProtected           */
    #ifndef MNS_AVOID_ATTACH
        #define MNS_23  ((word) 23)             /* MnsEHCIGoSemiProtected       */
        #define MNS_24  ((word) 24)             /* MnsEHCIGoAttached            */
        #define MNS_26  ((word) 26)             /* MnsHandleVersionInfo         */
        #define MNS_27  ((word) 27)             /* MnsHandleEHCIStateStatus     */
    #endif

    #ifdef NS_MNS_MNS2

        #define MNS_29  ((word) 29)             /* MnsRequestLayer2       */
        #define MNS_30  ((word) 30)             /* MnsRequestLayer2Timer  */

        #define MNS2_OPT_1
        #define MNS2_OPT_2

    #endif

    #ifdef NS_MNS_MHP

        #define MNS_31  ((word) 31)             /* MnsRequestHigh         */
        #define MNS_32  ((word) 32)             /* MnsRequestHighTimerTx  */
        #define MNS_33  ((word) 33)             /* MnsRequestHighTimerRx  */
        #define MNS_71  ((word) 71)             /* MhpTxDelayTimeout      */

        #define MNSH_OPT_1
        #define MNSH_OPT_2

    #endif

    #ifdef NS_MNS_PMHS

        #define MNS_34  ((word) 34)             /* MnsRequestPacket         */
        #define MNS_35  ((word) 35)             /* MnsRequestPacketTimerTx  */
        #define MNS_36  ((word) 36)             /* MnsRequestPacketTimerRx  */

        #define MNSP_OPT_1
        #define MNSP_OPT_3

    #endif

    #define MNS_43      ((word) 43)             /* MnsGetEHCIState */

    #define MNS_45      ((word) 45)             /* MostRegisterTimer    */
    #define MNS_46      ((word) 46)             /* MostUnregisterTimer  */
    #ifndef PACKET_COMMUNICATION_ONLY
        #define MNS_49  ((word) 49)             /* MnsFireWatchdogTrigger */
        #define MNS_104 ((word) 104)            /* MnsWDTxComplete */
    #endif

    #define MNS_INIT_TIMER          ((word) 3000)
    #define MNS_WD_STATUS_TIMEOUT   ((word) 3500)

    /*! Power of two for the watchdog timeout safety margin */
    #ifndef MNS_TM_WATCHDOG_MARGIN
        #define MNS_TM_WDSM_POWER        2
    #elif MNS_TM_WATCHDOG_MARGIN == 1
        #define MNS_TM_WDSM_POWER        1
    #elif MNS_TM_WATCHDOG_MARGIN == 2
        #define MNS_TM_WDSM_POWER        2
    #else
        #define MNS_TM_WDSM_POWER        2
    #endif

    #ifndef PACKET_COMMUNICATION_ONLY
        #define MNS_51  ((word) 51)             /* MnsRetry */
        #define MNS_52  ((word) 52)             /* MnsCheckWatchdogMode */
        #define MNS_53  ((word) 53)             /* MnsHandleWatchdogModeStatus */
    #endif
    #define MNS_54      ((word) 54)             /* MostIsSupported              */
    #define MNS_55      ((word) 55)             /* MnsNtfCheck                  */
    #define MNS_56      ((word) 56)             /* MnsMemCpy                    */
    #define MNS_57      ((word) 57)             /* MostGetVersionInfo           */
    #define MNS_58      ((word) 58)             /* MostGetRevision              */
    #define MNS_59      ((word) 59)             /* MnsMemSet                    */
    #ifndef MNS_AVOID_ATTACH
        #define MNS_60  ((word) 60)             /* MnsRequestEHCIState          */
        #define MNS_61  ((word) 61)             /* MnsForceProtected            */
    #endif
    #define MNS_62      ((word) 62)             /* MostCheckTimers              */
    #define MNS_63      ((word) 63)             /* GetNetServicesConfig         */
    #define MNS_64      ((word) 64)             /* StopNetServices              */
    #ifndef PACKET_COMMUNICATION_ONLY
        #define MNS_65  ((word) 65)             /* MostAllowRemoteAccess        */
        #define MNS_66  ((word) 66)             /* MnsEnforceRemoteAccess       */
        #define MNS_68  ((word) 68)             /* MnsHandleError               */
        #define MNS_69  ((word) 69)             /* MnsPMReady                   */
        #define MNS_70  ((word) 70)             /* MnsResetComplete             */

    #endif

    #ifndef PACKET_COMMUNICATION_ONLY
    #if (defined NS_MNS_MNS2) || (defined NS_MNS_MHP)
        #define MNS_73 ((word) 73)              /* MnsCheckHigherLayers */
    #endif

    #ifdef NS_MNS_MNS2
        #define MNS_74 ((word) 74)              /* MnsStartLayer2 */
        #define MNS_75 ((word) 75)              /* MnsStopLayer2 */
    #endif
    #endif

    #ifdef NS_MNS_MHP
        #define MNS_76 ((word) 76)              /* MnsStartPacketLayers */
        #define MNS_77 ((word) 77)              /* MnsStopPacketLayers */
    #endif

    #define MNS_STD_BURST ((byte) 0x01)

    #define MNS_96      ((word) 96)             /* MnsTMHandleEvent */
    #define MNS_97      ((word) 97)             /* MnsTMUpdate */
    #define MNS_98      ((word) 98)             /* MnsTMDiff */
    #define MNS_100     ((word) 100)            /* MnsTMSchedule */
    #define MNS_102     ((word) 102)            /* MnsSignalNetOn */


    #ifndef PACKET_COMMUNICATION_ONLY
    #ifndef DISABLE_ES_0201XX_025               /* "hidden" switch for disabling workaround */
        #define MNS_105 ((word) 105)            /* MnsDisableSCError */
    #endif
    #endif

    #define MNS_106     ((word) 106)            /* MnsTimerRegistered */
    #define MNS_107     ((word) 107)            /* MnsReRegisterTimer */

    #define MNS_108     ((word) 108)            /* MnsGetWatchdogKickTimeout */

    #define MNS_109     ((word) 109)            /* MostNtfIsRequired */


    #define MNS_112     ((word) 112)            /* MnsGetNetServicesVersion */

    #ifdef MDM_MNS_INIT_TIMEOUT
        #define MNS_113     ((word) 113)        /* MnsSendDbgMnsInitTimeout */
    #endif
    #ifdef MDM_MNS_VERSION_CONFLICT
        #define MNS_114     ((word) 114)        /* MnsSendDbgMnsVersionConflict */
    #endif
    #ifdef PMS_RX_OPT3
        #define MNS_115     ((word) 115)        /* MnsCheckNbminNotification */
    #endif

    #define MNS_116     ((word) 116)            /* MnsPrepareReInit */

    #define MNS_117     ((word) 117)            /* MnsCheckTermination */

#endif /* MNS_MIN */

#endif /* _MNS_AD_H */
