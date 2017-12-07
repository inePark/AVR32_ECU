/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Public API of the Message Interface Service (MIS)
File:           mis_pb.h
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
  * \brief      Public API of the Message Interface Service (MIS)
  */

#ifndef _MIS_PB_H
#define _MIS_PB_H

#include "mostdef1.h"   /* Definition File for MOST NetServices Basic Layer */

/*
==============================================================================
    service events
==============================================================================
*/

#ifdef MIS_1

    #define MIS_P_NONE                  ((word) 0x0000)
    #define MIS_P_SYNC                  ((word) 0x0001)
    #define MIS_P_SYNC_RESET            ((word) 0x0002)
    #define MIS_P_SYNC_FAILED           ((word) 0x0004)
    #define MIS_P_BUF_FREED             ((word) 0x0008)
    #define MIS_P_RESET_TIMER           ((word) 0x0010)
    #define MIS_P_SYNC_NOT_SUPPORTED    ((word) 0x0020)
    /*
    #define MIS_P_FREE07                ((word) 0x0040)
    #define MIS_P_FREE08                ((word) 0x0080)
    #define MIS_P_FREE09                ((word) 0x0100)
    #define MIS_P_FREE10                ((word) 0x0200)
    #define MIS_P_FREE11                ((word) 0x0400)
    #define MIS_P_FREE12                ((word) 0x0800)
    #define MIS_P_FREE13                ((word) 0x1000)
    #define MIS_P_FREE14                ((word) 0x2000)
    #define MIS_P_FREE15                ((word) 0x4000)
    #define MIS_P_FREE16                ((word) 0x8000)
    */
    #define MIS_P_FIRST                 MIS_P_SYNC
    #define MIS_P_LAST                  MIS_P_SYNC_NOT_SUPPORTED

#endif

#endif /* _MIS_PB_H */
