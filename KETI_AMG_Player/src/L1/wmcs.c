/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Implementation of the MOST Processor Control Service 
                Wrapper (WMCS)
File:           wmcs.c
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
  * \brief      Implementation of the MOST Processor Control Service 
  *             Wrapper (WMCS)
  * \details    This Service provides functions to change and request general
  *             properties of the INIC and its MOST Processor. Examples are the
  *             address settings, packet retry settings etc.
  */

/*
==============================================================================
    Includes
==============================================================================
*/
#include "mns.h"
#include "ams.h"
#include "wmcs.h"
#include "wmcs_pv.h"
#include "pms.h"
#ifdef VMSV_MIN
#include "vmsv.h"
#endif

/*
==============================================================================
    NetServices trace: module trace macros
==============================================================================
*/

#if (MNS_DEBUG & NST_C_FUNC_ENTRIES)
    #define T_API_ENTRY(func)   MNS_TRACE((MNS_P_SRV_WMCS, NST_E_FUNC_ENTRY_API, 1, func))
    #define T_LIB_ENTRY(func)   MNS_TRACE((MNS_P_SRV_WMCS, NST_E_FUNC_ENTRY_LIB, 1, func))
    #define T_MOD_ENTRY(func)   MNS_TRACE((MNS_P_SRV_WMCS, NST_E_FUNC_ENTRY_MOD, 1, func))
#else
    #define T_API_ENTRY(func)
    #define T_LIB_ENTRY(func)
    #define T_MOD_ENTRY(func)
#endif

#if (MNS_DEBUG & NST_C_FUNC_EXITS)
    #define T_API_EXIT(func)    MNS_TRACE((MNS_P_SRV_WMCS, NST_E_FUNC_EXIT_API, 1, func))
    #define T_LIB_EXIT(func)    MNS_TRACE((MNS_P_SRV_WMCS, NST_E_FUNC_EXIT_LIB, 1, func))
    #define T_MOD_EXIT(func)    MNS_TRACE((MNS_P_SRV_WMCS, NST_E_FUNC_EXIT_MOD, 1, func))
#else
    #define T_API_EXIT(func)
    #define T_LIB_EXIT(func)
    #define T_MOD_EXIT(func)
#endif

#if (MNS_DEBUG & NST_C_INIT)
    #define T_INIT()            MNS_TRACE((MNS_P_SRV_WMCS, NST_E_INIT, 0))
#else
    #define T_INIT()
#endif

#if (MNS_DEBUG & NST_C_SERVICE)
    #define T_SERVICE(event)    MNS_TRACE((MNS_P_SRV_WMCS, NST_E_SERVICE, 1, event))
#else
    #define T_SERVICE(event)
#endif

#if (MNS_DEBUG & NST_C_REQUESTS)
    #define T_REQUEST(event)    MNS_TRACE((MNS_P_SRV_WMCS, NST_E_REQUEST, 1, event))
#else
    #define T_REQUEST(event)
#endif

#if (MNS_DEBUG & NST_C_ASSERTS)
    #define FAILED_ASSERT()     MNS_TRACE((MNS_P_SRV_WMCS, NST_E_ASSERT, 1, __LINE__))
    #define ASSERT(exp)         if(!(exp)) FAILED_ASSERT()
#else
    #define FAILED_ASSERT()
    #define ASSERT(exp)
#endif

#define TAKE_EVENTS()   WAIT4MX(MX_WMCS_PE)
#define GIVE_EVENTS()   REL_MX(MX_WMCS_PE)
#define TAKE_WMCS()     WAIT4MX(MX_WMCS_CS)
#define GIVE_WMCS()     REL_MX(MX_WMCS_CS)

#define SCHEDULE_RETRY(flag) { TAKE_EVENTS(); wmcs.retry |= flag; GIVE_EVENTS(); }
#define RETRY_DONE(flag)     { TAKE_EVENTS(); wmcs.retry &= ~flag; GIVE_EVENTS(); }
#define PROCESS_RETRY(retry_flag, retry_func) { if ((ERR_NO == result) && (retry & retry_flag)) { RETRY_DONE(retry_flag); result = retry_func(); }}


/*
================================================================================
    Module Internal Variables
================================================================================
*/

#ifdef MCS_MIN
    /*! Data variable of the WMCS module */
    static WmcsData wmcs;
#endif  /* #ifdef MCS_MIN */

/*
==============================================================================
==============================================================================
    Module Implementation
==============================================================================
==============================================================================
*/

#ifdef WMCS_0
void WmcsInit(void)
{
  #if (defined MEP_MIN) && (defined _OS81110_MEP)
    byte i;
  #endif

    T_LIB_ENTRY(WMCS_0);
    T_INIT();

    TAKE_EVENTS();
    wmcs.pending_events = WMCS_P_NONE;
    wmcs.latest_handled_event = WMCS_P_NONE;
    wmcs.retry = 0;
    GIVE_EVENTS();

    TAKE_WMCS();
    wmcs.memory.pending          = MNS_FALSE;

  #if (defined MEP_MIN) && (defined _OS81110_MEP)
    wmcs.mepfiltermode.pending   = MNS_FALSE;
    wmcs.mephashtable.pending    = MNS_FALSE;
    wmcs.eui48.pending           = MNS_FALSE;

    for (i=0; i<6; ++i)
    {
        wmcs.eui48_shadow[i] = 0;
    }
  #endif

  #ifdef _OS81110_PCK_LLR
    wmcs.packetretrytime.pending = MNS_FALSE;
  #endif
    GIVE_WMCS();

    MostRegisterTimer(&(wmcs.coding_errors.timer), WmcsSetPendingEvent, WMCS_P_CE_TIMER);

    T_LIB_EXIT(WMCS_0);
}
#endif

#ifdef WMCS_1
void WmcsService(void)
{
    word event_to_handle;
    bool request_flag;

    T_LIB_ENTRY(WMCS_1);

    event_to_handle = WmcsGetNextEventToHandle();
    request_flag    = MNS_FALSE;

    T_SERVICE(event_to_handle);

    switch (event_to_handle)
    {
        case WMCS_P_BUF_FREED:
            (void)WmcsRetry();
            break;

        case WMCS_P_GO_PROTECTED:
            Wmcs_Go_Protected();
            break;

        case WMCS_P_GO_SEMI_PROTECTED:
            Wmcs_Go_SemiProtected();
            break;

        case WMCS_P_NODE_ADDR_RETRY:
            {
                word temp;

                TAKE_WMCS();
                temp = wmcs.node_addr.scheduled;
                GIVE_WMCS();

                MostSetNodeAdr(temp);
            }
            break;

        case WMCS_P_NODE_ADDR_SETGET_RETRY:
            {
                word temp;

                TAKE_WMCS();
                temp = wmcs.node_addr.scheduled;
                GIVE_WMCS();

                MostSetGetNodeAdr(temp);
            }
            break;

        case WMCS_P_GROUP_ADDR_RETRY:
            {
                byte temp;

                TAKE_WMCS();
                temp = wmcs.group_addr.scheduled;
                GIVE_WMCS();

                MostSetGroupAdr(temp);
            }
            break;

        case WMCS_P_RMCK_RETRY:
            {
                byte rmck_divider;

                TAKE_WMCS();
                rmck_divider = wmcs.rmck_divider;
                GIVE_WMCS();

                MostSelectClockOutput(rmck_divider);
            }
            break;

        case WMCS_P_NTF_COMPLETE:
            MnsServiceInitComplete(MNS_PHASE_INIT, MNS_P_SRV_WMCS);
            break;

        case WMCS_P_CE_TIMER:
            WmcsCodingErrorsTimeout();
            break;

        default:
            event_to_handle = WMCS_P_NONE;
            break;
    }

    TAKE_EVENTS();
    request_flag = (WMCS_P_NONE != wmcs.pending_events) ? MNS_TRUE : MNS_FALSE;
    GIVE_EVENTS();

    if (MNS_FALSE != request_flag)
    {
        MnsSetPendingService(MNS_P_SRV_WMCS);
    }

    T_LIB_EXIT(WMCS_1);
}
#endif

#ifdef WMCS_2
void WmcsSetPendingEvent(word event_flag)
{
    T_MOD_ENTRY(WMCS_2);

    T_REQUEST(event_flag);
    MnsSetPendingEventFlag(event_flag, MX_WMCS_PE,
                           &wmcs.pending_events, MNS_P_SRV_WMCS);
    T_MOD_EXIT(WMCS_2);
}
#endif

#ifdef WMCS_3
static word WmcsGetNextEventToHandle(void)
{
    word result;

    T_MOD_ENTRY(WMCS_3);
    result = MnsGetNextEventFlagToCall(MX_WMCS_PE,
                                       &wmcs.pending_events,
                                       &wmcs.latest_handled_event,
                                       WMCS_P_FIRST, WMCS_P_LAST);
    T_MOD_EXIT(WMCS_3);
    return(result);
}
#endif

#ifdef WMCS_4
word MostGetNodeAdr(void)
{
    word result;

    T_API_ENTRY(WMCS_4);
    TAKE_WMCS();
    result = wmcs.node_addr.value;
    GIVE_WMCS();
    T_API_EXIT(WMCS_4);

    return(result);
}
#endif

