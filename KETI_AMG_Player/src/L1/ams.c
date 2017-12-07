/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Implementation of the Application Message Service Wrapper (WAMS)
File:           ams.c
Version:        3.0.x-SR-1  
Language:       C
Author(s):      S.Kerber, S.Semmler, T.Jahnke
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
  * \brief      Implementation of the Application Message Service Wrapper (WAMS)
  * \details    The Application Message Service provides an API to send and
  *             receive MOST Control Messages to or from any function block
  *             (FBlock) in the MOST Network, and to/from any local function
  *             block (beside local FBlock INIC).
  * \remarks    This module is not intended to send any commands to the local
  *             FBlock INIC. MOST NetServices does not allow control messages that
  *             are aimed at the local instance of FBlock INIC. The application
  *             has to use the propriate MOST NetServices API functions instead
  *             to be able to control the local Network Interface.
  */


/*
==============================================================================
    Includes
==============================================================================
*/

#include "mbm.h"
#include "pms.h"
#include "mis.h"
#include "mns.h"
#include "ams.h"
#ifdef VMSV_MIN
    #include "vmsv.h"
#endif
#include "wmcs.h"
#ifdef AAM_MIN
#include "aam.h"
#endif
#include "ams_pv.h"



/*
==============================================================================
    NetServices trace: module trace macros
==============================================================================
*/

#if (MNS_DEBUG & NST_C_FUNC_ENTRIES)
    #define T_API_ENTRY(func)   MNS_TRACE((MNS_P_SRV_AMS, NST_E_FUNC_ENTRY_API, 1, func))
    #define T_LIB_ENTRY(func)   MNS_TRACE((MNS_P_SRV_AMS, NST_E_FUNC_ENTRY_LIB, 1, func))
    #define T_MOD_ENTRY(func)   MNS_TRACE((MNS_P_SRV_AMS, NST_E_FUNC_ENTRY_MOD, 1, func))
#else
    #define T_API_ENTRY(func)
    #define T_LIB_ENTRY(func)
    #define T_MOD_ENTRY(func)
#endif

#if (MNS_DEBUG & NST_C_FUNC_EXITS)
    #define T_API_EXIT(func)    MNS_TRACE((MNS_P_SRV_AMS, NST_E_FUNC_EXIT_API, 1, func))
    #define T_LIB_EXIT(func)    MNS_TRACE((MNS_P_SRV_AMS, NST_E_FUNC_EXIT_LIB, 1, func))
    #define T_MOD_EXIT(func)    MNS_TRACE((MNS_P_SRV_AMS, NST_E_FUNC_EXIT_MOD, 1, func))
#else
    #define T_API_EXIT(func)
    #define T_LIB_EXIT(func)
    #define T_MOD_EXIT(func)
#endif

#if (MNS_DEBUG & NST_C_INIT)
    #define T_INIT()            MNS_TRACE((MNS_P_SRV_AMS, NST_E_INIT, 0))
#else
    #define T_INIT()
#endif

#if (MNS_DEBUG & NST_C_SERVICE)
    #define T_SERVICE(event)    MNS_TRACE((MNS_P_SRV_AMS, NST_E_SERVICE, 1, event))
#else
    #define T_SERVICE(event)
#endif

#if (MNS_DEBUG & NST_C_REQUESTS)
    #define T_REQUEST(event)    MNS_TRACE((MNS_P_SRV_AMS, NST_E_REQUEST, 1, event))
#else
    #define T_REQUEST(event)
#endif

#if (MNS_DEBUG & NST_C_TX)
    #define T_TX(tgt, fb, inst, func, op, len)  MNS_TRACE((MNS_P_SRV_AMS, NST_E_TX, 6, tgt, fb, inst, func, op, len))
#else
    #define T_TX(tgt, fb, inst, func, op, len)
#endif

#if (MNS_DEBUG & NST_C_RX_MORE)
    #define T_RX(src, fb, inst, func, op, len)  MNS_TRACE((MNS_P_SRV_AMS, NST_E_RX, 6, src, fb, inst, func, op, len))
#else
    #define T_RX(src, fb, inst, func, op, len)
#endif

#if (MNS_DEBUG & NST_C_ASSERTS)
    #define FAILED_ASSERT()     MNS_TRACE((MNS_P_SRV_AMS, NST_E_ASSERT, 1, __LINE__))
    #define ASSERT(exp)         if(!(exp)) FAILED_ASSERT()
#else
    #define FAILED_ASSERT()
    #define ASSERT(exp)
#endif

#define TAKE_EVENTS()   WAIT4MX(MX_AMS_PE)
#define GIVE_EVENTS()   REL_MX(MX_AMS_PE)
#define TAKE_MSG()      WAIT4MX(MX_AMS_CS)
#define GIVE_MSG()      REL_MX(MX_AMS_CS)

#define SCHEDULE_RETRY(flag) { TAKE_EVENTS(); msg.retry |= flag; GIVE_EVENTS(); }
#define RETRY_DONE(flag)     { TAKE_EVENTS(); msg.retry &= ~flag; GIVE_EVENTS(); }
#define PROCESS_RETRY(retry_flag, retry_func) { if ((ERR_NO == result) && (retry & retry_flag)) { RETRY_DONE(retry_flag); result = retry_func(); }}


/*
================================================================================
    Module Internal Variables
================================================================================
*/

#ifdef AMS_MIN
    /*! Data variable of the AMS module */
    static TMsgData msg;
#else
    /* ANSI C requires that something be declared in the compilation unit. */
    extern byte dummy;
#endif  /* #ifdef AMS_MIN */


/*
==============================================================================
==============================================================================
    Module Implementation
==============================================================================
==============================================================================
*/
#ifdef AMS_0
void MsgInit(struct TMsgConfig *cfg_ptr)
{
    T_LIB_ENTRY(AMS_0);

    ASSERT(cfg_ptr);

    T_INIT();

    msg.cfg_ptr = cfg_ptr;

    if ((byte)0 == msg.cfg_ptr->rx_burst)
    {
        msg.cfg_ptr->rx_burst = MSG_STD_RX_BURST;
    }

    #ifdef NS_AMS_MSV2
    msg.cbTxMsgFlushed_fptr = cfg_ptr->tx_msg_flushed_fptr;
    #endif

    TAKE_EVENTS();
    msg.pending_events = AMS_P_NONE;
    msg.latest_handled_event = AMS_P_NONE;
    msg.retry  = 0;
    msg.net_on = MNS_FALSE;
    GIVE_EVENTS();

    TAKE_MSG();
    #ifdef MSG_RX_SEG_COOPERATIVE
    msg.seg.ats.value   = INIC_ATS_ISSEHCSEG;
    msg.seg.mode        = MSG_SEG_COOPERATIVE;
    #endif

    #ifdef MSG_RX_SEG_INIC_ONLY
    msg.seg.ats.value   = INIC_ATS_NOEHCSEG;
    msg.seg.mode        = MSG_SEG_INIC_ONLY;
    #endif

    #ifdef MSG_RX_SEG_EHC_ONLY
    msg.seg.ats.value   = INIC_ATS_CMSEHCSEG;
    msg.seg.mode        = MSG_SEG_EHC_ONLY;
    #endif

    msg.mid_level_retries   = DEF_MID_LEVEL_RETRIES;
    msg.seg.ats.shadow      = INIC_SHADOW_INVALID_BYTE;
    msg.retry_cfg.shadow.total_attempts = PMS_DEF_RETRY_1;
    msg.retry_cfg.shadow.time           = 0;
    msg.retry_cfg.busy                  = MNS_FALSE;
    GIVE_MSG();

    MbmQueueInit(&(msg.tx_queue), MX_AMS_TX_Q);
    MbmQueueInit(&(msg.rx_queue), MX_AMS_RX_Q);

    MnsServiceInitComplete(MNS_PHASE_INIT, MNS_P_SRV_AMS);
    T_LIB_EXIT(AMS_0);
}
#endif

