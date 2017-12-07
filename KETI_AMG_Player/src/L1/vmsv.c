/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Implementation of the Virtual MOST Supervisor (VMSV)
File:           vmsv.c
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
  * \brief      Implementation of the Virtual MOST Supervisor (VMSV)
  */

#include "pms.h"
#include "mns.h"
#include "ams.h"
#include "wads.h"
#include "vmsv.h"
#include "mdm.h"
#include "vmsv_pv.h"


/*
==============================================================================
    NetServices trace: module trace macros
==============================================================================
*/

#if (MNS_DEBUG & NST_C_FUNC_ENTRIES)
    #define T_API_ENTRY(func)   MNS_TRACE((MNS_P_SRV_VMSV, NST_E_FUNC_ENTRY_API, 1, func))
    #define T_LIB_ENTRY(func)   MNS_TRACE((MNS_P_SRV_VMSV, NST_E_FUNC_ENTRY_LIB, 1, func))
    #define T_MOD_ENTRY(func)   MNS_TRACE((MNS_P_SRV_VMSV, NST_E_FUNC_ENTRY_MOD, 1, func))
#else
    #define T_API_ENTRY(func)
    #define T_LIB_ENTRY(func)
    #define T_MOD_ENTRY(func)
#endif

#if (MNS_DEBUG & NST_C_FUNC_EXITS)
    #define T_API_EXIT(func)    MNS_TRACE((MNS_P_SRV_VMSV, NST_E_FUNC_EXIT_API, 1, func))
    #define T_LIB_EXIT(func)    MNS_TRACE((MNS_P_SRV_VMSV, NST_E_FUNC_EXIT_LIB, 1, func))
    #define T_MOD_EXIT(func)    MNS_TRACE((MNS_P_SRV_VMSV, NST_E_FUNC_EXIT_MOD, 1, func))
#else
    #define T_API_EXIT(func)
    #define T_LIB_EXIT(func)
    #define T_MOD_EXIT(func)
#endif

#if (MNS_DEBUG & NST_C_INIT)
    #define T_INIT()            MNS_TRACE((MNS_P_SRV_VMSV, NST_E_INIT, 0))
#else
    #define T_INIT()
#endif

#if (MNS_DEBUG & NST_C_SERVICE)
    #define T_SERVICE(event)    MNS_TRACE((MNS_P_SRV_VMSV, NST_E_SERVICE, 1, event))
#else
    #define T_SERVICE(event)
#endif

#if (MNS_DEBUG & NST_C_REQUESTS)
    #define T_REQUEST(event)    MNS_TRACE((MNS_P_SRV_VMSV, NST_E_REQUEST, 1, event))
#else
    #define T_REQUEST(event)
#endif


#ifdef MSV_DIAG_RESULT_MSG
#if (MNS_DEBUG & NST_C_STATES)
    #define T_COMM_CHANGE(state)   MNS_TRACE((MNS_P_SRV_VMSV, NST_E_STATE_CHANGE, 2, NST_P1_COMM, state))
#else
    #define T_COMM_CHANGE(state)
#endif
#endif

#if (MNS_DEBUG & NST_C_ASSERTS)
    #define FAILED_ASSERT()     MNS_TRACE((MNS_P_SRV_VMSV, NST_E_ASSERT, 1, __LINE__))
    #define ASSERT(exp)         if(!(exp)) FAILED_ASSERT()
#else
    #define FAILED_ASSERT()
    #define ASSERT(exp)
#endif

#define TAKE_EVENTS()   WAIT4MX(MX_VMSV_PE)
#define GIVE_EVENTS()   REL_MX(MX_VMSV_PE)
#define TAKE_VMSV()     WAIT4MX(MX_VMSV_CS)
#define GIVE_VMSV()     REL_MX(MX_VMSV_CS)


/*
================================================================================
    Module Internal Variables
================================================================================
*/

#ifdef VMSV_MIN
    /*! Data variable of the VMSV module */
    static VmsvData vmsv;
#endif  /* #ifdef VMSV_MIN */

/*
==============================================================================
==============================================================================
    Module Implementation
==============================================================================
==============================================================================
*/

#ifdef VMSV_0
void VmsvInit(struct TVmsvConfig *cfg_ptr)
{
    T_LIB_ENTRY(VMSV_0);

    ASSERT(cfg_ptr);
    T_INIT();

    vmsv.cfg_ptr = cfg_ptr;

    TAKE_EVENTS();
    vmsv.events.pending     = VMSV_P_NONE;
    vmsv.events.latest      = VMSV_P_NONE;
    GIVE_EVENTS();

    vmsv.update_nwm_address_fptr = NULL;
    vmsv.store_error_info2_fptr = NULL;
    MostRegisterTimer(&(vmsv.check_phase.timer), VmsvSetPendingEvent, VMSV_P_CHECK_PHASE);

    #ifdef MSV_DIAG_RESULT_MSG
    MostRegisterTimer(&(vmsv.nwstartup.comm_timer), VmsvSetPendingEvent, VMSV_P_COMM_TIMER);

    TAKE_VMSV();
    vmsv.shadow.ni_wakeup_mode.mode    = INIC_SHADOW_INVALID_BYTE;
    vmsv.shadow.ni_wakeup_mode.request = INIC_SHADOW_INVALID_BYTE;
    vmsv.shadow.changed                = MNS_FALSE;
    GIVE_VMSV();
    #endif

    #ifdef NS_MSV_ET
    MostRegisterTimer(&(vmsv.plt.state_timer), VmsvSetPendingEvent, VMSV_P_PHYSICAL_LAYER_TEST);

    TAKE_VMSV();
    vmsv.plt.test_state     = VMSV_PLT_OFF;
    vmsv.plt.test_done      = MNS_FALSE;
    vmsv.plr.lock_status    = MNS_FALSE;
    vmsv.plr.err_count      = (word)0;
    GIVE_VMSV();
    #endif

    #ifdef _OS81110_SSO
    /* restore ssoresult from nonvolatile memory */
    if (vmsv.cfg_ptr->sso_restore_ssoresult_fptr)
    {
        byte ssoresult;
        ssoresult = vmsv.cfg_ptr->sso_restore_ssoresult_fptr();

        TAKE_VMSV();
        vmsv.shadow.ssoresult = ssoresult;
        GIVE_VMSV();
    }
    else
    {
        TAKE_VMSV();
        vmsv.shadow.ssoresult = SDR_NO_RESULT;
        GIVE_VMSV();
    }
    #endif

    TAKE_VMSV();
    vmsv.emergency_state = MNS_FALSE;
    GIVE_VMSV();

    T_LIB_EXIT(VMSV_0);
}
#endif

#ifdef VMSV_1
void VmsvService(void)
{
    word event_to_handle;
    bool request_flag;

    T_LIB_ENTRY(VMSV_1);

    event_to_handle = VmsvGetNextEventToHandle();
    request_flag    = MNS_FALSE;

    T_SERVICE(event_to_handle);

    switch (event_to_handle)
    {
        case VMSV_P_GO_PROTECTED:
            VmsvEHCIGoProtected();
            break;

        case VMSV_P_GO_SEMI_PROTECTED:
            VmsvEHCIGoSemiProtected();
            break;

        case VMSV_P_GO_ATTACHED:
            VmsvEHCIGoAttached();
            break;

        case VMSV_P_NWSTARTUP_RETRY:
            VmsvNWStartUpRetry();
            break;

        case VMSV_P_NWSHUTDOWN_RETRY:
            MostShutDown();
            break;

        case VMSV_P_SHADOW_CHANGE:
            VmsvFireCallbacks();
            break;

        case VMSV_P_NTF_COMPLETE:
            MnsServiceInitComplete(MNS_PHASE_INIT, MNS_P_SRV_VMSV);
            break;

        case VMSV_P_CHECK_PHASE:
            break;

        case VMSV_P_BUF_FREED:
            (void)VmsvRetry();
            break;

        case VMSV_P_REPORT_EM:
            VmsvReportSysErrMonitor();
            break;

        #ifdef MSV_DIAG_RESULT_MSG
        case VMSV_P_COMM_TIMER:
            {
                byte next_state = VMSV_COMM_OFF;

                TAKE_VMSV();
                switch (vmsv.nwstartup.comm_state)
                {
                    case VMSV_COMM_CALL_CB:
                        next_state = VMSV_COMM_STARTNW;
                        break;

                    case VMSV_COMM_STARTNW:
                        next_state = VMSV_COMM_BROADCAST;
                        break;

                    case VMSV_COMM_BROADCAST:
                        next_state = VMSV_COMM_WAIT_AFTER_SEND;
                        break;

                    default:
                        FAILED_ASSERT();
                        break;
                }
                GIVE_VMSV();

                if (VMSV_COMM_OFF != next_state)
                {
                    VmsvCommRBDResultPos0(next_state);
                }
            }
            break;
        #endif

        #ifdef VMSV_95
        case VMSV_P_PHYSICAL_LAYER_TEST:
            (void)VmsvPhysicalLayerTestProcedure();
            break;
        #endif

        #ifdef VMSV_100
        case VMSV_P_EMERGENCYCONDITION_RETRY:
        {
            bool state;

            TAKE_VMSV();
            state = vmsv.emergency_state;
            GIVE_VMSV();

            MostEmergencyCondition(state);
            break;
        }
        #endif

        default:
            event_to_handle = VMSV_P_NONE;
            FAILED_ASSERT();
            break;
    }

    TAKE_EVENTS();
    request_flag = (VMSV_P_NONE != vmsv.events.pending) ? MNS_TRUE : MNS_FALSE;
    GIVE_EVENTS();

    if (MNS_FALSE != request_flag)
    {
        MnsSetPendingService(MNS_P_SRV_VMSV);
    }

    T_LIB_EXIT(VMSV_1);
}
#endif

#ifdef VMSV_2
void VmsvSetPendingEvent(word event_flag)
{
    T_MOD_ENTRY(VMSV_2);
    T_REQUEST(event_flag);
    MnsSetPendingEventFlag(event_flag, MX_VMSV_PE,
                           &vmsv.events.pending, MNS_P_SRV_VMSV);
    T_MOD_EXIT(VMSV_2);
}
#endif

#ifdef VMSV_3
static word VmsvGetNextEventToHandle(void)
{
    word result;

    T_MOD_ENTRY(VMSV_3);
    result = MnsGetNextEventFlagToCall(MX_VMSV_PE,
                                       &vmsv.events.pending,
                                       &vmsv.events.latest,
                                       VMSV_P_FIRST, VMSV_P_LAST);
    T_MOD_EXIT(VMSV_3);
    return(result);
}
#endif

#ifdef VMSV_4
byte MostStartUp(byte dev_mode, byte options)
{
    byte result;
    byte new_state;

    T_API_ENTRY(VMSV_4);

    result    = VMSV_MSU_TRANSMITTED;
    new_state = VMSV_NWSTARTUP_COMPLETE;

    if (    (MNS_MASTER       != dev_mode)
         && (MNS_SLAVE        != dev_mode)
         && (MNS_DEVMODE_AUTO != dev_mode))
    {
        result = ERR_NOT_SUPPORTED;
    }
    else if (DIAGNOSIS & options)
    {
        if (MNS_FALSE == MostIsSupported(NSF_RBD))
        {
            result = ERR_NOT_SUPPORTED;
        }
        else if ((MNS_DEVMODE_AUTO != dev_mode) && (dev_mode != MostGetDevMode()))
        {
            if (MNS_NET_ON == MostGetState())
            {
                result = VMSV_MSU_DEVMODE_CONFLICT;
            }
            else
            {
                new_state = VMSV_NWSTARTUP_DEVMODE;
            }
        }
        else
        {
            new_state = VMSV_NWSTARTUP_RBD;
        }
    }
    else
    {
        if (MNS_NET_ON == MostGetState())
        {
            result = VMSV_MSU_ALREADY_ON;
        }
        else if ((MNS_DEVMODE_AUTO != dev_mode) && (dev_mode != MostGetDevMode()))
        {
            new_state = VMSV_NWSTARTUP_DEVMODE;
        }
        else
        {
            new_state = VMSV_NWSTARTUP_NWSTARTUP;
        }
    }

    if (VMSV_NWSTARTUP_COMPLETE != new_state)
    {
        TAKE_VMSV();
        vmsv.nwstartup.state     = new_state;
        vmsv.nwstartup.dev_mode  = dev_mode;
        vmsv.nwstartup.diagnosis = (DIAGNOSIS == options) ? MNS_TRUE : MNS_FALSE;
        GIVE_VMSV();
        VmsvNWStartUpRetry();
    }

    T_API_EXIT(VMSV_4);
    return(result);
}
#endif

#ifdef VMSV_5
static void VmsvNWStartUpRetry(void)
{
    TMsgTx *msg_ptr;
    bool problem;

    T_MOD_ENTRY(VMSV_5);

    problem = MNS_FALSE;
    msg_ptr = MsgGetTxPtrExt(7);
    if (msg_ptr)
    {
        msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
        msg_ptr->FBlock_ID = FBLOCK_INIC;

        TAKE_VMSV();
        switch (vmsv.nwstartup.state)
        {
            case VMSV_NWSTARTUP_DEVMODE:
                msg_ptr->Func_ID   = FUNCID_INIC_DEVICEMODE;
                msg_ptr->Operation = OP_SET;
                msg_ptr->Length    = (word)1;
                msg_ptr->Data[0]   = vmsv.nwstartup.dev_mode;
                break;

            case VMSV_NWSTARTUP_NWSTARTUP:
                msg_ptr->Func_ID   = FUNCID_INIC_NWSTARTUP;
                msg_ptr->Operation = OP_STARTRESULT;
                msg_ptr->Length    = (word)0;
                break;

            case VMSV_NWSTARTUP_RBD:
                msg_ptr->Func_ID   = FUNCID_INIC_RBDTRIGGER;
                msg_ptr->Operation = OP_START;
                msg_ptr->Length    = (word)1;
                msg_ptr->Data[0]   = (byte)0x00;

                #ifdef MSV_DIAG_RESULT_MSG
                    vmsv.nwstartup.comm_state = VMSV_COMM_OFF;
                #endif

                break;

             default:
                FAILED_ASSERT();
                problem = MNS_TRUE;
                break;
        }
        if (VMSV_NWSTARTUP_DEVMODE != vmsv.nwstartup.state)
        {
            vmsv.nwstartup.state = VMSV_NWSTARTUP_COMPLETE;
        }
        GIVE_VMSV();

        if (MNS_FALSE == problem)
        {
            MsgSend3(msg_ptr);
        }
        else
        {
            MsgTxUnused(msg_ptr);
        }
    }
    else
    {
        VmsvSetPendingEvent(VMSV_P_NWSTARTUP_RETRY);
    }
    T_MOD_EXIT(VMSV_5);
}
#endif