#ifdef WMCS_5
byte MostGetNodePos(void)
{
    byte result;

    T_API_ENTRY(WMCS_5);
    TAKE_WMCS();
    result = wmcs.node_pos;
    GIVE_WMCS();
    T_API_EXIT(WMCS_5);

    return(result);
}
#endif

#ifdef WMCS_6
byte MostGetGroupAdr(void)
{
    byte result;

    T_API_ENTRY(WMCS_6);
    TAKE_WMCS();
    result = wmcs.group_addr.value;
    GIVE_WMCS();
    T_API_EXIT(WMCS_6);

    return(result);
}
#endif

#ifdef WMCS_8
void MostSetGetNodeAdr(word address)
{
    TMsgTx *msg_ptr;

    T_API_ENTRY(WMCS_8);

    msg_ptr = MsgGetTxPtrExt(2);

    TAKE_WMCS();
    wmcs.node_addr.scheduled = address;
    GIVE_WMCS();

    if (msg_ptr)
    {
        msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
        msg_ptr->FBlock_ID = FBLOCK_NETBLOCK;
        msg_ptr->Inst_ID   = 0;
        msg_ptr->Func_ID   = FUNC_NODEADDRESS;
        msg_ptr->Operation = OP_SETGET;
        msg_ptr->Length    = 2;
        msg_ptr->Data[0]   = HB(address);
        msg_ptr->Data[1]   = LB(address);

        MsgSend3(msg_ptr);
    }
    else
    {
        WmcsSetPendingEvent(WMCS_P_NODE_ADDR_SETGET_RETRY);
    }

    T_API_EXIT(WMCS_8);
}
#endif

#ifdef WMCS_9
void MostSetNodeAdr(word address)
{
    TMsgTx *msg_ptr;

    T_API_ENTRY(WMCS_9);

    msg_ptr = MsgGetTxPtrExt(2);

    TAKE_WMCS();
    wmcs.node_addr.scheduled = address;
    GIVE_WMCS();

    if (msg_ptr)
    {
        msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
        msg_ptr->FBlock_ID = FBLOCK_NETBLOCK;
        msg_ptr->Inst_ID   = 0;
        msg_ptr->Func_ID   = FUNC_NODEADDRESS;
        msg_ptr->Operation = OP_SET;
        msg_ptr->Length    = 2;
        msg_ptr->Data[0]   = HB(address);
        msg_ptr->Data[1]   = LB(address);

        MsgSend3(msg_ptr);
    }
    else
    {
        WmcsSetPendingEvent(WMCS_P_NODE_ADDR_RETRY);
    }

    T_API_EXIT(WMCS_9);
}
#endif

#ifdef WMCS_10
void MostSetGroupAdr(byte address)
{
    TMsgTx *msg_ptr;

    T_API_ENTRY(WMCS_10);

    msg_ptr = MsgGetTxPtrExt(2);
    TAKE_WMCS();
    wmcs.group_addr.scheduled = address;
    GIVE_WMCS();

    if (msg_ptr)
    {
        msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
        msg_ptr->FBlock_ID = FBLOCK_NETBLOCK;
        msg_ptr->Inst_ID   = 0;
        msg_ptr->Func_ID   = FUNC_GROUPADDRESS;
        msg_ptr->Operation = OP_SET;
        msg_ptr->Length    = 2;
        msg_ptr->Data[0]   = 3;
        msg_ptr->Data[1]   = address;

        MsgSend3(msg_ptr);
    }
    else
    {
        WmcsSetPendingEvent(WMCS_P_GROUP_ADDR_RETRY);
    }

    T_API_EXIT(WMCS_10);
}
#endif

#ifdef WMCS_11
byte MostCheckOwnAddress(word address)
{
    byte result;
    word node_addr;
    byte node_pos;
    byte group_addr;

    T_LIB_ENTRY(WMCS_11);

    result = MOST_CHECK_ADDR_NONE;

    TAKE_WMCS();
    node_addr  = wmcs.node_addr.value;
    node_pos   = wmcs.node_pos;
    group_addr = wmcs.group_addr.value;
    GIVE_WMCS();

    if (MSG_TGT_EHC == address)
    {
        result = MOST_CHECK_ADDR_INTERN;
    }
    else if (node_addr == address)
    {
        result = MOST_CHECK_ADDR_NODE;
    }
    else if ((0x400 + node_pos) == address)
    {
        result = MOST_CHECK_ADDR_POS;
    }
    else if ((0x300 + group_addr) == address)
    {
        result = MOST_CHECK_ADDR_GROUP;
    }
    else if (MSG_TGT_BROADCAST_BLOCKING == address)
    {
        result = MOST_CHECK_ADDR_BROADCAST;
    }
    else if (MSG_TGT_BROADCAST_UNBLOCKING == address)
    {
        result = MOST_CHECK_ADDR_BROADCAST;
    }

    T_LIB_EXIT(WMCS_11);
    return (result);
}
#endif

#ifdef WMCS_14
static void Wmcs_Go_Protected(void)
{

    T_MOD_ENTRY(WMCS_14);

    TAKE_WMCS();
    wmcs.node_pos                   = INIC_SHADOW_INVALID_BYTE;
    wmcs.node_addr.value            = INIC_SHADOW_INVALID_WORD;
    wmcs.node_addr.scheduled        = INIC_SHADOW_INVALID_BYTE;
    wmcs.group_addr.value           = INIC_SHADOW_INVALID_BYTE;
    wmcs.group_addr.scheduled       = INIC_SHADOW_INVALID_BYTE;
    wmcs.rmck_divider               = INIC_SHADOW_INVALID_BYTE;
    wmcs.shadow.alt_packet_addr     = INIC_SHADOW_INVALID_WORD;
    wmcs.coding_errors.cb_ptr       = NULL;
    wmcs.coding_errors.counter      = WMCS_CE_INACTIVE;
    wmcs.coding_errors.deadtime     = 0;
    wmcs.coding_errors.timeout      = 0;
    wmcs.coding_errors.busy_reset   = MNS_FALSE;

    wmcs.memory.pending             = MNS_FALSE;

  #if (defined MEP_MIN) && (defined _OS81110_MEP)
    wmcs.mepfiltermode.pending      = MNS_FALSE;
    wmcs.mephashtable.pending       = MNS_FALSE;
    wmcs.eui48.pending              = MNS_FALSE;
  #endif

  #ifdef _OS81110_PCK_LLR
    wmcs.packetretrytime.pending    = MNS_FALSE;
  #endif


    GIVE_WMCS();

    MostClearTimer(&(wmcs.coding_errors.timer));

    TAKE_EVENTS();
    wmcs.retry = 0;
    GIVE_EVENTS();

  #ifdef MNS_AVOID_ATTACH
    MnsServiceInitComplete(MNS_PHASE_INIT, MNS_P_SRV_WMCS);
  #endif

    MnsServiceInitComplete(MNS_PHASE_RESET, MNS_P_SRV_WMCS);

    T_MOD_EXIT(WMCS_14);
}
#endif

#ifdef WMCS_15
static void Wmcs_Go_SemiProtected(void)
{
    T_MOD_ENTRY(WMCS_15);

    #ifdef WMCS_58
    (void)WmcsRequestEUI48();
    #endif

    T_MOD_EXIT(WMCS_15);
}
#endif

#ifdef WMCS_25
void WmcsRefreshNodePos(byte pos)
{
    T_LIB_ENTRY(WMCS_25);

    TAKE_WMCS();
    wmcs.node_pos = pos;
    GIVE_WMCS();

    #ifdef VMSV_58
    VmsvRefreshNodePos(pos);
    #endif

    T_LIB_EXIT(WMCS_25);
}
#endif

#ifdef WMCS_17
void WmcsHandleNodeAddrStatus(TMsgRx *msg_ptr)
{
    T_LIB_ENTRY(WMCS_17);

    ASSERT(msg_ptr);

    TAKE_WMCS();
    wmcs.node_addr.value = (word)(((word)msg_ptr->Data[0] << 8) |
                      (word)msg_ptr->Data[1]);
    GIVE_WMCS();

    MnsNtfCheck(NTF_NODE_ADDR);

    T_LIB_EXIT(WMCS_17);
}
#endif

#ifdef WMCS_20
void WmcsHandleGroupAddrStatus(TMsgRx *msg_ptr)
{
    T_LIB_ENTRY(WMCS_20);

    ASSERT(msg_ptr);

    TAKE_WMCS();
    wmcs.group_addr.value = msg_ptr->Data[1];
    GIVE_WMCS();

    MnsNtfCheck(NTF_GROUP_ADDR);

    T_LIB_EXIT(WMCS_20);
}
#endif

#ifdef WMCS_21
void MostSelectClockOutput(byte rmck_divider)
{
    TMsgTx *msg_ptr;

    T_API_ENTRY(WMCS_21);

    msg_ptr = MsgGetTxPtrExt(1);

    TAKE_WMCS();
    wmcs.rmck_divider = rmck_divider;
    GIVE_WMCS();

    if (msg_ptr)
    {
        msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
        msg_ptr->FBlock_ID = FBLOCK_INIC;
        msg_ptr->Inst_ID   = 0;
        msg_ptr->Func_ID   = FUNCID_INIC_RMCK;
        msg_ptr->Operation = OP_SET;
        msg_ptr->Data[0]   = rmck_divider;

        MsgSend3(msg_ptr);
    }
    else
    {
        WmcsSetPendingEvent(WMCS_P_RMCK_RETRY);
    }

    T_API_EXIT(WMCS_21);
}
#endif


