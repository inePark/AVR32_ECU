/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Public API of the Virtual MOST Supervisor (VMSV)
File:           vmsv_pb.h
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
  * \brief      Public API of the Virtual MOST Supervisor (VMSV)
  */

#ifndef _VMSV_PB_H
#define _VMSV_PB_H

#include "mostdef1.h"

#define VMSV_P_NONE                     ((word) 0x0000)

#define VMSV_P_NWSTARTUP_RETRY          ((word) 0x0001)
#define VMSV_P_NWSHUTDOWN_RETRY         ((word) 0x0002)
#define VMSV_P_GO_PROTECTED             ((word) 0x0004)
#define VMSV_P_GO_ATTACHED              ((word) 0x0008)
#define VMSV_P_SHADOW_CHANGE            ((word) 0x0010)
#define VMSV_P_NTF_COMPLETE             ((word) 0x0020)
#define VMSV_P_BUF_FREED                ((word) 0x0040)
#define VMSV_P_CHECK_PHASE              ((word) 0x0080)
#define VMSV_CHECK_PHASE_TIMER          ((word) 0x0100)
#define VMSV_P_REPORT_EM                ((word) 0x0200)
#define VMSV_P_GO_SEMI_PROTECTED        ((word) 0x0400)
#define VMSV_P_SRV_CHECK                ((word) 0x0800)
#define VMSV_P_PHYSICAL_LAYER_TEST      ((word) 0x1000)
#define VMSV_P_EMERGENCYCONDITION_RETRY ((word) 0x2000)
#define VMSV_P_COMM_TIMER               ((word) 0x4000)
/*
#define VMSV_P_FREE16               ((word) 0x8000)
*/

#define VMSV_P_FIRST                VMSV_P_NWSTARTUP_RETRY

#ifdef MSV_DIAG_RESULT_MSG
    #define VMSV_P_LAST             VMSV_P_COMM_TIMER
#else
    #define VMSV_P_LAST             VMSV_P_EMERGENCYCONDITION_RETRY
#endif


#define MSV_UNLOCK                  ((byte) 0x00)
#define MSV_LOCK_STABLE             ((byte) 0x01)

#define MSVAL_E_UNLOCK              ((byte) 0x00)
#define MSVAL_E_LOCKSTABLE          ((byte) 0x01)
#define MSVAL_E_MPR                 ((byte) 0x10)
#define MSVAL_E_MPRDEL_INC          ((byte) 0x11)
#define MSVAL_E_MPRDEL_DEC          ((byte) 0x12)
#define MSVAL_E_MPRDEL_EQUAL        ((byte) 0x13)
#define MSVAL_E_NETON               ((byte) 0x20)
#define MSVAL_E_SHUTDOWN            ((byte) 0x21)
#define MSVAL_E_NPR                 ((byte) 0x60)
#define MSVAL_E_NET_ACTIVITY        ((byte) 0x80)

#define MSVAL_ERR_UNLOCK_SHORT      ((byte) 0x00)
#define MSVAL_ERR_UNLOCK_CRITICAL   ((byte) 0x01)
#define MSVAL_ERR_INIT_ERROR        ((byte) 0x04)
#define MSVAL_ERR_STARTUP_FAILED    ((byte) 0x10)
#define MSVAL_ERR_STARTUP_BUSY      ((byte) 0x11)
#define MSVAL_ERR_SHUTDOWN_FAILED   ((byte) 0x12)
#define MSVAL_ERR_SHUTDOWN_BUSY     ((byte) 0x13)

#define VMSV_MSU_TRANSMITTED        ((byte) 0x00)
#define VMSV_MSU_ALREADY_ON         ((byte) 0x01)
#define VMSV_MSU_DEVMODE_CONFLICT   ((byte) 0x02)

#define MSVAL_S_OFF                 ((byte) 0x00)
#define MSVAL_S_INIT                ((byte) 0x01)
#define MSVAL_S_RBD                 ((byte) 0x02)
#define MSVAL_S_ON                  ((byte) 0x03)
#define MSVAL_S_RBDRES              ((byte) 0x04)

#define MSVAL_DIAG_OK               ((byte) 0x00)
#define MSVAL_DIAG_POS              ((byte) 0x01)
#define MSVAL_DIAG_FAILED           ((byte) 0x02)
#define MSVAL_DIAG_SIGNAL_NO_LOCK   ((byte) 0x03)

#define PMI_STATE_NORMAL            ((byte) 0x00)
#define PMI_STATE_CRITICAL          ((byte) 0x01)
#define PMI_STATE_OFF               ((byte) 0x02)

#define PMI_REQPOWOFF               ((byte) 0x01)
#define PMI_STP                     ((byte) 0x02)

#define PMI_CLEAR                   ((byte) 0x00)
#define PMI_SHUTDOWN                ((byte) 0x01)

/* MOST spec. V3.0 definitions                              */
/* These values are valid for MOST50 as well as for MOST150 */
#define MSV_TIMEOUT_MASTER   ((word) 3000)
#define MSV_TIMEOUT_SLAVE    ((word) 5000)


#define VMSV_INVALID_TIME_VALUE     ((byte) 0xFF)


typedef void TVmsvNetOnCB (TMnsResult result, dword time);


#ifdef VMSV_4
    byte MostStartUp(byte dev_mode, byte options);
#endif

#ifdef VMSV_6
    void MostShutDown(void);
#endif

#ifdef VMSV_7
    byte MostGetState(void);
#endif

#ifdef VMSV_8
    byte MostGetDevMode(void);
#endif

#ifdef VMSV_9
    byte MostGetNCState(void);
#endif

#ifdef NS_MSV_NB
    void NbRefreshNodePos(void);
    void NbGoNetOff(void);
#endif

#ifdef NS_MSV_ET
    void ET_Go_Net_Off(void);
    void ET_PhysicalLayerTest_Finished(void);
#endif

#ifdef VMSV_22
    byte MostSetDevMode(byte mode);
#endif

#ifdef VMSV_31
    byte MostGetMaxPos(void);
#endif

#ifdef VMSV_32
    typedef struct TVmsvInternalHooks
    {
        void (*update_nwm_address_fptr)(word address);
        void (*store_error_info2_fptr)(byte info);

    } TVmsvInternalHooks;
    void VmsvRegisterInternalHooks(TVmsvInternalHooks *cbs_ptr);
#endif

#ifdef VMSV_49
    byte MostGetPMIState(byte *state_ptr, byte *events_ptr);
#endif

#ifdef VMSV_54
    byte MostGetSysErrMonitor(TMnsStdCB *cb_ptr);
#endif

#ifdef VMSV_71
    byte MostGetNetOnTime(TVmsvNetOnCB *cb_ptr);
#endif

#ifdef VMSV_81
    bool MostGetLockState(void);
#endif

#ifdef VMSV_87
    void VmsvResetSSOResult(void);
#endif

#ifdef VMSV_88
    byte VmsvGetSSOResult(TMsgTx *msg_ptr);
#endif

#ifdef VMSV_94
    byte VmsvPhysicalLayerTestStart(pTMsgRx rx_ptr, byte type, word lead_in, dword duration, word lead_out);
#endif

#ifdef VMSV_96
    bool VmsvPhysicalLayerTestResult( bool *lock_status, dword *num_errors);
#endif

#ifdef VMSV_100
void MostEmergencyCondition(bool state);
#endif


#endif /* _VMSV_PB_H */
