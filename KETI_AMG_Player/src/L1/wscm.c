/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Implementation of the Socket Connection Manager Wrapper (WSCM)
File:           wscm.c
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
  * \brief    Implementation of the Socket Connection Manager Wrapper (WSCM)
  * \details  This module provides ports and sockets to form routing paths
  *           inside the INIC. Most of the functionality of this module is
  *           provided by functions and properties inside the INIC. In this
  *           case the module works here as proxy between INIC and EHC
  */


#include "mns.h"
#include "ams.h"
#include "wads.h"
#include "wscm.h"
#include "vmsv.h"
#include "wscm_pv.h"


/*
==============================================================================
    NetServices trace: module trace macros
==============================================================================
*/

#if (MNS_DEBUG & NST_C_FUNC_ENTRIES)
    #define T_API_ENTRY(func)   MNS_TRACE((MNS_P_SRV_WSCM, NST_E_FUNC_ENTRY_API, 1, func))
    #define T_LIB_ENTRY(func)   MNS_TRACE((MNS_P_SRV_WSCM, NST_E_FUNC_ENTRY_LIB, 1, func))
    #define T_MOD_ENTRY(func)   MNS_TRACE((MNS_P_SRV_WSCM, NST_E_FUNC_ENTRY_MOD, 1, func))
#else
    #define T_API_ENTRY(func)
    #define T_LIB_ENTRY(func)
    #define T_MOD_ENTRY(func)
#endif

#if (MNS_DEBUG & NST_C_FUNC_EXITS)
    #define T_API_EXIT(func)    MNS_TRACE((MNS_P_SRV_WSCM, NST_E_FUNC_EXIT_API, 1, func))
    #define T_LIB_EXIT(func)    MNS_TRACE((MNS_P_SRV_WSCM, NST_E_FUNC_EXIT_LIB, 1, func))
    #define T_MOD_EXIT(func)    MNS_TRACE((MNS_P_SRV_WSCM, NST_E_FUNC_EXIT_MOD, 1, func))
#else
    #define T_API_EXIT(func)
    #define T_LIB_EXIT(func)
    #define T_MOD_EXIT(func)
#endif

#if (MNS_DEBUG & NST_C_INIT)
    #define T_INIT()            MNS_TRACE((MNS_P_SRV_WSCM, NST_E_INIT, 0))
#else
    #define T_INIT()
#endif

#if (MNS_DEBUG & NST_C_SERVICE)
    #define T_SERVICE(event)    MNS_TRACE((MNS_P_SRV_WSCM, NST_E_SERVICE, 1, event))
#else
    #define T_SERVICE(event)
#endif

#if (MNS_DEBUG & NST_C_REQUESTS)
    #define T_REQUEST(event)    MNS_TRACE((MNS_P_SRV_WSCM, NST_E_REQUEST, 1, event))
#else
    #define T_REQUEST(event)
#endif

#if (MNS_DEBUG & NST_C_ASSERTS)
    #define FAILED_ASSERT()     MNS_TRACE((MNS_P_SRV_WSCM, NST_E_ASSERT, 1, __LINE__))
    #define ASSERT(exp)         if(!(exp)) FAILED_ASSERT()
#else
    #define FAILED_ASSERT()
    #define ASSERT(exp)
#endif

#define TAKE_EVENTS()               WAIT4MX(MX_WSCM_PE)
#define GIVE_EVENTS()               REL_MX(MX_WSCM_PE)
#define WAIT4CS()                   WAIT4MX(MX_WSCM_CS)
#define REL_CS()                    REL_MX(MX_WSCM_CS)

/*
================================================================================
    Module Internal Variables
================================================================================
*/

#ifdef SCM_MIN

    static WscmData wscm;

    static _CONST WscmErrCodes wscm_err_codes[] =
    {
        { FUNCID_INIC_CREATESOCKET,             0xA1 },
        { FUNCID_INIC_DESTROYSOCKET,            0xA2 },
        { FUNCID_INIC_CONNECTSOCKETS,           0xA3 },
        { FUNCID_INIC_DISCONNECTSOCKETS,        0xA4 },
        { FUNCID_INIC_MEDIALBALLOCATEONLY,      0xA5 },
        { FUNCID_INIC_MEDIALBDEALLOCATEONLY,    0xA6 },
        { FUNCID_INIC_MUTECONNECTION,           0xA9 },
        { FUNCID_INIC_DEMUTECONNECTION,         0xAA },
        { FUNCID_INIC_SOURCEDROP,               0xAB },
        { FUNCID_INIC_REMOTEGETSOURCE,          0xAD },
        { 0x00,                                 0x00 }
    };

#endif


/*
==============================================================================
==============================================================================
    Module Implementation
==============================================================================
==============================================================================
*/
#ifdef SCM_0
void ScmInit(struct TScmConfig *cfg_ptr)
{
    byte mode;

    T_LIB_ENTRY(SCM_0);
    T_INIT();

    TAKE_EVENTS();
    wscm.pending_events       = WSCM_P_NONE;
    wscm.latest_handled_event = WSCM_P_NONE;
    GIVE_EVENTS();

    #ifndef ADS_MIN
    if (SCM_PM_AUTO == cfg_ptr->packet.mode)
    {
        cfg_ptr->packet.mode = SCM_PM_NONE;
        FAILED_ASSERT();    /* Packet mode SMC_PM_AUTO is selected, while ADS_MIN is not defined */
    }
    #endif

    WAIT4CS();
    wscm.cfg_ptr                            = cfg_ptr;
    wscm.api_locked                         = MNS_FALSE;

    wscm.pm.desc.in.port_id                 = SCM_PORT_ID_MEDIALB;
    wscm.pm.desc.in.direction               = SCM_IN;
    wscm.pm.desc.in.datatype                = SCM_TYPE_PACKET;
    wscm.pm.desc.in.blockwidth              = cfg_ptr->packet.in.bandwidth;
    wscm.pm.desc.in.medialb.channel_addr    = cfg_ptr->packet.in.address;

    wscm.pm.desc.out.port_id                = SCM_PORT_ID_MEDIALB;
    wscm.pm.desc.out.direction              = SCM_OUT;
    wscm.pm.desc.out.datatype               = SCM_TYPE_PACKET;
    wscm.pm.desc.out.blockwidth             = cfg_ptr->packet.out.bandwidth;
    wscm.pm.desc.out.medialb.channel_addr   = cfg_ptr->packet.out.address;

    wscm.pm.desc.portdesc.port_id                       = SCM_PORT_ID_MEDIALB;
    wscm.pm.desc.portdesc.config.medialb.port_mode      = SCM_PORT_CFG_MLB_MODE_CTL;
    wscm.pm.desc.portdesc.config.medialb.clock_config   = cfg_ptr->packet.clock_config;

    wscm.pm.state                           = SCM_PM_NEW_BORN;
    wscm.pm.statebuf                        = SCM_PM_NEW_BORN;
    mode                                    = wscm.cfg_ptr->packet.mode;
    wscm.pm.mediaLBportOpen                 = MNS_FALSE;
    REL_CS();

    if(SCM_PM_NONE != mode)
    {
        MostRegisterTimer(&(wscm.pm.timer),ScmPMRecheck, 0);
    }

    T_LIB_EXIT(SCM_0);
}
#endif

#ifdef SCM_1
void ScmService(void)
{
    word event_to_handle;
    bool request_flag;

    T_LIB_ENTRY(SCM_1);

    event_to_handle = ScmGetNextEventToHandle();
    request_flag    = MNS_FALSE;

    T_SERVICE(event_to_handle);

    switch (event_to_handle)
    {
        case WSCM_P_GO_PROTECTED:
            ScmGoProtected();
            break;

        case WSCM_P_NTF_COMPLETE:
            MnsServiceInitComplete(MNS_PHASE_INIT, MNS_P_SRV_WSCM);
            break;

        case WSCM_P_PM:
            ScmPMService();
            break;

        default:
            FAILED_ASSERT();
            event_to_handle = WSCM_P_NONE;
            break;
    }

    TAKE_EVENTS();
    request_flag = (WSCM_P_NONE != wscm.pending_events) ? MNS_TRUE : MNS_FALSE;
    GIVE_EVENTS();

    if (MNS_FALSE != request_flag)
    {
        MnsSetPendingService(MNS_P_SRV_WSCM);
    }

    T_LIB_EXIT(SCM_1);
}
#endif

#ifdef SCM_2
void ScmSetPendingEvent(word event_flag)
{
    T_LIB_ENTRY(SCM_2);

    T_REQUEST(event_flag);
    MnsSetPendingEventFlag(event_flag, MX_WSCM_PE,
                           &wscm.pending_events, MNS_P_SRV_WSCM);
    T_LIB_EXIT(SCM_2);
}
#endif

#ifdef SCM_3
static word ScmGetNextEventToHandle(void)
{
    word result;

    T_MOD_ENTRY(SCM_3);
    result = MnsGetNextEventFlagToCall(MX_WSCM_PE,
                                       &wscm.pending_events,
                                       &wscm.latest_handled_event,
                                       WSCM_P_FIRST, WSCM_P_LAST);
    T_MOD_EXIT(SCM_3);
    return(result);
}
#endif

#ifdef SCM_4
void ScmGoProtected(void)
{
    T_MOD_ENTRY(SCM_4);

    WAIT4CS();
        wscm.result_list_ptr       = NULL;
        wscm.cb_ptr                = NULL;
        wscm.cb_func_id            = INIC_SHADOW_INVALID_WORD;
        wscm.mute_mode             = INIC_SHADOW_INVALID_BYTE;
        wscm.api_locked            = MNS_FALSE;
        wscm.api_ext               = MNS_FALSE;
        wscm.bandwidth.preset      = INIC_SHADOW_INVALID_BYTE;
        wscm.bandwidth.current     = INIC_SHADOW_INVALID_BYTE;
        wscm.bandwidth.total       = INIC_SHADOW_INVALID_BYTE;
        wscm.bandwidth.free        = INIC_SHADOW_INVALID_WORD;
        wscm.bandwidth.packet      = INIC_SHADOW_INVALID_BYTE;
        wscm.bandwidth.used        = INIC_SHADOW_INVALID_BYTE;
        wscm.pm.state             &= SCM_PM_KILL_WAIT_STATE; /* kill wait states */
        wscm.pm.handle.in          = SCM_HANDLE_INVALID;
        wscm.pm.handle.out         = SCM_HANDLE_INVALID;
        wscm.pm.mediaLBportOpen    = MNS_FALSE;
    REL_CS();

    MnsServiceInitComplete(MNS_PHASE_RESET, MNS_P_SRV_WSCM);

    T_MOD_EXIT(SCM_4);
}
#endif