/*! Retry handling in case of "buf freed" event.
  *
  * @author     SSemmler
  * @updated       5/11/2005
  * @version    2.0.3
  *
  * @return     ERR_NO or ERR_BUFOV (retry is scheduled)
  */
#ifdef WMCS_23
static byte WmcsRetry(void)
{
    word retry;
    byte result;

    T_MOD_ENTRY(WMCS_23);

    result = ERR_NO;
    TAKE_WMCS();
    retry = wmcs.retry;
    GIVE_WMCS();

    PROCESS_RETRY    (WMCS_RETRY_REQUEST_CODINGERR,   WmcsRequestCodingErrors);
#ifdef WMCS_58
    PROCESS_RETRY    (WMCS_RETRY_REQUEST_EUI48,       WmcsRequestEUI48);
#endif

    T_MOD_EXIT(WMCS_23);

    return(result);
}
#endif


#ifdef WMCS_26
/*!
  * \brief      Internal helper function to send INIC.NumCodingErrors.Get.
  *
  * \return     ERR_NO or ERR_BUFOV (retry is scheduled)
  *
  */
static byte WmcsRequestCodingErrors(void)
{
    byte   result;
    TMsgTx *msg_ptr;

    T_MOD_ENTRY(WMCS_26);

    result  = ERR_NO;
    msg_ptr = MsgGetTxPtrExt(0);

    if (msg_ptr)
    {
        msg_ptr->FBlock_ID = FBLOCK_INIC;
        msg_ptr->Func_ID   = FUNCID_INIC_NUMCODINGERRORS;
        msg_ptr->Operation = OP_GET;

        MsgSend3(msg_ptr);
    }
    else
    {
        SCHEDULE_RETRY(WMCS_RETRY_REQUEST_CODINGERR);
        result = ERR_BUFOV;
    }

    T_MOD_EXIT(WMCS_26);
    return(result);
}
#endif

#ifdef WMCS_27
/*!
  * \brief      Internal helper function to parse INIC.NumCodingErrors.Status.
  *
  * \return     nothing
  */
void WmcsHandleCodingErrorsStatus(TMsgRx *msg_ptr)
{
    TWmcsCodingErrorCB *cb_ptr ;

    T_LIB_ENTRY(WMCS_27);

    ASSERT(msg_ptr);

    TAKE_WMCS();
    cb_ptr = wmcs.coding_errors.cb_ptr;
    wmcs.coding_errors.cb_ptr     = NULL;
    wmcs.coding_errors.busy_reset = MNS_FALSE;
    GIVE_WMCS();

    if(cb_ptr)
    {
        cb_ptr(NSR_S_CE_INIC, (word)((msg_ptr->Data[0] << 8) | msg_ptr->Data[1]));
    }

    T_LIB_EXIT(WMCS_27);
}
#endif

#ifdef WMCS_28
/*!
  * \brief      API function to query the INIC for the current number of coding errors.
  *
  * \details    As soon as the result is sent by the INIC, the counter is reset. NET_ON and
  *             reset of the INIC reset the counter also.
  *
  *             If MostCountCodingErros() was called before, the callback returns the result
  *             of an advanced measurement and resets the counter to zero. If the advanced
  *             measurement was already finished, the next call to MostGetCodingErrors()
  *             will return the counter of the INIC as mentioned above, otherwise the
  *             the advanced measurement continues with a reset counter.
  *
  *             Giving NULL as a parameter resets the counter inside the INIC.
  *
  * \return     ERR_NO if successful, ERR_BUFOV if busy
  */
byte MostGetCodingErrors(TWmcsCodingErrorCB *cb_ptr)
{
    byte result;
    bool busy;

    T_API_ENTRY(WMCS_28);

    result = ERR_NO;

    TAKE_WMCS();
    busy = (wmcs.coding_errors.cb_ptr || (MNS_FALSE != wmcs.coding_errors.busy_reset)) ? MNS_TRUE : MNS_FALSE;
    GIVE_WMCS();

    if (MNS_FALSE != busy)
    {
        if (cb_ptr)
        {
            /* busy: application has to retry later */
            result = ERR_BUFOV;
            cb_ptr(NSR_E_BUSY, 0);
        }

        /* if this was a call using NULL, we do not have to retry since the
           already pending job will reset the counter anyway */
    }
    else if((TWmcsCodingErrorCB*)WmcsCodingErrorsCallback == cb_ptr)
    {
        /* internal call */
        TAKE_WMCS();
        wmcs.coding_errors.cb_ptr = cb_ptr;
        GIVE_WMCS();
        (void)WmcsRequestCodingErrors();
    }
    else
    {
        TMnsResult code          = NSR_S_CE_INIC;  /* normal procedure */
        word       coding_errors = 0;

        /* if this is not a reset action */
        if (cb_ptr)
        {
            /* external call: check if we have an extended job running */
            TAKE_WMCS();
            if(WMCS_CE_INACTIVE != wmcs.coding_errors.counter)
            {
                /* there is a running or finished job */
                coding_errors = (word) wmcs.coding_errors.counter;

                /* is the job finished? */
                if((word)0 != wmcs.coding_errors.deadtime)
                {
                    /* no, it is not: just reset counter to zero */
                    wmcs.coding_errors.counter = 0;
                    code                       = NSR_S_CE_EXT_ONGOING;
                }
                else
                {
                    /* yes it is: flag inactive */
                    wmcs.coding_errors.counter = WMCS_CE_INACTIVE;
                    code                       = NSR_S_CE_EXT_RESULT;
                }
            }
            GIVE_WMCS();
        }
        else
        {
            /* this is a reset operation */
            TAKE_WMCS();
            wmcs.coding_errors.busy_reset = MNS_TRUE;

            if(WMCS_CE_INACTIVE != wmcs.coding_errors.counter)
            {
                wmcs.coding_errors.counter = 0;
            }
            GIVE_WMCS();
        }

        /* do we have an extended job result? */
        if(NSR_S_CE_INIC != code)
        {
            /* call the callback immediately with the extended result */
            if (cb_ptr)
            {
                cb_ptr(code, coding_errors);
            }
        }
        else
        {
            /* normal procedure */
            TAKE_WMCS();
            /* can be NULL in case of a reset, the field busy_rest is also
               MNS_TRUE in that case */
            wmcs.coding_errors.cb_ptr = cb_ptr;
            GIVE_WMCS();
            (void)WmcsRequestCodingErrors();
        }

    }
    T_API_EXIT(WMCS_28);

    return(result);
}
#endif

#ifdef WMCS_29
/*!
  * \brief      API function to start advanced counting for coding errors.
  *
  * \details    Coding errors counted through "deadtime" are being counted as
  *             one. Measurement runs for "timeout". The next call to MostGetCodingErrors()
  *             is going to return the result of this advanced counting implementation. Calling
  *             with parameters "0, 0" discards an active measurement and resets MostGetCodingErros()
  *             to normal mode.
  *
  * \param[in]  timeout   description for parameter timeout
  * \return     ERR_NO if successful, ERR_PARAM if deadtime is larger than
  *             timeout
  */
byte MostCountCodingErrors(word timeout)
{
    byte result;
    bool clear_timer;

    T_API_ENTRY(WMCS_29);

    result      = ERR_NO;
    clear_timer = MNS_FALSE;

    /* check if we are already running */
    TAKE_WMCS();
    if(WMCS_CE_INACTIVE != wmcs.coding_errors.counter)
    {
        /* reset running job completely */
        wmcs.coding_errors.counter  = WMCS_CE_INACTIVE;
        wmcs.coding_errors.timeout  = 0;
        wmcs.coding_errors.deadtime = 0;
        clear_timer                 = MNS_TRUE;
    }
    GIVE_WMCS();
    if(MNS_FALSE != clear_timer)
    {
        MostClearTimer(&(wmcs.coding_errors.timer));
    }

    /* do we want to start another job? */
    if((word)0 != timeout)
    {
        /*reset counter in INIC if possible, WmcsCodingErrorCallback will be
          called*/
        result = MostGetCodingErrors(WmcsCodingErrorsCallback);
        if(ERR_NO == result)
        {
            /* new params are valid? */
            if (timeout  < WMCS_CE_TIMEOUT_MIN)
            {
                result = ERR_PARAM;
            }
            else
            {
                /* set the parameters, so WmcsCodingErrorsCallback can start
                   the new job */
                TAKE_WMCS();
                wmcs.coding_errors.timeout  = timeout;
                if ((word)0xFFFF != timeout)
                {
                    wmcs.coding_errors.deadtime = timeout;
                }
                else
                {
                    wmcs.coding_errors.deadtime = (word)1000;
                }
                GIVE_WMCS();
            }
        }
    }

    T_API_EXIT(WMCS_29);
    return(result);
}
#endif

#ifdef WMCS_30
/*!
  * \brief      Internal callback function receives coding error result from INIC.
  *
  * \param[in]  result description for parameter result
  * \param[in]  coding_errors description for parameter coding_errors
  * \return     nothing
  */
