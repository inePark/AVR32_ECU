/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Adjustment of the Message Buffer Management (MBM)
File:           mbm_ad.h
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
  * \brief      Adjustment of the Message Buffer Management (MBM)
  */

#ifndef _MBM_AD_H
#define _MBM_AD_H

#include "rules_ad.h"

#ifdef MBM_MIN
    /*
    ----------------------------------------------------------------------------
        API
    ----------------------------------------------------------------------------
    */
    #define MBM_2   ((word) 2)  /* MbmFree             */
    #define MBM_24  ((word)24)  /* MbmGetBufData       */
    #define MBM_25  ((word)25)  /* MbmGetBufLength     */

    /*
    ----------------------------------------------------------------------------
        Library
    ----------------------------------------------------------------------------
    */
    #define MBM_0   ((word) 0)  /* MbmInit              */
    #define MBM_1   ((word) 1)  /* MbmAllocate          */
    #define MBM_3   ((word) 3)  /* MbmQueueInit         */
    #define MBM_4   ((word) 4)  /* MbmEnqueue           */
    #define MBM_5   ((word) 5)  /* _MbmEnqueue          */
    #define MBM_6   ((word) 6)  /* MbmEnqueueFirst      */
    #define MBM_7   ((word) 7)  /* _MbmEnqueueFirst     */
    #define MBM_8   ((word) 8)  /* MbmDequeue           */
    #define MBM_9   ((word) 9)  /* _MbmDequeue          */
    #define MBM_11  ((word)11)  /* MbmDequeueLast       */
    #define MBM_12  ((word)12)  /* _MbmPeek             */
    #define MBM_13  ((word)13)  /* _MbmUnlink           */
    #define MBM_14  ((word)14)  /* MbmReserve           */
    #define MBM_15  ((word)15)  /* MbmPush              */

    #ifndef PACKET_COMMUNICATION_ONLY
        #define MBM_16  ((word)16)  /* MbmChangeType    */
    #endif

    #define MBM_17  ((word)17)  /* MbmPullHeaders       */
    #define MBM_19  ((word)19)  /* MbmGetExtPayloadLen  */
    #define MBM_23  ((word)23)  /* MbmFlush             */
    #define MBM_29  ((word)29)  /* MbmGetBuf            */
    #define MBM_31  ((word)31)  /* MbmEnqueueBehind     */
    #define MBM_33  ((word)33)  /* _MbmEnqueueBehind    */

    #define MBM_30  ((word)30)  /* MbmIsFromPool        */
    /*
    ----------------------------------------------------------------------------
        Module
    ----------------------------------------------------------------------------
    */
    #define MBM_20  ((word)20)  /* _MbmInitPool        */
    #define MBM_21  ((word)21)  /* _MbmQueueInit       */
    #define MBM_26  ((word)26)  /* _MbmMemAlloc        */
    #define MBM_27  ((word)27)  /* MbmMemFree          */
    #define MBM_28  ((word)28)  /* _MbmMemInit         */

    #ifdef MBM_USAGE_API
        #define MBM_37  ((word)37)  /* MbmGetUsage          */
        #define MBM_38  ((word)38)  /* MbmGetUsageTop       */
        #define MBM_39  ((word)39)  /* MbmGetFragmentation  */
        #define MBM_40  ((word)40)  /* _MbmUpdateUsage      */
        #define MBM_41  ((word)41)  /* MbmSetUsageThreshold */
        #define MBM_45  ((word)45)  /* MbmGetUsageTopExt    */
    #endif


    #define MBM_35  ((word)35)  /* MbmGetUsedTxBuffers      */
    #define MBM_36  ((word)36)  /* MbmGetUsedRxBuffers      */

    #define MBM_42  ((word)42)  /* MbmAllocateRxMinPayload  */

    #ifdef MDM_MBM_BUFFER_ALLOCATION_FAILED
        #define MBM_43  ((word)43)  /* _MbmSendDbgMbmBufferAllocationFailed */
    #endif

    #define MBM_44              /* MbmGetHandleByMsgPtr     */
#endif



#endif /* Header guard */
