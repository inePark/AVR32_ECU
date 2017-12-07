/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         FBlock ET 'Enhanced Testability' (FBlockID: 0x0F)
File:           et.c
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

#include "mostns.h"
#include "nbehc.h"
#include "vmsv.h"
#ifdef NETWORKMASTER_SHADOW
#include "netwm_s.h"
#endif

#ifdef NTF_MIN
extern TNtfFBlockL NtfFBlockTab[NTF_MAX_FBLOCKS];
#endif
/*
+------------------------------------------------------------------------------+
| Declaration of FBlock ETs Function Implementations                           |
+------------------------------------------------------------------------------+
*/
#ifdef FUNCID_ET_FBLOCKINFO
    byte ET_FBlockInfo_Get(pTMsgTx Tx_Ptr, pTMsgRx Rx_Ptr);
#endif

#ifdef FUNCID_ET_AUTOWAKEUP
    byte ET_AutoWakeup_Set(pTMsgTx tx_ptr, pTMsgRx rx_ptr);
    byte ET_AutoWakeup_Get(pTMsgTx tx_ptr);
#endif

#ifdef FUNCID_ET_SHUTDOWN
    byte ET_Shutdown(pTMsgTx tx_ptr, pTMsgRx rx_ptr);
#endif

#ifdef FUNCID_ET_SHUTDOWNSUSPENDMODE
    byte ET_SuspMode_Set(pTMsgTx tx_ptr, pTMsgRx rx_ptr);
#endif

#ifdef FUNCID_ET_SENDMESSAGE
    byte ET_SendMessage(pTMsgTx tx_ptr);
#endif

#ifdef FUNCID_ET_ECHOMESSAGE
    byte ET_EchoMessage(pTMsgTx tx_ptr, pTMsgRx rx_ptr);
#endif

#ifdef FUNCID_ET_MESSAGEBUFSIZE
    byte ET_MsgBufSize_Get(pTMsgTx tx_ptr);
#endif

#if (defined FUNCID_ET_MESSAGEBUFSIZE) || (defined FUNCID_ET_SENDMESSAGE)
void ET_CalcMsgBufSize_Rx(word *num, word *length);
#endif

#if (defined FUNCID_ET_MESSAGEBUFSIZE) || (defined FUNCID_ET_SENDMESSAGE)
void ET_CalcMsgBufSize_Tx(word *num, word *length);
#endif

#ifdef FUNCID_ET_CODINGERRORS
    byte ET_CodingErrors_Set(pTMsgTx tx_ptr, pTMsgRx rx_ptr);
    byte ET_CodingErrors_Get(pTMsgTx tx_ptr);
#endif

#ifdef FUNCID_ET_RESET
    byte ET_Reset_Start(void);
#endif

#ifdef FUNCID_ET_CENTRALREGISTRYSIZE
    byte ET_CentralRegistrySize_Get(pTMsgTx tx_ptr);
#endif

#ifdef FUNCID_ET_NOTIFICATIONMATRIXSIZE
    byte ET_NtfMatrixSize_Get(pTMsgTx tx_ptr);
#endif


#ifdef FUNCID_ET_SYSTEMSTATE
    byte ET_SystemState_Get(pTMsgTx tx_ptr);
#endif

#ifdef FUNCID_ET_ECLTRIGGER
    byte ET_ECLTrigger_StartResult(pTMsgTx tx_ptr, pTMsgRx rx_ptr);
#endif

#ifdef FUNCID_ET_ECLINITIATORSTATE
    byte ET_ECLInitiatorState_Get(pTMsgTx tx_ptr, pTMsgRx rx_ptr);
#endif

#ifdef FUNCID_ET_DSIDSOCOUNT
    byte ET_DSIDSOCount_Get(pTMsgTx tx_ptr, pTMsgRx rx_ptr);
#endif

#ifdef FUNCID_ET_DSO
    byte ET_DSO_StartAck(pTMsgTx tx_ptr, pTMsgRx rx_ptr);
#endif

#ifdef FUNCID_ET_DSIHOLD
    byte ET_DSIHold_StartAck(pTMsgTx tx_ptr, pTMsgRx rx_ptr);
#endif

#ifdef FUNCID_ET_VERSION
    byte ET_Version_Get(pTMsgTx tx_ptr);
#endif

#ifdef FUNCID_ET_MOSTREMOTERESET
    byte ET_RReset_Set(pTMsgTx tx_ptr, pTMsgRx rx_ptr);
#endif

#ifdef FUNCID_ET_PHYSICALLAYERTEST
    byte ET_PhysicalLayerTest_Start(pTMsgTx tx_ptr, pTMsgRx rx_ptr);
#endif

#ifdef FUNCID_ET_PHYSICALLAYERRESULT
byte ET_PhysicalLayer_Result(pTMsgTx tx_ptr);
#endif

/*
+------------------------------------------------------------------------------+
| Properties used in the TAB and the C file.                                   |
+------------------------------------------------------------------------------+
*/
#ifdef FUNCID_ET_DIAGRESULT
    byte ET_DiagResult;                           /* holds ET.DiagResult.Status */
#endif

#ifdef FUNCID_ET_SHUTDOWNSUSPENDMODE
    byte ET_SuspMode;                    /* holds ET.ShutdownSuspendMode.Status */
#endif

#include "t_et.tab"                                    /* import the TAB file */

/*
+------------------------------------------------------------------------------+
| Declaration of internal used values (for better readability)                 |
+------------------------------------------------------------------------------+
*/

#define RXFILTERRESULT_TRANSPARENT                                 ((byte) 0x00)

#ifdef FUNCID_ET_AUTOWAKEUP

    #define ET_AWU_PARAM_DELAY                                          ((byte) 0x00)
    #define ET_AWU_PARAM_DIAGNOSIS                                      ((byte) 0x01)
    #define ET_AWU_PARAM_ATTENUATION                                    ((byte) 0x02)
    #define ET_AWU_PARAM_DURATION                                       ((byte) 0x03)
    #define ET_AWU_LENGTH                                              ((word) 0x0004)
    #define ET_AWU_DURATION_INFINITE                                    ((byte) 0x00)

#endif

#ifdef FUNCID_ET_SHUTDOWN

    #define ET_SHUTDOWN_IF_POWERMASTER                               ((byte) 0x00)
    #define ET_SHUTDOWN_SIM_SHUTDOWN_TEMPERATURE                     ((byte) 0x01)
    #define ET_SHUTDOWN_SIM_DEAD_TEMPERATURE                         ((byte) 0x02)

    #define ET_SDN_TYPE                                              ((byte) 0x00)

    #define ET_SDN_SUCCESS                                           ((byte) 0x00)
    #define ET_SDN_UNABLE_TO_PROCESS                                 ((byte) 0x01)
    #define ET_SDN_NOT_POWERMASTER                                   ((byte) 0x02)

#endif

#ifdef FUNCID_ET_SHUTDOWNSUSPENDMODE

    #define ET_SHUTDOWNSUSPENDMODE_OFF                               ((byte) 0x00)
    #define ET_SHUTDOWNSUSPENDMODE_ON                                ((byte) 0x01)
    #define ET_SHUTDOWNSUSPENDMODE_DEFAULT                           ((byte) 0x02)

#endif

#ifdef FUNCID_ET_MESSAGEBUFSIZE

    #define MBS_PARAM_COUNT_RX                                       ((byte) 0x00)
    #define MBS_PARAM_LENGTH_RX                                      ((byte) 0x02)
    #define MBS_PARAM_COUNT_TX                                       ((byte) 0x04) /*  (2*sizeof(word)) */
    #define MBS_PARAM_LENGTH_TX                                      ((byte) 0x06) /*  (3*sizeof(word)) */
    #define MBS_PARAM_SHARED                                         ((byte) 0x08) /*  (4*sizeof(word)) */
    #define MBS_LENGTH                                             ((word) 0x0009) /* ((4*sizeof(word)) + sizeof(byte)) */

    #define MBS_DYN_MEM_ALLOC                                       ((word)0x0000)

    #define MBS_NUM_INIC_BUFS                                           ((word) 4) /* number of INIC msg bufs */
    #define MBS_CTRL_MAX_PAYLOAD                                       ((word) 45)
    #define MBS_LCMSMAX_50                                             ((word) 12)
    #define MBS_LCMSMAX_150                                            ((word) 45)
    #define MBS_NOT_SHARED                                           ((byte) 0x00)
    #define MBS_SHARED                                               ((byte) 0x01)