static void WmcsCodingErrorsCallback(TMnsResult result, word coding_errors)
{
    word set_timer;

    T_MOD_ENTRY(WMCS_30);

    (void) result;
    set_timer = 0;
    TAKE_WMCS();

    /* is this the callback from the "reset" stage of a new job? */
    if((WMCS_CE_INACTIVE == wmcs.coding_errors.counter) && ((word)0 != wmcs.coding_errors.deadtime))
    {
        /* start new job by setting counter to zero and start deadtime timer,
           when it times out WmcsCodingErrorsTimeout is going to be called */
        wmcs.coding_errors.counter = (word)0;
        if((word)0xFFFF != wmcs.coding_errors.timeout)
        {
            wmcs.coding_errors.timeout -= wmcs.coding_errors.deadtime;
        }
        set_timer = wmcs.coding_errors.deadtime;
    }
    else if((WMCS_CE_INACTIVE != wmcs.coding_errors.counter) && ((word)0 != coding_errors))
    {
        /* this is a call during a job, just increase the counter if there were coding errors */
        dword temp;

        temp = (dword)wmcs.coding_errors.counter + (dword)coding_errors;

        if ((dword)WMCS_CE_INACTIVE <= temp)                /* check for overflow */
        {
            wmcs.coding_errors.counter = WMCS_CE_INACTIVE - (word)1;
        }
        else
        {
            wmcs.coding_errors.counter = (word)temp;
        }
    }

    GIVE_WMCS();

    if(set_timer)
    {
        MostSetTimer(&(wmcs.coding_errors.timer), set_timer, MNS_FALSE);
    }

    T_MOD_EXIT(WMCS_30);
}
#endif

#ifdef WMCS_31
static void WmcsCodingErrorsTimeout(void)
{
    word new_deadtime;
    word counter;
    word timeout;

    /* this is a deadtime timeout */

    T_MOD_ENTRY(WMCS_31);
    new_deadtime = 0;
    TAKE_WMCS();
    counter = wmcs.coding_errors.counter;
    timeout = wmcs.coding_errors.timeout;
    GIVE_WMCS();

    /* if we are not active, we should not be here */
    ASSERT(WMCS_CE_INACTIVE != counter);
    if(WMCS_CE_INACTIVE != counter)
    {
        /* ask the INIC for coding errors during deadtime (resets the counter) */
        if(ERR_NO != MostGetCodingErrors(WmcsCodingErrorsCallback))
        {
            /* we got a problem, lets try again next service run */
            WmcsSetPendingEvent(WMCS_P_CE_TIMER);
        }
        else if((word)0 != timeout)
        {
            /* time is not yet over, so we schedule a new deadtime */

            TAKE_WMCS();
            if((word)0xFFFF != wmcs.coding_errors.timeout)
            {
                if(wmcs.coding_errors.timeout > wmcs.coding_errors.deadtime)
                {
                    new_deadtime =  wmcs.coding_errors.deadtime;
                }
                else
                {
                    new_deadtime =  wmcs.coding_errors.timeout;
                }
                wmcs.coding_errors.timeout -= new_deadtime;
            }
            else
            {
                new_deadtime =  wmcs.coding_errors.deadtime;
            }
            GIVE_WMCS();

            MostSetTimer(&(wmcs.coding_errors.timer), new_deadtime, MNS_FALSE);
        }
        else
        {
            /* this was the last deadtime period, we reset deadtime also to
               signal the job was finished */
            TAKE_WMCS();
            wmcs.coding_errors.deadtime = 0;
            GIVE_WMCS();
        }
    }

    T_MOD_EXIT(WMCS_31);
}
#endif



#ifdef WMCS_33
/*! API function to read Configuration String
  *
  * @author     RWilhelm
  * @updated    08/25/2006
  * @version    3.0.2
  *
  * @return     ERR_NO if successful,
  *             ERR_PARAM if callback pointer is NULL
  *                       or no memory pointer given
  *                       memory area too small
  *             ERR_BUFOV if no msg buffer available
  */
byte MostReadConfString(TWmcsReadConfStringCB *cb_ptr,
                        byte *conf_string,
                        byte len)
{
    byte   result;
    TMsgTx *msg_ptr;

    T_API_ENTRY(WMCS_33);

    result = ERR_NO;

    TAKE_WMCS();
    if (MNS_FALSE != wmcs.memory.pending)
    {
        result = ERR_BUFOV;                                /* pending request */
    }
    GIVE_WMCS();

    if (ERR_NO == result)
    {
        if (cb_ptr)
        {
            TAKE_WMCS();
            wmcs.memory.cs_cb_ptr = cb_ptr;
            GIVE_WMCS();
        }
        else
        {
            result = ERR_PARAM;                  /* no callback pointer given */
        }
    }

    if (ERR_NO == result)
    {
        if (conf_string)
        {
            TAKE_WMCS();
            wmcs.memory.conf_string = conf_string;
            GIVE_WMCS();
        }
        else
        {
            result = ERR_PARAM;                       /* no memory area given */
        }
    }

    if (ERR_NO == result)
    {
        if (MNS_FALSE != MostIsSupported(NSF_ROM))
        {
            if (len < INIC_MEM_CS_LEN)
            {
                result = ERR_PARAM;                      /* memory area too small */
            }
        }
        else
        {
            if (len < INIC_MEM_CS_LEN * NUM_CS_SEGMENTS)
            {
                result = ERR_PARAM;                      /* memory area too small */
            }
        }

        TAKE_WMCS();
        wmcs.memory.cs_msg_num = 0;
        GIVE_WMCS();
    }

    if (ERR_NO == result)
    {
        if (MNS_FALSE != MostIsSupported(NSF_ROM))
        {
            msg_ptr = MsgGetTxPtrExt(1);
            if (msg_ptr)
            {
                TAKE_WMCS();
                wmcs.memory.pending = MNS_TRUE;
                GIVE_WMCS();

                msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
                msg_ptr->FBlock_ID = FBLOCK_INIC;
                msg_ptr->Inst_ID   = 0;
                msg_ptr->Func_ID   = FUNCID_INIC_CONF_STRING;
                msg_ptr->Operation = OP_STARTRESULT;
                msg_ptr->Data[0]   = MEM_READ_TYPE_CONF;

                MsgSend3(msg_ptr);
            }
            else
            {
                result = ERR_BUFOV;                     /* no Tx buffer available */
            }
        }
        else
        {
            msg_ptr = MsgGetTxPtrExt(5);
            if (msg_ptr)
            {
                TAKE_WMCS();
                wmcs.memory.pending = MNS_TRUE;
                GIVE_WMCS();

                msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
                msg_ptr->FBlock_ID = FBLOCK_INIC;
                msg_ptr->Inst_ID   = 0;
                msg_ptr->Func_ID   = FUNCID_INIC_MEMORY;
                msg_ptr->Operation = OP_GET;
                msg_ptr->Data[0]   = (byte)((INIC_MEM_CONF_STRING >> 24) & 0x000000FF);
                msg_ptr->Data[1]   = (byte)((INIC_MEM_CONF_STRING >> 16) & 0x000000FF);
                msg_ptr->Data[2]   = (byte)((INIC_MEM_CONF_STRING >>  8) & 0x000000FF);
                msg_ptr->Data[3]   = (byte) (INIC_MEM_CONF_STRING        & 0x000000FF);
                msg_ptr->Data[4]   = INIC_MEM_CS_LEN;

                MsgSend3(msg_ptr);
            }
            else
            {
                result = ERR_BUFOV;                     /* no Tx buffer available */
            }
        }
    }
    T_API_EXIT(WMCS_33);

    return(result);
}
#endif


#ifdef WMCS_34
/*! Internal helper function to parse INIC.Memory.Status.
  *
  * @author     RWilhelm
  * @updated    11/20/2006
  * @version    2.1.1
  *
  * @return     nothing
  */
