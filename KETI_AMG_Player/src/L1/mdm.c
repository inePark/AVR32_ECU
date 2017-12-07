/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Implementation of the MOST debug message module (MDM)
File:           mdm.c
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
  * \brief      Implementation of the MOST Debug Message Module (MDM).
  *
  * \details    This module contains functions to build and send out debug
  *             messages. These functions are used by the MNS as well as the
  *             application. The MDM module provides an interface to the MBM
  *             and the PMS module.
  *             The MBM module is used to handle the debug message queue.
  *             However, the message buffers are part of the MDM module.
  *             The PMS module is used to send the debug messages to the
  *             INIC.
  *
  * \remarks    The MNS uses the library function MdmMsgSendMns()
  *             The Application uses the API function MdmMsgSendApp()
  *
  *
  */


/*
==============================================================================
    Includes
==============================================================================
*/

#include "mbm.h"
#include "pms.h"
#include "mns.h"
#include "mdm.h"
#include "mdm_pv.h"




/*
==============================================================================
    NetServices trace: module trace macros
==============================================================================
*/

#if (MNS_DEBUG & NST_C_FUNC_ENTRIES)
    #define T_API_ENTRY(func)   MNS_TRACE((MNS_P_SRV_MDM, NST_E_FUNC_ENTRY_API, 1, func))
    #define T_LIB_ENTRY(func)   MNS_TRACE((MNS_P_SRV_MDM, NST_E_FUNC_ENTRY_LIB, 1, func))
    #define T_MOD_ENTRY(func)   MNS_TRACE((MNS_P_SRV_MDM, NST_E_FUNC_ENTRY_MOD, 1, func))
#else
    #define T_API_ENTRY(func)
    #define T_LIB_ENTRY(func)
    #define T_MOD_ENTRY(func)
#endif

#if (MNS_DEBUG & NST_C_FUNC_EXITS)
    #define T_API_EXIT(func)    MNS_TRACE((MNS_P_SRV_MDM, NST_E_FUNC_EXIT_API, 1, func))
    #define T_LIB_EXIT(func)    MNS_TRACE((MNS_P_SRV_MDM, NST_E_FUNC_EXIT_LIB, 1, func))
    #define T_MOD_EXIT(func)    MNS_TRACE((MNS_P_SRV_MDM, NST_E_FUNC_EXIT_MOD, 1, func))
#else
    #define T_API_EXIT(func)
    #define T_LIB_EXIT(func)
    #define T_MOD_EXIT(func)
#endif

#if (MNS_DEBUG & NST_C_INIT)
    typedef int T_INT;
    #define T_INIT()            MNS_TRACE(((T_INT)MNS_P_SRV_MDM, (T_INT)NST_E_INIT, (T_INT)0))
#else
    #define T_INIT()
#endif

#ifdef _lint
    /*lint -emacro(*,*ASSERT)*/
    #define FAILED_ASSERT()     return
    #define ASSERT(exp)         if(!(exp)) return
#else
  #if (MNS_DEBUG & NST_C_ASSERTS)
    #define FAILED_ASSERT()     MNS_TRACE((MNS_P_SRV_MDM, NST_E_ASSERT, 1, __LINE__))
    #define ASSERT(exp)         if(!(exp)) FAILED_ASSERT()
  #else
    #define FAILED_ASSERT()
    #define ASSERT(exp)
  #endif
#endif

#define TAKE_MDM()              WAIT4MX(MX_MDM_CS)
#define GIVE_MDM()              REL_MX(MX_MDM_CS)


/*
================================================================================
    Module Internal Variables
================================================================================
*/

#ifdef MDM_MIN
    /*! Data variable of the MDM module */
    static TMdmData mdm;
#endif  /* #ifdef MDM_MIN */



/*
==============================================================================
==============================================================================
    Module Implementation
==============================================================================
==============================================================================
*/
/*
==============================================================================
    API Function Implementations
==============================================================================
*/

#ifdef MDM_2
/*!
  * \brief      Prepares an application debug message for sending.
  *
  * \details    This function prepares an application debug message for
  *             sending. In a last step the message is passed to the PMS.
  *
  * \param[in]  params_ptr   Message parameter container
  *               - func_id      MOST function id
  *               - case_id      Debug case identifier
  *               - debug_level  Debug level
  *               - data_ptr     Pointer to the data (Byte stream;
  *                              big endian Byte order!)
  *               - length       Data length in number of bytes
  * \return     status/error
  *               ERR_NO:            Everything is in order
  *               ERR_PARAM:         Error! Wrong input parameter
  *               ERR_NOTAVAIL:      Error! Unkown MOST function id
  *               ERR_NOT_SUPPORTED: Info! Debug level too low
  *
  */