#endif

#ifdef FUNCID_ET_CODINGERRORS
    #define ET_CE_OFF                                                ((byte) 0x00)
    #define ET_CE_ON                                                 ((byte) 0x01)
    #define ET_CE_RESET                                              ((byte) 0x02)
#endif

#ifdef FUNCID_ET_NOTIFICATIONMATRIXSIZE
#define NTF_PARAM_MIN                                               ((byte) 0x00)
#define NTF_PARAM_MAX                                               ((byte) 0x01)
#define NTF_LENGTH                                                ((word) 0x0002)
#define NTF_OK                                                      ((byte) 0x00)
#endif


#ifdef FUNCID_ET_VERSION
#define ET_VS_PARAM_MAJOR                                             ((byte) 0x00)
#define ET_VS_PARAM_MINOR                                             ((byte) 0x01)
#define ET_VS_PARAM_BUILD                                             ((byte) 0x02)
#define ET_VS_LENGTH                                                ((word) 0x0003)
#endif


#ifdef FUNCID_ET_SYSTEMSTATE
    #define ET_NCS_OK                                              ((byte) 0x00)
    #define ET_NCS_NOTOK                                           ((byte) 0x01)
#endif

#ifdef FUNCID_ET_MOSTREMOTERESET

    #define ET_MRR_PARAM_SETTINGS                                              (0x00)
    #define ET_MRR_RX_LENGTH                                           ((word)0x0001)
    #define ET_MRR_TX_LENGTH                                           ((word)0x0001)

    #define ET_MRR_SETTING_FACTORY                                      ((byte) 0x00)
    #define ET_MRR_SETTING_CURRENT                                      ((byte) 0x01)

#endif


#ifdef FUNCID_ET_PHYSICALLAYERTEST
    #define ET_PLT_RX_LENGTH                                              ((byte) 11)

    #define ET_PLT_TYPE_IDX                                                ((byte) 2)
    #define ET_PLT_LEADIN_IDX                                              ((byte) 3)
    #define ET_PLT_DURATION_IDX                                            ((byte) 5)
    #define ET_PLT_LEADOUT_IDX                                             ((byte) 9)

    #define ET_PLT_DURATION_MIN                                          ((dword) 50)

    #define ET_PLT_STATE_OFF                                               ((byte) 0)
    #define ET_PLT_STATE_LEAD_IN                                           ((byte) 1)
    #define ET_PLT_STATE_TEST                                              ((byte) 2)
    #define ET_PLT_STATE_LEADOUT                                           ((byte) 3)

#endif

#ifdef FUNCID_ET_SHUTDOWNSUSPENDMODE

    #define RXFILTERRESULT_DISPATCHED_NOTX                           ((byte) 0x01)
    #define RXFILTERRESULT_DISPATCHED                                ((byte) 0x02)

    #define NBSHUTDOWN_QUERY                                         ((byte) 0x00)
    #define NBSHUTDOWN_SUSPEND                                       ((byte) 0x01)
    #define NBSHUTDOWN_EXECUTE                                       ((byte) 0x02)
    #define NBSHUTDOWN_TEMPERATURE                                   ((byte) 0x03)

#endif

/*
+------------------------------------------------------------------------------+
| declaration of internal variables                                            |
+------------------------------------------------------------------------------+
*/
#ifdef FUNCID_ET_AUTOWAKEUP

    typedef struct tag_AutoWakeupParams       /* used to schedule an AutoWakeup */
    {
        byte delay;                 /* time to wait after shutdown (Go_Net_Off()) */
        bool diag;                  /* startup using diagnosis? */
        byte duration;              /* wake up active time  */
    } AutoWakeupParams;

    static AutoWakeupParams ET_AutoWakeup_Params;

#endif



#ifdef FUNCID_ET_CODINGERRORS
    static struct TETCodingErrors
    {
        TMsgTx *tx_ptr;
        bool   status;
        word   off_counter;

    } ET_CodingErrors;

    static void ET_CodingErrors_CB(TMnsResult result, word coding_errors);
#endif



/*
+------------------------------------------------------------------------------+
| forward declarations of internal functions                                   |
+------------------------------------------------------------------------------+
*/
#ifdef FUNCID_ET_SHUTDOWNSUSPENDMODE
    static byte ET_SuspMode_Filter(pTMsgTx tx_ptr, pTMsgRx rx_ptr);
#endif




#ifdef FUNCID_ET_AUTOWAKEUP
/*
+------------------------------------------------------------------------------+
| Function    : ET_AutoWakeup_Set()                                            |
| Description : Retrieves the parameters and stores them in the internal       |
|               struct, which is evaluated on NetOff event.                    |
| Parameter(s): ptr at received message and ptr at message to send             |
| Returns     : OpType                                                         |
+------------------------------------------------------------------------------+
*/
byte ET_AutoWakeup_Set(pTMsgTx tx_ptr, pTMsgRx rx_ptr)
{
    byte retval;

    #ifndef CMD_ADD8
    if(ET_AWU_LENGTH != rx_ptr->Length)
    {
        return(CmdErrorMsg(tx_ptr, ERR_LENGTH));              /* return OP_ERROR */
    }
    #endif

    retval = OP_NO_REPORT;

    if ((byte)0 == rx_ptr->Data[ET_AWU_PARAM_DELAY])                                /* AutoWakeup is disabld */
    {
        ET_AutoWakeup_Params.delay    = (byte)0x00;
        ET_AutoWakeup_Params.diag     = MNS_FALSE;
        ET_AutoWakeup_Params.duration = ET_AWU_DURATION_INFINITE;
    }
    #ifndef CAPABILITY_TO_WAKE
    else if ((byte)0 == (rx_ptr->Data[ET_AWU_PARAM_DIAGNOSIS]) )                    /* no diagnosis         */
    {
        retval = CmdErrorParamNotAvailable(tx_ptr,                                  /* Ptr at message to send */
                                           ET_AWU_PARAM_DIAGNOSIS + 1,              /* Position of failed parameter */
                                           &rx_ptr->Data[ET_AWU_PARAM_DIAGNOSIS],   /* Ptr at first Byte of failed parameter */
                                           0x01);                                   /* Number of bytes of failed parameter data type */
    }
    #endif
    else
    {
        ET_AutoWakeup_Params.delay    = rx_ptr->Data[ET_AWU_PARAM_DELAY];
        ET_AutoWakeup_Params.diag     = ((byte)0 != rx_ptr->Data[ET_AWU_PARAM_DIAGNOSIS]) ? MNS_TRUE : MNS_FALSE;
        ET_AutoWakeup_Params.duration = rx_ptr->Data[ET_AWU_PARAM_DURATION];
    }

    return(retval);
}

/*
+------------------------------------------------------------------------------+
| Function    : ET_AutoWakeup_Get()                                            |
| Description : Copies the values from the internal struct into the TX buffer. |
| Parameter(s): ptr at message to send                                         |
| Returns     : OP_STATUS                                                      |
+------------------------------------------------------------------------------+
*/
byte ET_AutoWakeup_Get(pTMsgTx tx_ptr)
{
    tx_ptr->Data[ET_AWU_PARAM_DELAY]       = ET_AutoWakeup_Params.delay;
    tx_ptr->Data[ET_AWU_PARAM_DIAGNOSIS]   = (MNS_FALSE != ET_AutoWakeup_Params.diag) ? (byte)0x01 : (byte)0x00;
    tx_ptr->Data[ET_AWU_PARAM_ATTENUATION] = (byte)0x00;
    tx_ptr->Data[ET_AWU_PARAM_DURATION]    = ET_AutoWakeup_Params.duration;
    tx_ptr->Length = ET_AWU_LENGTH;

    return(OP_STATUS);
}
#endif