void WmcsHandleMemoryStatus(TMsgRx *msg_ptr)
{
    dword use_case_id;

    T_MOD_ENTRY(WMCS_34);

    ASSERT(msg_ptr);

    use_case_id =     (dword)((dword)msg_ptr->Data[0] << 24L)
                    + (dword)((dword)msg_ptr->Data[1] << 16L)
                    + (dword)((dword)msg_ptr->Data[2] <<  8L)
                    + (dword)((dword)msg_ptr->Data[3]);

    if (INIC_MEM_CONF_STRING == (use_case_id & 0xFFFFFF00))  /* Conf. String request */
    {
        byte *src;
        byte *tgt;
        byte cs_msg_num;
        byte i;

        src = &msg_ptr->Data[WMCS_MEMORY_OFFSET];
        TAKE_WMCS();
        tgt = wmcs.memory.conf_string + (byte)(use_case_id & 0xFF);
        GIVE_WMCS();

        /* assemble ConfigurationString */
        for (i=0; i<INIC_MEM_CS_LEN; ++i)
        {
            *tgt++ = *src++;
        }
        TAKE_WMCS();
        cs_msg_num = wmcs.memory.cs_msg_num;
        GIVE_WMCS();

        ++cs_msg_num;
        if (NUM_CS_SEGMENTS > cs_msg_num) /* more segments to be requested? */
        {
            TMsgTx *tx_ptr;
            dword  mem_addr = use_case_id + INIC_MEM_CS_LEN;

            tx_ptr = MsgGetTxPtrExt(5);
            if (tx_ptr)
            {
                tx_ptr->Tgt_Adr   = MSG_TGT_INIC;
                tx_ptr->FBlock_ID = FBLOCK_INIC;
                tx_ptr->Inst_ID   = 0;
                tx_ptr->Func_ID   = FUNCID_INIC_MEMORY;
                tx_ptr->Operation = OP_GET;
                tx_ptr->Data[0]   = (byte)((mem_addr >> 24) & 0x000000FF);
                tx_ptr->Data[1]   = (byte)((mem_addr >> 16) & 0x000000FF);
                tx_ptr->Data[2]   = (byte)((mem_addr >>  8) & 0x000000FF);
                tx_ptr->Data[3]   = (byte) (mem_addr        & 0x000000FF);
                tx_ptr->Data[4]   = INIC_MEM_CS_LEN;

                MsgSend3(tx_ptr);  /* request next ConfigString segment */

                TAKE_WMCS();
                wmcs.memory.cs_msg_num = cs_msg_num;
                GIVE_WMCS();
            }
            else
            {
                TWmcsReadConfStringCB   *cb_ptr = NULL;

                TAKE_WMCS();
                cb_ptr      = wmcs.memory.cs_cb_ptr;
                GIVE_WMCS();

                ASSERT(cb_ptr);

                if (cb_ptr)
                {
                    cb_ptr(NSR_E_MEMORY_BUF_OV, NULL, 0);
                }

                TAKE_WMCS();
                wmcs.memory.cs_cb_ptr = NULL;   /* for distinction in error case */
                wmcs.memory.pending   = MNS_FALSE;
                GIVE_WMCS();
            }

        }
        else /* all segments received, report to application */
        {
            TWmcsReadConfStringCB   *cb_ptr = NULL;
            byte *conf_string;
            dword len;

            TAKE_WMCS();
            cb_ptr      = wmcs.memory.cs_cb_ptr;
            conf_string = (byte*)wmcs.memory.conf_string;
            len         = use_case_id;
            len        += INIC_MEM_CS_LEN;
            len        -= INIC_MEM_CONF_STRING;
            GIVE_WMCS();

            ASSERT(cb_ptr);

            if (cb_ptr)
            {
                cb_ptr(NSR_S_OK, conf_string,(byte)len);
            }

            TAKE_WMCS();
            wmcs.memory.cs_cb_ptr = NULL;    /* for distinction in error case */
            wmcs.memory.pending   = MNS_FALSE;
            GIVE_WMCS();
        }

    }
    else                                                 /* invalid UseCaseID */
    {
        FAILED_ASSERT();
        TAKE_WMCS();
        wmcs.memory.cs_cb_ptr = NULL;
        wmcs.memory.pending   = MNS_FALSE;
        GIVE_WMCS();
    }

    T_MOD_EXIT(WMCS_34);
}
#endif

#ifdef WMCS_35
/*! Internal helper function to parse INIC.Memory.Error.
  *
  * @author     RWilhelm
  * @updated    28/04/2010
  * @version    3.0.3
  *
  * @return     nothing
  */
void WmcsHandleMemoryError(TMsgRx *msg_ptr)
{
    TMnsResult             result;
    TWmcsReadConfStringCB  *cs_cb_ptr;

    T_MOD_ENTRY(WMCS_35);
    ASSERT(msg_ptr);

    result    = NSR_BUILD(MNS_FALSE, 0xA0, msg_ptr->Data[0]);

    TAKE_WMCS();
    cs_cb_ptr = wmcs.memory.cs_cb_ptr;
    GIVE_WMCS();

    if (cs_cb_ptr)
    {
        cs_cb_ptr(result, NULL, 0);

        TAKE_WMCS();
        wmcs.memory.cs_cb_ptr = NULL;        /* for distinction in error case */
        GIVE_WMCS();
    }
    else  /* invalid UseCaseID, error message without pending request */
    {
        FAILED_ASSERT();
    }

    TAKE_WMCS();
    wmcs.memory.pending   = MNS_FALSE;
    GIVE_WMCS();

    T_MOD_EXIT(WMCS_35);
}
#endif








#ifdef WMCS_42
/*! API function to enable various options for the destination
  * address match logic for MOST Ethernet packets
  *
  * @author     RWilhelm
  * @updated    09/10/2007
  * @version    2.5.0
  *
  * @param      mode         filter mode
  * @param      cb_ptr       pointer to result callback function
  *
  * @return     ERR_BUFOV if no msg buffer available
  *             ERR_PARAM no callback pointer given
  */
byte MostSetMEPFilterMode(word mode, TWmcsMEPFilterModeCB *cb_ptr)
{
    byte       result;
    TMnsResult cb_res;
    TMsgTx     *msg_ptr;

    T_API_ENTRY(WMCS_42);

    result = ERR_NO;
    cb_res = NSR_S_OK;
    TAKE_WMCS();
    if (MNS_FALSE != wmcs.mepfiltermode.pending)
    {
        result = ERR_BUFOV;                                /* pending request */
        cb_res = NSR_E_BUSY;
    }
    GIVE_WMCS();

    if (ERR_NO == result)
    {
        if (cb_ptr)
        {
            TAKE_WMCS();
            wmcs.mepfiltermode.cb_ptr = cb_ptr;
            GIVE_WMCS();
        }
        else
        {
            result = ERR_PARAM;                  /* no callback pointer given */
        }
    }

    if (ERR_NO == result)
    {
        msg_ptr = MsgGetTxPtrExt(2);
        if (msg_ptr)
        {
            msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
            msg_ptr->FBlock_ID = FBLOCK_INIC;
            msg_ptr->Inst_ID   = 0;
            msg_ptr->Func_ID   = FUNCID_INIC_MEPFILTERMODE;
            msg_ptr->Operation = OP_SETGET;
            msg_ptr->Data[0]   = (byte)((mode >> 8) & 0x00FF);
            msg_ptr->Data[1]   = (byte)(mode & 0x00FF);

            MsgSend3(msg_ptr);

            TAKE_WMCS();
            wmcs.mepfiltermode.pending = MNS_TRUE;
            GIVE_WMCS();
        }
        else
        {
            result = ERR_BUFOV;                     /* no Tx buffer available */
            cb_res = NSR_E_BUSY;
        }
    }

    if (ERR_NO != result)
    {
        if (cb_ptr)
        {
            cb_ptr(cb_res, 0);
        }
    }

    T_API_EXIT(WMCS_42);

    return(result);
}
#endif

#ifdef WMCS_43
/*! API function to read the various options for the destination
  * address match logic for MOST Ethernet packets
  *
  * @author     RWilhelm
  * @updated    09/11/2007
  * @version    2.5.0
  *
  * @param      cb_ptr      callback function to present the result
  *
  * @return     ERR_PARAM   no callback pointer given
  *             ERR_BUFOV if no msg buffer available
  */
byte MostGetMEPFilterMode(TWmcsMEPFilterModeCB *cb_ptr)
{
    byte       result;
    TMnsResult cb_res;
    TMsgTx     *msg_ptr;

    T_API_ENTRY(WMCS_43);

    result = ERR_NO;
    cb_res = NSR_S_OK;
    TAKE_WMCS();
    if (MNS_FALSE != wmcs.mepfiltermode.pending)
    {
        result = ERR_BUFOV;                                /* pending request */
        cb_res = NSR_E_BUSY;
    }
    GIVE_WMCS();

    if (ERR_NO == result)
    {
        if (cb_ptr)
        {
            TAKE_WMCS();
            wmcs.mepfiltermode.cb_ptr = cb_ptr;
            GIVE_WMCS();
        }
        else
        {
            result = ERR_PARAM;                  /* no callback pointer given */
        }
    }

    if (ERR_NO == result)
    {
        msg_ptr = MsgGetTxPtrExt(0);
        if (msg_ptr)
        {
            msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
            msg_ptr->FBlock_ID = FBLOCK_INIC;
            msg_ptr->Inst_ID   = 0;
            msg_ptr->Func_ID   = FUNCID_INIC_MEPFILTERMODE;
            msg_ptr->Operation = OP_GET;

            MsgSend3(msg_ptr);

            TAKE_WMCS();
            wmcs.mepfiltermode.pending = MNS_TRUE;
            GIVE_WMCS();
        }
        else
        {
            result = ERR_BUFOV;                     /* no Tx buffer available */
            cb_res = NSR_E_BUSY;
        }
    }

    if (ERR_NO != result)
    {
        if (cb_ptr)
        {
            cb_ptr(cb_res, 0);
        }
    }


    T_API_EXIT(WMCS_43);

    return(result);
}
#endif


#ifdef WMCS_44
/*! Internal helper function to parse INIC.MEPFilterMode.Status
  *
  * @author     RWilhelm
  * @updated    9/11/2007
  * @version    2.5.0
  *
  * @param      msg_ptr  status message from INIC
  *
  * @return     nothing
  */