byte MdmMsgSendApp(const TMdmParamsApp * params_ptr)
{
    byte ret;
    word index;
    byte debug_level;

    T_API_ENTRY(MDM_2);

    if(MNS_FALSE == MdmValidatePtr((void *)params_ptr))                     /* Validate passed pointer */
    {
        ret = ERR_PARAM;                                                    /* Error: Pointer is NULL */
    }
    else
    {
        if( (MNS_FALSE == MdmValidateFuncId(params_ptr->func_id))       ||  /* Validate passed function id */
            (MNS_FALSE == MdmValidateCaseId(params_ptr->case_id))       ||  /* Validate passed case id */
            (MNS_FALSE == MdmValidateDbgLevel(params_ptr->debug_level)) ||  /* Validate passed debug level */
            (MNS_FALSE == MdmValidateLength(params_ptr->length)) )          /* Validate passed data length */
        {
            ret = ERR_PARAM;                                                /* Error: Wrong parameter */
        }
        else
        {
            TAKE_MDM();
            index = _MdmGetTblIndexApp(params_ptr->func_id);

            if(MDM_NOT_REGISTERED != index)                                 /* Is passed MOST function id registered ? */
            {
                debug_level = mdm.app_registry_tbl[index].debug_level;      /* Get associated debug level */
                GIVE_MDM();

                /* Check if debug level too low */
                if((debug_level < params_ptr->debug_level) || (MDM_CFG_LEVEL_NONE == params_ptr->debug_level))
                {
                    ret = ERR_NOT_SUPPORTED;                                /* Info: Debug level too low */
                }
                else
                {
                    /* Call module internal send routine */
                    MdmMsgSendInternal(MDM_CLASS_ID_APP,
                                       params_ptr->func_id,
                                       params_ptr->case_id,
                                       params_ptr->debug_level,
                                       params_ptr->data_ptr,
                                       params_ptr->length);

                    ret = ERR_NO;                                           /* Everything is in order */
                }
            }
            else
            {
                GIVE_MDM();
                ret = ERR_NOTAVAIL;                                         /* Error: Unknown function id (function id not registered) */
            }
        }
    }

    T_API_EXIT(MDM_2);

    return(ret);                                            /* Return status/error */
}
#endif  /* #ifdef MDM_2 */



#ifdef MDM_12
/*!
  * \brief      Set debug level for application function id.
  *
  * \details    This function sets the debug level for a passed
  *             MOST function id.
  *
  * \param[in]  func_id      MOST function id
  * \param[in]  debug_level  Debug level
  * \return     status/error
  *               ERR_NO:       Everything is in order
  *               ERR_PARAM:    Error! Wrong input parameter
  *               ERR_NOTAVAIL: Error! Unknown function id
  *
  */
byte MdmSetDbgLevelApp(word func_id, byte debug_level)
{
    word index;
    byte ret;

    T_API_ENTRY(MDM_12);

    if( (MNS_FALSE == MdmValidateDbgLevel(debug_level)) ||          /* Validate passed debug level */
        (MNS_FALSE == MdmValidateFuncId(func_id)) )                 /* Validate passed function id */
    {
        ret = ERR_PARAM;                                            /* Error: Wrong input parameter */
    }
    else
    {
        TAKE_MDM();
        index = _MdmGetTblIndexApp(func_id);                        /* Search MOST function id in registry table */

        if(MDM_NOT_REGISTERED != index)                             /* Is MOST function id registered ? */
        {
            mdm.app_registry_tbl[index].debug_level = debug_level;  /* Update debug level */
            ret = ERR_NO;                                           /* Update succeeded */
        }
        else
        {
            ret = ERR_NOTAVAIL;                                     /* Error: Unknown function id */
        }
        GIVE_MDM();

        T_API_EXIT(MDM_12);
    }

    return(ret);                                                    /* Return status/error */
}
#endif  /* #ifdef MDM_12 */



#ifdef MDM_13
/*!
  * \brief      Get debug level for passed application function id.
  *
  * \details    This function returns the debug level for a passed
  *             MOST function id.
  *
  * \param[in]  func_id      MOST function id
  * \return     debug_level/error
  *               debug level: if the function id is well-known
  *               0xFF         Error! Function id not registered
  *                            or function id is invalid
  *
  */
byte MdmGetDbgLevelApp(word func_id)
{
    word index;
    byte ret;

    T_API_ENTRY(MDM_13);

    if(MNS_FALSE == MdmValidateFuncId(func_id))             /* Validate passed function id */
    {
        ret = (byte)0xFF;                                   /* Error: Invalid MOST function id */
    }
    else
    {
        TAKE_MDM();
        index = _MdmGetTblIndexApp(func_id);                /* Search function id in registry table */

        if(MDM_NOT_REGISTERED != index)                     /* Is function id registered ? */
        {
            ret = mdm.app_registry_tbl[index].debug_level;  /* Return associated debug level */
        }
        else
        {
            ret = (byte)0xFF;                               /* Error: Unknown function id => Return 0xFF */
        }
        GIVE_MDM();
    }

    T_API_EXIT(MDM_13);

    return(ret);                                        /* Return status/error */
}
#endif  /* #ifdef MDM_13 */