/*
+------------------------------------------------------------------------------+
| Function    : ET_Shutdown()                                                  |
| Description : This method is used to trigger different shutdown scenarios.   |
| Parameter(s): ptr at received message and ptr at message to send             |
| Returns     : OpType                                                         |
+------------------------------------------------------------------------------+
*/
#ifdef FUNCID_ET_SHUTDOWN
byte ET_Shutdown(pTMsgTx tx_ptr, pTMsgRx rx_ptr)
{
    byte type;
    byte result;
    byte retval;

    retval = OP_NO_REPORT;

    result = ET_SDN_UNABLE_TO_PROCESS;  /* should not be necessary, but just to be sure */

    #ifndef CMD_ADD8
    if(rx_ptr->Length != (word)1)
    {
        return(CmdErrorMsg(tx_ptr, ERR_LENGTH));              /* return OP_ERROR */
    }
    #endif

    type = rx_ptr->Data[ET_SDN_TYPE];

    switch (type)                                              /* range check */
    {
        case ET_SHUTDOWN_IF_POWERMASTER:
        case ET_SHUTDOWN_SIM_DEAD_TEMPERATURE:
        case ET_SHUTDOWN_SIM_SHUTDOWN_TEMPERATURE:
        {
            result = ET_Shutdown_Request(type);
            break;
        }
        default:
        {
            retval = CmdErrorParamWrong(tx_ptr,                     /* Ptr at message to send */
                                        (byte)0x01,                 /* Position of failed parameter */
                                        &rx_ptr->Data[ET_SDN_TYPE], /* Ptr at first Byte of failed parameter */
                                        (byte)0x01);                /* Number of bytes of failed parameter data type */

            break;
        }
    }

    if (OP_NO_REPORT == retval)
    {
        switch (result)
        {
            case ET_SDN_SUCCESS:                       /* shutdown will be processed */
            {
                break;
            }
            case ET_SDN_UNABLE_TO_PROCESS:                         /* cannot process */
            {
                retval = CmdErrorMsg(tx_ptr, ERR_NOTAVAILABLE);
                break;
            }
            case ET_SDN_NOT_POWERMASTER:                 /* cannot handle type==0x00 */
            {
                if (ET_SHUTDOWN_IF_POWERMASTER == type)
                {
                    retval = CmdErrorParamNotAvailable(tx_ptr, (byte)0x01,
                                                       &rx_ptr->Data[ET_SDN_TYPE], (byte)0x01);
                    break;
                }
                /*lint --e(616) the missing break statement is by design; ERR_DEVICE_MALFUNC ... */
            }
            default:                                   /* unexpected return value */
            {
                retval = CmdErrorMsg(tx_ptr, ERR_DEVICE_MALFUNC);
                break;
            }
        }   /* switch  (result) */
    } /* if (OP_NO_REPORT == retval) */

    return(retval);
}
#endif

/*
+------------------------------------------------------------------------------+
| Function    : ET_SuspMode_Set()                                              |
| Description : If set to "On" the device will respond with a report           |
|               NetBlock.Shutdown.Result(Suspend) to each                      |
|               NetBlock.Shutdown.Start(Query) it receives.                    |
| Parameter(s): ptr at received message and ptr at message to send             |
| Returns     : OpType                                                         |
+------------------------------------------------------------------------------+
*/
#ifdef FUNCID_ET_SHUTDOWNSUSPENDMODE
byte ET_SuspMode_Set(pTMsgTx tx_ptr, pTMsgRx rx_ptr)
{
    byte retval;

    retval = OP_NO_REPORT;                                /* default: no status */

    #ifndef CMD_ADD8
    if(rx_ptr->Length != (word)1)
    {
        return(CmdErrorMsg(tx_ptr, ERR_LENGTH));              /* return OP_ERROR */
    }
    #endif
    switch(rx_ptr->Data[0x00])
    {
        case ET_SHUTDOWNSUSPENDMODE_OFF:
        case ET_SHUTDOWNSUSPENDMODE_ON:
        case ET_SHUTDOWNSUSPENDMODE_DEFAULT:
        {
            ET_SuspMode = rx_ptr->Data[0x00];
            break;
        }
        default:
        {
            /* prepare error message: wrong parameter */
            retval = CmdErrorParamWrong(tx_ptr, (byte)0x01, &rx_ptr->Data[0x00], (byte)0x01);
            break;
        }
    }


    return(retval);
}
#endif

/*
+------------------------------------------------------------------------------+
| Function    : ET_SendMessage()                                               |
| Description : The device will answer to this function call by sending an     |
|               application message via control channel. Length of the         |
|               complete message should be determined by the maximum message   |
|               length the device can use for sending.                         |
| Parameter(s): ptr at message to send                                         |
| Returns     : OP_RESULT                                                      |
+------------------------------------------------------------------------------+
*/
#ifdef FUNCID_ET_SENDMESSAGE
byte ET_SendMessage(pTMsgTx tx_ptr)
{
    word index;
    word max_length;
    word dummy;


    ET_CalcMsgBufSize_Tx(&dummy, &max_length);

    if (MBS_DYN_MEM_ALLOC == max_length)
    {
        if (MNS_FALSE != MostIsSupported(NSF_MOST_150))
        {
            max_length = (MBS_LCMSMAX_150 - (word)1) << 2;
        }
        else if (MNS_FALSE != MostIsSupported(NSF_MOST_50))     /* MOST_50 */
        {
            max_length = (MBS_LCMSMAX_50 - (word)1) << 2;
        }
        else
        {
            return(CmdErrorMsg(tx_ptr, ERR_PROCESSING));
        }
    }

    if (MAX_MSG_TX_DATA < max_length)                           /* avoid overflow of tx_ptr->Data[] */
    {
        max_length = MAX_MSG_TX_DATA;
    }

    for(index = (word)0x00; index < max_length; index++)
    {
        tx_ptr->Data[index] = LB(index);
    }
    tx_ptr->Length = max_length;

    return(OP_RESULT);
}
#endif

/*
+------------------------------------------------------------------------------+
| Function    : ET_EchoMessage()                                               |
| Description : The device will answer to this request by sending back an      |
|               application message via control channel that contains the      |
|               same data as the request.                                      |
| Parameter(s): ptr at received message and ptr at message to send             |
| Returns     : OpType                                                         |
+------------------------------------------------------------------------------+
*/
#ifdef FUNCID_ET_ECHOMESSAGE
byte ET_EchoMessage(pTMsgTx tx_ptr, pTMsgRx rx_ptr)
{
    word source_index;
    word target_index;

    source_index = (word)0x00;
    target_index = (word)0x00;

    if(rx_ptr->Length > MAX_MSG_TX_DATA)
    {
        source_index = (word)(rx_ptr->Length - MAX_MSG_TX_DATA);
    }
    for(; source_index < rx_ptr->Length; source_index++)
    {
        tx_ptr->Data[target_index] = rx_ptr->Data[source_index];
        target_index++;
    }
    tx_ptr->Length = target_index;
    tx_ptr->Operation = OP_RESULT;

    return(tx_ptr->Operation);
}
#endif

/*
+------------------------------------------------------------------------------+
| Function    : ET_MsgBufSize_Get()                                            |
| Description : Read only function to retrieve the number of message buffers   |
|               and their size in bytes from a device.                         |
| Parameter(s): ptr at message to send                                         |
| Returns     : OpType                                                         |
+------------------------------------------------------------------------------+
*/
#ifdef FUNCID_ET_MESSAGEBUFSIZE
byte ET_MsgBufSize_Get(pTMsgTx tx_ptr)
{
    word count_rx;
    word length_rx;
    word count_tx;
    word length_tx;
    byte shared;
    byte retval;


    ET_CalcMsgBufSize_Rx(&count_rx, &length_rx);
    ET_CalcMsgBufSize_Tx(&count_tx, &length_tx);

    shared = ET_SharedRxTxMsgBuf_Query();    /* callback ... shared buffer use? */

    switch (shared)
    {
        case MBS_SHARED:
        case MBS_NOT_SHARED:
        {
            /* prepare tx msg in exact order as described in the FB proposal */
            CmdEncodeWord(&(tx_ptr->Data[MBS_PARAM_COUNT_RX]),  &count_rx);
            CmdEncodeWord(&(tx_ptr->Data[MBS_PARAM_LENGTH_RX]), &length_rx);
            CmdEncodeWord(&(tx_ptr->Data[MBS_PARAM_COUNT_TX]),  &count_tx);
            CmdEncodeWord(&(tx_ptr->Data[MBS_PARAM_LENGTH_TX]), &length_tx);
            tx_ptr->Data[MBS_PARAM_SHARED] = shared;
            tx_ptr->Length = MBS_LENGTH;

            retval = OP_STATUS;
            break;
        }

        default:
        {
            retval = CmdErrorMsg(tx_ptr, ERR_DEVICE_MALFUNC);
            break;
        }
    }

    return(retval);
}
#endif

