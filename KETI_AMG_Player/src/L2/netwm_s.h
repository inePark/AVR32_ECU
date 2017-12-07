/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Header File of NetworkMaster Shadow Module
File:           NetwM_S.h
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


#ifndef _NETWMS_H
#define _NETWMS_H



/*-----------------------------------------------------------------*/
/*  Definitions                                                    */
/*-----------------------------------------------------------------*/





/*-----------------------------------------------------------------*/
/*  Variables needed by MOST NetServices                           */
/*-----------------------------------------------------------------*/






/*-----------------------------------------------------------------*/
/*  Functions available within this module                         */
/*  needed by the Rx command interpreter                           */
/*-----------------------------------------------------------------*/

#ifdef NM_I1C
byte NM_Configuration_Status( pTMsgRx Rx_Ptr);
#endif

#ifdef NM_I1F
byte NM_Configuration_Error( pTMsgRx Rx_Ptr);
#endif

#ifdef NM_I2C
byte NM_CentralReg_Status( pTMsgRx Rx_Ptr);
#endif

#ifdef NM_I2F
byte NM_CentralReg_Error( pTMsgRx Rx_Ptr);
#endif

#ifdef NM_I4C
byte NM_SaveConfig_Result( pTMsgRx Rx_Ptr);
#endif

#ifdef NM_I4CA
byte NM_SaveConfig_ResultAck( pTMsgRx Rx_Ptr);
#endif

#ifdef NM_I4F
byte NM_SaveConfig_Error( pTMsgRx Rx_Ptr);
#endif

#ifdef NM_I4FA
byte NM_SaveConfig_ErrorAck( pTMsgRx Rx_Ptr);
#endif

#ifdef NM_I5C
byte NM_FktIDs_Status(pTMsgRx Rx_Ptr);
#endif

#ifdef NM_I5F
byte NM_FktIDs_Error(pTMsgRx Rx_Ptr);
#endif

#ifdef NM_I6C
byte NM_SystemAvail_Status(pTMsgRx Rx_Ptr);
#endif

#ifdef NM_I6F
byte NM_SystemAvail_Error(pTMsgRx Rx_Ptr);
#endif

#ifdef NM_I7C
byte NM_FBlockInfo_Status(pTMsgRx Rx_Ptr);
#endif

#ifdef NM_I7F
byte NM_FBlockInfo_Error(pTMsgRx Rx_Ptr);
#endif

#ifdef NM_I8C
byte NM_Version_Status(pTMsgRx Rx_Ptr);
#endif

#ifdef NM_I8F
byte NM_Version_Error(pTMsgRx Rx_Ptr);
#endif

#ifdef NM_I9CA
byte NM_OwnConfigInvalid_ResultAck(pTMsgRx Rx_Ptr);
#endif

#ifdef NM_I9FA
byte NM_OwnConfigInvalid_ErrorAck(pTMsgRx Rx_Ptr);
#endif

/*-----------------------------------------------------------------*/
/*  Functions available within this module                         */
/*  needed by the MOST NetServices Layer II                        */
/*-----------------------------------------------------------------*/


#ifdef NM_1
word NmGetNWMAddr(void);
#endif


#endif /* NETWMS_H */

/* end of NetwM_S.h */
