/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Implementation of the Port Message Service (PMS)
File:           pms.c
Version:        3.0.x-SR-1  
Language:       C
Author(s):      R.Lundstrom, T.Jahnke
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
  * \brief      Implementation of the Port Message Service (PMS)
  */


/*
==============================================================================
    Includes
==============================================================================
*/

#include "mns.h"
#include "mbm.h"
#include "mis.h"
#include "ams.h"
#include "pms.h"
#include "pms_pv.h"


#ifdef PMS_MLOG_STORE
    #include "mlog.h"
    #ifdef MLOG_ENABLED
        #define PMS_LOG_RX(data_ptr)    MLogAddLine(PMS_MLOG_STORE, 0, data_ptr)
        #define PMS_LOG_TX(data_ptr)    MLogAddLine(PMS_MLOG_STORE, 1, data_ptr)
    #endif
#endif

#ifndef PMS_LOG_RX
    #define PMS_LOG_RX(data_ptr)
    #define PMS_LOG_TX(data_ptr)
#endif

/*
==============================================================================
    NetServices trace: module trace macros
==============================================================================
*/

#if (MNS_DEBUG & NST_C_FUNC_ENTRIES)
    #define T_API_ENTRY(func)   MNS_TRACE((MNS_P_SRV_PMS, NST_E_FUNC_ENTRY_API, 1, func))
    #define T_LIB_ENTRY(func)   MNS_TRACE((MNS_P_SRV_PMS, NST_E_FUNC_ENTRY_LIB, 1, func))
    #define T_MOD_ENTRY(func)   MNS_TRACE((MNS_P_SRV_PMS, NST_E_FUNC_ENTRY_MOD, 1, func))
#else
    #define T_API_ENTRY(func)
    #define T_LIB_ENTRY(func)
    #define T_MOD_ENTRY(func)
#endif

#if (MNS_DEBUG & NST_C_FUNC_EXITS)
    #define T_API_EXIT(func)    MNS_TRACE((MNS_P_SRV_PMS, NST_E_FUNC_EXIT_API, 1, func))
    #define T_LIB_EXIT(func)    MNS_TRACE((MNS_P_SRV_PMS, NST_E_FUNC_EXIT_LIB, 1, func))
    #define T_MOD_EXIT(func)    MNS_TRACE((MNS_P_SRV_PMS, NST_E_FUNC_EXIT_MOD, 1, func))
#else
    #define T_API_EXIT(func)
    #define T_LIB_EXIT(func)
    #define T_MOD_EXIT(func)
#endif

#if (MNS_DEBUG & NST_C_INIT)
    #define T_INIT()            MNS_TRACE((MNS_P_SRV_PMS, NST_E_INIT, 0))
#else
    #define T_INIT()
#endif

#if (MNS_DEBUG & NST_C_SERVICE)
    #define T_SERVICE(event)    MNS_TRACE((MNS_P_SRV_PMS, NST_E_SERVICE, 1, event))
#else
    #define T_SERVICE(event)
#endif

#if (MNS_DEBUG & NST_C_REQUESTS)
    #define T_REQUEST(event)    MNS_TRACE((MNS_P_SRV_PMS, NST_E_REQUEST, 1, event))
#else
    #define T_REQUEST(event)
#endif

#ifdef _lint
    /*lint -emacro(*,*ASSERT)*/
    #define FAILED_ASSERT()     return
    #define ASSERT(exp)         if(!(exp)) return
#else
  #if (MNS_DEBUG & NST_C_ASSERTS)
    #define FAILED_ASSERT()     MNS_TRACE((MNS_P_SRV_PMS, NST_E_ASSERT, 1, __LINE__))
    #define ASSERT(exp)         if(!(exp)) FAILED_ASSERT()
  #else
    #define FAILED_ASSERT()
    #define ASSERT(exp)
  #endif
#endif


/*
==============================================================================
    Packet communication only
==============================================================================
*/

#ifdef PACKET_COMMUNICATION_ONLY
    /* Asynchronous only, replace the rest of the content */
    #include "pmsa.inc"
#else

/*
==============================================================================
    Module internal variables
==============================================================================
*/

#ifdef PMS_MIN

    static TMostTimer pmsSyncTimer;
    #ifdef PMS_28
    static TMostTimer pmsGbgTimer; /* Garbage collector */
    #endif

    static PmsInitStruct *pmsInitData_ptr; /* Callbacks and configuration */

    word     pmsPending;
    static byte     pmsCnt;

    static PmsSyncState pmsSyncState;
    static word pmsSyncTimeout;
    static word pmsMaxCmsPayload;
    static bool pmsSupportsTelId4;

    static PmsIface i2c;
    static PmsIface generalCtrl;

    static PmsFifo  icmFifo;
    static PmsFifo  mcmFifo;

    #ifndef PMS_TX_NOSEG
    static MbmShadow txSegShadows[PMS_NUM_ACTIVE_SEGMENTS];
    static MbmQueue  txSegShadowQ;
    #endif

    #ifdef MDP_MIN
    static PmsFifo  mdpFifo;
    #endif

    #ifdef ADS_MIN  /* Not needed for MDP_MIN, only I2C and no need for the pre-alloc Q */
    static PmsIface generalData;
    #if (ADS_RX_PREALLOC > 0)
    static MbmQueue mdpRxPreallocQ;
    #endif
    #endif

    static MbmQueue rxQ;
    static MbmQueue rxStatusQ;

    #ifndef PMS_RX_NOSEG
    static MbmQueue rxSegQ;
    #endif
    static byte     segErrData[PMS_BUFSIZE_SEG_ERROR];
    static MbmBuf   segErrMsg;

    static HMBMBUF  cmdRsvd;    /* Reserved command msg to prevent deadlock   */
    static HMBMBUF  cmdFree;    /* Same message when free                     */

    static PmsStruct pms;


    /*
    ----------------------------------------------------------------------------
        FIFO specific constant configurations
    ----------------------------------------------------------------------------
    */
    static _CONST PmsFifoDefCfg icmDefCfg =
    {
        PMS_M_ICM_RX_SCF0_DEFAULT,                   /* byte rxScf0Default,   */
        PMS_M_ICM_RX_SCF1_DEFAULT                    /* byte rxScf1Default,   */
    };

    static _CONST PmsFifoDefCfg mcmDefCfg =
    {
        PMS_M_MCM_RX_SCF0_DEFAULT,                   /* byte rxScf0Default,   */
        PMS_M_MCM_RX_SCF1_DEFAULT                    /* byte rxScf1Default,   */
    };


    /*!
     * Table with pointers to FIFOs, used to arbitrate them.
     * FIFOs are in prioritized order.
     */
    static PmsFifo * _CONST fifoPtrPrioTab[] =
    {
        &icmFifo
        , &mcmFifo
        #ifdef MDP_MIN
        , &mdpFifo
        #endif
        ,NULL
    };
    

    /*!
     * Table of FIFO numbers matched to their mask for smooth conversion, note that
     * this is NOT related to the prioritized order of the fifoPtrPrioTab.
     */
    static _CONST byte fifoNoToMaskTab[] =
    {
        PMS_M_FIFO_MCM,
        PMS_M_FIFO_MDP,
        PMS_M_FIFO_ICM,
        PMS_M_FIFO_ALL
    };

#endif  /* #ifdef PMS_MIN */


/*
==============================================================================
    Implementation of API functions
==============================================================================
*/

/*
--------------------------------------------------------------------------------
 Function:      PmsRx(...)

 Description:   Receives incoming messages of any type from the LLD.

 Input(s):      handle : Received message

 Return(s):     None.
--------------------------------------------------------------------------------
*/
#ifdef PMS_0
void PmsRx(HMBMBUF handle)
{
    MbmQueue *q_ptr;
    word     event;

    T_API_ENTRY(PMS_0);

    ASSERT(handle);
    if(!handle) /*lint !e774 boolean always false */
    {
        T_API_EXIT(PMS_0);
        return;
    }

    PMS_LOG_RX(handle->hdr_ptr);

    if( PMS_RX_IS_STATUS(handle) )
    {
        q_ptr = &rxStatusQ;
        event = PMS_P_RX_STATUS;
    }
    else
    {
        q_ptr = &rxQ;
        #ifdef PMS_RX_OPT4
        event = PMS_P_RX | PMS_P_RX_ACK | PMS_P_TX_TRIG;
        #else
        event = PMS_P_RX;
        #endif
    }
    MbmEnqueue(q_ptr, handle);

    #ifdef PMS_RX_TRIGGERS_TX
    /*
     * Some LLDs might block TX while an RX is ongoing, in such case it may be
     * necessary to trigger TX on completion of RX.
     */
    event |= PMS_P_TX_TRIG;
    #endif

    PmsSetPendingEvent((word)event);

    T_API_EXIT(PMS_0);
}
#endif

#ifdef PMS_16
static _INLINE void _PmsTxNextSegment(HMBMBUF handle)
{
    byte *head_ptr;
    byte i;
    word len;
    MbmCtrlTx *cmsg_ptr;

    T_MOD_ENTRY(PMS_16);

    /*
     * Check if the message is segmented by checking the telId of the
     * compressed headers (if it is 1-3 it is segmented).
     */
    head_ptr = MBM_GET_HDR_PTR(handle); /* Buffer start    */

    /**
     * Get index to length-field as:
     * sizeof(PML) + PMHL + sizeof(PMHL) + sizeof(FuncID + Op)
     * => PMHL + 5
     */
    i = (head_ptr[2] + (byte)5);

    cmsg_ptr = MBM_GET_CTRL_TX_PTR(handle);

    if ((head_ptr[i] & (byte)0xF0) == (byte)0x40)
    {
        /* previous message was TelID 0x4 */
        MbmPullHeaders(handle);
        (void)_PmsCompressHeaderMCM(handle, MNS_FALSE);
    }
    else
    {
        /* 1. Set payload pointer (add max payload - 1 for Message counter) */
        MBM_GET_PAYLOAD_PTR(handle) += (PMS_CTRL_MAX_PAYLOAD - 1);

        /* 2. Calculate remaining length (add 1 for Message Counter) */
        len = (word)((cmsg_ptr->Length - (MBM_GET_PAYLOAD_PTR(handle) - cmsg_ptr->Data)) + (word)1);

        /* 3. Set tel_id & length */
        if( len > (word)PMS_CTRL_MAX_PAYLOAD )
        {
            /* Not last segment */
            len = ((word)0x2000 | (word)PMS_CTRL_MAX_PAYLOAD);
        }
        else
        {
            /* Last segment */
            word old_len;
            word pml;

            /* Mark as last to avoid another prepare */
            MBM_SET_STATUS_BITS(handle, MBM_STAT_TX_LAST_SEG);

            /* Calculate and possibly set different PML */
            DECODE_WORD(&pml, head_ptr);
            DECODE_WORD(&old_len, &head_ptr[i]);
            old_len &= (word)0x0FFF;
            pml -= (word)(old_len - len);
            head_ptr[0] = HB(pml);
            head_ptr[1] = LB(pml);

            /**
             * Set length of additional data, subtract 1 since Message
             * Counter is located with the headers.
             */
            MBM_SET_PAYLOAD_LEN(handle, (len - (word)1));

            len |= (word)0x3000;
        }
        /* Write new length + tel_id directly to message buffer */
        head_ptr[i++] = HB(len);
        head_ptr[i++] = LB(len);

        /* 3. Increment Message Counter */
        head_ptr[i] += (byte)1;
    }

    T_MOD_EXIT(PMS_16);
}
#endif

#ifdef PMS_1
/*!
  * \brief      Called by the LLD when it has completed using a buffer, it can
  *             be done as soon as the LLD does not need the buffer anymore. It
  *             may not reference the buffer after this.
  * \details    N.B. PREFERABLY THE LLD SHOULD CALL THIS FUNCTION WHEN IT IS
  *             READY TO PROCESS ANOTHER BUFFER SINCE THIS ALSO TRIGGERS THE
  *             EVENT TO TRY THE NEXT BUFFER FOR SENDING.
  * \param[in]  handle      Handle to buffer released by LLD
  */
void PmsTxRelease(HMBMBUF handle)
{
    word events;
    bool free;

    T_API_ENTRY(PMS_1);

    events = (word)0;
    free   = MNS_FALSE;

    ASSERT(handle);
    if(!handle) /*lint !e774 Always false due to assert */
    {
        T_API_EXIT(PMS_1);
        return;
    }

    WAIT4MX(MX_PMS_CS);

    MBM_SET_STATUS_BITS(handle, MBM_STAT_TX_SENT);

    if( PMS_TX_IS_CMD_OR_STATUS(handle) )
    {
        /* Command message, free immediately */
        if( cmdRsvd == handle )
        {
            /* This was the Command message dedicated to SyncC */
            cmdFree = handle; /* Or cmdRsvd... same, same... */
        }
        else if( MBM_GET_STATUS(handle) & MBM_STAT_RSVD )
        {
            MBM_CLR_STATUS_BITS(handle, MBM_STAT_RSVD);

            #ifdef PMS_RX_SEND_ACK
            if (pms.needRetriggerAck)
            {
                pms.needRetriggerAck = 0;
                events |= PMS_P_RX_ACK;
            }
            #endif
        }
        else
        {
            /* Free handle when releasing mutex */
            free = MNS_TRUE;
        }
    }
    else
    {
        PmsFifo *fifo_ptr = PmsGetTxFifo(handle);
        #ifndef PMS_TX_NOSEG
        MbmBuf *pending_ptr;

        WAIT4MX(MX_PMS_Q);
        if( MBM_STAT_TX_SEG & MBM_GET_STATUS(handle) )
        {
            ASSERT(&mcmFifo == fifo_ptr); /* Else the message has probably been garbled */

            pending_ptr = (MbmBuf *)_PMS_ALLOC_SHADOW();
            /*
             * Since a pending message never could be triggered if there
             * were no shadows available and this is the only place where
             * they are allocated it is not is not possible that NULL is
             * returned unless the system is experiencing som serious
             * problem
             */
            ASSERT(pending_ptr);
            ((MbmShadow *)pending_ptr)->real_ptr = handle;

            if( handle->type & MBM_STAT_TX_LAST_SEG )
            {
                /* Shadow of the last segment */
                pending_ptr->type |= MBM_STAT_TX_LAST_SEG;
            }
            else
            {
                /* Prepare & enqueue the next segment */
                HMBMBUF     hCur;

                /* This shadow does not reference the last segment */
                pending_ptr->type &= (word)~MBM_STAT_TX_LAST_SEG;

                _PmsTxNextSegment(handle);

                hCur = mcmFifo.msgQ.next_ptr;
                /* Do not overtake messages that has bypassed */
                while( ((HMBMBUF)&mcmFifo.msgQ != hCur) && (hCur->type & MBM_STAT_TX_BYPASS) )
                {
                    hCur = hCur->next_ptr;
                }

                /* Enqueue before wherever we stopped... */
                _MbmEnqueueBehind(&mcmFifo.msgQ, hCur->prev_ptr, handle);
            }
        }
        else
        {
            pending_ptr = handle;
        }
        _MbmEnqueue(&fifo_ptr->pendingQ, pending_ptr);

        REL_MX(MX_PMS_Q);
        #else
        MbmEnqueue(&fifo_ptr->pendingQ, handle);
        #endif
    }

    /* Release from interface */
    if( i2c.active == handle )
    {
        i2c.active = NULL;
    }
    else if( generalCtrl.active == handle )
    {
        generalCtrl.active = NULL;
    }
    #ifdef ADS_MIN /* Not needed for MDP_MIN, no TX */
    else if( generalData.active == handle )
    {
        generalData.active  = NULL;
        events             |= PMS_P_MDP_TXSTATUS;
    }
    #endif

    /* Check if TX_TRIG needs to be fired for any FIFO */
    if(    (HAS_TX(&icmFifo) && PMS_IFACE_READY(*icmFifo.iface_ptr))
        || (HAS_TX(&mcmFifo) && PMS_IFACE_READY(*mcmFifo.iface_ptr))
    #ifdef ADS_MIN /* Not needed for MDP_MIN, no TX */
        || (HAS_TX(&mdpFifo) && PMS_IFACE_READY(*mdpFifo.iface_ptr))
    #endif
      )
    {
        events |= PMS_P_TX_TRIG;
    }
    REL_MX(MX_PMS_CS);

    if( MBM_QUEUE_LENGTH(&rxStatusQ) )
    {
        /* Status processing may have been waiting for this to complete */
        events |= PMS_P_RX_STATUS;
    }

    if( MNS_FALSE != free )
    {
        MbmFree(handle);
    }

    if( events )
    {
        PmsSetPendingEvent(events);
    }

    T_API_EXIT(PMS_1);
}
#endif

#ifdef PMS_2
/*!
  * \brief      Sets the Interface used by the PMS to communicate
  *             with the INIC.
  * \details    As soon as it is called, all outgoing messages and data
  *             packets belonging to the respective FIFO will be redirected
  *             to that interface. The default interface used by the MOST
  *             NetServices is given during initialization. Please note that
  *             the FIFO interface for ICM and MCM can not be set seperately.
  *             In order to set the interface for both (MCM/ICM) FIFOs you have
  *             to use PMS_M_FIFO_CTRL.
  * \param[in]  fifomask    FIFO to set the interface. Possible values are:
  *                         -# PMS_M_FIFO_CTRL (all control messages (ICM, MCM))
  *                         -# PMS_M_FIFO_DATA (data packets (MDP))
  * \param[in]  iface       Interface for outgoing messages and data packets
  *                         (direction from EHC to INIC). Possible values are:
  *                         -# PMS_IFACE_I2C (using I2C)
  *                         -# PMS_IFACE_GENERAL (using MediaLB or SPI)
  */
void PmsSetFifoInterface(byte fifomask, byte iface)
{
    #if (defined ADS_MIN) && (defined PMS_RX_SEND_ACK)/* Only available with ADS_MIN */
    HMBMBUF handle_to_free;
    bool flush_mdp;
    #endif

    T_API_ENTRY(PMS_2);

    #ifndef PMS_RX_SEND_ACK
    (void)iface;
    #endif

    #if (defined ADS_MIN) && (defined PMS_RX_SEND_ACK)/* Only available with ADS_MIN */
    handle_to_free = NULL;
    flush_mdp = MNS_FALSE;
    #endif

    WAIT4MX(MX_PMS_CS);
    if ( (PMS_M_FIFO_CTRL & fifomask) == PMS_M_FIFO_CTRL )
    {
        #ifdef PMS_RX_SEND_ACK
        icmFifo.iface_ptr = (PMS_IFACE_I2C == iface) ? &i2c : &generalCtrl;
        mcmFifo.iface_ptr = (PMS_IFACE_I2C == iface) ? &i2c : &generalCtrl;
        #else
        icmFifo.iface_ptr = &generalCtrl;
        mcmFifo.iface_ptr = &generalCtrl;
        #endif
    }
    #if (MNS_DEBUG & NST_C_ASSERTS)
    else if (PMS_M_FIFO_CTRL & fifomask)
    {
        FAILED_ASSERT(); /* Wrong parameter: fifomask must not be PMS_M_FIFO_MCM    */
                         /* 'or' PMS_M_FIFO_ICM; use PMS_M_FIFO_CTRL instead to set */
                         /* both FIFOs to the same interface                        */
    }
    #endif

    #ifdef MDP_MIN
    if( PMS_M_FIFO_MDP & fifomask )
    {
      #ifdef ADS_MIN /* Other interfaces only available with ADS_MIN */
        #ifdef PMS_RX_SEND_ACK
        if( PMS_IFACE_I2C == iface )
        {
            if (&i2c != mdpFifo.iface_ptr)
            {
                flush_mdp = MNS_TRUE;
                mdpFifo.iface_ptr = &i2c;
                mdpFifo.syncGuard = (byte)0;
            }

            /* A small buffer needs to be preallocated for the acknowledge mechanism */
            if( NULL == pms.rxPreAllocPtrTab[PMS_IDX_MDP_RX_PREALLOC] )
            {
                pms.needPrealloc = 1;
                PmsSetPendingEvent(PMS_P_RX_ACK);   /* Forces preallocation attempt */
            }
        }
        else
        {
            mdpFifo.iface_ptr = &generalData;

            if( pms.rxPreAllocPtrTab[PMS_IDX_MDP_RX_PREALLOC] )
            {
                /* No acknowledges are used and hence no preallocated buffer */
                handle_to_free = pms.rxPreAllocPtrTab[PMS_IDX_MDP_RX_PREALLOC];
                pms.rxPreAllocPtrTab[PMS_IDX_MDP_RX_PREALLOC] = NULL;
                mdpFifo.rxStatus = PMS_STATUS_NONE;
            }
        }
        #else /* PMS_RX_SEND_ACK */
        mdpFifo.iface_ptr = &generalData;
        #endif
      #else /* ADS_MIN */
        mdpFifo.iface_ptr = &i2c; /* Always I2C when only MDP_MIN */
      #endif
    }
    #endif
    REL_MX(MX_PMS_CS);

    #if (defined ADS_MIN) && (defined PMS_RX_SEND_ACK) /* There can only be a buffer to free with ADS */
    if( handle_to_free )
    {
        MbmFree(handle_to_free);
    }

    if (MNS_FALSE != flush_mdp)
    {
        MbmFlush(&mdpFifo.msgQ);
    }
    #endif

    T_API_EXIT(PMS_2);
}
#endif

#ifdef PMS_3
/*!
  * \brief      Gets the Interface used by the PMS to communicate
  *             with the INIC (direction EHC->INIC).
  * \param[in]  fifomask    FIFO that uses the interface. Possible values are:
  *                         -# PMS_M_FIFO_CTRL (all control messages (ICM, MCM))
  *                         -# PMS_M_FIFO_DATA (data packets (MDP))
  * \return     Interface for outgoing messages and data packets
  *             (direction from EHC to INIC). Possible values are:
  *             -# PMS_IFACE_I2C (using I2C)
  *             -# PMS_IFACE_GENERAL (using MediaLB or SPI).
  */