/*
+------------------------------------------------------------------------------+
| Function    : ET_CalcMsgBufSize_Rx()                                         |
| Description : Calculate number and size of rx messages                       |
| Parameter(s): ptr at result variables                                        |
| Returns     : nothing                                                        |
+------------------------------------------------------------------------------+
*/
#if (defined FUNCID_ET_MESSAGEBUFSIZE) || (defined FUNCID_ET_SENDMESSAGE)
void ET_CalcMsgBufSize_Rx(word *num, word *length)
{
    byte seg_mode;

    *num = MBS_DYN_MEM_ALLOC;

    /* max. length of msgs depends on segmentation mode */
    /* and  use of preallocated buffers                 */
    seg_mode = MsgGetSegMode();

    if (MSG_SEG_INIC_ONLY == seg_mode)
    {
        *length = MBS_CTRL_MAX_PAYLOAD;
    }
    else if (INIC_SHADOW_INVALID_BYTE != seg_mode)
    {
        #if PMS_RX_SEG_PREALLOC_SIZE
        *length = PMS_RX_SEG_PREALLOC_SIZE;
        #else
        *length = MBS_DYN_MEM_ALLOC;
        #endif
    }
    else
    {
        *length = MBS_DYN_MEM_ALLOC;
    }

}
#endif

/*
+------------------------------------------------------------------------------+
| Function    : ET_CalcMsgBufSize_Tx()                                         |
| Description : Calculate number and size of tx messages                       |
| Parameter(s): ptr at result variables                                        |
| Returns     : nothing                                                        |
+------------------------------------------------------------------------------+
*/
#if (defined FUNCID_ET_MESSAGEBUFSIZE) || (defined FUNCID_ET_SENDMESSAGE)
void ET_CalcMsgBufSize_Tx(word *num, word *length)
{
    *num = MBS_DYN_MEM_ALLOC;

    #ifdef AMS_TX_NOSEG
    *length = MBS_CTRL_MAX_PAYLOAD;
    #else
    *length = MBS_DYN_MEM_ALLOC;
    #endif
}
#endif

/*
+------------------------------------------------------------------------------+
| Function    : ET_CodingErrors_Set()                                          |
| Description : The coding-error counting may be turned on, turned off or      |
|               reset.                                                         |
| Parameter(s): ptr at received message and ptr at message to send             |
| Returns     : OP_NO_REPORT or OP_ERROR                                       |
+------------------------------------------------------------------------------+
*/
#ifdef FUNCID_ET_CODINGERRORS
byte ET_CodingErrors_Set(pTMsgTx tx_ptr, pTMsgRx rx_ptr)
{
    byte retval;
    byte ce_mode;
    byte length;
    word timeout;

    retval = OP_NO_REPORT;                                /* default: no status */

    length = (byte) rx_ptr->Length;

    if (length != (byte)5)
    {
        return(CmdErrorMsg(tx_ptr, ERR_LENGTH));              /* return OP_ERROR */
    }

    ce_mode = rx_ptr->Data[0];
    switch(ce_mode)
    {
        case ET_CE_OFF:                          /* Turning Off Coding Error Counting*/
        {
            /* set mode to OFF, that means status MNS_FALSE */
            ET_CodingErrors.status = MNS_FALSE;
            /* request current counter from INIC ... will be stored in off_counter */
            retval = (ERR_NO == MostGetCodingErrors(ET_CodingErrors_CB)) ? OP_NO_REPORT : CMD_RX_REPEAT;
            break;
        }
        case ET_CE_ON:                           /* Turning On Coding Error Counting*/
        {
            byte l1_result = ERR_NO;

            ET_CodingErrors.status      = MNS_TRUE;   /* mode "on" */
            ET_CodingErrors.off_counter = (word)0;
            (void)MostCountCodingErrors((word)0);  /* stop extended mode if running,
                                             return value does not need to be checked
                                             since "stop" always works. */

            CmdDecodeWord(&timeout, &(rx_ptr->Data[3]));

            if (  (timeout < WMCS_CE_TIMEOUT_MIN))
            {
                return(CmdErrorParamWrong(tx_ptr, (byte)3, &(rx_ptr->Data[2]), (byte)2));
            }


            l1_result = MostCountCodingErrors(timeout);
            if(ERR_BUFOV == l1_result)
            {
                /* L1 was busy, we have to retry to get the extended counting */
                return(CMD_RX_REPEAT);
            }

            retval = OP_NO_REPORT;
            break;
        }
        default:
        {
            retval = CmdErrorParamWrong(tx_ptr, (byte)0x01, &(rx_ptr->Data[0]), (byte)0x01);
            break;
        }
    }
    return(retval);
}

/*
+------------------------------------------------------------------------------+
| Function    : ET_CodingErrors_Get()                                          |
| Description : Retrieving the actual Coding Error Counter value.              |
| Parameter(s): ptr at message to send                                         |
| Returns     : OpType                                                         |
+------------------------------------------------------------------------------+
*/
byte ET_CodingErrors_Get(pTMsgTx tx_ptr)
{
    byte retval = CMD_TX_RETAIN;

    if(MNS_FALSE != ET_CodingErrors.status)
    {
        /* ON */

        /* check for busy */
        if(ET_CodingErrors.tx_ptr)
        {
            retval = CmdErrorMsg(tx_ptr, ERR_BUSY);
        }
        else
        {
            /* set ET_CodingErrors.tx_ptr since CB is called synchonously in
               case of an extended measurement */
            ET_CodingErrors.tx_ptr = tx_ptr;

            if(ERR_NO != MostGetCodingErrors(ET_CodingErrors_CB))
            {
                /* we have not used the tx_ptr */
                ET_CodingErrors.tx_ptr = NULL;

                /* we need to retry, since the L1 API is busy */
                retval = CMD_RX_REPEAT;
            }
        }
    }
    else
    {
        /* OFF */

        /* send the stored value of off_counter and reset it */
        dword val = (dword) ET_CodingErrors.off_counter;
        ET_CodingErrors.off_counter = (word)0;

        tx_ptr->Data[0] = (byte)0x00;                   /* OFF */
        CmdEncodeLong(&(tx_ptr->Data[1]), &val);
        tx_ptr->Length = (word)5;

        retval = OP_STATUS;
    }

    return(retval);
}

static void ET_CodingErrors_CB(TMnsResult result, word coding_errors)
{
    if(MNS_FALSE != ET_CodingErrors.status)
    {
        /* ON */

        if((ET_CodingErrors.tx_ptr) && (NSR_E_BUSY != result))
        {
            /* recover tx_ptr and send status message */
            dword   val    = (dword) coding_errors;
            TMsgTx *tx_ptr = ET_CodingErrors.tx_ptr;
            ET_CodingErrors.tx_ptr = NULL;

            if(NSR_S_CE_EXT_RESULT == result)
            {
                ET_CodingErrors.status = MNS_FALSE; /* auto switch OFF */
            }

            tx_ptr->Data[0] = (MNS_FALSE != ET_CodingErrors.status) ? (byte)0x01 : (byte)0x00;
            CmdEncodeLong(&(tx_ptr->Data[1]), &val);
            tx_ptr->Length = (word)5;
            tx_ptr->Operation = OP_STATUS;

            AmsMsgSend(tx_ptr);
        }
    }
    else
    {
        /* OFF */

        /* just store the off_counter for use in .Get */
        ET_CodingErrors.off_counter = coding_errors;
    }
}
#endif


/*
+------------------------------------------------------------------------------+
| Function    : ET_Reset_Start()                                               |
| Description : This function triggers the device to perform a reset.          |
| Parameter(s): None                                                           |
| Returns     : OP_NO_REPORT                                                   |
+------------------------------------------------------------------------------+
*/
#ifdef FUNCID_ET_RESET
byte ET_Reset_Start(void)
{
    ET_Reset_Request();
    return(OP_NO_REPORT);
}

