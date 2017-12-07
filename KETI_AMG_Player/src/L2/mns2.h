/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Header File of MOST NetService Kernel (Application Socket)
File:           mns2.h
Version:        3.0.x-SR-1  
Language:       C
Author(s):      S.Kerber
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
Date    By      Description

==============================================================================
*/


#ifndef _MNS2_H
#define _MNS2_H



/*-----------------------------------------------------------------*/
/*  Definitions                                                    */
/*-----------------------------------------------------------------*/






/*-----------------------------------------------------------------*/
/*  Variables available within this module                         */
/*  needed by the MOST NetServices Layer II                        */
/*-----------------------------------------------------------------*/





/*-----------------------------------------------------------------*/
/*  Functions available within this module                         */
/*  needed by the MOST NetServices Layer II                        */
/*-----------------------------------------------------------------*/

#ifdef MNS2_10
void MnsPending2Set(word flags);
#endif

#ifdef MNS2_11
void MnsPending2SetAndCall(word flags);
#endif




#endif /* _MNS2_H */

/* end of mns2.h */