#ifdef MDM_14
/*!
  * \brief      Register application function id.
  *
  * \details    This function registers the passed MOST application
  *             function id. For this purpose the function id is stored
  *             in the MDM registry table. Also, the passed debug level
  *             is stored.
  *
  * \param[in]  func_id      MOST function id
  * \param[in]  debug_level  Debug level
  * \return     status/error
  *               ERR_NO:          Everything is in order
  *               ERR_PARAM:       Error! Wrong input parameter
  *               ERR_BUFOV:       Error! No buffer available
  *               ERR_ALREADY_SET: Error! Passed function id already stored
  *
  */
byte MdmRegisterFuncIdApp(word func_id, byte debug_level)
{
    word i;
    word index;
    byte ret;

    T_API_ENTRY(MDM_14);

    if( (MNS_FALSE == MdmValidateFuncId(func_id)) ||                        /* Validate passed function id */
        (MNS_FALSE == MdmValidateDbgLevel(debug_level)) )                   /* Validate passed debug level */
    {
        ret = ERR_PARAM;                                                    /* Error: Wrong input parameter */
    }
    else
    {
        TAKE_MDM();
        if(MDM_CFG_APP_NUM_FUNC_IDS > mdm.app_registry_tbl_cnt)                 /* Buffer available ? */
        {
            index = _MdmGetTblIndexApp(func_id);                                /* Search function id in registry table */

            if(MDM_NOT_REGISTERED == index)                                     /* Function id not yet registered ? */
            {
                for(i=(word)0; i<MDM_CFG_APP_NUM_FUNC_IDS; i++)                 /* Search free table cell */
                {
                    if(mdm.app_registry_tbl[i].func_id == MDM_UNUSED)           /* Is table cell unused? */
                    {
                        mdm.app_registry_tbl[i].func_id     = func_id;          /* Register passed function id */
                        mdm.app_registry_tbl[i].debug_level = debug_level;      /* Set debug level */
                        mdm.app_registry_tbl_cnt++;                             /* Increment number of registered function ids */
                        break;
                    }
                }

                ASSERT(i < MDM_CFG_APP_NUM_FUNC_IDS);       /* Check for corrupt counter state (mdm.app_registry_tbl_cnt) */

                ret = ERR_NO;                               /* Registration succeeded */
            }
            else
            {
                ret = ERR_ALREADY_SET;                      /* Error: Passed function id already known */
            }
        }
        else
        {
            ret = ERR_BUFOV;                                /* Error: No buffer available */
        }
        GIVE_MDM();
    }

    T_API_EXIT(MDM_14);

    return(ret);                                            /* Return status/error */
}
#endif  /* #ifdef MDM_14 */



#ifdef MDM_15
/*!
  * \brief      Unregister application function id.
  *
  * \details    This function removes the passed MOST application
  *             function id from the MDM function id list.
  *
  * \param[in]  func_id      MOST function id
  * \return     status/error
  *               ERR_NO:       Everything is in order
  *               ERR_PARAM:    Error! Wrong input parameter
  *               ERR_NOTAVAIL: Error! Function id not registered
  *
  */
byte MdmUnregisterFuncIdApp(word func_id)
{
    word index;
    byte ret;

    T_API_ENTRY(MDM_15);

    if(MNS_FALSE == MdmValidateFuncId(func_id))                 /* Validate passed function id */
    {
        ret = ERR_PARAM;                                        /* Error: Invalid function id */
    }
    else
    {
        TAKE_MDM();
        index = _MdmGetTblIndexApp(func_id);                    /* Search function id in registry table */

        if(MDM_NOT_REGISTERED != index)                         /* Function id registered? */
        {
            mdm.app_registry_tbl[index].func_id = MDM_UNUSED;   /* Unregister passed function id */
            mdm.app_registry_tbl_cnt--;                         /* Decrement number of registered function ids */

            ret = ERR_NO;                                       /* Unregistration succeeded */
        }
        else
        {
            ret = ERR_NOTAVAIL;                                 /* Error: Passed function id unknown */
        }
        GIVE_MDM();
    }

    T_API_EXIT(MDM_15);

    return(ret);                                                /* Return status/error */
}
#endif  /* #ifdef MDM_15 */



#ifdef MDM_16
/*!
  * \brief      Get a list of function id/debug level pairs.
  *
  * \details    This function returns a list of function id/debug level pairs
  *             stored in the MDM registry table.
  *
  * \param[out] dbg_level_list    Pointer to debug level list (Pointer to word array)
  * \param[in]  max_list_entries  Size of the passed array "dbg_level_list"
  * \param[out] length_ptr        Pointer for number of list entries
  * \return     status/error
  *               ERR_NO:       At least one function is registered
  *               ERR_PARAM:    Error! Wrong input parameter
  *               ERR_NOTAVAIL: Info! List is empty
  *               ERR_BUFOV:    Info! Passed array too small to hold the total number 
  *                             of registered function ids
  *
  */
