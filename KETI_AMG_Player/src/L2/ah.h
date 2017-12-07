/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Header File of Address Search Handler
File:           ah.h
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


#ifndef _AH_H
#define _AH_H



/*-----------------------------------------------------------------*/
/*  Definitions                                                    */
/*-----------------------------------------------------------------*/





/*-----------------------------------------------------------------*/
/*  Variables needed by MOST NetServices Layer II                  */
/*-----------------------------------------------------------------*/

/*#if (ADDRH_SIZE_DEVICE_TAB > 0) */
/*extern TDevTab DevTab[];                                              */ /* decentral device registry */
/*#endif                                                                */ /* (needed by nb.c) */





/*-----------------------------------------------------------------*/
/*  Functions available within this module                         */
/*  needed by the MOST NetServices Layer II                        */
/*-----------------------------------------------------------------*/

#ifdef AH_3
void AddrHService(void);                                                /* is called periodically by NetService Kernel */
#endif

#ifdef AH_5
void AddrHCheckFBlockIDs(pTMsgRx Rx_Ptr);                   /* is called by NB_FBlockIDs_Status() to  */
#endif                                                                  /* read the status msg */


#ifdef AH_7
void AddrHCheckCentralRegStatus(pTMsgRx Rx_Ptr);            /* is called by NM_CentralReg_Status() to  */
#endif                                                                  /* read the status msg */

#ifdef AH_10
void AddrHCheckCentralRegError(pTMsgRx Rx_Ptr);             /* is called by NM_CentralReg_Error() to  */
#endif                                                                  /* read the error msg */

#ifdef AH_11
void AddrHStoreDevTabAH(void);                                          /* called by module NB.C */
#endif

#ifdef AH_12
void AddrHDevTabDel(pTMsgRx Rx_Ptr);                        /* called by ConfigStatusChanged() */
#endif

#ifdef AH_14
void AddrHClearTasks(void);                                             /* clear pending tasks of Address Handler */
#endif

#endif /* _AH_H */

/* end of ah.h */
