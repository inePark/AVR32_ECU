/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Adjustment of the Port Message Service (PMS)
File:           pms_ad.h
Version:        3.0.x-SR-1  
Language:       C
Author(s):      R.Lundstrom, T.Jahnke
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
  * \brief      Adjustment of the Port Message Service (PMS)
  */

#ifndef _PMS_AD_H
#define _PMS_AD_H

#include "rules_ad.h"

/*
 * Define PMS_NO_SLOTNTF to use the old communication method, without
 * double-buffering and notification of available slots.
 */
#ifdef PMS_DBL_BUFFER_ENABLE
    #ifdef  PMS_NO_SLOTNTF
        #error Definition of PMS_NO_SLOTNTF not allowed
    #endif
    #define PMS_MCM_SLOTS   ((byte)2)
#else
    #define PMS_NO_SLOTNTF
    #define PMS_MCM_SLOTS   ((byte)1)
#endif

/*
 * Define PMS_APPENDABLE_PAYLOAD to be able to append SegCnt or
 * append.payload in front of the payload.
 */
#if ((defined AAM_MIN) && (defined AAM_NO_COPY_ENABLE)) || (defined MOST_HIGH_PROTOCOL)
    #define PMS_APPENDABLE_PAYLOAD
    #if (defined AAM_MIN) && (defined AAM_NO_COPY_ENABLE)
        #define PMS_APPENDABLE_SEG_CNT
    #endif
#endif