void WmcsHandleMEPFilterModeStatus(TMsgRx *msg_ptr)
{
    TWmcsMEPFilterModeCB *cb_ptr;

    T_MOD_ENTRY(WMCS_44);

    ASSERT(msg_ptr);

    TAKE_WMCS();
    cb_ptr = wmcs.mepfiltermode.cb_ptr;
    wmcs.mepfiltermode.cb_ptr     = NULL;
    wmcs.mepfiltermode.pending    = MNS_FALSE;
    GIVE_WMCS();

    if(cb_ptr)
    {
        cb_ptr(NSR_S_OK, (word)((msg_ptr->Data[0] << 8) | msg_ptr->Data[1]));
    }

    T_MOD_EXIT(WMCS_44);
}
#endif



#ifdef WMCS_45
/*! Internal helper function to parse INIC.MEPFilterMode.Error
  *
  * @author     RWilhelm
  * @updated    9/11/2007
  * @version    2.5.0
  *
  * @param      msg_ptr  status message from INIC
  *
  * @return     nothing
  */
void WmcsHandleMEPFilterModeError(TMsgRx *msg_ptr)
{
    TMnsResult           result;
    TWmcsMEPFilterModeCB *cb_ptr;

    T_MOD_ENTRY(WMCS_45);

    ASSERT(msg_ptr);
    result  = NSR_BUILD(MNS_FALSE, 0xA0, msg_ptr->Data[0]);

    TAKE_WMCS();
    cb_ptr = wmcs.mepfiltermode.cb_ptr;
    GIVE_WMCS();

    if (cb_ptr)                         /* call respective callback function */
    {
        cb_ptr(result, 0);

        TAKE_WMCS();
        wmcs.mepfiltermode.cb_ptr = NULL;    /* for distinction in error case */
        GIVE_WMCS();
    }
    else                             /* error message without pending request */
    {
        FAILED_ASSERT();
    }

    TAKE_WMCS();
    wmcs.mepfiltermode.pending = MNS_FALSE;
    GIVE_WMCS();

    T_MOD_EXIT(WMCS_45);
}
#endif


#ifdef WMCS_46
/*! API function to access the hash table to allow receiption of
  * multicast Ethernet frames
  *
  * @author     RWilhelm
  * @updated    09/10/2007
  * @version    2.5.0
  *
  * @param      hash          pointer to hash table
  * @param      cb_ptr      callback function to present a possible error
  *
  * @return     ERR_NO if successful,
  *             ERR_PARAM if hash pointer is NULL
  *                       or no callbackpointer given
  *             ERR_BUFOV if no msg buffer available
  *                       or a GET request is pending
  */
byte MostSetMEPHashTable(word *hash, TWmcsMEPHashTableCB *cb_ptr)
{
    byte       result;
    TMnsResult cb_res;
    TMsgTx     *msg_ptr;
    word       temp;

    T_API_ENTRY(WMCS_46);

    result = ERR_NO;
    cb_res = NSR_S_OK;
    TAKE_WMCS();
    if (MNS_FALSE != wmcs.mephashtable.pending)
    {
        result = ERR_BUFOV;                                /* pending request */
        cb_res = NSR_E_BUSY;
    }
    GIVE_WMCS();

    if (ERR_NO == result)
    {
        if (cb_ptr)
        {
            TAKE_WMCS();
            wmcs.mephashtable.cb_ptr = cb_ptr;
            GIVE_WMCS();
        }
        else
        {
            result = ERR_PARAM;                  /* no callback pointer given */
        }
    }

    if (ERR_NO == result)
    {
        if (NULL == hash)
        {
            result = ERR_PARAM;
            cb_res = NSR_E_PARAM;
        }
    }

    if (ERR_NO == result)
    {
        msg_ptr = MsgGetTxPtrExt(8);
        if (msg_ptr)
        {
            msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
            msg_ptr->FBlock_ID = FBLOCK_INIC;
            msg_ptr->Inst_ID   = 0;
            msg_ptr->Func_ID   = FUNCID_INIC_MEPHASHTABLE;
            msg_ptr->Operation = OP_SETGET;
            temp = *hash++;
            msg_ptr->Data[0]   = (byte)((temp >> 8) & 0x00FF);
            msg_ptr->Data[1]   = (byte)( temp & 0x00FF);
            temp = *hash++;
            msg_ptr->Data[2]   = (byte)((temp >> 8) & 0x00FF);
            msg_ptr->Data[3]   = (byte)( temp & 0x00FF);
            temp = *hash++;
            msg_ptr->Data[4]   = (byte)((temp >> 8) & 0x00FF);
            msg_ptr->Data[5]   = (byte)( temp & 0x00FF);
            temp = *hash;
            msg_ptr->Data[6]   = (byte)((temp >> 8) & 0x00FF);
            msg_ptr->Data[7]   = (byte)( temp & 0x00FF);

            MsgSend3(msg_ptr);

            TAKE_WMCS();
            wmcs.mephashtable.pending = MNS_TRUE;
            GIVE_WMCS();
        }
        else
        {
            result = ERR_BUFOV;                     /* no Tx buffer available */
            cb_res = NSR_E_BUSY;
        }
    }

    if (ERR_NO != result)
    {
        if (cb_ptr)
        {
            cb_ptr(cb_res, 0);
        }
    }

    T_API_EXIT(WMCS_46);

    return(result);
}
#endif


#ifdef WMCS_47
/*! API function to read the hash table which allows receiption of
  * multicast Ethernet frames
  *
  * @author     RWilhelm
  * @updated    09/12/2007
  * @version    2.5.0
  *
  * @param      cb_ptr      callback function to present the result
  *
  * @return     ERR_PARAM   no callback pointer given
  *             ERR_BUFOV   request pending or no Tx buffer available
  */
byte MostGetMEPHashTable(TWmcsMEPHashTableCB *cb_ptr)
{
    byte       result;
    TMnsResult cb_res;
    TMsgTx     *msg_ptr;

    T_API_ENTRY(WMCS_47);

    result = ERR_NO;
    cb_res = NSR_S_OK;
    TAKE_WMCS();
    if (MNS_FALSE != wmcs.mephashtable.pending)
    {
        result = ERR_BUFOV;                                /* pending request */
        cb_res = NSR_E_BUSY;
    }
    GIVE_WMCS();

    if (ERR_NO == result)
    {
        if (cb_ptr)
        {
            TAKE_WMCS();
            wmcs.mephashtable.cb_ptr = cb_ptr;
            GIVE_WMCS();
        }
        else
        {
            result = ERR_PARAM;                  /* no callback pointer given */
        }
    }

    if (ERR_NO == result)
    {
        msg_ptr = MsgGetTxPtrExt(0);
        if (msg_ptr)
        {
            msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
            msg_ptr->FBlock_ID = FBLOCK_INIC;
            msg_ptr->Inst_ID   = 0;
            msg_ptr->Func_ID   = FUNCID_INIC_MEPHASHTABLE;
            msg_ptr->Operation = OP_GET;

            MsgSend3(msg_ptr);

            TAKE_WMCS();
            wmcs.mephashtable.pending = MNS_TRUE;
            GIVE_WMCS();
        }
        else
        {
            result = ERR_BUFOV;                     /* no Tx buffer available */
            cb_res = NSR_E_BUSY;
        }
    }

    if (ERR_NO != result)
    {
        if (cb_ptr)
        {
            cb_ptr(cb_res, 0);
        }
    }

    T_API_EXIT(WMCS_47);

    return(result);
}
#endif

#ifdef WMCS_48
/*! Internal helper function to parse INIC.MEPHashTable.Status
  *
  * @author     RWilhelm
  * @updated    9/12/2007
  * @version    2.5.0
  *
  * @param      msg_ptr  status message from INIC
  *
  * @return     nothing
  */
void WmcsHandleMEPHashTableStatus(TMsgRx *msg_ptr)
{
    TWmcsMEPHashTableCB *cb_ptr;
    word hash[4];
    byte i;

    T_MOD_ENTRY(WMCS_48);

    ASSERT(msg_ptr);

    TAKE_WMCS();
    cb_ptr = wmcs.mephashtable.cb_ptr;
    wmcs.mephashtable.cb_ptr     = NULL;
    wmcs.mephashtable.pending    = MNS_FALSE;
    GIVE_WMCS();

    if(cb_ptr)
    {
        for (i=0; i<4; ++i)
        {
            hash[i] = (word)((msg_ptr->Data[2*i] << 8) | msg_ptr->Data[2*i+1]);
        }

        cb_ptr(NSR_S_OK, &hash[0]);
    }

    T_MOD_EXIT(WMCS_48);
}
#endif



#ifdef WMCS_49
/*! Internal helper function to parse INIC.MEPHashTable.Error
  *
  * @author     RWilhelm
  * @updated    9/12/2007
  * @version    2.5.0
  *
  * @param      msg_ptr  status message from INIC
  *
  * @return     nothing
  */
void WmcsHandleMEPHashTableError(TMsgRx *msg_ptr)
{
    TMnsResult          result;
    TWmcsMEPHashTableCB *cb_ptr;

    T_MOD_ENTRY(WMCS_49);

    ASSERT(msg_ptr);
    result  = NSR_BUILD(MNS_FALSE, 0xA0, msg_ptr->Data[0]);

    TAKE_WMCS();
    cb_ptr = wmcs.mephashtable.cb_ptr;
    GIVE_WMCS();

    if (cb_ptr)                         /* call respective callback function */
    {
        cb_ptr(result, NULL);

        TAKE_WMCS();
        wmcs.mephashtable.cb_ptr = NULL;     /* for distinction in error case */
        GIVE_WMCS();
    }
    else                             /* error message without pending request */
    {
        FAILED_ASSERT();
    }

    TAKE_WMCS();
    wmcs.mephashtable.pending = MNS_FALSE;
    GIVE_WMCS();

    T_MOD_EXIT(WMCS_49);
}
#endif




