/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Adjustment of the MOST Debug Message Module (MDM)
File:           mdm_ad.h
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
  * \brief      Adjustment of the MOST Debug Message Module (MDM)
  */

#ifndef _MDM_AD_H
#define _MDM_AD_H

#include "rules_ad.h"
#include "mdm_case_id.h"

#ifdef MDM_MIN

    /*
    ----------------------------------------------------------------------------
        Library
    ----------------------------------------------------------------------------
    */
    #define MDM_0    ((word)0)      /*! MdmInit                    */
    #define MDM_1    ((word)1)      /*! MdmMsgSendMns              */
    #define MDM_2    ((word)2)      /*! MdmSendLostMsgInfo         */
    /*
    ----------------------------------------------------------------------------
        API
    ----------------------------------------------------------------------------
    */
    #define MDM_3    ((word)3)      /*! MdmMsgSendApp              */
    #define MDM_12   ((word)12)     /*! MdmSetDbgLevelApp          */
    #define MDM_13   ((word)13)     /*! MdmGetDbgLevelApp          */
    #define MDM_14   ((word)14)     /*! MdmRegisterFuncIdApp       */
    #define MDM_15   ((word)15)     /*! MdmUnregisterFuncIdApp     */
    #define MDM_16   ((word)16)     /*! MdmGetDbgLevelListApp      */
    /*
    ----------------------------------------------------------------------------
        Module
    ----------------------------------------------------------------------------
    */
    #define MDM_4    ((word)4)      /*! MdmReqBufPtr               */
    #define MDM_5    ((word)5)      /*! MdmSetBufFree              */
    #define MDM_6    ((word)6)      /*! MdmMsgSendInternal         */
    #define MDM_7    ((word)7)      /*! MdmTxComplete              */
    #define MDM_8    ((word)8)      /*! MdmSetDbgLevel             */
    #define MDM_9    ((word)9)      /*! MdmSetTimeStamp            */
    #define MDM_10   ((word)10)     /*! MdmSetCaseId               */
    #define MDM_11   ((word)11)     /*! MdmSetClassSpecificParams  */
    #define MDM_17   ((word)17)     /*! MdmGetTblIndexApp          */
    #define MDM_18   ((word)18)     /*! MdmValidateFuncId          */
    #define MDM_19   ((word)19)     /*! MdmValidateDbgLevel        */
    #define MDM_20   ((word)20)     /*! MdmValidateCaseId          */
    #define MDM_21   ((word)21)     /*! MdmValidateLength          */
    #define MDM_22   ((word)22)     /*! MdmValidatePtr             */
    #define MDM_23   ((word)23)     /*! MdmGetCaseId               */

#endif /* #ifdef MDM_MIN */


#endif /* #ifndef _MDM_AD_H */


/*
==============================================================================
    End Of File
==============================================================================
*/