byte PmsGetFifoInterface(byte fifomask)
{
    byte iface;
    T_API_ENTRY(PMS_3);
    ASSERT(PMS_M_FIFO_ALL != fifomask);

    iface = PMS_IFACE_I2C;
    WAIT4MX(MX_PMS_CS);

    #ifdef MDP_MIN
    if( PMS_M_FIFO_MDP & fifomask )
    {
      #ifdef ADS_MIN /* Other interfaces only with ADS_MIN */
        iface = (mdpFifo.iface_ptr == &i2c) ? PMS_IFACE_I2C : PMS_IFACE_GENERAL;
      #else /* Only MDP_MIN */
        iface = PMS_IFACE_I2C;
      #endif
    }
    else
    #endif
    if( PMS_M_FIFO_CTRL & fifomask )
    {
        /* ICM and MCM FIFO always use the same interface */
        iface = (icmFifo.iface_ptr == &i2c) ? PMS_IFACE_I2C : PMS_IFACE_GENERAL;
    }

    REL_MX(MX_PMS_CS);
    T_API_EXIT(PMS_3);
    return( iface );
}
#endif

#ifdef PMS_26
/*!
  * \brief      Returns a message buffer to the Low-Level-Driver in order
  *             to receive a message on the MediaLB MCM RX channel.
  * \details    This function must be called if PMS_RX_OPT3 is defined in
  *             adjust1.h.
  *             If PMS_RX_OPT3 is not defined PmsGetRxBuf() must be called
  *             to get a message buffer for any incoming control message
  *             or MDP.
  * \param[in]  size    The required size of the RX buffer
  * \return     The message buffer or NULL if no message buffer is available.
  */
HMBMBUF PmsGetRxBufMcm(word size)
{
    HMBMBUF handle;
    word timeout;
    bool start_timer;

    T_API_ENTRY(PMS_26);

    handle = MbmGetBuf(size);
    timeout = (word)0;
    start_timer = MNS_FALSE;

    if( !handle )
    {
        if( size <= PMS_CFG_BUFSIZE_SMALL )
        {
            WAIT4MX(MX_PMS_CS);
            handle = pms.rxPreAllocPtrTab[PMS_IDX_MCM_RX_PREALLOC];
            pms.rxPreAllocPtrTab[PMS_IDX_MCM_RX_PREALLOC] = NULL;
            pms.needPrealloc = 1;

            if (!handle  && !pms.rxPreAllocHasTimedOut && !pms.rxPreAllocFailedTimerStarted)
            {
                pms.rxPreAllocFailedTimerStarted = 1;
                start_timer = MNS_TRUE;
            }
            REL_MX(MX_PMS_CS);

            if (MNS_FALSE != start_timer)
            {
                timeout = MnsGetWatchdogKickTimeout(); /* initial start for the RX watchdog timer */
                MostSetTimer(&pms.rxPreAllocFailedTimer, timeout, MNS_FALSE);
            }
        }

        if (!handle)
        {
            PmsFireBufFreed();
        }
    }

    T_API_EXIT(PMS_26);
    return handle;
}
#endif

#ifdef PMS_27
/*!
  * \brief      Returns a message buffer to the Low-Level-Driver in order
  *             to receive a message on the MediaLB MDP RX channel.
  * \details    This function must be called if PMS_RX_OPT3 is defined in
  *             adjust1.h.
  *             If PMS_RX_OPT3 is not defined PmsGetRxBuf() must be called
  *             to get a message buffer for any incoming control message
  *             or MDP.
  * \param[in]  size    The required size of the RX buffer
  * \return     The message buffer or NULL if no message buffer is available.
  */
HMBMBUF PmsGetRxBufMdp(word size)
{
    HMBMBUF handle;

    T_API_ENTRY(PMS_27);
    handle = MbmGetBuf(size);

    #if (ADS_RX_PREALLOC > 0)
    if (!handle)
    {
        if( size <= PMS_CFG_BUFSIZE_DATA_LARGE )
        {
            handle = MbmDequeue(&mdpRxPreallocQ);
        }

        if (!handle)
        {
            PmsFireBufFreed();
        }
    }
    #else
    if (!handle)
    {
        PmsFireBufFreed();
    }
    #endif

    T_API_EXIT(PMS_27);
    return handle;
}
#endif

#ifdef PMS_31
/*!
  * \brief      Returns a message buffer to the Low-Level-Driver in order
  *             to receive a message from the INIC
  * \details    If PMS_RX_OPT3 is defined in adjust1.h the LLD has to use
  *             PmsGetRxBuf() only to request a buffer for an incoming message
  *             on MediaLB channel address 0x0002. In addition PmsGetRxBufMcm()
  *             and PmsGetRxBufMdp() have to be used to get a buffer for incoming
  *             messages on the respective RX channel.
  *             If PMS_RX_OPT3 is not defined this function must be called to get
  *             a message buffer for any incoming control message or MDP.
  * \param[in]  size    The required size of the RX buffer
  * \return     The message buffer or NULL if no message buffer is available.
  */
HMBMBUF PmsGetRxBuf(word size)
{
    HMBMBUF handle;

    T_API_ENTRY(PMS_31);

    /* Try to allocate new buffer first */
    handle = MbmGetBuf(size);

    if( !handle )
    {
        if( size <= PMS_CFG_BUFSIZE_SMALL )
        {
            WAIT4MX(MX_PMS_CS);
            handle = pms.rxPreAllocPtrTab[PMS_IDX_RSVD_RX_PREALLOC];
            pms.rxPreAllocPtrTab[PMS_IDX_RSVD_RX_PREALLOC] = NULL;
            pms.needPrealloc = 1;
            REL_MX(MX_PMS_CS);
        }
        #if ((ADS_RX_PREALLOC > 0) && !defined (PMS_RX_OPT3))
        else if( size <= PMS_CFG_BUFSIZE_DATA_LARGE )
        {
            handle = MbmDequeue(&mdpRxPreallocQ);
        }
        #endif

        if (!handle)
        {
            PmsFireBufFreed();
        }
    }
    T_API_EXIT(PMS_31);

    return( handle );
}
#endif

#ifdef PMS_41
void PmsSetExternalBufferAvailable(void)
{
    T_API_ENTRY(PMS_41);
    if( MBM_QUEUE_LENGTH(&pms.rxPayloadWaitingQ) )
    {
        PmsSetPendingEvent(PMS_P_PROCESS_WAITING_Q);
    }
    T_API_EXIT(PMS_41);
}
#endif

#ifdef PMS_42
void PmsReleasePayload(TMsgRx *rx_ptr)
{
    HMBMBUF handle;

    T_LIB_ENTRY(PMS_42);

    ASSERT(rx_ptr);
    if( rx_ptr )
    {
        handle = MbmGetHandleByMsgPtr(rx_ptr);


        ASSERT(!handle->start_ptr); /* If this triggers the payload is internal! */

        handle->size = (word)0;
        MBM_NO_USER_TX_FREE(handle);  /* Shall no longer have user's TX payload free called */
    }

    T_LIB_EXIT(PMS_42);
}
#endif

#ifdef PMS_48
void PmsPrepareMepHeaderExt(byte *header_ptr, word current_message_length, byte retry)
{
    T_API_ENTRY(PMS_48);

    ASSERT(header_ptr);
    if( header_ptr )
    {
        current_message_length += 6; /* Include this header, PML not counted */
        *header_ptr++ = HB(current_message_length); /* PML MSB */
        *header_ptr++ = LB(current_message_length); /* PML LSB */
        *header_ptr++ = (byte)0x05;                 /* PMHL    */
        *header_ptr++ = (PMS_FIFONO_MEP << FPH_B_FIFONO_LSB) | FPH_MSGTYPE_DATA; /* FPH */
                                                    /* High nibble Retry, low Priority */
        *header_ptr++ = ( (byte)((retry & (byte)0x0F) << 4) ) | (PMS_DEF_PRIO & (byte)0x0F);
        *header_ptr++ = (byte)0x00;                 /* stuffing byte 5 */
        *header_ptr++ = (byte)0x00;                 /* stuffing byte 6 */
        *header_ptr   = (byte)0x00;                 /* stuffing byte 7 */
    }

    T_API_EXIT(PMS_48);
}
#endif

#ifdef PMS_49
void PmsTxReady(void)
{
    T_API_ENTRY(PMS_49);
    PmsSetPendingEvent(PMS_P_TX_TRIG);
    T_API_EXIT(PMS_49);
}
#endif

/*
==============================================================================
    Implementation of Library functions
==============================================================================
*/
#ifdef PMS_8
void MbmFree(HMBMBUF handle)
{
    #if (defined ADS_MIN) && !(defined PMS_RX_OPT3) /* Not needed for MDP_MIN, only I2C */
    bool mdpOnMlb;
    #endif

    #ifndef PMS_RX_OPT3
    bool trigger;               /* Trigger the acknowledge mechanism */
    #endif
    word i;
    word timeout;
    bool start_timer;
    bool ok;
    bool is_command;

    T_LIB_ENTRY(PMS_8);

    #ifndef PMS_RX_OPT3
    trigger = MNS_FALSE;
    #endif
    timeout = (word)0;
    start_timer = MNS_FALSE;
    is_command  = MNS_FALSE;

    /*lint -e{960} right-hand operator has no side effects (read only)*/
    if( handle && (MNS_FALSE == MbmIsFromPool(handle)) )
    {
        /* Statically allocated message, just clear RSVD bit to mark it available */
        MBM_CLR_STATUS_BITS(handle, MBM_STAT_RSVD);
        handle = NULL;  /* Processed */
    }

    #if defined(MSG_RX_USER_PAYLOAD) || defined(MSG_TX_USER_PAYLOAD) || defined(DATA_TX_USER_PAYLOAD)
    if( handle )
    {
        #ifdef MSG_RX_USER_PAYLOAD
        if( HAS_RX_USER_PAYLOAD(handle) )
        {
            /* RX message with user payload */
            pmsInitData_ptr->cbFreePayloadBuf_fptr(MBM_GET_CTRL_RX_PTR(handle));
            handle->size = (word)0;
        }
          #if defined(MSG_TX_USER_PAYLOAD) || defined(DATA_TX_USER_PAYLOAD)
        else
          #endif
        #endif

        #if defined(MSG_TX_USER_PAYLOAD) || defined(DATA_TX_USER_PAYLOAD)
        if( !(MBM_GET_TYPE(handle) & MBM_TYPE_FIFO_BIT) )   /* Ignore FIFOStatus/Command */
        {
            /*
             * The "user" payload pointer can be taken from any message structure
             * since they all start at the same offset. dataTx is always defined.
             *
             * If the *Data member points outside the "internal" buffer space then
             * this is externally allocated, this is checked in the free function.
             */
            PmsFreeUserTxPayload(handle->msg.dataTx.Data, handle);
        }
        #endif
    }
    #endif

    #if (ADS_RX_PREALLOC > 0)
    if( handle && (PMS_CFG_BUFSIZE_DATA_LARGE == handle->size) && (MBM_QUEUE_LENGTH(&mdpRxPreallocQ) < PMS_CFG_ADS_RX_PREALLOC) )
    {
        if( MBM_TYPE_TX_BIT & MBM_GET_TYPE(handle) )
        {
            ok = MbmChangeType(handle, (word)0); /* Check reserved */
        }
        else
        {
            ok = MNS_TRUE;
        }

        if( MNS_FALSE != ok )
        {
            MbmReserve(handle, (word)0);
            MbmEnqueue(&mdpRxPreallocQ, handle);
            handle = NULL;
        }
        /* Else it'll be freed later */
    }
    #endif

    if( pms.needPrealloc )
    {
        if( handle )
        {
            /* Only small buffers hereafter */
            if( PMS_CFG_BUFSIZE_SMALL != handle->size )
            {
                ok = MNS_FALSE;
            }
            else if( MBM_TYPE_TX_BIT & MBM_GET_TYPE(handle) )
            {
                is_command = (MBM_TYPE_CMD_TX == MBM_GET_TYPE(handle)) ? MNS_TRUE : MNS_FALSE;

                ok = MbmChangeType(handle, (word)0); /* Check reserved */

                if( MNS_FALSE != is_command )
                {
                    /*
                     * Ignore false return value if this was a FIFO-Command message,
                     * these are converted from RX messages and their 'return' to
                     * pre-allocated buffers may not be blocked.
                     */
                    ok = MNS_TRUE;
                }
            }
            else
            {
                ok = MNS_TRUE;
            }

            if( MNS_FALSE != ok ) /* It is our size, use it */
            {
                MbmReserve(handle, (word)0);
            }
            else
            {
                MbmFree2(handle);
                handle = NULL;
            }
        }

        /* Try to preallocate all necessary RX buffers */
        WAIT4MX(MX_PMS_CS);
        #if (defined ADS_MIN) && !(defined PMS_RX_OPT3)  /* Not needed for MDP_MIN, only I2C */
        mdpOnMlb = (&generalData == mdpFifo.iface_ptr) ? MNS_TRUE : MNS_FALSE;
        #endif
        i = (word)0;

        do
        {
            #if (defined ADS_MIN) && !(defined PMS_RX_OPT3) /* Not required for MDP_MIN, no preallocated buffer */
            if( (i != PMS_IDX_MDP_RX_PREALLOC) || (MNS_FALSE == mdpOnMlb) ) /* MDP over MLB does not require this buffer */
            #endif
            {
                if( NULL == pms.rxPreAllocPtrTab[i] )
                {
                    /* FIFO needs buffer */
                    if( !handle )
                    {
                        handle = MbmGetBuf(PMS_CFG_BUFSIZE_SMALL);
                    }

                    if( handle )
                    {
                        pms.rxPreAllocPtrTab[i] = handle;
                        handle  = NULL; /* used */
                        #ifndef PMS_RX_OPT3
                        trigger = MNS_TRUE; /* Trigger the acknowledge mechanism */
                        #endif
                        if( PMS_IDX_MCM_RX_PREALLOC == i )
                        {
                            /* No need for a timeout */
                            start_timer = MNS_TRUE;
                            timeout = (word)0;
                        }
                    }
                    else
                    {
                        break; /* quit */
                    }
                }
            }
            i++;
        } while( i < (word)PMS_RX_PREALLOC ); /* With only MDP_MIN no MDP prealloc will be included */

        if( (word)PMS_RX_PREALLOC == i )
        {
            pms.needPrealloc = 0;
            /*
             * All needed preallocated buffers filled, else 'break' would have
             * prevented incrementing 'i'.
             */
        }
        else
        {
            /* Not all FIFOs has a pre-allocated buffer */

            if( NULL == pms.rxPreAllocPtrTab[PMS_IDX_MCM_RX_PREALLOC] )
            {
                /*
                 * Check the pre-allocation failed timer exclusively for MCM.
                 * Don't care if it is already running.
                 */
                if( !pms.rxPreAllocFailedTimerStarted )
                {
                    #ifdef PMS_RX_OPT3
                    if( pms.rxPreAllocHasTimedOut )
                    {
                        /*
                         * The initial timer is started during PmsRx()
                         * if the MCM slot is not filled. When the
                         * timer elapses the next timeout is shorter.
                         */
                        pms.rxPreAllocFailedTimerStarted = 1;
                        start_timer = MNS_TRUE;
                        timeout = PMS_T_PREALLOC_RETRY;
                    }
                    #else
                    timeout = MnsGetWatchdogKickTimeout();

                    if( pms.rxPreAllocHasTimedOut )
                    {
                        /*
                         * There has been at least one timeout and the slot has not
                         * been filled yet, make next timeout short.
                         */
                        timeout = PMS_T_PREALLOC_RETRY;
                    }
                    pms.rxPreAllocFailedTimerStarted = 1;
                    start_timer = MNS_TRUE;
                    #endif
                }
            }
        }
        REL_MX(MX_PMS_CS);

        if( MNS_FALSE != start_timer )
        {
            MostSetTimer(&pms.rxPreAllocFailedTimer, timeout, MNS_FALSE);

            if ((word)0 == timeout)
            {
                WAIT4MX(MX_PMS_CS);
                pms.rxPreAllocFailedTimerStarted = 0;     /* reset the rxPreAllocFailedTimerStarted flag if the timer was stopped */
                pms.rxPreAllocHasTimedOut = 0;            /* Clear the flag, we don't need short timeouts */
                REL_MX(MX_PMS_CS);
            }
        }
    } /* if(pms.needPrealloc) */

    if( handle )
    {
        MbmFree2(handle);
    }

    if( MNS_FALSE != pms.fire_buf_freed )
    {
        pms.fire_buf_freed = MNS_FALSE;

        if( pmsInitData_ptr->cbBufFreed_fptr )
        {
            pmsInitData_ptr->cbBufFreed_fptr();
        }
    }

    #ifndef PMS_RX_OPT3
    if( MNS_FALSE != trigger )
    {
        PmsSetPendingEvent(PMS_P_RX_ACK);
    }
    #endif

    T_LIB_EXIT(PMS_8);
}
#endif

#ifdef PMS_4
void PmsInit(PmsInitStruct *is_ptr)
{
    #if !defined(PMS_TX_NOSEG) || (ADS_RX_PREALLOC > 0)
    word i;
    #endif
    T_LIB_ENTRY(PMS_4);

    ASSERT(is_ptr);

    T_INIT();

    /* - Start initialization of the pms structure -------------------------- */
    MNS_MEM_SET((byte *)&pms, (byte)0, (word)sizeof(pms));    /* Default zerofill */

    /* Set necessary values */
    pms.needPrealloc = 1;   /* Need to fill preallocated buffers first */
    /* - End initialize of the pms structure -------------------------------- */

    pmsInitData_ptr = is_ptr;

    #ifdef MSG_RX_USER_PAYLOAD
    /* No point in going on without them */
    ASSERT(pmsInitData_ptr->cbGetPayloadBuf_fptr && pmsInitData_ptr->cbFreePayloadBuf_fptr);
    #endif

    #ifdef AMS_TX_OUT_FILTER
    ASSERT(NULL != pmsInitData_ptr->cbTxOutFilter_fptr);
    #endif

    MostRegisterTimer(&pmsSyncTimer, PmsSetPendingEvent, PMS_P_TIMEOUT);
    #ifdef PMS_28
    MostRegisterTimer(&pmsGbgTimer, PmsSetPendingEvent, PMS_P_GBG_COL);
    #endif
    MostRegisterTimer(&pms.rxPreAllocFailedTimer, PmsSetPendingEvent, PMS_P_TIMEOUT_RX_PREALLOC);

    WAIT4MX(MX_PMS_PE);
    pmsPending        = PMS_P_NONE;
    REL_MX(MX_PMS_PE);
    pmsSyncState      = PMS_S_INIT;
    pmsSyncTimeout    = PMS_T_SYNC;
    pmsMaxCmsPayload  = PMS_MAX_CMS_PAYLOAD_MOST50;
    pmsSupportsTelId4 = MNS_FALSE;
    pmsCnt            = (byte)1;

    WAIT4MX(MX_PMS_CS);

    cmdFree = MbmAllocate(PMS_CMD_SIZE, (word)0, MBM_TYPE_CMD_TX);
    cmdRsvd = cmdFree;
    ASSERT(cmdRsvd);    /* Will not work without it */

    MNS_MEM_SET((byte *)&segErrMsg, (byte)0, (word)sizeof(segErrMsg));
    segErrMsg.start_ptr = segErrData;
    segErrMsg.size      = (word)PMS_BUFSIZE_SEG_ERROR;
    segErrMsg.type      = MBM_TYPE_CTRL_TX;
    MbmReserve(&segErrMsg, (word)PMS_CTRL_HDR_MAX_SIZE);
    segErrMsg.msg.msgTx.Operation = OP_ERROR;
    segErrMsg.msg.msgTx.Length    = (word)2;
    segErrMsg.msg.msgTx.Data[0]   = (byte)0x0C;        /* Segmentation error */
    segErrMsg.msg.msgTx.MidLevelRetries = PMS_SEG_ERROR_MLR;
    segErrMsg.msg.msgTx.LowLevelRetries = PMS_SEG_ERROR_LLR;

    /* Init Interfaces */
    PMS_IFACE_INIT(i2c);
    i2c.tx_fptr = is_ptr->i2cTx_fptr;
    PMS_IFACE_INIT(generalCtrl);
    generalCtrl.tx_fptr = is_ptr->ctrlTx_fptr;

    #ifdef ADS_MIN /* No MLB/SPI interface for MDP_MIN */
    PMS_IFACE_INIT(generalData);
    generalData.tx_fptr = is_ptr->dataTx_fptr;
    #endif

    /*
    */
    pms.mid_level_retries = (byte)DEF_MID_LEVEL_RETRIES;
    pms.low_level_retries = PMS_DEF_RETRY_1;
    REL_MX(MX_PMS_CS);


    WAIT4MX(MX_PMS_Q);
    MbmQueueInit(&rxQ, (word)MX_PMS_Q);
    MbmQueueInit(&rxStatusQ, (word)MX_PMS_Q);

    #if (ADS_RX_PREALLOC > 0)
    MbmQueueInit(&mdpRxPreallocQ, (word)MX_PMS_Q);
    i = (word)0;
    while( i < PMS_CFG_ADS_RX_PREALLOC )
    {
        HMBMBUF handle = MbmGetBuf(PMS_CFG_BUFSIZE_DATA_LARGE);
        if( handle )
        {
            _MbmEnqueue(&mdpRxPreallocQ, handle);
            i++;
        }
        else
        {
            /* Cannot get enough buffers, incorrect configuration */
            i = PMS_CFG_ADS_RX_PREALLOC;    /* quit trying */
        }
    }
    #endif

    #ifndef PMS_RX_NOSEG
    MbmQueueInit(&rxSegQ, (word)MX_PMS_Q);
    #endif

    #ifndef PMS_TX_NOSEG
    MbmQueueInit(&txSegShadowQ, (word)MX_PMS_Q);
    for( i = (word)0; i < (word)PMS_NUM_ACTIVE_SEGMENTS; i++ )
    {
        txSegShadows[i].type = MBM_TYPE_CTRL_TX | MBM_TYPE_SHADOW;
        _MbmEnqueue(&txSegShadowQ, (HMBMBUF)&txSegShadows[i]);
    }
    #endif
    #ifdef MSG_RX_USER_PAYLOAD
    MbmQueueInit(&pms.rxPayloadWaitingQ, (word)MX_PMS_Q);
    #endif
    REL_MX(MX_PMS_Q);

    PmsFifoInit(&icmFifo);
    PmsFifoInit(&mcmFifo);
    PmsSetFifoInterface(PMS_M_FIFO_CTRL, pmsInitData_ptr->icmIface);

    #ifdef MDP_MIN
    PmsFifoInit(&mdpFifo);
      #ifdef ADS_MIN
      PmsSetFifoInterface(PMS_M_FIFO_MDP, pmsInitData_ptr->mdpIface);
      #else
      PmsSetFifoInterface(PMS_M_FIFO_MDP, PMS_IFACE_I2C); /* Only I2C with MDP_MIN alone */
      #endif
    #endif

    PmsSetPendingEvent(PMS_P_RX_ACK);
    T_LIB_EXIT(PMS_4);
}
#endif

