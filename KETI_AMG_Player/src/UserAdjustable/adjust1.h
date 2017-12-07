/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Sample Adjustment for Layer I
File:           adjust1.h
Version:        3.0.3
Language:       C
Author(s):      S.Semmler, S.Kerber
Date:           11.June.2010

FileGroup:      Layer I
Customer ID:    4130FF8A030003.N.GM
FeatureCode:    FCR1
------------------------------------------------------------------------------

                (c) Copyright 1998-2010
                SMSC
                All Rights Reserved

------------------------------------------------------------------------------



Modifications
~~~~~~~~~~~~~
Date            By      Description

==============================================================================
*/

#ifndef _ADJUST1_H
#define _ADJUST1_H

#include <asf.h>



#define MOST_INIC_ID          0x03      /* Select the used INIC derivative:   */
                                        /*     0x02:  INIC50 (e.g.,OS81082)   */
                                        /*     0x03:  INIC150(e.g.,OS81110)   */

#define MNS_IGNORE_VERSION_INFO

/* -----------------------------------------------------------------------------
 * Interface to Application Layer
 * -----------------------------------------------------------------------------
 */

#define SERVICE_LAYER_II            /* Enable MOST NetServices Layer II */
/* #define NS_AMS_AH        */      /* Enable AH support for the AMS    */
#define NS_MNS_MNS2                 /* Enable Kernel support            */
#define NS_MSV_NB                   /* Enable VMSV support of the NB    */
#define NS_AMS_MSV2                 /* Enable interface between AMS and MSV2 */
#define NS_MSV_ET                   /* Enable interface between MSV and ET */

/* -----------------------------------------------------------------------------
 * Interface to MOST High Protocol Service
 * -----------------------------------------------------------------------------
 */

/* #define MOST_HIGH_PROTOCOL */    /* Enable MOSThigh Protocol */
/* #define NS_MNS_MHP         */    /* Enable Kernel support    */
/* #define NS_WADS_MHP        */    /* Enable wADS support      */
/* #define NS_PMS_PMHT        */    /* Enable PMS support for PMHT */

/* -----------------------------------------------------------------------------
 * Interface to Packetizer for MOST High Protocol Service
 * -----------------------------------------------------------------------------
 */

/* #define PACKETIZER_FOR_MOST_HIGH */
/* #define NS_MNS_PMHS              */


#if (defined SERVICE_LAYER_II) || (defined MOST_HIGH_PROTOCOL)
#define DECLARE_LEGACY_SWITCHES
#define DECLARE_LEGACY_API
#endif


/*
------------------------------------------------------------------------------
 Message Buffer Module Configuration
------------------------------------------------------------------------------
*/

#define MBM_MEM_UNIT_SIZE       4           /* The unit size defines the smallest amount
                                             * of payload the MBM will allocate.
                                             * Valid range: 1..128 (bytes)
                                             */

#define MBM_MEM_UNIT_NUM        512         /* The unit number is multiplied by unit
                                             * size to determine the amount of memory
                                             * to reserve for payload buffers. The unit
                                             * number may not exceed 65535.
                                             */

#define MBM_MSG_NUM             30          /* The message number is the number of buffer
                                             * handles allocated for use.
                                             */

#define MBM_MSG_RSVD_RX         10
#define MBM_MSG_RSVD_TX         10           /* "reserved rx" and "reserved tx" is used to tell
                                             * how many buffers are exclusively reserved for
                                             * RX and TX respectively. The sum of both values
                                             * must never exceed MBM_MSG_NUM.
                                             */


/*
------------------------------------------------------------------------------
 Application Message Service Configuration
------------------------------------------------------------------------------
*/

#define AMS_ADD1                     /* byte order functions */

#define TIME_MSG_TX_RETRY       20          /* Time to wait until next mid level
                                             * retry (in ms)
                                             */

#define DEF_MID_LEVEL_RETRIES   3           /* Default number of mid level
                                             * retries for messages sent from
                                             * application
                                             */

#define DEF_MID_LEVEL_RETRIES_INT_PROC  2   /* For internal messages */

#define MAX_TX_HANDLE           0           /* size of optional field that can
                                             * be used e.g. as transmission
                                             * handle
                                             */

#define MAX_EXT_DATA            0           /* size of optional field that can
                                             * be used e.g.for a timestamp,
                                             * etc.
                                             */

/* #define AMS_TX_NOSEG   */                /* no TX segementation */

/*#define AMS_TX_BYPASS_FILTER*/

                                            /* Applicable for MOST50 only:                        */
