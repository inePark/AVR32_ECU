/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Private declarations and definitions of the MOST Processor 
                Control Service Wrapper (WMCS)
File:           wmcs_pv.h
Version:        3.0.x-SR-1  
Language:       C
Author(s):      R.Wilhelm, R.Eberhardt
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
  * \brief      Private declarations and definitions of the MOST Processor 
  *             Control Service Wrapper (WMCS)
  */

#ifndef _WMCS_PV_H
#define _WMCS_PV_H


/*
==============================================================================
    Includes
==============================================================================
*/
#include "wmcs.h"


/*
==============================================================================
    Rules
==============================================================================
*/



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

#define WMCS_RETRY_ALT_PADDR            ((word) 0x0001)
#define WMCS_RETRY_REQUEST_CODINGERR    ((word) 0x0002)
#define WMCS_RETRY_REQUEST_ALLOC_TABLE  ((word) 0x0004)
#define WMCS_RETRY_REQUEST_EUI48        ((word) 0x0008)

#define WMCS_CE_INACTIVE                ((word) 0xFFFF)


#define NUM_CS_SEGMENTS 2

#define WMCS_MEMORY_OFFSET              ((byte) 5)

#define WMCS_MEMORY_OFFSET_ROM          ((byte) 1)

#define MEM_READ_TYPE_PINFO             ((byte) 0x00)
#define MEM_READ_TYPE_CONF              ((byte) 0x02)

#define RESET_INIC                      ((byte)0x01)
#define RESET_EHC                       ((byte)0x02)
#define RESET_PMEM                      ((byte)0x04)

#define MAX_CHANNELS_25                 ((byte) 60)
#define MAX_CNT_25                      ((byte) 32)

/*
==============================================================================
    Module Internal Structures
==============================================================================
*/

#ifdef MCS_MIN

    typedef struct WmcsData
    {
        word pending_events;
        word latest_handled_event;
        byte node_pos;
        byte rmck_divider;

        word retry;

        struct WmcsNodeAddrData
        {
            word value;
            word scheduled;

        } node_addr;

        struct WmcsGroupAddrData
        {
            byte value;
            byte scheduled;

        } group_addr;

        struct WmcsDataShadow
        {
            word alt_packet_addr;

        } shadow;

        struct WmcsCodingErrors
        {
            word                counter;
            TMostTimer          timer;
            word                deadtime;
            word                timeout;
            TWmcsCodingErrorCB *cb_ptr;
            bool                busy_reset;

        } coding_errors;

        struct WmcsMemory
        {
            TWmcsReadConfStringCB   *cs_cb_ptr;
            byte                    *conf_string;
            byte                    cs_msg_num;
            bool                    pending;
        } memory;


    #if (defined MEP_MIN) && (defined _OS81110_MEP)
        struct WmcsMEPFilterMode
        {
            TWmcsMEPFilterModeCB    *cb_ptr;
            bool                    pending;
        } mepfiltermode;

        struct WmcsMEPHashTable
        {
            TWmcsMEPHashTableCB     *cb_ptr;
            bool                    pending;
        } mephashtable;

        struct WmcsEUI48
        {
            TWmcsEUI48CB            *cb_ptr;
            bool                    pending;
        } eui48;

        byte eui48_shadow[6];
    #endif

    #ifdef _OS81110_PCK_LLR
        struct WmcsPacketRetryTime
        {
            TWmcsPacketRetryTimeCB  *cb_ptr;
            bool                    pending;
        } packetretrytime;

    #endif
    } WmcsData;

#endif  /* #ifdef MCS_MIN */


/*
==============================================================================
    Module Internal Function Prototypes
==============================================================================
*/
#ifdef WMCS_3
    static word WmcsGetNextEventToHandle(void);
#endif

#ifdef WMCS_14
    static void Wmcs_Go_Protected(void);
#endif

#ifdef WMCS_15
    static void Wmcs_Go_SemiProtected(void);
#endif

#ifdef WMCS_23
    static byte WmcsRetry(void);
#endif

#ifdef WMCS_26
    static byte WmcsRequestCodingErrors(void);
#endif

#ifdef WMCS_30
    static void WmcsCodingErrorsCallback(TMnsResult result, word coding_errors);
#endif

#ifdef WMCS_31
    static void WmcsCodingErrorsTimeout(void);
#endif

#ifdef WMCS_58
    static byte WmcsRequestEUI48(void);
#endif





#endif /* #ifndef _WMCS_PV_H */

/*
==============================================================================
    End Of File
==============================================================================
*/