#ifdef PMS_5
void PmsService(void)
{
    word event;
    bool discard_message;
    #ifdef MSG_RX_USER_PAYLOAD
    bool success;
    #endif

    T_LIB_ENTRY(PMS_5);

    discard_message = MNS_FALSE;

    WAIT4MX(MX_PMS_PE);
    event = pmsPending;
    pmsPending = PMS_P_NONE;
    REL_MX(MX_PMS_PE);

    T_SERVICE(event);

    #ifndef PMS_RX_NOSEG
    if( event & (PMS_P_GO_PROTECTED | PMS_P_GO_NET_OFF ) )
    {
        /* Remove all pending segments */
        MbmFlush(&rxSegQ);
        #ifdef MSG_RX_USER_PAYLOAD
        MbmFlush(&pms.rxPayloadWaitingQ);
        #endif
    }
    #endif

    #ifndef PMS_RX_OPT4
    if( event & PMS_P_RX_ACK)
    {
        #ifdef PMS_32
        PmsRxHandleAck();
        #else
        MbmFree(NULL);          /* Try to pre-allocate: relevant for PMS_RX_OPT3 */
        #endif

        if( !pms.ifacesStarted )
        {
            pms.ifacesStarted = 1;
            ASSERT(pmsInitData_ptr->startIfaces_fptr);
            pmsInitData_ptr->startIfaces_fptr();

            /* A timeout will force a SyncC, in case this was a startup */
            PmsSetPendingEvent(PMS_P_TIMEOUT);
        }
    }
    #endif

    #ifdef MSG_RX_USER_PAYLOAD
    success = MNS_TRUE;
    while( MBM_QUEUE_LENGTH(&pms.rxPayloadWaitingQ) && (MNS_FALSE != success) )
    {
        success = PmsProcessRxMsgOut(NULL, &mcmFifo);
        /* If processing returns false we need to wait for PmsSetExternalBufferAvailable() */
    }
    #endif

    if( event & PMS_P_RX )
    {
        PmsProcessRxMsgQueue();
    }

    #ifdef PMS_RX_OPT4
    if( event & PMS_P_RX_ACK)
    {
        PmsRxHandleAck();

        if( !pms.ifacesStarted )
        {
            pms.ifacesStarted = 1;
            ASSERT(pmsInitData_ptr->startIfaces_fptr);
            pmsInitData_ptr->startIfaces_fptr();

            /* A timeout will force a SyncC, in case this was a startup */
            PmsSetPendingEvent(PMS_P_TIMEOUT);
        }
    }
    #endif

    if( event & PMS_P_RX_STATUS )
    {
        PmsProcessRxStatusQueue();
    }

    if( event & PMS_P_TX_TRIG )
    {
        PmsFifo * _CONST *fifo_pptr;
        /*lint -e{413} intended use of null ptr in macro*/
        word offset = PMS_OFFSETOF(PmsFifo, cmdQ);
        word num_queues = (word)2;

        /*
         * First try sending all commands/statuses (offset is set to cmdQ),
         * then try all messages (offset is set to msgQ), for a total of
         * 2 queues.
         */
        while( num_queues-- )
        {
            fifo_pptr = fifoPtrPrioTab;

            do
            {
                MbmQueue *q_ptr = (MbmQueue *)((byte *)*fifo_pptr + offset);
                if( MBM_QUEUE_LENGTH(q_ptr) )
                {
                    PmsFifoTxTrigger(*fifo_pptr, q_ptr);
                }
                fifo_pptr++;
            } while( *fifo_pptr );

            /*lint -e{413} intended use of null ptr in macro*/
            offset = PMS_OFFSETOF(PmsFifo, msgQ);   /* set offset to the message queues */
        }
    }

    if( event & (PMS_P_TIMEOUT | PMS_P_SEND_SYNCC) )
    {
        PmsSyncHandler(event);
    }

    #ifdef ADS_MIN /* Not needed for MDP_MIN, no TX */
    if( event & PMS_P_MDP_TXSTATUS )
    {
        PmsFifoProcessStatus(&mdpFifo, PMS_M_STAT_SLOTAV | PMS_XMIT_SUCCESS, NULL);
    }
    #endif

    #ifdef PMS_28
    if( event & (PMS_P_GBG_COL | PMS_P_RETRIGGER_GBG) )
    {
        /* If ONLY PMS_P_RETRIGGER_GBG is set, then segments should not be marked */
        PmsRxSegGarbageCollector((event & PMS_P_GBG_COL) ? MNS_FALSE : MNS_TRUE);
    }
    #endif


    if( event & PMS_P_TIMEOUT_RX_PREALLOC )
    {
        WAIT4MX(MX_PMS_CS);
        if (NULL == pms.rxPreAllocPtrTab[PMS_IDX_MCM_RX_PREALLOC])
        {
            discard_message = MNS_TRUE;
            pms.rxPreAllocHasTimedOut = 1;
            pms.rxPreAllocFailedTimerStarted = 0;
        }
        REL_MX(MX_PMS_CS);

        /* Timeout trying to fill the preallocated buffers, try to free */
        if (MNS_FALSE != discard_message)
        {
            #ifdef AMS_42
            if( MNS_FALSE == PmsDiscardPendingRx() )
            {
                (void)MsgDiscardRx();
            }
            #else
            (void)PmsDiscardPendingRx();
            #endif
        }
    }

    if( event & PMS_P_GO_SEMI_PROTECTED )
    {
        if (MNS_FALSE != MostIsSupported(NSF_MOST_150))
        {
            pmsMaxCmsPayload = PMS_MAX_CMS_PAYLOAD_MOST150;
            pmsSupportsTelId4 = MNS_TRUE;
        }
        else
        {
            pmsMaxCmsPayload = PMS_MAX_CMS_PAYLOAD_MOST50;
            pmsSupportsTelId4 = MNS_FALSE;
        }
    }

    WAIT4MX(MX_PMS_PE);
    event = (PMS_P_NONE != pmsPending) ? MNS_TRUE : MNS_FALSE;
    REL_MX(MX_PMS_PE);

    if( event )
    {
        MnsSetPendingService(MNS_P_SRV_PMS);
    }

    T_LIB_EXIT(PMS_5);
}
#endif

#ifdef PMS_6
HMBMBUF PmsGetBuf(word size, word type)
{
    HMBMBUF handle;
    word    reserved;
    bool    size_ok;

    T_LIB_ENTRY(PMS_6);
    ASSERT((word)0 == (type & (word)~MBM_TYPE_MASK)); /* parameter type must not contain status bits */

    handle = NULL;

    if( MBM_TYPE_CTRL_TX == type )
    {
        reserved = (word)PMS_CTRL_HDR_MAX_SIZE;
    }
    #ifdef ADS_MIN /* Not needed for MDP_MIN, no TX */
    else if( MBM_TYPE_DATA_TX == type )
    {
        reserved = (word)PMS_DATA_HDR_MAX_SIZE;
    }
    #endif
    else
    {
        reserved = (word)0;
    }

    size_ok = (size > (word)PMS_ALLOC_MAX_SIZE) ? MNS_FALSE : MNS_TRUE;

    if (MNS_FALSE != size_ok)
    {
        size  += reserved;
        #ifdef MSG_RX_USER_PAYLOAD
        handle = MbmAllocate((MBM_TYPE_CTRL_RX == type) ? (word)0 : size, reserved, type);
        #else
        handle = MbmAllocate(size, reserved, type);
        #endif
    }

    if( handle )
    {
        PmsFillDefaultHeader(handle);

        #ifdef MSG_RX_USER_PAYLOAD
        if( (MBM_TYPE_CTRL_RX == type) && size )
        {
            MBM_GET_CTRL_RX_PTR(handle)->Data = NULL; /* New payload needed */
            handle->size = pmsInitData_ptr->cbGetPayloadBuf_fptr(MBM_GET_CTRL_RX_PTR(handle), size);

            if( !handle->size )
            {
                MbmFree(handle);
                handle = NULL;
            }
        }
        #endif
    }

    if (!handle && (MNS_FALSE != size_ok))
    {
        PmsFireBufFreed();
    }

    T_LIB_EXIT(PMS_6);
    return( handle );
}
#endif


#ifdef PMS_7
void PmsSend(HMBMBUF handle, PmsTxStatusHandler cbTxStatus_fptr)
{
    T_LIB_ENTRY(PMS_7);
    ASSERT(handle);
    PmsSendBypass(handle, cbTxStatus_fptr, MNS_FALSE);
    T_LIB_EXIT(PMS_7);
}
#endif


#ifdef PMS_54
void PmsSendBypass(HMBMBUF handle, PmsTxStatusHandler cbTxStatus_fptr, bool auto_bypass)
{
    PmsFifo *fifo_ptr;
    HMBMBUF  hCur = NULL;

    T_LIB_ENTRY(PMS_54);

    ASSERT(handle);
    if( !handle ) /*lint !e774 boolean always false */
    {
        T_LIB_EXIT(PMS_54);
        return;
    }

    handle->cbTxStatus_fptr = cbTxStatus_fptr;  /* Set callback for TX result */

    if (PmsCompressHeader(handle))              /* Append the headers into the TX buffer */
    {
        fifo_ptr = PmsGetTxFifo(handle);        /* Get destination FIFO       */

        if( (&mcmFifo == fifo_ptr) && (handle->msg.msgTx.Length <= pmsMaxCmsPayload) )
        {
            #ifdef AMS_TX_BYPASS_FILTER
            if( (MNS_FALSE != auto_bypass) || (pmsInitData_ptr->cbTxBypass_fptr && (MNS_FALSE != pmsInitData_ptr->cbTxBypass_fptr(&handle->msg.msgTx))) )
            #else
            if( MNS_FALSE != auto_bypass )
            #endif
            {
                MBM_SET_STATUS_BITS(handle, MBM_STAT_TX_BYPASS);
                WAIT4MX(MX_PMS_Q);
                hCur = fifo_ptr->msgQ.prev_ptr;
                /* Enqueue behind all other bypassing messages */
                while( (hCur != (HMBMBUF)&fifo_ptr->msgQ) && !(hCur->type & MBM_STAT_TX_BYPASS) )
                {
                    hCur = hCur->prev_ptr;
                }
                /* Wherever we stopped, the message should be enqueued behind */

                ASSERT(hCur);
                /*
                 * If the assert fails, i.e if hCur becomes NULL we're in an
                 * unrecoverable error state, most likely some operation has been
                 * performed on a buffer in the msgQ (i.e. when not having ownership).
                 */
            }
        }

        if( !hCur )
        {
            WAIT4MX(MX_PMS_Q);
            hCur = fifo_ptr->msgQ.prev_ptr;
        }
        _MbmEnqueueBehind(&fifo_ptr->msgQ, hCur, handle);
        REL_MX(MX_PMS_Q);

        if( PMS_IFACE_READY(*fifo_ptr->iface_ptr) && fifo_ptr->txSlotsAvailable )
        {
            /*
             * Only necessary to trigger if the interface is ready for
             * another buffer and the INIC has a FIFO slot available,
             * otherwise it will be retriggered when the interface
             * has completed or a slot becomes available.
             */
            PmsSetPendingEvent(PMS_P_TX_TRIG);
        }
    }
    else
    {
        MbmFree(handle);    /* packet exceeds the maximum length */
        handle = NULL;
    }

    T_LIB_EXIT(PMS_54);
}
#endif

#ifdef PMS_22
void PmsSync(void)
{
    T_LIB_ENTRY(PMS_22);
    PmsSyncHandler(PMS_P_SEND_SYNCC);
    T_LIB_EXIT(PMS_22);
}
#endif

#ifdef PMS_43
/*
--------------------------------------------------------------------------------
 Function:      PmsInjectWaitingRx(...)

 Description:   Injects an RX message into the queue waiting for an user
                allocated payload. This message has to consist of a message
                with a NS buffer (MBM).

 Input(s):      Handle : The RX message to inject.

 Return(s):     -
--------------------------------------------------------------------------------
*/
void PmsInjectWaitingRx(HMBMBUF handle)
{
    T_MOD_ENTRY(PMS_43);
    MbmEnqueue(&pms.rxPayloadWaitingQ, handle);
    PmsSetPendingEvent(PMS_P_PROCESS_WAITING_Q);
    T_MOD_EXIT(PMS_43);
}
#endif

/*
==============================================================================
    Implementation of Module functions
==============================================================================
*/
#ifdef PMS_9
static void PmsFifoInit(PmsFifo *fifo_ptr)
{
    T_MOD_ENTRY(PMS_9);
    MbmQueueInit(&fifo_ptr->msgQ, (word)MX_PMS_Q);
    MbmQueueInit(&fifo_ptr->cmdQ, (word)MX_PMS_Q);
    MbmQueueInit(&fifo_ptr->pendingQ, (word)MX_PMS_Q);
    fifo_ptr->txSlotsAvailable = (byte)0;                 /* Set on initial Sync */
    fifo_ptr->syncGuard = (byte)0;
    #ifdef PMS_32
    {
        /* Initialize buffer for acknowledge messages */
        MbmBuf *buf_ptr      = &fifo_ptr->ack;
        buf_ptr->start_ptr   = fifo_ptr->ackData;
        buf_ptr->hdr_ptr     = fifo_ptr->ackData;
        buf_ptr->payload_ptr = fifo_ptr->ackData;
        buf_ptr->hdr_len     = (word)0;
        buf_ptr->payload_len = (word)0;
        buf_ptr->size      = PMS_CMD_SIZE;
        buf_ptr->type      = MBM_TYPE_STATUS_TX;
        #ifdef PMS_USE_HANDLE
        buf_ptr->handle    = PMS_DEF_HANDLE;
        #endif
    }
    fifo_ptr->rxStatus = PMS_STATUS_NONE;
    #endif

    #ifdef MDP_MIN
    if( &mdpFifo == fifo_ptr )
    {
        fifo_ptr->timeout = PMS_T_MDP;
    }
    else
    #endif
    if( &icmFifo == fifo_ptr )
    {
        fifo_ptr->timeout = PMS_T_ICM;
    }
    else
    {
        fifo_ptr->timeout = PMS_T_MCM;
    }

    #if defined(MDP_MIN) && !defined(ADS_MIN)
    if( &mdpFifo != fifo_ptr ) /* No need to register since it isn't used with MDP_MIN only */
    #endif
    {
        MostRegisterTimer(&fifo_ptr->timer, PmsSetPendingEvent, PMS_P_TIMEOUT);
    }

    T_MOD_EXIT(PMS_9);
}
#endif

#ifdef PMS_10
void PmsSetPendingEvent(word event)
{
    T_MOD_ENTRY(PMS_10);
    T_REQUEST(event);
    MnsSetPendingEventFlag(event, (word)MX_PMS_PE, &pmsPending, MNS_P_SRV_PMS);
    T_MOD_EXIT(PMS_10);
}
#endif


#ifdef PMS_11
static void PmsFillDefaultHeader(HMBMBUF handle)
{
    #if (MAX_TX_HANDLE > 0) || (MAX_DATA_TX_HANDLE > 0)
    word i;
    byte *d_ptr;
    #endif
    MbmMsgUnion *m_ptr;

    T_MOD_ENTRY(PMS_11);

    #if (MAX_TX_HANDLE > 0) || (MAX_DATA_TX_HANDLE > 0)
    i = (word)0;
    d_ptr = NULL;
    #endif
    m_ptr = &handle->msg;

    if( MBM_TYPE_CTRL_TX == MBM_GET_TYPE(handle) )
    {
        m_ptr->msgTx.Tgt_Adr   = PMS_DEF_TGTADDR;
        m_ptr->msgTx.FBlock_ID = PMS_DEF_FBLOCK_ID;
        m_ptr->msgTx.Inst_ID   = PMS_DEF_INST_ID;
        m_ptr->msgTx.Operation = (byte)0x00;
        m_ptr->msgTx.Func_ID   = (word)0x0000;
        m_ptr->msgTx.Length    = (word)0x0000;
        WAIT4MX(MX_PMS_CS);
        m_ptr->msgTx.MidLevelRetries = pms.mid_level_retries;
        m_ptr->msgTx.LowLevelRetries = pms.low_level_retries;
        REL_MX(MX_PMS_CS);
        #if MAX_TX_HANDLE > 0
        d_ptr = &m_ptr->msgTx.TxHandle[0];
        i     = (word)MAX_TX_HANDLE;
        #endif
    }
    #ifdef ADS_MIN /* Not needed for MDP_MIN, no TX */
    else if( MBM_TYPE_DATA_TX == MBM_GET_TYPE(handle) )
    {
        #ifdef _OS81110_PCK_LLR
        m_ptr->dataTx.Retry     = (byte)(MDP_DEFAULT_RETRY);
        #endif
        m_ptr->dataTx.Tgt_Adr_H = PMS_DEF_TGTADDR_0; /*lint !e778 high byte evaluates to '0'*/
        m_ptr->dataTx.Tgt_Adr_L = PMS_DEF_TGTADDR_1;
        m_ptr->dataTx.Length    = (word)0;
        #if MAX_DATA_TX_HANDLE > 0
        d_ptr = &m_ptr->dataTx.TxHandle[0];
        i     = (word)MAX_DATA_TX_HANDLE;
        #endif
    }
    #endif

    handle->tel_id      = PMS_TELID_0;

    #if (MAX_TX_HANDLE > 0) || (MAX_DATA_TX_HANDLE > 0)
    /* If the actual structure doesn't have a handle i will be 0 */
    if( d_ptr )
    {
        while( i-- )
        {
            *d_ptr++ = (byte)0;
        }
    }
    #endif

    #ifdef PMS_USE_HANDLE
    handle->handle = PMS_DEF_HANDLE;
    #endif
    T_MOD_EXIT(PMS_11);
}
#endif

#ifdef PMS_12
static bool PmsCompressHeader(HMBMBUF handle)
{
    bool  success;

    T_MOD_ENTRY(PMS_12);
    success = MNS_FALSE;

    /*
     * Reset header, necessary if this is a message that is resent by the application,
     * i.e. has not been freed since last use.
     */
    MbmPullHeaders(handle);
    MBM_CLR_STATUS_BITS(handle, MBM_STAT_NONPROTECTED_MASK);

    #ifdef ADS_MIN /* Not needed for MDP_MIN, no TX */
    if( MBM_TYPE_CTRL_TX == MBM_GET_TYPE(handle) )
    #endif
    {
        MbmCtrlTx *cmsg_ptr = MBM_GET_CTRL_TX_PTR(handle);              /* Control message */

        if( (MSG_TGT_INIC == cmsg_ptr->Tgt_Adr)
            && (FBLOCK_INIC == cmsg_ptr->FBlock_ID) )
        {
            success = PmsCompressHeaderICM(handle);                     /* INIC control message */
        }
        else
        {
            WAIT4MX(MX_PMS_CS);
            success = _PmsCompressHeaderMCM(handle, pmsSupportsTelId4); /* MOST Control Message, use MCM */
            REL_MX(MX_PMS_CS);
        }
    }
    #ifdef ADS_MIN /* Not needed for MDP_MIN, no TX */
    else
    {
        success = PmsCompressHeaderMDP(handle);                         /* MOST Data Packet / ADS */
    }
    #endif

    T_MOD_EXIT(PMS_12);

    return success;
}
#endif

