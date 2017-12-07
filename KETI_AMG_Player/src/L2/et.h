/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         FBlock ET 'Enhanced Testability' (FBlockID: 0x0F)
File:           et.h
Version:        3.0.x-SR-1  
Language:       C
Author(s):      S.Semmler
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
#ifndef _ET_H
#define _ET_H

/*
+------------------------------------------------------------------------------+
| Preprocessor hints                                                           |
+------------------------------------------------------------------------------+
*/

#ifdef ET_ADD6
    #ifndef NETWORKMASTER_LOCAL
        #error NETWORKMASTER_LOCAL must be defined if ET_ADD6 is used!
    #endif
#endif


/*
+------------------------------------------------------------------------------+
| Declaration of FBlock ETs FunctionIDs                                        |
+------------------------------------------------------------------------------+
*/
#ifdef ET_MIN
    #define FUNCID_ET_VERSION                                       ((word) 0x010)
    #define FUNCID_ET_FBLOCKINFO                                    ((word) 0x011)
    #define FUNCID_ET_AUTOWAKEUP                                    ((word) 0x201)
    #define FUNCID_ET_DIAGRESULT                                    ((word) 0x203)
    #define FUNCID_ET_SHUTDOWN                                      ((word) 0x204)
    #define FUNCID_ET_SHUTDOWNSUSPENDMODE                           ((word) 0x205)
    #define FUNCID_ET_SENDMESSAGE                                   ((word) 0x207)
    #define FUNCID_ET_ECHOMESSAGE                                   ((word) 0x208)
    #define FUNCID_ET_MESSAGEBUFSIZE                                ((word) 0x209)
    #define FUNCID_ET_RESET                                         ((word) 0x211)
    #define FUNCID_ET_NOTIFICATIONMATRIXSIZE                        ((word) 0x213)
    #define FUNCID_ET_CODINGERRORS                                  ((word) 0x20F)
    #define FUNCID_ET_SYSTEMSTATE                                   ((word) 0x216)
    #define FUNCID_ET_MOSTREMOTERESET                               ((word) 0x217)
    #define FUNCID_ET_PHYSICALLAYERTEST                             ((word) 0x218)
    #define FUNCID_ET_PHYSICALLAYERRESULT                           ((word) 0x219)
    #define FUNCID_ET_VERSION2                                      ((word) 0xF00)
#endif


#if (defined MHP_TX_MIN) || (defined PMHT_MIN) || (defined MHP_RX_MIN) || (defined PACKET_ON_SECOND_HOST)
  #define FUNCID_ET_DSIDSOCOUNT                                   ((word) 0x3FD)
#endif

#if (defined MHP_TX_MIN) || (defined PMHT_MIN) || (defined PACKET_ON_SECOND_HOST)
  #define FUNCID_ET_DSO                                           ((word) 0x3FE)
#endif

#if  (defined MHP_RX_MIN) || (defined PACKET_ON_SECOND_HOST)
  #define FUNCID_ET_DSIHOLD                                       ((word) 0x3FF)
#endif


#ifdef ET_ADD6
    #define FUNCID_ET_CENTRALREGISTRYSIZE                           ((word) 0x212)
#endif

#ifdef ET_ADD8
    #define FUNCID_ET_ECLTRIGGER                                    ((word) 0x220)
    #define FUNCID_ET_ECLINITIATORSTATE                             ((word) 0x221)
#endif

#if (defined FUNCID_ET_DSO)
    #define ET_DSO_OK                                               ((byte) 0x00)
    #define ET_DSO_BUSY                                             ((byte) 0x01)
    #define ET_DSO_NO_MEMORY                                        ((byte) 0x02)
    #define ET_DSO_NEXTPACKET                                       ((byte) 0x03)
#endif

#ifdef FUNCID_ET_NOTIFICATIONMATRIXSIZE
    typedef struct tag_NtfMatrixSize
    {
        byte        MinSize;                 /* Minimum Notification Matrix Size */
        byte        MaxSize;                 /* Maximum Notification Matrix Size */
    }TNtfMatrixSize;
#endif



/*
+------------------------------------------------------------------------------+
| Declaration of service routines (called from the application)                |
+------------------------------------------------------------------------------+
*/


#ifdef ET_MIN

    void ET_Init(void);
    void ET_Go_Net_Off(void);
    byte ET_RxFilter(pTMsgTx tx_ptr, pTMsgRx rx_ptr);
    void ET_Store_Diag_Result(byte diag_result, byte *info);

#endif

/*
+------------------------------------------------------------------------------+
| Extern declarations of callbacks (implemented by the application)            |
+------------------------------------------------------------------------------+
*/
#ifdef ET_MIN

    extern byte ET_Shutdown_Request(byte type);
    extern byte ET_SharedRxTxMsgBuf_Query(void);
    extern void ET_Reset_Request(void);
    extern byte ET_NtfMatrixSize_Query(TNtfMatrixSize *pNtfMatrixSize);
    extern void ET_AutoWakeup_Request(byte delay, bool diag, byte duration);
    extern void ET_MOSTRemoteReset_Request(word src_adr, byte settings);
    #ifdef PACKET_ON_SECOND_HOST
        extern byte ET_DSIDSOCount_Request(pTMsgRx rx_ptr, pTMsgTx tx_ptr);
    #endif
    extern void ET_PhysicalLayerTest_Status(bool running);
    extern bool ET_FBlockInfo_Query(word id, bool answer_prepared, pTMsgTx tx_ptr);

    #if (!defined NETWORKMASTER_LOCAL) && (!defined NETWORKMASTER_SHADOW)
    extern void ET_SystemState_Request(word tgt_addr);
    #endif

#endif

#if (defined MHP_TX_MIN) || (defined PMHT_MIN) || (defined PACKET_ON_SECOND_HOST)
    extern byte ET_DSO_Request(word num_packets, byte next_packet_method, byte ack_mode,
                           word tgt_addr, byte fblock_id, byte inst_id, word fkt_id, byte op_type);
#endif

#if (defined MHP_RX_MIN) || (defined PACKET_ON_SECOND_HOST)
    extern void ET_DSIHold_Request(byte hold, byte dsi_idx);
#endif


#ifdef ET_ADD6
    extern word ET_CentralRegistrySize_Query(void);
#endif

#ifdef ET_ADD8
    extern void ET_ECLTrigger_Request(word src_addr, byte ecl_action, byte length, byte* ecl_data);
    extern void ET_ECLInitiatorState_Query (word node_class, byte* comm_state, byte* alive, byte* signal);
#endif



#endif /* _ET_H */
