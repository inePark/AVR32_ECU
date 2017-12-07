/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Adjustment rules for MOST NetService Kernel (Basic Layer)
File:           rules_ad.h
Version:        3.0.x-SR-1  
Language:       C
Author(s):      S.Semmler
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
  * \brief      Adjustment rules for MOST NetService Kernel (Basic Layer)
  */

#ifndef _RULES_AD_H
#define _RULES_AD_H


/*
-------------------------------------------------------------------------------
 MOST NetServices version number
-------------------------------------------------------------------------------

    The version number is intentionally defined here (before including adjust1.h).
    This allows version dependent settings in the adjust files.
*/
#define MNS_VERSION_MAJOR     0x03
#define MNS_VERSION_MINOR     0x00
#define MNS_VERSION_RELEASE   0x04
#define MNS_VERSION_STEP      0xD0

#include "adjust1.h"


/*
------------------------------------------------------------------------------
    Debug and Trace rules
------------------------------------------------------------------------------
*/

#ifndef MNS_DEBUG
    #define MNS_DEBUG 0
#endif


/*
------------------------------------------------------------------------------
    Rules dealing with name space
------------------------------------------------------------------------------
*/

#ifdef MNS_TRUE
    #error MNS_TRUE is reserved for MOST NetServices
#endif

#ifdef MNS_FALSE
    #error MNS_FALSE is reserved for MOST NetServices
#endif

#ifdef MNS_ON
    #error MNS_ON is reserved for MOST NetServices
#endif

#ifdef MNS_OFF
    #error MNS_OFF is reserved for MOST NetServices
#endif

/*
------------------------------------------------------------------------------
    INIC variant settings
------------------------------------------------------------------------------
*/


#ifndef MOST_INIC_ID
    #define MOST_INIC_ID 0x03  /* default: INIC150 */
#endif

#if (MOST_INIC_ID == 0x02)    /* INIC50 */
    #define _OS81082_PERMIT
    #define DISABLE_ES_0201XX_025
#elif (MOST_INIC_ID == 0x03)    /* INIC150 */
    #define _OS81110_PERMIT
    #define _OS81110_MDM
    #define _OS81110_MDP_MAX_PL
    #define _OS81110_MEP
    #define _OS81110_ISO
    #define _OS81110_PCK_LLR
    #define _OS81110_MCM_LLR
    #define _OS81110_SPI
    #define _OS81110_QOS
    #define _OS81110_SEG_MODE
    #define _OS81110_SSO
    #define _OS81110_WD_TH
    #define DISABLE_ES_0201XX_025

#else
    #error "unknown INIC derivative selected"
#endif


#if (MOST_INIC_ID == 0x02)    /* INIC50 */
    #ifdef MSV_DIAG_RESULT_MSG
        #error "MSV_DIAG_RESULT_MSG is not available for MOST50 systems."
    #endif
#endif


