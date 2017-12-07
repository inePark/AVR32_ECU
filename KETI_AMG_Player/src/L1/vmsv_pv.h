/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Private declarations and definitions of the Virtual MOST 
                Supervisor (VMSV)
File:           vmsv_pv.h
Version:        3.0.x-SR-1  
Language:       C
Author(s):      R.Hanke
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
  * \brief      Private declarations and definitions of the Virtual MOST 
  *             Supervisor (VMSV)
  */

#ifndef _VMSV_PV_H
#define _VMSV_PV_H


/*
==============================================================================
    Includes
==============================================================================
*/

#include "vmsv.h"



/*
==============================================================================
    Rules
==============================================================================
*/

#ifdef VMSV_MIN
  
#endif  /* #ifdef VMSV_MIN */

/*
==============================================================================
    Module Internal Definitions
==============================================================================
*/


/*
==============================================================================
    Module Internal Macros
==============================================================================
*/

#define VMSV_NWSTARTUP_COMPLETE     ((byte) 0x00)
#define VMSV_NWSTARTUP_DEVMODE      ((byte) 0x01)
#define VMSV_NWSTARTUP_NWSTARTUP    ((byte) 0x02)
#define VMSV_NWSTARTUP_RBD          ((byte) 0x04)

#define VMSV_SHADOW_NONE                ((byte) 0x00)
#define VMSV_SHADOW_NISTATE_CHANGED     ((byte) 0x01)
#define VMSV_SHADOW_NIEVENT_CHANGED     ((byte) 0x02)
#define VMSV_SHADOW_LOCKSTATE_CHANGED   ((byte) 0x04)
#define VMSV_SHADOW_NCEDELAYED_CHANGED  ((byte) 0x08)
#define VMSV_SHADOW_RBDRESULT_CHANGED   ((byte) 0x10)
#define VMSV_SHADOW_PMISTATE_CHANGED    ((byte) 0x20)
#define VMSV_SHADOW_ATTENUATION_CHANGED ((byte) 0x40)

#define VMSV_CP_DO_NOTHING              ((byte) 0x00)
#define VMSV_CP_INJECTED                ((byte) 0x01)
#define VMSV_CP_WAITING                 ((byte) 0x02)

#define VMSV_RETRY_FBLOCKIDS_GET        ((word)0x0001)
#define VMSV_RETRY_ATTENUATION          ((word)0x0002)
#define VMSV_RETRY_SYSERRMONITOR        ((word)0x0004)
#define VMSV_RETRY_CFG_STATE            ((word)0x0008)

#ifdef MSV_DIAG_RESULT_MSG
    #define VMSV_RETRY_RBDRESULT        ((word)0x0010)
#endif

#define VMSV_RETRY_REQUEST_NETONTIMER   ((word)0x0020)
#define VMSV_RETRY_REQUEST_NIWAKEUPMODE ((word)0x0040)
#define VMSV_RETRY_SET_NIWAKEUPMODE     ((word)0x0080)
#define VMSV_RETRY_PLT                  ((word)0x0100)
#define VMSV_RETRY_SSORESULT            ((word)0x0200)

#define VMSV_COMM_OFF                   ((byte) 0x00)
#define VMSV_COMM_CALL_CB               ((byte) 0x01)
#define VMSV_COMM_STARTNW               ((byte) 0x02)
#define VMSV_COMM_BROADCAST             ((byte) 0x03)
#define VMSV_COMM_RESTORE_DEVMODE       ((byte) 0x04)
#define VMSV_COMM_WAIT_FOR_MSG          ((byte) 0x05)
#define VMSV_COMM_WAIT_AFTER_SEND       ((byte) 0x06)

#define VMSV_PLT_OFF                    ((byte) 0x00)
#define VMSV_PLT_WAIT_TX                ((byte) 0x01)
#define VMSV_PLT_START                  ((byte) 0x02)
#define VMSV_PLT_LEAD_IN                ((byte) 0x03)
#define VMSV_PLT_TEST1                  ((byte) 0x04)
#define VMSV_PLT_TEST2                  ((byte) 0x05)
#define VMSV_PLT_LEAD_OUT               ((byte) 0x06)
#define VMSV_PLT_END                    ((byte) 0x07)

#define VMSV_PLT_TYPE_AUTO              ((byte) 0x00)
#define VMSV_PLT_TYPE_MASTER            ((byte) 0x01)
#define VMSV_PLT_TYPE_SLAVE             ((byte) 0x02)