#ifdef PMS_MIN

        #define PMS_0           ((word)  0) /* PmsRx                          */
        #define PMS_1           ((word)  1) /* PmsTxRelease                   */
        #define PMS_31          ((word) 31) /* PmsGetRxBuf                    */

        #define PMS_4           ((word)  4) /* PmsInit                        */
        #define PMS_5           ((word)  5) /* PmsService                     */
        #define PMS_6           ((word)  6) /* PmsGetBuf                      */
        #define PMS_7           ((word)  7) /* PmsSend                        */
        #define PMS_8           ((word)  8) /* MbmFree                        */
        #define PMS_10          ((word) 10) /* PmsSetPendingEvent             */

        #define PMS_9           ((word)  9) /* PmsFifoInit                    */
        #define PMS_11          ((word) 11) /* PmsFillDefaultHeader           */
        #define PMS_12          ((word) 12) /* PmsCompressHeader              */
        #define PMS_64          ((word) 64) /* PmsCompressHeaderICM           */
        #define PMS_65          ((word) 65) /* _PmsCompressHeaderMCM          */
      #ifdef ADS_MIN
        #define PMS_66          ((word) 66) /* PmsCompressHeaderMDP           */
      #endif
        #define PMS_13          ((word) 13) /* PmsDecompressHeader            */
        #define PMS_17          ((word) 17) /* PmsFifoTxTrigger               */
        #define PMS_18          ((word) 18) /* PmsProcessRxMsgQueue           */
        #define PMS_19          ((word) 19) /* PmsFifoProcessStatus           */
        #define PMS_37          ((word) 37) /* PmsRespondCommand              */

    #ifdef NS_PMS_PMHT
        #define PMS_45          ((word) 45) /* PmsTxStarted                   */
    #endif

        #define PMS_46          ((word) 46) /* PmsPrepareReInit               */

        #define PMS_62          ((word) 62) /* PmsInitExternalBuf             */
        #define PMS_63          ((word) 63) /* PmsFireBufFreed                */

    #if (defined MEP_MIN) && (defined _OS81110_MEP)
        #define PMS_48          ((word) 48) /* PmsPrepareMepHeaderExt         */
    #endif

    #ifndef PACKET_COMMUNICATION_ONLY
      #ifdef PMS_RX_OPT3
        #define PMS_26          ((word) 26) /* PmsGetRxBufMcm                 */
        #define PMS_27          ((word) 27) /* PmsGetRxBufMdp                 */
      #else
        #define PMS_RX_SEND_ACK             /* Enables fifo_ptr->rxStatus field  */
        #define PMS_32          ((word) 32) /* PmsRxHandleAck                 */
      #endif

        #define PMS_2           ((word)  2) /* PmsSetFifoInterface            */
        #define PMS_3           ((word)  3) /* PmsGetFifoInterface            */
        #define PMS_15          ((word) 15) /* PmsGetTxFifo                   */
        #define PMS_53          ((word) 53) /* PmsGetRxFifo                   */
        #define PMS_20          ((word) 20) /* PmsSendCommand                 */
        #define PMS_21          ((word) 21) /* PmsSyncIn                      */
        #define PMS_23          ((word) 23) /* PmsSyncHandler                 */
        #define PMS_30          ((word) 30) /* PmsProcessRxStatusQueue        */

        #define PMS_39          ((word) 39) /* PmsProcessCompressedRxMsg      */
        #define PMS_40          ((word) 40) /* PmsProcessRxMsgOut             */
        #define PMS_49          ((word) 49) /* PmsTxReady                     */

      #ifdef NS_AMS_MSV2
        #define PMS_44          ((word) 44) /* PmsFlushMsgTxBuffer            */
      #endif

        #define PMS_22          ((word) 22) /* PmsSync                        */

      #ifdef MSG_RX_USER_PAYLOAD
        #define PMS_41          ((word) 41) /* PmsSetExternalBufferAvailable  */
        #define PMS_42          ((word) 42) /* PmsReleasePayload              */
        #define PMS_43          ((word) 43) /* PmsInjectWaitingRx             */
      #endif

      #if defined(MSG_TX_USER_PAYLOAD) || defined(DATA_TX_USER_PAYLOAD)
        #define PMS_47          ((word) 47) /* PmsFlushTxQueue                */
      #endif

        #define PMS_54          ((word) 54) /* PmsSendBypass                  */

      #ifndef PMS_TX_NOSEG
        #define PMS_16          ((word) 16) /* _PmsTxNextSegment              */
        #define PMS_36          ((word) 36) /* PmsSetReferencesToNull         */
        #define PMS_38          ((word) 38) /* PmsFreeShadow                  */
        #define PMS_61          ((word) 61) /* PmsTxSyncSegmented             */
      #endif

      #ifndef PMS_RX_NOSEG
        #define PMS_24          ((word) 24) /* PmsRxSegProcess                */
        #define PMS_25          ((word) 25) /* PmsRxSegFindMatch              */
        #define PMS_34          ((word) 34) /* PmsRxSegAppend                 */

      #ifdef PMS_USE_GARBAGE_COLLECTOR
        #define PMS_28          ((word) 28) /* PmsRxSegGarbageCollector       */
      #endif

        #define PMS_29          ((word) 29) /* PmsRxSegError                  */
        #define PMS_33          ((word) 33) /* PmsRxSegMatch                  */
    #endif

        #define PMS_35          ((word) 35) /* PmsSegErrorTxStatus            */
        #define PMS_50          ((word) 50) /* PmsDiscardRx                   */
        #define PMS_55          ((word) 55) /* PmsExtendSyncTimeout           */
        #define PMS_56          ((word) 56) /* PmsHandleRetryParamsStatus     */
        #define PMS_57          ((word) 57) /* PmsHandleMidLevelRetriesStatus */
        #define PMS_58          ((word) 58) /* PmsInsertRetryValues           */

      #if defined(MSG_TX_USER_PAYLOAD) || defined(DATA_TX_USER_PAYLOAD)
        #define PMS_51          ((word) 51) /* PmsFreeUserTxPayload           */
      #endif

      #if defined(MSG_RX_USER_PAYLOAD) || !defined(PMS_RX_NOSEG)
        #define PMS_52          ((word) 52) /* PmsDiscardPendingRx            */
      #endif
    #endif    /* PACKET_COMMUNICATION_ONLY */
#endif      /* PMS_MIN */

/*
================================================================================
    Configuration
================================================================================
*/