/*
------------------------------------------------------------------------------
    Default rules
------------------------------------------------------------------------------
*/

    /* ___ Default services ___ */

    #ifndef MBM_MIN
        #define MBM_MIN
    #endif

    #ifndef PMS_MIN
        #define PMS_MIN
    #endif

    #ifndef MNS_MIN
        #define MNS_MIN
    #endif

    #ifndef MIS_MIN
        #define MIS_MIN
    #endif

    #ifndef AMS_MIN
        #define AMS_MIN
    #endif

    #ifdef MDM_LEVEL
        #if (MDM_LEVEL != 0) && (defined _OS81110_MDM) && (!defined PACKET_COMMUNICATION_ONLY)
            #define MDM_MIN
        #endif
    #else
        #define MDM_LEVEL 0
    #endif

    #if (!defined MCS_MIN) && (!defined PACKET_COMMUNICATION_ONLY)
        #define MCS_MIN
    #endif


    #ifdef PACKET_COMMUNICATION_ONLY
        #ifdef VMSV_MIN                     /* in case the user has defined VMSV_MIN in adjust1.h */
            #undef VMSV_MIN
        #endif
    #else
        #ifndef VMSV_MIN
            #define VMSV_MIN
        #endif
    #endif
    

    #ifndef MNS_MEM_CPY
        #define MNS_MEM_CPY    MnsMemCpy
    #endif

    #ifndef MNS_MEM_SET
        #define MNS_MEM_SET    MnsMemSet
    #endif

    /* ___ Message Buffer Module ___ */
    #ifdef MBM_ADD1
        #error "MBM_ADD1 is currently not supported"
    #endif

    /* ___ Port Message Service ___ */

    #ifndef PMS_OFFSETOF
        #error "Since MOST NetServices V2.1.0 the macro PMS_OFFSETOF needs to be declared in adjust1.h (please see example)."
    #endif

    #if (defined PMS_RX_OPT3) && (defined PMS_RX_OPT4)
        #ifndef ALLOW_AUTO_CONFIG
            #error "It is not allowed to define PMS_RX_OPT4 together with PMS_RX_OPT3."
        #else
            #undef PMS_RX_OPT4
        #endif
    #endif

    #ifdef AMS_RX_NOSEG
        #undef AMS_RX_NOSEG
    #endif

    #ifdef MSG_RX_AUTOMATIC_ERR_MSG
        #undef MSG_RX_AUTOMATIC_ERR_MSG
    #endif


    #ifdef _OS81110_SEG_MODE
      #if defined (MSG_RX_SEG_INIC_ONLY) || defined (MSG_RX_SEG_COOPERATIVE)
          #error Definition of MSG_RX_SEG_INIC_ONLY or MSG_RX_SEG_COOPERATIVE not allowed for MOST150
      #endif
      #if (!defined MSG_RX_SEG_EHC_ONLY)
          #define  MSG_RX_SEG_EHC_ONLY
      #endif
    #else
      #ifdef MSG_RX_SEG_COOPERATIVE
          #undef MSG_RX_SEG_INIC_ONLY
          #undef MSG_RX_SEG_EHC_ONLY
      #elif (defined MSG_RX_SEG_EHC_ONLY)
          #undef MSG_RX_SEG_INIC_ONLY
      #elif (!defined MSG_RX_SEG_INIC_ONLY)
          #define MSG_RX_SEG_COOPERATIVE
      #endif
    #endif

    #if defined(MSG_RX_SEG_INIC_ONLY) || defined(PACKET_COMMUNICATION_ONLY)
        #define AMS_RX_NOSEG
    #endif

    #ifdef MSG_RX_SEG_EHC_ONLY
        #define MSG_RX_AUTOMATIC_ERR_MSG
    #endif

    #ifdef PMS_RX_OPT3
        #ifndef MSG_RX_SEG_EHC_ONLY
            #error "MSG_RX_SEG_EHC_ONLY is needed if PMS_RX_OPT3 is defined."
        #endif

        #ifndef PMS_RX_OPT3_CHADDR
            #error "PMS_RX_OPT3_CHADDR is needed if PMS_RX_OPT3 is defined."
        #elif (PMS_RX_OPT3_CHADDR <= 0x04)
            #error "PMS_RX_OPT3_CHADDR must be bigger than 0x04."
        #endif
    #endif


    #ifdef AMS_TX_NOSEG
        #define PMS_TX_NOSEG
    #endif

    #ifdef AMS_RX_NOSEG
        #define PMS_RX_NOSEG
        #undef PMS_RX_SEG_PREALLOC_SIZE
        #undef MSG_RX_AUTOMATIC_ERR_MSG
        #undef PMS_USE_GARBAGE_COLLECTOR
    #endif
    /*
     * If PMS_RX_SEG_PREALLOC_SIZE is not defined set it to 0 to revert
     * to the old behavior of NSv2 where it was possible to receive
     * segmented messages larger than PMS_RX_SEG_PREALLOC_SIZE
     */
    #ifndef PMS_RX_SEG_PREALLOC_SIZE
        #define PMS_RX_SEG_PREALLOC_SIZE        0
    #endif

    #ifndef AMS_RX_NOSEG
        #if (PMS_RX_SEG_PREALLOC_SIZE <= 45)
            #ifndef MSG_RX_USER_PAYLOAD
                #error "V3.0.x requires PMS_RX_SEG_PREALLOC_SIZE > 45 or MSG_RX_USER_PAYLOAD to be defined."
            #elif (PMS_RX_SEG_PREALLOC_SIZE != 0)
                #error "V3.0.x requires PMS_RX_SEG_PREALLOC_SIZE > 45 or PMS_RX_SEG_PREALLOC_SIZE not defined."
            #endif
        #elif (defined MSG_RX_USER_PAYLOAD)
            #error "It is not allowed to define MSG_RX_USER_PAYLOAD together with PMS_RX_SEG_PREALLOC_SIZE"
        #endif
    #endif

    #ifdef MSG_RX_SEG_EHC_ONLY
        #ifndef PMS_USE_GARBAGE_COLLECTOR
            #define PMS_USE_GARBAGE_COLLECTOR
        #endif
    #endif

    /*
     * MDP_MIN is a subset of ADS_MIN and if it alone is defined it
     * automatically discards and acknowledges MDP over I2C, since a timeout
     * on an ack sets the INIC in protected mode. It is not necessary if the
     * INIC is started with MLB as default interface (or if the INIC can be
     * prevented from forwarding MDP), hence in those cases
     * MDP_NO_AUTO_DISCARD can be defined to save a little space (only
     * makes any difference without ADS_MIN of course).
     */
    #if defined(ADS_MIN) || !defined(MDP_NO_AUTO_DISCARD)
        #define MDP_MIN
    #else
        /* MDP_MIN requires either ADS_MIN or NOT MDP_NO_AUTO_DISCARD */
        #undef MDP_MIN
    #endif

    /* ___ Application Message Service ___ */
    #ifdef AMS_MIN
        #if (defined MSG_TX_USER_PAYLOAD_EXT_CB) && !(defined MSG_TX_USER_PAYLOAD)
            #error MSG_TX_USER_PAYLOAD_EXT_CB requires MSG_TX_USER_PAYLOAD
        #endif
    #endif

    /* ___ Asynchronous Data Service ___ */
    #ifdef ADS_MIN
        #ifndef ADS_RX_PREALLOC
            #define ADS_RX_PREALLOC 1
        #endif
        #if (defined DATA_TX_USER_PAYLOAD_EXT_CB) && !(defined DATA_TX_USER_PAYLOAD)
            #error DATA_TX_USER_PAYLOAD_EXT_CB requires DATA_TX_USER_PAYLOAD
        #endif
    #else
        #undef  DATA_TX_USER_PAYLOAD
        #undef  DATA_TX_USER_PAYLOAD_EXT_CB
        #undef  ADS_RX_PREALLOC  /* Just in case */
        #define ADS_RX_PREALLOC 0
    #endif

    #ifndef MDP_DEFAULT_RETRY
        #define MDP_DEFAULT_RETRY   0x01
    #endif

    #ifndef MEP_DEFAULT_RETRY
        #define MEP_DEFAULT_RETRY   0x01
    #endif

    #ifndef ADS_RX_PREALLOC
        #define ADS_RX_PREALLOC 0
    #endif



    /* ___ Legacy switches ___ */

    #ifdef SERVICE_LAYER_II
        #ifndef DECLARE_LEGACY_SWITCHES
            #ifndef ALLOW_AUTO_CONFIG
                #error "DECLARE_LEGACY_SWITCHES is needed if SERVICE_LAYER_II."
            #else
                #define DECLARE_LEGACY_SWITCHES
            #endif
        #endif

        #ifndef DECLARE_LEGACY_API
            #ifndef ALLOW_AUTO_CONFIG
                #error "DECLARE_LEGACY_API is needed if SERVICE_LAYER_II."
            #else
                #define DECLARE_LEGACY_API
            #endif
        #endif
    #endif

    #ifdef MOST_HIGH_PROTOCOL
        #ifndef DECLARE_LEGACY_SWITCHES
            #ifndef ALLOW_AUTO_CONFIG
                #error "DECLARE_LEGACY_SWITCHES is needed if MOST_HIGH_PROTOCOL."
            #else
                #define DECLARE_LEGACY_SWITCHES
            #endif
        #endif

        #ifndef DECLARE_LEGACY_API
            #ifndef ALLOW_AUTO_CONFIG
                #error "DECLARE_LEGACY_API is needed if MOST_HIGH_PROTOCOL."
            #else
                #define DECLARE_LEGACY_API
            #endif
        #endif
    #endif

    /* ___ Service Layer II Support ___ */

    #if (defined SERVICE_LAYER_II) && (!defined PACKET_COMMUNICATION_ONLY)
        #include "adjust2.h"
    #endif

    /* ___ MOST High Protocol Support ___ */
    #ifdef MOST_HIGH_PROTOCOL
        #include "adjustmh.h"

        #ifdef MHP_NO_INTF_MNS
            #ifndef ALLOW_AUTO_CONFIG
                #error "MHP_NO_INTF_MNS is not supported by the MOST NetServices V3.x for INIC."
            #else
                #undef MHP_NO_INTF_MNS
            #endif
        #endif
    #endif