#ifdef SCM_7
byte ScmOpenPort(TScmPortDesc *desc_ptr, TMnsStdCB *cb_ptr)
{
    byte        result;
    TMnsResult  cb_res;
    bool        go;
    bool        release_lock;

    T_API_ENTRY(SCM_7);
    ASSERT(desc_ptr);

    result          = ERR_NO;
    cb_res          = NSR_S_OK;
    go              = MNS_FALSE;
    release_lock    = MNS_FALSE;

    WAIT4CS();
    if (MNS_FALSE == wscm.api_locked)
    {
        wscm.api_locked = MNS_TRUE;
        wscm.cb_ptr     = (TMnsStdCB *) cb_ptr;
        wscm.cb_func_id = FUNCID_INIC_OPENPORT;
        go              = MNS_TRUE;
    }
    REL_CS();

    if (MNS_FALSE != go)
    {
        TMsgTx* msg_ptr;
        msg_ptr = MsgGetTxPtrExt(5);

        if (msg_ptr)
        {
            msg_ptr->Tgt_Adr    = MSG_TGT_INIC;
            msg_ptr->FBlock_ID  = FBLOCK_INIC;
            msg_ptr->Func_ID    = FUNCID_INIC_OPENPORT;
            msg_ptr->Operation  = OP_STARTRESULT;
            msg_ptr->Data[0]    = desc_ptr->port_id;

            switch (SCM_PORT_ID_INST_MASK & desc_ptr->port_id) /* masked out instances */
            {
                 case SCM_PORT_ID_MEDIALB:
                    msg_ptr->Data[1] = (byte)MEDIALB_AUTO;
                    msg_ptr->Data[2] = desc_ptr->config.medialb.port_mode;
                    msg_ptr->Data[3] = desc_ptr->config.medialb.clock_config;
                    msg_ptr->Length  = 4;
                    MsgSend3(msg_ptr);   /* handler will release the lock and call cb_ptr */
                    break;

                case SCM_PORT_ID_STREAM:
                    msg_ptr->Data[1] = desc_ptr->config.streaming.clock_drive_mode;
                    msg_ptr->Data[2] = desc_ptr->config.streaming.port_mode;
                    msg_ptr->Data[3] = desc_ptr->config.streaming.data_format;
                    msg_ptr->Length  = 4;
                    MsgSend3(msg_ptr);   /* handler will release the lock and call cb_ptr */
                    break;

                #ifdef _OS81110_ISO
                case SCM_PORT_ID_TRANSPORT:
                    msg_ptr->Data[1] = desc_ptr->config.transport.port_mode;
                    msg_ptr->Length  = 2;
                    MsgSend3(msg_ptr);   /* handler will release the lock and call cb_ptr */
                    break;
                #endif

                #ifdef _OS81110_ISO
                case SCM_PORT_ID_RMCK:
                    msg_ptr->Data[1] = desc_ptr->config.rmck.drive_mode;
                    msg_ptr->Length = 2;
                    if( SCM_RMCK_SCKFSY_INIC_OUT == desc_ptr->config.rmck.drive_mode )
                    {
                        msg_ptr->Data[2] = desc_ptr->config.rmck.divider;
                        msg_ptr->Data[3] = desc_ptr->config.rmck.clock_source;
                        msg_ptr->Length = 4;
                        if (SCM_RMCK_ISOC_LINKED_TO_PORT==desc_ptr->config.rmck.clock_source)
                        {
                            msg_ptr->Data[4] = desc_ptr->config.rmck.linked_to_port_id;
                            msg_ptr->Length = 5;
                        }
                    }
                    MsgSend3(msg_ptr);   /* handler will release the lock and call cb_ptr */
                    break;
                #endif

                #ifdef _OS81110_SPI
                case SCM_PORT_ID_SPI:
                    msg_ptr->Data[1] = desc_ptr->config.spi.port_mode;
                    msg_ptr->Data[2] = desc_ptr->config.spi.clock_mode;
                    msg_ptr->Data[3] = desc_ptr->config.spi.interrupt_threshold;
                    msg_ptr->Length  = 4;
                    MsgSend3(msg_ptr);   /* handler will release the lock and call cb_ptr */
                    break;
                #endif

                default:
                    release_lock = MNS_TRUE;
                    result = ERR_PARAM;
                    cb_res = NSR_E_PARAM;
                    MsgTxUnused(msg_ptr);
                    FAILED_ASSERT();
                    break;
            }
        }
        else
        {
            result = ERR_BUFOV;
            cb_res = NSR_E_BUSY;
            release_lock = MNS_TRUE;
        }
    }
    else    /* api was already locked */
    {
         result = ERR_BUFOV;
         cb_res = NSR_E_BUSY;
    }

    if ( MNS_FALSE != release_lock )
    {
        WAIT4CS();
        wscm.api_locked = MNS_FALSE;
        wscm.cb_ptr     = NULL;
        wscm.cb_func_id = INIC_SHADOW_INVALID_WORD;
        REL_CS();
    }

    if ( (ERR_NO != result) && cb_ptr)
    {
        cb_ptr(cb_res);
    }

    T_API_EXIT(SCM_7);
    return(result);
}
#endif


#ifdef SCM_8