byte MdmGetDbgLevelListApp(word *dbg_level_list, word max_list_entries, word *length_ptr)
{
    word tmp;
    word i;
    byte ret;

    T_API_ENTRY(MDM_16);

    if( (MNS_FALSE == MdmValidatePtr(dbg_level_list)) ||            /* Validate dbg_level_list pointer */
        (MNS_FALSE == MdmValidatePtr(length_ptr)) )                 /* Validate length_ptr pointer */
    {
        ret = ERR_PARAM;                                            /* Error: Wrong input parameter */
    }
    else
    {
        *length_ptr = (word)0;

        TAKE_MDM();
        if((word)0 < mdm.app_registry_tbl_cnt)                      /* At least one function id in registry table ? */
        {
            ret = ERR_NO;                                           /* At least one function id registered */

            for(i=(word)0; i<MDM_CFG_APP_NUM_FUNC_IDS; i++)
            {
                if(mdm.app_registry_tbl[i].func_id != MDM_UNUSED)   /* Is table cell used? */
                {
                    if(*length_ptr < max_list_entries)              /* Check maximum size of passed array */
                    {
                        tmp = (word)((word)mdm.app_registry_tbl[i].debug_level << 12);      /* Debug Level (Bit 15...12) */
                        tmp |= (word)((word)0x0FFF & mdm.app_registry_tbl[i].func_id);      /* Function Id (Bit 11...0) */
                        *dbg_level_list++ = tmp;
                    }
                    else
                    {
                        ret = ERR_BUFOV;                            /* Info: Passed array too small */
                    }
                    (*length_ptr)++;
                }
            }
        }
        else
        {
            ret = ERR_NOTAVAIL;                                     /* Info: List is empty */
        }
        GIVE_MDM();
    }

    T_API_EXIT(MDM_16);

    return(ret);
}
#endif  /* #ifdef MDM_16 */



/*
================================================================================
    Library Function Implementations
================================================================================
*/

#ifdef MDM_0
/*!
  * \brief      Initializes the debug message module.
  *
  * \details    This function initilazes the message queue, the message buffer
  *             and other module parameters. Each buffer cell is enqueued in
  *             the message queue.
  *
  * \param[in]  is_ptr  Pointer to initialization structure for module MDM
  *
  */
void MdmInit(MdmInitStruct *is_ptr)
{
    byte i;

    T_LIB_ENTRY(MDM_0);

    ASSERT(NULL != is_ptr);

    T_INIT();

    TAKE_MDM();
    MDM_MEM_SET((byte *)&(mdm), (byte)0x00, (word)sizeof(mdm));         /* Clean-up data structure */
    GIVE_MDM();

    MbmQueueInit(&(mdm.msg_queue), (word)MX_MDM_Q);                     /* Initialize message queue */

    /* Initialize debug message buffer */
    for(i=(byte)0; i<MDM_CFG_NUM_BUF; i++)
    {
        TAKE_MDM();
        mdm.msg_buf.cell[i].handle = &mdm.msg_buf.cell[i].mbm_buf;      /* Point to static buffer */
        GIVE_MDM();
        /* Initialize PMS specific buffer parameters */
        PmsInitExternalBuf(mdm.msg_buf.cell[i].handle,
                           mdm.msg_buf.cell[i].payload,
                           MDM_CFG_MAX_TOTAL_SIZE,
                           MBM_TYPE_CTRL_TX);
        TAKE_MDM();
        mdm.msg_buf.cell[i].handle->msg.msgTx.LowLevelRetries = (byte)1;    /* Do not use low level retries */
        mdm.msg_buf.cell[i].handle->msg.msgTx.MidLevelRetries = (byte)0;    /* Do not use mid level retries */
        GIVE_MDM();

        MbmEnqueue(&(mdm.msg_queue), mdm.msg_buf.cell[i].handle);       /* Enqueue buffer cell */
    }

    TAKE_MDM();
    mdm.msg_buf.cnt_used_mns = (byte)0;                 /* No buffer used for MNS debug messages */
    mdm.msg_buf.cnt_used_app = (byte)0;                 /* No buffer used for application debug messages */
    mdm.lost_msg_mns         = MNS_FALSE;               /* No MNS debug message was lost */
    mdm.lost_msg_app         = MNS_FALSE;               /* No application debug message was lost */
    GIVE_MDM();

    if(NULL != is_ptr->app_config_fptr)
    {
        is_ptr->app_config_fptr();              /* At this point the application is able to register debug function
                                                   ids and set the corresponding default debug levels */
    }

    T_LIB_EXIT(MDM_0);
}
#endif  /* #ifdef MDM_0 */