#ifdef VMSV_6
void MostShutDown(void)
{
    TMsgTx *msg_ptr;

    T_API_ENTRY(VMSV_6);

    msg_ptr = MsgGetTxPtrExt(0);

    if (msg_ptr)
    {
        msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
        msg_ptr->FBlock_ID = FBLOCK_INIC;
        msg_ptr->Func_ID   = FUNCID_INIC_NWSHUTDOWN;
        msg_ptr->Operation = OP_STARTRESULT;
        msg_ptr->Length    = (word)0;

        MsgSend3(msg_ptr);
    }
    else
    {
        VmsvSetPendingEvent(VMSV_P_NWSHUTDOWN_RETRY);
    }

    T_API_EXIT(VMSV_6);
}
#endif

#ifdef VMSV_7
byte MostGetState(void)
{
    byte state;
    bool reported;

    T_API_ENTRY(VMSV_7);
    TAKE_VMSV();
    state = vmsv.shadow.nistate;
    reported = vmsv.shadow.neton_reported;
    GIVE_VMSV();

    if(MSVAL_S_ON == state)
    {
        if (MNS_FALSE == reported)
        {
            #ifdef SCM_MIN
            byte boundary = ScmGetBoundary();
            bool bdvalid  = (INIC_SHADOW_INVALID_BYTE != boundary) &&
                            ((MNS_FALSE != MostIsSupported(NSF_MOST_25)) ? ((0x00 != boundary) ? MNS_TRUE : MNS_FALSE) : MNS_TRUE);
            #else
            bool bdvalid  = MNS_TRUE;
            #endif

           /* MSVAL_E_NETON shall only be communicated when the
            * boundary is valid
            * see also VmsvHandleNIStateChange() and ScmHandleBandwidthStatus()
            */
            if (EHCISTATE_IS_ATTACHED() && (MNS_FALSE != bdvalid))
            {
                MnsSignalNetOn();
                VmsvReportNetOn();
                MnsDistribEvent(MNS_P_SRV_VMSV, VMSV_P_SRV_CHECK);
            }
        }
        state = MNS_NET_ON;
    }
    else
    {
        state = MNS_NET_NOTAVAIL;
    }

    T_API_EXIT(VMSV_7);
    return(state);
}

#endif



#ifdef VMSV_8
byte MostGetDevMode(void)
{
    byte state;

    T_API_ENTRY(VMSV_8);
    TAKE_VMSV();
    state = vmsv.shadow.devmode;
    GIVE_VMSV();
    T_API_EXIT(VMSV_8);

    return(state);
}
#endif

#ifdef VMSV_9
byte MostGetNCState(void)
{
    byte temp;
    T_API_ENTRY(VMSV_9);

    TAKE_VMSV();
    temp = vmsv.shadow.ncstate;
    GIVE_VMSV();

    T_API_EXIT(VMSV_9);
    return(temp);
}
#endif

#ifdef VMSV_11
static void VmsvEHCIGoProtected(void)
{
    T_MOD_ENTRY(VMSV_11);

    TAKE_VMSV();
    vmsv.nwstartup.state          = VMSV_NWSTARTUP_COMPLETE;
    vmsv.nwstartup.dev_mode       = INIC_SHADOW_INVALID_BYTE;
    vmsv.nwstartup.diagnosis      = MNS_FALSE;

    #ifdef MSV_DIAG_RESULT_MSG
    vmsv.nwstartup.comm_state     = VMSV_COMM_OFF;
    T_COMM_CHANGE(VMSV_COMM_OFF);
    #endif

    vmsv.shadow.nistate            = INIC_SHADOW_INVALID_BYTE;
    vmsv.shadow.late_nistate       = INIC_SHADOW_INVALID_BYTE;
    vmsv.shadow.nievent            = INIC_SHADOW_INVALID_BYTE;
    vmsv.shadow.devmode            = INIC_SHADOW_INVALID_BYTE;
    vmsv.shadow.ncstate            = NCS_NOT_OK;
    vmsv.shadow.nwm_addr           = NCS_INVALID_NWM;
    vmsv.shadow.ncstate_flags      = INIC_SHADOW_INVALID_BYTE;
    vmsv.shadow.lockstate          = INIC_SHADOW_INVALID_BYTE;
    vmsv.shadow.ncedelayed[0]      = INIC_SHADOW_INVALID_BYTE;
    vmsv.shadow.ncedelayed[1]      = INIC_SHADOW_INVALID_BYTE;
    vmsv.shadow.rbdresult[0]       = INIC_SHADOW_INVALID_BYTE;
    vmsv.shadow.rbdresult[1]       = INIC_SHADOW_INVALID_BYTE;
    vmsv.check_phase.state         = VMSV_CP_DO_NOTHING;
    vmsv.shadow.node_pos_check     = INIC_SHADOW_INVALID_WORD;
    vmsv.shadow.neton_reported     = MNS_FALSE;
    vmsv.shadow.delayed_nce        = MNS_FALSE;
    vmsv.shadow.delayed_mpr        = MNS_FALSE;
    vmsv.shadow.pmi.state          = INIC_SHADOW_INVALID_BYTE;
    vmsv.shadow.pmi.events         = INIC_SHADOW_INVALID_BYTE;
    vmsv.sys_err_monitor.cb_ptr   = NULL;
    vmsv.sys_err_monitor.bit_mask = (byte)0;
    vmsv.net_on_cb_ptr            = NULL;
    vmsv.attached                 = MNS_FALSE;
    vmsv.fblock_ids_get_received  = MNS_FALSE;
    vmsv.fblock_ids_status_sent   = MNS_FALSE;
    GIVE_VMSV();

    TAKE_EVENTS();
    vmsv.retry = (word)0;
    GIVE_EVENTS();

    MnsServiceInitComplete(MNS_PHASE_RESET, MNS_P_SRV_VMSV);

    T_MOD_EXIT(VMSV_11);
}
#endif

#ifdef VMSV_12
static void VmsvEHCIGoAttached(void)
{
    T_LIB_ENTRY(VMSV_12);

    VmsvCheckForNpr(MNS_TRUE);
    VmsvSetPendingEvent(VMSV_P_SHADOW_CHANGE);
    MostSetTimer(&(vmsv.check_phase.timer), VMSV_CHECK_PHASE_TIMER, MNS_FALSE);

    TAKE_VMSV();
    if ((NCS_INVALID_NWM != vmsv.shadow.nwm_addr) &&
        (MNS_FALSE == vmsv.fblock_ids_get_received) &&
        (MSVAL_S_ON == vmsv.shadow.nistate))
    {
        vmsv.check_phase.state = (NCS_OK == vmsv.shadow.ncstate) ?
                                 VMSV_CP_INJECTED : VMSV_CP_WAITING;
    }
    vmsv.attached = MNS_TRUE;
    GIVE_VMSV();

    if (VMSV_CP_INJECTED == vmsv.check_phase.state)
    {
        (void)VmsvInjectFBlockIDsGet();
        (void)VmsvInjectCfgState();
    }

    T_LIB_EXIT(VMSV_12);
}
#endif

#ifdef VMSV_13
void VmsvHandleDeviceModeStatus(TMsgRx *msg_ptr)
{
    bool state_change;

    T_LIB_ENTRY(VMSV_13);

    ASSERT(msg_ptr);

    state_change = MNS_FALSE;

    TAKE_VMSV();
    vmsv.shadow.devmode = msg_ptr->Data[0];
    if(VMSV_NWSTARTUP_DEVMODE == vmsv.nwstartup.state)
    {
        vmsv.nwstartup.state = (MNS_FALSE != vmsv.nwstartup.diagnosis) ?
            VMSV_NWSTARTUP_RBD : VMSV_NWSTARTUP_NWSTARTUP;
        state_change = MNS_TRUE;
    }
    GIVE_VMSV();

    if (MNS_FALSE != state_change)
    {
        VmsvNWStartUpRetry();
    }
    if (!EHCISTATE_IS_ATTACHED())
    {
        MnsNtfCheck(NTF_DEVMODE);
    }

    T_LIB_EXIT(VMSV_13);
}
#endif

/*! Message handler for INIC.LockState.Status.
  *
  * @author     SSemmler
  * @updated       4/7/2005
  * @version    2.0.3
  * @see        VmsvFireCallbacks
  * @see        VmsvHandleLockStateChange
  *
  * @param      msg_ptr     the received message
  */
#ifdef VMSV_14
void VmsvHandleLockStateStatus(TMsgRx *msg_ptr)
{
    T_LIB_ENTRY(VMSV_14);

    ASSERT(msg_ptr);

    TAKE_VMSV();
    vmsv.shadow.changed |= VMSV_SHADOW_LOCKSTATE_CHANGED;
    vmsv.shadow.lockstate  = msg_ptr->Data[0];
    GIVE_VMSV();

    if (EHCISTATE_IS_ATTACHED())
    {
        VmsvSetPendingEvent(VMSV_P_SHADOW_CHANGE);
    }
    else
    {
        MnsNtfCheck(NTF_LOCKSTATE);
    }

    #ifdef NS_MSV_ET
    TAKE_VMSV();
    if (   (    (VMSV_PLT_TEST1 == vmsv.plt.test_state)
             || (VMSV_PLT_TEST2 == vmsv.plt.test_state) )
        && (MSVAL_E_UNLOCK == vmsv.shadow.lockstate) )
    {
        vmsv.plr.lock_status = MNS_TRUE;
    }
    GIVE_VMSV();
    #endif

    T_LIB_EXIT(VMSV_14);
}
#endif

/*! Message handler for INIC.NIEvent.Status.
  *
  * @author     SSemmler
  * @updated       4/6/2005
  * @version    2.0.3
  * @see        VmsvFireCallbacks
  * @see        VmsvHandleNIEventChange
  *
  * @param      msg_ptr     the received message
  */
#ifdef VMSV_16
void VmsvHandleNIEventStatus(TMsgRx *msg_ptr)
{
    T_LIB_ENTRY(VMSV_16);

    ASSERT(msg_ptr);

    TAKE_VMSV();
    vmsv.shadow.changed |= VMSV_SHADOW_NIEVENT_CHANGED;
    vmsv.shadow.nievent  = msg_ptr->Data[0];
    GIVE_VMSV();

    if (EHCISTATE_IS_ATTACHED())
    {
        VmsvSetPendingEvent(VMSV_P_SHADOW_CHANGE);
    }

    T_LIB_EXIT(VMSV_16);
}
#endif

#ifdef VMSV_17
void VmsvHandleNIStateStatus(TMsgRx *msg_ptr)
{
    bool attached;

    T_LIB_ENTRY(VMSV_17);

    attached = EHCISTATE_IS_ATTACHED() ? MNS_TRUE : MNS_FALSE;

    ASSERT(msg_ptr);

    TAKE_VMSV();
    vmsv.shadow.changed       |= VMSV_SHADOW_NISTATE_CHANGED;
    vmsv.shadow.late_nistate  =  vmsv.shadow.nistate;
    vmsv.shadow.nistate       = msg_ptr->Data[0];

    if (NISTATE_NET_OFF == msg_ptr->Data[0])
    {
        vmsv.shadow.ncstate  = NCS_NOT_OK;
        vmsv.shadow.nwm_addr = NCS_INVALID_NWM;
    }

    GIVE_VMSV();

    MnsDistribEvent(MNS_P_SRV_VMSV, VMSV_P_SRV_CHECK);

    if ((NISTATE_NET_ON == msg_ptr->Data[0]) && (MNS_FALSE != attached))
    {
        #ifdef AMS_MIN
            #ifdef SCM_MIN
            byte boundary = ScmGetBoundary();
            bool bdvalid  = (INIC_SHADOW_INVALID_BYTE != boundary) &&
                            ((MNS_FALSE != MostIsSupported(NSF_MOST_25)) ? ((0x00 != boundary) ? MNS_TRUE : MNS_FALSE) : MNS_TRUE);
            #else
            bool bdvalid  = MNS_TRUE;
            #endif

            if (MNS_FALSE != bdvalid)
            {
                MnsSignalNetOn();
            }
        #endif
    }
    else if (NISTATE_NET_INIT != msg_ptr->Data[0])
    {
        #ifdef AMS_MIN
            MsgNIStateNetOn(MNS_FALSE);
        #endif
    }
    else    /* (NET_INIT == msg_ptr->Data[0]) */
    {
        #ifdef AMS_53
            MsgNIStateNetInit();
        #endif
    }

    if ((NISTATE_NET_ON == msg_ptr->Data[0]) && (MNS_FALSE != attached) && (MNS_FALSE != MnsPMReady()))
    {
        #ifdef ADS_10
            DataNIStateNetOn(MNS_TRUE);
        #endif
    }
    else if (NISTATE_NET_INIT != msg_ptr->Data[0])
    {
        #ifdef ADS_10
            DataNIStateNetOn(MNS_FALSE);
        #endif
    }

    if (NISTATE_NET_OFF == msg_ptr->Data[0])
    {
        MnsDistribEvent(MNS_P_SRV_MNS, MNS_P_AS_GO_NET_OFF);
    }

    if (MNS_FALSE != attached)
    {
        VmsvSetPendingEvent(VMSV_P_SHADOW_CHANGE);
    }
    else
    {
        MnsNtfCheck(NTF_NISTATE);
    }


    T_LIB_EXIT(VMSV_17);
}
#endif

/*! Message handler for INIC.NCEDelayed.Status.
  *
  * @author     SSemmler
  * @updated       4/6/2005
  * @version    2.0.3
  * @see        VmsvFireCallbacks
  * @see        VmsvHandleNCEDelayedChange
  *
  * @param      msg_ptr     the received message
  */
#ifdef VMSV_18
void VmsvHandleNCEDelayedStatus(TMsgRx *msg_ptr)
{
    T_LIB_ENTRY(VMSV_18);

    ASSERT(msg_ptr);

    TAKE_VMSV();
    vmsv.shadow.changed       |= VMSV_SHADOW_NCEDELAYED_CHANGED;
    vmsv.shadow.ncedelayed[0]  = msg_ptr->Data[0];
    vmsv.shadow.ncedelayed[1]  = msg_ptr->Data[1];
    GIVE_VMSV();

    if (EHCISTATE_IS_ATTACHED())
    {
        VmsvSetPendingEvent(VMSV_P_SHADOW_CHANGE);
    }

    T_LIB_EXIT(VMSV_18);
}
#endif

