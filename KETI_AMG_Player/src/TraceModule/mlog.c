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

===============================================================================
*/

#include "mlog.h"

#ifdef MLOG_ENABLED
/*
================================================================================
    Macros
================================================================================
*/
#ifndef MLOG_MEMCPY
    /* Basic loop copy operation if MLOG_MEMCPY isn't defined */
    #define MLOG_MEMCPY(d_ptr, s_ptr, n) {unsigned int pos=0;while(pos<n){d_ptr[pos]=s_ptr[pos];pos++;}}
#endif

#ifndef MLOG_TIMESTAMP
    /* No timestamp */
    #define MLOG_TIMESTAMP()  0
#endif

/* Void macros if they aren't defined */
#ifndef MLOG_ENTER_CRITICAL_SECTION
    #define MLOG_ENTER_CRITICAL_SECTION()
#endif
#ifndef MLOG_LEAVE_CRITICAL_SECTION
    #define MLOG_LEAVE_CRITICAL_SECTION()
#endif
#ifndef MLOG_DUMP_LINE
    #define MLOG_DUMP_LINE(s,t,i,d,n)
#endif

/*
================================================================================
    Variables
================================================================================
*/
/*! Array of log stores, global for easy access from external modules */
MLogStore mlog_stores[MLOG_NUM_STORES];

/*
================================================================================
    Functions
================================================================================
*/
/*!
 * \brief       Initializes all logs. Actually only makes sure they are
 *              unlocked. Should not be necessary to call if uninitialized
 *              variables are set to zero at startup.
 */
void MLogInitAll(void)
{
    unsigned char store = MLOG_NUM_STORES;

    while( store-- ) /* Last index is MLOG_NUM_STORES - 1, first is 0 */
    {
        mlog_stores[store].locked = 0; /* No need to clear line, circular */
    }
}
/*----------------------------------------------------------------------------*/

/*!
 * \brief       Adds a line to a log in a store.
 *
 * \param[in]   store   - Log store
 * \param[in]   id      - ID of the message, used to distinguish different
 *                        types within a store, e.g. 0 = RX and 1 = TX.
 * \param[in]   src_ptr - The source data to copy to the logged line.
 */
void MLogAddLine(unsigned char store, unsigned char id, unsigned char *src_ptr)
{
    MLogLine  *line_ptr;
    MLogStore *store_ptr = &mlog_stores[store];

    MLOG_ENTER_CRITICAL_SECTION();
    if( !store_ptr->locked )
    {
        if( store_ptr->line >= (unsigned int)MLOG_NUM_LINES )
        {
            store_ptr->line = 0;
        }
        line_ptr = &store_ptr->lines[store_ptr->line++];

        line_ptr->timestamp = (MLogTimestamp)MLOG_TIMESTAMP();
        line_ptr->id = id;
        (void)MLOG_MEMCPY(line_ptr->data, src_ptr, MLOG_LINE_SIZE);
    }
    MLOG_LEAVE_CRITICAL_SECTION();
}
/*----------------------------------------------------------------------------*/

/*!
 * \brief       Dumps all logged lines of a store. Locks the store first if
 *              it is unlocked, to avoid further added lines while dumping.
 *
 * \param[in]   store - The log store to dump.
 */
void MLogDumpStore(unsigned char store)
{
    MLogStore *store_ptr = &mlog_stores[store];
    unsigned int num_lines = MLOG_NUM_LINES;
    unsigned int line;

    if( !store_ptr->locked )
    {
        MLOG_ENTER_CRITICAL_SECTION();
        /* Dumping a store form different contexts is not supported, hence no check */
        store_ptr->locked = 1;
        MLOG_LEAVE_CRITICAL_SECTION();
    }

    line = store_ptr->line; /* Oldest stored is the next input line */
    while( num_lines-- )
    {
        MLogLine *line_ptr;

        if( line >= (unsigned int)MLOG_NUM_LINES )
        {
            line = 0;
        }
        line_ptr = &store_ptr->lines[line++];

        MLOG_DUMP_LINE(store, line_ptr->timestamp, line_ptr->id, line_ptr->data, MLOG_LINE_SIZE);
    }

    store_ptr->locked = 0; /* Not necessary to lock critical section when releasing */
}
/*----------------------------------------------------------------------------*/

/*!
 * \brief       Dumps all log stores by first locking them all from further
 *              write access and then dumping each in order.
 */
void MLogDumpAll(void)
{
    unsigned char store = MLOG_NUM_STORES;

    /* Lock all stores first */
    MLOG_ENTER_CRITICAL_SECTION();
    while( store )
    {
        store--; /* 0...(MLOG_NUM_STORES-1) */
        mlog_stores[store].locked = 1;
    }
    MLOG_LEAVE_CRITICAL_SECTION();

    /* store = 0, dump stores in order */
    while( store < (unsigned char)MLOG_NUM_STORES )
    {
        MLogDumpStore(store++);
    }
}
/*----------------------------------------------------------------------------*/

/*!
 * \brief       Locks a store from further added lines
 *
 * \param[in]   store - Log store to lock
 */
void MLogLockStore(unsigned char store)
{
    MLOG_ENTER_CRITICAL_SECTION();
    mlog_stores[store].locked = 1;
    MLOG_LEAVE_CRITICAL_SECTION();
}
/*----------------------------------------------------------------------------*/

void MLogLockAll(void)
{
    unsigned char store = MLOG_NUM_STORES;

    MLOG_ENTER_CRITICAL_SECTION();
    while( store )
    {
        store--; /* 0..(MLOG_NUM_STORES-1) */
        mlog_stores[store].locked = 1;
    }
    MLOG_LEAVE_CRITICAL_SECTION();
}
/*----------------------------------------------------------------------------*/

#endif /* #if MLOG_NUM_STORES && MLOG_NUM_LINES && MLOG_LINE_SIZE */
