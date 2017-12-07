/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         MOST Supervisor Layer II
File:           msv2.c
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


#include "ah.h"

#include "cmd.h"
#include "mns2.h"
#include "msv2.h"

#include "nbehc.h"

#ifdef NS_INC_MSV2
#include NS_INC_MSV2
#endif




/*
------------------------------------------------------------------------------
        Local Definitions
------------------------------------------------------------------------------
*/

#ifndef TIME_CONFIG_STATUS
    #define TIME_CONFIG_STATUS ((word)15000)                  /* timeoutvalue in [ms] (only NetworkSlave) */
#endif

#define MSV2_JUSTBORN   ((word)0xFFFF)                          /* NodeAddress if Device is just born */

#define MASK_INIT_STATES    ((byte)0x03)                        /* Mask for states 0x01, 0x02, and 0x03 */



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



/*
------------------------------------------------------------------------------
        Global variables / imported variables
------------------------------------------------------------------------------
*/

#if defined(MSV2_0) && defined(NETWORKMASTER_LOCAL)
byte ConfigState;
#endif

#ifdef NETWORKMASTER_SHADOW
extern word NetworkMaster_Addr;
#endif




/*
------------------------------------------------------------------------------
        Local variables and buffers
------------------------------------------------------------------------------
*/

#ifdef MSV2_0
byte ConfigStatusOk;
#endif

#ifdef ENABLE_CFG_MSG_FILTER
bool MsgConfigStateFilterOn;
#endif


/*
------------------------------------------------------------------------------
        Local Function Prototypes (not available for the outside)
------------------------------------------------------------------------------
*/

#ifdef MSV2_4
void SendConfigStatus(byte status, pTMsgTx tx_ptr);
#endif


#ifdef MSV2_9
static void SetNWMAddressShadow(word address);
#endif


#ifdef MSV2_12
bool GetConfigStateFilterStatus(void);
#endif





/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : MostSupervisor2Init()                                      */
/* Description : Initializes the MOST Supervisor Layer II                   */
/* Parameter(s): None                                                       */
/* Returns     : Nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef MSV2_0
void MostSupervisor2Init(void)
{
    TVmsvInternalHooks hooks;

    #ifdef MSV2_9
    hooks.update_nwm_address_fptr = SetNWMAddressShadow;
    #else
    hooks.update_nwm_address_fptr = NULL;
    #endif
    hooks.store_error_info2_fptr = Store_Error_Info2;
    VmsvRegisterInternalHooks(&hooks);

    #ifdef NETWORKMASTER_LOCAL
    ConfigState = (byte)0x01;                                     /* Starting the State Machine controlling the */
                                                            /* the Configuration Process */
    ConfigStatusOk = NWM_CFG_STATUS_NOTOK;                  /* Init Flag */
    #else
    ConfigStatusOk = MostGetNCState();
    #endif


    #if (defined MSV2_MIN) && (!defined  NETWORKMASTER_LOCAL)  /* clear the decentral registry */
    #ifdef AH_01
    AddrHDevTabInit(MNS_TRUE);
    #endif

    /*  Start System Communication Init */
    /*--------------------------------- */
    if (NWM_CFG_STATUS_OK != ConfigStatusOk)
    {
        SystemCommunicationInit(ConfigStatusOk);                /* Callback Application: */
                                                                /* Start SystemCommunicationInit Process (Init Notification,...) */
    }
    #endif

    #ifdef MSV2_4
    ConfigCheckStateMaster();                               /* Service first state and set node address */
    #endif
    /*------------------------ */

    #ifdef NETWORKMASTER_LOCAL
    MNS2_REQUEST_CALL(MNS2_P_MSV2_STATE)                    /* Set Request Flag and call application */
    #endif


    #ifdef ENABLE_CFG_MSG_FILTER
    MsgConfigStateFilterOn = MNS_TRUE;
    #endif

}
#endif