#ifdef PMS_64
static bool PmsCompressHeaderICM(HMBMBUF handle)
{
    MbmCtrlTx *cmsg_ptr;
    byte      *tgt_ptr;
    word      len;
    byte      fph; 
    byte      pmhl;
    #ifdef PMS_USE_HANDLE
    byte      stuffing_bytes;
    byte      scf0;
    #endif

    T_MOD_ENTRY(PMS_64);

    /* Control message */
    cmsg_ptr        = MBM_GET_CTRL_TX_PTR(handle);
    #ifdef PMS_USE_HANDLE
    stuffing_bytes  = (byte)0;
    scf0            = (byte)0;             /* SCF0 for final message */
    #endif

    /* Local INIC, use ICM */
    fph = (byte)(PMS_FIFONO_ICM << FPH_B_FIFONO_LSB) | FPH_MSGTYPE_DATA;
    /*
    */
    if( !(pmsCnt++ & (byte)0x0F) )
    {
       fph |= (byte)0x80;
    }

    pmhl = (byte)1;                           /* only FPH */
    #ifdef PMS_USE_HANDLE
    /* N.B. SCF0, HANDLE and stuffing bytes will be used only if
     * we have to send the handle */
    if( PMS_DEF_HANDLE != handle->handle )
    {
        pmhl    = (byte)5;                    /* FPH + SCF0 + Handle + 2x Stuffing-Byte*/
        fph    |= FPH_M_SCF0_ENABLED;
        scf0   |= PMS_M_SCF0_HANDLE;
        stuffing_bytes = (byte)2;
    }
    #endif
    /*           PML + PMHL + PMH + (FktID, OpType, TelID, TelLen) */
    len = ( (word)2  +  (word)1  +  (word)pmhl  +  (word)4 );

    tgt_ptr = MbmPush(handle, len); /* align header */

    len -= (word)2;                 /* Subtract size of PML */

    len += (word)((cmsg_ptr->Length > (word)PMS_CTRL_MAX_PAYLOAD) ?
                  (word)PMS_CTRL_MAX_PAYLOAD : cmsg_ptr->Length);

    /* - start first quadlet - */
    *tgt_ptr++ = HB(len);
    *tgt_ptr++ = LB(len);
    *tgt_ptr++ = pmhl;
    *tgt_ptr++ = fph;
    /* - end first quadlet - */

    #ifdef PMS_USE_HANDLE
    /* - start second quadlet - */
    if( scf0 & PMS_M_SCF0_HANDLE )
    {
        *tgt_ptr++ = scf0;           /* SCF0 enabled */
        *tgt_ptr++ = handle->handle; /* byte Handle      */
    }

    for (; stuffing_bytes > 0; stuffing_bytes--)
    {
        *tgt_ptr++ = 0;
    }
    /* - end second quadlet - */
    #endif

    /* - start last quadlet - */
    *tgt_ptr++ = (byte)(cmsg_ptr->Func_ID >> 4);
    *tgt_ptr++ = (byte)((byte)(LB(cmsg_ptr->Func_ID) << 4)  | cmsg_ptr->Operation);

    len        = cmsg_ptr->Length;
    *tgt_ptr++ = HB(len) | (byte)(handle->tel_id << 4);
    *tgt_ptr++ = LB(len);
    /* - end last quadlet - */

    tgt_ptr    = cmsg_ptr->Data;                    /* Payload */

    MBM_SET_PAYLOAD_PTR(handle, tgt_ptr);
    MBM_SET_PAYLOAD_LEN(handle, len);
    T_MOD_EXIT(PMS_64);

    return MNS_TRUE;
}
#endif

#ifdef PMS_65
static bool _PmsCompressHeaderMCM(HMBMBUF handle, bool telId4Enabled)
{
    MbmCtrlTx *cmsg_ptr;
    byte      *tgt_ptr;

    word       body_data_len;
    byte       int_data_len;
    byte       fdh_len;

    byte       fph;                         /* FIFO Protocol Header */
    byte       scf0;                        /* SCF0 for final message */
    bool       success;

    T_MOD_ENTRY(PMS_65);

    cmsg_ptr    = MBM_GET_CTRL_TX_PTR(handle);
    success     = MNS_TRUE;

    fdh_len         = (byte)4;              /* incl. TgtDevID, FBlockID, InstID */
    int_data_len    = (byte)0;              /* MsgCnt, MsgSize */
    body_data_len   = (word)0;              /* data size */

    fph             = (byte)(PMS_FIFONO_MCM << FPH_B_FIFONO_LSB) | FPH_MSGTYPE_DATA;
    scf0            = (byte)0;             /* SCF0 for final message */

    if ( (pms.low_level_retries != cmsg_ptr->LowLevelRetries) || (pms.mid_level_retries != cmsg_ptr->MidLevelRetries))
    {
        scf0    |= PMS_M_SCF0_RETRY;
        fph     |= FPH_M_SCF0_ENABLED;
        fdh_len += (byte)2;
    }

    #ifdef PMS_USE_HANDLE
    if( PMS_DEF_HANDLE != handle->handle )
    {
        scf0    |= PMS_M_SCF0_HANDLE;
        fph     |= FPH_M_SCF0_ENABLED;
        fdh_len += (byte)1;
    }
    #endif

    if ((FPH_M_SCF0_ENABLED & fph) == FPH_M_SCF0_ENABLED)
    {
        fdh_len += (byte)1;
    }
    
    if ((word)PMS_CTRL_MAX_PAYLOAD < cmsg_ptr->Length)
    {
        #ifndef PMS_TX_NOSEG
        MBM_SET_STATUS_BITS(handle, MBM_STAT_TX_SEG);   /* segmented */

        if (MNS_FALSE != telId4Enabled)
        {
            handle->tel_id = PMS_TELID_4;
            int_data_len  += (byte)2;                   /* internally add MsgSize */
            body_data_len  = (word)0;
        }
        else
        {
            handle->tel_id = PMS_TELID_1;
            int_data_len++;                             /* internally add MsgCnt*/
            body_data_len  = (word)PMS_CTRL_MAX_PAYLOAD - (word)1;
        }
        #else
        success = MNS_FALSE;                            /* segmentation not supported */
        FAILED_ASSERT();
        #endif

        #if (defined PMS_APPENDABLE_PAYLOAD) && (defined PMS_APPENDABLE_SEG_CNT)
        if( handle->type & MBM_STAT_TX_APPEND )
        {
            success = MNS_FALSE;                        /* combination with appdbl payload not possible*/
            FAILED_ASSERT();
        }
        #endif
    }
    else
    {
        #if (defined PMS_APPENDABLE_PAYLOAD) && (defined PMS_APPENDABLE_SEG_CNT)
        if( handle->type & MBM_STAT_TX_APPEND )
        {
            int_data_len++;                             /* includes additional field SegCnt within reserved header */
            body_data_len = handle->append.payloadLen; /* handle->tel_id is already set */

            if ((body_data_len + (word)int_data_len) > (word)PMS_CTRL_MAX_PAYLOAD)
            {
                success = MNS_FALSE;
                FAILED_ASSERT();
            }
        }
        else
        #endif
        {
            body_data_len = cmsg_ptr->Length;
        }
    }
    
    if (MNS_FALSE != success)
    {
        /* calculate size of rsvd header */
        byte stuffing_bytes;
        word len = (word)PMS_PM_PRE_FDH_LEN + (word)fdh_len + (word)PMS_PM_INT_BODY_LEN + (word)int_data_len;
        
        stuffing_bytes = (byte)(PMS_ALIGN(len,(word)4) - len);      /* calculate stuffing bytes */
        tgt_ptr = MbmPush(handle, (word)(len+stuffing_bytes));      /* align header */

        len = (len + stuffing_bytes + body_data_len) - (word)2;                      /* calculate size of PML */

        /* - start first quadlet - */
        *tgt_ptr++ = HB(len);
        *tgt_ptr++ = LB(len);
        *tgt_ptr++ = (byte)1 + fdh_len + stuffing_bytes;            /* pmhl*/
        *tgt_ptr++ = fph;
        /* - end first quadlet - */

        if ((byte)0 != scf0)
        {
            *tgt_ptr++ = scf0;
        }

        #ifdef PMS_USE_HANDLE
        if( scf0 & PMS_M_SCF0_HANDLE )
        {
            *tgt_ptr++ = handle->handle;                            /* byte Handle */
        }
        #endif

        if( scf0 & PMS_M_SCF0_RETRY )
        {
            *tgt_ptr++ = cmsg_ptr->MidLevelRetries;                 /* word Retry */
            *tgt_ptr++ = cmsg_ptr->LowLevelRetries;
        }

        *tgt_ptr++ = HB(cmsg_ptr->Tgt_Adr);                         /* word TgtDeviceID */
        *tgt_ptr++ = LB(cmsg_ptr->Tgt_Adr);
        *tgt_ptr++ = cmsg_ptr->FBlock_ID;                           /* word FuncAddr */
        *tgt_ptr++ = cmsg_ptr->Inst_ID;

        for (;stuffing_bytes > (byte)0; stuffing_bytes--)
        {
            *tgt_ptr++ = (byte)0;
        }

        *tgt_ptr++ = (byte)(cmsg_ptr->Func_ID >> 4);
        *tgt_ptr++ = (byte)((byte)(LB(cmsg_ptr->Func_ID) << 4)  | cmsg_ptr->Operation);

        len = int_data_len + body_data_len;

        *tgt_ptr++ = HB(len) | (byte)(handle->tel_id << 4); /* Tel_ID = 0x0 or 0x1 */
        *tgt_ptr++ = LB(len);

        len = body_data_len;                                /* Restore Data Length */

        if(handle->type & MBM_STAT_TX_SEG)
        {
            if (PMS_TELID_4 == handle->tel_id)
            {
                *tgt_ptr++ = HB(cmsg_ptr->Length);          /* Message Size (TelID 0x4) */
                *tgt_ptr++ = LB(cmsg_ptr->Length);
            }
            else
            {
                *tgt_ptr++ = (byte)0x00;                    /* Message Counter */
            }
        }
        #if (defined PMS_APPENDABLE_PAYLOAD) && (defined PMS_APPENDABLE_SEG_CNT)
        else if (handle->type & MBM_STAT_TX_APPEND)
        {
            *tgt_ptr++ = handle->append.segCnt;             /* Message Counter */
            len = cmsg_ptr->Length;                         /* Restore Data Length */
        }
        #endif

        tgt_ptr = cmsg_ptr->Data;                           /* Payload */

        MBM_SET_PAYLOAD_PTR(handle, tgt_ptr);
        MBM_SET_PAYLOAD_LEN(handle, len);
    }

    T_MOD_EXIT(PMS_65);
    
    return success;
}
#endif

#ifdef PMS_66
static bool PmsCompressHeaderMDP(HMBMBUF handle)
{
    MbmDataTx *dmsg_ptr;
    byte      *tgt_ptr;
    word      len;
    byte      fph;              /* FIFO Protocol Header */
    bool      success;

    T_MOD_ENTRY(PMS_66);

    success = MNS_TRUE;
    /* Asynchronous packet */
    dmsg_ptr = MBM_GET_DATA_TX_PTR(handle);

    fph = (byte)(PMS_FIFONO_MDP << FPH_B_FIFONO_LSB) | FPH_MSGTYPE_DATA;
    /* No SCFs allowed, only default */

    /* header length = PML + PMHL + *PMHL + Length = 10 */
    tgt_ptr = MbmPush(handle, (word)PMS_DATA_HDR_MAX_SIZE);

    /*
    ------------------------------------------------------------------------
        Start filling message
    ------------------------------------------------------------------------
    */
    #ifdef PMS_APPENDABLE_PAYLOAD
    if(handle->type & MBM_STAT_TX_APPEND)
    {
        len = (word)( (word)(PMS_DATA_HDR_MAX_SIZE-2)
                + dmsg_ptr->Length                          /* the port message includes */
                + handle->append.payloadLen);              /* additional external payload */
    }
    else
    #endif
    {
        len = (word)( (word)(PMS_DATA_HDR_MAX_SIZE-2)
                        + dmsg_ptr->Length);                /* Header is static 8 */
    }

    *tgt_ptr++ = HB(len);                                   /* PML */
    *tgt_ptr++ = LB(len);
    *tgt_ptr++ = (byte)5;                                   /* Static PMHL */
    *tgt_ptr++ = fph;

    #ifdef _OS81110_PCK_LLR
                                                            /* byte Retry */
                                                            /* byte Priority */
    *tgt_ptr++ = (byte)((dmsg_ptr->Retry & (byte)0x0F) << 4) | (PMS_DEF_PRIO & (byte)0x0F);
    #else
    *tgt_ptr++ = PMS_DEF_PRIO;                              /* byte Priority */
    #endif
    *tgt_ptr++ = dmsg_ptr->Tgt_Adr_H;                       /* word TgtDeviceID */
    *tgt_ptr++ = dmsg_ptr->Tgt_Adr_L;
    *tgt_ptr++ = (byte)0x00;                                /* Static filler */

    #ifdef PMS_APPENDABLE_PAYLOAD
    if(handle->type & MBM_STAT_TX_APPEND)
    {
        len = dmsg_ptr->Length + handle->append.payloadLen;
    }
    else
    #endif
    {
        len = dmsg_ptr->Length;
    }

    *tgt_ptr++ = HB(len);
    *tgt_ptr++ = LB(len);

    if( &i2c == mdpFifo.iface_ptr )
    {
        if( len > (word)PMS_I2C_DATA_MAX_PAYLOAD )
        {
            success = MNS_FALSE;
            FAILED_ASSERT();
        }
    }
    else if( len > (word)PMS_DATA_MAX_PAYLOAD )
    {
        success = MNS_FALSE;
        FAILED_ASSERT();
    }

    #ifdef PMS_APPENDABLE_PAYLOAD
    if( handle->type & MBM_STAT_TX_APPEND )
    {
        len = dmsg_ptr->Length;             /* restore the correct user payload length */
                                            /* skip length checks */
    }
    #endif

    tgt_ptr    = dmsg_ptr->Data;            /* Payload */

    MBM_SET_PAYLOAD_PTR(handle, tgt_ptr);
    MBM_SET_PAYLOAD_LEN(handle, len);
    T_MOD_EXIT(PMS_66);

    return success;
}
#endif

#ifdef PMS_13
static void PmsDecompressHeader(HMBMBUF handle, const PmsFifoDefCfg *def_ptr)
{
    MbmMsgUnion          *msg_ptr;
    word                  len;
    #if ((defined ADS_MIN) && (!defined PMS_DISABLE_ADS_LEN_CHECKS)) || (!defined PMS_DISABLE_CTRL_LEN_CHECKS)
    word                  pml;
    #endif
    byte                 *src_ptr;

    T_MOD_ENTRY(PMS_13);
    msg_ptr = &handle->msg;
    len = (word)0;

    #if ((defined ADS_MIN) && (!defined PMS_DISABLE_ADS_LEN_CHECKS)) || (!defined PMS_DISABLE_CTRL_LEN_CHECKS)
    DECODE_WORD(&pml, MBM_GET_HDR_PTR(handle));                       /* PML */
    #endif

    src_ptr = MBM_GET_HDR_PTR(handle) + FPH_INDEX;                    /* => FPH   */

    if( def_ptr )
    {
        /* Control Message (ICM/MCM) */
        #ifndef PMS_DISABLE_CTRL_LEN_CHECKS
        byte                  pmhl;
        #endif
        byte                  scf0;
        byte                  scf1;
        MBM_SET_TYPE(handle, MBM_TYPE_CTRL_RX);

        /*!
         * Check SCF0 enabled bit in FPH and move to next, if bit was
         * set add the next byte as SCF0 and move to next
         */
        scf0 = (FPH_M_SCF0_ENABLED & *src_ptr++) ?
               *src_ptr++ : def_ptr->rxScf0Default;

        /* If SCF0 had SCF1 enabled then add this as well and move on */
        scf1 = (PMS_M_SCF0_SCF1_EN & scf0) ?
               *src_ptr++ : def_ptr->rxScf1Default;

        if( scf0 & PMS_M_SCF0_TIMESTAMP )
        {
            src_ptr += 4;   /* Ignore timestamp, no destination field */
        }

        #ifdef PMS_USE_HANDLE
        handle->handle = (scf0 & PMS_M_SCF0_HANDLE) ?
                         *src_ptr++ : PMS_DEF_HANDLE;
        #else
        if( scf0 & PMS_M_SCF0_HANDLE )
        {
            src_ptr++; /* Skip */
        }
        #endif

        msg_ptr->msgRx.Rcv_Type = (scf1 & PMS_M_SCF1_TGTDEVTYPE) ?
                                  *src_ptr++ : PMS_DEF_TGTDEVTYPE;

        #ifdef CTRL_FILTER_ID
        msg_ptr->msgRx.Filter_ID = (byte)0x00;
        #endif

        if( scf1 & PMS_M_SCF1_SRCDEVID )
        {
            DECODE_WORD(&msg_ptr->msgRx.Src_Adr, src_ptr);
            src_ptr += 2;
        }
        else
        {
            msg_ptr->msgRx.Src_Adr = PMS_DEF_SRCADDR;
        }

        if( scf1 & PMS_M_SCF1_FUNCADDR )
        {
            msg_ptr->msgRx.FBlock_ID = *src_ptr++;
            msg_ptr->msgRx.Inst_ID   = *src_ptr++;
        }
        else
        {
            msg_ptr->msgRx.FBlock_ID = PMS_DEF_FBLOCK_ID;
            msg_ptr->msgRx.Inst_ID   = PMS_DEF_INST_ID;
        }

        PMS_SKIP_HEADER(src_ptr, handle); /* Skips any fillers */

        /* Decode mandatory fields */
        DECODE_WORD(&msg_ptr->msgRx.Func_ID, src_ptr);    /* Func_ID */
        msg_ptr->msgRx.Func_ID >>= 4;                     /* Shift out Operation */
        src_ptr++;                                        /* Operation in Low Nibble */

        msg_ptr->msgRx.Operation = *src_ptr++ & ((byte) 0x0F); /* Operation */

        DECODE_WORD(&len, src_ptr);                       /* Length (+TelId) */
        src_ptr += 2;

        /* Always pass on a valid Data pointer, even without payload */
        msg_ptr->msgRx.Data = (src_ptr < (handle->start_ptr + handle->size))
                              ? src_ptr : &mbmMemArray[0];

        handle->tel_id = HB(len) >> 4;
        len &= (word)0x0FFF;                              /* Payload length */
        msg_ptr->msgRx.Length = len;

        #ifndef PMS_DISABLE_CTRL_LEN_CHECKS
        pmhl = MBM_GET_HDR_PTR(handle)[2];
        /*  PML != SZ(PMHL) +   PMHL      +  SZ(FuncId_OpType,TelID_TelLen) + TelLen */
        if (pml != ((word)1 +  (word)pmhl + (word)4                         + len))
        {
            msg_ptr->msgRx.Length = PMS_INVALID_PM_LENGTH;
        }
        #endif
    }
    #ifdef ADS_MIN /* Not needed with only MDP_MIN, message will be discarded */
    else
    {
        /* Async Packet (MDP) */
        MBM_SET_TYPE(handle, MBM_TYPE_DATA_RX);

        /*!
         * Header is currently static
         */

        src_ptr++;                        /* Skip FPH */
        msg_ptr->dataRx.Rcv_Type  = *src_ptr++;
        msg_ptr->dataRx.Src_Adr_H = *src_ptr++;
        msg_ptr->dataRx.Src_Adr_L = *src_ptr++;

        PMS_SKIP_HEADER(src_ptr, handle); /* Skips any fillers  */

        DECODE_WORD(&len, src_ptr);       /* Length             */
        src_ptr += (word)2;               /* Skips length field */
        #ifndef PMS_DISABLE_ADS_LEN_CHECKS
        if (pml == (len + (word)8))       /* plausibility check for a correct MDP message */
        {                                 /* pml must be exactly the data body length + 8 bytes(PMH+Length) */
            msg_ptr->dataRx.Length = len;
        }
        else
        {
            msg_ptr->dataRx.Length = PMS_INVALID_PM_LENGTH;
        }
        #else
        msg_ptr->dataRx.Length = len;
        #endif

        /* Always pass on a valid Data pointer, even without payload */
        msg_ptr->dataRx.Data   = (src_ptr < (handle->start_ptr + handle->size))
                                ? src_ptr : &mbmMemArray[0];
    }
    #endif

    MBM_SET_PAYLOAD_PTR(handle, src_ptr);
    MBM_SET_PAYLOAD_LEN(handle, len);
    MBM_SET_HDR_LEN(handle, (word)(src_ptr - MBM_GET_HDR_PTR(handle)));

    T_MOD_EXIT(PMS_13);
}
#endif

#ifdef PMS_15
static PmsFifo *PmsGetTxFifo(HMBMBUF tx_handle)
{
    PmsFifo *fifo_ptr;
    MbmCtrlTx *cmsg_ptr;

    T_MOD_ENTRY(PMS_15);

    cmsg_ptr = MBM_GET_CTRL_TX_PTR(tx_handle);

  #ifdef ADS_MIN /* Not needed for MDP_MIN only, no TX */
    if( MBM_TYPE_DATA == (MBM_GET_TYPE(tx_handle) & MBM_TYPE_MSG_MASK) )
    {
        fifo_ptr = &mdpFifo;
    }
    else
  #else
    /* Only control messages are allowed */
    ASSERT(MBM_TYPE_CTRL == (MBM_GET_TYPE(tx_handle) & MBM_TYPE_MSG_MASK));
  #endif
    if( (MSG_TGT_INIC == cmsg_ptr->Tgt_Adr) && (FBLOCK_INIC == cmsg_ptr->FBlock_ID) )
    {
        fifo_ptr = &icmFifo;
    }
    else
    {
        fifo_ptr = &mcmFifo;
    }

    T_MOD_EXIT(PMS_15);

    return( fifo_ptr );
}
#endif

#ifdef PMS_53
static PmsFifo *PmsGetRxFifo(HCMBMBUF rx_handle)
{
    PmsFifo *fifo_ptr;

    T_MOD_ENTRY(PMS_53);

    fifo_ptr = NULL;

    switch( rx_handle->hdr_ptr[FPH_INDEX] & FPH_M_FIFONO )
    {
        case ((byte)(PMS_FIFONO_ICM << FPH_B_FIFONO_LSB)):
            fifo_ptr = &icmFifo;
            break;

        case ((byte)(PMS_FIFONO_MCM << FPH_B_FIFONO_LSB)):
            fifo_ptr = &mcmFifo;
            break;

        #ifdef MDP_MIN
        case ((byte)(PMS_FIFONO_MDP << FPH_B_FIFONO_LSB)):
            fifo_ptr = &mdpFifo;
            break;
        #endif

        #if (!defined MEP_MIN) && (defined _OS81110_MEP)
        case ((byte)(PMS_FIFONO_MEP << FPH_B_FIFONO_LSB)):
            /* fifo_ptr = NULL; */
            /* MEP is not supported by the LLD */
            break;
        #endif

        default:
            FAILED_ASSERT(); /* Unexpected/garbled message */
            break; /*lint !e527 Unreachable code when linting */
    }

    T_MOD_EXIT(PMS_53);

    return( fifo_ptr );
}
#endif