#define VMSV_TIMEOUT_ATTENUATION        ((word) 0x0080)


/* Values of NIWakeUpMode */
#define VMSV_NIWAKEUPMODE_DEVICEAUTOMODE    ((byte) 0x01)
#define VMSV_NIWAKEUPMODE_WAKEUPBAN         ((byte) 0x02)


#define SCHEDULE_RETRY(flag) { TAKE_EVENTS(); vmsv.retry |= flag; GIVE_EVENTS(); }
#define RETRY_DONE(flag)     { TAKE_EVENTS(); vmsv.retry &= ~flag; GIVE_EVENTS(); }
#define PROCESS_RETRY(retry_flag, retry_func) { if ((ERR_NO == result) && (retry & retry_flag)) { RETRY_DONE(retry_flag); result = retry_func(); }}


/*
==============================================================================
    Module Internal Structures
==============================================================================
*/

#ifdef VMSV_MIN

    /*! Data of the vMSV module */
    typedef struct VmsvData
    {
        /*! vMSV event masks for processing by the MNS Kernel */
        struct VmsvDataEvents
        {
            /*! 16-bit mask of pending events. */
            word pending;

            /*! 16-bit mask of the latest served event (1 or 0 bits set). */
            word latest;

        } events;

        /*! State information used by MostStartup() */
        struct VmsvDataNWStartup
        {
            /*! State of the NWStartup process (combined calls of INIC.NWStartUp
              * INIC.DeviceMode and INIC.RBDTrigger).
              */
            byte state;

            /*! Target device mode for the NWStartup process. */
            byte dev_mode;

            /*! Wheter or not to use diagnosis during startup. */
            bool diagnosis;

            #ifdef MSV_DIAG_RESULT_MSG
                byte       comm_state;
                TMostTimer comm_timer;
                byte       saved_dev_mode;
            #endif
        } nwstartup;

        /*! Shadow values of INIC properties needed by vMSV. */
        struct VmsvDataShadow
        {
            /*! 8-bit change mask for the shadow values, to enable delayed
              * calling of the callbacks.
              */
            byte changed;

            /*! Shadow of INIC.NIState */
            byte nistate;

            /*! Flag if MSVAL_E_NETON was already reported. The event
              * depends on a valid Bandwith.
              */
            bool neton_reported;

            /*! Flag to see wheter MSVAL_E_MPRCHANGE must be reported
              * after going into "net on"
              */
            bool delayed_mpr;

            /*! Flag to see whether MSVAL_E_MPR* needs to be reported
              * after going into "net on"
              */
            bool delayed_nce;

            /*! Late shadow of INIC.NIState
              * ... for generating MSVAL_ERR_INIT_ERROR
              */
            byte late_nistate;

            /*! Shadow of INIC.NIEvent */
            byte nievent;

            /*! Shadow of INIC.NCEDelayed (index 0 = new MPR & 1 = old MPR ) */
            byte ncedelayed[2];

            /*! Shadow of INIC.RBDResult */
            byte rbdresult[2];

            #ifdef _OS81110_SSO
            /*! Shadow of INIC.SSOResult */
            byte ssoresult;
            #endif

            /*! Shadow of INIC.DeviceMode */
            byte devmode;

            /*! Shadow of INIC.NCState State */
            byte ncstate;

            /*! Shadow of INIC.NCState State Flags Field */
            byte ncstate_flags;

            /*! Shadow of INIC.NCState NWMAddress */
            word nwm_addr;

            /*! Shadow of INIC.LockState */
            byte lockstate;

            /*! Shadow of the past and current NodePos */
            word node_pos_check;

            struct TVmsvPMIShadow
            {
                /*! Shadow of the PMIState */
                byte state;

                /*! Shadow of the last PMIState events */
                byte events;

            } pmi;



            #ifdef MSV_DIAG_RESULT_MSG
            /*! Shadow of INIC.NIWakeUpMode */
            struct TVmsvNIWakeUpMode
            {
                byte mode;
                byte request;
            } ni_wakeup_mode;
            #endif
        } shadow;

        struct TVmsvConfig *cfg_ptr;

        void (*update_nwm_address_fptr)(word address);
        void (*store_error_info2_fptr)(byte info);

        struct TVmsvInitPhase
        {
            TMostTimer timer;

            byte state;

        } check_phase;

        bool attached;
        bool fblock_ids_get_received;
        bool fblock_ids_status_sent;

        /*! @field retry is the retry flags bit field */
        word retry;




        /*! @struct holds the information needed by MostGetSysErrMonitor() */
        struct TVmsvSysErrMonitor
        {
            /*! @field is not NULL while MostGetSysErrMonitor() is busy */
            TMnsStdCB *cb_ptr;

            /*! @field contains the bit mask of received errors from the INIC */
            byte bit_mask;

        } sys_err_monitor;

        /* @callback to be called, when INIC.NetOnTimer.Status is received */
        TVmsvNetOnCB *net_on_cb_ptr;

      #ifdef NS_MSV_ET
        struct TVmsvPhysicalLayerTest
        {
            bool  test_done;

            byte  type;
            word  lead_in;
            word  lead_out;
            dword duration;

            byte saved_dev_mode;

            byte test_state;
            TMostTimer state_timer;
        } plt;

        struct TVmsvPhysicalLayerResult
        {
            bool  lock_status;
            word  err_count;
        } plr;
      #endif

        /* @holds the emergency condition status */
        bool emergency_state;

    } VmsvData;