#ifdef AMS_1
void MsgService(void)
{
    word event_to_handle;
    bool request_flag;

    T_LIB_ENTRY(AMS_1);

    event_to_handle = MsgGetNextEventToHandle();
    request_flag    = MNS_FALSE;

    T_SERVICE(event_to_handle);

    switch (event_to_handle)
    {
        case AMS_P_BUF_FREED:
            (void)MsgRetry();
            break;

        case AMS_P_TX:
            {
                HMBMBUF handle = MbmDequeue(&(msg.tx_queue));

                ASSERT(handle);
                if (handle)
                {
                    TMsgTx *msg_ptr = MBM_GET_CTRL_TX_PTR(handle);

                    ASSERT(msg_ptr);
                    if (msg_ptr)
                    {
                        MsgSend2(msg_ptr);
                    }
                }
            }
            break;

        case AMS_P_RX:
            MsgRxOutTrigger(NULL);
            break;

        case AMS_P_GO_PROTECTED:
            MsgEHCIGoProtected();
            break;

        case AMS_P_GO_SEMI_PROTECTED:
            MsgEHCIGoSemiProtected();
            break;


        case AMS_P_SHADOW_CHANGE:
            if (msg.cfg_ptr->retry_config_adjusted_fptr)
            {
                TMsgRetryConfig config;

                TAKE_MSG();
                config.time           = msg.retry_cfg.shadow.time;
                config.total_attempts = msg.retry_cfg.shadow.total_attempts;
                GIVE_MSG();

                msg.cfg_ptr->retry_config_adjusted_fptr(&config);
            }
            break;

        default:
            FAILED_ASSERT();
            event_to_handle = AMS_P_NONE;
            break;
    }

    TAKE_EVENTS();
    request_flag = (AMS_P_NONE != msg.pending_events) ? MNS_TRUE : MNS_FALSE;
    GIVE_EVENTS();

    if (MNS_FALSE != request_flag)
    {
        MnsSetPendingService(MNS_P_SRV_AMS);
    }

    T_LIB_EXIT(AMS_1);
}
#endif

#ifdef AMS_2
void MsgSetPendingEvent(word event_flag)
{
    T_MOD_ENTRY(AMS_2);

    T_REQUEST(event_flag);
    MnsSetPendingEventFlag(event_flag, MX_AMS_PE,
                           &msg.pending_events, MNS_P_SRV_AMS);
    T_MOD_EXIT(AMS_2);
}
#endif

#ifdef AMS_3
static word MsgGetNextEventToHandle(void)
{
    word result;

    T_MOD_ENTRY(AMS_3);
    result = MnsGetNextEventFlagToCall(MX_AMS_PE,
                                       &msg.pending_events,
                                       &msg.latest_handled_event,
                                       AMS_P_FIRST, AMS_P_LAST);
    T_MOD_EXIT(AMS_3);
    return(result);
}
#endif

#ifdef AMS_4
/*!
  * \brief      Gets a free message buffer for an application message
  * \details    When there is no free buffer the function returns
  *             \c NULL. The next attempt can be made after the
  *             callback \c general.on_buf_freed_fptr() was called.
  * \param[in]  size    The amount of payload that should be reserved.
  *                     If size = 0 the message has no payload or the
  *                     application assigns external payload (the AMS
  *                     will take a look at the length field before
  *                     sending).
  * \return     Possible return values:
  *             - Pointer to a free message buffer.
  *             - \c NULL if there is no free buffer.
  */
TMsgTx * MsgGetTxPtrExt(word size)
{
    HMBMBUF handle;
    TMsgTx *msg_ptr;

    T_API_ENTRY(AMS_4);

    handle  = NULL;
    msg_ptr = NULL;

    handle = PmsGetBuf(size, MBM_TYPE_CTRL_TX);
    if (handle)
    {
        msg_ptr = MBM_GET_CTRL_TX_PTR(handle);
        ASSERT(msg_ptr);
        msg_ptr->Length = size;
        #ifdef CTRL_FILTER_ID
        msg_ptr->Filter_ID = 0x00;
        #endif
        if (!size)
        {
            msg_ptr->Data = NULL;
        }
    }

    T_API_EXIT(AMS_4);
    return(msg_ptr);
}
#endif

#ifdef AMS_5
/*!
  * \brief      Schedules an application message for sending
  * \details    When the transmission has been completed the callback
  *             msg.tx_status_fptr() is called.
  * \param[in]  msg_ptr     Pointer to the message.
  */
void AmsMsgSend(TMsgTx *msg_ptr)
{
    T_API_ENTRY(AMS_5);

    ASSERT(msg_ptr);
    ASSERT(((byte)64) > msg_ptr->MidLevelRetries);
    #ifdef _OS81110_MCM_LLR
    ASSERT(((byte)32) > msg_ptr->LowLevelRetries);
    #endif

    if(EHCISTATE_IS_ATTACHED() && msg_ptr)
    {
        #ifdef NS_AMS_AH
        if (MSG_TGT_UNKNOWN == msg_ptr->Tgt_Adr)
        {
            AddrHSearchStart(msg_ptr);
        }
        else
        #endif
        {
            MsgSend2(msg_ptr);
        }
    }
    else
    {
        FAILED_ASSERT();
        if (msg.cfg_ptr->tx_status_fptr)
        {
            (void)msg.cfg_ptr->tx_status_fptr(XMIT_PROTECTED_MODE, msg_ptr);
        }
        MsgTxUnused(msg_ptr);
    }

    T_API_EXIT(AMS_5);
}
#endif

#ifdef AMS_6
void MsgSend2(TMsgTx *msg_ptr)
{
    HMBMBUF handle;
    byte result;

    T_API_ENTRY(AMS_6);

    handle = NULL;
    result = MSG_TX_FILTER_DEFAULT;

    ASSERT(msg_ptr);
    /*lint -e{413} See ASSERT  */
    handle = MbmGetHandleByMsgPtr(msg_ptr);
    ASSERT(handle);

    #ifdef NS_AMS_MSV2
    result = ConfigStateFilterV2(msg_ptr);        /* Configuration state based message filter */
    if ( msg.cbTxMsgFlushed_fptr &&
        (MSG_TX_FILTER_CANCEL == result) )
    {
        msg.cbTxMsgFlushed_fptr(msg_ptr);
    }
    #endif

    #ifdef VMSV_MIN
    if (MSG_TX_FILTER_DEFAULT == result)
    {
        result = VmsvTxFilter(msg_ptr);
    }
    #endif

    if ((MSG_TX_FILTER_DEFAULT == result) &&
         msg.cfg_ptr->tx_filter_fptr      &&
         EHCISTATE_IS_ATTACHED())
    {
        result = msg.cfg_ptr->tx_filter_fptr(msg_ptr);
    }

    switch (result)
    {
        case MSG_TX_FILTER_CANCEL:
            MbmFree(handle);
            break;

        case MSG_TX_FILTER_BUSY:
            MbmEnqueue(&(msg.tx_queue), handle);
            MsgSetPendingEvent(AMS_P_TX);
            break;

        default:
            FAILED_ASSERT();

        /*lint -e(616) default case before MSG_TX_FILTER_DEFAULT */
        case MSG_TX_FILTER_DEFAULT:
            if ((MSG_TGT_INIC == msg_ptr->Tgt_Adr) &&
                (FBLOCK_INIC == msg_ptr->FBlock_ID))
            {
                FAILED_ASSERT();
                (void)MsgTxFinal(handle, XMIT_NO_ICM);
            }
            else
            {
                MsgSend3(msg_ptr);
            }
            break;
    }

    T_API_EXIT(AMS_6);
}
#endif

