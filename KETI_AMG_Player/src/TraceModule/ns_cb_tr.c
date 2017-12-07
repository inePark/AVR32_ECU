/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Trace Modul
File:           ns_cb_tr.c
Version:        3.0.3
Language:       C
Author(s):      R. Wilhelm
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



#ifndef _lint
#include <stdio.h>
#endif

#include "mostns.h"
#include "mnstrace.h"
#include "defines.h"


#if 0
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : PrintTraceLine()                                           */
/* Description : Callback function which prints a debug message             */
/*                                                                          */
/* Parameter(s): str  pointer to message to print                           */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void PrintTraceLine(const char *str)
{
    LOG_NOR("%s\n", str);
}


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : PrintErrorLine()                                           */
/* Description : Callback function which prints the parser's error          */
/*               and assert debug messagess                                 */
/* Parameter(s): str  pointer to message to print                           */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void PrintErrorLine(const char *str)
{
    LOG_ERR("%s\n", str);
}
#endif


