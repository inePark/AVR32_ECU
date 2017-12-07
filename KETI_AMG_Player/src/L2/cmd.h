/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Header File of MOST Command Interpreter
File:           cmd.h
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

#ifndef _CMD_H
#define _CMD_H





/*-----------------------------------------------------------------*/
/*  Definitions                                                    */
/*-----------------------------------------------------------------*/



/*-----------------------------------------------------------------*/
/*  Variables needed by MOST NetServices                           */
/*-----------------------------------------------------------------*/

#ifdef CMD_MIN
extern _CONST struct FBlock_L_Type FBlocks[];           /* FBlock Table (ROM) */
extern _CONST byte InstIDsDefault[];                    /* Array of InstIDs default values (ROM) */
#endif



/*-----------------------------------------------------------------*/
/*  Functions available within this module                         */
/*  needed by the MOST NetServices                                 */
/*-----------------------------------------------------------------*/



#endif /* _CMD_H */

/* end of cmd.h */