#ifdef VMSV_58
void VmsvRefreshNodePos(byte pos)
{
    T_LIB_ENTRY(VMSV_58);

    TAKE_VMSV();
    vmsv.shadow.node_pos_check &= (word) 0xFF00;
    vmsv.shadow.node_pos_check |= pos;
    GIVE_VMSV();

    if (EHCISTATE_IS_ATTACHED())
    {
        VmsvCheckForNpr(MNS_FALSE);
    }

    T_LIB_EXIT(VMSV_58);
}
#endif

#ifdef VMSV_20
void VmsvHandleNCStateStatus(TMsgRx *msg_ptr)
{
    bool attached;

    T_LIB_ENTRY(VMSV_20);

    ASSERT(msg_ptr);

    TAKE_VMSV();
    attached = vmsv.attached;
    GIVE_VMSV();

    if (!EHCISTATE_IS_ATTACHED() ||
        (MostGetTimer(&vmsv.check_phase.timer) > (word)0) ||
        (MNS_FALSE == attached) )
    {
        TAKE_VMSV();
        vmsv.shadow.ncstate         = msg_ptr->Data[0];
        vmsv.shadow.nwm_addr        = (word)(((word)msg_ptr->Data[1] << 8) |
                                              (word)msg_ptr->Data[2]);
        vmsv.shadow.ncstate_flags   = msg_ptr->Data[3];
        GIVE_VMSV();

        if (!EHCISTATE_IS_ATTACHED() && (MNS_FALSE == attached))
        {
            MnsNtfCheck(NTF_NCSTATE);
        }
        else
        {
            VmsvHandleNCStateChange();
        }
    }

    T_LIB_EXIT(VMSV_20);
}
#endif

#ifdef VMSV_22
/*!
  * \brief      Changes the current device mode
  * \details    This function is used to set the device as a timing master or
  *             slave before the network is started. The default mode is retrieved
  *             from the INIC's configuration string.
  * \param[in]  mode    The device mode:
  *                     -# MNS_SLAVE: The network interface is configured in
  *                        slave mode
  *                     -# MNS_MASTER: The network interface is configured as
  *                        timing master
  * \return     The function returns the standard error mask:
  *             -# ERR_NO
  *             -# ERR_BUFOV
  *             -# ERR_PARAM
  */
byte MostSetDevMode(byte mode)
{
    TMsgTx *msg_ptr;
    byte    result;

    T_API_ENTRY(VMSV_22);

    result  = ERR_NO;

    if ((MNS_SLAVE == mode)||(MNS_MASTER == mode))
    {
        msg_ptr = MsgGetTxPtrExt(1);
        if (msg_ptr)
        {
            msg_ptr->Tgt_Adr    = MSG_TGT_INIC;
            msg_ptr->FBlock_ID  = FBLOCK_INIC;
            msg_ptr->Func_ID    = FUNCID_INIC_DEVICEMODE;
            msg_ptr->Operation  = OP_SET;
            msg_ptr->Data[0]    = mode;
            msg_ptr->Length     = (word)1;

            MsgSend3(msg_ptr);
        }
        else
        {
            result = ERR_BUFOV;
        }
    }
    else
    {
        result = ERR_PARAM;
    }

    T_API_EXIT(VMSV_22);
    return(result);
}
#endif

/*! Message handler for INIC.RBDResult.Status.
  *
  * @author     SSemmler
  * @updated       4/7/2005
  * @version    2.0.3
  * @see        VmsvFireCallbacks
  * @see        VmsvHandleRBDResultChange
  *
  * @param      msg_ptr     the received message
  */
#ifdef VMSV_23
void VmsvHandleRBDResultStatus(TMsgRx *msg_ptr)
{
    T_LIB_ENTRY(VMSV_23);

    ASSERT(msg_ptr);

    TAKE_VMSV();
    vmsv.shadow.changed |= VMSV_SHADOW_RBDRESULT_CHANGED;
    vmsv.shadow.rbdresult[0] = msg_ptr->Data[0];
    if (msg_ptr->Length > 1)
    {
        vmsv.shadow.rbdresult[1] = msg_ptr->Data[1];
    }
    else
    {
        /* set to 0 if not used */
        vmsv.shadow.rbdresult[1] = (byte)0;
    }
    GIVE_VMSV();

    if (EHCISTATE_IS_ATTACHED())
    {
        VmsvSetPendingEvent(VMSV_P_SHADOW_CHANGE);
    }

    T_LIB_EXIT(VMSV_23);
}
#endif


/*! This module function is called whenever the VMSV_P_SHADOW_CHANGE event is
  * seen. It figures the changes out, serves one of them and reschedules the
  * event if there are more changes to process (one change at a time).
  *
  * @author     SSemmler
  * @updated    4/6/2005
  * @version    2.0.3
  */
#ifdef VMSV_25
static void VmsvFireCallbacks(void)
{
    byte change;
    byte bit_pos;
    byte test;

    T_MOD_ENTRY(VMSV_25);

    change  = (byte)0;
    bit_pos = (byte)0;
    test    = (byte)0;

    TAKE_VMSV();
    while (((byte)0 == change) && (bit_pos < 8))
    {
        test = 1 << bit_pos;

        if (test & vmsv.shadow.changed)
        {
            vmsv.shadow.changed &= ~test;
            change = test;
        }
        else
        {
            bit_pos++;
        }
    }
    if (vmsv.shadow.changed)
    {
        VmsvSetPendingEvent(VMSV_P_SHADOW_CHANGE);
    }
    GIVE_VMSV();

    switch (change)
    {
        case VMSV_SHADOW_NISTATE_CHANGED:
            VmsvHandleNIStateChange();
            break;

        case VMSV_SHADOW_NIEVENT_CHANGED:
            VmsvHandleNIEventChange();
            break;

        case VMSV_SHADOW_NCEDELAYED_CHANGED:
            VmsvHandleNCEDelayedChange();
            break;

        case VMSV_SHADOW_LOCKSTATE_CHANGED:
            VmsvHandleLockStateChange();
            break;

        case VMSV_SHADOW_RBDRESULT_CHANGED:
            VmsvHandleRBDResultChange();
            break;

        case VMSV_SHADOW_PMISTATE_CHANGED:
            VmsvHandlePMIStateChange();
            break;

        default:
            FAILED_ASSERT();
            break;
    }

    T_MOD_EXIT(VMSV_25);
}
#endif

/*! Delayed callback processing for INIC.NIState.Status. Runs only during
  * attached mode.
  *
  * @author     SSemmler
  * @updated       4/6/2005
  * @version    2.0.3
  * @see        VmsvFireCallbacks
  * @see        VmsvHandleNIStateStatus
  */
#ifdef VMSV_26
static void VmsvHandleNIStateChange(void)
{
    byte event;
    byte info[3];
    byte nistate;
    byte late_nistate;
    bool reported;

    T_MOD_ENTRY(VMSV_26);

    event   = (byte)0xFF;
    info[0] = (byte)0;
    info[1] = (byte)0;
    info[2] = (byte)0;

    TAKE_VMSV();
    late_nistate = vmsv.shadow.late_nistate;
    nistate      = vmsv.shadow.nistate;
    reported     = vmsv.shadow.neton_reported;
    GIVE_VMSV();

    ASSERT(INIC_SHADOW_INVALID_BYTE != nistate);

    switch (nistate)
    {
        case NISTATE_NET_ON:
            {
                #ifdef SCM_MIN
                byte boundary = ScmGetBoundary();
                bool bdvalid  = (INIC_SHADOW_INVALID_BYTE != boundary) &&
                                ((MNS_FALSE != MostIsSupported(NSF_MOST_25)) ? ((0x00 != boundary) ? MNS_TRUE : MNS_FALSE) : MNS_TRUE);
                #else
                bool bdvalid  = MNS_TRUE;
                #endif

                /* MSVAL_E_NETON shall only be communicated when the
                 * boundary is valid
                 * see also MostGetState() and ScmHandleBandwidthStatus()
                 */
                if ((MNS_FALSE == reported) && (MNS_FALSE != bdvalid))
                {
                    MnsSignalNetOn();
                    VmsvReportNetOn();
                    MnsDistribEvent(MNS_P_SRV_VMSV, VMSV_P_SRV_CHECK);
                }
            }
            break;

        case NISTATE_NET_OFF:
            event = MSVAL_E_SHUTDOWN;

            #ifdef MSV_DIAG_RESULT_MSG
            {
                bool react = MNS_FALSE;

                TAKE_VMSV();
                react = (VMSV_COMM_BROADCAST == vmsv.nwstartup.comm_state) ?
                        MNS_TRUE : MNS_FALSE;
                GIVE_VMSV();

                if (MNS_FALSE != react)
                {
                    VmsvCommRBDResultPos0(VMSV_COMM_RESTORE_DEVMODE);
                }
            }
            #endif

        /*lint -e(616) control flows into case default */
        default:
            TAKE_VMSV();
            vmsv.shadow.neton_reported = MNS_FALSE;
            vmsv.shadow.delayed_nce    = MNS_FALSE;
            vmsv.shadow.delayed_mpr    = MNS_FALSE;
            GIVE_VMSV();
            break;
    }

    if (vmsv.cfg_ptr->msval_state_changed_fptr)
    {
        vmsv.cfg_ptr->msval_state_changed_fptr(nistate);
    }
    if (0xFF != event && vmsv.cfg_ptr->msval_event_fptr)
    {
        vmsv.cfg_ptr->msval_event_fptr(event, info);
    }

    if ((MSVAL_S_INIT == late_nistate) && (MSVAL_S_OFF == nistate) &&
         vmsv.cfg_ptr->msval_error_fptr)
    {
        vmsv.cfg_ptr->msval_error_fptr(MSVAL_ERR_INIT_ERROR, NULL);
    }

    T_MOD_EXIT(VMSV_26);
}
#endif

/*! Delayed callback processing for INIC.NIState.Status. Runs only during
  * attached mode.
  *
  * @author     SSemmler
  * @updated       4/6/2005
  * @version    2.0.3
  * @see        VmsvFireCallbacks
  * @see        VmsvHandleNIEventStatus
  */
#ifdef VMSV_27
static void VmsvHandleNIEventChange(void)
{
    byte nievent;
    byte state;
    bool reported;

    T_LIB_ENTRY(VMSV_27);

    TAKE_VMSV();
    nievent = vmsv.shadow.nievent;
    state = vmsv.shadow.nistate;
    reported = vmsv.shadow.neton_reported;
    GIVE_VMSV();

    ASSERT(INIC_SHADOW_INVALID_BYTE != nievent);


    /* NIState needs to be "net on" and reported to be able to fire
     * MSVAL_E_MPR
     */
    if ((INIC_NIEVENT_MPR_CHANGED == nievent) &&
        vmsv.cfg_ptr->msval_event_fptr)
    {
        if((MSVAL_S_ON == state) && (MNS_FALSE != reported))
        {
            TAKE_VMSV();
            vmsv.shadow.delayed_mpr = MNS_FALSE;
            GIVE_VMSV();

            vmsv.cfg_ptr->msval_event_fptr(MSVAL_E_MPR, NULL);
        }
        else
        {
            TAKE_VMSV();
            vmsv.shadow.delayed_mpr = MNS_TRUE;
            GIVE_VMSV();
        }
    }
    else if ((INIC_NIEVENT_CRITICAL_UNLOCK == nievent) &&
             vmsv.cfg_ptr->msval_error_fptr)
    {
        vmsv.cfg_ptr->msval_error_fptr(MSVAL_ERR_UNLOCK_CRITICAL, NULL);
    }
    else if ((INIC_NIEVENT_SHORT_UNLOCK == nievent) && vmsv.cfg_ptr->msval_error_fptr)
    {
        vmsv.cfg_ptr->msval_error_fptr(MSVAL_ERR_UNLOCK_SHORT, NULL);
    }
    else if ((INIC_NIEVENT_NET_ACTIVITY == nievent) && vmsv.cfg_ptr->msval_event_fptr)
    {
        vmsv.cfg_ptr->msval_event_fptr(MSVAL_E_NET_ACTIVITY, NULL);
    }
    else if (INIC_NIEVENT_TIMEOUT_CFG_STATUS == nievent)
    {
        if ((NCS_INVALID_NWM != vmsv.shadow.nwm_addr) && vmsv.update_nwm_address_fptr)
        {
            vmsv.update_nwm_address_fptr(vmsv.shadow.nwm_addr);
            if(NCS_NOT_OK == MostGetNCState())
            {
                (void)VmsvInjectFBlockIDsGet();
            }
        }
        if (vmsv.store_error_info2_fptr)
        {
            vmsv.store_error_info2_fptr(INIC_NIEVENT_TIMEOUT_CFG_STATUS);
            #ifdef VMSV_101
            VmsvSendMdmMsv2NwmConfStateNok();
            #endif
        }
    }

    T_LIB_EXIT(VMSV_27);
}
#endif

/*! Delayed callback processing for INIC.NCEDelayed.Status. Runs only during
  * attached mode.
  *
  * @author     SSemmler
  * @updated       4/6/2005
  * @version    2.0.3
  * @see        VmsvFireCallbacks
  * @see        VmsvHandleNCEDelayedStatus
  */
#ifdef VMSV_28
static void VmsvHandleNCEDelayedChange(void)
{
    byte ncedelayed[2];
    byte state;
    byte info[3];
    bool reported;

    T_MOD_ENTRY(VMSV_28);

    info[0] = (byte)0;
    info[1] = (byte)0;
    info[2] = (byte)0;

    TAKE_VMSV();
    ncedelayed[0] = vmsv.shadow.ncedelayed[0];
    ncedelayed[1] = vmsv.shadow.ncedelayed[1];
    state = vmsv.shadow.nistate;
    reported = vmsv.shadow.neton_reported;
    GIVE_VMSV();

    ASSERT((INIC_SHADOW_INVALID_BYTE != ncedelayed[0]) &&
           (INIC_SHADOW_INVALID_BYTE != ncedelayed[1]));

    /* "net on" needs to be already reported, before MSVAL_E_MPR* can be */
    if (vmsv.cfg_ptr->msval_event_fptr)
    {
        if ((MSVAL_S_ON == state) && (MNS_FALSE != reported))
        {
            byte event;

            TAKE_VMSV();
            vmsv.shadow.delayed_nce = MNS_FALSE;
            GIVE_VMSV();

            info[0] = (byte)2;
            info[1] = ncedelayed[0];                /* ... new MPR */
            info[2] = ncedelayed[1];                /* ... old MPR */

            if (info[1] == info[2])
            {
                info[0] = (byte)1;                  /* length = 1 */
                event   = MSVAL_E_MPRDEL_EQUAL;
            }
            else
            {
                event = (info[1] < info[2]) ?
                        MSVAL_E_MPRDEL_DEC : MSVAL_E_MPRDEL_INC;
            }

            vmsv.cfg_ptr->msval_event_fptr(event, info);
        }
        else
        {
            TAKE_VMSV();
            vmsv.shadow.delayed_nce = MNS_TRUE;
            GIVE_VMSV();
        }
    }

    T_MOD_EXIT(VMSV_28);
}
#endif

