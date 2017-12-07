/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Memory Logger
File:           mlog.c
Version:        3.0.3
Language:       C
Author(s):      R.Lundstrom
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
Date    By      Description

==============================================================================
*/
#ifndef _MLOG_H
#define _MLOG_H

#include "mostns.h"

/*
 * MLOG_INCLUDE can be defined if a header should be included without
 * modification of this file.
 */
#ifdef MLOG_INCLUDE
    #include MLOG_INCLUDE
#endif

/*
 * N.B.
 * The configuration can be done in adjust1.h or directly in this file.
 */

/*
================================================================================
    Configuration, function macros
================================================================================
*/
/* Define the timestamp function to give each log entry one. */
/*#define MLOG_TIMESTAMP() <e.g. GetTickCount()>*/ /* <return at least a 16-bit timestamp> */

/* Target specific memory copy, else a loop will be automatically defined */
/*#define MLOG_MEMCPY(dest, src, n)       <usually memcpy>(dest, src, n)*/

/* Set a function that should be called for each logged line */
/*#define MLOG_DUMP_LINE(u8_store, timestamp, u8_id, u8_ptr_data, data_len)  <dump function>(u8_store, timestamp, u8_id, u8_ptr_data, data_len)*/

/*
 * A Critical section is required if adding lines to a store can be done from
 * different contexts.
 * It is recommended, but not entirely necessary if only dumping could be done
 * from a different context, though without the first or last line migh not be
 * correct when dumping in case a write operation was preempted by the dump.
 * In a single context it is not needed at all.
 */
/*#define MLOG_ENTER_CRITICAL_SECTION() <E.g. EnterCriticalSection(&crit)>*/
/*#define MLOG_LEAVE_CRITICAL_SECTION() <E.g. LeaveCriticalSection(&crit)>*/

/*
================================================================================
    Configuration, log stores
================================================================================
*/
/*#define MLOG_NUM_STORES   2*/   /*!< Number of separate stores */
/*#define MLOG_NUM_LINES  200*/   /*!< Number of lines in each store */
/*#define MLOG_LINE_SIZE   16*/   /*!< Bytes per line (excluding timestamp) */

#if MLOG_NUM_STORES && MLOG_NUM_LINES && MLOG_LINE_SIZE
    #define MLOG_ENABLED

    /*
    ================================================================================
        Types
    ================================================================================
    */
    typedef unsigned short MLogTimestamp; /*!< For easy modification of the size of timestamps */

    /*! A line in the log */
    typedef struct MLogLine
    {
        MLogTimestamp timestamp;
        unsigned char id; /*!< Can be used to distinguish data in a log, e.g. 0 = RX and 1 = TX */
        unsigned char data[MLOG_LINE_SIZE];
    } MLogLine;

    /*! A separate log store */
    typedef struct MLogStore
    {
        unsigned int  line;   /*!< Next input line (also oldest) */
        unsigned char locked; /*!< Locked for dumping (no write) */
        MLogLine lines[MLOG_NUM_LINES];
    } MLogStore;

    /*
    ================================================================================
        Global variables
    ================================================================================
    */
    extern MLogStore mlog_stores[]; /* Exported for easy external access */

    /*
    ================================================================================
        API
    ================================================================================
    */
    void MLogInitAll(void);
    void MLogAddLine(unsigned char store, unsigned char id, unsigned char *src_ptr);
    void MLogDumpStore(unsigned char store);
    void MLogDumpAll(void);
    void MLogLockStore(unsigned char store);
    void MLogLockAll(void);
#else
    #define MLogInitAll()
    #define MLogAddLine(s,i,p)
    #define MLogDumpStore(s)
    #define MLogDumpAll()
    #define MLogLockStore(s)
    #define MLogLockAll()
#endif

#endif /* Header guard */