byte ScmCreateSocket(TScmSocketDesc *desc_ptr, TScmCreateSocketCB *cb_ptr)
{
    byte result;
    TMnsResult cb_res;
    bool go;
    bool release_lock;

    T_API_ENTRY(SCM_8);
    ASSERT(desc_ptr);

    result = ERR_NO;
    cb_res = NSR_S_OK;
    go     = MNS_FALSE;
    release_lock = MNS_FALSE;

    WAIT4CS();
    if (MNS_FALSE == wscm.api_locked)
    {
        wscm.api_locked  = MNS_TRUE;
        wscm.cb_ptr      = (TMnsStdCB *) cb_ptr;
        wscm.cb_func_id  = FUNCID_INIC_CREATESOCKET;
        go = MNS_TRUE;
    }
    REL_CS();

    if (MNS_FALSE != go)
    {
        TMsgTx *msg_ptr;
        msg_ptr = MsgGetTxPtrExt(40);

        if (msg_ptr)
        {
            msg_ptr->Tgt_Adr    = MSG_TGT_INIC;
            msg_ptr->FBlock_ID  = FBLOCK_INIC;
            msg_ptr->Func_ID    = FUNCID_INIC_CREATESOCKET;
            msg_ptr->Operation  = OP_STARTRESULT;
            msg_ptr->Data[0]    = desc_ptr->port_id;
            msg_ptr->Data[1]    = desc_ptr->direction;
            msg_ptr->Data[2]    = desc_ptr->datatype;
            msg_ptr->Data[3]    = HB(desc_ptr->blockwidth);
            msg_ptr->Data[4]    = LB(desc_ptr->blockwidth);
            msg_ptr->Length     = 5;
            switch (SCM_PORT_ID_INST_MASK & desc_ptr->port_id)
            {
                case SCM_PORT_ID_MEDIALB:
                    msg_ptr->Data[5] = HB(desc_ptr->medialb.channel_addr);
                    msg_ptr->Data[6] = LB(desc_ptr->medialb.channel_addr);
                    msg_ptr->Length  = 7;

                    if ((SCM_TYPE_SYNC == desc_ptr->datatype) &&
                        (MNS_FALSE != MostIsSupported(NSF_EXT_SOCKETS)))
                    {
                        if (desc_ptr->medialb.channel_blockwidth)
                        {
                            msg_ptr->Data[7] = desc_ptr->medialb.offset;
                            msg_ptr->Data[8] = desc_ptr->medialb.channel_blockwidth;
                            msg_ptr->Length  = 9;

                        }
                        else if (desc_ptr->medialb.offset)
                        {
                            msg_ptr->Data[7] = desc_ptr->medialb.offset;
                            msg_ptr->Length  = 8;
                        }
                    }
                    #ifdef _OS81110_ISO
                    else if(SCM_TYPE_ISOPACKET == desc_ptr->datatype)
                    {
                        msg_ptr->Data[7] = HB(desc_ptr->medialb.iso_packet_size);
                        msg_ptr->Data[8] = LB(desc_ptr->medialb.iso_packet_size);
                        msg_ptr->Data[9] = (MNS_FALSE != desc_ptr->medialb.flow_control_enable) ? (byte)0x01 : (byte)0x00;
                        msg_ptr->Length  = 10;
                    }
                    #endif

                    #ifdef _OS81110_ISO
                    else if(SCM_TYPE_ISODFDATA == desc_ptr->datatype)
                    {
                        msg_ptr->Data[7] = HB(desc_ptr->medialb.iso_packet_size);
                        msg_ptr->Data[8] = LB(desc_ptr->medialb.iso_packet_size);
                        msg_ptr->Data[9] = desc_ptr->medialb.data_sample_freq_ref;
                        msg_ptr->Data[10]= (MNS_FALSE != desc_ptr->medialb.flow_control_enable) ? (byte)0x01 : (byte)0x00;
                        msg_ptr->Length  = 11;
                    }
                    #endif

                    #ifdef _OS81110_ISO
                    else if(SCM_TYPE_ISODFPHASE == desc_ptr->datatype)
                    {
                        msg_ptr->Data[7] = HB(desc_ptr->medialb.iso_packet_size);
                        msg_ptr->Data[8] = LB(desc_ptr->medialb.iso_packet_size);
                        msg_ptr->Data[9] = desc_ptr->medialb.data_sample_freq_ref;
                        msg_ptr->Length  = 10;
                    }
                    #endif

                    MsgSend3(msg_ptr);  /* handler will release the lock and call cb_ptr */
                    break;

                case SCM_PORT_ID_MOST:
                    {
                    byte most_preallocated;
                    byte most_preallocated_pos = 5;

                    if (MNS_FALSE != MostIsSupported(NSF_CONN_LABELS))
                    {
                        if (SCM_OUT == desc_ptr->direction)
                        {
                            most_preallocated = SCM_PA_NOT;
                        }
                        else
                        {
                            if ((MNS_FALSE != MostIsSupported(NSF_MOST_150)) &&
                                (desc_ptr->most.flags & SCM_MOST_FLAGS_MULTIPLE_CONNECTIONS))
                            {
                                most_preallocated = SCM_PA_CL_MULTCON;
                            }
                            else
                            {
                                most_preallocated = SCM_PA_CL;
                            }
                        }
                    }
                    else
                    {
                        if (SCM_IN == desc_ptr->direction)
                        {
                            most_preallocated = SCM_PA_SBC;
                        }
                        else
                        {
                            most_preallocated = (desc_ptr->most.flags & SCM_MOST_FLAGS_SOURCE_CONNECT) ? SCM_PA_SBC : SCM_PA_NOT;
                        }
                    }

                    switch (desc_ptr->datatype)
                    {
                        case SCM_TYPE_SYNC:
                            most_preallocated_pos = 5;
                            break;
                        #ifdef _OS81110_QOS
                        case SCM_TYPE_QOS_IP_STREAM:
                            most_preallocated_pos = 5;
                            break;
                        #endif

                        #ifdef _OS81110_ISO
                        case SCM_TYPE_ISOPACKET:
                            msg_ptr->Data[5] = HB(desc_ptr->most.iso_packet_size);
                            msg_ptr->Data[6] = LB(desc_ptr->most.iso_packet_size);
                            most_preallocated_pos = 7;
                            break;
                        case SCM_TYPE_ISODFDATA:
                            msg_ptr->Data[5] = desc_ptr->most.data_frame_blockwidth;
                            msg_ptr->Data[6] = desc_ptr->most.data_sample_freq_ref;
                            most_preallocated_pos = 7;
                            break;
                        case SCM_TYPE_ISODFPHASE:
                            msg_ptr->Data[5] = desc_ptr->most.data_sample_freq_ref;
                            most_preallocated_pos = 6;
                            break;
                        #endif
                        default:
                            FAILED_ASSERT();
                            result = ERR_PARAM;
                            cb_res = NSR_E_PARAM;
                            MsgTxUnused(msg_ptr);
                            msg_ptr = NULL;
                            release_lock = MNS_TRUE;
                            break;
                    }

                    if (msg_ptr)
                    {
                        byte i = 0;

                        switch (most_preallocated)
                        {
                            case SCM_PA_NOT:        /* not preallocated */
                                msg_ptr->Data[most_preallocated_pos++] = most_preallocated;
                                msg_ptr->Length  = most_preallocated_pos;
                                WAIT4CS();
                                wscm.result_list_ptr = desc_ptr->most.result_list_ptr;
                                REL_CS();
                                break;
                            case SCM_PA_CL:         /* preallocated - connection label      */
                            case SCM_PA_CL_MULTCON: /*              - multiple connections  */
                                msg_ptr->Data[most_preallocated_pos++] = most_preallocated;
                                msg_ptr->Data[most_preallocated_pos++] = HB(desc_ptr->most.list_ptr[0]);
                                msg_ptr->Data[most_preallocated_pos++] = LB(desc_ptr->most.list_ptr[0]);
                                msg_ptr->Length  = most_preallocated_pos;
                                break;
                            case SCM_PA_SBC:        /* preallocated - single Byte channels */
                                msg_ptr->Data[most_preallocated_pos++] = most_preallocated;

                                for (; i < desc_ptr->most.list_len; ++i)
                                {
                                    msg_ptr->Data[most_preallocated_pos+i] = (byte) desc_ptr->most.list_ptr[i];
                                }

                                msg_ptr->Length = most_preallocated_pos + desc_ptr->most.list_len;
                                break;
                            default:
                                break;
                        }

                        MsgSend3(msg_ptr);  /* handler will release the lock and call cb_ptr */
                    }
                    }
                    break;

                case SCM_PORT_ID_STREAM: /*lint !e616 control flows into case/default intenionally! */
                    msg_ptr->Data[5] = desc_ptr->streaming.interface_id;
                    msg_ptr->Length  = 6;

                    if(SCM_TYPE_SYNC == desc_ptr->datatype)
                    {
                        if ((MNS_FALSE != MostIsSupported(NSF_EXT_SOCKETS)) &&
                            desc_ptr->streaming.offset)
                        {
                            msg_ptr->Data[6] = desc_ptr->streaming.offset;
                            msg_ptr->Length  = 7;
                        }
                    }
                    #ifdef _OS81110_ISO
                    else if(SCM_TYPE_ISODFDATA == desc_ptr->datatype)
                    {
                        msg_ptr->Data[6] = desc_ptr->streaming.data_sample_freq_ref;
                        msg_ptr->Length = 7;
                    }
                    else if(SCM_TYPE_ISODFPHASE == desc_ptr->datatype)
                    {
                        msg_ptr->Data[6] = desc_ptr->streaming.data_sample_freq_ref;
                        msg_ptr->Length = 7;
                    }
                    #endif
                    MsgSend3(msg_ptr);  /* handler will release the lock and call cb_ptr */
                    break;

                #ifdef _OS81110_ISO
                case SCM_PORT_ID_TRANSPORT:
                    msg_ptr->Data[5] = HB(desc_ptr->transport.iso_packet_size);
                    msg_ptr->Data[6] = LB(desc_ptr->transport.iso_packet_size);
                    msg_ptr->Length  = 7;
                    MsgSend3(msg_ptr);  /* handler will release the lock and call cb_ptr */
                    break;
                #endif

                #ifdef _OS81110_ISO
                case SCM_PORT_ID_RMCK:
                    msg_ptr->Data[5] = desc_ptr->rmck.data_sample_freq_ref;
                    msg_ptr->Length  = 6;
                    MsgSend3(msg_ptr);  /* handler will release the lock and call cb_ptr */
                    break;
                #endif

                #ifdef _OS81110_SPI
                case SCM_PORT_ID_SPI:
                    if(SCM_TYPE_PACKET == desc_ptr->datatype)
                    {
                        msg_ptr->Data[3]    = (byte)0xFF;
                        msg_ptr->Data[4]    = (byte)0xFF;
                    }
                    MsgSend3(msg_ptr);  /* handler will release the lock and call cb_ptr */
                    break;
                #endif

                default:
                    FAILED_ASSERT();
                    result = ERR_PARAM;
                    cb_res = NSR_E_PARAM;
                    MsgTxUnused(msg_ptr);
                    release_lock = MNS_TRUE;
                    break;
            }   /* switch */
        }
        else    /* no msg_ptr */
        {
            result = ERR_BUFOV;
            cb_res = NSR_E_BUSY;
            release_lock = MNS_TRUE;
        }
    }
    else    /* api was already locked */
    {
        result = ERR_BUFOV;
        cb_res = NSR_E_BUSY;
    }

    if ( MNS_FALSE != release_lock )
    {
        WAIT4CS();
        wscm.api_locked = MNS_FALSE;
        wscm.cb_ptr     = NULL;
        wscm.cb_func_id = INIC_SHADOW_INVALID_WORD;
        REL_CS();
    }

    if ( (ERR_NO != result) && cb_ptr)
    {
        cb_ptr(cb_res, 0, 0, NULL);
    }

    T_API_EXIT(SCM_8);
    return(result);
}
#endif




#ifdef SCM_10
byte ScmGetMuteMode(void)
{
    byte result;

    T_API_ENTRY(SCM_10);
    WAIT4CS();
    result = wscm.mute_mode;
    REL_CS();
    T_API_EXIT(SCM_10);

    return(result);
}
#endif

#ifdef SCM_11
byte ScmGetBoundary(void)
{
    byte result;

    T_API_ENTRY(SCM_11);
    WAIT4CS();
    result = wscm.bandwidth.current;
    REL_CS();
    T_API_EXIT(SCM_11);

    return(result);
}
#endif




#ifdef SCM_15
byte ScmAllocOnlyMlb(word channel_addr, byte bandwidth,
                     TScmAllocOnlyMlbCB *cb_ptr)
{
    bool        go;
    byte        result;
    TMnsResult  cb_res;

    T_API_ENTRY(SCM_15);

    go      = MNS_FALSE;
    result  = ERR_NO;
    cb_res  = NSR_S_OK;

    WAIT4CS();
    if (MNS_FALSE == wscm.api_locked)
    {
        wscm.api_locked  = MNS_TRUE;
        go               = MNS_TRUE;
        wscm.cb_ptr      = (TMnsStdCB *) cb_ptr;
        wscm.cb_func_id  = FUNCID_INIC_MEDIALBALLOCATEONLY;
    }
    REL_CS();

    if (MNS_FALSE != go)
    {
        TMsgTx *msg_ptr;
        msg_ptr = MsgGetTxPtrExt(3);

        if (msg_ptr)
        {
            msg_ptr->Tgt_Adr    = MSG_TGT_INIC;
            msg_ptr->FBlock_ID  = FBLOCK_INIC;
            msg_ptr->Func_ID    = FUNCID_INIC_MEDIALBALLOCATEONLY;
            msg_ptr->Operation  = OP_STARTRESULT;
            msg_ptr->Data[0]    = HB(channel_addr);
            msg_ptr->Data[1]    = LB(channel_addr);
            msg_ptr->Data[2]    = bandwidth;

            MsgSend3(msg_ptr);  /* handler will release the lock and call cb_ptr */
        }
        else
        {
            result  = ERR_BUFOV;
            cb_res  = NSR_E_BUSY;
            WAIT4CS();
            wscm.api_locked  = MNS_FALSE;
            wscm.cb_ptr      = NULL;
            wscm.cb_func_id  = INIC_SHADOW_INVALID_WORD;
            REL_CS();
        }
    }
    else    /* api was already locked */
    {
        result  = ERR_BUFOV;
        cb_res  = NSR_E_BUSY;
    }

    if ( (ERR_NO != result) && cb_ptr)
    {
        cb_ptr(cb_res, 0);
    }

    T_API_EXIT(SCM_15);
    return(result);
}
#endif


#ifdef SCM_16
byte ScmDeallocOnlyMlb(word channel_addr, TMnsStdCB *cb_ptr)
{
    bool        go;
    byte        result;
    TMnsResult  cb_res;

    T_API_ENTRY(SCM_16);

    go      = MNS_FALSE;
    result  = ERR_NO;
    cb_res  = NSR_S_OK;

    WAIT4CS();
    if (MNS_FALSE == wscm.api_locked)
    {
        wscm.api_locked  = MNS_TRUE;
        go               = MNS_TRUE;
        wscm.cb_ptr      = (TMnsStdCB *) cb_ptr;
        wscm.cb_func_id  = FUNCID_INIC_MEDIALBDEALLOCATEONLY;
    }
    REL_CS();

    if (MNS_FALSE != go)
    {
        TMsgTx *msg_ptr = MsgGetTxPtrExt(2);

        if (msg_ptr)
        {
            msg_ptr->Tgt_Adr    = MSG_TGT_INIC;
            msg_ptr->FBlock_ID  = FBLOCK_INIC;
            msg_ptr->Func_ID    = FUNCID_INIC_MEDIALBDEALLOCATEONLY;
            msg_ptr->Operation  = OP_STARTRESULT;
            msg_ptr->Data[0]    = HB(channel_addr);
            msg_ptr->Data[1]    = LB(channel_addr);

            MsgSend3(msg_ptr);  /* handler will release the lock and call cb_ptr */
        }
        else
        {
            result  = ERR_BUFOV;
            cb_res  = NSR_E_BUSY;
            WAIT4CS();
            wscm.api_locked  = MNS_FALSE;
            wscm.cb_ptr      = NULL;
            wscm.cb_func_id  = INIC_SHADOW_INVALID_WORD;
            REL_CS();
        }
    }
    else /* api was already locked */
    {
        result  = ERR_BUFOV;
        cb_res  = NSR_E_BUSY;
    }

    if ( (ERR_NO != result) && cb_ptr)
    {
        cb_ptr(cb_res);
    }

    T_API_EXIT(SCM_16);
    return(result);
}
#endif

