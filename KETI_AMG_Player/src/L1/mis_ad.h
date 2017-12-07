/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Adjustment of the Message Interface Service (MIS)
File:           mis_ad.h
Version:        3.0.x-SR-1  
Language:       C
Author(s):      S.Semmler, S.Kerber
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
  * \brief      Adjustment of the Message Interface Service (MIS)
  */

#ifndef _MIS_AD_H
#define _MIS_AD_H

#include "rules_ad.h"

#ifdef MIS_MIN

    #define MIS_0   ((word) 0)     /* MisInit                  */
    #define MIS_1   ((word) 1)     /* MisService               */
    #define MIS_2   ((word) 2)     /* MisGetNextEventToHandle  */
    #define MIS_3   ((word) 3)     /* MisSetPendingEvent       */
    #define MIS_4   ((word) 4)     /* MisHandleBufFreed        */
    #define MIS_5   ((word) 5)     /* MisHandlePmsEvent        */
    #ifndef PACKET_COMMUNICATION_ONLY
        #define MIS_6   ((word)  6)     /* MisHandleInicMsg         */
        #define MIS_7   ((word)  7)     /* MisHandleMostMsg         */
        #define MIS_12  ((word) 12)     /* MisFilterMostMsg         */
    #endif
    #define MIS_8   ((word)  8)     /* MisHandlePacketData      */
    #define MIS_9   ((word)  9)     /* MisResetInic */
    #define MIS_10  ((word) 10)     /* MisGetResetCount */
    #define MIS_11  ((word) 11)     /* MisSetResetCount */

    #define MIS_RESET_TRIES   1
    #define MIS_RESET_TIMEOUT ((word) 1000)

#endif /* #ifdef MIS_MIN */

#endif /* _MIS_AD_H */
