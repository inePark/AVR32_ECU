/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Trace Modul
File:           mnstrace.h
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




#ifndef _MNSTRACE_H
#define _MNSTRACE_H




/*--------------------------------------------------------------------------*/
/* Definitions of special filter types                                       */
/*--------------------------------------------------------------------------*/
#define FILT_INIC_WATCHDOG 0x0001
#define FILT_INIC_TIMER    0x0002

/*--------------------------------------------------------------------------*/
/* Definitions of ParmList ItemTypes                                        */
/*--------------------------------------------------------------------------*/
#define  TY_P_NO_PARM       ((word) 0x0001)
#define  TY_P_NUMBER        ((word) 0x0002)
#define  TY_P_ADDRESS       ((word) 0x0003)
#define  TY_P_FUNC          ((word) 0x0004)
#define  TY_P_EVENT         ((word) 0x0006)
#define  TY_P_FBLOCK        ((word) 0x0007)
#define  TY_P_FKTID         ((word) 0x0008)
#define  TY_P_OPTYPE        ((word) 0x0009)
#define  TY_P_ASSERT        ((word) 0x000A)
#define  TY_P_STATEMACHINE  ((word) 0x000B)
#define  TY_P_STATE         ((word) 0x000C)
#define  TY_P_SERVICE       ((word) 0x000D)
#define  TY_P_STEP          ((word) 0x000F)
#define  TY_P_CHIP          ((word) 0x0010)
#define  TY_P_L2_FLAG       ((word) 0x0011)
#define  TY_P_MHP_FLAG      ((word) 0x0012)
#define  TY_P_PMHS_FLAG     ((word) 0x0013)
#define  TY_P_SRVMASK       ((word) 0x0014)
#define  TY_P_CFGMASK       ((word) 0x0015)
#define  TY_P_TM_OP         ((word) 0x0016)
#define  TY_P_TM_EV         ((word) 0x0017)
#define  TY_P_TM_FLAGS      ((word) 0x0018)
#define  TY_P_TRANSCAUSE    ((word) 0x0019)


/*--------------------------------------------------------------------------*/
/* OpType types                                                             */
/*--------------------------------------------------------------------------*/
#define OP_FUNC  0x0000
#define OP_PROP  0x0010

/*--------------------------------------------------------------------------*/
/* list end tags                                                            */
/*--------------------------------------------------------------------------*/
#define EV_TERMINATION      ((word) 0xFFFF)
#define SRV_TERMINATION     ((word) 0xFFFF)
#define STR_TERMINATION     ((word) 0xFFFF)
#define PAR_TERMINATION     ((word) 0xFFFF)



/* max number of parameters */
#define MNS_TRACE_MAX_PARAM_COUNT 6


/*--------------------------------------------------------------------------*/
/* type definitions                                                         */
/*--------------------------------------------------------------------------*/

typedef dword TFilter;                              /* filter variables     */


typedef _CONST struct Ty_NumString                  /* numbered string      */
{
    word                num;                        /* item ID              */
    _CONST char         *str;                       /* description string   */
} TNumString, *pTNumString;


typedef _CONST struct Ty_ParmList                   /* Line of a parameter table    */
{
    int                 ItemType;                   /* parameter type               */
    _CONST char         *FormatString;              /* description of parameter     */
} TParmList, *pTParmList;


typedef _CONST struct Ty_Events                     /* Line of a Event ID table     */
{
    int                 Event;                      /* Event ID                     */
    pTParmList          ParmList;                   /* Pointer to parameter table   */
    _CONST char         *FormatString;              /* description of event         */
} TEvents, *pTEvents;


typedef _CONST struct Ty_Services                   /* line of T_Services table     */
{
    int                 Service;                    /* service ID                   */
    pTNumString         FuncList;                   /* pointer to event table       */
    TFilter             *ServiceFilter;             /* pointer to filter variable   */
    TFilter             DefaultFilter;              /* default value for service filter */
    _CONST char         *ServiceName;               /* service name                 */
} TServices, *pTServices;


typedef _CONST struct Ty_Funcs
{
    word                num;                        /* item ID              */
    char                type;                     /* property or function */
    _CONST char         *name;                       /* description string   */
} TFktList, *pTFktList;

typedef _CONST struct  Ty_FBlockList                 /* line of FBlockList table        */
{
    word                fbNum;                      /* fblock ID                        */
    _CONST char         *name;                      /* fblock name                      */
     TFktList           *fktList;                  /* pointer to belonging FktID table */
} TFBlockList, *pTFBlockList;

typedef _CONST struct  Ty_StateList             /* line of state table              */
{
    word                id;                     /* state id                         */
    _CONST char         *name;                  /* name of the state                */
} TStateList, *pTStateList;

typedef _CONST struct  Ty_StateMList            /* line of statemachine table       */
{
    word                id;                     /* state machine id                 */
    _CONST char         *name;                  /* name of the state machine        */
     TStateList         *sList;                 /* pointer to belonging state table */
} TStateMList, *pTStateMList;



/*--------------------------------------------------------------------------*/
/* function declarations                                                    */
/*--------------------------------------------------------------------------*/




/* API functions */
void MnsTraceInit(void);
void MnsTraceParser( int service, int event, unsigned long timestamp,
                        int num_parm, int *params );

byte SetTraceFilter(int service, TFilter filter);
byte GetTraceFilter(int service, TFilter *filter);
byte EnableFilterEvent(int service, int event);
byte DisableFilterEvent(int service, int event);

/* callback functions */
void PrintTraceLine(const char *str);
void PrintErrorLine(const char *str);




#endif