#ifdef MDM_1
/*!
  * \brief      Prepares an MNS debug message for sending.
  *
  * \details    This function calls the internal function
  *             MdmMsgSendInternal().
  *
  * \param[in]  case_id   Debug message case identifier
  * \param[in]  data_ptr  Pointer to the data (Byte stream)
  * \param[in]  length    Data length in number of bytes
  *
  */
void MdmMsgSendMns(word case_id, const byte * const data_ptr, byte length)
{
    byte debug_level;

    T_LIB_ENTRY(MDM_1);

    /* Extract debug level from passed case identifier (bits 2^14..2^15). */
    debug_level = MDM_EXTRACT_DBG_LEVEL(case_id);

    /* Remove bits for debug level (bits 2^14..2^15) */
    case_id = MDM_REMOVE_DEBUG_LEVEL(case_id);

    MdmMsgSendInternal(MDM_CLASS_ID_MNS, MDM_FUNC_ID_MNS, case_id, debug_level, data_ptr, length);

    T_LIB_EXIT(MDM_1);
}
#endif  /* #ifdef MDM_1 */



/*
==============================================================================
    Internal Function Implementations
==============================================================================
*/

#ifdef MDM_3
/*!
  * \brief      Prepares "Lost Debug Message" info for sending.
  *
  * \details    This debug message signalizes that one or more
  *             debug messages could not be sent.
  * \param[in]  class_id  Debug class identifier
  * \param[in]  func_id   MOST function id
  */
static void MdmSendLostMsgInfo(byte class_id, word func_id)
{
    byte debug_level;
    word case_id;
    bool check;

    T_MOD_ENTRY(MDM_3);

    TAKE_MDM();
    /* Reset "Lost Debug Message" Flag */
    switch(class_id)
    {
        case MDM_CLASS_ID_MNS:
            mdm.lost_msg_mns = MNS_FALSE;
            check            = MNS_TRUE;
            break;
        case MDM_CLASS_ID_APP:
            mdm.lost_msg_app = MNS_FALSE;
            check            = MNS_TRUE;
            break;
        default:
            check            = MNS_FALSE;
            FAILED_ASSERT();                /* Unexpected debug class */
            break;                          /*lint !e527 Unreachable code when linting */
    }
    GIVE_MDM();

    if(MNS_FALSE != check)                  /*lint !e774 Only always true, when linting*/
    {
        /* Extract debug level from passed case identifier (bits 2^14..2^15). */
        debug_level = MDM_EXTRACT_DBG_LEVEL(MDM_MSG_LOST);

        /* Remove bits for debug level (bits 2^14..2^15) */
        case_id = MDM_REMOVE_DEBUG_LEVEL(MDM_MSG_LOST);

        /* Send "Lost Debug Message" info */
        MdmMsgSendInternal(class_id, func_id, case_id, debug_level, NULL, (byte)0);
    }

    T_MOD_EXIT(MDM_3);
}
#endif  /* #ifdef MDM_3 */



#ifdef MDM_4
static HMBMBUF MdmReqBufPtr(byte class_id)
{
    byte    *used_ptr;              /* Pointer to number of used buffers */
    byte    *used_other_ptr;        /* Pointer to number of used buffers used by other debug class */
    byte    limit;                  /* Limit of reserved buffers */
    byte    limit_other;            /* Limit of reserved buffers reserved for other debug class */
    HMBMBUF ret;

    T_MOD_ENTRY(MDM_4);

    ret = NULL;                     /* Default return value is NULL pointer */

    /* Identification of debug class. Get associated working parameters. */
    switch(class_id)
    {
        case MDM_CLASS_ID_MNS:
            used_ptr       = &mdm.msg_buf.cnt_used_mns;
            used_other_ptr = &mdm.msg_buf.cnt_used_app;
            limit          = MDM_CFG_RSVD_MNS;
            limit_other    = MDM_CFG_RSVD_APP;
            break;
        case MDM_CLASS_ID_APP:
            used_ptr       = &mdm.msg_buf.cnt_used_app;
            used_other_ptr = &mdm.msg_buf.cnt_used_mns;
            limit          = MDM_CFG_RSVD_APP;
            limit_other    = MDM_CFG_RSVD_MNS;
            break;
        default:
            used_ptr       = NULL;
            FAILED_ASSERT();                /* Unexpected debug class */
            break;                          /*lint !e527 Unreachable code when linting */
    }

    if(NULL != used_ptr)                    /*lint !e774 Only always true, when linting*/
    {
        /*
         * Check if free buffer is available:
         * Number of used buffer cells < reserved buffer cells ?
         * OR
         * Shared buffer is used by other debug class and shared buffer is still available?
         * OR
         * Other debug class doesn't use shared buffers and shared buffer is still available?
         */
        TAKE_MDM();
        if( (*used_ptr < limit) ||
            ((*used_other_ptr > limit_other) && ((*used_ptr + *used_other_ptr) < MDM_CFG_NUM_BUF)) ||
            ((*used_other_ptr <= limit_other) && ((*used_ptr + limit_other) < MDM_CFG_NUM_BUF)) )
        {
            (*used_ptr)++;                          /* Buffer cell successfully dequeued, increment used-buffer-counter */
            GIVE_MDM();
            ret = MbmDequeue(&(mdm.msg_queue));     /* Dequeue buffer cell */
            if(NULL == ret)                         /* Error while dequeuing? */
            {
                TAKE_MDM();
                (*used_ptr)--;                      /* Error occurred! Decrement used-buffer-counter */
                GIVE_MDM();
                FAILED_ASSERT();                    /* Conflict free buffer info MDM <-> MBM */
            }
        }
        else
        {
            GIVE_MDM();
        }
    }

    T_MOD_EXIT(MDM_4);

    return(ret);                                    /* Return buffer handle */
}
#endif  /* #ifdef MDM_4 */



