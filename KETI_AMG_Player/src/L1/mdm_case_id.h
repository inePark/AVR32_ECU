/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         MOST Debug Message case identifiers for MNS
File:           mdm_case_id.h
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
  * \brief      MOST Debug Message case identifiers for MNS
  *
  * \details    This file contains all debug message case identifiers for
  *             the MOST NetServices. This file is associated with the MOST
  *             debug message module (MDM).
  *
  * \remarks    This is an automatically generated file. Do not edit!
  *
  */

#ifndef _MDM_CASE_ID_H
#define _MDM_CASE_ID_H

#ifdef MDM_MIN

/*
==============================================================================
    Includes
==============================================================================
*/

#include "mostdef1.h"


/*
==============================================================================
    DEBUG LEVELS
==============================================================================
*/
#define MDM_LEVEL_NONE                           0
#define MDM_LEVEL_ERROR                          1
#define MDM_LEVEL_WARNING                        2
#define MDM_LEVEL_INFO                           3

#define MDM_LEVEL_MASK_NONE                      ((word)0x0000)
#define MDM_LEVEL_MASK_ERROR                     ((word)0x4000)
#define MDM_LEVEL_MASK_WARNING                   ((word)0x8000)
#define MDM_LEVEL_MASK_INFO                      ((word)0xC000)

#define MDM_LEVEL_BITMASK                        ((word)0xC000)


/*
==============================================================================
    Case Identifier - Reserved
==============================================================================
*/
#define MDM_CASE_ID_RESERVED_01                  ((word)0x0000)
#define MDM_CASE_ID_RESERVED_02                  ((word)0xFFFF)


/*
==============================================================================
    Case Identifier - Lost Debug Message Info
==============================================================================
*/
#define MDM_MSG_LOST                             (MDM_LEVEL_MASK_ERROR | (word)0x0200)


/*
==============================================================================
    Case Identifier - Debug Level "ERROR"
==============================================================================
*/
#if (MDM_LEVEL >= MDM_LEVEL_ERROR)
    #define MDM_MNS_INIT_TIMEOUT                 (MDM_LEVEL_MASK_ERROR | (word)0x0500)
    #define MDM_MNS_VERSION_CONFLICT             (MDM_LEVEL_MASK_ERROR | (word)0x0501)
    #define MDM_MHP_UNKNOWN_TRIGGER_STATE        (MDM_LEVEL_MASK_ERROR | (word)0x1301)
    #define MDM_MSV2_NWM_CONF_STATE_NOK          (MDM_LEVEL_MASK_ERROR | (word)0x0F00)
#endif


/*
==============================================================================
    Case Identifier - Debug Level "WARNING"
==============================================================================
*/
#if (MDM_LEVEL >= MDM_LEVEL_WARNING)
    /* NONE */
#endif


/*
==============================================================================
    Case Identifier - Debug Level "INFO"
==============================================================================
*/
#if (MDM_LEVEL >= MDM_LEVEL_INFO)
    #define MDM_MBM_BUFFER_ALLOCATION_FAILED     (MDM_LEVEL_MASK_INFO | (word)0x0300)
    #define MDM_MHP_SILENT_TERMINATION           (MDM_LEVEL_MASK_INFO | (word)0x1300)
#endif


#endif  /* #ifdef MDM_MIN */

#endif  /* #ifndef _MDM_CASE_ID_H */

/*
==============================================================================
    End Of File
==============================================================================
*/
