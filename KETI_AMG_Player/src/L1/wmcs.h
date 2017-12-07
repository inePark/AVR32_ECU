/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Internal API of the MOST Processor Control Service 
                Wrapper (WMCS)
File:           wmcs.h
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
  * \brief      Internal API of the MOST Processor Control Service 
  *             Wrapper (WMCS)
  */

#ifndef WMCS_H
#define WMCS_H

#include "mostns1.h"

#ifdef WMCS_0
    void WmcsInit(void);
#endif

#ifdef WMCS_1
    void WmcsService(void);
#endif

#ifdef WMCS_2
    void WmcsSetPendingEvent(word event_flag);
#endif

#ifdef WMCS_11
    byte MostCheckOwnAddress(word address);
#endif

#ifdef WMCS_25
    void WmcsRefreshNodePos(byte pos);
#endif

#ifdef WMCS_17
    void WmcsHandleNodeAddrStatus(TMsgRx *msg_ptr);
#endif

#ifdef WMCS_20
    void WmcsHandleGroupAddrStatus(TMsgRx *msg_ptr);
#endif

#ifdef WMCS_27
void WmcsHandleCodingErrorsStatus(TMsgRx *msg_ptr);
#endif

#ifdef WMCS_34
void WmcsHandleMemoryStatus(TMsgRx *msg_ptr);
#endif

#ifdef WMCS_35
void WmcsHandleMemoryError(TMsgRx *msg_ptr);
#endif

#ifdef WMCS_44
void WmcsHandleMEPFilterModeStatus(TMsgRx *msg_ptr);
#endif

#ifdef WMCS_45
void WmcsHandleMEPFilterModeError(TMsgRx *msg_ptr);
#endif

#ifdef WMCS_48
void WmcsHandleMEPHashTableStatus(TMsgRx *msg_ptr);
#endif

#ifdef WMCS_49
void WmcsHandleMEPHashTableError(TMsgRx *msg_ptr);
#endif

#ifdef WMCS_51
void WmcsHandleSetEUI48Result(TMsgRx *msg_ptr);
#endif

#ifdef WMCS_52
void WmcsHandleSetEUI48Error(TMsgRx *msg_ptr);
#endif

#ifdef WMCS_55
void WmcsHandlePacketRetryTimeStatus(TMsgRx *msg_ptr);
#endif

#ifdef WMCS_56
void WmcsHandlePacketRetryTimeError(TMsgRx *msg_ptr);
#endif

#ifdef WMCS_59
void WmcsHandleEUI48Status(TMsgRx *msg_ptr);
#endif

#ifdef WMCS_60
byte WmcsSetINICMemPending(bool on);
#endif

#ifdef WMCS_61
void WmcsHandleConfStringResult(TMsgRx *msg_ptr);
#endif

#ifdef WMCS_62
void WmcsHandleConfStringError(TMsgRx *msg_ptr);
#endif




#endif  /* WMCS_H  */