#ifdef MDM_5
static void MdmSetBufFree(byte class_id, HMBMBUF handle)
{
    bool check;
    bool lost_msg_mns;
    bool lost_msg_app;

    T_MOD_ENTRY(MDM_5);

    ASSERT(NULL != handle);

    TAKE_MDM();
    /* Decrement counter for number of used buffer cells */
    if((MDM_CLASS_ID_MNS == class_id) && ((byte)0 < mdm.msg_buf.cnt_used_mns))
    {
        mdm.msg_buf.cnt_used_mns--;
        lost_msg_mns = mdm.lost_msg_mns;
        lost_msg_app = mdm.lost_msg_app;
        check = MNS_TRUE;
    }
    else if((MDM_CLASS_ID_APP == class_id) && ((byte)0 < mdm.msg_buf.cnt_used_app))
    {
        mdm.msg_buf.cnt_used_app--;
        lost_msg_mns = mdm.lost_msg_mns;
        lost_msg_app = mdm.lost_msg_app;
        check = MNS_TRUE;
    }
    else
    {
        check = MNS_FALSE;
        FAILED_ASSERT();                                            /* Unexpected debug class or no buffer in use */
    }
    GIVE_MDM();

    if(MNS_FALSE != check)                                          /*lint !e774 Only always true, when linting*/
    {
        MbmEnqueue(&(mdm.msg_queue), handle);                       /* Enqueue buffer cell (set buffer cell free) */

        /* Send "Lost Debug Message" info if necessary */
        if(MNS_FALSE != lost_msg_mns)
        {
            MdmSendLostMsgInfo(MDM_CLASS_ID_MNS, MDM_FUNC_ID_MNS);              /* Send MNS "Lost Debug Message" info */
        }
        if(MNS_FALSE != lost_msg_app)
        {
            MdmSendLostMsgInfo(MDM_CLASS_ID_APP, handle->msg.msgTx.Func_ID);    /* Send application "Lost Debug Message" info */
        }
    }

    T_MOD_EXIT(MDM_5);
}
#endif  /* #ifdef MDM_5 */



#ifdef MDM_6
static void MdmMsgSendInternal(byte class_id, word func_id, word case_id, byte debug_level, const byte * const data_ptr, byte length)
{
    HMBMBUF    handle;
    bool       *lost_msg_ptr;

    T_API_ENTRY(MDM_6);

    /* Identification of debug class. Get associated working parameters. */
    switch(class_id)
    {
        case MDM_CLASS_ID_MNS:
            lost_msg_ptr = &mdm.lost_msg_mns;
            break;
        case MDM_CLASS_ID_APP:
            lost_msg_ptr = &mdm.lost_msg_app;
            break;
        default:
            lost_msg_ptr = NULL;
            FAILED_ASSERT();                                /* Unexpected debug class */
            break;                                          /*lint !e527 Unreachable code when linting */
    }

    if(NULL != lost_msg_ptr)                                /*lint !e774 Only always true, when linting*/
    {
        handle = MdmReqBufPtr(class_id);                    /* Request free buffer. */
        if(NULL != handle)                                  /* Free buffer available? */
        {
            MdmSetClassSpecificParams(class_id, func_id, handle);   /* Set debug class specific parameters */
            MdmSetDbgLevel(handle, debug_level);                    /* Set debug level */
            MdmSetTimeStamp(handle);                                /* Set current timestamp */
            MdmSetCaseId(handle, case_id);                          /* Set debug message case identifier */

            /* Does the debug message contain case specific values? */
            if((NULL != data_ptr) && ((byte)0 < length))
            {
                /* Copy case specific values */
                MDM_MEM_CPY((handle->msg.msgTx.Data + MDM_VALUES_INDEX), data_ptr, (word)length);

                handle->msg.msgTx.Length = ((word)length + (word)MDM_HEADER_SIZE);  /* Set message length:
                                                                                       Length of case specific values +
                                                                                       length of message header */
            }
            else
            {
                handle->msg.msgTx.Length = (word)MDM_HEADER_SIZE;           /* Set message length = length of message header */
            }

            MBM_NO_USER_TX_FREE(handle);                                    /* Avoid calling user free callback for external payload buffer */
            PmsSendBypass(handle, MdmTxComplete, MNS_TRUE);                 /* Pass debug message to PMS */
        }
        else
        {
            TAKE_MDM();
            *lost_msg_ptr = MNS_TRUE;                                       /* No buffer available => debug message lost */
            GIVE_MDM();
        }
    }

    T_API_EXIT(MDM_6);
}
#endif  /* #ifdef MDM_6 */