/*
------------------------------------------------------------------------------
    Use Case PACKET_COMMUNICATION_ONLY
------------------------------------------------------------------------------
*/

#if (defined PACKET_ON_SECOND_HOST) && (defined PACKET_COMMUNICATION_ONLY)
    #error "PACKET_ON_SECOND_HOST and PACKET_COMMUNICATION_ONLY defined on the same host!"
#endif

#ifdef PACKET_COMMUNICATION_ONLY


    /* ___ "Always on" Services which are not needed in this use case ___ */
    #ifdef AMS_MIN
        #undef AMS_MIN
    #endif

    #ifdef MCS_MIN
        #undef MCS_MIN
    #endif

    /* ___ Optional Services which are needed ___ */

    #ifndef ADS_MIN
        #ifndef ALLOW_AUTO_CONFIG
            #error "ADS_MIN is needed if PACKET_COMMUNICATION_ONLY."
        #else
            #define ADS_MIN
        #endif
    #endif

    #undef  ADS_RX_PREALLOC  /* Just in case */
    #define ADS_RX_PREALLOC 0

    #ifndef MAX_DATA_TX_HANDLE
        #define MAX_DATA_TX_HANDLE 0
    #endif

    #ifndef MAX_DATA_EXT_DATA
        #define MAX_DATA_EXT_DATA 0
    #endif

    #ifndef MNS_AVOID_ATTACH
        #define MNS_AVOID_ATTACH
    #endif


