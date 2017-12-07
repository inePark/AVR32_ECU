/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Trace Modul
File:           mnstrace.c
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





/*
------------------------------------------------------------------------------
        Include Files
------------------------------------------------------------------------------
*/
#ifndef _lint
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#endif

#include "mostns.h"

#include "mnstrace.h"

#include "mnstrace.tab"




/*
------------------------------------------------------------------------------
        Macro Definitions
------------------------------------------------------------------------------
*/
#define ADD_STR(a)          strcat(lineout, a); printLine = MNS_TRUE;
#define ADD_STRING(a,b)     sprintf(strtemp, (a), (b)); strcat(lineout, strtemp); printLine = MNS_TRUE;

#define ADD_ERROR(a,b)      sprintf(strtemp, (a), (b)); strcat(lineout, strtemp); printError = MNS_TRUE;

#define ADD_TIMESTAMP()     ADD_STRING("%10lu ", timestamp);


/*--------------------------------------------------------------------------*/
/* function declarations                                                    */
/*--------------------------------------------------------------------------*/

char* GetName(TNumString *list, int func);
pTFBlockList GetFBlock(TFBlockList *list, int fblock);
pTFktList GetFktName(pTFktList list, int item);
byte MnsTraceSpecialFilter(int service, int event, int num_parm, int *params);

pTStateMList GetStateM(pTStateMList list, int item);
pTStateList GetState(pTStateList list, int item);

