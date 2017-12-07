/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Public API of the MOST Debug Message Module (MDM)
File:           mdm_pb.h
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
  * \brief      Public API of the MOST Debug Message Module (MDM)
  */

#ifndef _MDM_PB_H
#define _MDM_PB_H

#include "mostdef1.h"

/*
==============================================================================
    Module Global Definitions
==============================================================================
*/


/*
==============================================================================
    Module Global Structures
==============================================================================
*/

/*! \brief      Parameter structure for application debug messages */
typedef struct TMdmParamsApp
{
    word func_id;                   /*! MOST function id */
    word case_id;                   /*! Debug message case identifier */
    byte debug_level;               /*! Debug level of the message */
    const byte *data_ptr;           /*! Data pointer for case specific values (if not used set to NULL) */
    byte length;                    /*! Data length in number of bytes (if not used set to 0) */

} TMdmParamsApp;


/*
==============================================================================
    Module Global API Prototypes
==============================================================================
*/

#ifdef MDM_2
    extern byte MdmMsgSendApp(const TMdmParamsApp *params_ptr);
#endif  /* #ifdef MDM_2 */

#ifdef MDM_12
    extern byte MdmSetDbgLevelApp(word func_id, byte debug_level);
#endif  /* #ifdef MDM_12 */

#ifdef MDM_13
    extern byte MdmGetDbgLevelApp(word func_id);
#endif  /* #ifdef MDM_13 */

#ifdef MDM_14
    extern byte MdmRegisterFuncIdApp(word func_id, byte debug_level);
#endif  /* #ifdef MDM_14 */

#ifdef MDM_15
    extern byte MdmUnregisterFuncIdApp(word func_id);
#endif  /* #ifdef MDM_15 */

#ifdef MDM_16
    extern byte MdmGetDbgLevelListApp(word *dbg_level_list, word max_list_entries, word *length_ptr);
#endif  /* #ifdef MDM_16 */

#endif /* #ifndef _MDM_PB_H */


/*
==============================================================================
    End Of File
==============================================================================
*/
