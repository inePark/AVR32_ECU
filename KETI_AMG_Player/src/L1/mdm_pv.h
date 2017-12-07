/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Private declarations and definitions of the MOST Debug
                Message Module (MDM)
File:           mdm_pv.h
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
  * \brief      Private declarations and definitions of the MOST Debug
  *             Message Module (MDM)
  */

#ifndef _MDM_PV_H
#define _MDM_PV_H


/*
==============================================================================
    Includes
==============================================================================
*/

#include "mbm.h"
#include "pms.h"
#include "mdm.h"


/*
==============================================================================
    Internal Adjust Definitions
==============================================================================
*/

/*!
 * Maximum number of bytes for case specific values of MNS debug messages.
 */
#define MDM_MAX_MSG_SIZE_MNS    7


/*
==============================================================================
    Rules
==============================================================================
*/

#ifdef MDM_MIN
    #if MDM_NUM_BUF > 255
        #error MDM_NUM_BUF must not be bigger than 255
    #endif

    #if MDM_MAX_MSG_SIZE_APP > 38
        #error MDM_MAX_MSG_SIZE_APP must not be bigger than 38
    #endif

    #if (MDM_RSVD_MNS + MDM_RSVD_APP) > MDM_NUM_BUF
        #error The sum of MDM_RSVD_MNS and MDM_RSVD_APP must not be bigger than MDM_NUM_BUF
    #endif

    #if MDM_APP_NUM_FUNC_IDS > 4092
        #error MDM_APP_NUM_FUNC_IDS must not be bigger than 4092
    #endif
#endif  /* #ifdef MDM_MIN */


/*
==============================================================================
    Module Internal Definitions
==============================================================================
*/

/*!
 * Debug Classes
 */
#define MDM_CLASS_ID_MNS                ((byte)0x00)                    /*! Debug Class: MOST NetServices (MNS) */
#define MDM_CLASS_ID_APP                ((byte)0x01)                    /*! Debug Class: Application */

/*!
 * MNS debug message parameters
 */
#define MDM_TGT_ADDR_MNS                ((word)0x0FF0)
#define MDM_FBLOCK_MNS                  FBLOCK_DEBUG
#define MDM_FUNC_ID_MNS                 ((word)0x0002)
#define MDM_OP_STATUS_MNS               OP_STATUS

/*!
 * Application debug message parameters
 */
#define MDM_TGT_ADDR_APP                ((word)0x0FF0)
#define MDM_FBLOCK_APP                  FBLOCK_DEBUG
#define MDM_OP_STATUS_APP               OP_STATUS

/*!
 * Reserved function ids for Adjust_Application and NIC debug messages
 */
#define MDM_FUNC_ID_ADJ_APP             ((word)0x0000)                  /*! Adjust Application Debug Messages */
#define MDM_FUNC_ID_NIC                 ((word)0x0001)                  /*! INIC Debug Messages */
#define MDM_FUNC_ID_APP_WC              ((word)0x0FFF)                  /*! Wildcard for Adjust Application Debug Messages */

/*!
 * Byte indices in payload byte stream
 */
#define MDM_DBG_MSG_LEVEL_INDEX         0                               /*! Debug level */
#define MDM_TIME_STAMP_INDEX_HW_HB      1                               /*! Time Stamp - High Word, High Byte */
#define MDM_TIME_STAMP_INDEX_HW_LB      2                               /*! Time Stamp - High Word, Low Byte */
#define MDM_TIME_STAMP_INDEX_LW_HB      3                               /*! Time Stamp - Low Word, High Byte */
#define MDM_TIME_STAMP_INDEX_LW_LB      4                               /*! Time Stamp - Low Word, Low Byte */
#define MDM_CASE_ID_INDEX_HB            5                               /*! Case Id - High Byte */
#define MDM_CASE_ID_INDEX_LB            6                               /*! Case Id - Low Byte */
#define MDM_VALUES_INDEX                7                               /*! Start of case specific values */

/*! Debug message header size in bytes */
#define MDM_HEADER_SIZE                 ((byte)7)

/*! Maximal payload per debug message in bytes */
#if (MDM_MAX_MSG_SIZE_MNS > MDM_MAX_MSG_SIZE_APP)
    #define MDM_CFG_MAX_MSG_SIZE        ((byte)MDM_MAX_MSG_SIZE_MNS)
#else
    #define MDM_CFG_MAX_MSG_SIZE        ((byte)MDM_MAX_MSG_SIZE_APP)
#endif

/*! Maximal message size in bytes */
#define MDM_CFG_MAX_TOTAL_SIZE          (MDM_HEADER_SIZE + MDM_CFG_MAX_MSG_SIZE + (byte)PMS_CTRL_HDR_MAX_SIZE)

/*!
 * Message buffer configuration values
 */
#define MDM_CFG_NUM_BUF                 ((byte)MDM_NUM_BUF)             /*! Total number of buffer cells (MNS + application + shared buffers) */

#define MDM_CFG_RSVD_MNS                ((byte)MDM_RSVD_MNS)            /*! Maximal number of buffer cells for MNS debug messages */
#define MDM_CFG_RSVD_APP                ((byte)MDM_RSVD_APP)            /*! Maximal number of buffer cells for application debug messages */