#ifdef TIME_MSG_RX
    #define PMS_RX_SEG_TIMEOUT  TIME_MSG_RX
#endif

#ifndef PMS_RX_SEG_TIMEOUT
    /*!
     * Timeout between subsequent RX segments before the message is discarded
     * by the garbage collector.
     */
    #define PMS_RX_SEG_TIMEOUT  ((word)5000)
#endif

/*!
 * Maximum number of messages forwarded on each run of the service, improves
 * performance, but a too high number might steal CPU time from the services
 * needing to process the messages (if they are in the same thread!)
 */
#define PMS_STD_RX_BURST        ((byte) 0x04)

/* Maximum consecutive attempts to synchronize before error state */
#define PMS_MAX_SYNC_ATTEMPTS   ((word)1)

/*#define PMS_TX_FORCE_TRIG*/
#define PMS_RX_TRIGGERS_TX

#define PMS_NUM_ACTIVE_SEGMENTS     1 /* Maximum number of segments enqueued in the INIC simultanously */

/* retry values for segmentation errors */
#define PMS_SEG_ERROR_LLR   ((byte)1)   /* number of Low Level Retries (total attempts) */
#define PMS_SEG_ERROR_MLR   ((byte)0)   /* number of Mid Level Retries */

/*
--------------------------------------------------------------------------------
    Available RX slots.
    Simply do NOT modify!
--------------------------------------------------------------------------------
*/
#define PMS_RX_SLOTS_ICM    1
#define PMS_RX_SLOTS_MCM    1

#ifdef ADS_MIN /* MDP_MIN does not need any slot since it'll discard buffer */
    #ifndef PMS_RX_OPT3
        #define PMS_RX_SLOTS_MDP    1
    #else
        #define PMS_RX_SLOTS_MDP    0
    #endif
#else
    #define PMS_RX_SLOTS_MDP    0
#endif

/*!
 * Preallocated messages need to be one more than what's needed for the FIFOs
 * to allow status messages to be received when there is no "slot" available.
 */
#define PMS_RX_PREALLOC     (PMS_RX_SLOTS_ICM + PMS_RX_SLOTS_MCM + PMS_RX_SLOTS_MDP + 1)

/*
--------------------------------------------------------------------------------
    Timeouts
--------------------------------------------------------------------------------
*/
#define PMS_T_ICM       ((word)500)  /* Time to wait for status on ICM messages (ms) */
#define PMS_T_MCM       ((word)2000) /* Time to wait for status on MCM messages (ms) */
#define PMS_T_MDP       ((word)2000) /* Time to wait for status on MDP messages over I2C (ms) */
#define PMS_T_SYNC      ((word)300)  /* Time to wait for SyncS on a SyncC */
#define PMS_T_SYNC_EXT  ((word)3000) /* Extended time to wait for SyncS on a SyncC */

#define PMS_T_PREALLOC_RETRY    ((word)20)  /* Retry time if an RX preallocation failed and
                                             * already has timed out at least once.
                                             */

#if (defined PMS_MCM_MLR_THRESHOLD) && (defined PMS_MCM_MAX_TIMEOUT)
    #define PMS_CFG_MCM_MLR_THRESHOLD   ((byte)PMS_MCM_MLR_THRESHOLD)
    #define PMS_CFG_MCM_MAX_TIMEOUT     ((word)PMS_MCM_MAX_TIMEOUT)
#elif (!defined PMS_MCM_MLR_THRESHOLD) && (!defined PMS_MCM_MAX_TIMEOUT)
    #define PMS_CFG_MCM_MLR_THRESHOLD   ((byte)40)
    #define PMS_CFG_MCM_MAX_TIMEOUT     ((word)3000)
#elif (defined PMS_MCM_MLR_THRESHOLD) || (defined PMS_MCM_MAX_TIMEOUT)
    #error "It is not allowed to define PMS_MCM_MLR_THRESHOLD without PMS_MCM_MAX_TIMEOUT or vice versa"
#endif


#endif /* _PMS_AD_H */
