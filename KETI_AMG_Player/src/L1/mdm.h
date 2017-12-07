/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Internal API of the MOST Debug Message Module (MDM)
File:           mdm.h
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
  * \brief      Internal API of the MOST Debug Message Module (MDM)
  */

#ifndef _MDM_H
#define _MDM_H


/*
==============================================================================
    Includes
==============================================================================
*/

#include "mostns1.h"


/*
==============================================================================
    Library Global Structures
==============================================================================
*/


/*
==============================================================================
    Library internal types
==============================================================================
*/

/*! \brief      Initialization structure for MDM module */
typedef struct MdmInitStruct
{
    void (*app_config_fptr)(void);          /*!< Callback pointer for application initialization */

} MdmInitStruct;


/*
==============================================================================
    Library Function Prototypes
==============================================================================
*/

#ifdef MDM_0
    extern void MdmInit(MdmInitStruct *is_ptr);
#endif  /* #ifdef MDM_0 */

#ifdef MDM_1
    extern void MdmMsgSendMns(word case_id, const byte * const data_ptr, byte length);
#endif  /* #ifdef MDM_1 */

#endif /* #ifndef _MDM_H */


/*
==============================================================================
    End Of File
==============================================================================
*/