#ifdef PMS_38
/*
--------------------------------------------------------------------------------
 Function:      PmsFreeShadow(...)

 Description:   Frees a shadow

 Input(s):      Pointer to shadow

 Return(s):     -
--------------------------------------------------------------------------------
*/
static _INLINE void _PmsFreeShadow(MbmShadow *shadow_ptr)
{
    T_MOD_ENTRY(PMS_38);

    ASSERT(shadow_ptr);

    _MbmEnqueue(&txSegShadowQ, (HMBMBUF)shadow_ptr);
    if( pms.needShadow )
    {
        pms.needShadow = 0;
        PmsSetPendingEvent(PMS_P_TX_TRIG);
    }
    T_MOD_EXIT(PMS_38);
}
#endif

#ifdef PMS_17
static void PmsFifoTxTrigger(PmsFifo *fifo_ptr, MbmQueue *q_ptr)
{
    PmsIface *iface_ptr;
    bool      ready;
    byte *add_ptr;
    word  add_len;

    T_MOD_ENTRY(PMS_17);

    add_ptr = NULL;
    add_len = (word)0;

    ASSERT(fifo_ptr);
    ASSERT(q_ptr);

    WAIT4MX(MX_PMS_CS);
    /* Store interface in case we're preempted by an interface change */
    iface_ptr = fifo_ptr->iface_ptr;
    ready     = PMS_IFACE_READY(*iface_ptr) ? MNS_TRUE : MNS_FALSE;
    REL_MX(MX_PMS_CS);

    /* First check if the Interface might accept this */
    if( MNS_FALSE != ready )
    {
        HMBMBUF handle;

        /*
         * If it's the message Q, make sure it is possible to send, if it is
         * the command Q, just try to dequeue the frontmost message.
         */
        if( (&fifo_ptr->msgQ == q_ptr) && (!fifo_ptr->txSlotsAvailable
           || (PMS_S_SYNCED != pmsSyncState) || MBM_QUEUE_LENGTH(&fifo_ptr->cmdQ)) )
        {
            handle = NULL;
        }
        else
        {
            handle = MbmDequeue(q_ptr);

            #ifndef PMS_TX_NOSEG

            if( (NULL != handle) && (MBM_STAT_TX_SEG == (MBM_STAT_TX_SEG & MBM_GET_STATUS(handle))) )
            {
                /*
                 * Do not send a segment if there will be no shadow available
                 * or start a new segmented message if there is one ongoing.
                 */
                if( (word)0 == PMS_CHECK_SHADOW() )
                {
                    MbmEnqueueFirst(q_ptr, handle);
                    handle = NULL;
                    pms.needShadow = 1;
                }
            }
            #endif

            #ifdef AMS_TX_OUT_FILTER
            if ((&mcmFifo.msgQ == q_ptr) && (NULL != handle))
            {
                if ((MSG_TGT_INIC != handle->msg.msgTx.Tgt_Adr) && (NULL != pmsInitData_ptr->cbTxOutFilter_fptr))
                {
                    if ( MSG_TX_FILTER_CANCEL == pmsInitData_ptr->cbTxOutFilter_fptr(MBM_GET_CTRL_TX_PTR(handle)) )
                    {
                        byte result;                                        /* abort transmission */
                        byte (*cbTxStatus_fptr)(struct MbmBuf *, byte);

                        cbTxStatus_fptr = handle->cbTxStatus_fptr;
                        result = cbTxStatus_fptr(handle, XMIT_TX_FILTER);   /* call tx status handler */

                        if (MSG_TX_RETRY != result)
                        {
                            #ifndef PMS_TX_NOSEG
                            if( MBM_STAT_TX_SEG == (MBM_STAT_TX_SEG & MBM_GET_STATUS(handle)) )
                            {
                                WAIT4MX(MX_PMS_Q);
                                _PmsSetReferencesToNull(handle);  /* cleanup shadows pointing at this handle */
                                REL_MX(MX_PMS_Q);
                            }
                            #endif
                            handle = NULL;      /* do not forward message - buffer is handled outside PMS */
                        }
                        else
                        {
                            FAILED_ASSERT();    /* MSG_TX_RETRY shall not be applied on filtered messages */
                        }

                        if((word)0 != MBM_QUEUE_LENGTH(&rxStatusQ))
                        {
                            PmsSetPendingEvent(PMS_P_TX_TRIG);  /* since there is no fifo status, we need to retrigger now */
                        }
                    }
                }
            }
            #endif
        }


        if( handle )
        {
            /* Check & possibly get extended payload */
            #ifdef PMS_APPENDABLE_PAYLOAD
            if(handle->type & MBM_STAT_TX_APPEND)
            {
                add_len = handle->append.payloadLen;
                add_ptr = handle->append.payloadPtr;
                ASSERT(0 == MbmGetExtPayloadLen(handle)); /* external payload is not allowed */
            }
            else
            #endif
            {
                add_len = MbmGetExtPayloadLen(handle);
                add_ptr = add_len ? MBM_GET_PAYLOAD_PTR(handle) : NULL;
            }

            /*
             * To be able to handle the both the case of an asynchronous LLD that
             * will call PmsTxRelease from another thread and a synchronous call
             * that may call it from within the actual TX function we need to set
             * everything up before calling TX and then perform a rollback if it
             * reports false (e.g. busy LLD).
             */

            iface_ptr->active   = handle;

            if( iface_ptr->tx_fptr((void *)handle, add_ptr, add_len) )
            {
                PMS_LOG_TX(handle->hdr_ptr); /* N.B. Payload will be invalid if add_ptr is used */

                if( &fifo_ptr->msgQ == q_ptr )
                {
                    #ifdef PMS_NO_SLOTNTF
                    /* Work only with 0 or 1 slot at a time */
                    fifo_ptr->txSlotsAvailable = 0;
                    #else
                    fifo_ptr->txSlotsAvailable--;
                    #endif

                    /*
                     * Only start the timer if it's not already running, it is only
                     * necessary to run it on the message at the head of the queue
                     */
                    if( (word)0 == MostGetTimer(&fifo_ptr->timer) )
                    {
                        #ifndef PMS_DISABLE_MCM_EXT_TIMEOUT
                        if ( (&mcmFifo == fifo_ptr) && (PMS_CFG_MCM_MLR_THRESHOLD < handle->msg.msgTx.MidLevelRetries) )
                        {
                            MostSetTimer(&fifo_ptr->timer, PMS_CFG_MCM_MAX_TIMEOUT, MNS_FALSE);
                        }
                        else
                        #endif
                        {
                            MostSetTimer(&fifo_ptr->timer, fifo_ptr->timeout, MNS_FALSE);
                        }
                    }
                }
            }
            else
            {
                /* LLD returned false, rollback settings for TX */
                iface_ptr->active   = NULL;      /* Unhook this from being the active one */

                MbmEnqueueFirst(q_ptr, handle);

                #ifdef PMS_TX_FORCE_TRIG
                PmsSetPendingEvent(PMS_P_TX_TRIG);
                #else
                /*
                No forced retrigger on LLD busy, this will be retriggered on
                completed transmissions (and possibly rx, since I2C is half
                duplex)
                */
                #endif
            }
        }
    }
    T_MOD_EXIT(PMS_17);
}
#endif

#ifdef PMS_40
static bool PmsProcessRxMsgOut(HMBMBUF handle, const PmsFifo *fifo_ptr)
{
    bool return_code;

    T_MOD_ENTRY(PMS_40);

    return_code = MNS_TRUE;

    ASSERT(fifo_ptr);

    #ifdef MSG_RX_USER_PAYLOAD
    if( &mcmFifo == fifo_ptr )
    {
        /*
         * Process the waiting Q if no handle was supplied or if this message does not
         * already have user allocated payload (segmented)
         */
        if( !handle || !HAS_RX_USER_PAYLOAD(handle) )
        {
            if( MBM_QUEUE_LENGTH(&pms.rxPayloadWaitingQ) )
            {
                if( handle )
                {
                    /* Enqueue this message, it is not allowed to overtake waiting */
                    MbmEnqueue(&pms.rxPayloadWaitingQ, handle);
                }
                handle = MbmDequeue(&pms.rxPayloadWaitingQ);
            }

            if( handle )
            {
                MbmCtrlRx *rx_ptr = MBM_GET_CTRL_RX_PTR(handle);
                word size = (word)0;
                byte *data_ptr = rx_ptr->Data;

                if( rx_ptr->Length )
                {
                    ASSERT(data_ptr);

                    rx_ptr->Data = NULL; /* New payload needed */
                    size = pmsInitData_ptr->cbGetPayloadBuf_fptr(rx_ptr, rx_ptr->Length);
                    ASSERT(rx_ptr->Length);     /* Length should not be changed */

                    if( size )
                    {
                        ASSERT(rx_ptr->Data);   /* Has to be set if allocation returned non-zero */
                        MNS_MEM_CPY(rx_ptr->Data, data_ptr, rx_ptr->Length);

                        #ifdef MSG_TX_USER_PAYLOAD
                        PmsFreeUserTxPayload(data_ptr, handle);
                        #endif
                    }
                    else
                    {
                        rx_ptr->Data = data_ptr; /* Restore in case it was external */
                    }
                }
                else
                {
                    /* Possible special case: External payload pointer, but 0-length payload */
                    #ifdef MSG_TX_USER_PAYLOAD
                    PmsFreeUserTxPayload(data_ptr, handle);
                    #endif
                    rx_ptr->Data = &mbmMemArray[0];
                }

                if( size || !rx_ptr->Length )
                {

                    MbmMemFree(handle);     /* Nullifies internal pointers also */
                    handle->size = size;    /* size with NULL as start_ptr indicates user payload */
                }
                else /* Wait for PmsSetExternalBufferAvailable() */
                {
                    MbmEnqueueFirst(&pms.rxPayloadWaitingQ, handle);
                    handle = NULL;
                    return_code = MNS_FALSE;
                }
            }
        }
    }
    #endif

    if( handle )
    {
        PmsRxHandler cbRx_fptr;

        if( &mcmFifo == fifo_ptr )
        {
            cbRx_fptr = pmsInitData_ptr->cbMcmRx_fptr;
        }
        #ifdef MDP_MIN
        else if( &mdpFifo == fifo_ptr )
        {
            #ifdef ADS_MIN    /* Callback only needed with ADS */
            cbRx_fptr = pmsInitData_ptr->cbMdpRx_fptr;
            #else
            cbRx_fptr = NULL; /* Discard */
            #endif
        }
        #endif
        else
        {
            cbRx_fptr = pmsInitData_ptr->cbIcmRx_fptr;
        }

        if( cbRx_fptr )
        {
            cbRx_fptr(handle);
        }
        else
        {
            MbmFree(handle);    /* No RX handler, discard */
        }
    }
    T_MOD_EXIT(PMS_40);

    return( return_code );
}
#endif

#ifdef PMS_39
static void PmsProcessCompressedRxMsg(HMBMBUF handle)
{
    PmsFifo *fifo_ptr;
    _CONST PmsFifoDefCfg *def_ptr;
    word index;

    T_MOD_ENTRY(PMS_39);

    fifo_ptr = PmsGetRxFifo(handle);
    def_ptr = NULL;
    index = (word)0;

    if( fifo_ptr
    #ifdef PMS_RX_SEND_ACK
        && ((&icmFifo == fifo_ptr) || (PMS_STATUS_NONE == fifo_ptr->rxStatus))
    #endif
        )
    {
        /* Get the FIFO default values */
        if( &icmFifo == fifo_ptr )
        {
            def_ptr = &icmDefCfg;
            index   = PMS_IDX_ICM_RX_PREALLOC;
        }
        else if( &mcmFifo == fifo_ptr )
        {
            def_ptr = &mcmDefCfg;
            #ifndef PMS_RX_OPT3
            index   = PMS_IDX_MCM_RX_PREALLOC;
            #endif
        }
        #ifdef ADS_MIN  /* Not needed for MDP_MIN, does not use prealloc */
        else if( &mdpFifo == fifo_ptr )
        {
            /* MDP has static values and thus NULL as def_ptr */
            #ifndef PMS_RX_OPT3
            index   = PMS_IDX_MDP_RX_PREALLOC;
            #endif
        }
        #endif

        /*
         * Possibly replace the default preallocated RX buffer in case we are
         * running low - this will prevent acknowledging this FIFO as ready
         * for another message from the INIC.
         */
        WAIT4MX(MX_PMS_CS);
        if( (NULL == pms.rxPreAllocPtrTab[PMS_IDX_RSVD_RX_PREALLOC]) && index )
        {
            pms.rxPreAllocPtrTab[PMS_IDX_RSVD_RX_PREALLOC] = pms.rxPreAllocPtrTab[index];
            pms.rxPreAllocPtrTab[index] = NULL;
            pms.needPrealloc = 1;
        }
        REL_MX(MX_PMS_CS);

        PmsDecompressHeader(handle, def_ptr);

        /* Set the status response to the INIC */
        #ifdef PMS_RX_SEND_ACK
        #ifdef ADS_MIN /* No check for only MDP_MIN, just ack */
        /* Unless it is MDP over MLB */
        if( (&mdpFifo != fifo_ptr) || (&generalData != mdpFifo.iface_ptr) )
        #endif
        {
            fifo_ptr->rxStatus = PMS_M_STAT_SLOTAV | PMS_XMIT_SUCCESS;
        }
        #endif

        if( (&mcmFifo == fifo_ptr) || (&icmFifo == fifo_ptr))
        {
            #ifndef PMS_DISABLE_CTRL_LEN_CHECKS
            TMsgRx *msg_ptr = MBM_GET_CTRL_RX_PTR(handle);
            ASSERT(NULL != msg_ptr);

            if (PMS_INVALID_PM_LENGTH == msg_ptr->Length)
            {
                MbmFree(handle);                            /* discard the message if length checks */
                handle = NULL;                              /* in PmsDecompressHeader() failed */
                FAILED_ASSERT();
            }
            #endif

            if ((&mcmFifo == fifo_ptr) && (NULL != handle))           /* segmented */
            {
                #ifndef PMS_RX_NOSEG
                handle = PmsRxSegProcess(handle);
                #else
                if( handle->tel_id )
                {
                    /* No segmentation support */
                    if (handle->tel_id < PMS_TELID_4)
                    {
                        mcmFifo.rxStatus = PMS_M_STAT_SLOTAV | PMS_XMIT_INT_FMAT_FAILURE;
                    }
                    MbmFree(handle);
                    handle = NULL;
                }
                #endif
            }
        }
        #if (defined ADS_MIN) && (!defined PMS_DISABLE_ADS_LEN_CHECKS)
        else if( &mdpFifo == fifo_ptr )
        {
            TDataRx *msg_ptr = MBM_GET_DATA_RX_PTR(handle);

            if (PMS_INVALID_PM_LENGTH == msg_ptr->Length)
            {
                MbmFree(handle);                            /* discard the message if length checks */
                handle = NULL;                              /* in PmsDecompressHeader() failed */
                FAILED_ASSERT();
            }
        }
        #endif

        if( NULL != handle )
        {
            if( &mcmFifo == fifo_ptr )
            {
                MisFilterMostMsg(handle);
            }

            (void)PmsProcessRxMsgOut(handle, fifo_ptr);
            /*
             * Hand to higher layer or waiting Q, do not care if retriggering is needed here,
             * this have to be done in the mainloop triggered by PmsSetExternalBufferAvailable()
             */
        }
    }
    else
    {
        /* Unrecognized destination FIFO */
        MbmFree(handle);
        handle = NULL;
    }

    #ifdef PMS_RX_SEND_ACK     /* no need to call MbmFree(NULL) in order to start a timer etc. */
    PmsSetPendingEvent(PMS_P_RX_ACK);
    #endif

    T_MOD_EXIT(PMS_39);
}
#endif

#ifdef PMS_18
static void PmsProcessRxMsgQueue(void)
{
    HMBMBUF handle;
    word burst_num;

    T_MOD_ENTRY(PMS_18);

    burst_num = pmsInitData_ptr->rx_burst;

    while( burst_num-- > (word)0 )
    {
        handle = MbmDequeue(&rxQ);

        if( handle )
        {
            /* Process message */
            PmsProcessCompressedRxMsg(handle);
        }
        else
        {
            burst_num = (word)0;
        }
    }

    if( MBM_QUEUE_LENGTH(&rxQ) )
    {
        PmsSetPendingEvent(PMS_P_RX);
    }
    T_MOD_EXIT(PMS_18);
}
#endif

#ifdef PMS_30
static void PmsProcessRxStatusQueue(void)
{
    byte *src_ptr;
    byte fifomask;
    byte pmhl;

    #ifndef PMS_TX_NOSEG
    bool wait_for_tx_release;
    #endif

    T_MOD_ENTRY(PMS_30);

    #ifndef PMS_TX_NOSEG
    /*
     * Process status only when a segmented MCM isn't in the LLD to prevent
     * concurrency issues.
     */
    WAIT4MX(MX_PMS_CS);
    /*
     * Do not process status while any interface is busy to prevent processing one that
     * handed over by the LLD before the matching TX message has been released.
     */
    #ifdef ADS_MIN
    wait_for_tx_release = (mcmFifo.iface_ptr->active || icmFifo.iface_ptr->active
                          || ((&i2c == mdpFifo.iface_ptr) && (mdpFifo.iface_ptr->active))) ? MNS_TRUE : MNS_FALSE;
    #else
    wait_for_tx_release = (mcmFifo.iface_ptr->active || icmFifo.iface_ptr->active) ? MNS_TRUE : MNS_FALSE;
    #endif
    REL_MX(MX_PMS_CS);
    if( MNS_FALSE == wait_for_tx_release )
    #endif
    {
        /*!
         * Process all received Status to avoid unnecessary delays in TX or
         * timeouts.
         */
        bool finished = MNS_FALSE;
        do
        {
            HMBMBUF handle = MbmDequeue(&rxStatusQ);
            if( NULL != handle )
            {
                src_ptr = &MBM_GET_HDR_PTR(handle)[2];  /* => PMHL */
                pmhl = *src_ptr++;                      /* => FPH  */

                /*!
                 * Translate FIFONO into mask, so we can propagate to all affected
                 * FIFOs (Only SyncS can actually be propagated to multiple).
                 */
                fifomask = fifoNoToMaskTab[(*src_ptr >> FPH_B_FIFONO_LSB) & (byte)0x03];

                MBM_SET_TYPE(handle, MBM_TYPE_STATUS_RX);

                /*
                 * Decode any handle and set to Status
                 */
                if( *src_ptr & FPH_M_SCF0_ENABLED )
                {
                    #ifdef PMS_USE_HANDLE
                    handle->handle = src_ptr[2];
                    #endif
                    src_ptr += 3;
                }
                else
                {
                    src_ptr++;
                }

                /*
                 * Ignore unexpected SyncS.Cmd, i.e. in state SYNCED
                 * => (Process if not SyncS.Cmd or not in state SYNCED)
                 */
                if( ((PMS_M_STAT_SYNCS | PMS_SYNCS_SYNC_CMD) != (*src_ptr & (PMS_M_STAT_SYNCS | PMS_M_SYNCS)))
                    || (PMS_S_SYNCED != pmsSyncState) )
                {
                    if( PMS_M_FIFO_ICM & fifomask )
                    {
                        PmsFifoProcessStatus(&icmFifo, *src_ptr, &handle);
                    }

                    if( (PMS_M_FIFO_MCM & fifomask) && handle )
                    {
                        PmsFifoProcessStatus(&mcmFifo, *src_ptr, &handle);

                        /* If a SyncS is received every enqueued message shall be removed */
                        if ( PMS_M_STAT_SYNCS & *src_ptr )
                        {
                            while ( MbmQueueLength(&mcmFifo.pendingQ) )
                            {
                                PmsFifoProcessStatus(&mcmFifo, *src_ptr, &handle);
                            }
                            #ifdef PMS_61
                            PmsTxSyncSegmented();
                            #endif
                        }
                    }

                    #ifdef ADS_MIN /* Not needed with MDP_MIN only, no TX */
                    if( (PMS_M_FIFO_MDP & fifomask) && handle )
                    {
                        PmsFifoProcessStatus(&mdpFifo, *src_ptr, &handle);
                    }
                    #endif

                    if( (PMS_M_STAT_SYNCS & *src_ptr) && handle )
                    {
                        pmhl = (pmhl + (byte)PMS_PML_LEN + (byte)PMS_PMHL_LEN)
                                - (byte)((src_ptr - MBM_GET_HDR_PTR(handle)));
                        PmsSyncIn(src_ptr, pmhl);
                    }
                }

                if( handle )
                {
                    MbmFree(handle); /* Free if not reused */
                }
            }
            else
            {
                finished = MNS_TRUE;
            }
        } while( MNS_FALSE == finished );
    }

    T_MOD_EXIT(PMS_30);
}
#endif



#ifdef PMS_36
/*
--------------------------------------------------------------------------------
 Function:      PmsSetReferencesToNull(...)

 Description:   Searches through the queue for shadow referencing the supplied
                original and replaces the reference with NULL.

 Input(s):      org_ptr   : The original buffer

 Return(s):     -
--------------------------------------------------------------------------------
*/
static _INLINE void _PmsSetReferencesToNull(const MbmBuf *org_ptr)
{
    MbmShadow *search_ptr;

    T_MOD_ENTRY(PMS_36);

    search_ptr = (MbmShadow *)mcmFifo.pendingQ.next_ptr;

    ASSERT(org_ptr);

    while( (MbmShadow *)&mcmFifo.pendingQ != search_ptr )
    {
        if( MBM_GET_TYPE(search_ptr) & MBM_TYPE_SHADOW )
        {
            if( search_ptr->real_ptr == org_ptr )
            {
                search_ptr->real_ptr = NULL;
            }
        }
        search_ptr = (MbmShadow *)search_ptr->next_ptr;
    }
    T_MOD_EXIT(PMS_36);
}
#endif