#define MDM_CFG_LEVEL                   ((byte)MDM_LEVEL)               /*! Current debug level */
#define MDM_CFG_LEVEL_NONE              ((byte)MDM_LEVEL_NONE)
#define MDM_CFG_LEVEL_ERROR             ((byte)MDM_LEVEL_ERROR)
#define MDM_CFG_LEVEL_WARNING           ((byte)MDM_LEVEL_WARNING)
#define MDM_CFG_LEVEL_INFO              ((byte)MDM_LEVEL_INFO)

#define MDM_CFG_APP_NUM_FUNC_IDS        ((word)MDM_APP_NUM_FUNC_IDS)

#define MDM_NOT_REGISTERED              ((word)0xFFFF)                  /*! Default return value of _MdmGetTblIndexApp(): Unregistred function id */
#define MDM_UNUSED                      ((word)0x0000)                  /*! Registry table cell not in use */

/*
==============================================================================
    Module Internal Macros
==============================================================================
*/

/*! Extract debug level from passed case identifier (bits 2^14..2^15). */
#define MDM_EXTRACT_DBG_LEVEL(value)    (byte)((word)((word)value & MDM_LEVEL_BITMASK) >> 14)

/*! Remove debug level from passed case identifier (bits 2^14..2^15). */
#define MDM_REMOVE_DEBUG_LEVEL(value)   (word)((word)value & (word)(~((word)MDM_LEVEL_BITMASK)))

#ifndef MDM_MEM_CPY
    #define MDM_MEM_CPY                 MNS_MEM_CPY
#endif

#ifndef MDM_MEM_SET
    #define MDM_MEM_SET                 MNS_MEM_SET
#endif


/*
==============================================================================
    Module Internal Structures
==============================================================================
*/

#ifdef MDM_MIN
typedef struct TMdmData
{
    struct TMdmBuf
    {
        struct TMdmBufCell
        {
            MbmBuf  mbm_buf;
            HMBMBUF handle;
            byte    payload[MDM_CFG_MAX_TOTAL_SIZE];

        } cell[MDM_CFG_NUM_BUF];

        byte cnt_used_mns;
        byte cnt_used_app;

    } msg_buf;

    MbmQueue msg_queue;

    bool lost_msg_mns;
    bool lost_msg_app;

    struct TAppRegistryTblEntry
    {
        word func_id;
        byte debug_level;
    } app_registry_tbl[MDM_APP_NUM_FUNC_IDS];

    word app_registry_tbl_cnt;

} TMdmData;
#endif  /* #ifdef MDM_MIN */


/*
==============================================================================
    Module Internal Function Prototypes
==============================================================================
*/

#ifdef MDM_3
    static void MdmSendLostMsgInfo(byte class_id, word func_id);
#endif  /* #ifdef MDM_3 */

#ifdef MDM_4
    static HMBMBUF MdmReqBufPtr(byte class_id);
#endif  /* #ifdef MDM_4 */

#ifdef MDM_5
    static void MdmSetBufFree(byte class_id, HMBMBUF handle);
#endif  /* #ifdef MDM_5 */

#ifdef MDM_6
    static void MdmMsgSendInternal(byte class_id, word func_id, word case_id, byte debug_level, const byte * const data_ptr, byte length);
#endif  /* #ifdef MDM_6 */

#ifdef MDM_7
    static byte MdmTxComplete(HMBMBUF handle, byte status);
#endif  /* #ifdef MDM_7 */

#ifdef MDM_8
    static _INLINE void MdmSetDbgLevel(HCMBMBUF handle, byte debug_level);
#endif  /* #ifdef MDM_8 */

#ifdef MDM_9
    static _INLINE void MdmSetTimeStamp(HCMBMBUF handle);
#endif  /* #ifdef MDM_9 */

#ifdef MDM_10
    static _INLINE void MdmSetCaseId(HCMBMBUF handle, word case_id);
#endif  /* #ifdef MDM_10 */

#ifdef MDM_11
    static _INLINE void MdmSetClassSpecificParams(byte class_id, word func_id, HMBMBUF handle);
#endif  /* #ifdef MDM_11 */

#ifdef MDM_17
    static word _MdmGetTblIndexApp(word func_id);
#endif  /* #ifdef MDM_17 */

#ifdef MDM_18
    static _INLINE bool MdmValidateFuncId(word func_id);
#endif  /* #ifdef MDM_18 */

#ifdef MDM_19
    static _INLINE bool MdmValidateDbgLevel(byte debug_level);
#endif  /* #ifdef MDM_19 */

#ifdef MDM_20
    static _INLINE bool MdmValidateCaseId(word case_id);
#endif  /* #ifdef MDM_20 */

#ifdef MDM_21
    static _INLINE bool MdmValidateLength(byte length);
#endif  /* #ifdef MDM_21 */

#ifdef MDM_22
    static _INLINE bool MdmValidatePtr(const void *ptr);
#endif  /* #ifdef MDM_22 */

#ifdef MDM_23
    static _INLINE word MdmGetCaseId(HCMBMBUF handle);
#endif  /* #ifdef MDM_23 */


#endif /* #ifndef _MDM_PV_H */

/*
==============================================================================
    End Of File
==============================================================================
*/