#endif


/*
+------------------------------------------------------------------------------+
| Function    : ET_CentralRegistrySize_Get()                                   |
| Description : The Central Registry Size is obtained by way of the callback   |
|               function ET_CentralRegistrySize_Query() and are put into the   |
|               TX buffer.                                                     |
| Parameter(s): ptr at message to send                                         |
| Returns     : OP_STATUS                                                      |
+------------------------------------------------------------------------------+
*/
#ifdef FUNCID_ET_CENTRALREGISTRYSIZE
byte ET_CentralRegistrySize_Get(pTMsgTx tx_ptr)
{
    word result;
    byte retval;

    result = ET_CentralRegistrySize_Query();
    CmdEncodeWord(&(tx_ptr->Data[0]), &result);
    tx_ptr->Length = (word)2;
    retval = OP_STATUS;

    return(retval);
}
#endif

/*
+------------------------------------------------------------------------------+
| Function    : ET_NtfMatrixSize_Get()                                         |
| Description : Determining the minimal and maximal values of the Ntf Matrixs  |
|               in terms of the row numbers.                                   |
| Parameter(s): ptr at message to send                                         |
| Returns     : OpType                                                         |
+------------------------------------------------------------------------------+
*/
#ifdef FUNCID_ET_NOTIFICATIONMATRIXSIZE
byte ET_NtfMatrixSize_Get(pTMsgTx tx_ptr)
{
    byte retval;

    #ifdef NTF_MIN
    TNtfFBlockL *pntf_tab_entry;
    byte temp_val;
    byte i;
    byte min_val;
    byte max_val;
    #else
    TNtfMatrixSize ntf_matrix_sizes;
    byte result;
    #endif

    #ifdef NTF_MIN
    pntf_tab_entry = &NtfFBlockTab[0];
    max_val = pntf_tab_entry->NumDev;
    min_val = max_val;
    for (i=(byte)0; i<NTF_MAX_FBLOCKS; i++)
    {
        temp_val = pntf_tab_entry->NumDev;
        if(temp_val > max_val)
        {
            max_val = temp_val;
        }
        if(temp_val < min_val)
        {
            min_val = temp_val;
        }
        pntf_tab_entry++;
    }

    tx_ptr->Data[NTF_PARAM_MIN] = min_val;
    tx_ptr->Data[NTF_PARAM_MAX] = max_val;
    tx_ptr->Length = NTF_LENGTH;
    retval = OP_STATUS;
    #else
    result = ET_NtfMatrixSize_Query(&ntf_matrix_sizes);
    if (NTF_OK == result)
    {
        tx_ptr->Data[NTF_PARAM_MIN] = ntf_matrix_sizes.MinSize;
        tx_ptr->Data[NTF_PARAM_MAX] = ntf_matrix_sizes.MaxSize;
        tx_ptr->Length = NTF_LENGTH;
        retval = OP_STATUS;
    }
    else
    {
        retval = CmdErrorMsg(tx_ptr, ERR_DEVICE_MALFUNC);
    }
    #endif

    return(retval);
}
#endif





/*
+------------------------------------------------------------------------------+
| Function    : ET_SystemState_Get()                                           |
| Description : reports the SystemState and the NWM logical address            |
|                                       to start as slave                      |
| Parameter(s): ptr at message to send                                         |
| Returns     : OpType                                                         |
+------------------------------------------------------------------------------+
*/
#ifdef FUNCID_ET_SYSTEMSTATE
byte ET_SystemState_Get(pTMsgTx tx_ptr)
{

    #if (defined NETWORKMASTER_LOCAL) || (defined NETWORKMASTER_SHADOW)
    word nwm_address;


    #ifdef NETWORKMASTER_LOCAL
    nwm_address = MostGetNodeAdr();         /* own address */
    if (INIC_SHADOW_INVALID_WORD == nwm_address)
    {
        nwm_address = (word)0x0000;               /* same behaviour as NmGetNWMAddr */
    }
    #else
    nwm_address = NmGetNWMAddr();           /* determined by module NETWM_S.C */
    #endif

    tx_ptr->Data[0] = (NCS_OK == MostGetNCState()) ? ET_NCS_OK : ET_NCS_NOTOK;
    CmdEncodeWord(&(tx_ptr->Data[1]), &nwm_address);
    tx_ptr->Length = (word)3;

    return(OP_STATUS);

    #else
    ET_SystemState_Request(tx_ptr->Tgt_Adr);
    return(OP_NO_REPORT);
    #endif

}
#endif


/*
+------------------------------------------------------------------------------+
| Function    : ET_ECLTrigger_StartResult()                                    |
| Description : triggers the ECL                                               |
|                                                                              |
| Parameter(s): ptr at received message                                        |
| Returns     : OpType                                                         |
+------------------------------------------------------------------------------+
*/
#ifdef FUNCID_ET_ECLTRIGGER
byte ET_ECLTrigger_StartResult(pTMsgTx tx_ptr, pTMsgRx rx_ptr)
{
    byte  retval;
    word  tgt_addr;
    byte  ecl_action;
    word  length;
    byte* ecl_data;

    retval = OP_NO_REPORT;

    #ifndef CMD_ADD8
    if(rx_ptr->Length < (word)1)
    {
        retval = CmdErrorMsg(tx_ptr, ERR_LENGTH);       /* return OP_ERROR */
    }
    else
    #endif
    {

        tgt_addr    = rx_ptr->Src_Adr;
        ecl_action  = rx_ptr->Data[0];
        length      = rx_ptr->Length;

        if ((word)1 < length)                           /* additional data? */
        {
            ecl_data = &(rx_ptr->Data[1]);
        }
        else
        {
            ecl_data = NULL;
        }

        --length;

        ET_ECLTrigger_Request(tgt_addr, ecl_action, length, ecl_data);
    }

    return(retval);
}
#endif

/*
+------------------------------------------------------------------------------+
| Function    : ET_ECLInitiatorState_Get()                                     |
| Description : reports the last state of the ECL                              |
|                                                                              |
| Parameter(s): ptr at received message                                        |
|               ptr at message to send                                         |
| Returns     : OpType                                                         |
+------------------------------------------------------------------------------+
*/
#ifdef FUNCID_ET_ECLINITIATORSTATE
byte ET_ECLInitiatorState_Get(pTMsgTx tx_ptr, pTMsgRx rx_ptr)
{
    byte retval;
    word node_class;
    byte comm_state;
    byte alive;
    byte signal;


    #ifndef CMD_ADD8
    if(rx_ptr->Length < (word)2)
    {
        retval = CmdErrorMsg(tx_ptr, ERR_LENGTH);       /* return OP_ERROR */
    }
    else
    #endif
    {

        CmdDecodeWord(&node_class, &(rx_ptr->Data[0]));
        comm_state = (byte)0x00;
        alive      = (byte)0x00;
        signal     = (byte)0x00;

        ET_ECLInitiatorState_Query(node_class, &comm_state, &alive, &signal);

        tx_ptr->Data[0] = comm_state;
        tx_ptr->Data[1] = alive;
        tx_ptr->Data[2] = signal;
        tx_ptr->Length  = (word)3;

        retval = OP_STATUS;
    }
    return(retval);
}
#endif




