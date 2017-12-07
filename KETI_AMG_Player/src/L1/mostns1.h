/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Public API of the MOST NetServices (Basic Layer)
File:           mostns1.h
Version:        3.0.x-SR-1  
Language:       C
Author(s):      S.Kerber, S.Semmler
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
  * \brief      Public API of the MOST NetServices (Basic Layer)
  */


#ifndef _MOSTNS1_H
#define _MOSTNS1_H

#include "rules_ad.h"


#include "pms_pb.h"     /* Port Message Service API                     */
#include "mbm_pb.h"     /* Message Buffer Management API                */
#include "mdm_pb.h"     /* Debug Messages API                           */
#include "mis_pb.h"     /* Message Interface Service API                */
#include "ams_pb.h"     /* Application Message Service API              */
#include "mns_pb.h"     /* MOST NetService Kernel API                   */
#include "wmcs_pb.h"    /* MOST Processor Control Service Wrapper API   */

#ifdef ADS_MIN
    #include "wads_pb.h"    /* Asynchronous Data Transmission Service API   */
#endif
#ifdef VMSV_MIN
    #include "vmsv_pb.h"    /* Virtual MOST Supervisor API                  */
#endif

#ifdef SCM_MIN
    #include "wscm_pb.h"    /* Socket Connection Manager Wrapper            */
#endif
#ifdef AAM_MIN
    #include "aam_pb.h"
#endif


#endif /* _MOSTNS1_H */
