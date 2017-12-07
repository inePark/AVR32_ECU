/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Adjustment of the MOST Processor Control Service 
                Wrapper (WMCS)
File:           wmcs_ad.h
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
  * \brief      Adjustment of the MOST Processor Control Service 
  *             Wrapper (WMCS)
  */

#ifndef WMCS_AD_H
#define WMCS_AD_H

#include "rules_ad.h"

#ifdef MCS_MIN

    #define WMCS_0      ((word)  0)   /* WmcsInit                   */
    #define WMCS_1      ((word)  1)   /* WmcsService                */
    #define WMCS_2      ((word)  2)   /* WmcsSetPendingEvent        */
    #define WMCS_3      ((word)  3)   /* WmcsGetNextEventToHandle   */
    #define WMCS_4      ((word)  4)   /* MostGetNodeAdr             */
    #define WMCS_5      ((word)  5)   /* MostGetNodePos             */
    #define WMCS_6      ((word)  6)   /* MostGetGroupAdr            */
    #define WMCS_8      ((word)  8)   /* MostSetGetNodeAdr          */
    #define WMCS_9      ((word)  9)   /* MostSetNodeAdr             */
    #define WMCS_10     ((word) 10)   /* MostSetGroupAdr            */
    #define WMCS_11     ((word) 11)   /* MostCheckOwnAddress        */
    #define WMCS_14     ((word) 14)   /* Wmcs_Go_Protected          */
    #define WMCS_15     ((word) 15)   /* Wmcs_Go_SemiProtected      */
    #define WMCS_17     ((word) 17)   /* WmcsHandleNodeAddrStatus   */
    #define WMCS_20     ((word) 20)   /* WmcsHandleGroupAddrStatus  */
    #define WMCS_21     ((word) 21)   /* MostSelectClockOutput      */
    #define WMCS_23     ((word) 23)   /* WmcsRetry                  */
    #define WMCS_25     ((word) 25)   /* WmcsRefreshNodePos         */
    #define WMCS_26     ((word) 26)   /* WmcsRequestCodingErrors    */
    #define WMCS_27     ((word) 27)   /* WmcsHandleCodingErrorsStatus   */
    #define WMCS_28     ((word) 28)   /* MostGetCodingErrors        */
    #define WMCS_29     ((word) 29)   /* MostCountCodingErrors      */
    #define WMCS_30     ((word) 30)   /* WmcsCodingErrorsCallback   */
    #define WMCS_31     ((word) 31)   /* WmcsCodingErrorsTimeout    */
    #define WMCS_33     ((word) 33)   /* MostReadConfString         */
    #define WMCS_34     ((word) 34)   /* WmcsHandleMemoryStatus     */
    #define WMCS_35     ((word) 35)   /* WmcsHandleMemoryError      */
  #if (defined MEP_MIN) && (defined _OS81110_MEP)
    #define WMCS_42     ((word) 42)   /* MostSetMEPFilterMode           */
    #define WMCS_43     ((word) 43)   /* MostGetMEPFilterMode           */
    #define WMCS_44     ((word) 44)   /* WmcsHandleMEPFilterModeStatus  */
    #define WMCS_45     ((word) 45)   /* WmcsHandleMEPFilterModeError   */
    #define WMCS_46     ((word) 46)   /* MostSetMEPHashTable            */
    #define WMCS_47     ((word) 47)   /* MostGetMEPHashTable            */
    #define WMCS_48     ((word) 48)   /* WmcsHandleMEPHashTableStatus   */
    #define WMCS_49     ((word) 49)   /* WmcsHandleMEPHashTableError    */
    #define WMCS_50     ((word) 50)   /* MostSetEUI48                   */
    #define WMCS_51     ((word) 51)   /* WmcsHandleSetEUI48Result       */
    #define WMCS_52     ((word) 52)   /* WmcsHandleSetEUI48Error        */
    #define WMCS_57     ((word) 57)   /* MostGetEUI48                   */
    #define WMCS_58     ((word) 58)   /* WmcsRequestEUI48               */
    #define WMCS_59     ((word) 59)   /* WmcsHandleEUI48Status          */
  #endif

  #ifdef _OS81110_PCK_LLR
    #define WMCS_53     ((word) 53)   /* MostSetPacketRetryTime           */
    #define WMCS_54     ((word) 54)   /* MostGetPacketRetryTime           */
    #define WMCS_55     ((word) 55)   /* WmcsHandlePacketRetryTimeStatus  */
    #define WMCS_56     ((word) 56)   /* WmcsHandlePacketRetryTimeError   */
  #endif

    #define WMCS_60     ((word) 60)   /* WmcsSetINICMemPending      */
    #define WMCS_61     ((word) 61)   /* WmcsHandleConfStringStatus     */
    #define WMCS_62     ((word) 62)   /* WmcsHandleConfStringError      */



  #ifdef DECLARE_LEGACY_SWITCHES
    #define MCS_ADD2
  #endif

#endif /* #ifdef MCS_MIN */


#endif /* WMCS_AD_H */