#ifdef FUNCID_ET_DSIDSOCOUNT
/*
+------------------------------------------------------------------------------+
| Function    : ET_DSIDSOCount_Get()                                           |
| Description : reports the number of possible MHP connections                 |
|                                                                              |
| Parameter(s): ptr at received message                                        |
|               ptr at message to send                                         |
| Returns     : OpType                                                         |
+------------------------------------------------------------------------------+
*/
byte ET_DSIDSOCount_Get(pTMsgTx tx_ptr, pTMsgRx rx_ptr)
{
    byte retval;

    #ifndef CMD_ADD8
        if(rx_ptr->Length != (word)0)
        {
            retval = CmdErrorMsg(tx_ptr, ERR_LENGTH);       /* return OP_ERROR */
        }
    #else
        (void)rx_ptr;
        retval = OP_STATUS;
    #endif

    #ifdef PACKET_ON_SECOND_HOST
        retval = ET_DSIDSOCount_Request(rx_ptr, tx_ptr);                   /* signal the application
                                                                             to fill in the expected values */
    #else
        #if (defined MHP_RX_MIN) && (defined MAX_MHP_RX)
            tx_ptr->Data[0] = MAX_MHP_RX;                     /* data[0] contains DSICount */
        #else
            tx_ptr->Data[0] = (byte)0;
        #endif

        #if ((defined MHP_TX_MIN) || (defined PMHT_MIN)) && (defined MAX_MHP_TX)
            tx_ptr->Data[1] = (byte)MAX_MHP_TX;                      /* data[1] contains DSOCount */
        #else
            tx_ptr->Data[1] = (byte)0;
        #endif
        tx_ptr->Length = (word)2;
        retval         = OP_STATUS;
    #endif
    return retval;
}
#endif


/*
+------------------------------------------------------------------------------+
| Function    : ET_DSO_StartAck()                                              |
| Description : triggers the application to open an MHP connection             |
|                                                                              |
| Parameter(s): ptr at received message                                        |
|               ptr at message to send                                         |
| Returns     : OP_ERROR or OP_NO_REPORT                                       |
+------------------------------------------------------------------------------+
*/
#ifdef FUNCID_ET_DSO
byte ET_DSO_StartAck(pTMsgTx tx_ptr, pTMsgRx rx_ptr)
{
    word num_packets;
    byte next_packet_method;
    byte ack_mode;
    word tgt_addr;
    byte inst_id;
    word fkt_id;
    byte op_type;
    byte ret_val;

    #ifndef CMD_ADD8
    if(rx_ptr->Length != (word)11)
    {
        return(CmdErrorMsg(tx_ptr, ERR_LENGTH));                /* return OP_ERROR */
    }
    #endif

    tgt_addr           = rx_ptr->Src_Adr;                       /* build MHP connection to sender of request */
    CmdDecodeWord(&num_packets, &rx_ptr->Data[2]);
    next_packet_method = rx_ptr->Data[4];
    ack_mode           = rx_ptr->Data[5];
    inst_id            = rx_ptr->Data[7];
    CmdDecodeWord(&fkt_id, &rx_ptr->Data[8]);
    op_type            = rx_ptr->Data[10];

    if ((byte)0x02 < next_packet_method)
    {
        ret_val = CmdErrorParamWrong(tx_ptr, (byte)3, &(rx_ptr->Data[4]), (byte)1);
    }
    else if ((byte)0x01 < ack_mode)
    {
        ret_val = CmdErrorParamWrong(tx_ptr, (byte)4, &(rx_ptr->Data[5]), (byte)1);
    }
    else if (FBLOCK_ET != rx_ptr->Data[6])                                               /* FBlockID == ET ? */
    {
        ret_val = CmdErrorParamWrong(tx_ptr, (byte)5, &(rx_ptr->Data[6]), (byte)1);
    }
    else if (((word)0x400 > fkt_id) || ((word)0x4FF < fkt_id))                      /* FktID  0x400 .. 0x4FF allowed*/
    {
        ret_val = CmdErrorParamWrong(tx_ptr, (byte)7, &(rx_ptr->Data[8]), (byte)2);
    }
    else if ((byte)8 < op_type)                                                     /* OpType 0..8  allowed*/
    {
        ret_val = CmdErrorParamWrong(tx_ptr, (byte)8, &(rx_ptr->Data[10]), (byte)1);
    }
    else
    {
        switch (ET_DSO_Request(num_packets, next_packet_method, ack_mode, tgt_addr, FBLOCK_ET, inst_id, fkt_id, op_type))
        {
            case ET_DSO_OK:
                ret_val = OP_NO_REPORT;
                break;

            case ET_DSO_BUSY:
                ret_val = CmdErrorMsg(tx_ptr, ERR_BUSY);
                break;

            case ET_DSO_NO_MEMORY:
                ret_val = CmdErrorMsg(tx_ptr, ERR_DEVICE_MALFUNC);
                break;

            default:
                ret_val = CmdErrorMsg(tx_ptr, ERR_PROCESSING);
                break;
        }
    }

    return(ret_val);
}
#endif

/*
+------------------------------------------------------------------------------+
| Function    : ET_DSIHold_StartAck()                                          |
| Description : set certain DSI connections to hold                            |
| Parameter(s): ptr at received message                                        |
|               ptr at message to send                                         |
| Returns     : OP_ERROR or OP_NO_REPORT                                       |
+------------------------------------------------------------------------------+
*/
#ifdef FUNCID_ET_DSIHOLD
byte ET_DSIHold_StartAck(pTMsgTx tx_ptr, pTMsgRx rx_ptr)
{
    byte hold;
    byte dsi_idx;

    #ifndef CMD_ADD8
    if(rx_ptr->Length != (word)4)
    {
        return(CmdErrorMsg(tx_ptr, ERR_LENGTH));              /* return OP_ERROR */
    }
    #else
    (void)tx_ptr;
    #endif

    hold    = rx_ptr->Data[2];
    dsi_idx = rx_ptr->Data[3];

    ET_DSIHold_Request(hold, dsi_idx);

    return(OP_NO_REPORT);
}
#endif


/*
+------------------------------------------------------------------------------+
| Function    : ET_Version_Get()                                               |
| Description : Get the version number of FBlock ET Function Catalog           |
|                                                                              |
| Parameter(s): ptr at message to send                                         |
| Returns     : OP_STATUS                                                      |
+------------------------------------------------------------------------------+
*/
#ifdef FUNCID_ET_VERSION
byte ET_Version_Get(pTMsgTx tx_ptr)
{
    byte retval;
    tx_ptr->Data[ET_VS_PARAM_MAJOR] = ET_VERSION_MAJOR;
    tx_ptr->Data[ET_VS_PARAM_MINOR] = ET_VERSION_MINOR;
    tx_ptr->Data[ET_VS_PARAM_BUILD] = ET_VERSION_BUILD;
    tx_ptr->Length = ET_VS_LENGTH;
    retval = OP_STATUS;

    return(retval);
}
#endif

/*
+------------------------------------------------------------------------------+
| Function    : ET_RReset_Set()                                                |
| Description : The function function enables the initialization respectively  |
|               re-initialization of all existing MOST applications in the     |
|               system.                                                        |
| Parameter(s): ptr at received message and ptr at message to send             |
| Returns     : OpType                                                         |
+------------------------------------------------------------------------------+
*/
#ifdef FUNCID_ET_MOSTREMOTERESET
byte ET_RReset_Set(pTMsgTx tx_ptr, pTMsgRx rx_ptr)
{
    byte settings;
    byte retval;

    #ifndef CMD_ADD8
    if(ET_MRR_RX_LENGTH != rx_ptr->Length)                     /* param check */
    {
        retval = CmdErrorMsg(tx_ptr, ERR_LENGTH);              /* return OP_ERROR */
    }
    else
    #endif
    {
        settings = rx_ptr->Data[ET_MRR_PARAM_SETTINGS];        /* range check */
        switch (settings)
        {
            case ET_MRR_SETTING_FACTORY:
            case ET_MRR_SETTING_CURRENT:
            {
                ET_MOSTRemoteReset_Request(rx_ptr->Src_Adr, settings);
                tx_ptr->Data[0] = settings;
                tx_ptr->Length  = ET_MRR_TX_LENGTH;
                retval = OP_STATUS;
                break;
            }

            default:
            {
                retval = CmdErrorParamWrong( tx_ptr, (byte)0x01, &rx_ptr->Data[0x00], (byte)0x01);
                break;
            }
        }
    }

    return(retval);
}
#endif