/*! Delayed callback processing for INIC.LockState.Status. Runs only during
  * attached mode.
  *
  * @author     SSemmler
  * @updated       4/7/2005
  * @version    2.0.3
  * @see        VmsvFireCallbacks
  * @see        VmsvHandleLockStateStatus
  */
#ifdef VMSV_29
static void VmsvHandleLockStateChange(void)
{
    byte event;

    T_MOD_ENTRY(VMSV_29);

    TAKE_VMSV();
    switch (vmsv.shadow.lockstate)          /* map lockstate to msval events */
    {
        case INIC_LOCKSTATE_UNLOCK:
            event = MSVAL_E_UNLOCK;
            break;

        case INIC_LOCKSTATE_STABLELOCK:
            event = MSVAL_E_LOCKSTABLE;
            break;

        default:
            event = vmsv.shadow.lockstate;
            break;
    }
    GIVE_VMSV();

    ASSERT(INIC_SHADOW_INVALID_BYTE != event);

    if (vmsv.cfg_ptr->msval_event_fptr)
    {
        vmsv.cfg_ptr->msval_event_fptr(event, NULL);
    }
    T_MOD_EXIT(VMSV_29);
}
#endif

/*! Delayed callback processing for INIC.RBDResult.Status. Runs only during
  * attached mode.
  *
  * @author     SSemmler
  * @updated       4/7/2005
  * @version    2.0.3
  * @see        VmsvFireCallbacks
  * @see        VmsvHandleRBDResultStatus
  */
#ifdef VMSV_30
static void VmsvHandleRBDResultChange(void)
{
    byte rbdresult[2];

    T_MOD_ENTRY(VMSV_30);

    TAKE_VMSV();
    rbdresult[0] = vmsv.shadow.rbdresult[0];
    rbdresult[1] = vmsv.shadow.rbdresult[1];
    GIVE_VMSV();

    ASSERT((INIC_SHADOW_INVALID_BYTE != rbdresult[0]) &&
           (INIC_SHADOW_INVALID_BYTE != rbdresult[1]));

    if ((byte)0xFF != rbdresult[0])
    {
        if (vmsv.cfg_ptr->msval_diag_result_fptr)
        {
            byte info[2];

            if (MSVAL_DIAG_POS == rbdresult[0])
            {
                info[0] = (byte)0x01;
                info[1] = rbdresult[1];
            }
            else
            {
                info[0] = (byte)0x00;
            }
            vmsv.cfg_ptr->msval_diag_result_fptr(rbdresult[0], info);
        }
    }

    #ifdef MSV_DIAG_RESULT_MSG
    VmsvCommRBDResult(rbdresult);
    #endif


    T_MOD_EXIT(VMSV_30);
}
#endif

/*! Delivers the current MPR.
  *
  * @author     SSemmler
  * @updated       5/10/2005
  * @version    2.0.3
  *
  * @return     The current MPR.
  */
#ifdef VMSV_31
byte MostGetMaxPos(void)
{
    byte result;

    T_API_ENTRY(VMSV_31);
    TAKE_VMSV();
    result = vmsv.shadow.ncedelayed[0];
    GIVE_VMSV();
    T_API_EXIT(VMSV_31);

    return(result);
}
#endif

/*! SMSC internal function to register hooks for Layer2. Undocumented API - no
  * compatibility garanteed. Do not use this function in your application!
  *
  * @author     SSemmler
  * @updated       9/11/2005
  * @version    2.0.4.Alpha.1
  *
  * @param      cbs_ptr points to an instance of TVmsvInternalHooks, containing
  *             callback definitions used by Layer2.
  */
#ifdef VMSV_32
void VmsvRegisterInternalHooks(TVmsvInternalHooks *cbs_ptr)
{
    T_LIB_ENTRY(VMSV_32);
    ASSERT(cbs_ptr);
    TAKE_VMSV();
    vmsv.update_nwm_address_fptr = cbs_ptr->update_nwm_address_fptr;
    vmsv.store_error_info2_fptr = cbs_ptr->store_error_info2_fptr;
    GIVE_VMSV();
    T_LIB_EXIT(VMSV_32);
}
#endif

#ifdef VMSV_33
/*! @function   VmsvGetNCStateShutdown
  * @abstract   Check for the NCS_SHUTDOWN bit in the NCFlags parameter of
  *             INIC.NCState.Status stored in the shadow.
  * @discussion If the INIC received an NetBlock.ShutDown.Start(DeviceShutdown)
  *             and has not yet seen a MNS_NET_ON or
  *             NetBlock.ShutDown.Start(WakeFromDeviceShutDown) since, this
  *             function returns MNS_TRUE.
  *
  * @author     SSemmler
  * @updated    2005-10-04
  * @version    2.0.4
  *
  * @return     MNS_TRUE if the mns.config.general.wakeup_query_fptr shall be
  *             called.
  */
bool VmsvGetNCStateShutdown(void)
{
    T_LIB_ENTRY(VMSV_33);
    T_LIB_EXIT(VMSV_33);

    return((NCS_SHUTDOWN_MASK & vmsv.shadow.ncstate_flags) ? MNS_TRUE : MNS_FALSE);
}
#endif

#ifdef VMSV_34
/*! @function   VmsvHandleConfigStatus
  * @abstract   Internal helper function to handle the
  *             NetworkMaster.Configuration.Status message.
  * @discussion In attached mode this function updates the vmsv.shadow.ncstate
  *             and vmsv.shadow.nwm_addr shadow variables. In Semi-Protected
  *             mode VmsvHandleNCStateStatus() is responsible.
  *
  * @author     SSemmler
  * @updated    2005-10-23
  * @version    2.0.4
  *
  * @param      msg_ptr points to the message received from the NetworkMaster
  */
void VmsvHandleConfigStatus(TMsgRx *msg_ptr)
{
    T_LIB_ENTRY(VMSV_34);

    ASSERT(msg_ptr);

    if (EHCISTATE_IS_ATTACHED())
    {
        MostClearTimer(&(vmsv.check_phase.timer));

        TAKE_VMSV();
        vmsv.shadow.ncstate  = msg_ptr->Data[0] ? NCS_OK : NCS_NOT_OK;
        vmsv.shadow.nwm_addr = msg_ptr->Src_Adr;
        GIVE_VMSV();

        VmsvHandleNCStateChange();
    }

    T_LIB_EXIT(VMSV_34);
}
#endif

#ifdef VMSV_35
/*! @function   VmsvHandleNCStateChange
  * @abstract   Internal helper function to handle the change of the network
  *             configuration received either by INIC.NCState or NWM.Config.
  *
  * @author     SSemmler
  * @updated    2005-10-23
  * @version    2.0.4
  */
static void VmsvHandleNCStateChange(void)
{
    bool inject;

    T_MOD_ENTRY(VMSV_35);

    inject = MNS_FALSE;

    TAKE_VMSV();
    if ( ((VMSV_CP_WAITING == vmsv.check_phase.state) ||
         ((VMSV_CP_DO_NOTHING == vmsv.check_phase.state) &&
         (MNS_FALSE == vmsv.fblock_ids_status_sent) && (MNS_FALSE != vmsv.attached))) &&
         (NCS_OK == vmsv.shadow.ncstate) )
    {
        inject = MNS_TRUE;
        vmsv.check_phase.state = VMSV_CP_INJECTED;
    }
    GIVE_VMSV();

    if (MNS_FALSE != inject)
    {
        (void)VmsvInjectFBlockIDsGet();
    }

    T_MOD_EXIT(VMSV_35);
}
#endif

#ifdef VMSV_36
/*! @function   VmsvHandleFBlockIDsGet
  * @abstract   Internal helper function to cancel the NCState check phase if
  *             NB.FBlockIDs.Get was received.
  *
  * @author     SSemmler
  * @updated    2005-10-23
  * @version    2.0.4
  */
void VmsvHandleFBlockIDsGet(void)
{
    T_LIB_ENTRY(VMSV_36);

    TAKE_VMSV();
    if (VMSV_CP_WAITING == vmsv.check_phase.state)
    {
        vmsv.check_phase.state = VMSV_CP_DO_NOTHING;
    }
    vmsv.fblock_ids_get_received = MNS_TRUE;
    GIVE_VMSV();

    T_LIB_EXIT(VMSV_36);
}
#endif

#ifdef VMSV_37
/*! @function   VmsvInjectFBlockIDsGet
  * @abstract   Internal helper function to inject an NB.FF.FBlockIDs.Get
  *             message.
  *
  * @author     SSemmler
  * @updated    2005-10-24
  * @version    2.0.4
  *
  * @return     The standard error mask (ERR_*). Relevant for the retries only.
  */
static byte VmsvInjectFBlockIDsGet(void)
{
    byte result;
    TMsgRx *msg_ptr;

    T_MOD_ENTRY(VMSV_37);

    result = ERR_NO;

    msg_ptr = MsgGetRxInPtrExt(0);
    if (msg_ptr)
    {
        TAKE_VMSV();
        msg_ptr->Src_Adr   = vmsv.shadow.nwm_addr;
        GIVE_VMSV();
        msg_ptr->FBlock_ID = FBLOCK_NETBLOCK;
        msg_ptr->Inst_ID   = (byte)0xFF;
        msg_ptr->Func_ID   = FUNC_FBLOCKIDS;
        msg_ptr->Operation = OP_GET;
        msg_ptr->Length    = (word)0;

        MsgRxInReady(msg_ptr);
    }
    else
    {
        SCHEDULE_RETRY(VMSV_RETRY_FBLOCKIDS_GET);
        result = ERR_BUFOV;
    }

    T_MOD_EXIT(VMSV_37);
    return(result);
}
#endif

#ifdef VMSV_38
/*! @function   VmsvRetry
  * @abstract   Retry handler for the vMSV module ("on buffer freed" event).
  *
  * @author     SSemmler
  * @updated    2005-10-24
  * @version    2.0.4
  *
  * @return     ERR_NO or ERR_BUFOV (retry is scheduled)
  */
static byte VmsvRetry(void)
{
    word retry;
    byte result;

    T_MOD_ENTRY(VMSV_38);

    retry  = (word)0;
    result = ERR_NO;

    TAKE_EVENTS();
    retry = vmsv.retry;
    GIVE_EVENTS();

    PROCESS_RETRY    (VMSV_RETRY_FBLOCKIDS_GET,         VmsvInjectFBlockIDsGet        );
    PROCESS_RETRY    (VMSV_RETRY_CFG_STATE,             VmsvInjectCfgState            );
    PROCESS_RETRY    (VMSV_RETRY_SYSERRMONITOR,         VmsvRequestSysErrMonitor      );
#ifdef VMSV_69
    PROCESS_RETRY    (VMSV_RETRY_RBDRESULT,             VmsvBroadcastRBDResult        );
#endif

    PROCESS_RETRY    (VMSV_RETRY_REQUEST_NETONTIMER,    VmsvRequestNetOnTime          );
#ifdef MSV_DIAG_RESULT_MSG
    PROCESS_RETRY    (VMSV_RETRY_REQUEST_NIWAKEUPMODE,  VmsvRequestNIWakeUpMode       );
    PROCESS_RETRY    (VMSV_RETRY_SET_NIWAKEUPMODE,      VmsvSetNIWakeUpMode           );
#endif

#ifdef VMSV_95
    PROCESS_RETRY    (VMSV_RETRY_PLT,                   VmsvPhysicalLayerTestProcedure);
#endif
#ifdef VMSV_99
    PROCESS_RETRY    (VMSV_RETRY_SSORESULT,             VmsvResetINICSSOResult        );
#endif

    T_MOD_EXIT(VMSV_38);

    return(result);
}
#endif

#ifdef VMSV_39
/*! @function   VmsvTxFilter
  * @abstract   Special Tx Filter to intercept empty FBlock list in certain
  *             situations.
  *
  * @author     SSemmler
  * @updated    2005-10-24
  * @version    2.0.4
  *
  * @return     MSG_TX_FILTER_DEFAULT or MSG_TX_FILTER_CANCEL
  */
byte VmsvTxFilter(TMsgTx *msg_ptr)
{
    byte result;
    bool check;
    bool inject;

    T_LIB_ENTRY(VMSV_39);

    result = MSG_TX_FILTER_DEFAULT;
    check  = MNS_FALSE;
    inject = MNS_FALSE;

    ASSERT(msg_ptr);

    if ((FBLOCK_NETBLOCK == msg_ptr->FBlock_ID) &&
        (FUNC_FBLOCKIDS  == msg_ptr->Func_ID)   &&
        (OP_STATUS       == msg_ptr->Operation))
    {
        TAKE_VMSV();

        vmsv.fblock_ids_status_sent = MNS_TRUE;
        if (VMSV_CP_INJECTED == vmsv.check_phase.state)
        {
            vmsv.check_phase.state = VMSV_CP_DO_NOTHING;

            if (0 == msg_ptr->Length)
            {
                result = MSG_TX_FILTER_CANCEL;
                inject = MNS_TRUE;
            }
        }

        GIVE_VMSV();

        if (MNS_FALSE != inject)
        {
            VmsvInjectCfgState();
        }
    }
    else if ((FBLOCK_NETWORKMASTER      == msg_ptr->FBlock_ID) &&
             (FUNCID_NWM_CONFIGURATION  == msg_ptr->Func_ID)   &&
             (OP_STATUS                 == msg_ptr->Operation) )
        {
            TAKE_VMSV();
            if (NISTATE_NET_ON == vmsv.shadow.nistate)
            {
                vmsv.shadow.ncstate  = (0x00 != msg_ptr->Data[0]) ? NCS_OK : NCS_NOT_OK;
            }
            GIVE_VMSV();
        }
    else
    {
        TAKE_VMSV();
        check = ((NCS_INVALID_NWM != vmsv.shadow.nwm_addr) &&
                 (NCS_NOT_OK == vmsv.shadow.ncstate)) ? MNS_TRUE : MNS_FALSE;
        GIVE_VMSV();

        if( (MNS_FALSE != check) && (FBLOCK_NETWORKMASTER == msg_ptr->FBlock_ID) &&
                               (FUNCID_NWM_CONFIGURATION  == msg_ptr->Func_ID)   &&
                               (OP_GET                    == msg_ptr->Operation))
        {
            (void)VmsvInjectCfgState();
            result = MSG_TX_FILTER_CANCEL;
        }
    }

    T_LIB_EXIT(VMSV_39);

    return(result);
}
#endif

