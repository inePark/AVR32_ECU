/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Header File of MOST Supervisor Layer II
File:           ah.c
Version:        3.0.x-SR-1  
Language:       C
Author(s):      S.Kerber
Date:           05.January.2011

FileGroup:      Layer II
Customer ID:    0018FF2A0300xx.N.KETI
FeatureCode:    FCR1
------------------------------------------------------------------------------

                (c) Copyright 1998-2011
                SMSC
                All Rights Reserved

------------------------------------------------------------------------------



Modifications
~~~~~~~~~~~~~
Date    By      Description

==============================================================================
*/

#ifndef _MSV2_H
#define _MSV2_H




/*-----------------------------------------------------------------*/
/* Preprocessor hints                                              */
/*-----------------------------------------------------------------*/

#ifdef MSV2_MIN

  #ifndef MCS_ADD2
    #error "MCS_ADD2 must be defined, if MSV2_MIN is used!"
  #endif

#endif



/*-----------------------------------------------------------------*/
/*  Definitions                                                    */
/*-----------------------------------------------------------------*/





/*-----------------------------------------------------------------*/
/*  Variables available within this module                         */
/*  needed by the MOST NetServices Layer II                        */
/*-----------------------------------------------------------------*/

#if defined(MSV2_0) && defined(NETWORKMASTER_LOCAL)
extern byte ConfigState;
#endif




/*-----------------------------------------------------------------*/
/*  Functions available within this module                         */
/*  needed by the MOST NetServices Layer II                        */
/*-----------------------------------------------------------------*/

#ifdef MSV2_3
void ConfigStatusChanged(pTMsgRx Rx_Ptr);
#endif

#ifdef MSV2_4
void ConfigCheckStateMaster(void);
#endif

#ifdef MSV2_8
bool CheckConfigState(void);
#endif

#ifdef MSV2_10
void NodeAddrChangeComplete(void);
#endif

#ifdef MSV2_11
void SetConfigStateFilterStatus(bool on);
#endif


#endif /* _MSV2_H */


/* end of msv2.h */