/* #define MSG_RX_SEG_COOPERATIVE */        /* Define one of 3 possible macros to determine       */
/* #define MSG_RX_SEG_INIC_ONLY */          /* the Rx segmentation mode. If none is selected,     */
/* #define MSG_RX_SEG_EHC_ONLY  */          /* MSG_RX_SEG_COOPERATIVE is chosen as default value. */


/*#define MSG_RX_USER_PAYLOAD*/             /* User supplies the payload buffer for
                                             * incoming messages to where the received
                                             * payload is copied before handed to the
                                             * application.
                                             */

/*#define MSG_TX_USER_PAYLOAD*/             /* Define this if user allocated payload is
                                             * intended to be used, i.e. if the
                                             * tx_ptr->Data is set by the application.
                                             */

#ifdef DECLARE_LEGACY_API

  #define MAX_MSG_TX_DATA      50           /* When this macro is defined a
                                             * V1-style MsgGetTxPtr() function
                                             * is provided, using the value of
                                             * the macro as argument to
                                             * MsgGetTxPtrExt()
                                             */

  #define MAX_MSG_RX_DATA      50

#endif




/*
------------------------------------------------------------------------------
 Application Data Service Configuration
------------------------------------------------------------------------------
*/

#define ADS_MIN

#ifdef ADS_MIN
    #define ADS_RX_PREALLOC     0       /* Number of initially preallocated RX buffers */
#endif

#define MAX_DATA_TX_HANDLE      0       /* size of optional field that can be
                                         * used e.g. as transmission handle,
                                         * etc.
                                         */

#define MAX_DATA_EXT_DATA       0       /* size of optional field that can be
                                         * used e.g.for a timestamp, etc.
                                         */

/*#define DATA_TX_USER_PAYLOAD*/        /* Define this if user allocated payload is
                                         * intended to be used, i.e. if the
                                         * tx_ptr->Data is set by the application.
                                         */


#define MDP_DEFAULT_RETRY_PRIO   0x1F   /* Default Retry|Prio value for MDP packets.
                                         * High nibble is retry and low is priority.
                                         * Only used for OS81110
                                         */

/*
------------------------------------------------------------------------------
  MOST NetServices Kernel Configuration
------------------------------------------------------------------------------
*/


/* #define PACKET_COMMUNICATION_ONLY */ /*  Packet communication is on this host
                                         *  the other layers on the EHC
                                         */

/* #define PACKET_ON_SECOND_HOST */     /*  This host has a PACKET_COMMUNICATION_ONLY
                                         *  counter part
                                         */

/* #define MNS_TM_WATCHDOG_MARGIN 2 */  /* determines the watchdog timeout margin
                                         * possible values are:
                                         * 1 - timeout margin is 50% of t_INIC_watchdog
                                         * 2 - timeout margin is 25% of t_INIC_watchdog
                                         * default value is 2
                                         */

/*
------------------------------------------------------------------------------
Virtual MOST Supervisor Configuration
------------------------------------------------------------------------------
*/

#define VMSV_MIN


/*--------------------------------------------------------*/
/* RBD according MOST Spec. Rev. 3.0                      */
/* (applicable for INIC150)                               */
/*--------------------------------------------------------*/
#define MSV_DIAG_RESULT_MSG




/*
------------------------------------------------------------------------------
Socket Connection Manager Wrapper Configuration
------------------------------------------------------------------------------
*/

#define SCM_MIN


/*
 ------------------------------------------------------------------------------
 Port Message Service Configuration
 ------------------------------------------------------------------------------
*/

/*#define PMS_DBL_BUFFER_ENABLE*/       /* Enables double buffering between PMS an INIC  */

#define PMS_RX_SEG_PREALLOC_SIZE  240   /* When the macro is defined a payload buffer of this
                                           size is pre-allocated when receiving the first segment
                                           of an incoming segmented control message. The max. size
                                           for incoming control messages is then limited to
                                           PMS_RX_SEG_PREALLOC_SIZE.
                                           If this macro is not defined MSG_RX_USER_PAYLOAD must
                                           be defined. In this case the application can give the
                                           intended size of payload. */

/*#define PMS_RX_OPT3*/                 /* MOST150 feature, enables RX optimization III */



/* The following macro retrieves the offset of a member from the beginning of
 * its parent structure.
 * There is no 100% portable and ANSI-compliant way to do this without using
 * the stdlib. Therefore we present three choices to choose from:
 *
 * 1 - use stdlib
 * 2 - simple solution using NULL pointer (some compiler may complain)
 * 3 - slightly complexer solution without NULL pointer (some other compilers
 *     may complain)
 *
 * It is up to the developer to choose, which method shall be used.
 */