/* ___ Optional Services which are not needed ___ */

    #ifdef SCM_MIN
        #ifndef ALLOW_AUTO_CONFIG
            #error "SCM_MIN is not possible if PACKET_COMMUNICATION_ONLY."
        #else
            #undef SCM_MIN
        #endif
    #endif

    #ifdef AAM_MIN
       #ifndef ALLOW_AUTO_CONFIG
           #error "AAM_MIN is not possible if PACKET_COMMUNICATION_ONLY."
       #else
           #undef AAM_MIN
       #endif
    #endif

    /* ___ Service Layer II Support ___ */

    #ifdef SERVICE_LAYER_II
        #ifndef ALLOW_AUTO_CONFIG
            #error "SERVICE_LAYER_II is not possible if PACKET_COMMUNICATION_ONLY."
        #else
            #undef SERVICE_LAYER_II
        #endif
    #endif

    #ifdef NS_AMS_AH
        #ifndef ALLOW_AUTO_CONFIG
            #error "NS_AMS_AH is not possible if PACKET_COMMUNICATION_ONLY."
        #else
            #undef NS_AMS_AH
        #endif
    #endif

    #ifdef NS_MNS_MNS2
        #ifndef ALLOW_AUTO_CONFIG
            #error "NS_MNS_MNS2 is not possible if PACKET_COMMUNICATION_ONLY."
        #else
            #undef NS_MNS_MNS2
        #endif
    #endif

    #ifdef NS_MSV_NB
        #ifndef ALLOW_AUTO_CONFIG
            #error "NS_MSV_NB is not possible if PACKET_COMMUNICATION_ONLY."
        #else
            #undef NS_MSV_NB
        #endif
    #endif

    #ifdef NS_MSV_ET
        #ifndef ALLOW_AUTO_CONFIG
            #error "NS_MSV_ET is not possible if PACKET_COMMUNICATION_ONLY."
        #else
            #undef NS_MSV_ET
        #endif
    #endif

#endif



#endif /* _RULES_AD_H */