#ifdef PMS_37
/*
--------------------------------------------------------------------------------
 Function:      PmsRespondCommand(...)

 Description:   Converts the supplied pointer to a status message handle into
                a command, sets the supplied command and sends it.

 Input(s):      fifo_ptr : target fifo
                h_ptr    : pointer to the handle to the original status message
                cmd      : desired command to send.

 Return(s):     -
--------------------------------------------------------------------------------
*/
static void PmsRespondCommand(PmsFifo *fifo_ptr, HMBMBUF *h_ptr, byte cmd)
{
    T_MOD_ENTRY(PMS_37);
    (void)MbmChangeType(*h_ptr, (word)MBM_TYPE_CMD_TX);
    (*h_ptr)->msg.ctrl = cmd;
    PmsSendCommand(fifo_ptr, *h_ptr);
    *h_ptr = NULL;                          /* Used */
    T_MOD_EXIT(PMS_37);
}
#endif

#ifdef PMS_19
static void PmsFifoProcessStatus(PmsFifo *fifo_ptr, byte status, HMBMBUF *h_ptr)
{
    #ifndef PMS_NO_SLOTNTF
    byte slots;
    #endif

    T_MOD_ENTRY(PMS_19);

    #ifndef PMS_NO_SLOTNTF
    slots = (&mcmFifo == fifo_ptr) ? PMS_MCM_SLOTS : (byte)1;
    #endif

    ASSERT(fifo_ptr);

    if( IS_COMPLETE_STATUS(status) )
    {
        bool ignore_status = MNS_FALSE;
        HMBMBUF txPending = MbmDequeue(&fifo_ptr->pendingQ);

        if( txPending )
        {
            byte cmd    = (byte)0;            /* Possible command to the INIC */
            #ifndef PMS_TX_NOSEG
            /* Segmented messages have "shadows" enqueued since MCM is double buffered and
             * hence the same original message can be enqueued twice in the INIC.
             */
            MbmShadow *shadow_ptr;
            MbmBuf    *prev_ptr = NULL;

            /* Shadows are used to reference the original for segmented messages */
            if( MBM_GET_TYPE(txPending) & MBM_TYPE_SHADOW )
            {
                shadow_ptr = (MbmShadow *)txPending;

                /* Get original */
                txPending = shadow_ptr->real_ptr;

                /* txPending may be NULL if a previous segment has failed. */
                if( txPending )
                {
                    if( IS_NOT_FAILED(status) && !IS_LAST_SEGMENT(shadow_ptr) )
                    {
                        /* Do not pass to application yet, wait for last segment to be sent */
                        txPending = NULL;
                    }
                    else
                    {
                        /* Message can be enqueued to Send Q. */
                        if( txPending->q_ptr )
                        {
                            WAIT4MX(MX_PMS_Q);
                            if( IS_FAILED(status) )
                            {
                                /*
                                 * Message has failed, we have to store it's position
                                 * in the queue if it should be retried.
                                 */
                                prev_ptr = txPending->prev_ptr;
                            }
                            /*
                             * Unlink from send Q, cannot be enqueued in case the callback
                             * keeps the message.
                             */
                            _MbmUnlink(txPending);
                            REL_MX(MX_PMS_Q);
                        }
                    }
                }
                else if( MESSAGE_IS_ENQUEUED_IN_INIC(status) )
                {
                    /* Shadow of a segmented message, this is a successive error since there
                     * was no reference to the original message stored with the shadow.
                     */
                    cmd = PMS_M_CMD_CANCEL;
                    status |= PMS_M_STAT_AUTOCAN;   /* Message will be removed */
                }
            }
            else
            {
                shadow_ptr = NULL;
            }

            if( txPending )
            #endif
            {
                PmsTxStatusHandler cbTxStatus_fptr = txPending->cbTxStatus_fptr;
                byte action;
                bool notify_status = MNS_TRUE;

                if ((&mdpFifo != fifo_ptr) || (&i2c == mdpFifo.iface_ptr))  /* supervision of icm, mcm and mdp over i2c */
                {
                    if ((byte)0 != (PMS_M_STAT_SYNCS & status))             /* this is a sync status */
                    {
                        if (PMS_SYNCS_SYNC_CMD == (PMS_M_SYNCS & status))   /* Status.SyncCReceived */
                        {
                            fifo_ptr->syncGuard += (byte)2;

                            if ((byte)5 <= fifo_ptr->syncGuard)
                            {
                                fifo_ptr->syncGuard = (byte)0;
                                MisResetInic();
                            }
                        }
                        else                                                /* any other sync status means reset */
                        {
                            fifo_ptr->syncGuard = (byte)0;
                            notify_status = MNS_FALSE;
                        }
                    }
                    else                                                    /* SyncS not set */
                    {
                        if ((byte)0 != fifo_ptr->syncGuard)
                        {
                            fifo_ptr->syncGuard--;
                        }
                    }
                }

                if( cbTxStatus_fptr && notify_status)
                {
                    byte userStatus;

                    /* Translate status */
                    switch( status & (PMS_M_XMIT | PMS_M_STAT_SYNCS) )
                    {
                        /* Most common case first */
                        case PMS_XMIT_SUCCESS:
                            userStatus = XMIT_SUCCESS;
                            break;

                        case PMS_XMIT_INT_FMAT_FAILURE:
                            userStatus = XMIT_FRMT;
                            break;

                        case PMS_XMIT_INT_MOST_NETWORK_OFF:
                            userStatus = XMIT_NET_OFF;
                            break;

                        case PMS_XMIT_INT_MOST_TIMEOUT:
                            userStatus = XMIT_TIMEOUT;
                            break;

                        case PMS_XMIT_EXT_MOST_WRONG_TARGET:
                            userStatus = XMIT_WRONGTARGET;
                            break;

                        case PMS_XMIT_EXT_MOST_SUCCESS:
                            userStatus = XMIT_SUCCESS | XMIT_BUF;
                            break;

                        case PMS_XMIT_EXT_MOST_BAD_CRC:
                            userStatus = XMIT_CRC;
                            break;

                        case PMS_XMIT_EXT_MOST_RECV_BUF_FULL:
                            userStatus = XMIT_BUF;
                            break;

                        case PMS_XMIT_EXT_MOST_SUCCESS | PMS_XMIT_EXT_MOST_BAD_CRC:
                            userStatus = XMIT_SUCCESS | XMIT_CRC;
                            break;

                        case PMS_XMIT_EXT_MOST_SUCCESS | PMS_XMIT_EXT_MOST_RECV_BUF_FULL:
                            userStatus = XMIT_SUCCESS | XMIT_BUF;
                            break;

                        default:
                            userStatus = (PMS_M_STAT_SYNCS & status)
                                         ? XMIT_SYNC : XMIT_INTERNAL;
                            break;
                    }
                    action = cbTxStatus_fptr(txPending, userStatus);
                    ASSERT((PMS_RELEASE == action) || (PMS_RETRY == action));
                }
                else
                {
                    /* No TxStatus function, free the pending message */
                    MbmFree(txPending);
                    action = PMS_RELEASE;
                }

                if( MESSAGE_IS_ENQUEUED_IN_INIC(status) )
                {
                    if( PMS_RETRY == action )
                    {
                        HMBMBUF handle;
                        #ifndef PMS_TX_NOSEG
                        if( shadow_ptr )
                        {
                            /* It is the shadow that shall be requeued on a retry */
                            handle = (HMBMBUF)shadow_ptr;
                            shadow_ptr = NULL;  /* Prevent deallocation */

                            if( prev_ptr )
                            {
                                WAIT4MX(MX_PMS_Q);
                                /*
                                 * The original message should go back where it was, this can only
                                 * be in the message queue.
                                 */
                                _MbmEnqueueBehind(&fifo_ptr->msgQ, prev_ptr, txPending);
                                REL_MX(MX_PMS_Q);
                            }
                        }
                        else
                        #endif
                        {
                            handle = txPending;
                        }
                        MbmEnqueueFirst(&fifo_ptr->pendingQ, handle);
                        txPending = NULL;
                        cmd       = PMS_M_CMD_RETRY;
                    }
                    else
                    {
                        cmd       = PMS_M_CMD_CANCEL;
                        status   |= PMS_M_STAT_AUTOCAN;   /* Message will be removed */
                        txPending = NULL;
                        /* For segmentation any shadow shall be deallocated. */
                    }
                }
                else if( PMS_RETRY != action )
                {
                    txPending = NULL;
                }

                if( txPending )
                {
                    /* "Retry" of messages that could not be kept in the INIC */
                    PmsSend(txPending, txPending->cbTxStatus_fptr);
                }
            }

            #ifndef PMS_TX_NOSEG
            if( shadow_ptr )
            {
                /* A shadow to a segmented message was processed and an action that
                 * does not retry the message inside the INIC was taken, hence the
                 * whole segmented message will fail and further references are
                 * removed.
                 */
                WAIT4MX(MX_PMS_Q);
                if( IS_FAILED(status) && shadow_ptr->real_ptr && !MESSAGE_IS_ENQUEUED_IN_INIC(status) )
                {
                    _PmsSetReferencesToNull(shadow_ptr->real_ptr);
                }
                _PMS_FREE_SHADOW(shadow_ptr);
                REL_MX(MX_PMS_Q);
            }
            #endif

            if( cmd )
            {
                PmsRespondCommand(fifo_ptr, h_ptr, cmd);
            }
        }
        else if( !(PMS_M_STAT_SYNCS & status) )
        {
            /* unexpected TxStatus, ignore */
            ignore_status = MNS_TRUE;
        }

        if( MNS_FALSE == ignore_status )
        {
            /*
             * If this was either a TxStatus or a SyncS and the message is NOT
             * in the INIC anymore (may be cancelled by a command), then
             * available slots has increased.
             */
            if( !MESSAGE_IS_ENQUEUED_IN_INIC(status) )
            {
                bool ready;
                WAIT4MX(MX_PMS_CS);
                ready = PMS_IFACE_READY(*fifo_ptr->iface_ptr) ? MNS_TRUE : MNS_FALSE;
                REL_MX(MX_PMS_CS);
                /*
                 * Only messages need to be triggered if a new slot becomes available,
                 * status and command messages does not need any slot so they are
                 * triggered from the availablity of the LLD.
                 */
                if( !fifo_ptr->txSlotsAvailable                 /* FIFO in INIC was previously full? */
                    && MBM_QUEUE_LENGTH(&fifo_ptr->msgQ)        /* Normal message to send?           */
                    && (MNS_FALSE != ready) )                   /* Is the LLD ready?                 */
                {
                    PmsSetPendingEvent(PMS_P_TX_TRIG);          /* Ok, trigger next FIFO message     */
                }
                #ifdef PMS_NO_SLOTNTF
                fifo_ptr->txSlotsAvailable = 1;                 /* Now the FIFO is ready             */
                #else
                if( PMS_M_STAT_SYNCS & status )
                {
                    fifo_ptr->txSlotsAvailable = slots;
                }
                else if( fifo_ptr->txSlotsAvailable < slots )
                {
                    fifo_ptr->txSlotsAvailable++;
                }
                #endif
            }

            /*
             * The TX-timer for messages on this FIFO shall be removed if there are
             * NO more messages pending or otherwise updated IF this was a TxStatus
             * message, i.e. either a message is retried and a new timeout issued
             * or one succeeded and the next one is now being processed inside the
             * INIC.
             * The check for complete status is to make sure that any timer is not
             * updated on a SlotAv.
             */
            #ifdef PMS_NO_SLOTNTF
            if( !fifo_ptr->txSlotsAvailable )
            #else
            if( fifo_ptr->txSlotsAvailable < slots )
            #endif
            {
                #ifndef PMS_DISABLE_MCM_EXT_TIMEOUT
                word fifo_timeout;
                HMBMBUF next_pending;

                WAIT4MX(MX_PMS_Q);
                next_pending = _MbmPeek(&fifo_ptr->pendingQ);

                if ( (&mcmFifo == fifo_ptr) && next_pending )
                {
                    if ( PMS_CFG_MCM_MLR_THRESHOLD < next_pending->msg.msgTx.MidLevelRetries )
                    {
                        fifo_timeout = PMS_CFG_MCM_MAX_TIMEOUT;
                    }
                    else
                    {
                        fifo_timeout = fifo_ptr->timeout;
                    }
                }
                else
                {
                    fifo_timeout = fifo_ptr->timeout;
                }
                REL_MX(MX_PMS_Q);

                /* Message pending (either a retry or next in FIFO), update timer */
                MostSetTimer(&fifo_ptr->timer, fifo_timeout, MNS_FALSE);
                #else
                /* Message pending (either a retry or next in FIFO), update timer */
                MostSetTimer(&fifo_ptr->timer, fifo_ptr->timeout, MNS_FALSE);
                #endif
            }
            else
            {
                MostClearTimer(&fifo_ptr->timer);
            }
        }
    }
    T_MOD_EXIT(PMS_19);
}
#endif

#ifdef PMS_20
static void PmsSendCommand(PmsFifo *fifo_ptr, HMBMBUF handle)
{
    byte fph;
    word length;
    bool ready;
    byte *tgt_ptr;

    T_MOD_ENTRY(PMS_20);

    tgt_ptr = MBM_RESET_HDR_PTR(handle);    /* Set & Get start of buffer  */

    ASSERT(fifo_ptr);

    if( (MBM_TYPE_CMD_TX == MBM_GET_TYPE(handle))
        && (PMS_M_CMD_SYNCC & handle->msg.ctrl) )
    {
        #ifndef PMS_NO_SLOTNTF
        handle->msg.ctrl |= PMS_M_CMD_SLOTNTF;
        #endif
        fph = (byte)(PMS_FIFONO_ALL << FPH_B_FIFONO_LSB);
    }
    #ifdef MDP_MIN
    else if( &mdpFifo == fifo_ptr )
    {
        fph = (byte)(PMS_FIFONO_MDP << FPH_B_FIFONO_LSB);
    }
    #endif
    else if( &icmFifo == fifo_ptr )
    {
        fph = (byte)(PMS_FIFONO_ICM << FPH_B_FIFONO_LSB);
    }
    else
    {
        fph = (byte)(PMS_FIFONO_MCM << FPH_B_FIFONO_LSB);
    }

    #ifdef PMS_USE_HANDLE
    /* Send handle for Commands that's not to ALL and handle != default */
    if( MBM_TYPE_CMD_TX == MBM_GET_TYPE(handle) )
    {
        if( (fph != (PMS_FIFONO_ALL << FPH_B_FIFONO_LSB))
            && (PMS_DEF_HANDLE != handle->handle) )
        {
            fph |= (FPH_M_SCF0_ENABLED | FPH_MSGTYPE_CMD);
        }
        else
        {
            fph |= FPH_MSGTYPE_CMD;
        }
    }
    else
    {
        fph |= FPH_MSGTYPE_STAT;
    }
    #else
    fph |= (MBM_TYPE_STATUS_TX == MBM_GET_TYPE(handle))
           ? FPH_MSGTYPE_STAT : FPH_MSGTYPE_CMD;
    #endif

    tgt_ptr[3] = fph;

    /* Calculate length */
    #ifdef PMS_USE_HANDLE
    length = (fph & FPH_M_SCF0_ENABLED) ? 4 : 2;
    #else
    length = (word)2;
    #endif

    #ifdef PMS_RX_OPT3
    if (PMS_M_CMD_SYNCC & handle->msg.ctrl)
    {
        length += 2;                 /* requires add. fields for protocol ID and MCM channel address */
    }
    #endif

    /* Fill command/status message */
    *tgt_ptr++ = (byte)0;                   /* PML MSB */
    *tgt_ptr++ = (byte)(length + (word)1);  /* PML LSB */
    *tgt_ptr++ = (byte)length;              /* PMHL    */

    #ifdef PMS_USE_HANDLE
    if( *tgt_ptr++ & FPH_M_SCF0_ENABLED )
    {
        *tgt_ptr++ = PMS_M_SCF0_HANDLE;
        *tgt_ptr++ = handle->handle;
    }
    #else
    tgt_ptr++;
    #endif

    *tgt_ptr++ = handle->msg.ctrl; /* Command/Status 1 */

    #ifdef PMS_RX_OPT3
    if (PMS_M_CMD_SYNCC & handle->msg.ctrl)
    {
        *tgt_ptr++ = PMS_PID_OPTIMIZED;             /* Protocol ID */
        *tgt_ptr++ = (byte)PMS_RX_OPT3_CHADDR;      /* add. MCM RX channel address */
    }
    #endif

    MBM_SET_HDR_LEN(handle, (length + (word)3));
    MbmEnqueue(&fifo_ptr->cmdQ, handle);

    WAIT4MX(MX_PMS_CS);
    ready = PMS_IFACE_READY(*fifo_ptr->iface_ptr) ? MNS_TRUE : MNS_FALSE;
    REL_MX(MX_PMS_CS);

    if( MNS_FALSE != ready )
    {
        /*!
         * Only necessary to trigger if the interface is ready for another
         * buffer, otherwise it will be retriggered when completed.
         */
        PmsSetPendingEvent(PMS_P_TX_TRIG);
    }
    T_MOD_EXIT(PMS_20);
}
#endif

#ifdef PMS_21
static void PmsSyncIn(const byte *status_ptr, byte size)
{
    word externEvent;
    T_MOD_ENTRY(PMS_21);

    if( PMS_SYNCS_SYNC_CMD == (byte)(PMS_M_SYNCS & (*status_ptr)) )
    {
        #ifndef PMS_RX_OPT3
        if ((byte)1 == size)
        {
            externEvent = PMS_E_SYNC_CMD;           /* legacy synchronization status message */
        }
        else
        {
            externEvent = PMS_E_SYNC_NOT_SUPPORTED; /* ext. synchr. status - INIC is running in optimized mode */
        }
        #else
        if ((size > (byte)2) && (PMS_PID_OPTIMIZED == status_ptr[1]) && ((byte)PMS_RX_OPT3_CHADDR == status_ptr[2]))
        {
            externEvent = PMS_E_SYNC_CMD;           /* Protocol ID or add. MCM RX channel address is correct */
        }
        else
        {
            externEvent = PMS_E_SYNC_NOT_SUPPORTED; /* optimized protocol or channel is not supported */
        }
        #endif
    }
    else
    {
        externEvent = PMS_E_SYNC_RESET;             /* Treat all other and also unexpected as a Reset */
    }

    PmsSyncHandler(PMS_P_SYNCS_CMD);

    PMS_EXTERNAL_EVENT(externEvent);

    T_MOD_EXIT(PMS_21);
}
#endif


#ifdef PMS_23
static void PmsSyncHandler(word events)
{
    static word syncCount; /* Count number of Sync attempts */
    byte actions;

    T_MOD_ENTRY(PMS_23);

    actions = PMS_A_NONE;

    switch( pmsSyncState )
    {
        case PMS_S_INIT:
            if( events & PMS_P_TIMEOUT )
            {
                pms.initSync = 0;
                actions = PMS_A_SYNCC | PMS_A_RESET_CNT;
                pmsSyncState = PMS_S_PENDING;
            }
            break;

        case PMS_S_PENDING:
        case PMS_S_SYNCED:
            if( events & PMS_P_SYNCS_CMD )
            {
                pmsSyncState = PMS_S_SYNCED;
                actions = PMS_A_RESET_CNT;

                MostClearTimer(&pmsSyncTimer);

                if( !pms.initSync )
                {
                    pms.initSync = 1;
                    MnsServiceInitComplete(MNS_PHASE_INIT, MNS_P_SRV_PMS);
                }
            }
            if( events & (PMS_P_TIMEOUT | PMS_P_SEND_SYNCC) )
            {
                actions = PMS_A_SYNCC;
            }
            break;

        case PMS_S_FAILED:
            if( events & PMS_P_SEND_SYNCC )
            {
                actions = PMS_A_SYNCC | PMS_A_RESET_CNT;
            }
            break;

        default:
            break;
    }

    if( actions & PMS_A_RESET_CNT )
    {
        syncCount = (word)0;
    }

    if( actions & PMS_A_SYNCC )
    {
        if( syncCount++ < PMS_MAX_SYNC_ATTEMPTS )
        {
            /* Force sending SyncC */
            HMBMBUF handle;

            WAIT4MX(MX_PMS_CS);
            handle  = cmdFree;
            cmdFree = NULL;
            REL_MX(MX_PMS_CS);

            /* Clear any message timers to prevent premature Sync timeout */
            MostClearTimer(&icmFifo.timer);
            MostClearTimer(&mcmFifo.timer);
            #ifdef ADS_MIN /* Not needed with MDP_MIN only, no TX */
            MostClearTimer(&mdpFifo.timer);
            #endif
            MostSetTimer(&pmsSyncTimer, pmsSyncTimeout, MNS_FALSE);

            if( handle )
            {
                handle->msg.ctrl = PMS_M_CMD_SYNCC;
                PmsSendCommand(&icmFifo, handle);
            }
            /* else a SyncC is already enqueued */

            pmsSyncState = PMS_S_PENDING;
        }
        else
        {
            pmsSyncState = PMS_S_FAILED;
            PMS_EXTERNAL_EVENT(PMS_E_SYNC_FAILED);
            syncCount = (word)0;
        }
    }

    T_MOD_EXIT(PMS_23);
}
#endif

/*
*/
#ifdef PMS_33
static bool PmsRxSegMatch(const MbmCtrlRx *l_ptr, const MbmCtrlRx *r_ptr)
{
    bool retval;

    T_MOD_ENTRY(PMS_33);

    ASSERT(l_ptr);
    ASSERT(r_ptr);

    if(    (l_ptr->Src_Adr   == r_ptr->Src_Adr)
        && (l_ptr->FBlock_ID == r_ptr->FBlock_ID)
        && (l_ptr->Inst_ID   == r_ptr->Inst_ID)
        && (l_ptr->Func_ID   == r_ptr->Func_ID)
        && (l_ptr->Operation == r_ptr->Operation) )
    {
        retval = MNS_TRUE;
    }
    else
    {
        retval = MNS_FALSE;
    }

    T_MOD_EXIT(PMS_33);
    return (retval);
}
#endif