#ifdef AMS_7
void MsgSend3(TMsgTx *msg_ptr)
{
    HMBMBUF handle;
    #ifdef MCS_MIN
    byte    msg_routing;
    #endif

    T_LIB_ENTRY(AMS_7);

    handle      = NULL;
    #ifdef MCS_MIN
    msg_routing = 0;
    #endif

    ASSERT(msg_ptr);
    /*lint -e{413} See ASSERT  */
    handle = MbmGetHandleByMsgPtr(msg_ptr);
    ASSERT(handle);

    T_TX(msg_ptr->Tgt_Adr,  msg_ptr->FBlock_ID, msg_ptr->Inst_ID,
         msg_ptr->Func_ID, msg_ptr->Operation, msg_ptr->Length);

    #ifdef MCS_MIN
    msg_routing = MostCheckOwnAddress(msg_ptr->Tgt_Adr);

    if ((msg_routing & MOST_CHECK_ADDR_MASK_ROUTING) ==
         MOST_CHECK_ADDR_INT_ONLY)
    {
        (void)MsgTxFinal(handle, XMIT_SUCCESS_INTERNAL);
    }
    else
    #endif

    #ifndef MNS_AVOID_ATTACH
    if ( EHCISTATE_IS_PROTECTED()                        &&
         !(( MSG_TGT_INIC    == msg_ptr->Tgt_Adr  )      &&
           ((FBLOCK_INIC     == msg_ptr->FBlock_ID)    ||
            (FBLOCK_NETBLOCK == msg_ptr->FBlock_ID))
           ))
    {
        (void)MsgTxFinal(handle, XMIT_PROTECTED_MODE);
    }
    else
    #endif
    {
        #ifdef AAM_MIN
        AamSend2(handle, MsgTxFinal);
        #else
        PmsSend(handle, MsgTxFinal);
        #endif
    }

    T_LIB_EXIT(AMS_7);
}
#endif

#ifdef AMS_8
byte MsgTxFinal(HMBMBUF handle, byte status)
{
    static byte sync_guard = (byte)0;

    TMsgTx *msg_ptr;
    byte    result;
    byte    cb_result;

    T_MOD_ENTRY(AMS_8);

    ASSERT(handle);
    msg_ptr   = MBM_GET_CTRL_TX_PTR(handle);
    result    = PMS_RELEASE;
    cb_result = MSG_TX_FREE;
    ASSERT(msg_ptr);

    /*lint -esym(960,14.10) no else condition - internal messages shall not effect sync_guard*/
    if (MSG_TGT_INIC != msg_ptr->Tgt_Adr)
    {
        if (XMIT_SYNC == status)
        {
            sync_guard += (byte)2;
    
            if ((byte)5 <= sync_guard)
            {
                sync_guard = (byte)0;
                MisResetInic();
            }
        }
        else if((XMIT_SUCCESS_INTERNAL != status) && (XMIT_PROTECTED_MODE != status))
        {
            if ((byte)0 != sync_guard)
            {
                sync_guard--;
            }
        }
    }
    /*lint +esym(960,14.10) re-enables rule*/

    if (((MSG_TGT_INIC == msg_ptr->Tgt_Adr)   &&
         (FBLOCK_INIC == msg_ptr->FBlock_ID)) &&
        !((XMIT_NO_ICM == status) || (XMIT_SYNC == status) || (XMIT_PROTECTED_MODE == status)))
    {
        ASSERT(XMIT_SUCCESS == status);
    }
    else
    {
        
        #ifdef MCS_MIN
        bool internal = (XMIT_SUCCESS_INTERNAL == status) ? MNS_TRUE : MNS_FALSE;
        byte addr_type = MostCheckOwnAddress(msg_ptr->Tgt_Adr);

        if (MNS_FALSE != internal)
        {
            status = XMIT_SUCCESS;
        }
        else if ((MOST_CHECK_ADDR_GROUP     == addr_type) ||
                 (MOST_CHECK_ADDR_BROADCAST == addr_type))
        {
            if ((XMIT_FRMT == status) || (XMIT_TX_FILTER == status))
            {
                internal = MNS_FALSE;
            }
            else
            {
                status |= XMIT_SUCCESS;
                internal = MNS_TRUE;
            }

        }
        #endif
        /*lint -esym(960,14.10) no else condition - no modification for non-internal messages*/
        if (EHCISTATE_IS_ATTACHED())                  /*lint +esym(960,14.10) re-enables rule*/
        {
            if (msg.cfg_ptr->tx_status_fptr)
            {
                cb_result = msg.cfg_ptr->tx_status_fptr(status, msg_ptr);

                if (((XMIT_SUCCESS == status) || (XMIT_NO_ICM == status)) &&
                    (MSG_TX_FREE != cb_result))
                {
                    FAILED_ASSERT();
                    cb_result = MSG_TX_FREE;
                }
            }
        }
        #ifdef MCS_MIN
        if ((MNS_FALSE != internal) && (MSG_TX_FREE == cb_result))
        {
            TMsgRx *rx_msg_ptr = NULL;


            cb_result = MSG_TX_POSTPONE;

            #ifdef MSG_TX_USER_PAYLOAD_EXT_CB
            /* message has tx user payload and will lead to a call of free_tx_payload_ext_fptr */
            if ( MBM_CTRL_TX_PTR_HAS_EXT_PAYLOAD(handle) && (MBM_STAT_NO_USER_FREE != (MBM_STAT_NO_USER_FREE & MBM_GET_STATUS(handle))) )
            {
                MBM_SET_STATUS_BITS(handle, MBM_STAT_USE_BACKUP);
                handle->msg_backup = handle->msg;   /* copy message union */
            }
            #endif

            (void)MbmChangeType(handle, MBM_TYPE_CTRL_RX);
            rx_msg_ptr = MBM_GET_CTRL_RX_PTR(handle);
            ASSERT(rx_msg_ptr);
            rx_msg_ptr->Src_Adr = MostGetNodeAdr();
            rx_msg_ptr->Rcv_Type = (addr_type & MOST_CHECK_ADDR_MASK_RTYP);
            #ifdef MSG_RX_USER_PAYLOAD
            PmsInjectWaitingRx(handle);
            #else
            MsgRxInReady(rx_msg_ptr);
            #endif
        }
        #endif
    }

    switch (cb_result)
    {
        case MSG_TX_RETRY:
            result = PMS_RETRY;
            break;

        case MSG_TX_POSTPONE:
            break;

        default:
            FAILED_ASSERT();

        /*lint -e(616,825) intended fallthrough, default case is before MSG_TX_FREE */
        case MSG_TX_FREE:
            MbmFree(handle);
            break;
    }

    T_MOD_EXIT(AMS_8);
    return(result);
}
#endif

#ifdef AMS_9
/*!
  * \brief      Releases a message buffer without sending the respective
  *             message.
  * \param[in]  msg_ptr     Pointer to the message.
  */
void MsgTxUnused(TMsgTx *msg_ptr)
{
    HMBMBUF handle;

    T_API_ENTRY(AMS_9);

    handle = NULL;

    ASSERT(msg_ptr);
    if (msg_ptr)
    {   /*lint -e{413} See ASSERT  */
        handle = MbmGetHandleByMsgPtr(msg_ptr);
        ASSERT(handle);
        MbmFree(handle);
    }

    T_API_EXIT(AMS_9);
}
#endif