#ifdef WMCS_50
/*! API function to write the MAC address (NetBlock.EUI48)
  *
  * @author     RWilhelm
  * @updated    09/10/2007
  * @version    2.5.0
  *
  * @param      cb_ptr      pointer to result callback function
  * @param      eui         pointer to eui value
  * @param      persist     indicates if EUI48 has to be stored persistently
  *
  * @return     ERR_NO      if successful,
  *             ERR_PARAM   pointer eui is NULL or
  *                         or no callback pointer given
  *             ERR_BUFOV   if no msg buffer available
  *                         or another request is pending
  */
byte MostSetEUI48(byte *eui, bool persist, TWmcsEUI48CB *cb_ptr)
{
    byte       result;
    TMnsResult cb_res;
    TMsgTx     *msg_ptr;

    T_API_ENTRY(WMCS_50);

    result = ERR_NO;
    cb_res = NSR_S_OK;
    TAKE_WMCS();
    if (MNS_FALSE != wmcs.eui48.pending)
    {
        result = ERR_BUFOV;                                /* pending request */
        cb_res = NSR_E_BUSY;
    }
    GIVE_WMCS();

    if (ERR_NO == result)
    {
        if (NULL == eui)
        {
            result = ERR_PARAM;
            cb_res = NSR_E_PARAM;
        }
    }

    if (ERR_NO == result)
    {
        if (cb_ptr)
        {
            TAKE_WMCS();
            wmcs.eui48.cb_ptr = cb_ptr;
            GIVE_WMCS();
        }
        else
        {
            result = ERR_PARAM;                  /* no callback pointer given */
        }
    }


    if (ERR_NO == result)
    {
        msg_ptr = MsgGetTxPtrExt(7);
        if (msg_ptr)
        {
            msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
            msg_ptr->FBlock_ID = FBLOCK_INIC;
            msg_ptr->Inst_ID   = 0;
            msg_ptr->Func_ID   = FUNCID_INIC_SETEUI48;
            msg_ptr->Operation = OP_STARTRESULT;
            msg_ptr->Data[0]   = (byte)(*eui++);
            msg_ptr->Data[1]   = (byte)(*eui++);
            msg_ptr->Data[2]   = (byte)(*eui++);
            msg_ptr->Data[3]   = (byte)(*eui++);
            msg_ptr->Data[4]   = (byte)(*eui++);
            msg_ptr->Data[5]   = (byte)(*eui);
            msg_ptr->Data[6]   = (MNS_FALSE != persist) ? (byte)0x01 : (byte)0x00;

            MsgSend3(msg_ptr);

            TAKE_WMCS();
            wmcs.eui48.pending = MNS_TRUE;
            GIVE_WMCS();
        }
        else
        {
            result = ERR_BUFOV;                     /* no Tx buffer available */
            cb_res = NSR_E_BUSY;
        }
    }

    if (ERR_NO != result)
    {
        if (cb_ptr)
        {
            cb_ptr(cb_res, NULL);
        }
    }

    T_API_EXIT(WMCS_50);

    return(result);
}
#endif

#ifdef WMCS_51
/*! Internal helper function to parse INIC.SetEUI48.Result
  *
  * @author     RWilhelm
  * @updated    9/12/2007
  * @version    2.5.0
  *
  * @param      msg_ptr  status message from INIC
  *
  * @return     nothing
  */
void WmcsHandleSetEUI48Result(TMsgRx *msg_ptr)
{
    TWmcsEUI48CB  *cb_ptr;
    byte          eui[6];
    byte          i;

    T_MOD_ENTRY(WMCS_51);

    ASSERT(msg_ptr);

    TAKE_WMCS();
    cb_ptr = wmcs.eui48.cb_ptr;

    for (i=0; i<6; ++i)              /* store the result in shadow array for access with MostGetEUI48 */
    {
        wmcs.eui48_shadow[i] = msg_ptr->Data[i];
    }
    GIVE_WMCS();

    if (cb_ptr)                       /* call respective callback function */
    {
        for (i=0; i<6; ++i)
        {
            eui[i] = msg_ptr->Data[i];
        }

        cb_ptr(NSR_S_OK, &eui[0]);

        TAKE_WMCS();
        wmcs.eui48.cb_ptr = NULL;        /* for distinction in error case */
        GIVE_WMCS();
    }
    else  /* error message without pending request */
    {
        FAILED_ASSERT();
    }

    TAKE_WMCS();
    wmcs.eui48.pending = MNS_FALSE;
    GIVE_WMCS();

    T_MOD_EXIT(WMCS_51);

}
#endif

#ifdef WMCS_52
/*! Internal helper function to parse INIC.SetEUI48.Error
  *
  * @author     RWilhelm
  * @updated    9/11/2007
  * @version    2.5.0
  *
  * @param      msg_ptr  status message from INIC
  *
  * @return     nothing
  */
void WmcsHandleSetEUI48Error(TMsgRx *msg_ptr)
{
    TMnsResult    result;
    TWmcsEUI48CB  *cb_ptr;

    T_MOD_ENTRY(WMCS_52);

    ASSERT(msg_ptr);
    result   = NSR_BUILD(MNS_FALSE, 0xA0, msg_ptr->Data[0]);

    TAKE_WMCS();
    cb_ptr = wmcs.eui48.cb_ptr;
    GIVE_WMCS();

    if (cb_ptr)                         /* call respective callback function */
    {
        cb_ptr(result, NULL);

        TAKE_WMCS();
        wmcs.eui48.cb_ptr = NULL;           /* for distinction in error case */
        GIVE_WMCS();
    }
    else                            /* error message without pending request */
    {
        FAILED_ASSERT();
    }

    TAKE_WMCS();
    wmcs.eui48.pending = MNS_FALSE;
    GIVE_WMCS();

    T_MOD_EXIT(WMCS_52);

}
#endif


#ifdef WMCS_53
/*! API function to set the wait time for low level retries
  * between data packets
  *
  * @author     RWilhelm
  * @updated    09/12/2007
  * @version    2.5.0
  *
  * @param      time         wait time
  * @param      cb_ptr      callback function to present a possible error
  *
  * @return     ERR_BUFOV    request pending or no Tx buffer available
  */
byte MostSetPacketRetryTime(byte time, TWmcsPacketRetryTimeCB *cb_ptr)
{
    byte       result;
    TMnsResult cb_res;
    TMsgTx     *msg_ptr;

    T_API_ENTRY(WMCS_53);

    result = ERR_NO;
    cb_res = NSR_S_OK;
    TAKE_WMCS();
    if (MNS_FALSE != wmcs.packetretrytime.pending)
    {
        result = ERR_BUFOV;                                /* pending request */
        cb_res = NSR_E_BUSY;
    }
    GIVE_WMCS();

    if (ERR_NO == result)
    {
        if (cb_ptr)
        {
            TAKE_WMCS();
            wmcs.packetretrytime.cb_ptr = cb_ptr;
            GIVE_WMCS();
        }
        else
        {
            result = ERR_PARAM;                  /* no callback pointer given */
        }
    }

    if (ERR_NO == result)
    {
        msg_ptr = MsgGetTxPtrExt(1);
        if (msg_ptr)
        {
            msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
            msg_ptr->FBlock_ID = FBLOCK_INIC;
            msg_ptr->Inst_ID   = 0;
            msg_ptr->Func_ID   = FUNCID_INIC_PACKETRETRYTIME;
            msg_ptr->Operation = OP_SETGET;
            msg_ptr->Data[0]   = time;

            MsgSend3(msg_ptr);

            TAKE_WMCS();
            wmcs.packetretrytime.pending = MNS_TRUE;
            GIVE_WMCS();
        }
        else
        {
            result = ERR_BUFOV;                     /* no Tx buffer available */
            cb_res = NSR_E_BUSY;
        }
    }

    if (ERR_NO != result)
    {
        if (cb_ptr)
        {
            cb_ptr(cb_res, 0);
        }
    }

    T_API_EXIT(WMCS_53);

    return(result);
}
#endif


#ifdef WMCS_54
/*! API function to read the wait time for low level retries
  * between data packets
  *
  * @author     RWilhelm
  * @updated    09/12/2007
  * @version    2.5.0
  *
  * @param      cb_ptr      callback function to present the result
  *
  * @return     ERR_PARAM   no callback pointer given
  */