#ifdef SCM_5
byte ScmSendHandleMsg(byte handle, TMnsStdCB *cb_ptr, word func_id)
{
    bool        go;
    byte        result;
    TMnsResult  cb_res;

    T_API_ENTRY(SCM_5);

    go      = MNS_FALSE;
    result  = ERR_NO;
    cb_res  = NSR_S_OK;

    WAIT4CS();
    if (MNS_FALSE == wscm.api_locked)
    {
        wscm.api_locked  = MNS_TRUE;
        go               = MNS_TRUE;
        wscm.cb_ptr      = (TMnsStdCB *) cb_ptr;
        wscm.cb_func_id  = func_id;
    }
    REL_CS();

    if (MNS_FALSE != go)
    {
        TMsgTx *msg_ptr;
        msg_ptr = MsgGetTxPtrExt(1);

        if (msg_ptr)
        {
            msg_ptr->Tgt_Adr    = MSG_TGT_INIC;
            msg_ptr->FBlock_ID  = FBLOCK_INIC;
            msg_ptr->Func_ID    = func_id;
            msg_ptr->Operation  = OP_STARTRESULT;
            msg_ptr->Data[0]    = handle;

            MsgSend3(msg_ptr);  /* handler will release the lock and call cb_ptr */
        }
        else
        {
            result  = ERR_BUFOV;
            cb_res  = NSR_E_BUSY;
            WAIT4CS();
            wscm.api_locked  = MNS_FALSE;
            wscm.cb_ptr      = NULL;
            wscm.cb_func_id  = INIC_SHADOW_INVALID_WORD;
            REL_CS();
        }
    }
    else /* api was already locked */
    {
        result  = ERR_BUFOV;
        cb_res  = NSR_E_BUSY;
    }

    if ( (ERR_NO != result) && cb_ptr)
    {
        cb_ptr(cb_res);
    }

    T_API_EXIT(SCM_5);
    return(result);
}
#endif


#ifdef SCM_6
byte ScmSetByteByMsg(byte value, TMnsStdCB *cb_ptr, word func_id)
{
    bool        go;
    bool        fire_cb_now; /* if "MNS_TRUE": will fire callback with actual error code before function is left */
    bool        new_val;
    byte        result;
    TMnsResult  cb_res;

    T_API_ENTRY(SCM_6);

    go          = MNS_FALSE;
    fire_cb_now = MNS_FALSE;
    new_val     = MNS_TRUE;
    result      = ERR_NO;
    cb_res      = NSR_S_OK;

    WAIT4CS();
    switch (func_id)
    {
        case FUNCID_INIC_MUTEMODE:
            new_val = (value == wscm.mute_mode) ? MNS_FALSE : MNS_TRUE;
            break;
        case FUNCID_INIC_BANDWIDTH:
            new_val = (value == wscm.bandwidth.preset) ? MNS_FALSE : MNS_TRUE;
            break;
        default:
            new_val     = MNS_FALSE;
            result      = ERR_PARAM;
            cb_res      = NSR_E_PARAM;
            break;
    }

    if ((MNS_FALSE == wscm.api_locked) && (MNS_FALSE != new_val))
    {
        wscm.api_locked  = MNS_TRUE;
        go               = MNS_TRUE;
        wscm.cb_ptr      = (TMnsStdCB *) cb_ptr;
        wscm.cb_func_id  = func_id;
    }
    REL_CS();

    if (MNS_FALSE != go)
    {
        TMsgTx *msg_ptr;
        msg_ptr = MsgGetTxPtrExt(1);

        if (msg_ptr)
        {
            msg_ptr->Tgt_Adr    = MSG_TGT_INIC;
            msg_ptr->FBlock_ID  = FBLOCK_INIC;
            msg_ptr->Func_ID    = func_id;
            msg_ptr->Operation  = OP_SET;
            msg_ptr->Data[0]    = value;
            msg_ptr->Length     = 1;

            MsgSend3(msg_ptr);      /* handler will release the lock and call cb_ptr */
        }
        else
        {
            fire_cb_now = MNS_TRUE;
            result      = ERR_BUFOV;
            cb_res      = NSR_E_BUSY;
            WAIT4CS();
            wscm.api_locked  = MNS_FALSE;
            wscm.cb_ptr      = NULL;
            wscm.cb_func_id  = INIC_SHADOW_INVALID_WORD;
            REL_CS();
        }
    }
    else if (ERR_PARAM == result)   /* function called with wrong param func_id */
    {
        fire_cb_now = MNS_TRUE;
    }
    else if (MNS_FALSE != new_val)  /* api was already locked */
    {
        fire_cb_now = MNS_TRUE;
        result      = ERR_BUFOV;
        cb_res      = NSR_E_BUSY;
    }
    else                            /* value was already set */
    {
        fire_cb_now = MNS_TRUE;         /* send NSR_S_OK */
        result      = ERR_NO;
        cb_res      = NSR_S_OK;
    }

    if ((MNS_FALSE != fire_cb_now) && cb_ptr)
    {
        cb_ptr(cb_res);
    }

    T_API_EXIT(SCM_6);
    return(result);
}
#endif


#ifdef SCM_18
void ScmHandleResult(TMsgRx *msg_ptr)
{
    TMnsStdCB  *tmp_cb_ptr;
    TScmListCB *tmp2_cb_ptr;

    T_MOD_ENTRY(SCM_18);

    ASSERT(msg_ptr);

    tmp_cb_ptr = NULL;
    tmp2_cb_ptr = NULL;

    WAIT4CS();
    if (msg_ptr->Func_ID ==  wscm.cb_func_id)
    {
        switch (wscm.cb_func_id)
        {
            case FUNCID_INIC_DESTROYSOCKET:
                if (MNS_FALSE != wscm.api_ext)
                {
                    wscm.api_ext = MNS_FALSE;
                    tmp2_cb_ptr = (TScmListCB *) wscm.cb_ptr;
                    break;
                }
            /*lint -e(616,825) control flows into case/default intentionally */
            default:
                tmp_cb_ptr = wscm.cb_ptr;
                break;
        }
        wscm.cb_ptr     = NULL;
        wscm.api_locked = MNS_FALSE;
    }
    REL_CS();

    if (tmp_cb_ptr)
    {
        tmp_cb_ptr(NSR_S_OK);
    }
    else if (tmp2_cb_ptr)
    {
        tmp2_cb_ptr(NSR_S_OK, 0, NULL);
    }

    T_MOD_EXIT(SCM_18);
}
#endif

#ifdef SCM_19
void ScmHandleCreateSocketResult(TMsgRx *msg_ptr)
{
    TScmCreateSocketCB *tmp_cb_ptr;

    T_MOD_ENTRY(SCM_19);

    ASSERT(msg_ptr);

    tmp_cb_ptr = NULL;

    WAIT4CS();
    if (msg_ptr->Func_ID ==  wscm.cb_func_id)
    {
        tmp_cb_ptr       = (TScmCreateSocketCB *) wscm.cb_ptr;
        wscm.cb_ptr = NULL;
        wscm.api_locked = MNS_FALSE;
    }
    REL_CS();

    if (tmp_cb_ptr)
    {
        if (msg_ptr->Length == 1)                /* without ResultList */
        {
            tmp_cb_ptr(NSR_S_OK,                 /* result */
                       msg_ptr->Data[0],         /* socket_handle */
                       0,                        /* list_len */
                       NULL );                   /* list_ptr */
        }
        else  /* with ResultList */
        {
            byte  list_len = 0;
            word *list_ptr = NULL;

            WAIT4CS();
            if (wscm.result_list_ptr)
            {
                byte i = 0;

                list_ptr = wscm.result_list_ptr;

                if (MNS_FALSE != MostIsSupported(NSF_CONN_LABELS))
                {
                    list_len = 1;

                    *list_ptr  = (word) msg_ptr->Data[2] << 8;
                    *list_ptr |= (word) msg_ptr->Data[3];
                }
                else
                {
                    list_len = (byte) msg_ptr->Length - 2;

                    for (; i < list_len; ++i)
                    {
                        *list_ptr++ = (word)msg_ptr->Data[2+i];
                    }
                }

                list_ptr = wscm.result_list_ptr;
                wscm.result_list_ptr = NULL;
            }
            else
            {
                FAILED_ASSERT();
            }
            REL_CS();

            tmp_cb_ptr(NSR_S_OK,                        /* result */
                       msg_ptr->Data[0],                /* socket_handle */
                       list_len,                        /* list_len */
                       list_ptr );                      /* list_ptr */
        }
    }

    T_MOD_EXIT(SCM_19);
}
#endif

#ifdef SCM_20
void ScmHandleConnectSocketsResult(TMsgRx *msg_ptr)
{
    TScmConnectSocketsCB *tmp_cb_ptr;

    T_MOD_ENTRY(SCM_20);

    ASSERT(msg_ptr);

    tmp_cb_ptr = NULL;

    WAIT4CS();
    if (msg_ptr->Func_ID ==  wscm.cb_func_id)
    {
        tmp_cb_ptr       = (TScmConnectSocketsCB *) wscm.cb_ptr;
        wscm.cb_ptr = NULL;
        wscm.api_locked = MNS_FALSE;
    }
    REL_CS();

    if (tmp_cb_ptr)
    {
        tmp_cb_ptr(NSR_S_OK, msg_ptr->Data[0]);
    }

    T_MOD_EXIT(SCM_20);
}
#endif