#endif  /* #ifdef VMSV_MIN */


/*
==============================================================================
    Module Internal Function Prototypes
==============================================================================
*/

#ifdef VMSV_3
    static word VmsvGetNextEventToHandle(void);
#endif

#ifdef VMSV_5
    static void VmsvNWStartUpRetry(void);
#endif

#ifdef VMSV_11
    static void VmsvEHCIGoProtected(void);
#endif

#ifdef VMSV_12
    static void VmsvEHCIGoAttached(void);
#endif

#ifdef VMSV_25
    static void VmsvFireCallbacks(void);
#endif

#ifdef VMSV_26
    static void VmsvHandleNIStateChange(void);
#endif

#ifdef VMSV_27
    static void VmsvHandleNIEventChange(void);
#endif

#ifdef VMSV_28
    static void VmsvHandleNCEDelayedChange(void);
#endif

#ifdef VMSV_29
    static void VmsvHandleLockStateChange(void);
#endif

#ifdef VMSV_30
    static void VmsvHandleRBDResultChange(void);
#endif

#ifdef VMSV_35
    static void VmsvHandleNCStateChange(void);
#endif

#ifdef VMSV_37
    static byte VmsvInjectFBlockIDsGet(void);
#endif

#ifdef VMSV_38
    static byte VmsvRetry(void);
#endif

#ifdef VMSV_45
    static void VmsvCheckForNpr(bool force);
#endif

#ifdef VMSV_46
    static void VmsvReportNetOn(void);
#endif

#ifdef VMSV_48
    static void VmsvHandlePMIStateChange(void);
#endif

#ifdef VMSV_55
    static byte VmsvRequestSysErrMonitor(void);
#endif

#ifdef VMSV_57
    static void VmsvReportSysErrMonitor(void);
#endif

#ifdef VMSV_59
    static byte VmsvInjectCfgState(void);
#endif

#ifdef VMSV_60
    static void VmsvEHCIGoSemiProtected(void);
#endif

#ifdef VMSV_66
    static void VmsvCommRBDResult(byte result[2]);
#endif

#ifdef VMSV_67
    static void VmsvCommRBDResultPos0(byte go_to_state);
#endif

#ifdef VMSV_68
    static void VmsvCommRBDResultPosX(void);
#endif

#ifdef VMSV_69
    static byte VmsvBroadcastRBDResult(void);
#endif
    
#ifdef VMSV_72
    static byte VmsvRequestNetOnTime(void);
#endif

#ifdef VMSV_92
    static byte VmsvRequestNIWakeUpMode(void);
#endif

#ifdef VMSV_93
    static byte VmsvSetNIWakeUpMode(void);
#endif

#ifdef VMSV_95
    static byte VmsvPhysicalLayerTestProcedure(void);
#endif

#ifdef VMSV_98
    static byte VmsvPhysicalLayerTestTxFinal(HMBMBUF handle, byte status);
#endif

#ifdef VMSV_99
    static byte VmsvResetINICSSOResult(void);
#endif

#ifdef VMSV_101
    static _INLINE void VmsvSendMdmMsv2NwmConfStateNok(void);
#endif




#endif /* #ifndef _VMSV_PV_H */

/*
==============================================================================
    End Of File
==============================================================================
*/