#ifdef PMS_24
static HMBMBUF PmsRxSegProcess(HMBMBUF hNew)
{
    #ifdef MSG_RX_AUTOMATIC_ERR_MSG
    HMBMBUF    hErr;
    #endif
    HMBMBUF    hOld;
    MbmCtrlRx *new_ptr;
    MbmCtrlRx *old_ptr;
    byte       error;
    byte       telId;
    bool       terminated;
    bool       skip_alloc;

    T_MOD_ENTRY(PMS_24);

    #ifdef MSG_RX_AUTOMATIC_ERR_MSG
    hErr        = hNew;                         /* Always reuse hNew for error message */
    #endif
    error       = (byte)0;
    skip_alloc  = MNS_FALSE;
    old_ptr     = NULL;
    new_ptr     = MBM_GET_CTRL_RX_PTR(hNew);
    terminated  = MNS_FALSE;
    telId       = hNew->tel_id;

    if (telId <= PMS_TELID_3)
    {
        if(telId && (new_ptr->Length < (word)2))
        {
            terminated = MNS_TRUE;              /* Invalid segmented message, it has to be the segment counter and at       */
            telId = PMS_TELID_3;                /* least one byte.                                                          */
        }
    }
    else if ((MNS_FALSE != pmsSupportsTelId4) && (PMS_TELID_4 == telId))
    {
        if ((word)2 <= new_ptr->Length)                 /* MsgSize - 2 or more bytes */
        {
            word prealloc_size;
            DECODE_WORD(&prealloc_size, new_ptr->Data);

            if (prealloc_size <= pmsMaxCmsPayload)
            {
                telId = PMS_TELID_UNKNOWN;              /* invalid message, ignore & free */
            }
        }
        else
        {
            telId = PMS_TELID_UNKNOWN;                  /* invalid message, ignore & free */
        }

    }
    else
    {
        telId = PMS_TELID_UNKNOWN;                      /* invalid message, ignore & free */
    }


    /*!
     * Find the last received segment from this source, this is easily performed
     * by searching the segmentation list backwards
     */
    hOld = PmsRxSegFindMatch(new_ptr, MNS_FALSE);

    if (hOld)
    {
        old_ptr = MBM_GET_CTRL_RX_PTR(hOld);
    }


    if ( (PMS_TELID_1 == telId) && hOld )
    {
        ASSERT(old_ptr);
        if ((word)0 == old_ptr->Length)
        {
            if (PMS_TELID4_MSGCNT_ALLOC_OK == hOld->payload_ptr[0])
            {
                /*
                 * TelID 0x4 was previous message
                 * and the payload was allocated correctly
                 */
                skip_alloc  = MNS_TRUE;
            }
            else if (PMS_TELID4_MSGCNT_ALLOC_FAILED == hOld->payload_ptr[0])
            {
                /* was not able to allocate the payload */
                PmsRxSegError(hOld, MSG_ERR_4);                     /* unlink and free hOld */
                hOld  = NULL;                                       /* not really neccesary */
                #ifdef MSG_RX_AUTOMATIC_ERR_MSG
                error = MSG_ERR_4 | KEEP_MESSAGE;                   /* use hNew to send segmentation error 0x04 */
                #else
                error = MSG_ERR_4 | KEEP_MESSAGE | STATUS_TO_INIC;  /* message is freed in section PMS_TELID_UNKNOWN */
                #endif
                PmsRxSegError(hNew, error);
                telId = PMS_TELID_UNKNOWN;
            }
        }
        /* else case is new start before end */
    }

    if((MNS_FALSE == skip_alloc) && ((telId <= PMS_TELID_1) || (PMS_TELID_4 == telId)) )   /* Not segmented or initial */
    {
        #ifdef MSG_RX_AUTOMATIC_ERR_MSG
        if( (PMS_TELID_1 == telId) && ((byte)0 != new_ptr->Data[0]) )
        {
            /* Initial segment with segment counter != 0 */
            if( hOld )
            {
                /*
                 * Had an old message with the same segmentation handle, notify
                 * restart before end and discard since the new initial has the
                 * wrong segment counter also.
                 */
                PmsRxSegError(hOld, MSG_ERR_7);
                hOld = NULL;
            }
            error = MSG_ERR_3 | KEEP_MESSAGE; /* Keep for hErr */
            PmsRxSegError(hNew, error);
            hNew = NULL; /* Will be hErr for the error message */
        }
        #endif

        if( hOld )
        {
            ASSERT(old_ptr);
            /* Received new start before last message was completed */
            /* Keep hOld as target for restarted message (or copying unsegmented to) */
            error = MSG_ERR_7 | KEEP_MESSAGE;
            PmsRxSegError(hOld, error);
            /* hNew will be used for the error after being appended (or copied). */

            if ( PMS_TELID_1 == telId )
            {
                /*
                 * Reset structure to restart message
                 * N.B. hNew is set to NULL (used as hErr)
                 *      after appending the payload
                 */
                old_ptr->Length = (word)0;
                old_ptr->Rcv_Type = new_ptr->Rcv_Type;
                hOld->payload_ptr = old_ptr->Data;
            }
            else if (PMS_TELID_4 == telId)
            {
                if ((old_ptr->Length > (word)0) || (PMS_TELID4_MSGCNT_ALLOC_OK == hOld->payload_ptr[0]))
                {
                    /* hOld was allocated correctly */
                    word prealloc_size;
                    DECODE_WORD(&prealloc_size, new_ptr->Data);

                    if (hOld->size >= prealloc_size)
                    {
                        /* Reset structure to restart message */
                        /* hOld can be re-used, hNew will be used to send the segmentation error message */
                        old_ptr->Length = (word)0;
                        old_ptr->Rcv_Type = new_ptr->Rcv_Type;
                        hOld->payload_ptr = old_ptr->Data;
                    }
                    else
                    {
                        /* Size of hOld is not big enough for the new message           */
                        /* Free hOld, we need a new allocation                          */
                        /* The error was set above -> if the allocation is successful,  */
                        /* hNew can be used for the segmentation error 0x7              */
                        MbmFree(hOld);
                        hOld = NULL;
                    }
                }
                else
                {
                    /* hOld was TelID "4" but could not allocate the needed size of payload */
                    /* -> hOld has no user payload, use hOld as hErr */

                     #ifdef MSG_RX_AUTOMATIC_ERR_MSG
                     hErr = hOld;       /* now hErr looses the reference to hNew */
                                        /* if alloc fails: hNew -> Q; if alloc ok: hNew is freed */
                     #else
                     MbmFree(hOld);     /* no status to the INIC !!! */
                     #endif
                     hOld = NULL;
                }
            }
            else /* PMS_TELID_0 */
            {
                ASSERT(PMS_TELID_0 == telId);

                #ifdef MSG_RX_AUTOMATIC_ERR_MSG
                if (hOld->size >= new_ptr->Length)
                {
                    /*
                     * hOld will be passed to the application (as hNew) and hNew has
                     * become hErr for the error message (top)
                     */
                    MNS_MEM_CPY(old_ptr->Data, new_ptr->Data, new_ptr->Length);
                    old_ptr->Length   = new_ptr->Length;
                    old_ptr->Rcv_Type = new_ptr->Rcv_Type;
                    hNew = hOld;
                }
                else
                {
                    /* The payload size of hOld is not big enough for the new message */
                    #ifdef MSG_RX_USER_PAYLOAD
                    if (HAS_RX_USER_PAYLOAD(hOld))
                    {
                        /* RX message with user payload */
                        pmsInitData_ptr->cbFreePayloadBuf_fptr(old_ptr);
                        hOld->size = (word)0;
                        hOld->payload_ptr = NULL;
                        /* try to substitute user payload with internal payload */
                        if(MNS_FALSE != MbmAllocateRxMinPayload(hOld))
                        {
                            /* now we can use hOld as segmentation error */
                            hErr = hOld;
                        }
                        else
                        {
                            MbmFree(hOld);
                            error = (byte)0;    /* prevent that hNew is used to send the error message    */
                            FAILED_ASSERT();    /* no internal memory left to sent segmentation error 0x7 */
                                                /* consider to increase the NetServices internal memory   */
                        }
                    }
                    else
                    #endif
                    {
                        /* hOld has internal payload -> we can use it as segmentation error message */
                        hErr = hOld;
                    }
                }
                #else
                MbmFree(hOld);
                #endif
                hOld = NULL;
            }
        }

        if( telId && hNew ) /* Initial segment and no error */
        {
            if( !hOld )
            {
                word prealloc_size;

                if (PMS_TELID_4 == telId)
                {
                    DECODE_WORD(&prealloc_size, new_ptr->Data);
                }
                else
                {
                    prealloc_size = PMS_CFG_RX_SEG_PREALLOC_SIZE;
                }

                #ifdef MSG_RX_USER_PAYLOAD
                if( !MBM_QUEUE_LENGTH(&pms.rxPayloadWaitingQ) )
                {
                    hOld = MbmAllocate((word)0, (word)0, MBM_TYPE_CTRL_RX);

                    if( hOld )
                    {
                        word size;
                        MbmCtrlRx *tgt_ptr = MBM_GET_CTRL_RX_PTR(hOld);
                        *tgt_ptr = *new_ptr;
                        /* Don't care about tgt_ptr->Data, it'll be handled next */

                        tgt_ptr->Data = NULL; /* New payload needed */
                        size = pmsInitData_ptr->cbGetPayloadBuf_fptr(tgt_ptr, prealloc_size);

                        if( size )
                        {
                            hOld->size        = size;
                            hOld->payload_ptr = tgt_ptr->Data;
                            tgt_ptr->Length   = (word)0; /* Nothing yet */
                        }
                        else
                        {
                            MbmFree(hOld);
                            hOld = NULL;
                        }
                    }
                }
                #else
                hOld = MbmAllocate(prealloc_size, (word)0, MBM_TYPE_CTRL_RX);
                #endif
            }

            if( hOld )
            {
                #ifndef MSG_RX_USER_PAYLOAD /* Already done with user payload */
                MbmCtrlRx *tgt_ptr = MBM_GET_CTRL_RX_PTR(hOld);
                byte *data_ptr = tgt_ptr->Data;
                *tgt_ptr = *new_ptr;
                tgt_ptr->Data   = data_ptr;
                tgt_ptr->Length = (word)0;          /* Nothing so far */
                #endif

                _MbmEnqueue(&rxSegQ, hOld);
            }
            else if (PMS_TELID_4 == telId)
            {
                /* Unable to allocate target buffer */
                new_ptr->Length = (word)0;          /* mark handle as initial message */
                hNew->payload_ptr[0] = PMS_TELID4_MSGCNT_ALLOC_FAILED;
                _MbmEnqueue(&rxSegQ, hNew);         /* store hNew */

                #ifdef MSG_RX_AUTOMATIC_ERR_MSG
                if (hErr == hNew)
                {
                    hErr = NULL;                    /* do not free hErr since we enqueue hNew! */
                    error = (byte)0;                /* prevent access to hErr */
                }
                #endif
                hNew = NULL;                        /* do not return hNew and do not copy payload */
            }
            else
            {
                /* Unable to allocate target buffer */
                #ifdef MSG_RX_AUTOMATIC_ERR_MSG
                error = MSG_ERR_4 | KEEP_MESSAGE;
                #else
                error = MSG_ERR_4 | STATUS_TO_INIC;
                #endif
                PmsRxSegError(hNew, error);
                hNew = NULL;
            }
        }
        /* Else keep hNew and return it */
    }
    else if( PMS_TELID_UNKNOWN == telId )
    {
        #ifndef MSG_RX_AUTOMATIC_ERR_MSG
        MbmFree(hNew);
        #endif
        hNew = NULL;        /* ignore unknown telegram -> return NULL (not completed) */
    }
    else if (hOld && old_ptr && (old_ptr->Length || (PMS_TELID_1 == telId) || (MNS_FALSE != terminated))) /* 0x2 <= telId <= 0x3 */
    {
        /*
         * Additional segment, check first if it's an ISS termination message.
         * ISS Termination has tel_id 3 and length 0.
         * If not terminated, check that the segment counter is expected.
         */
        if( (MNS_FALSE != terminated) || (((hOld->payload_ptr[0] + (byte)1) & (byte)0xFF) != new_ptr->Data[0]) )
        {
            /* Segment error or terminated */
            #ifdef MSG_RX_AUTOMATIC_ERR_MSG
            /* No status to the INIC, EHC handles segmentation */
            error = MSG_ERR_3 | NO_UNLINK;
            /* hOld should be kept and still linked, content of hNew ignored but reused as error message */
            #else
            MbmFree(hNew);
            /* No error to INIC on termination message */
            error = MSG_ERR_3 | ((MNS_FALSE != terminated) ? 0 : STATUS_TO_INIC);
            #endif
            PmsRxSegError(hOld, error);
            hNew = NULL;
        }
    }
    else /* Not initial segment and nothing pending from this source */
    {
        if(hOld)
        {
            PmsRxSegError(hOld, MSG_ERR_1); /* delete and unlink hOld if it was allocated by TelId 0x4 */
            hOld = NULL;
        }
        #ifdef MSG_RX_AUTOMATIC_ERR_MSG
        /* No INIC status, EHC based segmentation */
        error = MSG_ERR_1 | KEEP_MESSAGE;
        #else
        /* No error if it was a termination message */
        error = MSG_ERR_1 | ((MNS_FALSE != terminated) ? (byte)0 : STATUS_TO_INIC);
        #endif
        PmsRxSegError(hNew, error);
        hNew = NULL;
    }

    if( hNew && telId )                     /* Only enter if segmented */
    {
        ASSERT(hOld);   /* Should not get here without hOld */

        if ( PMS_TELID_4 == telId )
        {
            /* insert initial MsgCnt with 0xFF */
            hOld->payload_ptr[0] = PMS_TELID4_MSGCNT_ALLOC_OK;

            #ifdef MSG_RX_AUTOMATIC_ERR_MSG
            if (hErr != hNew)
            {
                MbmFree(hNew);      /* free hNew since hErr is the previous hOld */
            }
            #else
            MbmFree(hNew);
            #endif
            hNew = NULL;
        }
        else if( MNS_FALSE != PmsRxSegAppend(hOld, hNew) )
        {
            #ifndef MSG_RX_AUTOMATIC_ERR_MSG
            MbmFree(hNew);
            #endif
            hNew = NULL;

            /* The new segment was appended */
            if( PMS_TELID_3 == telId )
            {
                /* Message completed, unlink from segmentation list */
                _MbmUnlink(hOld);

                hNew = hOld; /* Return completed message */
            }
            else
            {
                MBM_CLR_STATUS_BITS(hOld, MBM_STAT_RX_GARBAGE_BIT); /* Protect from garbage collector. */
            }
        }
        else
        {
            /* TODO: Add real trace output, this is a configuration problem */
            /* There was not enough space left to append */

            PmsRxSegError(hOld, MSG_ERR_8);
            #ifdef MSG_RX_AUTOMATIC_ERR_MSG
            error = MSG_ERR_2 | KEEP_MESSAGE;
            #else
            error = MSG_ERR_2 | STATUS_TO_INIC;
            #endif
            PmsRxSegError(hNew, error);
            hNew = NULL;
        }
    }

    #ifdef MSG_RX_AUTOMATIC_ERR_MSG
    if( error && !(new_ptr->Rcv_Type & MSG_RCV_TYPE_BROADCAST) ) /* Not if broad/groupcast */
    {
        MbmCtrlTx *tx_ptr;

        ASSERT(hErr);
        ASSERT(hErr != hNew);

        if (MNS_FALSE == MbmChangeType(hErr, (word)MBM_TYPE_CTRL_TX))
        {
            WAIT4MX(MX_PMS_CS);
            if (NULL == pms.rxPreAllocPtrTab[PMS_IDX_MCM_RX_PREALLOC])
            {
                error = (byte)0;
            }
            REL_MX(MX_PMS_CS);
        }

        if ((byte)0 != error)
        {
            tx_ptr = MBM_GET_CTRL_TX_PTR(hErr);
            ASSERT(tx_ptr->Length >= (word)2);      /* Should be guaranteed by MbmGetBuf() */

            tx_ptr->Operation = OP_ERROR;
            tx_ptr->Length    = (word)2;
            tx_ptr->Data[0]   = (byte)0x0C;         /* Segmentation error */
            tx_ptr->Data[1]   = error & (byte)0xF;  /* Error ID           */

            tx_ptr->LowLevelRetries = PMS_SEG_ERROR_LLR;
            tx_ptr->MidLevelRetries = PMS_SEG_ERROR_MLR;

            PmsSend(hErr, NULL);
        }
        else
        {
            MbmFree(hErr);
            FAILED_ASSERT();                    /* Information: Discarding the segmentation */
                                                /* error message due to the lack of handles */
        }
    }
    else if( (hErr) && ((NULL == hNew) || (hErr != hNew)) )
    {
        /*
         * Discard unused hErr if it is does not match the original buffer
         * that is returned (i.e. a non-segmented message).
         */
        MbmFree(hErr);
    }
    #endif

    #ifdef PMS_28
    /* Ensure that the garbage collector runs if there are segments pending */
    if( MBM_QUEUE_LENGTH(&rxSegQ) > (word)0 )
    {
        if((word)0 == MostGetTimer(&pmsGbgTimer))
        {
            MostSetTimer(&pmsGbgTimer, PMS_RX_SEG_TIMEOUT, MNS_FALSE);
        }
    }
    else
    {
        MostClearTimer(&pmsGbgTimer);
    }
    #endif
    T_MOD_EXIT(PMS_24);

    return( hNew ); /* Non-NULL => any complete message */
}
#endif


/*
--------------------------------------------------------------------------------
 Function:      PmsRxSegFindMatch(...)

 Description:   Searches the rxSegQ in the direction specified for the first
                found node with a matching segmentation handle. To find the
                last one, just search the list backwards.

 Input(s):      new_ptr       :
                forward       : MNS_TRUE if list shall be searched forward or
                                MNS_FALSE for backwards.

 Return(s):     First found matching node or NULL if none in the list
--------------------------------------------------------------------------------
*/
#ifdef PMS_25
static HMBMBUF PmsRxSegFindMatch(const MbmCtrlRx *new_ptr, bool forward)
{
    HMBMBUF hCur;
    T_MOD_ENTRY(PMS_25);

    /*!
     * The rxSegQ does not need to be locked, it is only accessed from the
     * service thread.
     */
    hCur = (MNS_FALSE != forward) ? rxSegQ.next_ptr : rxSegQ.prev_ptr;
    while( ((HMBMBUF)&rxSegQ != hCur)
           && (MNS_FALSE == PmsRxSegMatch(new_ptr, MBM_GET_CTRL_RX_PTR(hCur))) )
    {
        hCur = (MNS_FALSE != forward) ? hCur->next_ptr : hCur->prev_ptr;
    }
    T_MOD_EXIT(PMS_25);
    /* Return NULL if search hit the head, else the first found match */
    return( ((HMBMBUF)&rxSegQ == hCur) ? NULL : hCur );
}
#endif

#ifdef PMS_34
static bool PmsRxSegAppend(HMBMBUF hAssembled, HMBMBUF hSegment)
{
    MbmCtrlRx *seg_ptr;
    MbmCtrlRx *rx_ptr;
    bool return_code;
    word size_needed;

    T_MOD_ENTRY(PMS_34);

    seg_ptr = MBM_GET_CTRL_RX_PTR(hSegment);
    rx_ptr  = MBM_GET_CTRL_RX_PTR(hAssembled);

    size_needed = seg_ptr->Length;
    if( PMS_TELID_3 == hSegment->tel_id )
    {
        size_needed--;  /* Does not need to store segment counter */
    }

    return_code = ((hAssembled->size - rx_ptr->Length) >= size_needed) ? MNS_TRUE : MNS_FALSE;

    #ifdef MSG_RX_USER_PAYLOAD
    if( (MNS_FALSE == return_code) && pmsInitData_ptr->cbReallocPayloadBuf_fptr )
    {
        #if PMS_RX_SEG_PREALLOC_SIZE
        word new_size = hAssembled->size + PMS_CFG_RX_SEG_PREALLOC_SIZE;
        #else
        word new_size = (word)0;
        #endif

        new_size = pmsInitData_ptr->cbReallocPayloadBuf_fptr(rx_ptr, new_size);
        hAssembled->size = new_size;

        if( new_size )
        {
            /* Restore position where to add the next segment in the new buffer */
            hAssembled->payload_ptr = rx_ptr->Data + rx_ptr->Length;

            return_code = ((new_size - rx_ptr->Length) >= size_needed) ? MNS_TRUE : MNS_FALSE;
        }
        else    /* No application buffer and app freed old one */
        {
            rx_ptr->Data = NULL;
            return_code  = MNS_FALSE;
        }
    }
    #endif

    if( MNS_FALSE != return_code )
    {
        byte *tgt_ptr = hAssembled->payload_ptr;
        byte *src_ptr = &seg_ptr->Data[1];  /* Skip segment counter */

        size_needed = seg_ptr->Length - (word)1;  /* Skip segment counter in copy */
        rx_ptr->Length += size_needed;

        MNS_MEM_CPY(tgt_ptr, src_ptr, size_needed);
        tgt_ptr += size_needed;

        if( PMS_TELID_3 != hSegment->tel_id )   /* Expect further segments? */
        {
            hAssembled->payload_ptr = tgt_ptr;  /* Position for next segment */

            /* Put segment counter behind payload to compare next counter, space
             * is left since 1 was subtracted from size_needed before copying.
             */
            *tgt_ptr = seg_ptr->Data[0];
        }
    }
    T_MOD_EXIT(PMS_34);
    return( return_code );
}
#endif

#ifdef PMS_35
static byte PmsSegErrorTxStatus(HMBMBUF handle, byte status)
{
    T_MOD_ENTRY(PMS_35);
    (void) handle;
    (void) status;
    MBM_CLR_STATUS_BITS(&segErrMsg, MBM_STAT_RSVD);
    #ifdef MSG_RX_AUTOMATIC_ERR_MSG
    if( pms.needSegErrMsg )
    {
        pms.needSegErrMsg = 0;
        PmsSetPendingEvent(PMS_P_RETRIGGER_GBG);
    }
    #endif
    T_MOD_EXIT(PMS_35);

    return( PMS_RELEASE );
}
#endif