#ifdef VMSV_40
/*! @function   VmsvRxFilter
  * @abstract   Special Rx Filter to intercept NB.FBlockIDs.Get and
  *             NWM.Configuration.Status messages.
  *
  * @author     SSemmler
  * @updated    2005-10-24
  * @version    2.0.4
  */
void VmsvRxFilter(TMsgRx *msg_ptr)
{
    T_LIB_ENTRY(VMSV_40);

    ASSERT(msg_ptr);

    if ((FBLOCK_NETWORKMASTER == msg_ptr->FBlock_ID) &&
        (FUNCID_NWM_CONFIGURATION == msg_ptr->Func_ID) &&
        (OP_STATUS == msg_ptr->Operation))
    {
        VmsvHandleConfigStatus(msg_ptr);
    }
    else if ((FBLOCK_NETBLOCK == msg_ptr->FBlock_ID) &&
             (FUNC_FBLOCKIDS == msg_ptr->Func_ID) &&
             (OP_GET == msg_ptr->Operation))
    {
        VmsvHandleFBlockIDsGet();
    }

    T_LIB_EXIT(VMSV_40);
}
#endif

#ifdef VMSV_41
/*! @function   VmsvHandleNWStartupError
  * @abstract   Internal helper function to parse the incoming
  *             INIC.NWStartup.Error message.
  *
  * @author     SSemmler
  * @updated    2005-10-25
  * @version    2.0.4
  *
  * @param      msg_ptr holds the telegram
  */
void VmsvHandleNWStartupError(TMsgRx *msg_ptr)
{
    T_LIB_ENTRY(VMSV_41);

    ASSERT(msg_ptr);

    if (vmsv.cfg_ptr->msval_error_fptr)
    {
        if (ERR_BUSY == msg_ptr->Data[0])
        {
            vmsv.cfg_ptr->msval_error_fptr(MSVAL_ERR_STARTUP_BUSY, NULL);
        }
        else
        {
            vmsv.cfg_ptr->msval_error_fptr(MSVAL_ERR_STARTUP_FAILED, NULL);
        }
    }

    T_LIB_EXIT(VMSV_41);
}
#endif

#ifdef VMSV_42
/*! @function   VmsvHandleNWShutdownError
  * @abstract   Internal helper function to parse the incoming
  *             INIC.NWShutdown.Error message.
  *
  * @author     SSemmler
  * @updated    2005-10-25
  * @version    2.0.4
  *
  * @param      msg_ptr holds the telegram
  */
void VmsvHandleNWShutdownError(TMsgRx *msg_ptr)
{
    T_LIB_ENTRY(VMSV_41);

    ASSERT(msg_ptr);

    if (vmsv.cfg_ptr->msval_error_fptr)
    {
        if (ERR_BUSY == msg_ptr->Data[0])
        {
            vmsv.cfg_ptr->msval_error_fptr(MSVAL_ERR_SHUTDOWN_BUSY, NULL);
        }
        else
        {
            vmsv.cfg_ptr->msval_error_fptr(MSVAL_ERR_SHUTDOWN_FAILED, NULL);
        }
    }

    T_LIB_EXIT(VMSV_41);
}
#endif

#ifdef VMSV_45
/*! @function   VmsvCheckForNpr
  * @abstract   This internal helper function checks if the Node Position
  *             changed and calls the msval_event_fptr with MSVAL_E_NPR.
  *
  * @author     SSemmler
  * @updated    2005-10-26
  * @version    2.0.4
  */
static void VmsvCheckForNpr(bool force)
{
    byte old_val;
    byte new_val;

    TAKE_VMSV();
    old_val = (byte) ((vmsv.shadow.node_pos_check & 0xFF00) >> 8);
    new_val = (byte) (vmsv.shadow.node_pos_check & 0x00FF);
    vmsv.shadow.node_pos_check &= (word) 0x00FF;
    vmsv.shadow.node_pos_check |= (word)(new_val << 8);
    GIVE_VMSV();

    if ((new_val != old_val) || (MNS_FALSE != force))
    {
        #ifdef NS_MSV_NB
        NbRefreshNodePos();
        #endif

        if(vmsv.cfg_ptr->msval_event_fptr)
        {
            byte info[2];
            info[0] = (byte)1;
            info[1] = MostGetNodePos();
            vmsv.cfg_ptr->msval_event_fptr(MSVAL_E_NPR, info);
        }
    }
}
#endif

#ifdef VMSV_46
/*! @function   VmsvReportNetOn
  * @abstract   Internal helper function to report MSVAL_E_NETON as soon as
  *             the boundary is valid. Reports also MSVAL_E_MPR* if they where
  *             received before this function was actually called.
  * @discussion The effort this function is part of, is to delay MSVAL_E_NETON
  *             till a valid boundary information was received from the INIC.
  *             Additionally the MSVAL_E_MPR* events need to be delayed and
  *             reported after MSVAL_E_NETON.
  *
  * @author     SSemmler
  * @updated    2005-10-26
  * @version    2.0.4
  */
static void VmsvReportNetOn(void)
{
    byte info[3];
    bool delayed_mpr;
    bool delayed_nce;

    T_MOD_ENTRY(VMSV_46);

    info[0] = (byte)1;
    info[1] = (byte)0;
    info[2] = (byte)0;

    TAKE_VMSV();
    info[1]                    = vmsv.shadow.devmode;
    vmsv.shadow.neton_reported = MNS_TRUE;
    delayed_mpr                = vmsv.shadow.delayed_mpr;
    vmsv.shadow.delayed_mpr    = MNS_FALSE;
    delayed_nce                = vmsv.shadow.delayed_nce;
    vmsv.shadow.delayed_nce    = MNS_FALSE;
    GIVE_VMSV();

    if (vmsv.cfg_ptr->msval_event_fptr)
    {
        vmsv.cfg_ptr->msval_event_fptr(MSVAL_E_NETON, info);

        if (MNS_FALSE != delayed_mpr)
        {
            vmsv.cfg_ptr->msval_event_fptr(MSVAL_E_MPR, NULL);
        }
        if (MNS_FALSE != delayed_nce)
        {
            VmsvHandleNCEDelayedChange();
        }
    }

    T_MOD_EXIT(VMSV_46);
}
#endif

#ifdef VMSV_47
/*! @function   VmsvHandlePMIStateStatus
  * @abstract   Internal helper function to parse the INIC.PMIState.Status
  *             messages.
  *
  * @author     SSemmler
  * @updated    2005-10-26
  * @version    2.0.4
  *
  * @param      msg_ptr holds the respective message
  */
void VmsvHandlePMIStateStatus(TMsgRx *msg_ptr)
{
    T_LIB_ENTRY(VMSV_47);

    ASSERT(msg_ptr);

    TAKE_VMSV();
    vmsv.shadow.changed    |= VMSV_SHADOW_PMISTATE_CHANGED;
    vmsv.shadow.pmi.state   = msg_ptr->Data[0];
    vmsv.shadow.pmi.events  = msg_ptr->Data[1];
    GIVE_VMSV();

    if (EHCISTATE_IS_ATTACHED())
    {
        VmsvSetPendingEvent(VMSV_P_SHADOW_CHANGE);
    }
    else
    {
        MnsNtfCheck(NTF_PMISTATE);
    }

    T_LIB_EXIT(VMSV_47);
}
#endif

#ifdef VMSV_48
/*! @function   VmsvHandlePMIStateChange
  * @abstract   Delayed calling of the callback related to the
  *             INIC.PMIState.Status message.
  *
  * @author     SSemmler
  * @updated    2005-10-26
  * @version    2.0.4
  */
static void VmsvHandlePMIStateChange(void)
{
    T_MOD_ENTRY(VMSV_48);

    if (vmsv.cfg_ptr->pmistate_changed_fptr)
    {
        byte state;
        byte events;

        TAKE_VMSV();
        state = vmsv.shadow.pmi.state;
        events = vmsv.shadow.pmi.events;
        GIVE_VMSV();

        vmsv.cfg_ptr->pmistate_changed_fptr(state, events);
    }

    T_MOD_EXIT(VMSV_48);
}
#endif

#ifdef VMSV_49
/*! @function   MostGetPMIState
  * @abstract   The state and events provided by vmsv.pmistate_changed_fptr
  *             will also be stored in shadow variables and therefore be
  *             available through the following API function..
  *
  * @author     SSemmler
  * @updated    2005-10-26
  * @version    2.0.4
  *
  * @param      state_ptr points to the byte that shall hold the state
  * @param      events_ptr points to the byte that shall hold the events
  * @return     The standard error mask (ERR_*).
  */
byte MostGetPMIState(byte *state_ptr, byte *events_ptr)
{
    byte result;

    T_API_ENTRY(VMSV_49);

    result = ERR_NO;

    if ((NULL == state_ptr) || (NULL == events_ptr))
    {
        result = ERR_PARAM;
    }
    else
    {
        TAKE_VMSV();
        *state_ptr = vmsv.shadow.pmi.state;
        *events_ptr = vmsv.shadow.pmi.events;
        GIVE_VMSV();

        if ((INIC_SHADOW_INVALID_BYTE == *state_ptr) ||
            (INIC_SHADOW_INVALID_BYTE == *events_ptr))
        {
            result = ERR_INVALID_SHADOW;
        }
    }

    T_API_EXIT(VMSV_49);

    return(result);
}
#endif






#ifdef VMSV_54
/*! @function   MostGetSysErrMonitor
  * @abstract   Delivers error information stored in the INIC. The call of
  *             this function resets the information.
  * @discussion The actual delivery is realized by the callback cb_ptr, since
  *             there has to be some communication with the INIC before the
  *             request can be fulfilled.
  *
  * @author     SSemmler
  * @updated    2005-11-01
  * @version    2.0.4
  *
  * @param      cb_ptr is the function pointer to the standard callback provided
  *             by the caller / application.
  * @return     ERR_NO, ERR_PARAM or ERR_BUFOV (busy)
  */
byte MostGetSysErrMonitor(TMnsStdCB *cb_ptr)
{
    byte result;

    T_API_ENTRY(VMSV_54);

    result = ERR_NO;

    if (cb_ptr)
    {
        TAKE_VMSV();
        if (NULL == vmsv.sys_err_monitor.cb_ptr)
        {
            vmsv.sys_err_monitor.cb_ptr = cb_ptr;
            GIVE_VMSV();

            (void)VmsvRequestSysErrMonitor();
        }
        else
        {
            GIVE_VMSV();
            result = ERR_BUFOV;
        }
    }
    else
    {
        result = ERR_PARAM;
    }

    T_API_EXIT(VMSV_54);
    return(result);
}
#endif

#ifdef VMSV_55
/*! @function   VmsvRequestSysErrMonitor
  * @abstract   Internal helper function to send the request
  *             INIC.SysErrMonitor.Get message to the INIC.
  *
  * @author     SSemmler
  * @updated    2005-11-01
  * @version    2.0.4
  *
  * @return     ERR_NO or ERR_BUFOV (retry is scheduled)
  */
static byte VmsvRequestSysErrMonitor(void)
{
    byte result;
    TMsgTx *msg_ptr;

    T_MOD_ENTRY(VMSV_55);

    result  = ERR_NO;
    msg_ptr = MsgGetTxPtrExt(0);
    if (msg_ptr)
    {
        msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
        msg_ptr->FBlock_ID = FBLOCK_INIC;
        msg_ptr->Func_ID   = FUNCID_INIC_SYSERRMONITOR;
        msg_ptr->Operation = OP_GET;

        MsgSend3(msg_ptr);
    }
    else
    {
        SCHEDULE_RETRY(VMSV_RETRY_SYSERRMONITOR);
        result = ERR_BUFOV;
    }

    T_MOD_EXIT(VMSV_55);
    return(result);
}
#endif

#ifdef VMSV_56
/*! @function   VmsvHandleSysErrMonitorStatus
  * @abstract   Internal helper function to process the
  *             INIC.SysErrMonitor.Status message.
  * @discussion In case there is no error, the application callback will be
  *             called inside this function with the code NSR_EM_NONE. If the
  *             bit mask sent by the status message contains errors, the event
  *             VMSV_P_REPORT_EM will be fired. The VMSV service will then
  *             report one of the errors and refire the event as long as it
  *             takes to report all of them.
  *
  * @author     SSemmler
  * @updated    2005-11-01
  * @version    2.0.4
  *
  * @param      msg_ptr will point to the INIC.SysErrMonitor.Status message.
  */
void VmsvHandleSysErrMonitorStatus(TMsgRx *msg_ptr)
{
    TMnsStdCB *cb_ptr;

    T_LIB_ENTRY(VMSV_56);

    ASSERT(msg_ptr);

    TAKE_VMSV();
    cb_ptr = vmsv.sys_err_monitor.cb_ptr;
    GIVE_VMSV();

    if (cb_ptr)
    {
        byte bit_mask = msg_ptr->Data[0];

        if (bit_mask)
        {
            TAKE_VMSV();
            vmsv.sys_err_monitor.bit_mask = bit_mask;
            GIVE_VMSV();

            VmsvSetPendingEvent(VMSV_P_REPORT_EM);
        }
        else
        {
            TAKE_VMSV();
            vmsv.sys_err_monitor.bit_mask = (byte)0;
            vmsv.sys_err_monitor.cb_ptr   = NULL;
            GIVE_VMSV();

            cb_ptr(NSR_EM_NONE);
        }
    }

    T_LIB_EXIT(VMSV_56);
}
#endif

