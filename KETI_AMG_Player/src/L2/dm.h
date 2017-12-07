/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         FBlock DebugMessages (Adjust_Application_DebugMessage)
File:           dm.h
Version:        3.0.x-SR-1  
Language:       C
Author(s):      R.Hanke
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
Date            By      Description

==============================================================================
*/

/*! \file
  * \brief      Function declarations of FBlock DebugMessages 
  *             (Adjust_Application_DebugMessage).
  */

#ifndef _DM_H
#define _DM_H


/*
==============================================================================
    Includes
==============================================================================
*/

#include "mostns.h"


/*
==============================================================================
    Function Prototypes
==============================================================================
*/

#ifdef DM_0
    extern byte DM_AdjAppDbgMsg_Set(pTMsgTx tx_ptr, pTMsgRx rx_ptr);
#endif  /* #ifdef DM_0 */

#ifdef DM_1
    extern byte DM_AdjAppDbgMsg_Get(pTMsgTx tx_ptr, pTMsgRx rx_ptr);
#endif  /* #ifdef DM_1 */

#ifdef DM_2
    extern byte DM_AdjAppDbgMsg_Status(pTMsgRx rx_ptr);
#endif  /* #ifdef DM_2 */

#ifdef DM_3
    extern byte DM_AdjAppDbgMsg_Error(pTMsgRx rx_ptr);
#endif  /* #ifdef DM_3 */

#endif /* #ifndef _DM_H */


/*
==============================================================================
    End Of File
==============================================================================
*/
