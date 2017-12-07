/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Address Search Handler
File:           ah.c
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



/*
------------------------------------------------------------------------------
        Include Files
------------------------------------------------------------------------------
*/


#include "mostns.h"

#include "nbehc.h"
#include "ah.h"
#include "mns2.h"

#ifdef NETWORKMASTER_SHADOW
#include "netwm_s.h"
#endif


#ifdef NS_INC_AH
#include NS_INC_AH
#endif


#ifdef AH_MIN
    #ifndef NS_AMS_AH
    #error NS_AMS_AH must be defined in adjust1.h (Layer I), if module AH (Layer II) is used !
    #endif
#endif





/*
------------------------------------------------------------------------------
        Local Definitions
------------------------------------------------------------------------------
*/


#ifdef ADDRH_TIMEOUT
    #define AH_TIME_ANSWER     ((word) ADDRH_TIMEOUT)                 /* timeout value (each try) */
#else
    #define AH_TIME_ANSWER     ((word) 200)                           /* timeout value (each try), Default: 200ms */
#endif



/*
------------------------------------------------------------------------------
        Macro Definitions
------------------------------------------------------------------------------
*/





/*
------------------------------------------------------------------------------
        Type Declaration
------------------------------------------------------------------------------
*/


#ifdef AH_MIN
typedef struct AddrHBuf_T                       /* task buffer of address handler */
{
    pTMsgTx Tx_Ptr;
}TAddrHBuf, *pTAddrHBuf;
#endif

#ifdef AH_MIN
typedef struct AddrHSearch_T                    /* struct containing current search parameters */
{
    byte State;                                 /* state of the state machine */
    byte Pos;                                   /* position to ask for FBlockIDs  */
    byte FBlock_ID;                             /* FBlock_ID to search */
    byte Inst_ID;                               /* Inst_ID to search */
    word Most_Addr;                             /* result, if search was succesfull */
}TAddrHSearch, *pTAddrHSearch;
#endif





/*
------------------------------------------------------------------------------
        Global variables / imported variables
------------------------------------------------------------------------------
*/




/*
------------------------------------------------------------------------------
        Local variables and buffers
------------------------------------------------------------------------------
*/


#if (ADDRH_SIZE_DEVICE_TAB > 0)
TDevTab DevTab[ADDRH_SIZE_DEVICE_TAB];          /* decentral device registry */
pTDevTab DevTabPtrWrite;                        /* write pointer  */
pTDevTab DevTabPtrRead;                         /* read pointer */
#endif



#ifdef AH_MIN
TAddrHBuf AddrHBuf[AH_SIZE_ADDR_HNDL_BUF];      /* task buffer of address handler */
pTAddrHBuf AddrHPtrIn;                          /* pointer for ingoing tasks */
pTAddrHBuf AddrHPtrOut;                         /* pointer for outgoing tasks */
byte AddrHNumTasks;
bool AddrHDevTabInitF;                          /* flag if decentral device table already initialized */
#endif



#ifdef AH_MIN
TAddrHSearch AddrHSearch;                       /* struct containing current search parameters  */
#endif


#if (defined AH_MIN) && (ADDRH_RETRY > 0)
byte AddrHRetry;                                /* Retry Counter */
#endif

#ifdef AH_MIN
TMostTimerHL TimerAddrH;                       /* Timer of Address Handler  */
#endif


/*
------------------------------------------------------------------------------
        Local Function Prototypes
------------------------------------------------------------------------------
*/

#ifdef AH_2
void AddrHPush(pTMsgTx ptrtxmsg);               /* Push a ptr on a tx msg into the task buffer of address search handler */
#endif

#ifdef AH_4
void AddrHComplete(byte success);                                       /* Finish the current address searching */
#endif                                                                  /* process (bad or ok) */