byte MostGetPacketRetryTime(TWmcsPacketRetryTimeCB *cb_ptr)
{
    byte       result;
    TMnsResult cb_res;
    TMsgTx     *msg_ptr;

    T_API_ENTRY(WMCS_54);

    result = ERR_NO;
    cb_res = NSR_S_OK;
    TAKE_WMCS();
    if (MNS_FALSE != wmcs.packetretrytime.pending)
    {
        result = ERR_BUFOV;                                /* pending request */
        cb_res = NSR_E_BUSY;
    }
    GIVE_WMCS();

    if (ERR_NO == result)
    {
        if (cb_ptr)
        {
            TAKE_WMCS();
            wmcs.packetretrytime.cb_ptr = cb_ptr;
            GIVE_WMCS();
        }
        else
        {
            result = ERR_PARAM;                  /* no callback pointer given */
        }
    }

    if (ERR_NO == result)
    {
        msg_ptr = MsgGetTxPtrExt(0);
        if (msg_ptr)
        {
            msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
            msg_ptr->FBlock_ID = FBLOCK_INIC;
            msg_ptr->Inst_ID   = 0;
            msg_ptr->Func_ID   = FUNCID_INIC_PACKETRETRYTIME;
            msg_ptr->Operation = OP_GET;

            MsgSend3(msg_ptr);

            TAKE_WMCS();
            wmcs.packetretrytime.pending = MNS_TRUE;
            GIVE_WMCS();
        }
        else
        {
            result = ERR_BUFOV;                     /* no Tx buffer available */
            cb_res = NSR_E_BUSY;
        }
    }

    if (ERR_NO != result)
    {
        if (cb_ptr)
        {
            cb_ptr(cb_res, 0);
        }
    }

    T_API_EXIT(WMCS_54);

    return(result);
}
#endif


#ifdef WMCS_55
/*! Internal helper function to parse INIC.PacketRetryTime.Status
  *
  * @author     RWilhelm
  * @updated    9/11/2007
  * @version    2.5.0
  *
  * @param      msg_ptr  status message from INIC
  *
  * @return     nothing
  */
void WmcsHandlePacketRetryTimeStatus(TMsgRx *msg_ptr)
{
    TWmcsPacketRetryTimeCB *cb_ptr;

    T_MOD_ENTRY(WMCS_55);

    ASSERT(msg_ptr);

    TAKE_WMCS();
    cb_ptr = wmcs.packetretrytime.cb_ptr;
    wmcs.packetretrytime.cb_ptr     = NULL;
    wmcs.packetretrytime.pending    = MNS_FALSE;
    GIVE_WMCS();

    if(cb_ptr)
    {
        cb_ptr(NSR_S_OK, msg_ptr->Data[0]);
    }

    T_MOD_EXIT(WMCS_55);
}
#endif


#ifdef WMCS_56
/*! Internal helper function to parse INIC.PacketRetryTime.Error
  *
  * @author     RWilhelm
  * @updated    9/12/2007
  * @version    2.5.0
  *
  * @param      msg_ptr  error message from INIC
  *
  * @return     nothing
  */
void WmcsHandlePacketRetryTimeError(TMsgRx *msg_ptr)
{
    TMnsResult             result;
    TWmcsPacketRetryTimeCB *cb_ptr;

    T_MOD_ENTRY(WMCS_56);

    ASSERT(msg_ptr);
    result  = NSR_BUILD(MNS_FALSE, 0xA0, msg_ptr->Data[0]);

    TAKE_WMCS();
    cb_ptr = wmcs.packetretrytime.cb_ptr;
    GIVE_WMCS();

    if (cb_ptr)                          /* call respective callback function */
    {
        cb_ptr(result, 0);

        TAKE_WMCS();
        wmcs.packetretrytime.cb_ptr = NULL;  /* for distinction in error case */
        GIVE_WMCS();
    }
    else                             /* error message without pending request */
    {
        FAILED_ASSERT();
    }

    TAKE_WMCS();
    wmcs.packetretrytime.pending = MNS_FALSE;
    GIVE_WMCS();

    T_MOD_EXIT(WMCS_56);
}
#endif



#ifdef WMCS_57

/*! API function to read EUI48 value
  *
  * @author     RWilhelm
  * @updated    12/15/2007
  * @version    3.0.0
  *
  * @param      eui48   pointer to array whre the result will be written
  *
  * @return     nothing
  */
void MostGetEUI48(byte *eui48)
{
    byte i;

    T_API_ENTRY(WMCS_57);

    if (eui48)
    {
        TAKE_WMCS();
        for (i=0; i<6; ++i)
        {
            *eui48++ = wmcs.eui48_shadow[i];
        }
        GIVE_WMCS();
    }

    T_API_EXIT(WMCS_57);

}
#endif

#ifdef WMCS_58
/*! internal function to request EUI48 value
  *
  * @author     RWilhelm
  * @updated    12/15/2007
  * @version    3.0.0
  *
  * @param      none
  *
  * @return     error message
  */
static byte WmcsRequestEUI48(void)
{
    byte   result;
    TMsgTx *msg_ptr;

    T_MOD_ENTRY(WMCS_58);

    result  = ERR_NO;
    msg_ptr = MsgGetTxPtrExt(0);

    if (msg_ptr)
    {
        msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
        msg_ptr->FBlock_ID = FBLOCK_NETBLOCK;
        msg_ptr->Inst_ID   = 0;
        msg_ptr->Func_ID   = FUNC_EUI48;
        msg_ptr->Operation = OP_GET;
        msg_ptr->Length    = 0;

        MsgSend3(msg_ptr);
    }
    else
    {
        SCHEDULE_RETRY(WMCS_RETRY_REQUEST_EUI48);
        result = ERR_BUFOV;
    }

    T_MOD_EXIT(WMCS_58);

    return(result);
}
#endif

#ifdef WMCS_59
/*! Internal helper function to parse NB.EUI48.Status
  *
  * @author     RWilhelm
  * @updated    12/15/2007
  * @version    3.0.0
  *
  * @param      msg_ptr  status message from INIC
  *
  * @return     nothing
  */
void WmcsHandleEUI48Status(TMsgRx *msg_ptr)
{
    byte i;
    T_LIB_ENTRY(WMCS_59);

    TAKE_WMCS();
    for (i=0; i<6; ++i)
    {
        wmcs.eui48_shadow[i] = msg_ptr->Data[i];
    }
    GIVE_WMCS();

    T_LIB_EXIT(WMCS_59);
}
#endif


#ifdef WMCS_60
/*! Internal helper function to set wmcs.memory.pending
  *
  * @author     RWilhelm
  * @updated    02/27/2009
  * @version    3.0.1
  *
  * @param      on  sets wmcs.memory.pending to MNS_TRUE or MNS_FALSE
  *
  * @return     error value
  */
byte WmcsSetINICMemPending(bool on)
{
    byte result;

    T_LIB_ENTRY(WMCS_60);

    result = ERR_NO;

    TAKE_WMCS();
    if (MNS_FALSE != on)
    {
        if (MNS_FALSE != wmcs.memory.pending)
        {
            result = ERR_BUFOV;                         /* pending request */
        }
        else
        {
            wmcs.memory.pending = MNS_TRUE;
        }
    }
    else
    {
        wmcs.memory.pending = MNS_FALSE;
    }
    GIVE_WMCS();

    T_LIB_EXIT(WMCS_60);

    return(result);
}
#endif

#ifdef WMCS_61
/*!
  * \brief      Internal helper function to parse INIC.ConfStringRead.Status
  * \param[in]  msg_ptr Pointer to the received message
  */
void WmcsHandleConfStringResult(TMsgRx *msg_ptr)
{
    TWmcsReadConfStringCB *cb_ptr;
    byte *src;
    byte *tgt;
    byte i;
    byte *conf_string;

    T_MOD_ENTRY(WMCS_61);

    ASSERT(msg_ptr);

    src = &msg_ptr->Data[WMCS_MEMORY_OFFSET_ROM];

    TAKE_WMCS();
    tgt = wmcs.memory.conf_string;
    GIVE_WMCS();

    /* assemble ConfigurationString */
    for (i=0; i<INIC_MEM_CS_LEN; ++i)
    {
        *tgt++ = *src++;
    }


    TAKE_WMCS();
    cb_ptr      = wmcs.memory.cs_cb_ptr;
    conf_string = (byte*)wmcs.memory.conf_string;
    GIVE_WMCS();

    ASSERT(cb_ptr);

    if (cb_ptr)
    {
        cb_ptr(NSR_S_OK, conf_string,(byte)INIC_MEM_CS_LEN);
    }

    TAKE_WMCS();
    wmcs.memory.cs_cb_ptr = NULL;
    wmcs.memory.pending   = MNS_FALSE;
    GIVE_WMCS();

    T_MOD_EXIT(WMCS_61);

}
#endif

#ifdef WMCS_62
/*!
  * \brief      Internal helper function to parse INIC.ConfStringRead.Error
  * \param[in]  msg_ptr Pointer to the received message
  */
void WmcsHandleConfStringError(TMsgRx *msg_ptr)
{
    TMnsResult            result;
    TWmcsReadConfStringCB *cs_cb_ptr;

    T_MOD_ENTRY(WMCS_62);

    ASSERT(msg_ptr);

    result    = NSR_BUILD(MNS_FALSE, 0xA0, msg_ptr->Data[0]);

    TAKE_WMCS();
    cs_cb_ptr = wmcs.memory.cs_cb_ptr;
    GIVE_WMCS();

    if (cs_cb_ptr)
    {
        cs_cb_ptr(result, NULL, 0);

        TAKE_WMCS();
        wmcs.memory.cs_cb_ptr = NULL;
        GIVE_WMCS();
    }
    else
    {
        FAILED_ASSERT();
    }

    TAKE_WMCS();
    wmcs.memory.pending   = MNS_FALSE;
    GIVE_WMCS();


    T_MOD_EXIT(WMCS_62);
}
#endif



