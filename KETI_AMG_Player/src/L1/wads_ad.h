/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Adjustment of the Asynchronous Data Service Wrapper (WADS)
File:           wads_ad.h
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
  * \brief      Adjustment of the Asynchronous Data Service Wrapper (WADS)
  */

#ifndef _ADS_AD_H
#define _ADS_AD_H

#include "rules_ad.h"

#ifdef ADS_MIN


    #define ADS_0       ((word)  0)     /* DataInit                   */
    #define ADS_1       ((word)  1)     /* DataService                */
    #define ADS_2       ((word)  2)     /* DataSetPendingEvent        */
    #define ADS_3       ((word)  3)     /* DataGetNextEventToHandle   */
    #define ADS_4       ((word)  4)     /* DataGetTxPtrExt            */
    #define ADS_5       ((word)  5)     /* DataSend                   */
    #define ADS_6       ((word)  6)     /* DataTxFinal                */
    #define ADS_8       ((word)  8)     /* DataRxTrigger              */
    #define ADS_9       ((word)  9)     /* DataRxOutTrigger           */
  #ifndef PACKET_COMMUNICATION_ONLY
    #define ADS_10      ((word) 10)     /* DataNIStateNetOn           */
  #endif

    #define DATA_PRIORITY_DEFAULT   ((byte)0x01)
    #define DATA_MAX_PRIORITY       ((byte)0x0F)

    #define DATA_STD_RX_BURST       ((byte) 0x01)

#endif /* #ifdef ADS_MIN */

#endif /* _ADS_AD_H */