int SpecialFilter;

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : MnsTraceInit()                                             */
/* Description : Initializes the trace filters                              */
/*                                                                          */
/* Parameter(s): none                                                       */
/*                                                                          */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void MnsTraceInit(void)
{
    pTServices  serviceTabPtr;

    serviceTabPtr = T_Services;

    while (SRV_TERMINATION != serviceTabPtr->Service)
    {
        SetTraceFilter(serviceTabPtr->Service, serviceTabPtr->DefaultFilter);
        serviceTabPtr++;
    }
    SetTraceFilter(SRV_TERMINATION, 0);                     /* filter all special items out*/
}



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : MnsTraceParser()                                           */
/* Description : Parses and filters debug messages                          */
/*                                                                          */
/* Parameter(s): service  module where the message is released              */
/*               event    event which causes the message                    */
/*               timestamp time stamp                                       */
/*               num_parm  number of following parameters                   */
/*               params    pointer to parameter list of variable length     */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void MnsTraceParser( int service, int event, unsigned long timestamp,
                        int num_parm, int *params )
{
    pTServices  serviceTabPtr = NULL;
    pTEvents    eventTabPtr   = NULL;
    pTParmList  parmListPtr   = NULL;

    char lineout[200];
    char strtemp[100];
    char *pParmStr;


    pTFBlockList pFblock;
    pTFktList    pFkt;

    pTStateMList pStateM;
    pTStateList  pState;

    int parm;
    int i;
    unsigned char found;
    unsigned char printLine;
    unsigned char printError;
    unsigned char exitFlag;


    *lineout      = '\0';
    printLine     = MNS_FALSE;
    printError    = MNS_FALSE;
    found         = MNS_FALSE;
    exitFlag      = MNS_FALSE;

    pFblock       = NULL;
    pFkt          = NULL;
    pStateM       = NULL;
    pState        = NULL;
    serviceTabPtr = T_Services;

    /*--------------------------------------------------------------------------*/
    /* check for special filters*/
    /*--------------------------------------------------------------------------*/
    if (MNS_FALSE == MnsTraceSpecialFilter(service, event, num_parm, params))
    {
        exitFlag  = MNS_TRUE;                                                   /* abort parsing*/
    }

    /*--------------------------------------------------------------------------*/
    /* search for service*/
    /*--------------------------------------------------------------------------*/
    if (MNS_P_NONE == service)
    {
        ADD_TIMESTAMP();
        ADD_ERROR("ERROR %s", "Service not defined!");
        exitFlag  = MNS_TRUE;                                                   /* abort parsing*/
    }

    if (!exitFlag)
    {
        while (!found && (SRV_TERMINATION != serviceTabPtr->Service))       /* scan the services list*/
        {
            if (serviceTabPtr->Service == service)                          /* service found?*/
            {
                found = MNS_TRUE;
            }
            else
                serviceTabPtr++;
        }
        if (!found)
        {
            ADD_TIMESTAMP();
            ADD_ERROR("ERROR Unknown service: 0x%04x", service);
            exitFlag  = MNS_TRUE;                                                   /* abort parsing*/
        }
    }
    /*--------------------------------------------------------------------------*/
    /* search for event*/
    /*--------------------------------------------------------------------------*/
    if (!exitFlag)
    {
        found = MNS_FALSE;
        if (*serviceTabPtr->ServiceFilter & (0x00000001 << (event-1)))      /*  event enabled ? */
        {
            ADD_TIMESTAMP();
            ADD_STRING("%s",    serviceTabPtr->ServiceName);               /* service name*/
            eventTabPtr = T_Events;
            while (!found && (EV_TERMINATION != eventTabPtr->Event))    /* scan the event list */
            {
                if (eventTabPtr->Event == event)                        /*  event found */
                {
                    found = MNS_TRUE;
                    parmListPtr = eventTabPtr->ParmList;                /* get parameter list */
                }
                else
                {
                    eventTabPtr++;
                }
            }
        }
        else
        {
            exitFlag  = MNS_TRUE;                                           /* abort parsing */
            printLine = MNS_FALSE;                                          /* do not print anything */
        }


        /*-------------------------------------------------------------------------- */
        /* search parameter list */
        /*-------------------------------------------------------------------------- */
        if (!exitFlag)
        {
            if (!found)
            {
                ADD_STRING("unknown event: 0x%04X", event);                     /* event not found in list */
            }
            else
            {
                ADD_STRING("%s", eventTabPtr->FormatString);
                if (num_parm != 0)
                {
                    for (i = 0; (i < num_parm) && !exitFlag; ++i, ++params)
                    {
                        parm = *params;
                        switch (parmListPtr->ItemType)
                        {
                            /*------------------------------------------------*/
                            case TY_P_NUMBER:
                                ADD_STRING(parmListPtr->FormatString, parm);
                                break;

                            /*------------------------------------------------*/
                            case TY_P_ADDRESS:
                                if (    (FBLOCK_INIC     == *(params+1))
                                     || (FBLOCK_NETBLOCK == *(params+1)) )
                                {
                                    ADD_STRING("%s", "Local.");
                                }
                                else
                                {
                                    ADD_STRING(parmListPtr->FormatString, parm);
                                }
                                break;

                            /*------------------------------------------------*/
                            case TY_P_FUNC:
                                if (NULL != (pParmStr = GetName(serviceTabPtr->FuncList, parm)))
                                {
                                    ADD_STRING(parmListPtr->FormatString, pParmStr);
                                }
                                else
                                {
                                    ADD_STRING("[unknown function 0x%04X].", parm);
                                }
                                break;

                            /*------------------------------------------------*/
                            case TY_P_EVENT:
                                ADD_STRING(parmListPtr->FormatString, parm);
                                break;

                            /*------------------------------------------------*/
                            case TY_P_FBLOCK:
                                if (NULL != (pFblock = GetFBlock(FBlockList, parm)))
                                {
                                    ADD_STRING(parmListPtr->FormatString, pFblock->name);
                                }
                                else
                                {
                                    ADD_STRING("[unknown FBlock 0x%02X].", parm);
                                }
                                break;

                            /*------------------------------------------------*/
                            case TY_P_FKTID:
                                if (NULL != pFblock)
                                {
                                    if (NULL != (pFkt = GetFktName(pFblock->fktList, parm)))
                                    {
                                        ADD_STRING(parmListPtr->FormatString, pFkt->name);
                                    }
                                    else
                                    {
                                        ADD_STRING("[unknown FktID 0x%03X].", parm);
                                    }
                                }
                                else
                                {
                                    ADD_STRING("[FktID 0x%03X].", parm);
                                }
                                break;

                            /*------------------------------------------------*/
                            case TY_P_OPTYPE:
                                if (NULL != pFkt)
                                {
                                    if (NULL != (pParmStr = GetName(OpTypeList, parm+(pFkt->type))))
                                {
                                    ADD_STRING(parmListPtr->FormatString, pParmStr);
                                }
                                else
                                {
                                    ADD_STRING("[unknown OpType 0x%01X].", parm);
                                }
                                }
                                else
                                {
                                    ADD_STRING("[OpType 0x%01X].", parm);
                                }
                                break;

                            /*------------------------------------------------*/
                            case TY_P_STATEMACHINE:
                                if (NULL != (pStateM = GetStateM(StateMList, parm)))
                                {
                                    ADD_STRING(parmListPtr->FormatString, pStateM->name);
                                }
                                else
                                {
                                    ADD_STRING("[unknown state machine 0x%02X].", parm);
                                }
                                break;

                            /*------------------------------------------------*/
                            case TY_P_CHIP:
                                if (NULL != (pParmStr = GetName(MNS_ChipList, parm)))
                                {
                                    ADD_STRING(parmListPtr->FormatString, pParmStr);
                                }
                                else
                                {
                                    ADD_STRING("[unknown chip 0x%04X].", parm);
                                }
                                break;

                            /*------------------------------------------------*/
                            case TY_P_TRANSCAUSE:
                                if (NULL != (pParmStr = GetName(MNS_TC_List, parm)))
                                {
                                    ADD_STRING(parmListPtr->FormatString, pParmStr);
                                }
                                else
                                {
                                    ADD_STRING("[unknown cause 0x%01X].", parm);
                                }
                                break;

                            /*------------------------------------------------*/
                            case TY_P_TM_OP:
                                if (NULL != (pParmStr = GetName(MNS_TM_OP, parm)))
                                {
                                    ADD_STRING(parmListPtr->FormatString, pParmStr);
                                }
                                else
                                {
                                    ADD_STRING("[unknown operation 0x%04X].", parm);
                                }
                                break;

                            /*------------------------------------------------*/
                            case TY_P_TM_EV:
                                if (NULL != (pParmStr = GetName(MNS_TM_EV, parm)))
                                {
                                    ADD_STRING(parmListPtr->FormatString, pParmStr);
                                }
                                else
                                {
                                    ADD_STRING("[unknown event 0x%04X].", parm);
                                }
                                break;

                                /*------------------------------------------------*/
                                case TY_P_TM_FLAGS:

                                switch(parm)
                                {
                                    case 0:
                                        ADD_STR("-");
                                        break;

                                    case 1:
                                        ADD_STR("Busy");
                                        break;

                                    case 2:
                                        ADD_STR("Schedule");
                                        break;

                                    case 3:
                                        ADD_STR("Busy & Schedule");
                                        break;

                                    default:
                                        ADD_STRING("0x%04X", parm);
                                        break;
                                }
                                break;

                            /*------------------------------------------------*/
                            case TY_P_STEP:



                                if ((parm > 0x00) && (parm < 0xA0))
                                {
                                    ADD_STRING(".Step.%d", parm);
                                }
                                else if ((parm > 0xA0) && (parm < 0xB0))
                                {
                                    ADD_STRING(".Alpha.%d", parm & 0xF);
                                }
                                else if ((parm > 0xB0) && (parm < 0xC0))
                                {
                                    ADD_STRING(".Beta.%d", parm & 0xF);
                                }
                                else if (parm == 0xD0)
                                {
                                    ADD_STR("Distrib");
                                }
                                else if ((parm > 0xD0) && (parm < 0xE0))
                                {
                                    ADD_STRING(".RC.%d", parm & 0xF);
                                }
                                else if (parm == 0xE0)
                                {
                                    ADD_STR("SR");
                                }
                                else if ((parm > 0xE0) && (parm < 0xF0))
                                {
                                    ADD_STRING(".SR.RC.%d", parm & 0xF);
                                }
                                else
                                {
                                    ADD_STRING(".[unknown code = 0x%02X]", parm);
                                }
                                ADD_STR("    ");
                                break;

                            /*------------------------------------------------*/
                            case TY_P_STATE:
                                if ((NULL != pStateM) && (NULL != pStateM->sList))
                                {
                                    if (NULL != (pState = GetState(pStateM->sList, parm)))
                                    {
                                        ADD_STRING(parmListPtr->FormatString, pState->name);
                                    }
                                    else
                                    {
                                        ADD_STRING("[unknown state 0x%02X].", parm);
                                    }
                                }
                                else
                                {
                                    ADD_STRING("[state 0x%02X].", parm);
                                }
                                break;

                            /*------------------------------------------------*/
                            case TY_P_SERVICE:
                                if (NULL != (pParmStr = GetName(MNS_ServiceList, parm)))
                                {
                                    ADD_STRING(parmListPtr->FormatString, pParmStr);
                                }
                                else
                                {
                                    ADD_STRING("[unknown service 0x%04X].", parm);
                                }
                                break;

                                /*------------------------------------------------*/
                            case TY_P_SRVMASK:
                                {
                                    int tmp = 0;
                                    int pos = 0;
                                    bool first = MNS_TRUE;

                                    while(parm)
                                    {
                                        tmp = (1 << pos) & parm;
                                        if(tmp)
                                        {
                                            parm &= ~tmp;
                                            if (!first)
                                            {
                                                ADD_STR(", ");
                                            }
                                            else
                                            {
                                                first = MNS_FALSE;
                                            }
                                            if (NULL != (pParmStr = GetName(MNS_ServiceList, tmp)))
                                            {
                                                ADD_STRING(parmListPtr->FormatString, pParmStr);
                                            }
                                            else
                                            {
                                                ADD_STRING("0x%04X", tmp);
                                            }
                                        }
                                        pos++;
                                    }
                                }
                                break;

                                /*------------------------------------------------*/
                            case TY_P_CFGMASK:
                                {
                                    int tmp = 0;
                                    int pos = 0;

                                    while(parm)
                                    {
                                        tmp = (1 << pos) & parm;
                                        if(tmp)
                                        {
                                            parm &= ~tmp;
                                            ADD_STR(", ");
                                            if (NULL != (pParmStr = GetName(MNS_ConfigList, tmp)))
                                            {
                                                ADD_STRING(parmListPtr->FormatString, pParmStr);
                                            }
                                            else
                                            {
                                                ADD_STRING("0x%04X", tmp);
                                            }
                                        }
                                        pos++;
                                    }
                                }
                                break;

                                /*------------------------------------------------*/
                            case TY_P_L2_FLAG:
                                if (NULL != (pParmStr = GetName(MNS_L2_FlagList, parm)))
                                {
                                    ADD_STRING(parmListPtr->FormatString, pParmStr);
                                }
                                else
                                {
                                    ADD_STRING("[unknown flag 0x%04X].", parm);
                                }
                                break;

                            /*------------------------------------------------*/
                            case TY_P_MHP_FLAG:
                                if (NULL != (pParmStr = GetName(MNS_MHP_FlagList, parm)))
                                {
                                    ADD_STRING(parmListPtr->FormatString, pParmStr);
                                }
                                else
                                {
                                    ADD_STRING("[unknown flag 0x%04X].", parm);
                                }
                                break;

                            /*------------------------------------------------*/
                            case TY_P_PMHS_FLAG:
                                if (NULL != (pParmStr = GetName(MNS_PMHS_FlagList, parm)))
                                {
                                    ADD_STRING(parmListPtr->FormatString, pParmStr);
                                }
                                else
                                {
                                    ADD_STRING("[unknown flag 0x%04X].", parm);
                                }
                                break;

                            /*------------------------------------------------*/
                            case TY_P_ASSERT:
                                ADD_ERROR(parmListPtr->FormatString, parm);
                                break;

                            /*------------------------------------------------*/
                            case PAR_TERMINATION:
                                exitFlag = MNS_TRUE;                       /* finish loop */
                                break;

                            /*------------------------------------------------*/
                            default:
                                ADD_STRING("TYPE_MISMATCH %d", parm);
                                break;
                        }
                        parmListPtr++;
                    }
                }
            }
        }
    }

    if (printError)
    {
        PrintErrorLine(lineout);
    }
    else if (printLine)
    {
        PrintTraceLine(lineout);
    }


}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : GetName()                                                  */
/* Description : searches a string in a numbered list                       */
/*                                                                          */
/* Parameter(s): list    pointer to a numbered string list                  */
/*               item    item whose description is searched                 */
/*                                                                          */
/* Returns     : pointer to the belonging string                            */
/*               or NULL pointer if not successful                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/
char* GetName(pTNumString list, int item)
{
    char        *ret;
    char        found;
    int         myNum;

    ret     = NULL;
    found   = MNS_FALSE;

    do
    {
        myNum = list->num;                          /* read item number from list */
        if (myNum == item)                          /* item found ?               */
        {
            found = MNS_TRUE;
            ret   = (char *)list->str;              /* return pointer to the string */
        }
        list++;
    }
    while (!found && (STR_TERMINATION != myNum));   /* found or end of list ? */

    return (ret);
}




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : GetFktName()                                                  */
/* Description : searches a string in a numbered list                       */
/*                                                                          */
/* Parameter(s): list    pointer to a numbered string list                  */
/*               item    item whose description is searched                 */
/*                                                                          */
/* Returns     : pointer to the belonging string                            */
/*               or NULL pointer if not successful                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/
pTFktList GetFktName(pTFktList list, int item)
{
    pTFktList   ret;
    char        found;
    int         myNum;

    ret     = NULL;
    found   = MNS_FALSE;

    do
    {
        myNum = list->num;                          /* read item number from list */
        if (myNum == item)                          /* item found ?               */
        {
            found = MNS_TRUE;
            ret   = list;                           /* return pointer to entry */
        }
        list++;
    }
    while (!found && (STR_TERMINATION != myNum));   /* found or end of list ? */

    return (ret);
}


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : GetStateM()                                                */
/* Description : searches a string in a numbered list                       */
/*                                                                          */
/* Parameter(s): list    pointer to a numbered string list                  */
/*               item    item whose description is searched                 */
/*                                                                          */
/* Returns     : pointer to the belonging string                            */
/*               or NULL pointer if not successful                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/
pTStateMList GetStateM(pTStateMList list, int item)
{
    pTStateMList ret;
    char         found;
    int          myID;

    ret     = NULL;
    found   = MNS_FALSE;

    do
    {
        myID = list->id;                          /* read item number from list */
        if (myID == item)                          /* item found ?               */
        {
            found = MNS_TRUE;
            ret   = list;                           /* return pointer to entry */
        }
        list++;
    }
    while (!found && (STR_TERMINATION != myID));   /* found or end of list ? */

    return (ret);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : GetState()                                                 */