#ifdef SCM_21
void ScmHandleMuteModeStatus(TMsgRx *msg_ptr)
{
    TMnsStdCB *tmp_cb_ptr;

    T_MOD_ENTRY(SCM_21);

    ASSERT(msg_ptr);

    tmp_cb_ptr = NULL;

    WAIT4CS();
    if (msg_ptr->Func_ID ==  wscm.cb_func_id)
    {
        tmp_cb_ptr       = wscm.cb_ptr;
        wscm.cb_ptr = NULL;
        wscm.api_locked = MNS_FALSE;
    }
    wscm.mute_mode = msg_ptr->Data[0];
    REL_CS();

    if (tmp_cb_ptr)
    {
        tmp_cb_ptr(NSR_S_OK);
    }
    MnsNtfCheck(NTF_MUTEMODE);

    T_MOD_EXIT(SCM_21);
}
#endif

#ifdef SCM_22
void ScmHandleMlbAllocOnlyResult(TMsgRx *msg_ptr)
{
    TScmAllocOnlyMlbCB *tmp_cb_ptr;

    T_MOD_ENTRY(SCM_22);

    ASSERT(msg_ptr);

    tmp_cb_ptr = NULL;

    WAIT4CS();
    if (msg_ptr->Func_ID ==  wscm.cb_func_id)
    {
        tmp_cb_ptr       = (TScmAllocOnlyMlbCB *) wscm.cb_ptr;
        wscm.cb_ptr = NULL;
        wscm.api_locked = MNS_FALSE;
    }
    REL_CS();

    if (tmp_cb_ptr)
    {
        tmp_cb_ptr(NSR_S_OK, msg_ptr->Data[0]);
    }

    T_MOD_EXIT(SCM_22);
}
#endif


#ifdef SCM_24

void ScmHandleBandwidthStatus(TMsgRx *msg_ptr)
{
    bool      most25;
    TMnsStdCB *tmp_cb_ptr;

    T_MOD_ENTRY(SCM_24);

    ASSERT(msg_ptr);
    tmp_cb_ptr = NULL;
    most25 = MostIsSupported(NSF_MOST_25);

    WAIT4CS();
    if (msg_ptr->Func_ID ==  wscm.cb_func_id)
    {
        tmp_cb_ptr           = wscm.cb_ptr;
        wscm.cb_ptr     = NULL;
        wscm.api_locked = MNS_FALSE;
    }

    /* if new bamdwidth */
    if(wscm.bandwidth.packet != (byte)(msg_ptr->Data[2] - msg_ptr->Data[1]))
    {
        wscm.pm.statebuf |= SCM_PM_W_BANDWIDTH;
    }

    wscm.bandwidth.preset  = msg_ptr->Data[0];                                              /* AssignBWInit */
    wscm.bandwidth.current = msg_ptr->Data[1];                                              /* AssignBW     */
    wscm.bandwidth.total   = msg_ptr->Data[2];                                              /* TotalBW      */
    wscm.bandwidth.free    = (word)(msg_ptr->Data[3] << 8);                                 /* AssignBWFree HB */
    wscm.bandwidth.free   |= (word) msg_ptr->Data[4];                                       /* AssignBWFree LB */
    wscm.bandwidth.used    = (byte)(wscm.bandwidth.current - (wscm.bandwidth.free >> 2));

    if ((0 == wscm.bandwidth.current) && (MNS_FALSE != most25))
    {
        wscm.bandwidth.packet  = 0;
    }
    else
    {
        wscm.bandwidth.packet  = (byte)(wscm.bandwidth.total - wscm.bandwidth.current);
    }
    REL_CS();

    /* MSVAL_E_NETON shall only be communicated when the
     * boundary is valid
     * -> implicit check if MSVAL_E_NETON was / shall be reported
     * see also VmsvHandleNIStateChange() and MostGetState()
     */
    (void)MostGetState();

    if (tmp_cb_ptr)
    {
        tmp_cb_ptr(NSR_S_OK);
    }

    ScmPMCheck();

    if (!EHCISTATE_IS_ATTACHED())
    {
        MnsNtfCheck(NTF_BANDWIDTH);
    }
    else
    {
        word max_bw = 0;

        if (wscm.cfg_ptr->bandwidth_changed_fptr)
        {
            wscm.cfg_ptr->bandwidth_changed_fptr(
              wscm.bandwidth.preset, wscm.bandwidth.current,
              wscm.bandwidth.total, wscm.bandwidth.free,
              wscm.bandwidth.packet, wscm.bandwidth.used);
        }

        max_bw = (word) (wscm.bandwidth.current << 2);
        if(MNS_FALSE != MostIsSupported(NSF_MOST_50))
        {
            max_bw++; /* left over byte */
        }

        if (wscm.cfg_ptr->on_error_fptr && (wscm.bandwidth.free == max_bw))
        {
            wscm.cfg_ptr->on_error_fptr(NSR_E_MOST_DEALLOCATED_ALL,
                                             SCM_HANDLE_INVALID);
        }
    }

    T_MOD_EXIT(SCM_24);
}
#endif

#ifdef SCM_26
void ScmHandleError(TMsgRx *msg_ptr)
{
    TMnsResult result;
    TMnsStdCB *tmp_cb_ptr;

    T_MOD_ENTRY(SCM_26);

    ASSERT(msg_ptr);
    result     = NSR_BUILD(MNS_FALSE, 0xA0, msg_ptr->Data[0]);
    tmp_cb_ptr = NULL;

    if (NSR_E_ERR_FUNC_SPECIFIC == result)
    {
        WscmErrCodes _CONST *err_ptr = &wscm_err_codes[0];

        while ((err_ptr->func_id) && (err_ptr->func_id != msg_ptr->Func_ID))
        {
            err_ptr++;
        }

        if ((err_ptr->func_id) && (err_ptr->func_id == msg_ptr->Func_ID))
        {
            result = NSR_BUILD(MNS_FALSE, err_ptr->func_code, msg_ptr->Data[1]);
        }
    }

    WAIT4CS();
    if (msg_ptr->Func_ID ==  wscm.cb_func_id)
    {
        tmp_cb_ptr       = wscm.cb_ptr;
        wscm.cb_ptr = NULL;
        wscm.api_locked = MNS_FALSE;
    }
    REL_CS();

    if (tmp_cb_ptr)
    {
        switch (msg_ptr->Func_ID)
        {
            case FUNCID_INIC_CREATESOCKET:
                {
                    byte                len         = 0;
                    word               *list_ptr    = NULL;
                    TScmCreateSocketCB *tmp2_cb_ptr =
                        (TScmCreateSocketCB *) tmp_cb_ptr;

                    ScmAssembleErrResList(msg_ptr, &len, &list_ptr);
                    tmp2_cb_ptr(result, 0, len, list_ptr);
                }
                break;

            case FUNCID_INIC_REMOTEGETSOURCE:
            {
                TScmGetSourceCB *tmp2_cb_ptr =
                    (TScmGetSourceCB *) tmp_cb_ptr;

                tmp2_cb_ptr(result, 0, 0, 0);
            }
                break;

            case FUNCID_INIC_CONNECTSOCKETS:
                {
                    TScmConnectSocketsCB *tmp2_cb_ptr =
                        (TScmConnectSocketsCB *) tmp_cb_ptr;
                    tmp2_cb_ptr(result, 0);
                }
                break;


            case FUNCID_INIC_MEDIALBALLOCATEONLY:
                {
                    TScmAllocOnlyMlbCB *tmp2_cb_ptr =
                        (TScmAllocOnlyMlbCB *) tmp_cb_ptr;
                    tmp2_cb_ptr(result, 0);
                }
                break;

            case FUNCID_INIC_DESTROYSOCKET:
            {
                bool api_ext = MNS_FALSE;

                WAIT4CS();
                api_ext = wscm.api_ext;
                wscm.api_ext = MNS_FALSE;
                REL_CS();

                if (MNS_FALSE != api_ext)
                {
                    byte                 len         = 0;
                    word                *list_ptr    = NULL;
                    TScmListCB *tmp2_cb_ptr =
                        (TScmListCB *) tmp_cb_ptr;

                    ScmAssembleErrResList(msg_ptr, &len, &list_ptr);
                    tmp2_cb_ptr(result, len, list_ptr);
                }
                else
                {
                    tmp_cb_ptr(result);
                }
            }
                break;

            default:
                tmp_cb_ptr(result);
                break;
        }
    }

    T_MOD_EXIT(SCM_26);
}
#endif

#ifdef SCM_28
void ScmHandleSourceDropStatus(TMsgRx *msg_ptr)
{
    T_LIB_ENTRY(SCM_28);

    ASSERT(msg_ptr);

    if (wscm.cfg_ptr->on_error_fptr && EHCISTATE_IS_ATTACHED())
    {
        wscm.cfg_ptr->on_error_fptr(NSR_E_SOURCE_DROP, msg_ptr->Data[0]);
    }

    T_LIB_EXIT(SCM_28);
}
#endif

#ifdef SCM_29
void ScmHandleSCErrorStatus(TMsgRx *msg_ptr)
{
    TMnsResult result;
    bool check_pm;

    T_LIB_ENTRY(SCM_29);

    ASSERT(msg_ptr);

    result   = NSR_E_FAILED;
    check_pm = MNS_FALSE;

    if(!((0xFF == msg_ptr->Data[0]) && (0x00 == msg_ptr->Data[1])))
    {
        switch(msg_ptr->Data[1])
        {
            case SCM_SCERROR_MLB:
                switch (msg_ptr->Data[0])
                {
                    case SCM_HANDLE_PACKET_IN:
                        result = NSR_E_PACKET_IN;
                        check_pm = MNS_TRUE;
                        WAIT4CS();
                        wscm.pm.handle.in = SCM_HANDLE_INVALID;
                        REL_CS();
                        break;

                    case SCM_HANDLE_PACKET_OUT:
                        result = NSR_E_PACKET_OUT;
                        check_pm = MNS_TRUE;
                        WAIT4CS();
                        wscm.pm.handle.out = SCM_HANDLE_INVALID;
                        REL_CS();
                        break;

                    default:
                        result = NSR_E_MEDIALB_BANDWIDTH;
                        break;
                }
                break;

            case SCM_SCERROR_MOST:
                result = NSR_E_MOST_BANDWIDTH;
                break;

            default:
                /* unknown */
                FAILED_ASSERT();
                break;
        }

        if(MNS_FALSE != check_pm)
        {
            ScmPMCheck();
        }

        if (wscm.cfg_ptr->on_error_fptr && EHCISTATE_IS_ATTACHED())
        {
            wscm.cfg_ptr->on_error_fptr(result, msg_ptr->Data[0]);
        }
    }

    T_LIB_EXIT(SCM_29);
}
#endif