#ifdef AMS_10
void MsgRxTrigger(TMsgRx *msg_ptr)
{
    T_LIB_ENTRY(AMS_10);

    ASSERT(msg_ptr);
    T_RX(msg_ptr->Src_Adr, msg_ptr->FBlock_ID, msg_ptr->Inst_ID,
         msg_ptr->Func_ID, msg_ptr->Operation, msg_ptr->Length);
    msg_ptr->UsageCnt = 0;

    #ifndef PMS_RX_OPT4
    if ((MNS_FALSE != msg.cfg_ptr->rx_direct) && (MNS_FALSE != msg.net_on) && ((word)0 == MbmQueueLength(&(msg.rx_queue))))
    {
        MsgRxOutTrigger(msg_ptr);
    }
    else
    #endif
    {
        /*lint -e{413} See ASSERT  */
        HMBMBUF handle = MbmGetHandleByMsgPtr(msg_ptr);
        ASSERT(handle);
        MbmEnqueue(&(msg.rx_queue), handle);
        if(MNS_FALSE != msg.net_on)
        {
            MsgSetPendingEvent(AMS_P_RX);
        }
    }

    T_LIB_EXIT(AMS_10);
}
#endif

#ifdef AMS_11
static void MsgRxOutTrigger(TMsgRx *msg_ptr)
{
    HMBMBUF handle;
    byte    rx_burst;

    T_MOD_ENTRY(AMS_11);

    handle   = NULL;

    /* if msg.cfg_ptr->rx_direct is MNS_TRUE, there shouldn't be anything in the
       queue, except messages received before MNS_NET_ON and BUSY operations */
    #ifndef PMS_RX_OPT4
    rx_burst = msg.cfg_ptr->rx_direct ? (byte)0xFF : msg.cfg_ptr->rx_burst;
    #else
    rx_burst = msg.cfg_ptr->rx_burst;
    #endif

    if(MNS_FALSE == msg.net_on)
    {
        T_MOD_EXIT(AMS_11);
        return;
    }

    if (!msg_ptr)
    {
        handle = MbmDequeue(&(msg.rx_queue));
        ASSERT((rx_burst > (byte)1) || handle);
        msg_ptr = handle ? MBM_GET_CTRL_RX_PTR(handle) : NULL;
    }
    else
    {
        /*lint -e{413} See ASSERT  */
        handle = MbmGetHandleByMsgPtr(msg_ptr);
    }

    ASSERT((rx_burst > (byte)1) || (msg_ptr && handle));

    while (msg_ptr && handle && rx_burst)
    {
        if (   (MSG_RX_F_NOT_FINISHED - 1)
             > (msg_ptr->UsageCnt & (MSG_RX_F_NOT_FINISHED - 1)) )              /* avoid overflow of msg_ptr->UsageCnt */
        {
            byte result   = MSG_RX_FREE;
            bool free_msg = MNS_FALSE;

            TAKE_MSG();
            msg_ptr->UsageCnt++;                                                /* counts retained messages */
            GIVE_MSG();
            if (msg.cfg_ptr->rx_complete_fptr && EHCISTATE_IS_ATTACHED())
            {
                result = msg.cfg_ptr->rx_complete_fptr(msg_ptr);
            }

            switch (result)
            {
                case MSG_RX_BUSY:
                case MSG_RX_INC_USAGE_CNT:
                    TAKE_MSG();
                    if (MSG_RX_BUSY == result)
                    {
                        msg_ptr->UsageCnt--;
                    }
                    msg_ptr->UsageCnt |= MSG_RX_F_NOT_FINISHED;
                    GIVE_MSG();
                    MbmEnqueueFirst(&(msg.rx_queue), handle);
                    /* force another service run */
                    rx_burst = 1;
                    break;

                case MSG_RX_TAKE:
                    TAKE_MSG();
                    if (MSG_RX_F_NOT_FINISHED == (msg_ptr->UsageCnt & MSG_RX_F_NOT_FINISHED))
                    {
                        msg_ptr->UsageCnt &= ~MSG_RX_F_NOT_FINISHED;
                        if (msg_ptr->UsageCnt == 0)                         /* Was only the "not finished flag" set, but no user registrated ? */
                        {
                            free_msg = MNS_TRUE;
                        }
                    }
                    GIVE_MSG();
                    if (MNS_FALSE != free_msg)
                    {
                        MsgFreeRxMsg(msg_ptr);                        /* Message must now be cleared, since all users are checked out */
                    }

                    msg_ptr = NULL;
                    handle = NULL;
                    break;

                default:
                    FAILED_ASSERT();

                /*lint -e(616) default case is meant that way */
                case MSG_RX_FREE:
                    TAKE_MSG();
                    msg_ptr->UsageCnt &= ~MSG_RX_F_NOT_FINISHED;
                    GIVE_MSG();
                    MsgFreeRxMsg(msg_ptr);
                    msg_ptr = NULL;
                    handle = NULL;
                    break;
            }
        }
        else
        {
            MbmEnqueueFirst(&(msg.rx_queue), handle);
            rx_burst = 1;
        }

        rx_burst--;

        if (MbmQueueLength(&(msg.rx_queue)))
        {
            if(!rx_burst)
            {
                MsgSetPendingEvent(AMS_P_RX);
            }
            else
            {
                handle = MbmDequeue(&(msg.rx_queue));
                ASSERT(handle);
                msg_ptr = handle ? MBM_GET_CTRL_RX_PTR(handle) : NULL;
            }
        }
    }

    T_MOD_EXIT(AMS_11);
}
#endif

#ifdef AMS_13
/*!
  * \brief      Releases a message after processing, if the callback
  *             function \c msg.rx_complete_fptr() was left by using one
  *             of the return values MSG_RX_TAKE or MSG_RX_INC_USAGE_CNT.
  * \details    Beside that, this API function can be used to discard a
  *             message buffer after allocating the buffer by using
  *             MsgGetRxInPtr() or MsgGetRxInPtrExt().
  * \param[in]  msg_ptr     Pointer to the message.
  */
void MsgFreeRxMsg(TMsgRx *msg_ptr)
{
    byte cnt;

    T_API_ENTRY(AMS_13);

    ASSERT(msg_ptr);

    if (msg_ptr)
    {
        TAKE_MSG();
        if (0 != (msg_ptr->UsageCnt & ~MSG_RX_F_NOT_FINISHED)) /* do not decrement, if only the flag is set, */
        {                                                                   /* but the usage cnt is already zero          */
            msg_ptr->UsageCnt--;
        }

        cnt = msg_ptr->UsageCnt;
        GIVE_MSG();

        if (!cnt)
        {
            /*lint -e{413} see ASSERT */
            HMBMBUF handle = MbmGetHandleByMsgPtr(msg_ptr);
            ASSERT(handle);
            MbmFree(handle);
        }
    }

    T_API_EXIT(AMS_13);
}
#endif


#ifdef AMS_12
void MsgRxError(byte error, TMsgRx *msg_ptr)
{
    T_LIB_ENTRY(AMS_12);

    ASSERT(msg_ptr);

    if (NULL != msg.cfg_ptr->rx_error_fptr)
    {
        msg.cfg_ptr->rx_error_fptr(error, msg_ptr);
    }

    T_LIB_EXIT(AMS_12);
}
#endif