#ifdef PMS_28
static void PmsRxSegGarbageCollector(bool no_mark)
{
    HMBMBUF hCur;
    T_MOD_ENTRY(PMS_28);

    hCur = _MbmPeek(&rxSegQ);
    while( hCur )
    {
        HMBMBUF hNext;
        ASSERT(&rxSegQ == hCur->q_ptr);
        hNext = MBM_NEXT(hCur);
        if( MBM_STAT_RX_GARBAGE_BIT & MBM_GET_STATUS(hCur) )
        {
            #ifdef MSG_RX_AUTOMATIC_ERR_MSG
            MbmCtrlRx *rx_ptr = MBM_GET_CTRL_RX_PTR(hCur);

            /*
             * The application should only be notified yet if this is either a
             * broad/groupcast or an error message was sent, else it will be
             * kept and delayed until the error message is available.
             * (groupcast included since = 0x03)
             */
            bool error_sent_or_broadcast = (rx_ptr->Rcv_Type & MSG_RCV_TYPE_BROADCAST) ? MNS_TRUE : MNS_FALSE;

            if( MNS_FALSE == error_sent_or_broadcast )
            {
                bool granted;

                WAIT4MX(MX_PMS_CS);
                if( !(MBM_GET_STATUS(&segErrMsg) & MBM_STAT_RSVD) )
                {
                    MBM_SET_STATUS_BITS(&segErrMsg, MBM_STAT_RSVD);
                    granted = MNS_TRUE;
                }
                else
                {
                    granted = MNS_FALSE;
                }
                REL_MX(MX_PMS_CS);

                if( MNS_FALSE != granted )
                {
                    segErrMsg.msg.msgTx.Tgt_Adr   = rx_ptr->Src_Adr;
                    segErrMsg.msg.msgTx.FBlock_ID = rx_ptr->FBlock_ID;
                    segErrMsg.msg.msgTx.Inst_ID   = rx_ptr->Inst_ID;
                    segErrMsg.msg.msgTx.Func_ID   = rx_ptr->Func_ID;
                    /* Operation, Length, Retries and Data[0] already set */
                    segErrMsg.msg.msgTx.Data[1]   = MSG_ERR_5;   /* Error ID           */

                    PmsSendBypass(&segErrMsg, PmsSegErrorTxStatus, MNS_TRUE);

                    error_sent_or_broadcast = MNS_TRUE;
                }
                else
                {
                    pms.needSegErrMsg = 1;
                }
            }

            if( MNS_FALSE != error_sent_or_broadcast )
            {
                PmsRxSegError(hCur, MSG_ERR_5);
            }
            #else
            PmsRxSegError(hCur, MSG_ERR_5);
            #endif
        }
        else
        {
            if( MNS_FALSE == no_mark )
            {
                MBM_SET_STATUS_BITS(hCur, MBM_STAT_RX_GARBAGE_BIT);
            }
        }

        hCur = hNext;
    }
    /* Ensure that garbage collector runs if there are segments pending */
    if( MBM_QUEUE_LENGTH(&rxSegQ) && !MostGetTimer(&pmsGbgTimer) )
    {
        MostSetTimer(&pmsGbgTimer, PMS_RX_SEG_TIMEOUT, MNS_FALSE);
    }
    T_MOD_EXIT(PMS_28);
}
#endif

#ifdef PMS_29
static void PmsRxSegError(HMBMBUF hErr, byte error)
{
    T_MOD_ENTRY(PMS_29);

    /* Call the higher layer */

    if( !(error & NO_UNLINK) && hErr->next_ptr )
    {
        _MbmUnlink(hErr);
    }
    if( !(error & (KEEP_MESSAGE|NO_UNLINK)) )
    {
        MbmFree(hErr);
    }

    #ifndef MSG_RX_AUTOMATIC_ERR_MSG /* Combination not possible */
    if( error & STATUS_TO_INIC )
    {
        mcmFifo.rxStatus = PMS_M_STAT_SLOTAV | PMS_XMIT_INT_FMAT_FAILURE;
    }
    #endif

    T_MOD_EXIT(PMS_29);
}
#endif

/*
*/

#ifdef PMS_32
static void PmsRxHandleAck(void)
{
    PmsFifo * _CONST *fifoPtrTab_ptr;
    word  i;

    T_MOD_ENTRY(PMS_32);

    if( pms.needPrealloc )
    {
        MbmFree(NULL); /* Try to pre-allocate */
    }

    /*!
     * Acknowledge anything pending if able. The fifoPtrPrioTab is sorted in
     * prioritized order.
     */
    fifoPtrTab_ptr = fifoPtrPrioTab;
    i = (word)1; /* Index to buffer pointers in the preallocated array */
    do
    {
        PmsFifo *fifo_ptr = *fifoPtrTab_ptr;

        /* Send acknowledge if a FIFO has one pending and also an RX buffer available */
        #if defined(MDP_MIN) && !defined(ADS_MIN)
        /*
         * With MDP_MIN alone MDP should be acknowledged but does not need a
         * pre-allocated buffer to be ack'ed (discarded when received).
         */
        if( (PMS_STATUS_NONE != fifo_ptr->rxStatus) && (pms.rxPreAllocPtrTab[i] || (&mdpFifo == fifo_ptr)) )
        #else
        if( (PMS_STATUS_NONE != fifo_ptr->rxStatus) && pms.rxPreAllocPtrTab[i] )
        #endif
        {
            HMBMBUF hBuf = &fifo_ptr->ack;   /* Dedicated acknowledge buffer */

            WAIT4MX(MX_PMS_CS);
            if( MBM_GET_STATUS(hBuf) & MBM_STAT_RSVD )      /* Already busy? */
            {
                hBuf = NULL;
                pms.needRetriggerAck = 1;                  /* retrigger PMS_P_RX_ACK in PmsTxRelease() */
            }
            else
            {
                MBM_SET_STATUS_BITS(hBuf, MBM_STAT_RSVD);   /* Available, reserve */
            }
            REL_MX(MX_PMS_CS);

            if( hBuf )
            {
                hBuf->msg.ctrl = fifo_ptr->rxStatus;
                fifo_ptr->rxStatus = PMS_STATUS_NONE;
                PmsSendCommand(fifo_ptr, hBuf);
            }
        }
        i++;                /* Next corresponding preallocated buffer */
        fifoPtrTab_ptr++;   /* Next FIFO */
    } while( *fifoPtrTab_ptr );

    T_MOD_EXIT(PMS_32);
}
#endif



#ifdef PMS_44
void PmsFlushMsgTxBuffer(void)
{
    HMBMBUF  handle, next;
    TMsgTx  *msg_ptr;
    word     num;
    MbmQueue tempQ; /* MbmFree may not be called with MX_PMS_Q locked,
                     * make a temporary queue for the nodes to free.
                     */

    T_LIB_ENTRY(PMS_44);

    num = 0;

    MbmQueueInit(&tempQ, MX_PMS_Q); /* Does not need a semaphore, but API requires */


    WAIT4MX(MX_PMS_Q);

    handle = _MbmPeek(&(mcmFifo.msgQ));                 /* get head element of queue */

    while( NULL != handle )                             /* end of list reached? */
    {
        if (handle->next_ptr != (struct MbmBuf *)(&mcmFifo.msgQ))     /* last element of list ? */
        {
            next = handle->next_ptr;
        }
        else
        {
            next = NULL;                                /* last element reached */
        }

        msg_ptr = MBM_GET_CTRL_TX_PTR(handle);          /* get pointer to msg */

        if(MSG_TX_FILTER_CANCEL == ConfigStateFilterV2(msg_ptr) )
        {
            ++num;
            _MbmUnlink(handle);
            _MbmEnqueue(&tempQ, handle); /* Shall be deleted */
        }

        handle = next;
    }
    REL_MX(MX_PMS_Q);

    do
    {
        /* The tempQ is local, and hence does not need to lock the semaphore, so
         * access can be optimized with _MbmDequeue.
         */
        handle = _MbmDequeue(&tempQ);
        if( NULL != handle )
        {
            MbmFree(handle);
        }
    } while( NULL != handle );

    if ( pmsInitData_ptr->cbTxBufferFlushed_fptr && (num > 0) )
    {
        pmsInitData_ptr->cbTxBufferFlushed_fptr(num);
    }

    T_LIB_EXIT(PMS_44);
}
#endif



#ifdef PMS_45
void PmsTxStarted(HMBMBUF handle)
{
    TDataTx *msg_ptr;
    byte     tel_id;

    T_API_ENTRY(PMS_45);

    ASSERT(handle);
    msg_ptr = MBM_GET_DATA_TX_PTR(handle);
    ASSERT(msg_ptr);

    tel_id  = msg_ptr->Data[4];
    tel_id &= 0xF0;

    if ((PMS_TELID_80 == tel_id) || (PMS_TELID_90 == tel_id))
    {
        MhpTxStarted(msg_ptr);
    }

    T_API_EXIT(PMS_45);
    return;
}
#endif

#ifdef PMS_46
void PmsPrepareReInit(void)
{
    T_LIB_ENTRY(PMS_46);

    /* Release all externally allocated memory for RX messages. */

    #ifdef MSG_RX_USER_PAYLOAD
        #ifndef PMS_RX_NOSEG
        MbmFlush(&rxSegQ);
        #endif
    #endif

    #ifdef MSG_TX_USER_PAYLOAD
    PmsFlushTxQueue(&mcmFifo.msgQ);
    PmsFlushTxQueue(&mcmFifo.pendingQ);
    #endif

    #ifdef DATA_TX_USER_PAYLOAD
    PmsFlushTxQueue(&mdpFifo.msgQ);
    PmsFlushTxQueue(&mdpFifo.pendingQ);
    #endif

    /* notify LLD to stop RX/TX */
    if( pms.ifacesStarted )
    {
        pms.ifacesStarted = 0;

        if (pmsInitData_ptr->stopIfaces_fptr)
        {
            pmsInitData_ptr->stopIfaces_fptr();
        }
    }

    T_LIB_EXIT(PMS_46);
}
#endif

#ifdef PMS_47
/*
--------------------------------------------------------------------------------
 Function:      PmsFlushTxQueue(...)

 Description:   Flushes any TX queue, calling the supplied function if a flushed
                message has external payload.

 Input(s):      q_ptr   : Queue to flush.

 Return(s):     -
--------------------------------------------------------------------------------
*/
void PmsFlushTxQueue(MbmQueue *q_ptr)
{
    HMBMBUF handle;

    T_LIB_ENTRY(PMS_47);
    do
    {
        handle = MbmDequeue(q_ptr);

        if( handle )
        {
            #ifndef PMS_TX_NOSEG
            if( MBM_GET_TYPE(handle) & MBM_TYPE_SHADOW )
            {
                /* The mcmFifo.pendingQ may contain "shadows" for segmented messages */
                WAIT4MX(MX_PMS_Q);
                _PMS_FREE_SHADOW((MbmShadow *)handle);
                REL_MX(MX_PMS_Q);
            }
            else
            #endif
            {
                MbmFree(handle);
            }
        }
    } while( handle );
    T_LIB_EXIT(PMS_47);
}
#endif

#ifdef PMS_50
bool PmsDiscardRx(HMBMBUF handle)
{
    bool retval;
    MbmCtrlRx *rx_ptr;

    T_LIB_ENTRY(PMS_50);

    rx_ptr = MBM_GET_CTRL_RX_PTR(handle);

    if( rx_ptr->Rcv_Type & MSG_RCV_TYPE_BROADCAST )
    {
        retval = MNS_TRUE; /* Broadcast, no error message */
    }
    else
    {
        WAIT4MX(MX_PMS_CS);
        if( !(MBM_GET_STATUS(&segErrMsg) & MBM_STAT_RSVD) )
        {
            MBM_SET_STATUS_BITS(&segErrMsg, MBM_STAT_RSVD);
            retval = MNS_TRUE;
        }
        else
        {
            retval = MNS_FALSE;
        }
        REL_MX(MX_PMS_CS);

        if( MNS_FALSE != retval )
        {
            segErrMsg.msg.msgTx.Tgt_Adr   = rx_ptr->Src_Adr;
            segErrMsg.msg.msgTx.FBlock_ID = rx_ptr->FBlock_ID;
            segErrMsg.msg.msgTx.Inst_ID   = rx_ptr->Inst_ID;
            segErrMsg.msg.msgTx.Func_ID   = rx_ptr->Func_ID;
            /* Operation, Length, Retries and Data[0] already set */
            segErrMsg.msg.msgTx.Data[1]   = MSG_ERR_4;

            PmsSendBypass(&segErrMsg, PmsSegErrorTxStatus, MNS_TRUE);
        }
    }

    if( MNS_FALSE != retval )
    {
        if( NULL != pmsInitData_ptr->cbRxErr_fptr )
        {
            pmsInitData_ptr->cbRxErr_fptr(MSG_ERR_4, rx_ptr);
        }
        MbmFree(handle);
    }

    T_LIB_EXIT(PMS_50);

    return( retval );
}
#endif

#ifdef PMS_51
static void PmsFreeUserTxPayload(byte *data_ptr, HMBMBUF handle)
{
    T_MOD_ENTRY(PMS_51);

    if( !(MBM_GET_STATUS(handle) & MBM_STAT_NO_USER_FREE) )
    {
        if( (NULL != data_ptr) && MBM_PTR_IS_EXTERNAL(data_ptr) )
        {
            #ifdef MSG_TX_USER_PAYLOAD
            if( MBM_TYPE_IS_CTRL(handle) )
            {
                #ifdef MSG_TX_USER_PAYLOAD_EXT_CB
                if(  NULL != pmsInitData_ptr->cbFreeUserTxMsgPayloadExt_fptr )
                {
                    if ( MBM_STAT_USE_BACKUP == (MBM_STAT_USE_BACKUP & MBM_GET_STATUS(handle)) )
                    {
                        /* message was converted to Rx in MsgTxFinal and msg_backup contains the former TxStruct */
                        ASSERT(data_ptr == handle->msg_backup.msgTx.Data);
                        pmsInitData_ptr->cbFreeUserTxMsgPayloadExt_fptr(&(handle->msg_backup.msgTx));
                    }
                    else
                    {
                        ASSERT(data_ptr == handle->msg.msgTx.Data);
                        pmsInitData_ptr->cbFreeUserTxMsgPayloadExt_fptr(MBM_GET_CTRL_TX_PTR(handle));
                    }
                }

                MBM_CLR_STATUS_BITS(handle, MBM_STAT_USE_BACKUP);

                #else
                if(  NULL != pmsInitData_ptr->cbFreeUserTxMsgPayload_fptr )
                {
                    pmsInitData_ptr->cbFreeUserTxMsgPayload_fptr(data_ptr);
                }
                #endif
            }
              #ifdef DATA_TX_USER_PAYLOAD
            else
              #endif
            #endif

            #ifdef DATA_TX_USER_PAYLOAD
            if( MBM_TYPE_IS_DATA(handle) )
            {
                #ifdef DATA_TX_USER_PAYLOAD_EXT_CB
                if( NULL != pmsInitData_ptr->cbFreeUserTxDataPayloadExt_fptr )
                {
                    pmsInitData_ptr->cbFreeUserTxDataPayloadExt_fptr(MBM_GET_DATA_TX_PTR(handle));
                }
                #else
                if( NULL != pmsInitData_ptr->cbFreeUserTxDataPayload_fptr )
                {
                    pmsInitData_ptr->cbFreeUserTxDataPayload_fptr(data_ptr);
                }
                #endif
            }
            #endif
            else
            {
                FAILED_ASSERT(); /* An unexpected buffer type */
            }
        }
    }
    else
    {
        /* Clear bit in case original buffer is converted */
        MBM_CLR_STATUS_BITS(handle, MBM_STAT_NO_USER_FREE);
    }

    T_MOD_EXIT(PMS_51);
}
#endif

#ifdef PMS_52
/*
--------------------------------------------------------------------------------
 Function:      PmsDiscardPendingRx()

 Description:   Discards the last received message from the payload waiting
                queue and unfinished segments

 Return(s):     MNS_TRUE if a buffer was freed, else MNS_FALSE
--------------------------------------------------------------------------------
*/
static bool PmsDiscardPendingRx(void)
{
    HMBMBUF   handle;
    MbmQueue *q_ptr;
    bool      result;

    T_MOD_ENTRY(PMS_52);

    result = MNS_FALSE;

  #ifdef MSG_RX_USER_PAYLOAD
    q_ptr = &pms.rxPayloadWaitingQ;
    handle = MbmDequeueLast(q_ptr);

    #ifndef PMS_RX_NOSEG
    if( !handle )
    {
        q_ptr = &rxSegQ;
        handle = MbmDequeueLast(q_ptr);
    }
    #endif

  #else /* #ifdef MSG_RX_USER_PAYLOAD */
    #ifdef PMS_RX_NOSEG
      #error This function is not needed, PMS_52 should not be defined
    #endif

    q_ptr = &rxSegQ;
    handle = MbmDequeueLast(q_ptr);
  #endif

    if (handle)
    {
        result = PmsDiscardRx(handle);

        if( MNS_FALSE == result )
        {
            MbmEnqueue(q_ptr, handle);
        }
    }

    T_MOD_EXIT(PMS_52);

    return(result);
}
#endif

#ifdef PMS_55
void PmsExtendSyncTimeout(void)
{
    T_LIB_ENTRY(PMS_55);
    pmsSyncTimeout = PMS_T_SYNC_EXT;
    T_LIB_EXIT(PMS_55);
}
#endif

#ifdef PMS_56
void PmsHandleRetryParamsStatus(TMsgRx *msg_ptr)        /*lint -esym( 818, msg_ptr ) function must match TMisHandlerFuncPtr*/
{
    T_LIB_ENTRY(PMS_56);
    ASSERT(msg_ptr);

    WAIT4MX(MX_PMS_CS);
    pms.low_level_retries = msg_ptr->Data[1];
    REL_MX(MX_PMS_CS);

    T_LIB_EXIT(PMS_56);
}                                                       /*lint +esym( 818, msg_ptr ) function must match TMisHandlerFuncPtr*/
#endif

#ifdef PMS_57
void PmsHandleMidLevelRetriesStatus(TMsgRx *msg_ptr)    /*lint -esym( 818, msg_ptr ) function must match TMisHandlerFuncPtr*/
{
    T_LIB_ENTRY(PMS_57);
    ASSERT(msg_ptr);

    WAIT4MX(MX_PMS_CS);
    pms.mid_level_retries = msg_ptr->Data[2];
    REL_MX(MX_PMS_CS);

    T_LIB_EXIT(PMS_57);
}                                                       /*lint +esym( 818, msg_ptr ) function must match TMisHandlerFuncPtr*/
#endif

#ifdef PMS_58
void PmsInsertRetryValues(TMsgTx* tx_ptr)
{
    T_LIB_ENTRY(PMS_58);
    ASSERT(tx_ptr);
    WAIT4MX(MX_PMS_CS);
    tx_ptr->MidLevelRetries = pms.mid_level_retries;
    tx_ptr->LowLevelRetries = pms.low_level_retries;
    REL_MX(MX_PMS_CS);
    T_LIB_EXIT(PMS_58);
}
#endif


#ifdef PMS_61
static void PmsTxSyncSegmented(void)
{
    HMBMBUF hCur;
    PmsTxStatusHandler cbTxStatus_fptr;
    byte cb_res;

    T_MOD_ENTRY(PMS_61);

    WAIT4MX(MX_PMS_Q);
    hCur = mcmFifo.msgQ.next_ptr;

    #ifdef AMS_TX_BYPASS_FILTER
    while(((HMBMBUF)&mcmFifo.msgQ != hCur) && (hCur->type & MBM_STAT_TX_BYPASS))
    {
        hCur = hCur->next_ptr;                               /* skip all bypassed messages */
    }
    #endif

    if( ((HMBMBUF)&mcmFifo.msgQ != hCur) && (MBM_STAT_TX_SEG & hCur->type)
        && (MBM_STAT_TX_SENT & hCur->type) )
    {
        _MbmUnlink(hCur);    /* unlink next message if it is segmented and already started */
    }
    else
    {
        hCur = NULL;
    }
    REL_MX(MX_PMS_Q);

    if (hCur)
    {
        cbTxStatus_fptr = hCur->cbTxStatus_fptr;                     /* notify application */

        if (cbTxStatus_fptr)
        {
            cb_res = cbTxStatus_fptr(hCur, XMIT_SYNC);
            ASSERT((PMS_RELEASE == cb_res) || (PMS_RETRY == cb_res));

            if (PMS_RETRY == cb_res)
            {
                PmsSend(hCur, hCur->cbTxStatus_fptr);/* enqueue message if retry is wanted */
            }
        }
        else
        {
            MbmFree(hCur);
        }
    }

    T_MOD_EXIT(PMS_61);
}
#endif

#ifdef PMS_62
void PmsInitExternalBuf(HMBMBUF handle, byte *payload, word size, word type)
{
    word reserved;

    T_LIB_ENTRY(PMS_62);

    ASSERT(handle);

    if(MBM_TYPE_CTRL_TX == type)
    {
        reserved = (word)PMS_CTRL_HDR_MAX_SIZE;
    }
    #ifdef ADS_MIN
    else if(MBM_TYPE_DATA_TX == type)
    {
        reserved = (word)PMS_DATA_HDR_MAX_SIZE;
    }
    #endif
    else
    {
        reserved = (word)0;
    }

    handle->start_ptr = payload;
    handle->type      = type;
    handle->size      = size;
    MbmReserve(handle, reserved);
    PmsFillDefaultHeader(handle);

    T_LIB_EXIT(PMS_62);
}
#endif


#ifdef PMS_63
static _INLINE void PmsFireBufFreed(void)
{
    pms.fire_buf_freed = MNS_TRUE;
}
#endif


#endif /* #ifdef PACKET_COMMUNICATION_ONLY */