#ifdef SCM_30
byte ScmGetBoundaryExt(TScmBoundaryInfo *info_ptr)
{
    byte result = ERR_PARAM;

    if (info_ptr)
    {
        result = ERR_NO;

        info_ptr->preset  = wscm.bandwidth.preset;
        info_ptr->current = wscm.bandwidth.current;
        info_ptr->total   = wscm.bandwidth.total;
        info_ptr->free    = wscm.bandwidth.free;
        info_ptr->packet  = wscm.bandwidth.packet;
        info_ptr->used    = wscm.bandwidth.used;
    }

    return(result);
}
#endif


#ifdef SCM_31
byte ScmDestroySocketExt(byte socket_handle, word *result_list_ptr,
                         TScmListCB *cb_ptr)
{
    bool        go;
    byte        result;
    TMnsResult  cb_res;

    T_API_ENTRY(SCM_31);

    go      = MNS_FALSE;
    result  = ERR_NO;
    cb_res  = NSR_S_OK;

    WAIT4CS();
    if (MNS_FALSE == wscm.api_locked)
    {
        wscm.api_locked         = MNS_TRUE;
        wscm.api_ext            = MNS_TRUE;
        wscm.cb_ptr             = (TMnsStdCB *) cb_ptr;
        wscm.cb_func_id         = FUNCID_INIC_DESTROYSOCKET;
        go = MNS_TRUE;
    }
    REL_CS();

    if (MNS_FALSE != go)
    {
        TMsgTx *msg_ptr;
        msg_ptr = MsgGetTxPtrExt(1);

        if (msg_ptr)
        {
            msg_ptr->Tgt_Adr    = MSG_TGT_INIC;
            msg_ptr->FBlock_ID  = FBLOCK_INIC;
            msg_ptr->Func_ID    = FUNCID_INIC_DESTROYSOCKET;
            msg_ptr->Operation  = OP_STARTRESULT;
            msg_ptr->Data[0]    = socket_handle;

            WAIT4CS();
            wscm.result_list_ptr    = result_list_ptr;
            REL_CS();
            MsgSend3(msg_ptr);  /* handler will release the lock and call cb_ptr */
        }
        else
        {
            result      = ERR_BUFOV;
            cb_res      = NSR_E_BUSY;
            WAIT4CS();
            wscm.api_locked  = MNS_FALSE;
            wscm.api_ext     = MNS_FALSE;
            wscm.cb_ptr      = NULL;
            wscm.cb_func_id  = INIC_SHADOW_INVALID_WORD;
            REL_CS();
        }
    }
    else    /* api was already locked */
    {
        result  = ERR_BUFOV;
        cb_res  = NSR_E_BUSY;
    }

    if ( (ERR_NO != result) && cb_ptr)
    {
        cb_ptr(cb_res, 0, NULL);
    }

    T_API_EXIT(SCM_31);
    return(result);
}
#endif


#ifdef SCM_32
byte ScmConnectSocketsExt( byte input_handle, byte output_handle,
                           bool default_mute,
                           TScmConnectSocketsCB *cb_ptr )
{
    bool        go;
    byte        result;
    TMnsResult  cb_res;

    T_API_ENTRY(SCM_32);

    go = MNS_FALSE;
    result = ERR_NO;
    cb_res = NSR_S_OK;

    WAIT4CS();
    if (MNS_FALSE == wscm.api_locked)
    {
        wscm.api_locked  = MNS_TRUE;
        go = MNS_TRUE;
        wscm.cb_ptr      = (TMnsStdCB *) cb_ptr;
        wscm.cb_func_id  = FUNCID_INIC_CONNECTSOCKETS;
    }
    REL_CS();

    if (MNS_FALSE != go)
    {
        TMsgTx*     msg_ptr;
        msg_ptr = MsgGetTxPtrExt(3);

        if (msg_ptr)
        {
            msg_ptr->Tgt_Adr    = MSG_TGT_INIC;
            msg_ptr->FBlock_ID  = FBLOCK_INIC;
            msg_ptr->Func_ID    = FUNCID_INIC_CONNECTSOCKETS;
            msg_ptr->Operation  = OP_STARTRESULT;
            msg_ptr->Data[0]    = input_handle;
            msg_ptr->Data[1]    = output_handle;
            msg_ptr->Data[2]    = (MNS_FALSE != default_mute) ? (byte)0x01 : (byte)0x00;

            MsgSend3(msg_ptr);  /* handler will release the lock and call cb_ptr */
        }
        else
        {
            result = ERR_BUFOV;
            cb_res = NSR_E_BUSY;
            WAIT4CS();
            wscm.api_locked = MNS_FALSE;
            wscm.cb_ptr     = NULL;
            wscm.cb_func_id = INIC_SHADOW_INVALID_WORD;
            REL_CS();
        }
    }
    else    /* api was already locked */
    {
        result = ERR_BUFOV;
        cb_res = NSR_E_BUSY;
    }

    if ( (ERR_NO != result) && cb_ptr ) /* call cb in all error cases */
    {
        cb_ptr(cb_res, 0);
    }

    T_API_EXIT(SCM_32);
    return(result);
}
#endif




#ifdef SCM_35
byte ScmGetSource(word connection_label, TScmGetSourceCB *cb_ptr)
{
    bool        go;
    byte        result;
    TMnsResult  cb_res;

    T_API_ENTRY(SCM_35);

    go      = MNS_FALSE;
    result  = ERR_NO;
    cb_res  = NSR_S_OK;

    WAIT4CS();
    if (MNS_FALSE == wscm.api_locked)
    {
        wscm.api_locked  = MNS_TRUE;
        go               = MNS_TRUE;
        wscm.cb_ptr      = (TMnsStdCB *) cb_ptr;
        wscm.cb_func_id  = FUNCID_INIC_REMOTEGETSOURCE;
    }
    REL_CS();

    if (MNS_FALSE != go)
    {
        TMsgTx *msg_ptr;
        msg_ptr = MsgGetTxPtrExt(2);

        if (msg_ptr)
        {
            msg_ptr->Tgt_Adr    = MSG_TGT_INIC;
            msg_ptr->FBlock_ID  = FBLOCK_INIC;
            msg_ptr->Func_ID    = FUNCID_INIC_REMOTEGETSOURCE;
            msg_ptr->Operation  = OP_STARTRESULT;
            msg_ptr->Data[0]    = HB(connection_label);
            msg_ptr->Data[1]    = LB(connection_label);

            MsgSend3(msg_ptr);  /* handler will release the lock and call cb_ptr */
        }
        else
        {
            result = ERR_BUFOV;
            cb_res = NSR_E_BUSY;
            WAIT4CS();
            wscm.api_locked = MNS_FALSE;
            wscm.cb_ptr     = NULL;
            wscm.cb_func_id = INIC_SHADOW_INVALID_WORD;
            REL_CS();
        }
    }
    else    /* api was already locked */
    {
        result = ERR_BUFOV;
        cb_res = NSR_E_BUSY;
    }

    if ( (ERR_NO != result) && cb_ptr ) /* call cb in all error cases */
    {
        cb_ptr(cb_res, 0, 0, 0);
    }

    T_API_EXIT(SCM_35);
    return(result);
}
#endif


#ifdef SCM_36
static void ScmAssembleErrResList(TMsgRx *msg_ptr, byte *len_ptr,
                                  word** list_ptr_ptr)
{
    T_MOD_ENTRY(SCM_36);

    ASSERT(msg_ptr);

    *len_ptr      = 0;
    *list_ptr_ptr = NULL;

    if (msg_ptr->Length > 2)
    {
        WAIT4CS();
        *list_ptr_ptr= wscm.result_list_ptr;
        wscm.result_list_ptr = NULL;
        REL_CS();

        if (*list_ptr_ptr)
        {
            byte count = 0;

            *len_ptr = (byte)((word)(msg_ptr->Length -2) >> 1);
            while(count < *len_ptr)
            {
                *list_ptr_ptr[count]  = (word)(msg_ptr->Data[(count << 1)+2] << 8);
                *list_ptr_ptr[count] |= (word)(msg_ptr->Data[(count << 1)+3]);
                count++;
            }
        }
    }

    T_MOD_EXIT(SCM_36);
}
#endif

#ifdef SCM_37
void ScmHandleRemoteGetSourceStatus(TMsgRx *msg_ptr)
{
    TScmGetSourceCB *tmp_cb_ptr;

    T_LIB_ENTRY(SCM_37);

    ASSERT(msg_ptr);

    tmp_cb_ptr = NULL;

    WAIT4CS();
    if (msg_ptr->Func_ID ==  wscm.cb_func_id)
    {
        tmp_cb_ptr = (TScmGetSourceCB *) wscm.cb_ptr;
        wscm.cb_ptr     = NULL;
        wscm.api_locked = MNS_FALSE;
    }
    REL_CS();

    if (tmp_cb_ptr)
    {
        word nod_addr   = 0;
        byte group_addr = 0;
        byte node_pos   = 0;

        if (6 == msg_ptr->Length)
        {
            nod_addr   = (word)(msg_ptr->Data[2] << 8);
            nod_addr  |= (word)(msg_ptr->Data[3]);
            group_addr = msg_ptr->Data[4];
            node_pos   = msg_ptr->Data[5];
        }

        tmp_cb_ptr(NSR_S_OK, nod_addr, group_addr, node_pos);
    }

    T_LIB_EXIT(SCM_37);
}
#endif