#ifdef AMS_14
/*!
  * \brief      Reserves an Rx buffer for a message, which should be
  *             received by an internal FBlock.
  * \details    In some applications it can be necessary to receive
  *             messages directly from internal FBlocks. In addition
  *             to the standard method, sending a message to a FBlock
  *             by using target address MSG_TGT_EHC, the message can be
  *             entered directly into the receive buffer of the AMS
  *             ( see function MsgRxInReady() ).
  * \param[in]  size    The amount of payload that should be reserved.
  *                     If <tt>size = 0</tt> the message has no payload
  *                     or the application assigns external payload (The
  *                     AMS will take a look at the \c Length field before
  *                     sending).
  */
TMsgRx * MsgGetRxInPtrExt(word size)
{
    HMBMBUF handle;
    TMsgRx *msg_ptr;

    T_API_ENTRY(AMS_14);

    handle  = NULL;
    msg_ptr = NULL;

    handle = PmsGetBuf(size, MBM_TYPE_CTRL_RX);
    if (handle)
    {
        msg_ptr = MBM_GET_CTRL_RX_PTR(handle);
        ASSERT(msg_ptr);
        msg_ptr->Src_Adr = MSG_SRC_EHC;
        msg_ptr->Rcv_Type = MSG_RCV_TYPE_LOGICAL;
        msg_ptr->Length  = size;
        #ifdef CTRL_FILTER_ID
            msg_ptr->Filter_ID = 0x00;
        #endif
    }

    T_API_EXIT(AMS_14);
    return(msg_ptr);
}
#endif

#ifdef AMS_15
/*!
  * \brief      Enters a message directly into the Rx queue.
  * \details    In some applications it can be necessary to receive
  *             messages directly from internal FBlocks. In addition
  *             to the standard method, sending a message to a FBlock
  *             by using target address MSG_TGT_EHC, the message can be
  *             entered directly into the receive buffer of the AMS.
  *             If the message needs to be discarded without sending it
  *             internally, the message buffer can be freed up by calling
  *             MsgFreeRxMsg().
  * \param[in]  msg_ptr     Pointer to the message.
  */
void MsgRxInReady(TMsgRx *msg_ptr)
{
    T_API_ENTRY(AMS_15);

    ASSERT(msg_ptr);
    MsgRxTrigger(msg_ptr);

    T_API_EXIT(AMS_15);
}
#endif

#ifdef AMS_16
/*!
  * \brief      Copies a variable of type byte into the respective
  *             message to transmit.
  * \param[out] tgt_ptr_ptr Points at the pointer pointing at the
  *                         respective data field. The data field
  *                         pointer will be incremented automatically
  *                         after encoding the respective data field.
  * \param[in]  src_ptr     Points at the source variable of type byte.
  */
void MsgTxDataByte(byte **tgt_ptr_ptr, byte *src_ptr)
{
    T_API_ENTRY(AMS_16);

    ASSERT(src_ptr);

    *((*tgt_ptr_ptr)++) = (byte)(*src_ptr);
    T_API_EXIT(AMS_16);
}
#endif

#ifdef AMS_17
/*!
  * \brief      Copies a variable of type word (16 bit) into the respective
  *             message to transmit.
  * \param[out] tgt_ptr_ptr Points at the pointer pointing at the
  *                         respective data field. The data field
  *                         pointer will be incremented automatically
  *                         after encoding the respective data field.
  * \param[in]  src_ptr     Points at the source variable of type word.
  */
void MsgTxDataWord(byte **tgt_ptr_ptr, word *src_ptr)
{
    T_API_ENTRY(AMS_17);

    ASSERT(src_ptr);

    *((*tgt_ptr_ptr)++) = HB(*src_ptr);
    *((*tgt_ptr_ptr)++) = LB(*src_ptr);
    T_API_EXIT(AMS_17);
}
#endif

#ifdef AMS_18
/*!
  * \brief      Copies a variable of type dword (32 bit) into the respective
  *             message to transmit.
  * \param[out] tgt_ptr_ptr Points at the pointer pointing at the
  *                         respective data field. The data field
  *                         pointer will be incremented automatically
  *                         after encoding the respective data field.
  * \param[in]  src_ptr     Points at the source variable of type dword.
  */
void MsgTxDataLong(byte **tgt_ptr_ptr, dword *src_ptr)
{
    word high_part;
    word low_part;
    dword temp;
    byte i;

    T_API_ENTRY(AMS_18);

    ASSERT(src_ptr);

    temp = *src_ptr;
    for (i=(byte)0;i<(byte)16;i++)
        temp = temp >> 1;

    high_part = (word)temp;
    MsgTxDataWord(tgt_ptr_ptr, &high_part);

    low_part = (word)(*src_ptr);
    MsgTxDataWord(tgt_ptr_ptr, &low_part);
    T_API_EXIT(AMS_18);
}
#endif

#ifdef AMS_19
/*!
  * \brief      Encodes a BCD coded number into two ASCII coded bytes.
  *             These two bytes are copied into the respective  message
  *             to transmit.
  * \param[out] tgt_ptr_ptr Points at the pointer pointing at the
  *                         respective data field. The data field
  *                         pointer will be incremented automatically
  *                         after encoding the respective data field.
  * \param[in]  number      The BCD coded number
  */
void MsgTxBcdToAscII(byte** tgt_ptr_ptr, byte number)
{
    T_API_ENTRY(AMS_19);
    *((*tgt_ptr_ptr)++) = (byte)(((number>>4)&0xF) + 0x30);
    *((*tgt_ptr_ptr)++) = (byte)((number&0xF) + 0x30);
    T_API_EXIT(AMS_19);
}
#endif

#ifdef AMS_20
/*!
  * \brief      Decodes a variable of type byte
  * \param[out] tgt_ptr     Points at the target variable of type byte.
  * \param[in]  src_ptr_ptr Points at the pointer pointing at the
  *                         respective data field. The data field
  *                         pointer will be incremented automatically
  *                         after decoding the respective data field.
  */
void MsgRxDataByte(byte *tgt_ptr, byte **src_ptr_ptr)
{
    T_API_ENTRY(AMS_20);

    ASSERT(src_ptr_ptr);

    *tgt_ptr = *((*src_ptr_ptr)++);
    T_API_EXIT(AMS_20);
}
#endif

#ifdef AMS_21
/*!
  * \brief      Decodes a variable of type word (16 bit)
  * \param[out] tgt_ptr     Points at the target variable of type word.
  * \param[in]  src_ptr_ptr Points at the pointer pointing at the
  *                         respective data field. The data field
  *                         pointer will be incremented automatically
  *                         after decoding the respective data field.
  */
void MsgRxDataWord(word *tgt_ptr, byte **src_ptr_ptr)
{
    byte high_byte;
    byte low_byte;

    T_API_ENTRY(AMS_21);

    ASSERT(src_ptr_ptr);

    high_byte = *((*src_ptr_ptr)++);
    low_byte  = *((*src_ptr_ptr)++);

    *tgt_ptr    = (word)(high_byte << 8);
    *tgt_ptr   += (word)(low_byte);
    T_API_EXIT(AMS_21);
}
#endif

#ifdef AMS_22
/*!
  * \brief      Decodes a variable of type dword (32 bit)
  * \param[out] tgt_ptr     Points at the target variable of type dword.
  * \param[in]  src_ptr_ptr Points at the pointer pointing at the
  *                         respective data field. The data field
  *                         pointer will be incremented automatically
  *                         after decoding the respective data field.
  */
