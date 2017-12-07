/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Header File of FBlock NetBlock (EHC part)
File:           nbehc.h
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

#ifndef _NB_H
#define _NB_H




/*-----------------------------------------------------------------*/
/* Preprocessor hints                                              */
/*-----------------------------------------------------------------*/

#ifdef NB_MIN

  #ifndef MCS_ADD2
    #error "MCS_ADD2 must be defined, if NB_MIN is used!"
  #endif

#endif



/*-----------------------------------------------------------------*/
/*  Definitions                                                    */
/*-----------------------------------------------------------------*/




/*-----------------------------------------------------------------*/
/*  Type Declaration of Module NetBlock (NB)                       */
/*-----------------------------------------------------------------*/

#ifdef NB_MIN

#if (NUM_FBLOCKS > 0)
typedef struct
{
    byte FBlockID[NUM_FBLOCKS];             /* Array of available FBlockIDs (except NetBlock) */
    byte InstID[NUM_FBLOCKS];               /* Array of corresponding InstIDs */
}TFBlockIDs;
#endif

typedef struct
{
    bool Suspend;                           /* Result will be reported only if Suspend == MNS_TRUE */
}TShutDown;


typedef struct                              /* Declaration of FBlock NetBlock */
{
    #if (NUM_FBLOCKS>0)
    TFBlockIDs      pFBlockIDs;
    #endif
/*  TDeviceInfo     pDeviceInfo;            */ /* --> Application specific (handled via callback function) */
    TShutDown       mShutDown;
}TNetBlock;

#endif






/*-----------------------------------------------------------------*/
/*  Variables needed by MOST NetServices                           */
/*-----------------------------------------------------------------*/

#ifdef NB_MIN
extern TNetBlock NetBlock;                                              /* Struct NetBlock */
#endif




/*-----------------------------------------------------------------*/
/*  Functions available within this module                         */
/*  called by other Layer II modules                               */
/*-----------------------------------------------------------------*/
#ifdef NB_12
void NbService(void);
#endif



/*-----------------------------------------------------------------*/
/*  Functions available within this module                         */
/*  needed by the Rx command interpreter                           */
/*-----------------------------------------------------------------*/

#ifdef NB_10
byte NbCheckPosValid(pTMsgRx rx_ptr);
#endif

#ifdef NB_I10
byte NB_NodeAddr_Set(pTMsgTx Tx_Ptr, pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I1C
byte NB_NodeAddr_Status(pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I1F
byte NB_NodeAddr_Error(pTMsgRx Rx_Ptr);
#endif


#ifdef NB_I20
byte NB_FBlockIDs_Set(pTMsgTx Tx_Ptr, pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I21
byte NB_FBlockIDs_Get(pTMsgTx Tx_Ptr, pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I2C
byte NB_FBlockIDs_Status(pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I2F
byte NB_FBlockIDs_Error(pTMsgRx Rx_Ptr);
#endif


#ifdef NB_I31
byte NB_DeviceInfo_Get(pTMsgTx Tx_Ptr, pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I3C
byte NB_DeviceInfo_Status(pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I3F
byte NB_DeviceInfo_Error(pTMsgRx Rx_Ptr);
#endif


#ifdef NB_I40
byte NB_GroupAddr_Set(pTMsgTx Tx_Ptr, pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I4C
byte NB_GroupAddr_Status(pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I4F
byte NB_GroupAddr_Error(pTMsgRx Rx_Ptr);
#endif




#ifdef NB_I70
byte NB_ShutDown_Start(pTMsgTx Tx_Ptr, pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I7C
byte NB_ShutDown_Result(pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I7F
byte NB_ShutDown_Error(pTMsgRx Rx_Ptr);
#endif


#ifdef NB_I9C
byte NB_NodePosition_Status(pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I9F
byte NB_NodePosition_Error(pTMsgRx Rx_Ptr);
#endif


#ifdef NB_I100
byte NB_RetryParameters_Set(pTMsgTx Tx_Ptr, pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I101
byte NB_RetryParameters_Get(pTMsgTx Tx_Ptr);
#endif

#ifdef NB_I10C
byte NB_RetryParameters_Status(pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I10F
byte NB_RetryParameters_Error(pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I11C
byte NB_SamplingFrequency_Status(pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I11F
byte NB_SamplingFrequency_Error(pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I142
byte NB_Boundary_SetGet(pTMsgTx Tx_Ptr, pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I14C
byte NB_Boundary_Status(pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I14F
byte NB_Boundary_Error(pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I151
byte NB_Version_Get(pTMsgTx Tx_Ptr);
#endif

#ifdef NB_I15C
byte NB_Version_Status(pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I15F
byte NB_Version_Error(pTMsgRx Rx_Ptr);
#endif


#ifdef NB_I160
byte NB_ShutDownReason_Set(pTMsgTx Tx_Ptr, pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I161
byte NB_ShutDownReason_Get(pTMsgTx Tx_Ptr);
#endif

#ifdef NB_I16C
byte NB_ShutDownReason_Status(pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I16F
byte NB_ShutDownReason_Error(pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I171
byte NB_FBlockInfo_Get(pTMsgTx Tx_Ptr, pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I17C
byte NB_FBlockInfo_Status(pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I17F
byte NB_FBlockInfo_Error(pTMsgRx Rx_Ptr);
#endif


#ifdef NB_I181
byte NB_ImplFBlocks_Get(pTMsgTx Tx_Ptr);
#endif

#ifdef NB_I18C
byte NB_ImplFBlocks_Status(pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I18F
byte NB_ImplFBlocks_Error(pTMsgRx Rx_Ptr);
#endif


#ifdef NB_I19C
byte NB_EUI48_Status(pTMsgRx Rx_Ptr);
#endif

#ifdef NB_I19F
byte NB_EUI48_Error(pTMsgRx Rx_Ptr);
#endif

#endif /* _NB_H */


/* end of nb.h */