/* Description : searches a string in a numbered list                       */
/*                                                                          */
/* Parameter(s): list    pointer to a numbered string list                  */
/*               item    item whose description is searched                 */
/*                                                                          */
/* Returns     : pointer to the belonging string                            */
/*               or NULL pointer if not successful                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/
pTStateList GetState(pTStateList list, int item)
{
    pTStateList ret;
    char        found;
    int         myID;

    ret     = NULL;
    found   = MNS_FALSE;

    do
    {
        myID = list->id;                           /* read item number from list */
        if (myID == item)                          /* item found ?               */
        {
            found = MNS_TRUE;
            ret   = list;                           /* return pointer to entry */
        }
        list++;
    }
    while (!found && (STR_TERMINATION != myID));   /* found or end of list ? */

    return (ret);
}


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : GetFBlock()                                                */
/* Description : searches a FBlock description in  string in a numbered list*/
/*                                                                          */
/* Parameter(s): list    pointer to a numbered FBlock list                  */
/*               fblock  fblock whose description is searched               */
/*                                                                          */
/* Returns     : pointer to the found entry                                 */
/*               or NULL pointer if not successful                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/
pTFBlockList GetFBlock(pTFBlockList list, int fblock)
{
    pTFBlockList  ret;
    char          found;
    int           num;

    ret = NULL;
    found = MNS_FALSE;

    do
    {
        num = list->fbNum;                          /* read fblock number from list */
        if (num == fblock)                          /* item found ?                 */
        {
            found = MNS_TRUE;
            ret   = list;                           /* return pointer to the entry */
        }
        list++;
    }
    while (!found && STR_TERMINATION != num);       /* found or end of list ? */

    return (ret);
}




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : SetTraceFilter()                                           */
/* Description : sets the filter variable of a certain service              */
/*                                                                          */
/* Parameter(s): service    service whose filter is set                     */
/*               filter     new filter value                                */
/*                                                                          */
/* Returns     : MNS_TRUE  if operation was succesful                           */
/*               MNS_FALSE if not successful (service not found)                */
/*                                                                          */
/*--------------------------------------------------------------------------*/
byte SetTraceFilter(int service, TFilter filter)
{
    pTServices serviceTabPtr;

    if (0xFFFF != service)                                      /* SpecialFilter */
    {
        serviceTabPtr = T_Services;                             /* go to start of services list */

        while (SRV_TERMINATION != serviceTabPtr->Service)       /* scan the services list */
        {
            if (serviceTabPtr->Service == service)              /* service found ? */
            {
                *(serviceTabPtr->ServiceFilter) = filter;       /* write filter mask */
                return(MNS_TRUE);
            }
            else
            {
                serviceTabPtr++;                                /* go to next list entry */
            }
        }
        return(MNS_FALSE);                                          /* service not found */
    }
    else
    {
        SpecialFilter = filter;
        return(MNS_TRUE);
    }
}


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : GetTraceFilter()                                           */
/* Description : reads the filter variable of a certain service             */
/*                                                                          */
/* Parameter(s): service    service whose filter is read                    */
/*               filter     pointer to filter variable                      */
/*                                                                          */
/* Returns     : MNS_TRUE  if operation was succesful                           */
/*               MNS_FALSE if not successful (service not found)                */
/*                                                                          */
/*--------------------------------------------------------------------------*/
byte GetTraceFilter(int service, TFilter * filter)
{
    pTServices serviceTabPtr;

    if (0xFFFF != service)                                      /* SpecialFilter ? */
    {
        serviceTabPtr = T_Services;                             /* go to the start of services list */

        while (SRV_TERMINATION != serviceTabPtr->Service)       /* scan the services list */
        {
            if (serviceTabPtr->Service == service)              /* respective service found ? */
            {
                *filter = *(serviceTabPtr->ServiceFilter);      /* read filter mask */
                return(MNS_TRUE);
            }
            else
            {
                serviceTabPtr++;                                /* go to next list entry */
            }
        }
        return(MNS_FALSE);                                          /* service not found */
    }
    else
    {
        *filter = SpecialFilter;
        return(MNS_TRUE);
    }
}


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : EnableFilterEvent()                                        */
/* Description : enables one filter event of a certain service              */
/*                                                                          */
/* Parameter(s): service    service whose filter is changed                 */
/*               event      event which is enabled                          */
/*                                                                          */
/* Returns     : MNS_TRUE  if operation was succesful                           */
/*               MNS_FALSE if not successful (service not found)                */
/*                                                                          */
/*--------------------------------------------------------------------------*/
byte EnableFilterEvent(int service, int event)
{
    pTServices serviceTabPtr;

    if (0xFFFF != service)
    {
        serviceTabPtr = T_Services;                             /* start of services list */

        while (SRV_TERMINATION != serviceTabPtr->Service)       /* scan the services list */
        {
            if (serviceTabPtr->Service == service)              /* service found ? */
            {
                *(serviceTabPtr->ServiceFilter) |= (0x00000001 << (event-1));   /* write filter mask */
                return(MNS_TRUE);
            }
            else
            {
                serviceTabPtr++;
            }
        }
        return(MNS_FALSE);
    }
    else
    {
        SpecialFilter |= (0x00000001 << (event-1));
        return(MNS_TRUE);
    }
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : DisableFilterEvent()                                       */
/* Description : disables one filter event of a certain service             */
/*                                                                          */
/* Parameter(s): service    service whose filter is changed                 */
/*               event      event which is disabled                         */
/*                                                                          */
/* Returns     : MNS_TRUE  if operation was succesful                           */
/*               MNS_FALSE if not successful (service not found)                */
/*                                                                          */
/*--------------------------------------------------------------------------*/
byte DisableFilterEvent(int service, int event)
{
    pTServices serviceTabPtr;

    if (0xFFFF != service)
    {
        serviceTabPtr = T_Services;                           /* start of services list */

        while (SRV_TERMINATION != serviceTabPtr->Service)     /* scan the services list */
        {
            if (serviceTabPtr->Service == service)            /* service found ? */
            {
                *(serviceTabPtr->ServiceFilter) &= ~(0x00000001 << (event-1));  /* write filter mask */
                return(MNS_TRUE);
            }
            else
            {
                serviceTabPtr++;
            }
        }
        return(MNS_FALSE);
    }
    else
    {
        SpecialFilter &= ~(0x00000001 << (event-1));
        return(MNS_TRUE);
    }
}


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : MnsTraceSpecialFilter()                                    */
/* Description : allows filtering befor parsing                             */
/*                                                                          */
/* Parameter(s): service   module where the message is released             */
/*               event     event which causes the message                   */
/*               num_parm  number of following parameters                   */
/*               params    pointer to parameter list of variable length     */
/*                                                                          */
/* Returns     : MNS_TRUE  if further operation shall be done                   */
/*               MNS_FALSE if no message should be printed                      */
/*                                                                          */
/*--------------------------------------------------------------------------*/
byte MnsTraceSpecialFilter(int service, int event, int num_parm, int *params)
{
    (void) service;
    (void) event;
    if (!(SpecialFilter & FILT_INIC_WATCHDOG)
        && (6 == num_parm)
        && (FBLOCK_INIC          == params[1])
        && (FUNCID_INIC_WATCHDOG == params[3]))
    {
        return(MNS_FALSE);
    }

    if (  !(SpecialFilter & FILT_INIC_TIMER)
        && (6 == num_parm)
        && (FBLOCK_INIC       == params[1])
        && (FUNCID_INIC_TIMER == params[3]))
    {
        return(MNS_FALSE);
    }

    /* add additional filters here */

    return (MNS_TRUE);
}