void MsgRxDataLong(dword *tgt_ptr, byte **src_ptr_ptr)
{
    word high_part;
    word low_part;
    dword temp;
    byte i;

    T_API_ENTRY(AMS_22);

    ASSERT(src_ptr_ptr);

    MsgRxDataWord(&high_part, src_ptr_ptr);
    MsgRxDataWord(&low_part, src_ptr_ptr);

    temp = (dword)high_part;
    for (i=0;i<16;i++)
        temp = temp << 1;

    *tgt_ptr = (dword)(temp + (dword)low_part);
    T_API_EXIT(AMS_22);
}
#endif


#ifdef AMS_26
static void MsgEHCIGoProtected(void)
{
    T_MOD_ENTRY(AMS_26);

    TAKE_EVENTS();
    msg.retry = 0;
    GIVE_EVENTS();

    TAKE_MSG();
    msg.mid_level_retries               = DEF_MID_LEVEL_RETRIES;
    msg.retry_cfg.total_attempts        = PMS_DEF_RETRY_1;
    msg.retry_cfg.time                  = 0;
    msg.retry_cfg.shadow.total_attempts = PMS_DEF_RETRY_1;
    msg.retry_cfg.shadow.time           = 0;
    msg.retry_cfg.busy                  = MNS_FALSE;
    GIVE_MSG();

    MnsServiceInitComplete(MNS_PHASE_RESET, MNS_P_SRV_AMS);

    T_MOD_EXIT(AMS_26);
}
#endif

#ifdef AMS_27
static void MsgEHCIGoSemiProtected(void)
{
    T_MOD_ENTRY(AMS_27);

    (void)MsgForceMidLevelRetries();
    if ((MNS_FALSE != MostIsSupported(NSF_MOST_25)) || (MNS_FALSE != MostIsSupported(NSF_MOST_50)))
    {
        (void)MsgRequestSegMode();
    }

    T_MOD_EXIT(AMS_27);
}
#endif

#ifdef AMS_28
static byte MsgRetry(void)
{
    word retry;
    byte result;

    T_MOD_ENTRY(AMS_28);

    retry  = 0;
    result = ERR_NO;

    TAKE_EVENTS();
    retry = msg.retry;
    GIVE_EVENTS();

    PROCESS_RETRY    (AMS_RETRY_MIDLEVEL_RETRIES,   MsgForceMidLevelRetries );
    PROCESS_RETRY    (MSG_RETRY_SEGMODE,            MsgForceSegMode         );
    PROCESS_RETRY    (MSG_RETRY_REQ_SEGMODE,        MsgRequestSegMode       );
    PROCESS_RETRY    (MSG_RETRY_REQ_RETRYPARAMS,    MsgRequestRetryParams   );
    PROCESS_RETRY    (MSG_RETRY_RETRYPARAMS,        MsgSendRetryParams      );

    T_MOD_EXIT(AMS_28);

    return(result);
}
#endif

#ifdef AMS_29
void MsgHandleAbilityToSegmentStatus(TMsgRx *msg_ptr)
{
    byte ats_shadow;
    byte ats_value;

    T_LIB_ENTRY(AMS_29);

    ASSERT(msg_ptr);

    TAKE_MSG();
    msg.seg.ats.shadow = msg_ptr->Data[0];
    ats_shadow         = msg.seg.ats.shadow;
    ats_value          = msg.seg.ats.value;
    GIVE_MSG();

    if (!EHCISTATE_IS_ATTACHED())
    {
        MnsNtfCheck(NTF_ABILITY_TO_SEGMENT);
    }

    /* Change the INIC.AbilityToSegment property if it differs from value
     * we want to have.
     */
    if (ats_value != ats_shadow)
    {
        (void)MsgForceSegMode();
    }

    T_LIB_EXIT(AMS_29);
}
#endif

#ifdef AMS_30
void MsgHandleAbilityToSegmentError(TMsgRx *msg_ptr)
{
    T_LIB_ENTRY(AMS_30);
    (void) msg_ptr;

    /* We send valid values to the INIC only, so in this case there must
     * be a problem with the firmware or someone else sent an invalid request
     * via the network. All we can do here is notify the application.
     */
    MnsReportError(NSR_E_INVALID_SEG_MODE_CONFIG);

    T_LIB_EXIT(AMS_30);
}
#endif

#ifdef AMS_31
/*!
  * \brief      Gets the current selected segmentation mode
  * \return     The used segmentation mode or INIC_SHADOW_INVALID_BYTE if the
  *             segmentation mode is not valid at the moment.
  */
byte MsgGetSegMode(void)
{
    byte result;

    T_API_ENTRY(AMS_31);

    result = INIC_SHADOW_INVALID_BYTE;

    if (EHCISTATE_IS_ATTACHED())
    {
        TAKE_MSG();
        if (msg.seg.ats.value == msg.seg.ats.shadow)
        {
            result = msg.seg.mode;
        }
        GIVE_MSG();
    }

    T_API_EXIT(AMS_31);

    return(result);
}
#endif


#ifdef AMS_33
static byte MsgForceSegMode(void)
{
    byte   result;
    TMsgTx *msg_ptr;

    T_MOD_ENTRY(AMS_33);

    result  = ERR_NO;
    msg_ptr = MsgGetTxPtrExt(1);

    if (msg_ptr)
    {
        msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
        msg_ptr->FBlock_ID = FBLOCK_INIC;
        msg_ptr->Func_ID   = FUNCID_INIC_ABILITYTOSEGMENT;
        msg_ptr->Operation = OP_SETGET;
        TAKE_MSG();
        msg_ptr->Data[0]   = msg.seg.ats.value;
        GIVE_MSG();

        MsgSend3(msg_ptr);
    }
    else
    {
        SCHEDULE_RETRY(MSG_RETRY_SEGMODE);
        result = ERR_BUFOV;
    }

    T_MOD_EXIT(AMS_33);
    return(result);
}
#endif

#ifdef AMS_34
static byte MsgRequestSegMode(void)
{
    byte   result;
    TMsgTx *msg_ptr;

    T_MOD_ENTRY(AMS_34);

    result  = ERR_NO;
    msg_ptr = MsgGetTxPtrExt(0);

    if (msg_ptr)
    {
        msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
        msg_ptr->FBlock_ID = FBLOCK_INIC;
        msg_ptr->Func_ID   = FUNCID_INIC_ABILITYTOSEGMENT;
        msg_ptr->Operation = OP_GET;

        MsgSend3(msg_ptr);
    }
    else
    {
        SCHEDULE_RETRY(MSG_RETRY_REQ_SEGMODE);
        result = ERR_BUFOV;
    }

    T_MOD_EXIT(AMS_34);
    return(result);
}
#endif

#ifdef AMS_34
static byte MsgForceMidLevelRetries(void)
{
    byte   result;
    TMsgTx *msg_ptr;

    T_MOD_ENTRY(AMS_34);

    result  = ERR_NO;
    msg_ptr = MsgGetTxPtrExt(3);

    if (msg_ptr)
    {
        msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
        msg_ptr->FBlock_ID = FBLOCK_INIC;
        msg_ptr->Func_ID   = FUNCID_INIC_MIDLEVELRETRY;
        msg_ptr->Operation = OP_SETGET;
        msg_ptr->Data[0]   = TIME_MSG_TX_RETRY;
        msg_ptr->Data[1]   = DEF_MID_LEVEL_RETRIES_INT_PROC;
        msg_ptr->Data[2]   = DEF_MID_LEVEL_RETRIES;

        MsgSend3(msg_ptr);
    }
    else
    {
        SCHEDULE_RETRY(AMS_RETRY_MIDLEVEL_RETRIES);
        result = ERR_BUFOV;
    }

    T_MOD_EXIT(AMS_34);

    return(result);
}
#endif