#ifdef VMSV_57
/*! @function   VmsvReportSysErrMonitor
  * @abstract   Reports one event from the system error monitor.
  * @discussion The function looks into the system error bit mask, reports
  *             one of its bits and if any are left reschedules the
  *             VMSV_P_REPORT_EM event. When all errors were reported, this
  *             function is called a last time to reset the members of the
  *             vmsv.sys_err_monitor structure.
  *
  * @author     SSemmler
  * @updated    2005-11-01
  * @version    2.0.4
  */
static void VmsvReportSysErrMonitor(void)
{
    byte        bit_mask;
    TMnsStdCB  *cb_ptr;

    T_MOD_ENTRY(VMSV_57);

    TAKE_VMSV();
    bit_mask = vmsv.sys_err_monitor.bit_mask;
    cb_ptr   = vmsv.sys_err_monitor.cb_ptr;
    GIVE_VMSV();

    if (bit_mask && cb_ptr)
    {
        byte pos = (byte)0;

        while (!(bit_mask & (1 << pos)) && pos < (byte)8)
        {
            pos++;
        }

        if (pos < 8)
        {
            byte event = (byte)(1 << pos);

            cb_ptr(NSR_BUILD(MNS_FALSE, NSR_CODE_SYS_ERR_MONITOR, event));

            TAKE_VMSV();
            vmsv.sys_err_monitor.bit_mask &= ~event;
            GIVE_VMSV();
        }

        VmsvSetPendingEvent(VMSV_P_REPORT_EM);
    }
    else
    {
        TAKE_VMSV();
        vmsv.sys_err_monitor.cb_ptr   = NULL;
        GIVE_VMSV();
    }

    T_MOD_EXIT(VMSV_57);
}
#endif

#ifdef VMSV_59
/*! @function   VmsvInjectCfgState
  * @abstract   Internal helper function to inject the
  *             NWM.Configuration.Status telegram.
  *
  * @author     SSemmler
  * @updated    2005-12-20
  * @version    2.0.x-SR-10
  *
  * @return     standard error mask (for retry handling only).
  */
static byte VmsvInjectCfgState(void)
{
    byte result;
    TMsgRx *msg_ptr;

    T_MOD_ENTRY(VMSV_59);

    result = ERR_NO;
    msg_ptr = MsgGetRxInPtrExt(1);

    if (msg_ptr)
    {
        TAKE_VMSV();
        msg_ptr->Src_Adr   = vmsv.shadow.nwm_addr;
        if (NCS_NWMINST_MASK & vmsv.shadow.ncstate_flags)
        {
            /* InstID is not 0x01 */
            msg_ptr->Inst_ID   = VMSV_NWM_INST_ID;
        }
        else
        {
            /* InstID is 0x01 or NCFlags is not supported */
            msg_ptr->Inst_ID   = VMSV_NWM_STD_INST_ID; /* 0x01 */
        }
        msg_ptr->FBlock_ID = FBLOCK_NETWORKMASTER;
        msg_ptr->Func_ID   = FUNCID_NWM_CONFIGURATION;
        msg_ptr->Operation = OP_STATUS;
        msg_ptr->Data[0]   = vmsv.shadow.ncstate;
        GIVE_VMSV();

        MsgRxInReady(msg_ptr);
    }
    else
    {
        SCHEDULE_RETRY(VMSV_RETRY_CFG_STATE);
        result = ERR_BUFOV;
    }

    T_MOD_EXIT(VMSV_59);
    return(result);
}
#endif

#ifdef VMSV_60
/*! @function   VmsvEHCIGoSemiProtected
  * @abstract   Internal function called when the Semi-Protected Mode is
  *             entered.
  *
  * @author     SSemmler
  * @updated    2006-01-27
  * @version    2.1.0
  */
static void VmsvEHCIGoSemiProtected(void)
{
    T_MOD_ENTRY(VMSV_60);

    #ifdef MSV_DIAG_RESULT_MSG
    if(MNS_MASTER == vmsv.shadow.devmode)
    {
        (void)VmsvRequestNIWakeUpMode();
    }
    #endif

    T_MOD_EXIT(VMSV_60);
}
#endif



#ifdef VMSV_66
/*! @function   VmsvCommRBDResult
  * @abstract   Helper function to communicate the result of the
  *             ring break diagnosis.
  * @discussion If a position was detected the handling depends on wheter the
  *             position is zero or greater. The sub-functions
  *             VmsvCommRBDResultPos0() and VmsvCommRBDResultPosX() handle the
  *             respective case.
  *
  * @author     SSemmler
  * @updated    2006-01-30
  * @version    2.1.0
  *
  * @param      result is an array containing the two result bytes communicated
  *                    by INIC.RBDResult.Status.
  */
static void VmsvCommRBDResult(byte result[2])
{
    T_MOD_ENTRY(VMSV_66);

    /* pos detected ? */
    if (MSVAL_DIAG_POS == result[0])
    {
        /* pos 0 or X ? */
        if(0 == result[1])
        {
            VmsvCommRBDResultPos0(VMSV_COMM_CALL_CB);
        }
        else
        {
            VmsvCommRBDResultPosX();
        }
    }
    else if (MSVAL_DIAG_SIGNAL_NO_LOCK == result[0])
    {
        VmsvCommRBDResultPos0(VMSV_COMM_CALL_CB);
    }


    T_MOD_EXIT(VMSV_66);
}
#endif

#ifdef VMSV_67
/*! @function   VmsvCommRBDResultPos0
  * @abstract   Helper function to communicate the result of the
  *             ring break diagnosis if the detected position is zero.
  *
  * @author     SSemmler
  * @updated    2006-01-30
  * @version    2.1.0
  *
  * @param      go_to_state is the state the state machine shall go to.
  */
static void VmsvCommRBDResultPos0(byte go_to_state)
{
    bool go_on;
    byte nistate;

    T_MOD_ENTRY(VMSV_67);

    go_on = MNS_FALSE;

    TAKE_VMSV();
    nistate = vmsv.shadow.nistate;

    /* check if the transition is ok */
    if(   (go_to_state == (vmsv.nwstartup.comm_state+1))
       || (VMSV_COMM_WAIT_AFTER_SEND == go_to_state) )
    {
        vmsv.nwstartup.comm_state = go_to_state;
        go_on = MNS_TRUE;
    }
    else
    {
        FAILED_ASSERT();
    }

    GIVE_VMSV();

    if (MNS_FALSE != go_on)
    {
        word timer = (word)0;

        T_COMM_CHANGE(go_to_state);

        switch (go_to_state)
        {
            case VMSV_COMM_CALL_CB:
                /* call the callback */
                if(vmsv.cfg_ptr->rbd_store_rbd_result_fptr)
                {
                    byte rbd_status;

                    TAKE_VMSV();
                    switch (vmsv.shadow.rbdresult[0])
                    {
                        case MSVAL_DIAG_POS:
                            rbd_status = RBD_NO_ACTIVITY;
                            break;

                        case MSVAL_DIAG_SIGNAL_NO_LOCK:
                            rbd_status = RBD_ACTIVITY_NO_LOCK;
                            break;

                        default:
                            rbd_status = INIC_SHADOW_INVALID_BYTE;
                            FAILED_ASSERT();
                            break;
                    }
                    GIVE_VMSV();

                    vmsv.cfg_ptr->rbd_store_rbd_result_fptr(rbd_status, vmsv.cfg_ptr->diag_id.length, vmsv.cfg_ptr->diag_id.stream);
                }

                /* save and set device mode */
                TAKE_VMSV();
                vmsv.nwstartup.saved_dev_mode = vmsv.shadow.devmode;
                GIVE_VMSV();
                if(MNS_MASTER == vmsv.nwstartup.saved_dev_mode)
                {
                    /* start t_wait_rbd */
                    timer = VMSV_DC_T_WAIT_RBD;
                }
                else
                {
                    MostSetDevMode(MNS_MASTER);
                    /* start t_prepre_rbd */
                    timer = VMSV_DC_T_PREPARE_RBD;
                }
                /* start timer */
                MostSetTimer(&vmsv.nwstartup.comm_timer, timer, MNS_FALSE);
                break;

            case VMSV_COMM_STARTNW:
                /* start the network */
                MostStartUp(MNS_MASTER, MNS_DEFAULT);
                /* start t_rbd */
                MostSetTimer(&vmsv.nwstartup.comm_timer, VMSV_DC_T_RBD, MNS_FALSE);
                break;

            case VMSV_COMM_BROADCAST:
                /* broadcast the message */
                VmsvBroadcastRBDResult();
                /* wait for net_off or start timer to shut the network down */
                if (NISTATE_NET_ON == nistate)
                {
                    MostSetTimer(&vmsv.nwstartup.comm_timer, VMSV_DC_T_RBD, MNS_FALSE);
                }
                break;

            case VMSV_COMM_RESTORE_DEVMODE:
                /* restore the previous device mode */
                MostSetDevMode(vmsv.nwstartup.saved_dev_mode);
                /* stop state machine */
                TAKE_VMSV();
                vmsv.nwstartup.comm_state = VMSV_COMM_OFF;
                GIVE_VMSV();
                T_COMM_CHANGE(VMSV_COMM_OFF);
                break;

            case VMSV_COMM_WAIT_AFTER_SEND:
                /* shut down if still in MNS_NET_ON */
                if (NISTATE_NET_ON == nistate)
                {
                    MostShutDown();
                }
                /* wait for net_off */
                break;


            default:
                FAILED_ASSERT();
        }
    }

    T_MOD_EXIT(VMSV_67);
}
#endif

#ifdef VMSV_68
/*! @function   VmsvCommRBDResultPosX
  * @abstract   Helper function to communicate the result of the
  *             ring break diagnosis if the detected position is greater then
  *             zero.
  *
  * @author     SSemmler
  * @updated    2006-01-30
  * @version    2.1.0
  */
static void VmsvCommRBDResultPosX(void)
{
    bool react;
    bool send_wakeup_mode;

    T_MOD_ENTRY(VMSV_68);

    react            = MNS_FALSE;
    send_wakeup_mode = MNS_FALSE;

    TAKE_VMSV();
    react = (VMSV_COMM_OFF == vmsv.nwstartup.comm_state) ? MNS_TRUE : MNS_FALSE;
    if(MNS_FALSE != react)
    {
        vmsv.nwstartup.comm_state = VMSV_COMM_WAIT_FOR_MSG;
        if (MNS_MASTER == vmsv.nwstartup.saved_dev_mode)
        {
            if (    (INIC_SHADOW_INVALID_BYTE != vmsv.shadow.ni_wakeup_mode.mode)
                 && (VMSV_NIWAKEUPMODE_WAKEUPBAN == (vmsv.shadow.ni_wakeup_mode.mode & VMSV_NIWAKEUPMODE_WAKEUPBAN)) )
            {
                vmsv.shadow.ni_wakeup_mode.request = vmsv.shadow.ni_wakeup_mode.mode & ~VMSV_NIWAKEUPMODE_WAKEUPBAN;
                send_wakeup_mode = MNS_TRUE;
            }
        }
    }
    GIVE_VMSV();
    ASSERT(react);

    if (MNS_FALSE != send_wakeup_mode)
    {
        VmsvSetNIWakeUpMode();
    }

    T_MOD_EXIT(VMSV_68);
}
#endif

#ifdef VMSV_69
/*! @function   VmsvBroadcastRBDResult
  * @abstract   Helper function to communicate the result of the
  *             ring break diagnosis by broadcasting the main FBlock ID.
  *
  * @author     SSemmler
  * @updated    2006-01-31
  * @version    2.1.0
  *
  * @return     standard error mask (for retry handling only).
  */
static byte VmsvBroadcastRBDResult(void)
{
    byte result;
    TMsgTx *msg_ptr;
    byte i;
    byte len;
    byte *diag_id;

    T_MOD_ENTRY(VMSV_69);

    result = ERR_NO;

    TAKE_VMSV();
    diag_id = vmsv.cfg_ptr->diag_id.stream;
    len     = vmsv.cfg_ptr->diag_id.length;
    GIVE_VMSV();

    if (NULL == diag_id)            /* prevent copying from NULL pointer */
    {
        len = (byte)0;
    }
    else if (MNS_FALSE != MostIsSupported(NSF_MOST_150))
    {
        len = (len < CTRL_MAX_PAYLOAD_OS81110 ) ? len : CTRL_MAX_PAYLOAD_OS81110 - 1;
    }
    else
    {
        len = (byte)0;
        FAILED_ASSERT();
    }

    msg_ptr = MsgGetTxPtrExt(1 + len);

    if (msg_ptr)
    {
        msg_ptr->Tgt_Adr   = MSG_TGT_BROADCAST;
        msg_ptr->FBlock_ID = FBLOCK_NETBLOCK;
        msg_ptr->Inst_ID   = (byte)0x00;
        msg_ptr->Func_ID   = FUNC_NB_RBDRESULT;
        msg_ptr->Operation = OP_STATUS;


        TAKE_VMSV();
        switch (vmsv.shadow.rbdresult[0])
        {
            case MSVAL_DIAG_POS:
                msg_ptr->Data[0] = RBD_NO_ACTIVITY;         /* ring break */
                break;

            case MSVAL_DIAG_SIGNAL_NO_LOCK:                 /* activity but no lock */
                msg_ptr->Data[0] = RBD_ACTIVITY_NO_LOCK;
                break;

            default:
                msg_ptr->Data[0] = INIC_SHADOW_INVALID_BYTE;     /* indicates SW Error */
                FAILED_ASSERT();
                break;
        }
        GIVE_VMSV();

        for (i=(byte)1; i <= len; ++i)     /* copy diag_id into message */
        {
            msg_ptr->Data[i] = *diag_id++;
        }

        MsgSend3(msg_ptr);
    }
    else
    {
        SCHEDULE_RETRY(VMSV_RETRY_RBDRESULT);
        result = ERR_BUFOV;
    }

    T_MOD_EXIT(VMSV_69);
    return(result);
}
#endif

#ifdef VMSV_70
/*! @function   VmsvHandleDiagResult
  * @abstract   Message handler function for
  *             NetBlock.RBDOwnMainFBlock.
  *
  * @author     SSemmler
  * @updated    2009-04-06
  * @version    3.0.1
  *
  * @param      msg_ptr points to the received message.
  */