#ifdef AH_6
bool AddrHIncPos(void);                                     /* Increment node position to ask for its FBlockIDs and check if last position */
#endif




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : AddrHInit()                                                */
/* Description : Init the Address Handler                                   */
/* Parameter(s): none                                                       */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef AH_0
void AddrHInit(void)
{
    byte i;

    AddrHDevTabInitF = MNS_FALSE;                       /* Decentrale device registry not yet initialized */

    /* Init the decentral device registry */
    /*------------------------------------------ */
    #ifndef MSV2_MIN                                /* this task will we done by Supervisor Layer II if implemented      */
    #ifdef AH_01
    AddrHDevTabInit(MNS_FALSE);                         /* restore the previous decentral registry                           */
    #endif
    #endif
    /*------------------------------------------ */


    /* Init buffer of address handler  */
    /*------------------------------------------ */
    AddrHPtrIn = AddrHBuf;
    for (i = (byte)0; i < AH_SIZE_ADDR_HNDL_BUF; i++)
    {
        AddrHPtrIn->Tx_Ptr = NULL;                   /* Clear task buffer */
        AddrHPtrIn++;
    }
    AddrHPtrIn    = AddrHBuf;
    AddrHPtrOut   = AddrHBuf;
    AddrHNumTasks = (byte)0;
    /*------------------------------------------ */


    /* Init the state machine  */
    /*------------------------------------------ */
    AddrHSearch.State = (byte)0;

    #ifdef MNS2_21
    MostRegisterTimer(&TimerAddrH,(TMostEventCB*)MnsRequestLayer2Timer, (word)0);
    MostClearTimer(&TimerAddrH);
    #endif
    /*------------------------------------------ */
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : AddrHDevTabInit()                                          */
/* Description : Init the decentral registry                                */
/* Parameter(s): clear flag                                                 */
/*                  MNS_TRUE:  Decentral registry will be cleared               */
/*                  MNS_FALSE: Decentral registry will be restored              */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef AH_01
void AddrHDevTabInit(bool ClearDevTab)
{
    byte i;

    #ifndef ADDRH_DEVICE_TAB_NONVOLATILE            /* if decentral registry is not stored  */
    (void)ClearDevTab;                              /* in non-volatile memory area          */
    #endif

    /* Init the decentral device registry */
    /*------------------------------------------ */

    #ifdef ADDRH_DEVICE_TAB_NONVOLATILE

    if (MNS_FALSE == ClearDevTab)
        AddrHRestoreDevTab(DevTab);             /* Restore decentral registry, if stored in non-volatile  */
    else                                        /* memory area */
    {
        DevTabPtrWrite = DevTab;
        for (i = (byte)0; i < (byte)ADDRH_SIZE_DEVICE_TAB; i++)   /* Clear stored decentral registry */
        {
            DevTabPtrWrite->FBlock_ID = (byte)0;
            DevTabPtrWrite++;
        }
    }
    #else
    DevTabPtrWrite = DevTab;
    for (i = (byte)0; i < (byte)ADDRH_SIZE_DEVICE_TAB; i++)       /* Init decentral registry, if not stored in non-volatile */
    {                                           /* memory area */
        DevTabPtrWrite->FBlock_ID = (byte)0;
        DevTabPtrWrite++;
    }
    #endif

    DevTabPtrWrite = DevTab;
    DevTabPtrRead  = DevTab;
    /*------------------------------------------ */

    AddrHDevTabInitF = MNS_TRUE;                        /* Flag: Decentrale device registry has been initialized */
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : AddrHSearchStart()                                         */
/* Description : Search for MOST Address in decentral registry.             */
/*               If the corresponding address is not found in the table,    */
/*               the next searching proccess will be started                */
/* Parameter(s): ptr on tx message                                          */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef AH_1
void AddrHSearchStart(pTMsgTx ptbuf)
{
    #ifdef NM_1
    word NWM_Address;

    NWM_Address = (word)0x0000;
    #endif

    if (MostGetState() != MNS_NET_ON)   /* AH is not initialized until NetOn-Event ( within callback Go_Net_On() ) */
    {                                   /* Therefore it is not possible to search the address, if not already in NetOn. */
        MsgSend2(ptbuf);                /* --> Send message to target 0xFFFF */
        return;
    }

    if ( (FBLOCK_ALL == ptbuf->FBlock_ID) || (INST_ALL == ptbuf->Inst_ID) )     /* broadcast on application level ? */
    {
        #if (defined MHP_T8) && (defined NS_AH_MHP)                             /* interface to MOST High enabled ?      */
        MhpAddrHComplete((word)0xFFFF, ptbuf->FBlock_ID, ptbuf->Inst_ID);       /* Broadcast not possible on MHP */
        #endif
        ptbuf->Tgt_Adr = MSG_TGT_BROADCAST_BLOCKING;        /* use broadcast addressing, if target address unknown AND  */

        #ifdef AMS_TX_ADD9
        ptbuf->MidLevelRetries = (byte)0;                   /* set number of mid level retries to 0 for broadcast message */
        #endif

        MsgSend2(ptbuf);                                    /* broadcast on application level is required (FBlockID OR InstID equal 0xFF) */
        return;
    }


    switch (ptbuf->FBlock_ID)                               /* check for any well-known FBlock ID */
    {
        case FBLOCK_NETBLOCK:                                           /* FBlock NetBlock, ET, or INIC ? */
        case FBLOCK_INIC:                                               /* -> use node position addressing, */
        case FBLOCK_ET:                                                 /* since their InstID is derived from the node position */
            ptbuf->Tgt_Adr = (word)0x0400 + (word)ptbuf->Inst_ID;       /* use node position addressing          */
            break;

        case FBLOCK_NETWORKMASTER:
            #ifdef NM_1
            NWM_Address = NmGetNWMAddr();
            if (NWM_Address != (word)0x0000)       /* Address already known ? */
            {
                ptbuf->Tgt_Adr = NWM_Address;
            }
            #endif
            break;

        default:
            break;
    }

    if (ptbuf->Tgt_Adr != (word)0xFFFF)   /* Could the address be resolved, due to a well-known FBlockID ? */
    {
        #if (defined MHP_T8) && (defined NS_AH_MHP)                         /* interface to MOST High enabled ?      */
        MhpAddrHComplete(ptbuf->Tgt_Adr, ptbuf->FBlock_ID, ptbuf->Inst_ID);   /* report desired address to MHP module */
        #endif
        MsgSend2(ptbuf);
        return;
    }

    #ifdef AH_8
    ptbuf->Tgt_Adr = AddrHDevTabGet(ptbuf->FBlock_ID, ptbuf->Inst_ID);
    if (ptbuf->Tgt_Adr != (word)0xFFFF )
    {
        #if (defined MHP_T8) && (defined NS_AH_MHP)                         /* interface to MOST High enabled ? */
        MhpAddrHComplete(ptbuf->Tgt_Adr, ptbuf->FBlock_ID, ptbuf->Inst_ID);   /* report desired address to MHP module */
        #endif
        MsgSend2(ptbuf);                                    /* Send message after completing the target address */
    }
    else
    {
        AddrHPush(ptbuf);                                   /* Push ptr on tx msg into task buffer of address handler, */
    }                                                       /* since address not found in decentral registry */

    #else
    AddrHPush(ptbuf);                                       /* Push ptr on tx msg into task buffer of address handler, */
    #endif                                                  /* since no decentral registry available */
}
#endif




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : AddrHPush()                                                */
/* Description : Push ptr on a tx msg into task buffer of address handler   */
/* Parameter(s): ptr on tx message                                          */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef AH_2
void AddrHPush(pTMsgTx ptrtxmsg)
{
    if (AddrHNumTasks < AH_SIZE_ADDR_HNDL_BUF)
    {
        AddrHNumTasks++;
        AddrHPtrIn->Tx_Ptr = ptrtxmsg;
        if (AddrHPtrIn == &AddrHBuf[AH_SIZE_ADDR_HNDL_BUF-1])
        {
            AddrHPtrIn = AddrHBuf;
        }
        else
        {
            AddrHPtrIn++;
        }
    }
    else
    {
        AddrHSearchFailed(ptrtxmsg);
        MsgTxUnused(ptrtxmsg);
    }

    MNS2_REQUEST_CALL(MNS2_P_AH_STATE)                      /* Set Request Flag and call application */
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : AddrHService()                                             */
/* Description : Looks for a new task in its buffer                         */
/*               and generates search message                               */
/* Parameter(s): none                                                       */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef AH_3
void AddrHService(void)
{
    pTMsgTx MyPtr;

    switch (AddrHSearch.State)
    {
        case 0x00:

            if (AddrHPtrOut->Tx_Ptr != NULL )                                /* if ready to start a new search  */
            {
                #ifdef AH_8
                /* Look again into the decentral device registry, since the result could be available meanwhile: */
                AddrHPtrOut->Tx_Ptr->Tgt_Adr = AddrHDevTabGet(AddrHPtrOut->Tx_Ptr->FBlock_ID, AddrHPtrOut->Tx_Ptr->Inst_ID);
                if (AddrHPtrOut->Tx_Ptr->Tgt_Adr != (word)0xFFFF )
                {
                    AddrHComplete((byte)2);
                    break;
                }
                #endif

                AddrHSearch.FBlock_ID = AddrHPtrOut->Tx_Ptr->FBlock_ID;     /* set search parameters */
                AddrHSearch.Inst_ID   = AddrHPtrOut->Tx_Ptr->Inst_ID;
                AddrHSearch.Pos       = (byte)0x00;

                #if (ADDRH_RETRY > 0)
                AddrHRetry = (byte)ADDRH_RETRY;
                #endif

                #ifdef CR_AVAILABLE
                #ifdef NETWORKMASTER_LOCAL
                AddrHSearch.State = (byte)0x01;                             /* search using central registry */
                #else
                if (NmGetNWMAddr() != (word)0x0000)                         /* NetworkMaster available ? */
                {
                    AddrHSearch.State = (byte)0x01;                         /* search using central registry */
                }
                else
                {
                    #ifdef ADDRH_SEARCH_FBLOCKIDS
                    AddrHSearch.State = (byte)0x03;                         /* search without central registry */
                    #else
                    AddrHComplete((byte)0);                                 /* Search Failed */
                    #endif
                }
                #endif
                #else
                #ifdef ADDRH_SEARCH_FBLOCKIDS
                AddrHSearch.State = (byte)0x03;                                   /* search without central registry */
                #else
                AddrHComplete((byte)0);                                           /* Search Failed */
                #endif
                #endif

                MNS2_REQUEST_SET(MNS2_P_AH_STATE)                           /* Set Request Flag */
            }
            break;

    /*------------------------------------------ */
    #ifdef CR_AVAILABLE

        case 0x01:
            if ( (MyPtr = MsgGetTxPtrRes()) != NULL )                       /* get reserved bufferline */
            {
                #ifdef NETWORKMASTER_LOCAL
                MyPtr->Tgt_Adr          = MOST_TGT_INTERN;           /* determined by module NETWM_S.C */
                #else
                MyPtr->Tgt_Adr          = NmGetNWMAddr();           /* determined by module NETWM_S.C */
                #endif
                MyPtr->FBlock_ID        = FBLOCK_NETWORKMASTER;
                MyPtr->Inst_ID          = (byte)0;
                MyPtr->Func_ID          = FUNC_CENTRALREGISTRY;
                MyPtr->Operation        = OP_GET;
                MyPtr->Data[0]          = AddrHSearch.FBlock_ID;
                MyPtr->Data[1]          = AddrHSearch.Inst_ID;
                MyPtr->Length           = (word)2;
                #ifdef AMS_TX_ADD9
                MyPtr->MidLevelRetries  = (byte)0;                              /* no mid level retries for AH messages */
                #endif
                AmsMsgSend(MyPtr);
                AddrHSearch.State = (byte)0x02;
                MostSetTimer(&TimerAddrH, AH_TIME_ANSWER, MNS_FALSE);
            }

            MNS2_REQUEST_SET(MNS2_P_AH_STATE)                                   /* Set Request Flag */
            break;

        case 0x02:
            if ((word)0 == MostGetTimer(&TimerAddrH))                           /* Check if timeout occurred                                      */
            {
                #if (ADDRH_RETRY > 0)
                if ((byte)0 == AddrHRetry)
                {
                    AddrHRetry = (byte)ADDRH_RETRY;

                    #if (ADDRH_SEARCH_METHOD == 3)                              /* Automatic Search Method ? */
                    AddrHSearch.State = (byte)0x03;                             /* -> third search process */
                    #else
                    AddrHComplete((byte)0);                                     /* Search Failed */
                    break;
                    #endif
                }
                else
                {
                    AddrHRetry--;
                    AddrHSearch.State = (byte)0x01;
                }

                MNS2_REQUEST_SET(MNS2_P_AH_STATE)                       /* Set Request Flag */

                #else
                    #if (ADDRH_SEARCH_METHOD == 3)                      /* Automatic Search Method ? */
                    AddrHSearch.State = (byte)0x03;                           /* -> third search process */
                    MNS2_REQUEST_SET(MNS2_P_AH_STATE)                   /* Set Request Flag */
                    #else
                    AddrHComplete((byte)0);                                   /* Search Failed */
                    #endif
                #endif
            }
            break;
    #endif

    /*------------------------------------------ */
    #ifdef ADDRH_SEARCH_FBLOCKIDS

        case 0x03:

            #ifndef AMS_TX_ADD6                                 /* only necessary, if it is not possible to send internal messages: */
            if(MOST_GET_NODE_POS() == AddrHSearch.Pos)          /* own position prevention */
            {
                if ( MNS_FALSE == AddrHIncPos() )               /* increment node position and check if last node reached */
                        return;                                 /* -> Search failed, since last node reached */
            }
            #endif

            if ( (MyPtr = MsgGetTxPtrRes()) != NULL )                           /* get reserved bufferline */
            {
                MyPtr->Tgt_Adr          = (word)( 0x400 + AddrHSearch.Pos );    /* when Pos == own posistion: msg is routed to RX queue in AMS */
                MyPtr->FBlock_ID        = FBLOCK_NETBLOCK;
                MyPtr->Inst_ID          = (byte)0;
                MyPtr->Func_ID          = FUNC_FBLOCKIDS;
                MyPtr->Operation        = OP_GET;
                MyPtr->Length           = (word)0;
                #ifdef AMS_TX_ADD9
                MyPtr->MidLevelRetries  = (byte)0;                              /* no mid level retries for AH messages */
                #endif
                AmsMsgSend(MyPtr);
                AddrHSearch.State       = (byte)0x04;
                MostSetTimer(&TimerAddrH, AH_TIME_ANSWER, MNS_FALSE);
            }

            MNS2_REQUEST_SET(MNS2_P_AH_STATE)                                   /* Set Request Flag */
            break;


        case 0x04:
            if (!(MostGetTimer(&TimerAddrH)))                                   /* Check if timeout occurred */
            {
                #if (ADDRH_RETRY > 0)
                if ((byte)0 == AddrHRetry)
                {
                    if ( MNS_FALSE == AddrHIncPos() )
                        return;                                                 /* -> Search failed, since last position reached */

                    else
                        AddrHRetry = (byte)ADDRH_RETRY;                 /* Init retry counter for next device */
                }
                else
                {
                    AddrHRetry--;
                }
                #else
                    if ( MNS_FALSE == AddrHIncPos() )
                        return;                                         /* -> Search failed, since last position reached */
                #endif

                AddrHSearch.State = (byte)0x03;
                MNS2_REQUEST_SET(MNS2_P_AH_STATE)                       /* Set Request Flag */
            }
            break;
    #endif

    }
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : AddrHComplete()                                            */
/* Description : Finish the address searching procedure                     */
/*               The current result stands in the struct AddrHSearch        */
/* Parameter(s):                                                            */
/*               0: address not found                                      */
/*               1: search successful, address will be entered in decentral */
/*                  device registry                                         */
/*               2: search successful, but do not enter it into decentral   */
/*                  device registry                                         */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef AH_4
void AddrHComplete(byte success)
{
    /*----------------------------- */
    /*    Search was succesful */
    /*----------------------------- */
    if (success)
    {
        if ((byte)1 == success)
        {
            AddrHPtrOut->Tx_Ptr->Tgt_Adr = AddrHSearch.Most_Addr;               /* Copy correct address into tx msg */

            #ifdef AH_9
            AddrHDevTabSet(AddrHSearch.Most_Addr, AddrHSearch.FBlock_ID, AddrHSearch.Inst_ID);/* Copy new address into  */
            #endif                                                                          /* decentral device registry */

            AddrHResult(AddrHSearch.Most_Addr, AddrHSearch.FBlock_ID, AddrHSearch.Inst_ID);   /* report result to application */
        }

        #if (defined MHP_T8) && (defined NS_AH_MHP)                         /* interface to MOST High enabled ? */
        MhpAddrHComplete(AddrHPtrOut->Tx_Ptr->Tgt_Adr, AddrHPtrOut->Tx_Ptr->FBlock_ID, AddrHPtrOut->Tx_Ptr->Inst_ID);
        #endif        /* report result to MHP module, but do not use parameters in  AddrHSearch-buffer, since it could be even case "success==0x02" */


        MsgSend2(AddrHPtrOut->Tx_Ptr);                                      /* Msg in Tx Buffer is now ready to send */
    }
    else
    /*----------------------------- */
    /*    Search failed */
    /*----------------------------- */
    {
        AddrHSearchFailed(AddrHPtrOut->Tx_Ptr);

        #if (defined MHP_T8) && (defined NS_AH_MHP)                         /* interface to MOST High enabled ? */
        MhpAddrHComplete((word)0xFFFF, AddrHSearch.FBlock_ID, AddrHSearch.Inst_ID); /* report failure to MHP module */
        #endif

        MsgTxUnused(AddrHPtrOut->Tx_Ptr);
    }

    /*----------------------------- */
    /*    success or failed */
    /*----------------------------- */
    AddrHPtrOut->Tx_Ptr = NULL;                                              /* free current job */

    if (AddrHPtrOut == &AddrHBuf[AH_SIZE_ADDR_HNDL_BUF-1])                     /* inc pointer (task buffer) */
    {
        AddrHPtrOut = AddrHBuf;                                             /* modulo buffer size */
    }
    else
    {
        AddrHPtrOut++;
    }

    AddrHNumTasks--;
    AddrHSearch.State = (byte)0;                                            /* Init the state machine */

    MNS2_REQUEST_CALL(MNS2_P_AH_STATE)                                      /* Set Request Flag and call application */
}
#endif





/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : AddrHCheckFBlockIDs()                                      */
/* Description : Check received status message of NetBlock.FBlockIDs        */
/* Parameter(s): ptr on rx msg                                              */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef AH_5
void AddrHCheckFBlockIDs(pTMsgRx Rx_Ptr)
{
    bool success;
    word counter;
    byte *Data_Ptr;

    if (AddrHSearch.State != (byte)0x04)                          /* return whenever not waiting for a status report */
        return;

    success = MNS_FALSE;
    counter = (word) ( (Rx_Ptr->Length) >> 1 );
    Data_Ptr = &Rx_Ptr->Data[0];

    while ((MNS_FALSE == success) && counter)
    {
        if (*Data_Ptr++ == AddrHSearch.FBlock_ID)           /* Check FBlock ID */
        {
            if (    (*Data_Ptr == AddrHSearch.Inst_ID)      /* Check Inst ID */
                 || (INST_ANY  == AddrHSearch.Inst_ID))     /* match also if searched InstID is 0x00 (don't care) */
                                                            /* NWM has to support requests for InstID 0x00 */
            {
                AddrHSearch.Most_Addr = Rx_Ptr->Src_Adr;    /* Get MOST address */
                success = MNS_TRUE;
            }
        }

        Data_Ptr++;
        counter--;

    }

    if (MNS_FALSE != success)
    {
        AddrHComplete((byte)1);                   /* Address found  */
    }
    else
    {
        if ( MNS_FALSE != AddrHIncPos() )         /* increment node position and try again if last position reached not yet */
        {
            #if (ADDRH_RETRY > 0)
            AddrHRetry = (byte)ADDRH_RETRY;       /* Init retry counter for next device */
            #endif
            AddrHSearch.State = (byte)0x03;

            MNS2_REQUEST_CALL(MNS2_P_AH_STATE)              /* Set Request Flag and call application */
        }
    }
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : AddrHIncPos()                                              */
/* Description : Increment node position to ask for its FBlockIDs           */
/*               This function checks if last position has reached          */
/*                                                                          */
/* Parameter(s): none                                                       */
/* Returns     : MNS_FALSE: last position reached                           */
/*               MNS_FALSE: last position not reached                       */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef AH_6
bool AddrHIncPos(void)
{
    byte mpr_reg;

    mpr_reg = MostGetMaxPos();

    AddrHSearch.Pos++;
    if (AddrHSearch.Pos >= mpr_reg)         /* last position reached ? */
    {
        AddrHComplete((byte)0);                   /* Search failed, */
        return(MNS_FALSE);                  /* since last position reached ! */
    }

    return(MNS_TRUE);                       /* -> continue searching process at next node position */
}
#endif




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : AddrHCheckCentralRegStatus()                               */
/* Description : Check received status message of NetworkMaster.CentralReg  */
/*               and write the demanded address into the struct AddrHSearch */
/* Parameter(s): ptr on rx msg                                              */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef AH_7
void AddrHCheckCentralRegStatus(pTMsgRx Rx_Ptr)
{
  #ifdef NETWORKMASTER_LOCAL
    if (AddrHSearch.State != (byte)0x02)                  /* return whenever not waiting for a status report */
        return;

    if ( (Rx_Ptr->Data[2] == AddrHSearch.FBlock_ID) &&
        ((Rx_Ptr->Data[3] == AddrHSearch.Inst_ID) || (INST_ANY  == AddrHSearch.Inst_ID)))   /* match also if searched InstID is 0x00 (don't care) */
                                                                                            /* NWM has to support requests for InstID 0x00       */
    {
        CmdDecodeWord(&AddrHSearch.Most_Addr,&Rx_Ptr->Data[0]);            /* Get correct Most Address */
        AddrHComplete((byte)1);
    }
  #else
    word i;

    byte fblock_id;
    byte inst_id;
    word device_id;

    for (i=(word)0; i < ((Rx_Ptr->Length) >> 2); ++i)               /* write all DeviceIDs to the decentral registry */
    {
        fblock_id = Rx_Ptr->Data[(i<<2) + 2];
        inst_id   = Rx_Ptr->Data[(i<<2) + 3];
        CmdDecodeWord(&device_id, &Rx_Ptr->Data[i<<2]);             /* Get correct Most Address */

        if (    ((byte)0x02 == AddrHSearch.State)                   /* do we search for it? */
             && (fblock_id  == AddrHSearch.FBlock_ID)
             && ((inst_id   == AddrHSearch.Inst_ID) || (INST_ANY  == AddrHSearch.Inst_ID)))     /* match also if searched InstID is 0x00 (don't care) */
                                                                                                /* NWM has to support requests for InstID 0x00       */
        {
            AddrHSearch.Most_Addr = device_id;                      /* Get correct Most Address */
            AddrHComplete((byte)1);
        }
        else
        {
            #ifdef AH_9
            AddrHDevTabSet(device_id, fblock_id, inst_id);
            #endif
        }
    }
  #endif
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : AddrHCheckCentralRegError()                                */
/* Description : Check received error message of NetworkMaster.CentralReg   */
/* Parameter(s): ptr on rx msg                                              */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef AH_10
void AddrHCheckCentralRegError(pTMsgRx Rx_Ptr)
{
    byte abort;             /* 0x01: search failed */
                            /* 0x00: error message do not comply */


    if ((byte)0x02 == AddrHSearch.State)                  /* are we waiting for a status report from Central Registry ? */
    {
        if (ERR_PARAM_NOTAVAILABLE == Rx_Ptr->Data[0])
        {
            abort = (byte)0;

            switch (Rx_Ptr->Data[1])
            {
                /*------------------- */
                /* FBlock ID failed                 */ /* NetworkMaster.00.CentralRegistry.Error(0x07.0x01.wrongFBlockID)  */
                /*------------------- */
                case 0x01:
                    if (Rx_Ptr->Data[2] == AddrHSearch.FBlock_ID)
                    {
                        abort = (byte)1;
                    }
                    break;

                /*------------------- */
                /* Inst ID failed                   */ /* NetworkMaster.00.CentralRegistry.Error(0x07.0x02.wrongInstID) */
                /*------------------- */
                case 0x02:
                    if (Rx_Ptr->Data[2] == AddrHSearch.Inst_ID)
                    {
                        abort = (byte)1;
                    }
                    break;

            }

            if (abort)
            {
                AddrHComplete((byte)0);                   /* Search Failed ! */
            }
        }
    }
}
#endif






/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : AddrHDevTabGet()                                           */
/* Description : Get Device Address from Device Table (Decentral Registry)  */
/*                                                                          */
/* Parameter(s): FBlockID, InstID                                           */
/* Returns     : MostAddress (0xFFFF:= Search failed)                       */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef AH_8
word AddrHDevTabGet(byte FBlockID, byte InstID)
{
    byte Counter;
    byte Inst_Tab;
    byte FBlock_Tab;

    if (MNS_FALSE != AddrHDevTabInitF)   /* Decentral registry already initialized ? */
    {
        Counter = (byte)ADDRH_SIZE_DEVICE_TAB;

        do
        {
            FBlock_Tab = DevTabPtrRead->FBlock_ID;              /* read FBlock_ID from table         */

            if (FBlock_Tab == FBlockID)
            {
                Inst_Tab = DevTabPtrRead->Inst_ID;

                if (Inst_Tab == InstID)
                {
                    return(DevTabPtrRead->Most_Addr);           /* -->  search succesful */
                }
            }

            Counter--;

            if (DevTabPtrRead == &DevTab[ADDRH_SIZE_DEVICE_TAB-1])    /* inc pointer modulo table size */
            {
                DevTabPtrRead = DevTab;
            }
            else
            {
                DevTabPtrRead++;
            }

        }while (Counter);                                       /* repeat until whole table was searched or address was found */
    }

    return ((word)0xFFFF);                                  /* -->  search failed */
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : AddrHDevTabSet()                                           */
/* Description : Set Device Address in Device Table (Decentral Registry)    */
/*                                                                          */
/* Parameter(s): MostAddress, FBlockID, InstID                              */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef AH_9
void AddrHDevTabSet(word MostAddr, byte FBlockID, byte InstID)
{
    bool found;
    byte i;


    if (MNS_FALSE != AddrHDevTabInitF)                                           /* Decentral registry already initialized ? */
    {
        found = MNS_FALSE;

        for (i = (byte)0; (i < (byte)ADDRH_SIZE_DEVICE_TAB) && (MNS_FALSE == found); ++i)
        {
            if (    (DevTab[i].FBlock_ID == FBlockID)               /* check for existing entries */
                 && (DevTab[i].Inst_ID   == InstID  ))
            {
                DevTab[i].Most_Addr = MostAddr;                     /* overwrite old value */
                found = MNS_TRUE;
            }
        }

        if (MNS_FALSE == found)
        {
            DevTabPtrWrite->FBlock_ID   = FBlockID;
            DevTabPtrWrite->Inst_ID     = InstID;
            DevTabPtrWrite->Most_Addr   = MostAddr;

            if (DevTabPtrWrite == &DevTab[ADDRH_SIZE_DEVICE_TAB-1])   /* inc pointer (decentral registry) */
            {
                DevTabPtrWrite = DevTab;                              /* modulo buffer size               */
            }
            else
            {
                DevTabPtrWrite++;
            }
        }
    }
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : AddrHStoreDevTabAH()                                       */
/* Description : Check if decentral device registry has been already        */
/*               initialized and call callback function to store the        */
/*               decentral device registry.                                 */
/*                                                                          */
/* Parameter(s): none                                                       */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef AH_11
void AddrHStoreDevTabAH(void)
{
    #ifdef AH_CB3
    if (MNS_FALSE == AddrHDevTabInitF)      /* Decentral registry not yet initialized up to now ?  */
    {
        AddrHDevTabInit(MNS_TRUE);  /* clear the decentral device registry before it will be stored, since the table */
                                /* has not been initialized up to now ! */
    }

    AddrHStoreDevTab(DevTab);                                   /* Store decentral registry in non-volatile menory area                  */
    #endif
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : AddrHDevTabDel()                                           */
/* Description : Deletes the given FBlockID/InstIDs entries                 */
/*               in the decentral device registry.                          */
/*                                                                          */
/* Parameter(s): pointer to the received message                            */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef AH_12
void AddrHDevTabDel(pTMsgRx Rx_Ptr)
{
    word i;

    for (i=(word)0; i < (word)((Rx_Ptr->Length - (word)1)>>1); ++i)
    {
        (void)AddrHDevTabDelEntry(Rx_Ptr->Data[(i<<1) + 1], Rx_Ptr->Data[(i<<1) + 2]);
    }
}
#endif

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : AddrHDevTabDelEntry()                                      */
/* Description : Deletes an entry in the decentral device registry and      */
/*               adjusts the write pointer.                                 */
/*                                                                          */
/* Parameter(s): FBlockID, InstID of the entry to delete                    */
/* Returns     : MNS_TRUE: decentral device registry already initialized        */
/*               MNS_FALSE: decentral device registry not yet initialized       */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef AH_13
bool AddrHDevTabDelEntry(byte FBlockID, byte InstID)
{
    byte     Counter;
    byte     Inst_Tab;
    byte     FBlock_Tab;
    pTDevTab ReadPtr, MovePtr;
    bool     found;

    if (MNS_FALSE != AddrHDevTabInitF)               /* Decentral registry not yet initialized up to now ? */
    {
        found   = MNS_FALSE;
        Counter = (byte)ADDRH_SIZE_DEVICE_TAB;
        ReadPtr = DevTab;

        do
        {
            FBlock_Tab = ReadPtr->FBlock_ID;                /* read FBlock_ID from table */

            if (FBlock_Tab == FBlockID)
            {
                Inst_Tab = ReadPtr->Inst_ID;

                if (Inst_Tab == InstID)
                {
                    found = MNS_TRUE;           /* -->  search succesful */
                }
            }

            Counter--;

            if (MNS_FALSE == found)
            {
                ReadPtr++;
            }

        }while (Counter && (MNS_FALSE == found));                                     /* repeat until whole table was searched or address was found */

        if (MNS_FALSE != found)
        {
            while (ReadPtr != DevTabPtrWrite)
            {
                if (ReadPtr == &DevTab[ADDRH_SIZE_DEVICE_TAB-1])  /* inc pointer modulo table size */
                {
                    MovePtr = DevTab;
                }
                else
                {
                    MovePtr = ReadPtr + 1;
                }

                ReadPtr->FBlock_ID = MovePtr->FBlock_ID;
                ReadPtr->Inst_ID   = MovePtr->Inst_ID;
                ReadPtr->Most_Addr = MovePtr->Most_Addr;

                if (ReadPtr == &DevTab[ADDRH_SIZE_DEVICE_TAB-1])  /* inc pointer modulo table size */
                {
                    ReadPtr = DevTab;
                }
                else
                {
                    ReadPtr++;
                }
            }

            if ((byte)0 == DevTabPtrWrite->FBlock_ID)
            {
                if (DevTabPtrWrite == DevTab)                   /* dec write pointer modulo table size */
                {
                    DevTabPtrWrite = &DevTab[ADDRH_SIZE_DEVICE_TAB-1];
                }
                else
                {
                    DevTabPtrWrite--;
                }
            }
            else
            {
                DevTabPtrWrite->FBlock_ID = (byte)0;                  /* clear entry */
            }

            return(MNS_TRUE);
        }
        else
        {
            return(MNS_FALSE);
        }
    }
    else
    {
        return(MNS_FALSE);
    }
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : AddrHClearTasks()                                          */
/* Description : Stops all pending tasks of the Address Handler, clears the */
/*               task list and sets the state machine to the idle state.    */
/*                                                                          */
/* Parameter(s): none                                                       */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef AH_14
void AddrHClearTasks(void)
{
    byte i;

    /* Init buffer of address handler */
    /*------------------------------------------ */
    AddrHPtrIn = AddrHBuf;
    for (i = (byte)0; i < AH_SIZE_ADDR_HNDL_BUF; i++)
    {
        if (AddrHPtrIn->Tx_Ptr)
        {
            AddrHSearchFailed(AddrHPtrIn->Tx_Ptr);          /* stop active tasks */
            MsgTxUnused(AddrHPtrIn->Tx_Ptr);
        }
        AddrHPtrIn->Tx_Ptr = NULL;                          /* Clear task buffer */
        AddrHPtrIn++;
    }
    AddrHPtrIn    = AddrHBuf;
    AddrHPtrOut   = AddrHBuf;
    AddrHNumTasks = (byte)0;
    /*------------------------------------------ */


    /* Init the state machine */
    /*------------------------------------------ */
    AddrHSearch.State = (byte)0;

    #ifdef MNS2_21
    MostClearTimer(&TimerAddrH);
    #endif
}
#endif
