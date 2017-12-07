/*
==============================================================================

Project:        MOST NetServices V2 for INIC
Module:         Core Application Framework (Layer I Callbacks)
File:           ns_cb_l1.h
Version:        2.0.3
Language:       C
Author(s):      S.Semmler
Date:           20.May.2005

FileGroup:      Layer I
Customer ID:    C058FE02020003.P.OSSGER.MODULES
FeatureCode:    FCR1.G1.0000.G2.0000.G7.0000
------------------------------------------------------------------------------

                (c) Copyright 1998-2005
                Oasis SiliconSystems AG
                All Rights Reserved

------------------------------------------------------------------------------



Modifications
~~~~~~~~~~~~~
Date            By      Description

==============================================================================
*/

#ifndef _MNS_CB_L1_H
#define _MNS_CB_L1_H


void prepare_mns_config(TNetServicesConfig *cfg_ptr);
void sys_take(int mutex_id);
void sys_give(int mutex_id);
void prepare_mns_config(TNetServicesConfig *cfg_ptr);

#endif /* _MNS_CB_L1_H */