/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*  NetworkSlave  -  Section */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : ConfigStatusChanged()                                      */
/* Description : Get new Configuration.Status Event and syncronize          */
/*               the State Machine                                          */
/*               (NetworkSlaveDevice only)                                  */
/* Parameter(s): New value Configuration.Status                             */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef MSV2_3
void ConfigStatusChanged(pTMsgRx Rx_Ptr)
{
    ConfigStatusOk = Rx_Ptr->Data[0];                       /* 0: NotOk, 1: Ok, 2: Invalid, 3: New */

    if (NWM_CFG_STATUS_NOTOK == ConfigStatusOk)
    {
        #ifdef AH_14
        AddrHClearTasks();                                  /* stop Address Handler    */
        #endif

        #ifdef ENABLE_CFG_MSG_FILTER
        SetConfigStateFilterStatus(MNS_ON);
        #endif
    }
    else
    {
        #ifdef ENABLE_CFG_MSG_FILTER
        SetConfigStateFilterStatus(MNS_OFF);
        #endif
    }

    #ifdef MSV2_CB5
    if (    (NWM_CFG_STATUS_INVALID == ConfigStatusOk )
         || (NWM_CFG_STATUS_NEW     == ConfigStatusOk )
         || (NWM_CFG_STATUS_NEWEXT  == ConfigStatusOk ) )
    {
        FBlockIDsChanged(Rx_Ptr);
    }
    #endif

    #ifdef AH_12
    if (NWM_CFG_STATUS_INVALID == ConfigStatusOk)        /* delete invalid entries in decentral registry */
    {
        AddrHDevTabDel(Rx_Ptr);
    }
    #endif

    if (NWM_CFG_STATUS_NOTOK == ConfigStatusOk)         /* NetworkMaster.Configuration.Status.(NotOk) */
    {
        if ((byte)0 == NbCheckPosValid(NULL))                 /* node adress can be changed only if node position is valid */
        {
            /*  Clear DeviceTable */
            /*-------------------- */
            #ifdef AH_01
            AddrHDevTabInit(MNS_TRUE);                  /* clear the decentral registry */
            #endif


            /*  Initialize Notification Service */
            /*---------------------------------- */
            #ifdef NTF_0                                /* due to a "NWM.Configuration.NotOk" message           */
            NtfInit();
            #endif
        }
    }

    /*  Start System Communication Init */
    /*--------------------------------- */
    SystemCommunicationInit(ConfigStatusOk);            /* Callback Application: */
                                                        /* Start SystemCommunicationInit Process (Init Notification,...) */
}
#endif