void VmsvHandleDiagResult(TMsgRx *msg_ptr)
{
    bool react;
    bool send_wakeup_mode;

    T_LIB_ENTRY(VMSV_70);

    react            = MNS_FALSE;
    send_wakeup_mode = MNS_FALSE;

    ASSERT(msg_ptr);

    TAKE_VMSV();
    react = (VMSV_COMM_WAIT_FOR_MSG == vmsv.nwstartup.comm_state) ?
            MNS_TRUE : MNS_FALSE;
    if (MNS_FALSE != react)
    {
        vmsv.nwstartup.comm_state = VMSV_COMM_OFF;
    }
    GIVE_VMSV();
    ASSERT(react);

    if (MNS_FALSE != react)
    {
        /* call the callback */
        if(vmsv.cfg_ptr->rbd_store_rbd_result_fptr)
        {
            vmsv.cfg_ptr->rbd_store_rbd_result_fptr(msg_ptr->Data[0], msg_ptr->Length - 1, &(msg_ptr->Data[1]));
        }
        if (MNS_MASTER == vmsv.nwstartup.saved_dev_mode)
        {
            if (    (INIC_SHADOW_INVALID_BYTE != vmsv.shadow.ni_wakeup_mode.mode)
                 && (VMSV_NIWAKEUPMODE_WAKEUPBAN == (vmsv.shadow.ni_wakeup_mode.mode & VMSV_NIWAKEUPMODE_WAKEUPBAN)) )
            {
                vmsv.shadow.ni_wakeup_mode.request = vmsv.shadow.ni_wakeup_mode.mode;
                send_wakeup_mode = MNS_TRUE;
            }
        }

        if (MNS_FALSE != send_wakeup_mode)
        {
            VmsvSetNIWakeUpMode();
        }
    }


    T_LIB_EXIT(VMSV_70);
}
#endif


#ifdef VMSV_71
/*! @function   MostGetNetOnTime
  * @abstract   API function to query the INIC.NetOnTimer property.
  *
  * @author     SSemmler
  * @updated    2006-02-03
  * @version    2.1.0
  *
  * @param      cb_ptr points to the callback the MOST NetServices will call as
  *             soon as the result is available.
  * @return     standard error mask
  */
byte MostGetNetOnTime(TVmsvNetOnCB *cb_ptr)
{
    byte       result;
    TMnsResult code;
    T_API_ENTRY(VMSV_71);

    result = ERR_NO;
    code   = NSR_E_FAILED;

    if(MNS_FALSE == MostIsSupported(NSF_NETONTIMER))
    {
        result = ERR_NOT_SUPPORTED;
        code = NSR_E_NOT_SUPPORTED;
    }
    else if(NULL == cb_ptr)
    {
        result = ERR_PARAM;
    }
    else
    {
        TAKE_VMSV();
        if(!(vmsv.net_on_cb_ptr))
        {
            vmsv.net_on_cb_ptr = cb_ptr;
        }
        else
        {
            result = ERR_BUFOV;
            code = NSR_E_BUSY;
        }
        GIVE_VMSV();
    }

    if(ERR_NO == result)
    {
        (void)VmsvRequestNetOnTime();
    }
    else if(cb_ptr)
    {
        cb_ptr(code, 0);
    }

    T_API_EXIT(VMSV_71);
    return(result);
}
#endif

#ifdef VMSV_72
/*! @function   VmsvRequestNetOnTime
  * @abstract   Internal helper function to request INIC.NetOnTimer.Status.
  *
  * @author     SSemmler
  * @updated    2006-02-03
  * @version    2.1.0
  *
  * @return     standard error mask (for retry handling only).
  */
static byte VmsvRequestNetOnTime(void)
{
    byte result;
    TMsgTx *msg_ptr;

    T_MOD_ENTRY(VMSV_72);

    result  = ERR_NO;
    msg_ptr = MsgGetTxPtrExt(0);
    if (msg_ptr)
    {
        msg_ptr->FBlock_ID = FBLOCK_INIC;
        msg_ptr->Func_ID   = FUNCID_INIC_NETONTIMER;
        msg_ptr->Operation = OP_GET;

        MsgSend3(msg_ptr);
    }
    else
    {
        SCHEDULE_RETRY(VMSV_RETRY_REQUEST_NETONTIMER);
        result = ERR_BUFOV;
    }

    T_MOD_EXIT(VMSV_72);
    return(result);
}
#endif

#ifdef VMSV_73
/*! @function   VmsvHandleNetOnTimerStatus
  * @abstract   Internal helper function to process the INIC.NetOnTimer.Status
  *             message.
  *
  * @author     SSemmler
  * @updated    2006-02-03
  * @version    2.1.0
  *
  * @param      msg_ptr is the actual reveiced message.
  */
void VmsvHandleNetOnTimerStatus(TMsgRx *msg_ptr)
{
    TVmsvNetOnCB *cb_ptr;

    T_LIB_ENTRY(VMSV_73);

    ASSERT(msg_ptr);

    TAKE_VMSV();
    cb_ptr             = vmsv.net_on_cb_ptr;
    vmsv.net_on_cb_ptr = NULL;
    GIVE_VMSV();

    if (cb_ptr)
    {
        dword time = (dword)(((dword)msg_ptr->Data[0] << 24) |
                             ((dword)msg_ptr->Data[1] << 16) |
                             ((dword)msg_ptr->Data[2] <<  8) |
                             ((dword)msg_ptr->Data[3]));

        cb_ptr(NSR_S_OK, time);
    }

    T_LIB_EXIT(VMSV_73);
}
#endif

#ifdef VMSV_74
/*! @function   VmsvHandleNetOnTimerError
  * @abstract   Internal helper function to process the INIC.NetOnTimer.Error
  *             message.
  *
  * @author     SSemmler
  * @updated    2006-02-03
  * @version    2.1.0
  *
  * @param      msg_ptr is the actual reveiced message.
  */
void VmsvHandleNetOnTimerError(TMsgRx *msg_ptr)
{
    TVmsvNetOnCB *cb_ptr;

    T_LIB_ENTRY(VMSV_74);

    ASSERT(msg_ptr);

    TAKE_VMSV();
    cb_ptr = vmsv.net_on_cb_ptr;
    vmsv.net_on_cb_ptr = NULL;
    GIVE_VMSV();

    if (cb_ptr)
    {
        cb_ptr(NSR_BUILD(MNS_FALSE, 0xA0, msg_ptr->Data[0]), 0);
    }

    T_LIB_EXIT(VMSV_74);
}
#endif



#ifdef VMSV_81
/*! @function   MostGetLockState
  * @abstract   API function to query the lock state.
  *
  * @author     SSemmler
  * @updated    2006-02-05
  * @version    2.1.0
  *
  * @return     MNS_TRUE if there is a stable lock, otherwise MNS_FALSE.
  */
bool MostGetLockState(void)
{
    bool result;

    T_LIB_ENTRY(VMSV_81);

    TAKE_VMSV();
    result = (MSVAL_E_LOCKSTABLE == vmsv.shadow.lockstate) ? MNS_TRUE : MNS_FALSE;
    GIVE_VMSV();

    T_LIB_EXIT(VMSV_81);

    return (result);
}
#endif




#ifdef VMSV_85
/*! @function   VmsvHandleSSOResultStatus
  * @abstract   Internal message handler for INIC.SSOResult.Status.
  *
  * @author     RWilhelm
  * @updated    2008-07-03
  * @version    2.5.0
  *
  * @param      msg_ptr points to the actual received message.
  */
void VmsvHandleSSOResultStatus(TMsgRx *msg_ptr)
{
    byte ssoresult;
    byte store;

    T_LIB_ENTRY(VMSV_85);

    ASSERT(msg_ptr);

    store     = MNS_FALSE;
    ssoresult = msg_ptr->Data[0];

    TAKE_VMSV();
    if (   ((SDR_NO_RESULT       == vmsv.shadow.ssoresult) && (vmsv.shadow.ssoresult <  ssoresult))
        || ((SDR_NO_FAULT_SAVED  == vmsv.shadow.ssoresult) && (vmsv.shadow.ssoresult <  ssoresult))
        || ((SDR_CRITICAL_UNLOCK == vmsv.shadow.ssoresult) && (SDR_SUDDEN_SIGNAL_OFF == ssoresult)) )
    {
        vmsv.shadow.ssoresult = ssoresult;
        store                 = MNS_TRUE;
    }
    GIVE_VMSV();

    /* store ssoresult in nonvolatile memory */
    if (store &&  vmsv.cfg_ptr->sso_store_ssoresult_fptr)
    {
        vmsv.cfg_ptr->sso_store_ssoresult_fptr(ssoresult);
    }

    T_LIB_EXIT(VMSV_85);
}
#endif

#ifdef VMSV_86
/*! @function   VmsvHandleSSOResultError
  * @abstract   Internal message handler for INIC.SSOResult.Error.
  *
  * @author     RWilhelm
  * @updated    2008-06-13
  * @version    2.5.0
  *
  * @param      msg_ptr points to the actual received message.
  */
void VmsvHandleSSOResultError(TMsgRx *msg_ptr)
{
    T_LIB_ENTRY(VMSV_86);
    #if (MNS_DEBUG & NST_C_ASSERTS)
    ASSERT(msg_ptr);
    #else
    (void) msg_ptr;
    #endif
    T_LIB_EXIT(VMSV_86);
}
#endif

#ifdef VMSV_87
/*! @function   VmsvResetSSOResult
  * @abstract   resets the shadow variable
  *
  * @author     RWilhelm
  * @updated    2010-02-18
  * @version    3.0.3
  *
  * @param      none
  * @returns    nothing
  */
void VmsvResetSSOResult(void)
{
    byte ssoresult;

    T_API_ENTRY(VMSV_87);

    ssoresult = SDR_NO_RESULT;

    TAKE_VMSV();
    vmsv.shadow.ssoresult = ssoresult;
    GIVE_VMSV();

    /* store ssoresult in nonvolatile memory */
    if (vmsv.cfg_ptr->sso_store_ssoresult_fptr)
    {
        vmsv.cfg_ptr->sso_store_ssoresult_fptr(ssoresult);
    }

    (void)VmsvResetINICSSOResult();

    T_API_EXIT(VMSV_87);
}
#endif

#ifdef VMSV_99
/*! @function   VmsvResetINICSSOResult
  * @abstract   resets the SSO result in INIC
  *
  * @author     RWilhelm
  * @created    2010-02-18
  * @version    3.0.3
  *
  * @param      none
  * @returns    nothing
  */
static byte VmsvResetINICSSOResult(void)
{
    byte result;
    TMsgTx *msg_ptr;

    T_MOD_ENTRY(VMSV_99);

    result = ERR_NO;

    /* reset SSO result in INIC */
    msg_ptr = MsgGetTxPtrExt(1);
    if (msg_ptr)
    {
        msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
        msg_ptr->FBlock_ID = FBLOCK_INIC;
        msg_ptr->Func_ID   = FUNCID_INIC_SSORESULT;
        msg_ptr->Operation = OP_SET;
        msg_ptr->Data[0]   = SDR_NO_RESULT;

        MsgSend3(msg_ptr);
    }
    else
    {
        SCHEDULE_RETRY(VMSV_RETRY_SSORESULT);
        result = ERR_BUFOV;
    }

    T_MOD_EXIT(VMSV_99);

    return(result);
}
#endif




#ifdef VMSV_88
/*! @function   VmsvGetSSOResult
  * @abstract   reads the shadow variable
  *
  * @author     RWilhelm
  * @updated    2008-07-03
  * @version    2.5.0
  *
  * @param      msg_ptr pointer to the result message
  */
byte VmsvGetSSOResult(TMsgTx *msg_ptr)
{
    byte ssoresult;
    byte i;
    byte len;
    byte *diag_id;


    T_API_ENTRY(VMSV_88);
    ASSERT(msg_ptr);

    TAKE_VMSV();
    ssoresult = vmsv.shadow.ssoresult;
    diag_id   = vmsv.cfg_ptr->diag_id.stream;
    len       = vmsv.cfg_ptr->diag_id.length;
    GIVE_VMSV();


    if (NULL == diag_id)            /* prevent copying from NULL pointer */
    {
        len = (byte)0;
    }
    else if (MNS_FALSE != MostIsSupported(NSF_MOST_150))
    {
        len = (len < CTRL_MAX_PAYLOAD_OS81110 ) ? len : CTRL_MAX_PAYLOAD_OS81110 - 1;
    }
    else
    {
        len = (byte)0;
        FAILED_ASSERT();
    }

    msg_ptr->Data[0] = ssoresult;
    for (i=(byte)1; i <= len; ++i)     /* copy diag_id into message */
    {
        msg_ptr->Data[i] = *diag_id++; /*lint !e613 suppress "len is always 0 if diag_id is NULL" */
    }
    msg_ptr->Length = len+1;

    T_API_EXIT(VMSV_88);

    return(len+1);
}
#endif


#ifdef VMSV_90
/*! @function   VmsvHandleNIWakeupModeStatus
  * @abstract   Internal message handler for INIC.NIWakeupMode.Status..
  *
  * @author     RWilhelm
  * @updated    2008-11-14
  *
  * @parameter  msg_ptr points to the actual received message.
  * @return     void
  */
void VmsvHandleNIWakeupModeStatus(TMsgRx *msg_ptr)
{
    T_LIB_ENTRY(VMSV_90);

    ASSERT(msg_ptr);

    TAKE_VMSV();
    vmsv.shadow.ni_wakeup_mode.mode = msg_ptr->Data[0];
    GIVE_VMSV();

    T_LIB_EXIT(VMSV_90);
}
#endif

#ifdef VMSV_91
/*! @function   VmsvHandleNIWakeupModeError
  * @abstract   Internal message handler for INIC.NIWakeupMode.Error.
  *
  * @author     RWilhelm
  * @updated    2008-11-14
  *
  * @parameter  msg_ptr points to the actual received message.
  * @return     void
  */
void VmsvHandleNIWakeupModeError(TMsgRx *msg_ptr)
{
    T_LIB_ENTRY(VMSV_91);
    ASSERT(msg_ptr);

    if(ERR_NOTAVAILABLE == msg_ptr->Data[1])
    {
        (void)VmsvRequestNIWakeUpMode();
    }
    else
    {
        FAILED_ASSERT();
    }

    T_LIB_EXIT(VMSV_91);
}
#endif

#ifdef VMSV_92
/*! @function   VmsvRequestNIWakeUpMode
  * @abstract   Internal helper function to send INIC.NIWakeupMode.Get.
  *
  * @author     RWilhelm
  * @updated    2008-11-25
  *
  * @return     standard error mask (for retry handling only).
  */
