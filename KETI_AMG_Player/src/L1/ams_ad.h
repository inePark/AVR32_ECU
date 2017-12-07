/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Adjustment of the Application Message Service Wrapper (WAMS)
File:           ams_ad.h
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
  * \brief      Adjustment of the Application Message Service Wrapper (WAMS)
  */

#ifndef _AMS_AD_H
#define _AMS_AD_H

#include "rules_ad.h"

#ifdef AMS_MIN

    #define AMS_0   ((word)  0)   /* MsgInit                    */
    #define AMS_1   ((word)  1)   /* MsgService                 */
    #define AMS_2   ((word)  2)   /* MsgSetPendingEvent         */
    #define AMS_3   ((word)  3)   /* MsgGetNextEventToHandle    */
    #define AMS_4   ((word)  4)   /* MsgGetTxPtrExt             */
    #define AMS_5   ((word)  5)   /* MsgSend                    */
    #define AMS_6   ((word)  6)   /* MsgSend2                   */
    #define AMS_7   ((word)  7)   /* MsgSend3                   */
    #define AMS_8   ((word)  8)   /* MsgTxFinal                 */
    #define AMS_9   ((word)  9)   /* MsgTxUnused                */
    #define AMS_10  ((word) 10)   /* MsgRxTrigger               */
    #define AMS_11  ((word) 11)   /* MsgRxOutTrigger            */

    #ifdef MSG_RX_SEG_EHC_ONLY
    #define AMS_12  ((word) 12)   /* MsgRxError                 */
    #endif

    #define AMS_13  ((word) 13)   /* MsgFreeRxMsg               */
    #define AMS_14  ((word) 14)   /* MsgGetRxInPtrExt           */
    #define AMS_15  ((word) 15)   /* MsgRxInReady               */

    #ifdef DECLARE_LEGACY_SWITCHES

        #define AMS_RX_ADD5
        #define AMS_TX_ADD6
        #define AMS_TX_ADD8
        #define AMS_TX_ADD9

    #endif


    #ifdef TIME_MSG_TX_RETRY
        #if (TIME_MSG_TX_RETRY > 30) || (TIME_MSG_TX_RETRY == 0)
            #error TIME_MSG_TX_RETRY out of range 1-30
        #endif
    #else
        #define TIME_MSG_TX_RETRY 20
    #endif

    #ifdef DEF_MID_LEVEL_RETRIES
        #if (DEF_MID_LEVEL_RETRIES > 5)
            #undef DEF_MID_LEVEL_RETRIES
            #define DEF_MID_LEVEL_RETRIES 5
        #endif
    #else
        #define DEF_MID_LEVEL_RETRIES 0
    #endif

    #ifdef DEF_MID_LEVEL_RETRIES_INT_PROC
        #if (DEF_MID_LEVEL_RETRIES_INT_PROC > 5)
            #undef DEF_MID_LEVEL_RETRIES_INT_PROC
            #define DEF_MID_LEVEL_RETRIES_INT_PROC 5
        #endif
    #else
        #define DEF_MID_LEVEL_RETRIES_INT_PROC 0
    #endif

    #ifndef DEF_MID_LEVEL_RETRIES_FBLOCKIDS
        #define DEF_MID_LEVEL_RETRIES_FBLOCKIDS 63
    #endif

    #ifdef AMS_ADD1

        #define AMS_16  ((word) 16)   /* MsgTxDataByte              */
        #define AMS_17  ((word) 17)   /* MsgTxDataWord              */
        #define AMS_18  ((word) 18)   /* MsgTxDataLong              */
        #define AMS_19  ((word) 19)   /* MsgTxBcdToAscII            */
        #define AMS_20  ((word) 20)   /* MsgRxDataByte              */
        #define AMS_21  ((word) 21)   /* MsgRxDataWord              */
        #define AMS_22  ((word) 22)   /* MsgRxDataLong              */

    #endif

    #ifdef NS_AMS_AH

        #define AMS_24  ((word) 24)   /* AddrHSearchStart           */

    #endif


    #define AMS_26  ((word) 26)       /* MsgEHCIGoProtected              */
    #define AMS_27  ((word) 27)       /* MsgEHCIGoSemiProtected          */
    #define AMS_28  ((word) 28)       /* MsgRetry                        */
    #define AMS_29  ((word) 29)       /* MsgHandleAbilityToSegmentStatus */
    #define AMS_30  ((word) 30)       /* MsgHandleAbilityToSegmentError  */
    #define AMS_31  ((word) 31)       /* MsgGetSegMode                   */
    #define AMS_33  ((word) 33)       /* MsgForceSegMode                 */
    #define AMS_34  ((word) 34)       /* MsgRequestSegMode               */
    #define AMS_35  ((word) 35)       /* MsgForceMidLevelRetries         */
    #define AMS_37  ((word) 37)       /* MsgHandleMidLevelRetriesStatus  */
    #define AMS_38  ((word) 38)       /* MsgNIStateNetOn                 */

    #if defined(MSG_RX_USER_PAYLOAD) || defined(MSG_TX_USER_PAYLOAD)
        #define AMS_39  ((word) 39)   /* MsgPrepareReInit                */
    #endif

    #ifdef MSG_RX_USER_PAYLOAD
        #define AMS_52  ((word) 52)   /* MsgReleasePayload               */
    #endif

    #define AMS_40  ((word) 40)       /* MsgGetUsedTxBuffers             */
    #define AMS_41  ((word) 41)       /* MsgGetUsedRxBuffers             */

    #define AMS_42  ((word) 42)       /* MsgDiscardRx                    */

    #define AMS_43  ((word) 43)       /* MsgTxBcdToStr                   */
    #define AMS_44  ((word) 44)       /* MsgVersionToISO8859             */
    #define AMS_45  ((word) 45)       /* MsgRequestRetryParams           */
    #define AMS_46  ((word) 46)       /* MsgSendRetryParams              */
    #define AMS_47  ((word) 47)       /* MsgHandleRetryOptionsStatus     */
    #define AMS_48  ((word) 48)       /* MsgHandleRetryParamsStatus      */
    #define AMS_49  ((word) 49)       /* MsgHandleRetryError             */
    #define AMS_50  ((word) 50)       /* MsgGetRetryConfig               */
    #define AMS_51  ((word) 51)       /* MsgSetRetryConfig               */

    #ifdef VMSV_MIN
        #define AMS_53  ((word) 53)   /* MsgNIStateNetInit               */
    #endif

    #define MSG_STD_RX_BURST ((byte) 0x01)

#endif /* #ifdef AMS_MIN */

#endif /* _AMS_AD_H */