#ifdef AMS_37
void MsgHandleMidLevelRetriesStatus(TMsgRx *msg_ptr)
{
    T_LIB_ENTRY(AMS_37);

    ASSERT(msg_ptr);

    TAKE_MSG();
    msg.mid_level_retries = msg_ptr->Data[2];
    GIVE_MSG();

    if (!EHCISTATE_IS_ATTACHED())
    {
        MnsNtfCheck(NTF_MIDLEVELRETRY);
    }

    T_LIB_EXIT(AMS_37);
}
#endif

#ifdef AMS_38
void MsgNIStateNetOn(bool on)
{
    T_LIB_ENTRY(AMS_38);

    msg.net_on = on;

    if (MNS_FALSE != on)
    {
        if (MbmQueueLength(&(msg.rx_queue)))
        {
            MsgSetPendingEvent(AMS_P_RX);
        }
    }
    else
    {
        while (MbmQueueLength(&(msg.rx_queue)))
        {
            HMBMBUF handle = MbmDequeue(&(msg.rx_queue));
            MbmFree(handle);
        }
    }

    T_LIB_EXIT(AMS_38);
}
#endif

#ifdef AMS_39
void MsgPrepareReInit(void)
{
    T_LIB_ENTRY(AMS_39);

    #ifdef MSG_RX_USER_PAYLOAD
    MbmFlush(&msg.rx_queue);
    #endif

    #ifdef MSG_TX_USER_PAYLOAD
    PmsFlushTxQueue(&msg.tx_queue);

        #ifdef NS_AMS_AH
        AddrHClearTasks();
        #endif
    #endif

    T_LIB_EXIT(AMS_39);
}
#endif

#ifdef AMS_40
/*!
  * \brief      Gets the number of all currently allocated transmit message buffers.
  * \return     The number of used Tx buffers
  */
word MsgGetUsedTxBuffers(void)
{
    word result;

    T_API_ENTRY(AMS_40);

    result = MbmGetUsedTxBuffers();

    T_API_EXIT(AMS_40);

    return (result);
}
#endif

#ifdef AMS_41
/*!
  * \brief      Gets the number of all currently allocated receive message buffers.
  * \return     The number of used Rx buffers
  */
word MsgGetUsedRxBuffers(void)
{
    word result;

    T_API_ENTRY(AMS_41);

    result = MbmGetUsedRxBuffers();

    T_API_EXIT(AMS_41);

    return (result);
}
#endif

#ifdef AMS_42
bool MsgDiscardRx(void)
{
    HMBMBUF handle;
    bool    result;

    T_LIB_ENTRY(AMS_42);

    result = MNS_FALSE;

    handle = MbmDequeueLast(&(msg.rx_queue));
    ASSERT(handle);

    if (handle)
    {
        if(MNS_FALSE != PmsDiscardRx(handle))
        {
            result = MNS_TRUE;
        }
        else
        {
            MbmEnqueue(&(msg.rx_queue), handle);
        }
    }


    T_LIB_EXIT(AMS_42);

    return(result);
}
#endif


#ifdef AMS_43
/*!
  * \brief      Encodes a BCD coded number into one or two ASCII coded bytes
  *             without leading 0. It also checks the validity of the BCD code
  *             number. Invalid digits are replaced by 0.
  *             These one or two bytes are copied into the respective message
  *             to transmit.
  * \param[out] tgt_ptr_ptr Points at the pointer pointing at the
  *                         respective data field. The data field
  *                         pointer will be incremented automatically
  *                         after encoding the respective data field.
  * \param[in]  bcd         The BCD coded number
  * \return                 the number of copied bytes
  */
byte MsgBcdToStr(byte** tgt_ptr_ptr, byte bcd)
{
    byte digit1;
    byte digit2;
    byte num;

    T_API_ENTRY(AMS_43);

    num = 0;

    digit1 = (bcd >> 4) & 0x0F;
    digit2 = bcd &0x0F;

    /* allow only digits in the range of 0..9 */
    digit1 = (digit1 <= 9) ? digit1 : 0;
    digit2 = (digit2 <= 9) ? digit2 : 0;

    /* no leading zero */
    if (0 != digit1)
    {
        *((*tgt_ptr_ptr)++) = digit1 + (byte)0x30;
        ++num;
    }

    *((*tgt_ptr_ptr)++) = digit2 + (byte)0x30;
    ++num;

    T_API_EXIT(AMS_43);

    return(num);
}
#endif

#ifdef AMS_44
/*!
  * \brief      encodes a 3 digit version number to an ISO8859 string
  * \param[out] version     Pointer to an array containing the version number
  * \param[in]  string      Pointer to the result string
  * \return     The length of the result string
  */
byte MsgVersionToISO8859(byte* version, byte *string)
{
    byte num;

    T_API_ENTRY(AMS_44);

    num = 0;

    num = MsgBcdToStr(&string, *version++);     /* first number */
    *string++ = (byte)('.');
    num++;

    num += MsgBcdToStr(&string, *version++);    /* second number */
    *string++ = (byte)('.');
    num++;

    num += MsgBcdToStr(&string, *version++);    /* third number */
    *string = (byte)('\0');
    num++;

    T_API_EXIT(AMS_44);

    return(num);
}
#endif


#ifdef AMS_45
static byte MsgRequestRetryParams(void)
{
    byte result;
    TMsgTx *msg_ptr;

    T_MOD_ENTRY(AMS_45);

    result = ERR_NO;
    msg_ptr = MsgGetTxPtrExt(0);

    TAKE_MSG();
    msg.retry_cfg.busy = MNS_TRUE;
    GIVE_MSG();

    if (msg_ptr)
    {
        msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
        msg_ptr->FBlock_ID = FBLOCK_NETBLOCK;
        msg_ptr->Func_ID   = FUNC_RETRYPARAMETERS;
        msg_ptr->Operation = OP_GET;

        MsgSend3(msg_ptr);
    }
    else
    {
        SCHEDULE_RETRY(MSG_RETRY_REQ_RETRYPARAMS);
        result = ERR_BUFOV;
    }

    T_MOD_EXIT(AMS_45);
    return(result);
}
#endif

#ifdef AMS_46
static byte MsgSendRetryParams(void)
{
    byte result;
    bool send_msg;
    TMsgTx *msg_ptr;

    T_MOD_ENTRY(AMS_46);

    result = ERR_NO;
    send_msg = MNS_FALSE;
    msg_ptr = MsgGetTxPtrExt(2);

    if (msg_ptr)
    {
        TAKE_MSG();
        if ((msg.retry_cfg.total_attempts != msg.retry_cfg.shadow.total_attempts)
            ||(msg.retry_cfg.time != msg.retry_cfg.shadow.time))    /* check changes once again in case retries are performed */
        {
            msg_ptr->Tgt_Adr    = MSG_TGT_INIC;                     /* at least one value will change */
            msg_ptr->FBlock_ID  = FBLOCK_NETBLOCK;
            msg_ptr->Func_ID    = FUNC_RETRYPARAMETERS;

            if (MNS_FALSE != MostIsSupported(NSF_MOST_150))
            {
                msg_ptr->Operation  = OP_SETGET;
            }
            else
            {
                msg_ptr->Operation  = OP_SET;
            }

            msg_ptr->Data[0]    = msg.retry_cfg.time;
            msg_ptr->Data[1]    = msg.retry_cfg.total_attempts;
            msg.retry_cfg.busy  = MNS_TRUE;
            send_msg            = MNS_TRUE;
        }
        else
        {
            /* no changes - release busy flag */
            msg.retry_cfg.busy = MNS_FALSE;
        }
        GIVE_MSG();

        if (MNS_FALSE != send_msg)
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
        SCHEDULE_RETRY(MSG_RETRY_RETRYPARAMS);
        result = ERR_BUFOV;
    }

    T_MOD_EXIT(AMS_46);
    return(result);
}
#endif