#ifdef MDM_7
static byte MdmTxComplete(HMBMBUF handle, byte status)
{
    word func_id;
    byte class_id;
    bool *lost_msg_ptr;

    T_MOD_ENTRY(MDM_7);

    ASSERT(NULL != handle);

    func_id = handle->msg.msgTx.Func_ID;

    switch(func_id)
    {
        case MDM_FUNC_ID_MNS:
            lost_msg_ptr = &mdm.lost_msg_mns;
            class_id     = MDM_CLASS_ID_MNS;
            break;
        default:
            lost_msg_ptr = &mdm.lost_msg_app;
            class_id     = MDM_CLASS_ID_APP;
            break;
    }

    if(XMIT_SUCCESS != status)                                  /* Transmission error occurred ? */
    {
        word case_id = MdmGetCaseId(handle);                    /* Extract case identifier from message payload */

        if(MDM_REMOVE_DEBUG_LEVEL(MDM_MSG_LOST) != case_id)     /* Message is not "Lost Debug Message" info ? */
        {
            TAKE_MDM();
            *lost_msg_ptr = MNS_TRUE;                           /* Transmission error => MNS MOST debug message lost */
            GIVE_MDM();
        }
    }

    MdmSetBufFree(class_id, handle);                            /* Free buffer cell ? */

    T_MOD_EXIT(MDM_7);

    return(PMS_RELEASE);
}
#endif  /* #ifdef MDM_7 */



#ifdef MDM_8
static _INLINE void MdmSetDbgLevel(HCMBMBUF handle, byte debug_level)
{
    T_MOD_ENTRY(MDM_8);

    ASSERT(NULL != handle);

    /* Set debug level in message payload. */
    handle->msg.msgTx.Data[MDM_DBG_MSG_LEVEL_INDEX] = debug_level;

    T_MOD_EXIT(MDM_8);
}
#endif  /* #ifdef MDM_8 */



#ifdef MDM_9
static _INLINE void MdmSetTimeStamp(HCMBMBUF handle)
{
    T_MOD_ENTRY(MDM_9);

    ASSERT(NULL != handle);

    /* Set time stamp in message payload
     * Today this feature isn't supported by MNS.
     * Default value: 0xFFFF FFFF
     */
    handle->msg.msgTx.Data[MDM_TIME_STAMP_INDEX_LW_LB] = (byte)(0xFF);
    handle->msg.msgTx.Data[MDM_TIME_STAMP_INDEX_LW_HB] = (byte)(0xFF);
    handle->msg.msgTx.Data[MDM_TIME_STAMP_INDEX_HW_LB] = (byte)(0xFF);
    handle->msg.msgTx.Data[MDM_TIME_STAMP_INDEX_HW_HB] = (byte)(0xFF);

    T_MOD_EXIT(MDM_9);
}
#endif  /* #ifdef MDM_9 */



#ifdef MDM_10
static _INLINE void MdmSetCaseId(HCMBMBUF handle, word case_id)
{
    T_MOD_ENTRY(MDM_10);

    ASSERT(NULL != handle);

    /* Set low Byte of debug case identifier */
    handle->msg.msgTx.Data[MDM_CASE_ID_INDEX_LB] = LB(case_id);
    /* Set high Byte of debug case identifier. */
    handle->msg.msgTx.Data[MDM_CASE_ID_INDEX_HB] = HB(case_id);

    T_MOD_EXIT(MDM_10);
}
#endif  /* #ifdef MDM_10 */



#ifdef MDM_11
static _INLINE void MdmSetClassSpecificParams(byte class_id, word func_id, HMBMBUF handle)
{
    T_MOD_ENTRY(MDM_11);

    ASSERT(NULL != handle);

    /* Distinguish between debug classes */
    switch(class_id)
    {
        case MDM_CLASS_ID_MNS:
            handle->msg.msgTx.Tgt_Adr   = MDM_TGT_ADDR_MNS;         /* Set Target Address */
            handle->msg.msgTx.FBlock_ID = MDM_FBLOCK_MNS;           /* Set FBlock ID */
            handle->msg.msgTx.Func_ID   = func_id;                  /* Set MOST Function ID */
            handle->msg.msgTx.Operation = MDM_OP_STATUS_MNS;        /* Set Operation status */
            break;
        case MDM_CLASS_ID_APP:
            handle->msg.msgTx.Tgt_Adr   = MDM_TGT_ADDR_APP;         /* Set Target Address */
            handle->msg.msgTx.FBlock_ID = MDM_FBLOCK_APP;           /* Set FBlock ID */
            handle->msg.msgTx.Func_ID   = func_id;                  /* Set MOST Function ID */
            handle->msg.msgTx.Operation = MDM_OP_STATUS_APP;        /* Set Operation status */
            break;
        default:
            FAILED_ASSERT();    /* Unexpected debug class */
            break;              /*lint !e527 Unreachable code when linting */
    }

    T_MOD_EXIT(MDM_11);
}
#endif  /* #ifdef MDM_11 */



