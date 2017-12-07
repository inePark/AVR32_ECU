/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Private declarations and definitions of the Socket Connection 
                Manager Wrapper (WSCM)
File:           wscm_pv.h
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
  * \brief      Private declarations and definitions of the Socket Connection 
  *             Manager Wrapper (WSCM)
  */

#ifndef _WSCM_PV_H
#define _WSCM_PV_H

/*
==============================================================================
    Includes
==============================================================================
*/
#include "wscm.h"

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
#define SCM_SCERROR_MLB             ((byte) 0x01)
#define SCM_SCERROR_MOST            ((byte) 0x02)

#define SCM_HANDLE_PACKET_IN        ((byte) 0xF0)
#define SCM_HANDLE_PACKET_OUT       ((byte) 0xF1)
#define SCM_HANDLE_INVALID          ((byte) 0xFF)

#define SCM_MLB_PM_MAX_BLOCKWIDTH   ((word)384)

#ifdef PMS_RX_OPT3
    #define SCM_NUM_CONTROL 12
#else
    #define SCM_NUM_CONTROL 8
#endif

#define SCM_MLB_PM_BW_256_FS   ((word)(28 - SCM_NUM_CONTROL))
#define SCM_MLB_PM_BW_512_FS   ((word)(60 - SCM_NUM_CONTROL))
#define SCM_MLB_PM_BW_1024_FS  ((word)(124 - SCM_NUM_CONTROL))
#define SCM_MLB_PM_BW_2048_FS  ((word)(228 - SCM_NUM_CONTROL))
#define SCM_MLB_PM_BW_3072_FS  ((word)(344 - SCM_NUM_CONTROL))
#define SCM_MLB_PM_BW_4096_FS  ((word)(464 - SCM_NUM_CONTROL))
#define SCM_MLB_PM_BW_6144_FS  ((word)(640 - SCM_NUM_CONTROL))
#define SCM_MLB_PM_BW_8192_FS  ((word)768)

#define SCM_PM_KILL_WAIT_STATE      ((word) 0x00FF)
#define SCM_PM_KILL_STATE           ((word) 0xFF00)

#define SCM_PM_NEW_BORN             ((word) 0x0000)
#define SCM_PM_C_MLB_PORT           ((word) 0x0001)
#define SCM_PM_D_IN_SOCKET          ((word) 0x0002)
#define SCM_PM_C_IN_SOCKET          ((word) 0x0004)
#define SCM_PM_D_OUT_SOCKET         ((word) 0x0008)
#define SCM_PM_C_OUT_SOCKET         ((word) 0x0010)
#define SCM_PM_WC_MLB_PORT          ((word) 0x0100)
#define SCM_PM_WD_IN_SOCKET         ((word) 0x0200)
#define SCM_PM_WC_IN_SOCKET         ((word) 0x0400)
#define SCM_PM_WD_OUT_SOCKET        ((word) 0x0800)
#define SCM_PM_WC_OUT_SOCKET        ((word) 0x1000)

#define SCM_PA_NOT                  ((byte) 0x00)
#define SCM_PA_CL                   ((byte) 0x01)
#define SCM_PA_SBC                  ((byte) 0x02)
#define SCM_PA_CL_MULTCON           ((byte) 0x03)

#define SCM_PM_W_BANDWIDTH          ((word) 0x0100)
#define SCM_PM_WAIT                 ((word) (SCM_PM_C_IN_SOCKET | SCM_PM_C_OUT_SOCKET | SCM_PM_W_BANDWIDTH))
#define TIME_PM_RECHECK             200

#define SCM_PM_RUN(func) if(ERR_BUFOV == func) { reschedule = MNS_TRUE; } else { set_wait_state = MNS_TRUE; }


/*
==============================================================================
    Module Internal Structures
==============================================================================
*/

#ifdef SCM_MIN
    typedef struct WscmData
    {
        word      pending_events;
        word      latest_handled_event;
        word     *result_list_ptr;
        TMnsStdCB *cb_ptr;
        word      cb_func_id;
        byte      mute_mode;
        bool      api_locked;
        bool      api_ext;

        struct TScmConfig *cfg_ptr;
        TScmBoundaryInfo bandwidth;

        struct TScmPMInfo
        {
            struct TScmPMSockets
            {
                byte in;
                byte out;

            } handle;

            struct TScmPMDesc
            {
                TScmSocketDesc in;
                TScmSocketDesc out;
                TScmPortDesc portdesc;

            } desc;

            word state;
            word statebuf;
            TMostTimer timer;
            bool mediaLBportOpen;
        } pm;

    } WscmData;


    typedef struct WscmErrCodes
    {
        word func_id;
        byte func_code;

    } WscmErrCodes;

#endif



/*
==============================================================================
    Module Internal Function Prototypes
==============================================================================
*/
#ifdef SCM_3
    static word ScmGetNextEventToHandle(void);
#endif

#ifdef SCM_4
    void ScmGoProtected(void);
#endif

#ifdef SCM_36
    static void ScmAssembleErrResList(TMsgRx *msg_ptr, byte *len_ptr,
                                      word** list_ptr_ptr);
#endif

#ifdef SCM_38
    static void ScmPMCheck(void);
#endif

#ifdef SCM_39
    static void ScmPMService(void);
#endif

#ifdef SCM_40
    static void ScmPMCreateResult(TMnsResult result, byte socket_handle,
                              byte list_len, word *list_ptr);
#endif

#ifdef SCM_41
    static void ScmPMDestroyResult(TMnsResult result);
#endif

#ifdef SCM_43
    static void ScmPMRecheck(word event);
#endif

#ifdef SCM_45
    static void ScmPMOpenPortResult(TMnsResult event);
#endif


#endif /* #ifndef _WSCM_PV_H */

/*
==============================================================================
    End Of File
==============================================================================
*/
