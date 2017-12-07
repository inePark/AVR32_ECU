/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Public API of the MOST NetServices
File:           mostns.h
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
  * \brief      Public API of the MOST NetServices
  */

#ifndef _MOSTNS_H
#define _MOSTNS_H

#include "mostns1.h"        /* Basic Layer API          */

#ifdef SERVICE_LAYER_II
    #include "adjust2.h"
    #include "mostns2.h"    /* Application Layer API    */
#endif

#ifdef MOST_HIGH_PROTOCOL
    #include "adjustmh.h"
    #include "mhp_pb.h"     /* MOST High Protocol Service API   */
  #ifdef PMHT_MIN
    #include "pmht_pb.h"    /* Parallel MOST High Transceiver Service API   */
  #endif
#endif

#ifdef PACKETIZER_FOR_MOST_HIGH
    #include "adjustp.h"
    #include "pcktizr.h"
#endif

#endif /* _MOSTNS_H */