#ifdef AMS_47
void MsgHandleRetryOptionsStatus(TMsgRx *msg_ptr)
{
    T_LIB_ENTRY(AMS_47);

    (void)msg_ptr;

    (void)MsgRequestRetryParams();

    T_LIB_EXIT(AMS_47);
}
#endif

#ifdef AMS_48
void MsgHandleRetryParamsStatus(TMsgRx *msg_ptr)
{
    bool shadow_changed;

    T_LIB_ENTRY(AMS_48);

    ASSERT(msg_ptr);

    TAKE_MSG();
    shadow_changed = MNS_FALSE;

    /* check if the shadow values have changed */
    shadow_changed = (msg.retry_cfg.shadow.time != msg_ptr->Data[0]) ? MNS_TRUE : MNS_FALSE;
    shadow_changed = ((MNS_FALSE != shadow_changed) || (msg.retry_cfg.shadow.total_attempts != msg_ptr->Data[1])) ? MNS_TRUE : MNS_FALSE;

    msg.retry_cfg.shadow.time           = msg_ptr->Data[0];
    msg.retry_cfg.shadow.total_attempts = msg_ptr->Data[1];
    msg.retry_cfg.busy                  = MNS_FALSE;
    GIVE_MSG();

    if (EHCISTATE_IS_ATTACHED())
    {
        if ((MNS_FALSE != shadow_changed) && msg.cfg_ptr->retry_config_adjusted_fptr)
        {
            /* fire callback asynchronously */
            MsgSetPendingEvent(AMS_P_SHADOW_CHANGE);
        }
    }
    else
    {
        MnsNtfCheck(NTF_RETRY_PARAMS);
    }

    T_LIB_EXIT(AMS_48);
}
#endif

#ifdef AMS_49
void MsgHandleRetryError(TMsgRx *msg_ptr)
{
    T_LIB_ENTRY(AMS_49);
    ASSERT(msg_ptr);

    if (FUNC_RETRYPARAMETERS == msg_ptr->Func_ID)
    {
        TAKE_MSG();
        msg.retry_cfg.busy           = MNS_FALSE;
        msg.retry_cfg.total_attempts = msg.retry_cfg.shadow.total_attempts;
        msg.retry_cfg.time           = msg.retry_cfg.shadow.time;
        GIVE_MSG();
    }
    else
    {
        /* not possible - the INIC.RetryOptions are not */
        /* changed by the MOST NetServices              */
        FAILED_ASSERT();
    }

    MnsReportError(NSR_E_INVALID_RETRY_CONFIG);

    T_LIB_EXIT(AMS_49);
}
#endif

#ifdef AMS_50
/*!
  * \brief      The function returns the current retry configuration used by
  *             the INIC.
  * \param[out] config_ptr  Pointer to the TMsgRetryConfig structure that will hold
  *                         the complete configuration after the function returns.
  * \return     Possible return values are...
  *             - \c ERR_NO if it succeeded
  *             - \c ERR_PARAM if the given configuration pointer is NULL
  *             - \c ERR_BUFOV if the returned configuration may not be up to date due to
  *             a change of the parameters is currently in progress. If the parameters
  *             will change subsequently the \c retry_config_adjusted_fptr will be called
  *             by the MOST NetServices.
  */
byte MsgGetRetryConfig(TMsgRetryConfig *config_ptr)
{
    byte result;

    T_API_ENTRY(AMS_50);

    result = ERR_NO;

    TAKE_MSG();
    if (config_ptr)
    {
        config_ptr->total_attempts = msg.retry_cfg.shadow.total_attempts;
        config_ptr->time           = msg.retry_cfg.shadow.time;
    }
    else
    {
        result = ERR_PARAM;
    }

    if ((ERR_NO == result) && (MNS_FALSE != msg.retry_cfg.busy))
    {
        /* currently processing - shadow values may change subsequently */
        result = ERR_BUFOV;
    }
    GIVE_MSG();

    T_API_EXIT(AMS_50);
    return(result);
}
#endif

#ifdef AMS_51
/*!
  * \brief      Sets the current retry configuration of the INIC.
  * \details    If the INIC however answers with an error message, the resulting
  *             code given to \c general.on_error_fptr() will be
  *             NSR_E_INVALID_RETRY_CONFIG.
  * \param[in]  config_ptr  Pointer to the TMsgRetryConfig structure holding
  *                         the retry configuration.
  * \return     Possible return values are...
  *             - \c ERR_NO if the function succeeded
  *             - \c ERR_ALREADY_SET if the retry configuration that is passed by the application
  *               was already set before the function was called
  *             - \c ERR_PARAM if the given configuration is not valid
  *             - \c ERR_BUFOV if the current retry settings cannot be changed due to a
  *               a change of the settings is already in progress. The end of the
  *               running change is signaled by \c retry_config_adjusted_fptr or if
  *               the \c general.on_error_fptr() returns NSR_E_INVALID_RETRY_CONFIG.
  */
byte MsgSetRetryConfig(TMsgRetryConfig *config_ptr)
{
    byte result;
    bool send_config;

    T_API_ENTRY(AMS_51);

    result = ERR_NO;
    send_config = MNS_FALSE;

    TAKE_MSG();
    if (MNS_FALSE == msg.retry_cfg.busy)
    {
        if (config_ptr)
        {
            if (!((config_ptr->total_attempts == msg.retry_cfg.shadow.total_attempts)
                  &&(config_ptr->time == msg.retry_cfg.shadow.time)) )
            {
                msg.retry_cfg.total_attempts = config_ptr->total_attempts;
                msg.retry_cfg.time = config_ptr->time;
                msg.retry_cfg.busy = MNS_TRUE;
                send_config        = MNS_TRUE;
            }
            else
            {
                result = ERR_ALREADY_SET;
            }
        }
        else
        {
            result = ERR_PARAM;
        }
    }
    else
    {
        result = ERR_BUFOV;
    }
    GIVE_MSG();

    if (MNS_FALSE != send_config)
    {
        (void)MsgSendRetryParams();
    }

    T_API_EXIT(AMS_51);
    return(result);
}
#endif

#ifdef AMS_52
/*!
  * \brief      The function marks the user payload buffer of a given message as freed.
  * \details    The application is allowed to pass a message received by the
  *             command interpreter or \c msg.rx_complete_fptr(). It will mark the buffer,
  *             which then will cause the MOST NetServices not to call the cleanup
  *             function \c free_payload_buf_fptr() when the buffer is freed. From that
  *             point on the application is responsible to deallocate the buffer after usage.
  * \param[in]  rx_ptr  Pointer to an RX message
  */
void MsgReleasePayload(TMsgRx *rx_ptr)
{
    T_API_ENTRY(AMS_52);

    PmsReleasePayload(rx_ptr);

    T_API_EXIT(AMS_52);
}
#endif

#ifdef AMS_53
void MsgNIStateNetInit(void)
{
    HMBMBUF handle;

    T_LIB_ENTRY(AMS_53);

    while ((word)0 != MbmQueueLength(&(msg.rx_queue)))
    {
        handle = MbmDequeue(&(msg.rx_queue));
        MbmFree(handle);
    }

    T_LIB_EXIT(AMS_53);
}
#endif

