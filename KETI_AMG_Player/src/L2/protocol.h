/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         General Definitions of MOST Protocol
File:           protocol.h
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

#ifndef _PROTOCOL_H
#define _PROTOCOL_H





/*------------------------------------------------------------------- */
/* General FunctionIDs                                                */
/*------------------------------------------------------------------- */

/* Mandatory: */
/*------------ */
#define FUNC_FKTIDS             ((word)0x000)   /* Availabe Function IDs */
#define FUNC_NOTIFICATION       ((word) 0x001)   /* Notification of events */
#define FUNC_NOTIFICATIONCHECK  ((word) 0x002)   /* NotificationCheck */

/* Sync Sources: */
/*--------------- */
#define FUNC_SOURCEINFO         ((word) 0x100)   /* Source Info */
#define FUNC_ALLOC              ((word) 0x101)   /* Allocate */
#define FUNC_DEALLOC            ((word) 0x102)   /* De-Allocate */
#define FUNC_SOURCEACT          ((word) 0x103)   /* Source Activity */
#define FUNC_SOURCENAME         ((word) 0x104)   /* Source Name */

/* Sync Sinks: */
/*------------- */
#define FUNC_SINKINFO           ((word) 0x110)   /* Sink Info */
#define FUNC_CONNECT            ((word) 0x111)   /* Connect */
#define FUNC_DISCONNECT         ((word) 0x112)   /* Disconnect */
#define FUNC_MUTE               ((word) 0x113)   /* Mute */
#define FUNC_SINKNAME           ((word) 0x114)   /* Sink Name */
#define FUNC_CONNECTTO          ((word) 0x115)   /* Connect To */

/* Sync Source or Sink: */
/*---------------------- */
#define FUNC_SYNCDATAINFO       ((word) 0x116)   /* Syncronous Data Info */

/* Multi-MMI-Handling: */
/*--------------------- */
#define FUNC_RESUSERS           ((word) 0x120)   /* Ressource Users */
/*------------------------------------------------------------------- */


/*------------------------------------------------------------------- */
/* FunctionIDs of FBlock NetBlock                                     */
/*------------------------------------------------------------------- */
/*                                                                    */
/*   NetBlock FktIDs are defined in mostdef1.h                        */
/*                                                                    */
/*------------------------------------------------------------------- */


/*------------------------------------------------------------------- */
/* FunctionIDs of FBlock NetworkMaster                                */
/*------------------------------------------------------------------- */
#define FUNC_CONFIGURATION      ((word) 0xA00)   /* Configuration */
#define FUNC_CENTRALREGISTRY    ((word) 0xA01)   /* Central Registry */
#define FUNC_SAVECONFIG         ((word) 0xA02)   /* Save Configuration */
#define FUNC_OWNCONFIGINVALID   ((word) 0xA03)   /* OwnConfigInvalid */
#define FUNC_SYSTEMAVAIL        ((word) 0xA10)   /* System Availability */
/*------------------------------------------------------------------- */


/*------------------------------------------------------------------- */
/* FunctionIDs of FBlock ConnectionMaster                             */
/*------------------------------------------------------------------- */
#define FUNC_BUILDSYNCCON       ((word) 0x200)   /* Build Sync Connection */
#define FUNC_REMOVESYNCCON      ((word) 0x201)   /* Remove Sync Connection */
#define FUNC_SYNCCONTABLE       ((word) 0x400)   /* Sync Connection Table */
#define FUNC_AVAILCHANNELS      ((word) 0x401)   /* Available Channels */

/*------------------------------------------------------------------- */








/*------------------------------------------------------------------- */
/* Parameters for NWM.Configuration.Status()                          */
/*------------------------------------------------------------------- */
#define NWM_CFG_STATUS_NOTOK        ((byte)0x00)
#define NWM_CFG_STATUS_OK           ((byte)0x01)
#define NWM_CFG_STATUS_INVALID      ((byte)0x02)
#define NWM_CFG_STATUS_NEW          ((byte)0x03)
#define NWM_CFG_STATUS_NEWEXT       ((byte)0x04)


/*------------------------------------------------------------------- */
/* String Format Identifiers                                          */
/*------------------------------------------------------------------- */

#define FRMT_UNICODE_UTF16  ((byte)0x00)
#define FRMT_ISO8859        ((byte)0x01)
#define FRMT_UNICODE_UTF8   ((byte)0x02)
#define FRMT_RDS            ((byte)0x03)
#define FRMT_DAB_SET_0001   ((byte)0x04)
#define FRMT_DAB_SET_0010   ((byte)0x05)
#define FRMT_DAB_SET_0011   ((byte)0x06)
#define FRMT_SHIFT_JIS      ((byte)0x07)


/*------------------------------------------------------------------- */
/* FBlockInfo: Maturity Identifiers                                   */
/*------------------------------------------------------------------- */

#define MAT_UNKNOWN                     ((byte)0x00)
#define MAT_INTERFACE_ONLY              ((byte)0x01)
#define MAT_PARTLY_IMPLEMENTED          ((byte)0x02)
#define MAT_FULLY_IMPLEMENTED           ((byte)0x03)
#define MAT_PARTLY_IMPLEMENTED_VERIFIED ((byte)0x11)
#define MAT_FULLY_IMPLEMENTED_VERIFIED  ((byte)0x12)

/*------------------------------------------------------------------- */
/* FBlockInfo: Condition Identifiers                                   */
/*------------------------------------------------------------------- */

#define FBI_ALL                 ((word)0xF000)
#define FBI_FBLOCK_NAME         ((word)0xF001)
#define FBI_SUPPLIER_VERSION    ((word)0xF002)
#define FBI_FBLOCK_VERSION      ((word)0xF003)
#define FBI_MOST_VERSION        ((word)0xF004)
#define FBI_SYSTEM_INTEGRATOR   ((word)0xF005)
#define FBI_FBLOCK_TYPE         ((word)0xF006)




#endif /* _PROTOCOL_H */