#ifdef SCM_38
static void ScmPMCheck(void)
{
    byte mode;
    byte in_socket;
    byte out_socket;
    word most_bandwidth;

    T_MOD_ENTRY(SCM_38);

    WAIT4CS();
    mode           = wscm.cfg_ptr->packet.mode;
    in_socket      = wscm.pm.handle.in;
    out_socket     = wscm.pm.handle.out;
    most_bandwidth = (word) (wscm.bandwidth.packet << 2);
    REL_CS();

    if (SCM_PM_NONE != mode)
    {
        bool failure = MNS_FALSE;

        word mlb_bandwidth = 0;

        switch (wscm.cfg_ptr->packet.clock_config)
        {
        case SCM_PORT_CFG_MLB_256_FS:
            mlb_bandwidth = SCM_MLB_PM_BW_256_FS;
            break;

        case SCM_PORT_CFG_MLB_512_FS:
            mlb_bandwidth = SCM_MLB_PM_BW_512_FS;
            break;

        case SCM_PORT_CFG_MLB_1024_FS:
            mlb_bandwidth = SCM_MLB_PM_BW_1024_FS;
            break;

        case SCM_PORT_CFG_MLB_2048_FS:
            mlb_bandwidth = SCM_MLB_PM_BW_2048_FS;
            break;

        case SCM_PORT_CFG_MLB_3072_FS:
            mlb_bandwidth = SCM_MLB_PM_BW_3072_FS;
            break;

        case SCM_PORT_CFG_MLB_4096_FS:
            mlb_bandwidth = SCM_MLB_PM_BW_4096_FS;
            break;

        case SCM_PORT_CFG_MLB_6144_FS:
            mlb_bandwidth = SCM_MLB_PM_BW_6144_FS;
            break;

        case SCM_PORT_CFG_MLB_8192_FS:
            mlb_bandwidth = SCM_MLB_PM_BW_8192_FS;
            break;

        default:
            FAILED_ASSERT();
            failure = MNS_TRUE;
            break;
        }

        if (MNS_FALSE == failure)
        {
            bool stop_higher_layers = MNS_FALSE;
            word state = SCM_PM_NEW_BORN;
            word statebuf = SCM_PM_NEW_BORN;
            byte current_interface = PmsGetFifoInterface(PMS_M_FIFO_CTRL);

            if (most_bandwidth)
            {
                word in_bandwidth = 0;
                word out_bandwidth = 0;

                word half_mlb_band = mlb_bandwidth >> 3;    /* devided by 8 -> number of quadlets for each socket */
                half_mlb_band <<= 2;                        /* multiplied with 4 -> blockwidth in bytes for each socket */

                WAIT4CS();

                in_bandwidth  =  wscm.cfg_ptr->packet.in.bandwidth;
                out_bandwidth =  wscm.cfg_ptr->packet.out.bandwidth;

                if ((in_bandwidth >= mlb_bandwidth)||(in_bandwidth > SCM_MLB_PM_MAX_BLOCKWIDTH))
                {
                    in_bandwidth = 0;   /* must be reconfigured */
                }
                if ((out_bandwidth >= mlb_bandwidth)||(out_bandwidth > SCM_MLB_PM_MAX_BLOCKWIDTH))
                {
                    out_bandwidth = 0;   /* must be reconfigured */
                }
                if (!((in_bandwidth & 3) == 0)) /* in_bandwidth mod 4 /= 0 */
                {
                    in_bandwidth = 0;  /* must be reconfigured */
                }
                if (!((out_bandwidth & 3) == 0)) /* out_bandwidth mod 4 /= 0 */
                {
                    out_bandwidth = 0;  /* must be reconfigured */
                }

                if ((in_bandwidth==0) && (out_bandwidth==0))
                {
                    in_bandwidth  = half_mlb_band;
                    out_bandwidth = half_mlb_band;
                }
                if (in_bandwidth == 0) /* and out_bandwidth not ! */
                {
                    in_bandwidth = mlb_bandwidth-out_bandwidth;  /* adjust out_bandwidth */
                    in_bandwidth = (in_bandwidth > SCM_MLB_PM_MAX_BLOCKWIDTH) ?
                                   SCM_MLB_PM_MAX_BLOCKWIDTH : in_bandwidth;
                }
                if (out_bandwidth == 0) /* and in_bandwidth not ! */
                {
                    out_bandwidth = mlb_bandwidth-in_bandwidth;  /* adjust in_bandwidth */
                    out_bandwidth = (out_bandwidth > SCM_MLB_PM_MAX_BLOCKWIDTH) ?
                                    SCM_MLB_PM_MAX_BLOCKWIDTH : out_bandwidth;
                }
                /* now both in_bandwidth and out_bandwidth are "valid"  */
                /* adjust the sum: */

                if ((in_bandwidth + out_bandwidth) > mlb_bandwidth)  /* the sum is too big */
                {
                    if ((in_bandwidth > half_mlb_band) && (out_bandwidth > half_mlb_band))
                    {
                        in_bandwidth  = half_mlb_band;
                        out_bandwidth = half_mlb_band;
                    }
                    else if (in_bandwidth > out_bandwidth)
                    {
                        in_bandwidth = mlb_bandwidth - out_bandwidth;
                    }
                    else /* in_bandwidth < out_bandwidth */
                    {
                        out_bandwidth = mlb_bandwidth - in_bandwidth;
                    }
                }

                REL_CS();

                if (MNS_FALSE != MostIsSupported(NSF_MOST_25))
                {
                    if (in_bandwidth > most_bandwidth)
                    {
                        in_bandwidth = most_bandwidth;
                    }
                    if (out_bandwidth > most_bandwidth)
                    {
                        out_bandwidth = most_bandwidth;
                    }
                }


                WAIT4CS();

                if (current_interface != PMS_IFACE_GENERAL )
                {
                     if (!(SCM_PM_WC_MLB_PORT & wscm.pm.state))
                     {
                        /*if "MediaLB port" not open*/
                        if (MNS_FALSE == wscm.pm.mediaLBportOpen)
                        {
                            if (!(SCM_PM_C_MLB_PORT & wscm.pm.state))
                            {
                                /* set Flag to open MediaLB port */
                                wscm.pm.statebuf |= SCM_PM_C_MLB_PORT;
                            }
                        }
                     }
                }

                if (!(SCM_PM_WC_IN_SOCKET & wscm.pm.state))
                {
                    /* if "in" socket exists */
                    if (SCM_HANDLE_INVALID != in_socket)
                    {
                        /* if the bandwidth differs */
                        if (in_bandwidth > wscm.pm.desc.in.blockwidth)
                        {
                            /* schedule destruction and creation of the socket */
                            wscm.pm.statebuf |= (SCM_PM_D_IN_SOCKET | SCM_PM_C_IN_SOCKET);
                        }
                    }
                    else if (!(SCM_PM_C_IN_SOCKET & wscm.pm.state))
                    {
                        /* schedule creation of the socket */
                        wscm.pm.statebuf |= SCM_PM_C_IN_SOCKET;
                    }
                }

                wscm.pm.desc.in.blockwidth = in_bandwidth;

                if(!(SCM_PM_WC_OUT_SOCKET & wscm.pm.state))
                {
                    /* if "out" socket exists */
                    if (SCM_HANDLE_INVALID != out_socket)
                    {
                        /* if the bandwidth differs */
                        if (out_bandwidth > wscm.pm.desc.out.blockwidth)
                        {
                            /* schedule destruction and creation of the socket */
                            wscm.pm.statebuf |= (SCM_PM_D_OUT_SOCKET | SCM_PM_C_OUT_SOCKET);
                        }
                    }
                    else if (!(SCM_PM_C_OUT_SOCKET & wscm.pm.state))
                    {
                        /* schedule creation of the socket */
                        wscm.pm.statebuf |= SCM_PM_C_OUT_SOCKET;
                    }
                }

                wscm.pm.desc.out.blockwidth = out_bandwidth;

                /* Are all messages received? */
                if (SCM_PM_WAIT == (SCM_PM_WAIT & wscm.pm.statebuf))
                {
                    /* write the new states*/
                    wscm.pm.state |= (wscm.pm.statebuf & ~SCM_PM_W_BANDWIDTH);
                    wscm.pm.statebuf = SCM_PM_NEW_BORN;
                }
                REL_CS();
            }

            WAIT4CS();
            state = wscm.pm.state;
            statebuf = wscm.pm.statebuf;
            stop_higher_layers = ((SCM_HANDLE_INVALID == wscm.pm.handle.in)                        ||
                                  (SCM_HANDLE_INVALID == wscm.pm.handle.out)                       ||
                                  ((SCM_PM_D_IN_SOCKET | SCM_PM_D_OUT_SOCKET) &  wscm.pm.statebuf) ||
                                  ((SCM_PM_D_IN_SOCKET | SCM_PM_D_OUT_SOCKET) &  wscm.pm.state)) ?
                                  MNS_TRUE : MNS_FALSE;
            REL_CS();


            if (MNS_FALSE != stop_higher_layers)
            {
                MnsDistribEvent(MNS_P_SRV_WSCM, WSCM_P_SRV_CHECK);
                #ifdef ADS_10
                DataNIStateNetOn(MNS_FALSE);
                #endif
            }


            if (state & SCM_PM_KILL_WAIT_STATE)
            {
                ScmSetPendingEvent(WSCM_P_PM);
            }

            if (statebuf)
            {
                /* set wait-time */
                MostSetTimer(&(wscm.pm.timer), TIME_PM_RECHECK, MNS_FALSE);
            }
            else
            {
                /* reset wait-time */
                MostSetTimer(&(wscm.pm.timer),  (word)0, MNS_FALSE);
            }
        }
    }

    T_MOD_EXIT(SCM_38);
}
#endif

#ifdef SCM_39
static void ScmPMService(void)
{
    word check;
    word count;
    bool reschedule;
    bool set_wait_state;
    bool done;
    byte in_socket;
    byte out_socket;

    T_MOD_ENTRY(SCM_39);

    check          = SCM_PM_NEW_BORN;
    count          = 0;
    reschedule     = MNS_FALSE;
    set_wait_state = MNS_FALSE;
    done           = MNS_FALSE;

    WAIT4CS();
    in_socket = wscm.pm.handle.in;
    out_socket = wscm.pm.handle.out;

    /* get the first bit to work on (not during wait states)*/
    if (wscm.pm.state)
    {
        /* no wait state is set and at least one other state is set  */
        if (!(wscm.pm.state & SCM_PM_KILL_STATE) && (wscm.pm.state & SCM_PM_KILL_WAIT_STATE))
        {
            while (count < 8)
            {
                check = (byte)(1 << count++);

                if (check & wscm.pm.state)
                {
                    wscm.pm.state &= ~check;
                    count = 8;
                }
            }
        }
    }
    else
    {
        done = MNS_TRUE;
    }
    REL_CS();

    /* if there is something todo */
    if (check)
    {
        switch (check)
        {
            case SCM_PM_C_MLB_PORT:
                /* open MediaLB port */
                SCM_PM_RUN(ScmOpenPort(&(wscm.pm.desc.portdesc), ScmPMOpenPortResult));
                break;

            case SCM_PM_D_IN_SOCKET:
                /* destroy "in" socket */
                ASSERT(SCM_HANDLE_INVALID != in_socket);
                SCM_PM_RUN(ScmDestroySocket(in_socket, ScmPMDestroyResult));
                break;

            case SCM_PM_C_IN_SOCKET:
                /* create "in" socket */
                ASSERT(SCM_HANDLE_INVALID == in_socket);
                SCM_PM_RUN(ScmCreateSocket(&(wscm.pm.desc.in), ScmPMCreateResult));
                break;

            case SCM_PM_D_OUT_SOCKET:
                /* destroy "out" socket */
                ASSERT(SCM_HANDLE_INVALID != out_socket);
                SCM_PM_RUN(ScmDestroySocket(out_socket, ScmPMDestroyResult));
                break;

            case SCM_PM_C_OUT_SOCKET:
                /* create "out" socket */
                ASSERT(SCM_HANDLE_INVALID == out_socket);
                SCM_PM_RUN(ScmCreateSocket(&(wscm.pm.desc.out), ScmPMCreateResult));
                break;

            default:
                /* unknown */
                FAILED_ASSERT();
                break;
        }

        WAIT4CS();

        /* set the wait state bit for this task, if we were able to run */
        if (MNS_FALSE != set_wait_state)
        {
            wscm.pm.state |= (word)(check << 8);
        }
        /* ... otherwise we need to reschedule this task */
        else if (MNS_FALSE != reschedule)
        {
            wscm.pm.state |= check;
        }

        /* check if we need to run again */
        check = wscm.pm.state;

        REL_CS();

        if (check & SCM_PM_KILL_WAIT_STATE)
        {
            ScmSetPendingEvent(WSCM_P_PM);
        }
    }
    else if (MNS_FALSE != done)
    {
        PmsSetFifoInterface(PMS_M_FIFO_MDP, PMS_IFACE_GENERAL);
        MnsDistribEvent(MNS_P_SRV_WSCM, WSCM_P_PM_DONE);
    }

    T_MOD_EXIT(SCM_39);
}
#endif