#ifdef MDM_17
static word _MdmGetTblIndexApp(word func_id)
{
    word i;
    word ret;

    T_MOD_ENTRY(MDM_17);

    ret = MDM_NOT_REGISTERED;       /* Defaul return value (Function id is unkown) */

    for(i=(word)0; i<MDM_CFG_APP_NUM_FUNC_IDS; i++)
    {
        if(mdm.app_registry_tbl[i].func_id == func_id)  /* Function id stored in current table cell ? */
        {
            ret = i;                /* Return current index */
            break;
        }
    }

    T_MOD_EXIT(MDM_17);

    return(ret);                    /* Return table index */
}
#endif  /* #ifdef MDM_17 */



#ifdef MDM_18
static _INLINE bool MdmValidateFuncId(word func_id)
{
    bool ret;

    T_MOD_ENTRY(MDM_18);

    if( (MDM_FUNC_ID_ADJ_APP == func_id) ||
        (MDM_FUNC_ID_NIC     == func_id) ||
        (MDM_FUNC_ID_APP_WC  == func_id) ||
        (MDM_FUNC_ID_MNS     == func_id) )
    {
        ret = MNS_FALSE;
    }
    else
    {
        ret = MNS_TRUE;
    }
    
    T_MOD_EXIT(MDM_18);

    return(ret);
}
#endif  /* #ifdef MDM_18 */



#ifdef MDM_19
static _INLINE bool MdmValidateDbgLevel(byte debug_level)
{
    bool ret;

    T_MOD_ENTRY(MDM_19);

    if( (MDM_CFG_LEVEL_NONE    != debug_level) &&
        (MDM_CFG_LEVEL_ERROR   != debug_level) &&
        (MDM_CFG_LEVEL_WARNING != debug_level) &&
        (MDM_CFG_LEVEL_INFO    != debug_level) )
    {
        ret = MNS_FALSE;
    }
    else
    {
        ret = MNS_TRUE;
    }
    
    T_MOD_EXIT(MDM_19);

    return(ret);
}
#endif  /* #ifdef MDM_19 */



#ifdef MDM_20
static _INLINE bool MdmValidateCaseId(word case_id)
{
    bool ret;

    T_MOD_ENTRY(MDM_20);

    if( (MDM_CASE_ID_RESERVED_01 == case_id) ||
        (MDM_CASE_ID_RESERVED_02 == case_id) )
    {
        ret = MNS_FALSE;
    }
    else
    {
        ret = MNS_TRUE;
    }
    
    T_MOD_EXIT(MDM_20);

    return(ret);
}
#endif  /* #ifdef MDM_20 */



#ifdef MDM_21
static _INLINE bool MdmValidateLength(byte length)
{
    bool ret;

    T_MOD_ENTRY(MDM_21);

    if(MDM_CFG_MAX_MSG_SIZE < length)
    {
        ret = MNS_FALSE;
    }
    else
    {
        ret = MNS_TRUE;
    }
    
    T_MOD_EXIT(MDM_21);

    return(ret);
}
#endif  /* #ifdef MDM_21 */



#ifdef MDM_22
static _INLINE bool MdmValidatePtr(const void *ptr)
{
    bool ret;

    T_MOD_ENTRY(MDM_22);

    if(ptr == NULL)
    {
        ret = MNS_FALSE;
    }
    else
    {
        ret = MNS_TRUE;
    }
    
    T_MOD_EXIT(MDM_22);

    return(ret);
}
#endif  /* #ifdef MDM_22 */



#ifdef MDM_23
static _INLINE word MdmGetCaseId(HCMBMBUF handle)
{
    word case_id;

    T_MOD_ENTRY(MDM_23);

    ASSERT(NULL != handle);
    
    /* Extract high byte of debug case identifier. */
    case_id = (word)((word)handle->msg.msgTx.Data[MDM_CASE_ID_INDEX_HB] << 8);
    /* Extract low byte of debug case identifier. */
    case_id |= (word)(((word)handle->msg.msgTx.Data[MDM_CASE_ID_INDEX_LB]) & ((word)0x00FF));

    T_MOD_EXIT(MDM_23);

    return(case_id);
}
#endif  /* #ifdef MDM_23 */

/*
==============================================================================
    End Of File
==============================================================================
*/