/*
+------------------------------------------------------------------------------+
| Function    : ET_PhysicalLayerTest_Start()                                   |
| Description : Start Physical Layer Test                                      |
| Parameter(s): ptr at received message and ptr at message to send             |
| Returns     : OP_RESULTACK or OP_ERROR_ACK                                   |
+------------------------------------------------------------------------------+
*/
#ifdef FUNCID_ET_PHYSICALLAYERTEST
byte ET_PhysicalLayerTest_Start(pTMsgTx tx_ptr, pTMsgRx rx_ptr)
{
    byte retval;

    byte  type;
    word  lead_in;
    word  lead_out;
    dword duration;

    retval = OP_RESULT;

    if(MNS_FALSE != MostIsSupported(NSF_MOST_150))
    {
        #ifndef CMD_ADD8
        if(ET_PLT_RX_LENGTH != rx_ptr->Length)                     /* param check */
        {
            retval = CmdErrorMsg(tx_ptr, ERR_LENGTH);              /* return OP_ERROR */
        }
        else
        #endif
        {
            type  = rx_ptr->Data[ET_PLT_TYPE_IDX];
            CmdDecodeWord(&lead_in,  &rx_ptr->Data[ET_PLT_LEADIN_IDX]);
            CmdDecodeLong(&duration, &rx_ptr->Data[ET_PLT_DURATION_IDX]);
            CmdDecodeWord(&lead_out, &rx_ptr->Data[ET_PLT_LEADOUT_IDX]);

            /* range check */
            if (0x02 < type)
            {
                retval = CmdErrorParamWrong( tx_ptr, (byte)0x01, &rx_ptr->Data[ET_PLT_TYPE_IDX], (byte)sizeof(type));
            }
            else if (ET_PLT_DURATION_MIN > duration)
            {
                retval = CmdErrorParamWrong( tx_ptr, (byte)0x03, &rx_ptr->Data[ET_PLT_DURATION_IDX], (byte)sizeof(duration));
            }

            if (OP_RESULT == retval)
            {
                /* start state machine in VMSV and send ResultAck message from VMSV */
                if (ERR_NO == VmsvPhysicalLayerTestStart(rx_ptr, type, lead_in, duration, lead_out))
                {
                    ET_PhysicalLayerTest_Status(MNS_TRUE);  /* inform application about start of PhysicalLayerTest */
                    retval = OP_NO_REPORT;
                }
                else
                {
                    retval = CmdErrorMsg(tx_ptr, ERR_PROCESSING);
                }
            }
        }
    }
    else
    {
        retval = CmdErrorMsg(tx_ptr, ERR_FKTID);   /* FuncID not found */
    }

    return(retval);
}
#endif

/*
+------------------------------------------------------------------------------+
| Function    : ET_PhysicalLayerTest_Finished()                                |
| Description : Report result of  Physical Layer Test                          |
| Parameter(s): none                                                           |
| Returns     : nothing                                                        |
+------------------------------------------------------------------------------+
*/
#ifdef FUNCID_ET_PHYSICALLAYERTEST
void ET_PhysicalLayerTest_Finished(void)
{
    ET_PhysicalLayerTest_Status(MNS_FALSE);  /* inform application about end of PhysicalLayerTest */
}
#endif

/*
+------------------------------------------------------------------------------+
| Function    : ET_PhysicalLayer_Result()                                      |
| Description : Report result of  Physical Layer Test                          |
| Parameter(s): ptr at message to send                                         |
| Returns     : OpType                                                         |
+------------------------------------------------------------------------------+
*/
#ifdef FUNCID_ET_PHYSICALLAYERRESULT
byte ET_PhysicalLayer_Result(pTMsgTx tx_ptr)
{
    byte  retval;
    dword num_errors;
    bool  lock_status;
    bool  valid;


    retval     = OP_STATUS;

    if(MNS_FALSE != MostIsSupported(NSF_MOST_150))
    {
        num_errors = (dword)0;
        valid = VmsvPhysicalLayerTestResult(&lock_status, &num_errors);

        if (MNS_FALSE == valid)
        {
            retval = CmdErrorMsg(tx_ptr, ERR_NOTAVAILABLE);
        }
        else
        {
            tx_ptr->Data[0] = (MNS_FALSE != lock_status) ? (byte)0x01 : (byte)0x00;
            CmdEncodeLong(&tx_ptr->Data[1], &num_errors);
            tx_ptr->Length = (word)5;
        }
    }
    else
    {
        retval = CmdErrorMsg(tx_ptr, ERR_FKTID);   /* FuncID not found */
    }

    return(retval);
}
#endif



/*
+------------------------------------------------------------------------------+
| Function    : ET_Init()                                                      |
| Description : Initialize  internal variables.                                |
| Parameter(s): -                                                              |
| Returns     : -                                                              |
+------------------------------------------------------------------------------+
*/
#ifdef ET_MIN
void ET_Init(void)
{
    #ifdef FUNCID_ET_SHUTDOWNSUSPENDMODE
    ET_SuspMode                             = ET_SHUTDOWNSUSPENDMODE_DEFAULT;
    #endif
    #ifdef FUNCID_ET_AUTOWAKEUP
    ET_AutoWakeup_Params.delay              = (byte)0x00;
    ET_AutoWakeup_Params.diag               = MNS_FALSE;
    ET_AutoWakeup_Params.duration           = ET_AWU_DURATION_INFINITE;
    #endif

    #ifdef FUNCID_ET_CODINGERRORS
    ET_CodingErrors.tx_ptr      = NULL;
    ET_CodingErrors.status      = MNS_FALSE;
    ET_CodingErrors.off_counter = (word)0;
    #endif

    #ifdef FUNCID_ET_DIAGRESULT
    if (MNS_FALSE == MostIsSupported(NSF_RBD))
    {
         ET_DiagResult = DIAG_OK;
    }
    #endif

}

/*
+------------------------------------------------------------------------------+
| Function    : ET_Go_Net_Off()                                                |
| Description : Called by the application if the device goes NetOff.           |
| Parameter(s): -                                                              |
| Returns     : -                                                              |
+------------------------------------------------------------------------------+
*/
void ET_Go_Net_Off(void)
{
    #ifdef FUNCID_ET_AUTOWAKEUP
    if (ET_AutoWakeup_Params.delay > (byte)0x00)
    {
        ET_AutoWakeup_Request( ET_AutoWakeup_Params.delay,
                               ET_AutoWakeup_Params.diag,
                               ET_AutoWakeup_Params.duration );
    }
    ET_AutoWakeup_Params.delay    = (byte)0x00;
    ET_AutoWakeup_Params.diag     = MNS_FALSE;
    ET_AutoWakeup_Params.duration = ET_AWU_DURATION_INFINITE;
    #endif
}
#endif

/*
+------------------------------------------------------------------------------+
| Function    : ET_RxFilter()                                                  |
| Description : Hook into CmdRxFilter()                                        |
| Parameter(s): ptr at received message and ptr at message to send             |
| Returns     : see RxFilterResult                                             |
+------------------------------------------------------------------------------+
*/
#ifdef ET_MIN
byte ET_RxFilter(pTMsgTx tx_ptr, pTMsgRx rx_ptr)
{
    byte retval;

    if (    (FBLOCK_ET == rx_ptr->FBlock_ID)
         && (OP_REPORTS > rx_ptr->Operation))
    {
        rx_ptr->Inst_ID = MOST_GET_NODE_POS();
    }

    retval = RXFILTERRESULT_TRANSPARENT;

    #ifdef FUNCID_ET_SHUTDOWNSUSPENDMODE
    if (FBLOCK_NETBLOCK == rx_ptr->FBlock_ID)
    {
        /* if ShutdownSuspendMode is not Default this FBlock has to intercept     */
        /* NetBlock.Shutdown.Start(Query) messages and handle them specially.     */
        if((ET_SHUTDOWNSUSPENDMODE_DEFAULT != ET_SuspMode)          &&
           (FUNC_SHUTDOWN                  == rx_ptr->Func_ID)      &&
           (OP_START                       == rx_ptr->Operation)    &&
           (NBSHUTDOWN_QUERY               == rx_ptr->Data[0x00]))
        {
            retval = ET_SuspMode_Filter(tx_ptr, rx_ptr);
        }
    }
    #else
        (void) tx_ptr;
    #endif

    return(retval);
}
#endif