#ifdef SCM_40
static void ScmPMCreateResult(TMnsResult result, byte socket_handle,
                              byte list_len, word *list_ptr)
{
    word state;
    (void) list_len;
    (void) list_ptr;

    T_MOD_ENTRY(SCM_40);

    WAIT4CS();
    state = wscm.pm.state;
    REL_CS();

    if(NSR_SUCCESS(result))
    {
        if (SCM_PM_WC_IN_SOCKET & state)
        {
            WAIT4CS();
            wscm.pm.state &= ~SCM_PM_WC_IN_SOCKET;
            wscm.pm.handle.in = socket_handle;
            REL_CS();

            ScmSetPendingEvent(WSCM_P_PM);
        }
        else if (SCM_PM_WC_OUT_SOCKET & state)
        {
            WAIT4CS();
            wscm.pm.state &= ~SCM_PM_WC_OUT_SOCKET;
            wscm.pm.handle.out = socket_handle;
            REL_CS();

            ScmSetPendingEvent(WSCM_P_PM);
        }
        else
        {
            /* wasn't waiting */
            FAILED_ASSERT();
        }
    }
    else if (NSR_E_ERR_BUSY == result)
    {
        /* INIC is busy, so we have to retry */
        if (SCM_PM_WC_IN_SOCKET & state)
        {
            WAIT4CS();
            wscm.pm.state &= ~SCM_PM_WC_IN_SOCKET;
            wscm.pm.state |= SCM_PM_C_IN_SOCKET;
            REL_CS();

            ScmSetPendingEvent(WSCM_P_PM);
        }
        else if (SCM_PM_WC_OUT_SOCKET & state)
        {
            WAIT4CS();
            wscm.pm.state &= ~SCM_PM_WC_OUT_SOCKET;
            wscm.pm.state |= SCM_PM_C_OUT_SOCKET;
            REL_CS();

            ScmSetPendingEvent(WSCM_P_PM);
        }
        else
        {
            /* wasn't waiting */
            FAILED_ASSERT();
        }
    }
    else if (NSR_E_BUSY != result)
    {
        word statebuf;
        /* no fancy recovery yet */
        FAILED_ASSERT();
        WAIT4CS();
        wscm.pm.state = SCM_PM_NEW_BORN;
        statebuf = wscm.pm.statebuf;
        REL_CS();

        if (SCM_PM_W_BANDWIDTH == (statebuf & SCM_PM_W_BANDWIDTH))
        {
            ScmPMCheck();
        }
    }

    T_MOD_EXIT(SCM_40);
}
#endif

#ifdef SCM_41
static void ScmPMDestroyResult(TMnsResult result)
{
    word state;
    bool resume;

    T_MOD_ENTRY(SCM_41);

    resume = MNS_FALSE;
    WAIT4CS();
    state = wscm.pm.state;
    REL_CS();

    if(NSR_SUCCESS(result))
    {
        if (SCM_PM_WD_IN_SOCKET & state)
        {
            WAIT4CS();
            wscm.pm.state &= ~SCM_PM_WD_IN_SOCKET;
            wscm.pm.handle.in = SCM_HANDLE_INVALID;
            REL_CS();
            resume = MNS_TRUE;
        }
        else if (SCM_PM_WD_OUT_SOCKET & state)
        {
            WAIT4CS();
            wscm.pm.state &= ~SCM_PM_WD_OUT_SOCKET;
            wscm.pm.handle.out = SCM_HANDLE_INVALID;
            REL_CS();
            resume = MNS_TRUE;
        }
        else
        {
            /* wasn't waiting */
            FAILED_ASSERT();
        }

        if (MNS_FALSE != resume)
        {
            ScmSetPendingEvent(WSCM_P_PM);
        }
    }
    else  if (NSR_E_ERR_BUSY == result)
    {
        /* INIC is busy, so we have to retry */
        if (SCM_PM_WD_IN_SOCKET & state)
        {
            WAIT4CS();
            wscm.pm.state &= ~SCM_PM_WD_IN_SOCKET;
            wscm.pm.state |= SCM_PM_D_IN_SOCKET;
            REL_CS();

            ScmSetPendingEvent(WSCM_P_PM);
        }
        else if (SCM_PM_WD_OUT_SOCKET & state)
        {
            WAIT4CS();
            wscm.pm.state &= ~SCM_PM_WD_OUT_SOCKET;
            wscm.pm.state |= SCM_PM_D_OUT_SOCKET;
            REL_CS();

            ScmSetPendingEvent(WSCM_P_PM);
        }
        else
        {
            /* wasn't waiting */
            FAILED_ASSERT();
        }
    }
    else if ((NSR_E_DYS_INVALID_HANDLE == result) ||
             (NSR_E_DYS_INTERNAL == result))
    {
        /* the socket doesn't exist or has been destroyed */
        if (SCM_PM_WD_IN_SOCKET & state)
        {
            WAIT4CS();
            wscm.pm.state &= ~SCM_PM_WD_IN_SOCKET;
            wscm.pm.handle.in = SCM_HANDLE_INVALID;
            REL_CS();
            resume = MNS_TRUE;
        }
        else if (SCM_PM_WD_OUT_SOCKET & state)
        {
            WAIT4CS();
            wscm.pm.state &= ~SCM_PM_WD_OUT_SOCKET;
            wscm.pm.handle.out = SCM_HANDLE_INVALID;
            REL_CS();
            resume = MNS_TRUE;
        }
        else
        {
            /* wasn't waiting */
            FAILED_ASSERT();
        }

        if (MNS_FALSE != resume)
        {
            ScmSetPendingEvent(WSCM_P_PM);
        }
    }
    else if (NSR_E_BUSY != result)
    {
        /* no fancy recovery yet */
        FAILED_ASSERT();
        WAIT4CS();
        wscm.pm.state = SCM_PM_NEW_BORN;
        REL_CS();
    }

    T_MOD_EXIT(SCM_41);
}
#endif

#ifdef SCM_42
bool ScmPMComplete(void)
{
    bool result;

    T_LIB_ENTRY(SCM_42);

    result = MNS_TRUE;

    if (SCM_PM_NONE != wscm.cfg_ptr->packet.mode)
    {
        WAIT4CS();
        if ((SCM_HANDLE_INVALID == wscm.pm.handle.in)     ||
            (SCM_HANDLE_INVALID == wscm.pm.handle.out))
        {
            result = MNS_FALSE;
        }
        REL_CS();
    }

    T_LIB_EXIT(SCM_42);

    return(result);
}
#endif

#ifdef SCM_43
static void ScmPMRecheck(word event)
{
    word state;
    (void) event;

    T_MOD_ENTRY(SCM_43);

    state = SCM_PM_NEW_BORN;

    WAIT4CS();

    /* if any state left over? */
    if (SCM_PM_NEW_BORN != wscm.pm.statebuf)
    {
        wscm.pm.state |= (wscm.pm.statebuf & ~SCM_PM_W_BANDWIDTH);
        wscm.pm.statebuf = SCM_PM_NEW_BORN;
    }

    state = wscm.pm.state;

    REL_CS();

    if (state & SCM_PM_KILL_WAIT_STATE)
    {
        ScmSetPendingEvent(WSCM_P_PM);
    }

    T_MOD_EXIT(SCM_43);
}
#endif

#ifdef SCM_44
void ScmHandleSCDemuteStatus(TMsgRx  *msg_ptr)
{
    T_LIB_ENTRY(SCM_44);

    ASSERT(msg_ptr);

    WAIT4CS();
    if ( wscm.cfg_ptr->sc_demute_fptr)
    {
        wscm.cfg_ptr->sc_demute_fptr(msg_ptr->Data, msg_ptr->Length);
    }
    REL_CS();
    T_LIB_EXIT(SCM_44);
}
#endif

#ifdef SCM_45
static void ScmPMOpenPortResult(TMnsResult result)
{
    word state;

    T_MOD_ENTRY(SCM_45);

    WAIT4CS();
    state = wscm.pm.state;
    REL_CS();

    if(NSR_SUCCESS(result))
    {
        if (SCM_PM_WC_MLB_PORT & state)
        {
            WAIT4CS();
            wscm.pm.state &= ~SCM_PM_WC_MLB_PORT;
            wscm.pm.mediaLBportOpen = MNS_TRUE;
            REL_CS();

            ScmSetPendingEvent(WSCM_P_PM);
        }
        else
        {
            /* wasn't waiting */
            FAILED_ASSERT();
        }
    }
    else if (NSR_E_ERR_BUSY == result)
    {
        /* INIC is busy, so we have to retry */
        if (SCM_PM_WC_MLB_PORT & state)
        {
            WAIT4CS();
            wscm.pm.state &= ~SCM_PM_WC_MLB_PORT;
            wscm.pm.state |= SCM_PM_C_MLB_PORT;
            REL_CS();

            ScmSetPendingEvent(WSCM_P_PM);
        }
        else
        {
            /* wasn't waiting */
            FAILED_ASSERT();
        }
    }
    else if (NSR_E_BUSY != result)
    {
        /* no fancy recovery yet */
        FAILED_ASSERT();

        WAIT4CS();
        wscm.pm.state = SCM_PM_NEW_BORN;
        REL_CS();
    }

    T_MOD_EXIT(SCM_45);
}
#endif