static byte VmsvRequestNIWakeUpMode(void)
{
    byte    result;
    TMsgTx  *msg_ptr;

    T_MOD_ENTRY(VMSV_92);

    result  = ERR_NO;
    msg_ptr = MsgGetTxPtrExt(0);

    if (msg_ptr)
    {
        msg_ptr->FBlock_ID = FBLOCK_INIC;
        msg_ptr->Func_ID   = FUNCID_INIC_NIWAKEUPMODE;
        msg_ptr->Operation = OP_GET;

        MsgSend3(msg_ptr);
    }
    else
    {
        SCHEDULE_RETRY(VMSV_RETRY_REQUEST_NIWAKEUPMODE);
        result = ERR_BUFOV;
    }

    T_MOD_EXIT(VMSV_92);
    return(result);
}
#endif

#ifdef VMSV_93
/*! @function   VmsvSetNIWakeUpMode
  * @abstract   Internal helper function to send INIC.NIWakeupMode.Set.
  *
  * @author     RWilhelm
  * @updated    2008-11-25
  *
  * @return     standard error mask (for retry handling only).
  */
static byte VmsvSetNIWakeUpMode(void)
{
    byte    result;
    byte    mode;
    TMsgTx  *msg_ptr;

    T_MOD_ENTRY(VMSV_93);

    result = ERR_NO;

    TAKE_VMSV();
    mode = vmsv.shadow.ni_wakeup_mode.request;
    GIVE_VMSV();

    if (INIC_SHADOW_INVALID_BYTE != mode)
    {
        msg_ptr = MsgGetTxPtrExt(1);

        if (msg_ptr)
        {
            msg_ptr->FBlock_ID = FBLOCK_INIC;
            msg_ptr->Func_ID   = FUNCID_INIC_NIWAKEUPMODE;
            msg_ptr->Operation = OP_SET;
            msg_ptr->Data[0]   = mode;
            MsgSend3(msg_ptr);
        }
        else
        {
            SCHEDULE_RETRY(VMSV_RETRY_SET_NIWAKEUPMODE);
            result = ERR_BUFOV;
        }

    }

    T_MOD_EXIT(VMSV_93);
    return(result);
}
#endif


#ifdef VMSV_94
byte VmsvPhysicalLayerTestStart(pTMsgRx rx_ptr, byte type, word lead_in, dword duration, word lead_out)
{
    HMBMBUF handle;
    pTMsgTx msg_ptr;
    byte    retval;
    byte    test_state;

    T_LIB_ENTRY(VMSV_94);

    retval = ERR_NO;

    TAKE_VMSV();
    vmsv.plt.type       = type;
    vmsv.plt.lead_in    = lead_in;
    vmsv.plt.lead_out   = lead_out;
    vmsv.plt.duration   = duration;
    test_state          = vmsv.plt.test_state;
    GIVE_VMSV();

    msg_ptr = MsgGetTxPtrExt(2);
    ASSERT(msg_ptr);

    if (msg_ptr)
    {
        msg_ptr->Tgt_Adr   = rx_ptr->Src_Adr;
        msg_ptr->FBlock_ID = rx_ptr->FBlock_ID;
        msg_ptr->Inst_ID   = rx_ptr->Inst_ID;
        msg_ptr->Func_ID   = rx_ptr->Func_ID;
        msg_ptr->Operation = OP_RESULTACK;
        msg_ptr->Length    = (word)2;
        msg_ptr->Data[0]   = rx_ptr->Data[0];
        msg_ptr->Data[1]   = rx_ptr->Data[1];

        handle = MbmGetHandleByMsgPtr(msg_ptr);
        PmsSend(handle, VmsvPhysicalLayerTestTxFinal);

        /* stop test if already running */
        if (VMSV_PLT_OFF != test_state)
        {
            MostClearTimer(&vmsv.plt.state_timer);
        }

        TAKE_VMSV();
        vmsv.plt.test_state = VMSV_PLT_WAIT_TX;
        GIVE_VMSV();
    }
    else
    {
        retval = ERR_BUFOV;
    }

    T_LIB_EXIT(VMSV_94);

    return(retval);
}
#endif

#ifdef VMSV_95
static byte VmsvPhysicalLayerTestProcedure(void)
{
    byte    test_state;
    byte    type;
    byte    dev_mode;
    word    lead_in_time;
    word    lead_out_time;
    dword   duration_time;
    word    diff_time;

    TMsgTx *msg_ptr;


    T_MOD_ENTRY(VMSV_95);

    TAKE_VMSV();
    test_state = vmsv.plt.test_state;
    GIVE_VMSV();

    switch (test_state)
    {
        case VMSV_PLT_OFF:
            FAILED_ASSERT();
            break;

        case VMSV_PLT_WAIT_TX:
            FAILED_ASSERT();
            break;

        case VMSV_PLT_START:
        {

            TAKE_VMSV();
            vmsv.plt.saved_dev_mode = vmsv.shadow.devmode;  /* save current devmode */
            type = vmsv.plt.type;

            switch (type)
            {
                case VMSV_PLT_TYPE_AUTO:
                    if (MNS_MASTER == vmsv.shadow.devmode)
                    {
                        dev_mode = MNS_RETIMEDBYPASS_MASTER;
                    }
                    else
                    {
                        dev_mode = MNS_RETIMEDBYPASS_SLAVE;
                    }
                    break;

                case VMSV_PLT_TYPE_MASTER:
                    dev_mode = MNS_RETIMEDBYPASS_MASTER;
                    break;

                case VMSV_PLT_TYPE_SLAVE:
                    dev_mode = MNS_RETIMEDBYPASS_SLAVE;
                    break;

                /* parameter error, finish procedure */
                default:
                    dev_mode   = INIC_SHADOW_INVALID_BYTE;    /* avoid lint warning */
                    test_state = VMSV_PLT_OFF;
                    break;
            }
            GIVE_VMSV();

            if (VMSV_PLT_OFF != test_state)
            {
                msg_ptr = MsgGetTxPtrExt(1);
                if (msg_ptr)
                {
                    msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
                    msg_ptr->FBlock_ID = FBLOCK_INIC;
                    msg_ptr->Func_ID   = FUNCID_INIC_DEVICEMODE;
                    msg_ptr->Operation = OP_SET;
                    msg_ptr->Length    = (word)1;
                    msg_ptr->Data[0]   = dev_mode;

                    MsgSend3(msg_ptr);

                    /* start timer t_lead_in */
                    TAKE_VMSV();
                    lead_in_time = vmsv.plt.lead_in;
                    GIVE_VMSV();

                    MostSetTimer(&vmsv.plt.state_timer, lead_in_time, MNS_FALSE);

                    test_state = VMSV_PLT_LEAD_IN;
                }
                else
                {
                    SCHEDULE_RETRY(VMSV_RETRY_PLT);
                }
            }

            break;
        }

        case VMSV_PLT_LEAD_IN:
        {
            msg_ptr = MsgGetTxPtrExt(0);
            if (msg_ptr)
            {
                msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
                msg_ptr->FBlock_ID = FBLOCK_INIC;
                msg_ptr->Func_ID   = FUNCID_INIC_NUMCODINGERRORS;
                msg_ptr->Operation = OP_GET;
                msg_ptr->Length    = (word)0;
                MsgSend3(msg_ptr);

                TAKE_VMSV();
                vmsv.plr.lock_status = MNS_FALSE;           /* reset result variables */
                vmsv.plr.err_count   = (word)0;
                GIVE_VMSV();

                test_state = VMSV_PLT_TEST1;                     /* goto next state, start timers */
                VmsvSetPendingEvent(VMSV_P_PHYSICAL_LAYER_TEST);
            }
            else
            {
                SCHEDULE_RETRY(VMSV_RETRY_PLT);
            }

            break;
        }

        case VMSV_PLT_TEST1:
        {
            /* start timer t_duration */
            TAKE_VMSV();
            duration_time = vmsv.plt.duration;
            GIVE_VMSV();

            if (duration_time > (dword)0xFFFF)
            {
                if (duration_time < (dword)0x1FFFF)     /* largest timer value is 0xFFFF*/
                {
                    diff_time = (word)(duration_time/2);  /* avoid very small values at the end */
                }
                else
                {
                    diff_time = (word)0xFFFF;
                }

                duration_time -= (dword)diff_time;
                TAKE_VMSV();
                vmsv.plt.duration = duration_time;
                GIVE_VMSV();

                MostSetTimer(&vmsv.plt.state_timer, diff_time, MNS_FALSE);
                /*remain in this state until duration ends*/
            }
            else
            {
                MostSetTimer(&vmsv.plt.state_timer, (word)duration_time, MNS_FALSE);
                test_state = VMSV_PLT_TEST2;
            }
            break;
        }

        case VMSV_PLT_TEST2:
        {
            msg_ptr = MsgGetTxPtrExt(0);
            if (msg_ptr)
            {
                msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
                msg_ptr->FBlock_ID = FBLOCK_INIC;
                msg_ptr->Func_ID   = FUNCID_INIC_NUMCODINGERRORS;
                msg_ptr->Operation = OP_GET;
                msg_ptr->Length    = (word)0;
                MsgSend3(msg_ptr);

                /* start timer t_lead_out */
                TAKE_VMSV();
                lead_out_time = vmsv.plt.lead_out;
                GIVE_VMSV();

                MostSetTimer(&vmsv.plt.state_timer, lead_out_time, MNS_FALSE);

                test_state = VMSV_PLT_LEAD_OUT;
            }
            else
            {
                SCHEDULE_RETRY(VMSV_RETRY_PLT);
            }

            break;
        }

        case VMSV_PLT_LEAD_OUT:
        {
            TAKE_VMSV();
            dev_mode = vmsv.plt.saved_dev_mode;
            GIVE_VMSV();

            msg_ptr = MsgGetTxPtrExt(1);
            if (msg_ptr)
            {
                msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
                msg_ptr->FBlock_ID = FBLOCK_INIC;
                msg_ptr->Func_ID   = FUNCID_INIC_DEVICEMODE;
                msg_ptr->Operation = OP_SET;
                msg_ptr->Length    = (word)1;
                msg_ptr->Data[0]   = dev_mode;
                MsgSend3(msg_ptr);

                /* finish state machine */
                TAKE_VMSV();
                vmsv.plt.test_done = MNS_TRUE;
                GIVE_VMSV();

                ET_PhysicalLayerTest_Finished();
                test_state = VMSV_PLT_OFF;
            }
            else
            {
                SCHEDULE_RETRY(VMSV_RETRY_PLT);
            }

            break;
        }

        default:
            FAILED_ASSERT();
            break;
    }

    TAKE_VMSV();
    vmsv.plt.test_state = test_state;
    GIVE_VMSV();


    T_MOD_EXIT(VMSV_95);

    return(test_state);
}
#endif

#ifdef VMSV_96
bool VmsvPhysicalLayerTestResult( bool *lock_status, dword *num_errors)
{
    bool valid;

    T_LIB_ENTRY(VMSV_96);

    TAKE_VMSV();
    valid =  vmsv.plt.test_done;
    if (MNS_FALSE != valid)
    {
        *num_errors  = (dword)vmsv.plr.err_count;
        *lock_status = vmsv.plr.lock_status;
    }
    GIVE_VMSV();

    T_LIB_EXIT(VMSV_96);

    return(valid);
}
#endif

#ifdef VMSV_97
void VmsvHandleCodingErrorsStatus(TMsgRx *msg_ptr)
{

    T_LIB_ENTRY(VMSV_97);

    ASSERT(msg_ptr);

    TAKE_VMSV();
    if (VMSV_PLT_LEAD_OUT == vmsv.plt.test_state)
    {
        vmsv.plr.err_count = (word)((msg_ptr->Data[0] << 8) | (msg_ptr->Data[1]));
    }
    GIVE_VMSV();

    T_LIB_EXIT(VMSV_97);
}
#endif




#ifdef VMSV_98
static byte VmsvPhysicalLayerTestTxFinal(HMBMBUF handle, byte status)
{

    T_MOD_ENTRY(VMSV_98);

    switch (status)
    {
        case XMIT_SUCCESS:
            /* start PLT state machine */
            TAKE_VMSV();
            vmsv.plt.test_state = VMSV_PLT_START;
            GIVE_VMSV();

            VmsvSetPendingEvent(VMSV_P_PHYSICAL_LAYER_TEST);
            break;

        default:            /* FAILED */
            break;
    }

    MbmFree(handle);
    T_MOD_EXIT(VMSV_98);

    return(PMS_RELEASE);
}
#endif

#ifdef VMSV_100
/*!
  * \brief      Announces a critical situation
  * \details    This function is used to force an immediate shutdown of the device
  *             because of a critical situation (e.g. overtemperature).
  *             It can also be used to hold a device in the NET_OFF state.
  * \param      state      emergency condition status
  */
void MostEmergencyCondition(bool state)
{
    TMsgTx *msg_ptr;

    T_API_ENTRY(VMSV_100);

    TAKE_VMSV();
    vmsv.emergency_state = state;
    GIVE_VMSV();

    msg_ptr = MsgGetTxPtrExt(1);

    if (msg_ptr)
    {
        msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
        msg_ptr->FBlock_ID = FBLOCK_INIC;
        msg_ptr->Func_ID   = FUNCID_INIC_PMISTATE;
        msg_ptr->Operation = OP_SET;
        if (MNS_FALSE != state)
        {
            msg_ptr->Data[0]   = PMI_SHUTDOWN;
        }
        else
        {
            msg_ptr->Data[0]   = PMI_CLEAR;
        }
        MsgSend3(msg_ptr);
    }
    else
    {
        VmsvSetPendingEvent(VMSV_P_EMERGENCYCONDITION_RETRY);
    }

    T_API_EXIT(VMSV_100);





}
#endif

#ifdef VMSV_101
static _INLINE void VmsvSendMdmMsv2NwmConfStateNok(void)
{
    T_MOD_ENTRY(VMSV_101);

    MdmMsgSendMns(MDM_MSV2_NWM_CONF_STATE_NOK, NULL, (byte)0);

    T_MOD_EXIT(VMSV_101);
}
#endif


#ifdef VMSV_102
bool VmsvPhysicalLayerTestActive(void)
{
    bool ret_val;

    T_LIB_ENTRY(VMSV_102);

    ret_val = MNS_FALSE;

    TAKE_VMSV();
    if (VMSV_PLT_OFF != vmsv.plt.test_state)
    {
        ret_val = MNS_TRUE;
    }
    GIVE_VMSV();


    T_LIB_EXIT(VMSV_102);

    return(ret_val);
}
#endif