#ifdef offsetof

    /* (1) if an implementation of stdlib's offsetof is available */
    #define PMS_OFFSETOF    offsetof

#else

    /* (2) with NULL pointer */
    #define PMS_OFFSETOF(s,m)   ((unsigned int)&(((s *)0)->m))

    /* (3) without NULL pointer
     *
     * extern word pmsPending;
     * #define PMS_OFFSETOF(s,m)   ((unsigned int)( (byte *)&((s *)&pmsPending)->m - (byte *)&pmsPending ))
     */

#endif

/*
 ------------------------------------------------------------------------------
 MEP Configuration
 ------------------------------------------------------------------------------
*/
/*#define MEP_MIN*/                     /* MOST150 feature, enable helper functions
                                         * and macros to identify MEP packets and
                                         * build the transmit header.
                                         */

#define MEP_DEFAULT_RETRY_PRIO 0x1F   /* Default Retry|Prio value for MEP packets
                                       * High nibble is retry and low is priority
                                             */

/*
------------------------------------------------------------------------------
Memory Copy and Memory Set Function

   By defining these macros it is possible to select a user defined memory copy
   or memory set function. This function can be adapted to the used hardware
   (e.g. DMA, Cache Alignments, DWord instead of byte copy loop).

   The function signature must be:
   - MNS_MEM_CPY: void FunctionName(byte* targetPointer, byte* sourcePointer, word size)
   - MNS_MEM_SET: void FunctionName(byte* tgt_ptr, byte value, word size)

------------------------------------------------------------------------------
*/

/* #define MNS_MEM_CPY        MyMemCpy */
/* #define MNS_MEM_SET        MyMemSet */

/*
------------------------------------------------------------------------------
Data type definitions

   The NetServices use the type names bool, byte, word and dword to identify
   boolean values, unsigned 8-bit integers, unsigned 16-bit integers and
   unsigned 32-bit integers.

   Note: Boolean values are written as 1 (MNS_TRUE) and 0 (MNS_FALSE)!

------------------------------------------------------------------------------
*/

//typedef unsigned char bool;
typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;

/*
------------------------------------------------------------------------------
Operating system abstraction

   The sys_take() and sys_give() functions are used to take and give mutexes.
   The constant MNS_REQUIRED_MUTEXES defines the amount of required mutexes and
   the functions take zero-based indizies (0 .. MNS_REQUIRED_MUTEXES-1).

------------------------------------------------------------------------------
*/

extern void mns_take(int mutex_id);
extern void mns_give(int mutex_id);

#define WAIT4MX(n)  mns_take(n)
#define REL_MX(n)   mns_give(n)

/*
------------------------------------------------------------------------------
 Compiler & target specific qualifiers

    Definitions and macros to specify plattform-dependend issues.

------------------------------------------------------------------------------
*/

/* Rom data qualifier, this is used for constant tables that needs to be     */
/* valid after startup.                                                      */
#define _CONST const

/*! This macro provides the posibility to enable "inline" handling as supported
  * by the C99 Standard or the GNU C compiler (and a lot others).
  */
#define _INLINE

/* high byte of most significant word  */
#define MSW_HB(dw)      (byte)((dw) >> 24)

/* low byte of most significant word   */
#define MSW_LB(dw)      (byte)(((dw) >> 16) & 0xFF)

/* high byte of least significant word */
#define LSW_HB(dw)      (byte)(((dw) >> 8) & 0xFF)

/* low byte of least significant word  */
#define LSW_LB(dw)      (byte)((dw) & 0xFF)

/* high byte of 16bit value  */
#define HB(value)       (byte)((value)>>8)

/* low byte of 16bit value   */
#define LB(value)       (byte)((value)&0xFF)

/* Big-Endian to target 16 bit   */
#define DECODE_WORD(w_ptr, msb_ptr) *(w_ptr) = (word)(((word)(msb_ptr)[0] << 8) | (msb_ptr)[1])

/* Big-Endian to target 32 bit   */
#define DECODE_DWORD(dw_ptr, msb_ptr) *(dw_ptr) = ((dword)(msb_ptr)[0] << 24) | ((dword)(msb_ptr)[1] << 16) | ((dword)(msb_ptr)[2] << 8) | (msb_ptr)[3]

/*
------------------------------------------------------------------------------
 Debug Interface

    Definitions and macros used for the debug message trace.

------------------------------------------------------------------------------
*/


#define MNS_DEBUG           NST_L_COMM
#define MNS_TRACE(args)     mns_trace args
extern void mns_trace(int service, int event, int pcnt, ...);

//#define MNS_TRACE(args)

#endif /* _ADJUST1_H */