/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*  NetworkMaster  -  Section */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : ConfigCheckStateMaster()                                   */
/* Description : Checks State Transission of the State Machine controlling  */
/*               the Configuration Process in a NetworkMaster Device        */
/* Parameter(s): none                                                       */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef MSV2_4
void ConfigCheckStateMaster(void)
{
    pTMsgTx TxPtr;


    switch (ConfigState)
    {
        /*-------------------------- */
        case 0x01:
        /*-------------------------- */

            ConfigState = (byte)0x02;                               /* Default: Wait-State */
                                                                    /* This command is ignored, if device is not just born, */
                                                                    /* of if the application calls CentralRegistryTriggerJustBorn() */
                                                                    /* within the function NbRestoreNodeAddr() ! */


            /*  Check NodeAddress */
            /*-------------------- */
            if (MSV2_JUSTBORN != MostGetNodeAdr() )                 /* Valid NodeAddress ? */
            {
                ConfigState = (byte)0x10;                           /* ->   Exit from Wait-State */
            }
            else                                                    /* Invalid NodeAddress ? */
            {                                                       /* ->   First Address Initialisation (Just Born) */

                if ((byte)0 == NbCheckPosValid(NULL))               /* node adress can be changed only if node position is valid */
                {
                    (void)NbSetNodeAddr( (word)(MOST_GET_NODE_POS() + (word)0x100) );
                                                                    /* Set new NodeAddress depending on */
                                                                    /* the NodePosition */
                                                                    /* NodeAddress = 0x0100 + Pos */
                                                                    /* And store current NodeAddress in non-volatile memory area */

                /*  ConfigState = 0x02;                             */ /* Enter Wait-State, and wait until application will trigger */
                /*                                                  */ /* the broadcast message NetworkMaster.Configuration.Status.NotOk */

                /*  This command appears in front of the address check, */
                /*  since the state is overwritten by calling CentralRegistryTriggerJustBorn(). */
                /*  Therefore this function can be called within NbRestoreNodeAddr() too, */
                /*  if wait-state is not required. */

                }
                else
                {
                    ConfigState = (byte)0x01;
                }
            }

            MNS2_REQUEST_SET(MNS2_P_MSV2_STATE)                     /* Set Request Flag */
            break;


        /*-------------------------- */
        /*  case 0x02:                                              */ /* just wait until application triggers */
        /*      break;                                              */ /* broadcast of NetworkMaster.Configuration.Status.NotOk */
        /*-------------------------- */


        /*-------------------------- */
        case 0x03:
        /*-------------------------- */

            TxPtr = MsgGetTxPtr();

            if (TxPtr != NULL)
            {
                /*  Send Configuration.Status.NotOk */
                /*---------------------------------- */
                SendConfigStatus(NWM_CFG_STATUS_NOTOK, TxPtr);

                #ifdef ENABLE_CFG_MSG_FILTER
                SetConfigStateFilterStatus(MNS_ON);
                #endif

                /*  Clear Central Registry */
                /*---------------------------------- */
                CentralRegistryClear();

                /*  Initialize Notification Service */
                /*---------------------------------- */
                #ifdef NTF_0                                        /* due to a "NWM.Configuration.NotOk" message */
                NtfInit();
                #endif

                ConfigState = (byte)0x10;
            }

            MNS2_REQUEST_SET(MNS2_P_MSV2_STATE)                     /* Set Request Flag */
            break;


        /*-------------------------- */
        case 0x10:
        /*-------------------------- */

            /*  Check System Configuration */
            /*--------------------------------- */
            CentralRegistryCheckStart();                            /* Callback Application */
                                                                    /* -> Start System Configuration Check */
            ConfigState = (byte)0x20;

            MNS2_REQUEST_SET(MNS2_P_MSV2_STATE)                     /* Set Request Flag */
            break;



        /*-------------------------- */
/*      case 0x20:                                                  */ /* just wait until System Configuration */
/*          break;                                                  */ /* Check has been complete */
        /*-------------------------- */


        /*-------------------------- */
        case 0x30:
        /*-------------------------- */

            if (NWM_CFG_STATUS_NOTOK != ConfigStatusOk)
            {
                if (NWM_CFG_STATUS_OK == ConfigStatusOk)
                {
                    TxPtr = MsgGetTxPtr();

                    if (TxPtr != NULL)
                    {
                        /*  Send Configuration.Status.Ok */
                        /*--------------------------------- */
                        SendConfigStatus(ConfigStatusOk, TxPtr);

                        #ifdef ENABLE_CFG_MSG_FILTER
                        SetConfigStateFilterStatus(MNS_OFF);
                        #endif

                        /*  Start System Communication Init */
                        /*--------------------------------- */
                        SystemCommunicationInit(ConfigStatusOk);    /* Callback Application: */
                                                                    /* Start SystemCommunicationInit Process (Init Notification,...) */
                        ConfigState = (byte)0x00;
                        break;
                    }
                }
                else
                {
                    #ifdef ENABLE_CFG_MSG_FILTER
                    SetConfigStateFilterStatus(MNS_OFF);
                    #endif

                    /*  Start System Communication Init */
                    /*--------------------------------- */
                    SystemCommunicationInit(ConfigStatusOk);        /* Callback Application: */
                                                                    /* Start SystemCommunicationInit Process (Init Notification,...) */
                    ConfigState = (byte)0x00;
                    break;
                }
            }
            else
            {
                if ((byte)0 == NbCheckPosValid(NULL))                     /* node adress can be changed only if node position is valid */
                {
                    word node_addr;

                    node_addr = MostGetNodeAdr();
                    if ( ( MSV2_JUSTBORN == node_addr ) ||
                         ( (0x0100 <= node_addr)  &&
                           (0x013F >= node_addr) ) )
                    {
                        NbSetGetNodeAddr( (word)(MOST_GET_NODE_POS() + 0x100) );

                                                                    /* Set new NodeAddress depending on */
                                                                    /* the NodePosition */
                                                                    /* NodeAddress = 0x0100 + Pos */
                                                                    /* And store current NodeAddress in non-volatile memory area */
                        ConfigState = (byte)0x31;
                    }
                    else
                    {
                        ConfigState = (byte)0x03;
                    }

                }
            }

            MNS2_REQUEST_SET(MNS2_P_MSV2_STATE)                     /* Set Request Flag */
            break;

        /*-------------------------- */
        case 0x31:                                                  /* just wait until NodeAddr. updates */
            break;
        /*-------------------------- */

        default:
            break;
    }
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : SendConfigStatus()                                         */
/* Description : Sends following broadcast message: Configuration.Status    */
/* Parameter(s): Status, Ptr on Tx Message Buffer                           */
/*                  Status:   0x00:  NotOk                                  */
/*                            0x01:  Ok                                     */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef MSV2_4
void SendConfigStatus(byte status, pTMsgTx tx_ptr)
{
    tx_ptr->Tgt_Adr         = MSG_TGT_BROADCAST_BLOCKING;
    tx_ptr->FBlock_ID       = FBLOCK_NETWORKMASTER;
    tx_ptr->Inst_ID         = NETWORKMASTER_INSTID;
    tx_ptr->Func_ID         = FUNC_CONFIGURATION;
    tx_ptr->Operation       = OP_STATUS;
    tx_ptr->Length          = (word)1;
    tx_ptr->Data[0]         = status;
    #ifdef AMS_TX_ADD9
    tx_ptr->MidLevelRetries = (byte)0;                            /* no mid level retries for NWM messages */
    #endif
    AmsMsgSend(tx_ptr);
}
#endif




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CentralRegistryCheckComplete()                             */
/* Description : Termination of System Configuration Check                  */
/*               (NetworkMaster only)                                       */
/* Parameter(s): Result of the System Configuration Check                   */
/*                  0: Configuration NotOk                                  */
/*                  1: Configuration Ok                                     */
/*                  2: Invalid (FBlocks disappeared)                        */
/*                  4: New FBlocks                                          */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef MSV2_5
void CentralRegistryCheckComplete(byte NewStatus)
{
    ConfigStatusOk  = NewStatus;                /* 0: NotOk, 1: Ok, 2: Invalid, 4: NewExt */
    ConfigState     = (byte)0x30;

    MNS2_REQUEST_CALL(MNS2_P_MSV2_STATE)        /* Set Request Flag and call application */
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CentralRegistryMprChanged()                                */
/* Description : Set new state of state machine, whenever MPR register      */
/*               changes                                                    */
/*               (NetworkMaster only)                                       */
/*                                                                          */
/* Parameter(s): new value of MPR register, last value of MPR register      */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef MSV2_6
void CentralRegistryMprChanged(byte mpr_new, byte mpr_old)
{
    (void) mpr_new;
    (void) mpr_old;

  /*if (mpr_new > 1)                                                */
    {
        if ( !(ConfigState & MASK_INIT_STATES) )                    /* if (   (ConfigState != 0x01) */
        {                                                           /*     && (ConfigState != 0x02) */
                                                                    /*     && (ConfigState != 0x03) ) */

                                                                    /* just to make sure, that initialisation process */
                                                                    /* has been completed and it is not interrupted by */
                                                                    /* MPR-Change-Event */
            ConfigState = (byte)0x10;

            MNS2_REQUEST_CALL(MNS2_P_MSV2_STATE)                    /* Set Request Flag and call application */
        }
        else
        if ((byte)0x02 == ConfigState)
        {                                                           /* Force trigger of broadcast message, if state machine is */
            CentralRegistryTriggerJustBorn();                       /* in wait-state, after starting up without any slave node */
        }                                                           /* (Mpr == 1). */
    }
}
#endif




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CentralRegistryTriggerJustBorn()                           */
/* Description : Triggers transmission of broadcast message:                */
/*               NetworkMaster.Configuration.Status.NotOk                   */
/*               (NetworkMaster only)                                       */
/* Parameter(s): none                                                       */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef MSV2_7
void CentralRegistryTriggerJustBorn(void)
{
    ConfigState    = (byte)0x03;
    MNS2_REQUEST_CALL(MNS2_P_MSV2_STATE)        /* Set Request Flag and call application */
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CheckConfigState()                                         */
/* Description : checks if there is a pending node address change request   */
/*                                                                          */
/* Parameter(s): none                                                       */
/* Returns     : MNS_TRUE   node address is valid                               */
/*               MNS_FALSE  node address is  temporarily invalid                */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef MSV2_8
bool CheckConfigState(void)
{
    /*  Network Slave: */
    /* ---------------------- */
    #if (defined MSV2_MIN) && (!defined  NETWORKMASTER_LOCAL)
    return(MNS_TRUE);
    #endif

    /*  Network Master: */
    /* ---------------------- */
    #ifdef  MSV2_4
    if (((byte)0x01 == ConfigState) || ((byte)0x30 == ConfigState))      /* node address valid ? */
    {
        return(MNS_FALSE);
    }
    else
    {
        return(MNS_TRUE);
    }
    #endif
}
#endif

#ifdef MSV2_9
static void SetNWMAddressShadow(word address)
{
    NetworkMaster_Addr = address;
}
#endif


#ifdef MSV2_10
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NodeAddrChangeComplete()                                   */
/* Description : Set new state of state machine, when Node Address changes  */
/*               (NetworkMaster only)                                       */
/* Parameter(s): none                                                       */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NodeAddrChangeComplete(void)
{
    if ((byte)0x31 == ConfigState)
    {
        ConfigState    = (byte)0x03;
        MNS2_REQUEST_CALL(MNS2_P_MSV2_STATE)        /* Set Request Flag and call application */
    }
}
#endif


#ifdef MSV2_11
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : SetConfigStateFilterStatus()                               */
/* Description : sets the status of the ConfigState Based Message Filter    */
/*                                                                          */
/* Parameter(s): on  new status                                             */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void SetConfigStateFilterStatus(bool on)
{
    if (MNS_OFF != on)
    {
        MsgConfigStateFilterOn = MNS_TRUE;

        PmsFlushMsgTxBuffer();

        #ifdef NS_MSV2_MHP
        #if (defined MHP_TX_MIN) || (defined PMHT_MIN)
        MhpSetConfigStateFilterStatus(MNS_ON);
        #endif
        #endif
    }
    else
    {
        MsgConfigStateFilterOn = MNS_FALSE;
        #ifdef NS_MSV2_MHP
        #if (defined MHP_TX_MIN) || (defined PMHT_MIN)
        MhpSetConfigStateFilterStatus(MNS_OFF);
        #endif
        #endif
    }
}
#endif

#ifdef MSV2_12
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : GetConfigStateFilterStatus()                               */
/* Description : reads the status of the ConfigState Based Message Filter   */
/*                                                                          */
/* Parameter(s): none                                                       */
/* Returns     : MNS_FALSE filter is not active                                 */
/*               MNS_TRUE  filter is active                                     */
/*                                                                          */
/*--------------------------------------------------------------------------*/
bool GetConfigStateFilterStatus(void)
{
    return(MsgConfigStateFilterOn);
}
#endif


#ifdef MSV2_13
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : ConfigStateFilterV2()                                      */
/* Description : filters messages depending on the Configuration Status     */
/*                                                                          */
/* Parameter(s): ptbuf pointer to the message buffer                        */
/* Returns     : MSG_TX_FILTER_DEFAULT message can be sent                  */
/*               MSG_TX_FILTER_CANCEL  message must not be sent             */
/*                                                                          */
/*--------------------------------------------------------------------------*/
byte ConfigStateFilterV2(TMsgTx *msg_ptr)
{
    byte result;
    byte fblock;

    result = MSG_TX_FILTER_CANCEL;


    if (MNS_FALSE != GetConfigStateFilterStatus())
    {
        fblock = msg_ptr->FBlock_ID;
        if (   (FBLOCK_NETBLOCK      == fblock)
            || (FBLOCK_INIC          == fblock)
            || (FBLOCK_NETWORKMASTER == fblock)
            || (FBLOCK_ET            == fblock)
            || (FBLOCK_DEBUG         == fblock) )
        {
            result = MSG_TX_FILTER_DEFAULT;
        }
        #ifdef CFG_STATE_FILTER_EXCEPTION_LIST
        else
        {
            byte i;         /* list will not exceed byte range */
            byte list[] = {CFG_STATE_FILTER_EXCEPTION_LIST};

            for (i=(byte)0; i<(byte)sizeof(list); ++i)
            {
                if (list[i] == fblock)
                {
                    result = MSG_TX_FILTER_DEFAULT;
                }
            }
        }
        #endif
    }
    else
    {
        result = MSG_TX_FILTER_DEFAULT;
    }

    return(result);
}
#endif