/*
+------------------------------------------------------------------------------+
| Function    : ET_SuspMode_Filter()                                           |
| Description : React on NetBlock.Shutdown.Start(Query) messages according     |
|               to the FBlock's ShutdownSuspendMode                            |
| Parameter(s): ptr at received message and ptr at message to send             |
| Returns     : see RxFilterResult                                             |
+------------------------------------------------------------------------------+
*/
#ifdef FUNCID_ET_SHUTDOWNSUSPENDMODE
static byte ET_SuspMode_Filter(pTMsgTx tx_ptr, pTMsgRx rx_ptr)
{
    byte result;

    /* this function is only called if a NetBlock.Shutdown.Start(Query) was re- */
    /* ceived and ShutdownSuspendMode is not Default ... see ET_RxFilter()      */

    result = RXFILTERRESULT_DISPATCHED_NOTX;                  /* send no result */
    if(ET_SHUTDOWNSUSPENDMODE_ON == ET_SuspMode)
    {
        tx_ptr->Tgt_Adr    = rx_ptr->Src_Adr;
        tx_ptr->FBlock_ID  = rx_ptr->FBlock_ID;
        tx_ptr->Inst_ID    = rx_ptr->Inst_ID;
        tx_ptr->Func_ID    = rx_ptr->Func_ID;
        tx_ptr->Operation  = OP_RESULT;
        tx_ptr->Data[0x00] = NBSHUTDOWN_SUSPEND;
        tx_ptr->Length     = (word)0x01;
        result             = RXFILTERRESULT_DISPATCHED;            /* send result */
    }
    return(result);
}
#endif

/*
+------------------------------------------------------------------------------+
| Function    : ET_Store_Diag_Result()                                         |
| Description : Called by the application when msval_diag_result from Layer I  |
|               was received.                                                  |
| Parameter(s): result of the diagnosis                                        |
| Returns     : -                                                              |
+------------------------------------------------------------------------------+
*/
#ifdef FUNCID_ET_DIAGRESULT
void ET_Store_Diag_Result(byte diag_result, byte *info)
{
    ET_DiagResult = (byte)0xFF;

    switch (diag_result)
    {
        case MSVAL_DIAG_OK:
            ET_DiagResult = DIAG_OK;
            break;

        case MSVAL_DIAG_POS:
            ET_DiagResult = *(info+1);
            break;

        case MSVAL_DIAG_FAILED:
            ET_DiagResult = DIAG_POOR;
            break;

        case MSVAL_DIAG_SIGNAL_NO_LOCK:
            ET_DiagResult = DIAG_POS0_SIGNAL;
            break;

        case 0xFF:
            /* default at startup : do nothing */
            break;

        default:
            break;
    }
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : ET_FBlockInfo_Get()                                        */
/* Description : Handle FBlockInfo Get Message                              */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OpType                                                     */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef FUNCID_ET_FBLOCKINFO
byte ET_FBlockInfo_Get(pTMsgTx Tx_Ptr, pTMsgRx Rx_Ptr)
{
    word rx_id;
    byte name[] = "FBlock EnhancedTestability";
    byte i;
    byte version[3];
    byte return_value;
    bool answer_prepared;

    #ifndef CMD_ADD8
    if (Rx_Ptr->Length != (word)2)                  /* parameter ID is required */
    {
        return( CmdErrorMsg(Tx_Ptr, ERR_LENGTH) );  /* return(OP_ERROR) */
    }
    #endif

    rx_id = (word)((word)Rx_Ptr->Data[0] << 8);     /* Data[0] contains HB of ID */
    rx_id += Rx_Ptr->Data[1];                       /* Data[1] contains LB of ID */

    Tx_Ptr->Data[0] = Rx_Ptr->Data[0];              /* prepare TX message */
    Tx_Ptr->Data[1] = Rx_Ptr->Data[1];
    Tx_Ptr->Length  = (word)2;

    answer_prepared = MNS_TRUE;

    if (rx_id < 0x1000)                             /* FktID ?*/
    {
        Tx_Ptr->Length  = (word)3;

        switch (rx_id)
        {

            case FUNCID_ET_FBLOCKINFO:
            case FUNCID_ET_AUTOWAKEUP:
            case FUNCID_ET_SHUTDOWN:
            case FUNCID_ET_SHUTDOWNSUSPENDMODE:
            case FUNCID_ET_MESSAGEBUFSIZE:
            case FUNCID_ET_RESET:
          #ifdef FUNCID_ET_CENTRALREGISTRYSIZE
            case FUNCID_ET_CENTRALREGISTRYSIZE:
          #endif
            case FUNCID_ET_MOSTREMOTERESET:
          #ifdef FUNCID_ET_DSO
            case FUNCID_ET_DSO:
          #endif
          #ifdef FUNCID_ET_DSIHOLD
            case FUNCID_ET_DSIHOLD:
          #endif
                Tx_Ptr->Data[2] = MAT_PARTLY_IMPLEMENTED_VERIFIED;
                break;

            case FUNCID_ET_VERSION:
            case FUNCID_ET_DIAGRESULT:
            case FUNCID_ET_SENDMESSAGE:
            case FUNCID_ET_ECHOMESSAGE:
            case FUNCID_ET_CODINGERRORS:
            case FUNCID_ET_SYSTEMSTATE:
            case FUNCID_ET_PHYSICALLAYERTEST:
            case FUNCID_ET_PHYSICALLAYERRESULT:
            case FUNCID_ET_VERSION2:
                Tx_Ptr->Data[2] = MAT_FULLY_IMPLEMENTED_VERIFIED;
                break;

            case FUNCID_ET_NOTIFICATIONMATRIXSIZE:
              #ifdef NTF_MIN
                Tx_Ptr->Data[2] = MAT_FULLY_IMPLEMENTED_VERIFIED;
              #else
                Tx_Ptr->Data[2] = MAT_PARTLY_IMPLEMENTED_VERIFIED;
              #endif
                break;

          #ifdef FUNCID_ET_DSIDSOCOUNT
            case FUNCID_ET_DSIDSOCOUNT:
              #ifdef PACKET_ON_SECOND_HOST
                Tx_Ptr->Data[2] = MAT_PARTLY_IMPLEMENTED_VERIFIED;
              #else
                Tx_Ptr->Data[2] = MAT_FULLY_IMPLEMENTED_VERIFIED;
              #endif
                break;
          #endif

            default:
                Tx_Ptr->Data[2] = MAT_UNKNOWN;
                answer_prepared = MNS_FALSE;
                break;
        }
    }
    else
    {
        switch (rx_id)
        {
            case FBI_FBLOCK_NAME:
                Tx_Ptr->Data[2] = FRMT_ISO8859;
                for(i=(byte)0; i<(byte)(sizeof(name)); ++i)
                {
                    Tx_Ptr->Data[i+3] = (byte)name[i];
                }
                Tx_Ptr->Length = (word)i + (word)3;
                break;

            case FBI_FBLOCK_VERSION:
                version[0] = ET_VERSION_MAJOR;
                version[1] = ET_VERSION_MINOR;
                version[2] = ET_VERSION_BUILD;

                Tx_Ptr->Data[2] = FRMT_ISO8859;

                i = MsgVersionToISO8859(&version[0], &Tx_Ptr->Data[3]);

                Tx_Ptr->Length  = (word)((word)i + (word)3);
                break;

            case FBI_MOST_VERSION:
                version[0]  = GFB_VERSION_MAJOR;
                version[1]  = GFB_VERSION_MINOR;
                version[2]  = GFB_VERSION_BUILD;

                Tx_Ptr->Data[2] = FRMT_ISO8859;

                i = MsgVersionToISO8859(&version[0], &Tx_Ptr->Data[3]);

                Tx_Ptr->Length  = (word)((word)i + (word)3);
                break;

            case FBI_FBLOCK_TYPE:
                Tx_Ptr->Data[2] = FRMT_ISO8859;
                Tx_Ptr->Data[3] = (byte)'\0';
                Tx_Ptr->Length  = (word)4;
                break;

            default :
                answer_prepared = MNS_FALSE;
                break;

        }
    }

    /* Call Application, because request cannot be answered by NetServices completely */
    if (MNS_FALSE == ET_FBlockInfo_Query(rx_id, answer_prepared, Tx_Ptr))                   /* unknown ID ? */
    {
        return_value = CmdErrorParamWrong( Tx_Ptr, (byte)1, &Rx_Ptr->Data[0], (byte)2);     /* prepare error message   */
    }
    else
    {
        return_value = OP_STATUS;
    }

    return(return_value);
}
#endif




