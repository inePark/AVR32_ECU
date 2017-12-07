/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Public API of the MOST Processor Control Service 
                Wrapper (WMCS)
File:           wmcs_pb.h
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
  * \brief      Public API of the MOST Processor Control Service 
  *             Wrapper (WMCS)
  */

#ifndef WMCS_PB_H
#define WMCS_PB_H

#include "mostdef1.h"

#define WMCS_P_NONE                     ((word) 0x0000)

#define WMCS_P_GO_PROTECTED             ((word) 0x0001)
#define WMCS_P_GO_SEMI_PROTECTED        ((word) 0x0002)
#define WMCS_P_GROUP_ADDR_RETRY         ((word) 0x0004)
#define WMCS_P_NODE_ADDR_RETRY          ((word) 0x0008)
#define WMCS_P_WD_MODE_RETRY            ((word) 0x0010)
#define WMCS_P_RMCK_RETRY               ((word) 0x0020)
#define WMCS_P_NTF_COMPLETE             ((word) 0x0040)
#define WMCS_P_BUF_FREED                ((word) 0x0080)
#define WMCS_P_CE_TIMER                 ((word) 0x0100)

#define WMCS_P_NODE_ADDR_SETGET_RETRY   ((word) 0x0200)
/*
#define WMCS_P_FREE11                   ((word) 0x0400)
#define WMCS_P_FREE12                   ((word) 0x0800)
#define WMCS_P_FREE13                   ((word) 0x1000)
#define WMCS_P_FREE14                   ((word) 0x2000)
#define WMCS_P_FREE15                   ((word) 0x4000)
#define WMCS_P_FREE16                   ((word) 0x8000)
*/
#define WMCS_P_FIRST                    (word) WMCS_P_GO_PROTECTED
#define WMCS_P_LAST                     (word) WMCS_P_NODE_ADDR_SETGET_RETRY

#define MOST_CHECK_ADDR_NONE            ((byte) 0x10)
#define MOST_CHECK_ADDR_INTERN          ((byte) 0x28)
#define MOST_CHECK_ADDR_NODE            ((byte) 0x20)
#define MOST_CHECK_ADDR_POS             ((byte) 0x21)
#define MOST_CHECK_ADDR_BROADCAST       ((byte) 0x32)
#define MOST_CHECK_ADDR_GROUP           ((byte) 0x33)
#define MOST_CHECK_ADDR_MASK_ROUTING    ((byte) 0xF0)
#define MOST_CHECK_ADDR_EXT_ONLY        ((byte) 0x10)
#define MOST_CHECK_ADDR_INT_ONLY        ((byte) 0x20)
#define MOST_CHECK_ADDR_EXTINT          ((byte) 0x30)
#define MOST_CHECK_ADDR_MASK_RTYP       ((byte) 0x03)

#define RMCK_OFF        ((byte) 0x00)
#define RMCK_64         ((byte) 0x01)
#define RMCK_128        ((byte) 0x02)
#define RMCK_256        ((byte) 0x03)
#define RMCK_384        ((byte) 0x04)
#define RMCK_512        ((byte) 0x05)
#define RMCK_768        ((byte) 0x06)
#define RMCK_1024       ((byte) 0x07)
#define RMCK_1536       ((byte) 0x08)


#define WMCS_CE_TIMEOUT_MIN ((word)25)
#define WMCS_CE_TIMEOUT_MAX ((word)0xFFFF)

#ifdef WMCS_4
    word MostGetNodeAdr(void);
#endif

#ifdef WMCS_5
    byte MostGetNodePos(void);
#endif

#ifdef WMCS_6
    byte MostGetGroupAdr(void);
#endif

#ifdef WMCS_8
    void MostSetGetNodeAdr(word address);
#endif

#ifdef WMCS_9
    void MostSetNodeAdr(word address);
#endif

#ifdef WMCS_10
    void MostSetGroupAdr(byte address);
#endif

#ifdef WMCS_21
    void MostSelectClockOutput(byte rmck_divider);
#endif

#ifdef WMCS_22
    void MostSetAltPAdr(word adr);
#endif

#ifdef WMCS_28
    typedef void TWmcsCodingErrorCB(TMnsResult result, word coding_errors);
    byte MostGetCodingErrors(TWmcsCodingErrorCB *cb_ptr);
#endif

#ifdef WMCS_29
    byte MostCountCodingErrors(word timeout);
#endif

#ifdef MCS_MIN
typedef void TWmcsReadConfStringCB(TMnsResult result,
                                       byte *conf_string,
                                       byte len);
#endif

#ifdef WMCS_33
    byte MostReadConfString(TWmcsReadConfStringCB *cb_ptr,
                            byte *conf_string,
                            byte len);
#endif



#ifdef MCS_MIN
    typedef void TWmcsMEPFilterModeCB(TMnsResult result, word mode);
#endif

#ifdef WMCS_42
    byte MostSetMEPFilterMode(word mode, TWmcsMEPFilterModeCB *cb_ptr);
#endif

#ifdef WMCS_43
    byte MostGetMEPFilterMode(TWmcsMEPFilterModeCB *cb_ptr);
#endif

#ifdef MCS_MIN
    typedef void TWmcsMEPHashTableCB(TMnsResult result, word *hash);
#endif

#ifdef WMCS_46
    byte MostSetMEPHashTable(word *hash, TWmcsMEPHashTableCB *cb_ptr);
#endif

#ifdef WMCS_47
    byte MostGetMEPHashTable(TWmcsMEPHashTableCB *cb_ptr);
#endif

#ifdef MCS_MIN
    typedef void TWmcsEUI48CB(TMnsResult result, byte *eui);
#endif

#ifdef WMCS_50
    byte MostSetEUI48(byte *eui, bool persist, TWmcsEUI48CB *cb_ptr);
#endif

#ifdef MCS_MIN
    typedef void TWmcsPacketRetryTimeCB(TMnsResult result, byte time);
#endif

#ifdef WMCS_53
    byte MostSetPacketRetryTime(byte time, TWmcsPacketRetryTimeCB *cb_ptr);
#endif

#ifdef WMCS_54
    byte MostGetPacketRetryTime(TWmcsPacketRetryTimeCB *cb_ptr);
#endif

#ifdef WMCS_57
void MostGetEUI48(byte *eui48);
#endif

#endif /* WMCS_PB_H */
