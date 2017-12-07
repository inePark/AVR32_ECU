/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         FBlock NetBlock (EHC part)
File:           nbehc.c
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


#include "cmd.h"
#include "nbehc.h"
#include "msv2.h"
#include "ah.h"
#include "ntfs.h"
#include "mns2.h"

#ifdef NS_INC_NB
#include NS_INC_NB
#endif



/*
------------------------------------------------------------------------------
        Local Definitions
------------------------------------------------------------------------------
*/


/*
------------------------------------------------------------------------------
        Macro Definitions
------------------------------------------------------------------------------
*/


#define NB_SHUTDOWN_QUERY       ((byte) 0x00)
#define NB_SHUTDOWN_SUSPEND     ((byte) 0x01)
#define NB_SHUTDOWN_EXECUTE     ((byte) 0x02)
#define NB_SHUTDOWN_TEMP        ((byte) 0x03)
#define NB_SHUTDOWN_DEVICE      ((byte) 0x04)
#define NB_SHUTDOWN_DEV_WAKEUP  ((byte) 0x05)



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
#if NUM_FBLOCKS_SHADOW > 0
extern byte InstIDShadow[];
#endif



/*
------------------------------------------------------------------------------
        Local variables and buffers
------------------------------------------------------------------------------
*/


#ifdef NB_MIN
TNetBlock   NetBlock;                   /* Structure containing all properties and methods of NetBlock */
bool        NodePositionValid;            /* node position valid ? */
#endif

#ifdef NB_MIN
bool    BoundaryGet_Pending;
pTMsgTx BoundaryPtr;
#endif

/*
------------------------------------------------------------------------------
        Local Function Prototypes
------------------------------------------------------------------------------
*/



/*
------------------------------------------------------------------------------
        Tables
------------------------------------------------------------------------------
*/

#ifdef NB_MIN
#include "t_nbehc.tab"                     /* Table containing all FUNC_IDS and all OP_TYPES of FBlock: NetBlock */
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbInit()                                                   */
/* Description : Init the NetBlock                                          */
/* Parameter(s): none                                                       */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_0
void NbInit(void)
{
    #if (NUM_FBLOCKS>0)
    byte i;
    byte fblock_index;          /* index in NetBlock.pFBlockIDs */
    #endif

    /* Init pFBlockIDs (with InstIDs) */
    /*------------------------------------------ */
    #if (NUM_FBLOCKS>0)
    fblock_index = (byte)0;

    for (i=(byte)1;i<=(NUM_FBLOCKS+CMD_NUM_WILDCARDS);i++)    /* i = index in FBLOCK table of Command Interpreter */
    {                                                       /* (forget about NetBlock; i=0) */
        #if (CMD_NUM_WILDCARDS > 0)
        if (   ((byte*)0  != FBlocks[i].Inst_Tab_Ptr)       /* Forget about wildcard "INST ID don't care" in FBLOCK table of Command Interpreter */
            || (FBLOCK_ET == FBlocks[i].FBlock      ) )     /* But accept FBlock ET */
        #endif
        {
            NetBlock.pFBlockIDs.FBlockID[fblock_index]  = FBlocks[i].FBlock;                /* Get FBlockIDs from table */
            NetBlock.pFBlockIDs.InstID[fblock_index]    = InstIDsDefault[fblock_index];     /* Get InstIDs default values (Rom Read) */
            fblock_index++;
        }
    }

    #ifdef NB_CB13
    NbRestoreInstIDs(&NetBlock.pFBlockIDs.InstID[0]);       /* overwrite default values with stored values from nonvolatile memory */
    #endif
    #endif


    /* Init pNodePosition */
    /*------------------------------------------ */
    NbRefreshNodePos();

    /* Init mShutDown */
    /*------------------------------------------ */
    NetBlock.mShutDown.Suspend = MNS_FALSE;

    #ifdef NB_ADD7
    BoundaryGet_Pending = MNS_FALSE;
    #endif

}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbService()                                                */
/* Description : service function of Netblock module                        */
/*               will be called by MostService2()                           */
/*                                                                          */
/* Parameter(s): none                                                       */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_12
void NbService(void)
{
}
#endif



/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*  Section needed by Rx Command Interpreter */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_FBlockIDs_Set()                                         */
/* Description : Set InstID of a certain FBlock (except FBlock: NetBlock)   */
/*               <FBlockID>,<InstOld>,<InstNew>                             */
/*                                                                          */
/* Parameter(s): ptr on tx message, ptr on rx message                       */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I20
byte NB_FBlockIDs_Set(pTMsgTx Tx_Ptr, pTMsgRx Rx_Ptr)
{
    #ifndef NB_CB16
    #if (NUM_FBLOCKS>0)
    byte FBlock_Rx;
    byte InstOld_Rx;
    byte InstNew_Rx;

    byte i;
    byte report;
    byte SearchStatus;
    #endif

    #ifdef CMD_ADD8
    (void) Tx_Ptr;
    #endif
    #endif

    #ifndef CMD_ADD8
    if (Rx_Ptr->Length != (word)3)
    {
        return( CmdErrorMsg(Tx_Ptr, ERR_LENGTH) );
    }
    #endif

    #ifndef NB_CB16
    #if (NUM_FBLOCKS>0)
    FBlock_Rx  = Rx_Ptr->Data[0];
    InstOld_Rx = Rx_Ptr->Data[1];
    InstNew_Rx = Rx_Ptr->Data[2];

    i            = 0;
    SearchStatus = 0;                                               /* SearchStatus: */
                                                                        /*    0x00 := FBlock.Inst found not yet */
                                                                        /*    0x01 := FBlock found */
                                                                        /*    0x02 := FBlock.Inst found */
    do
    {
        if (NetBlock.pFBlockIDs.FBlockID[i] == FBlock_Rx)               /* Check FBlockID */
        {
            SearchStatus = (byte)0x01;

            if (NetBlock.pFBlockIDs.InstID[i] == InstOld_Rx)            /* Check old InstID */
            {
                NetBlock.pFBlockIDs.InstID[i] = InstNew_Rx;             /* Set new InstID */
                SearchStatus = (byte)0x02;
            }
            else
            {
                i++;
            }
        }
        else
        {
            i++;
        }

    }while( i<NUM_FBLOCKS && SearchStatus<2 );

    switch(SearchStatus)
    {
        case 0x02:                                                                  /* no error occurred */
            #ifdef NB_CB12
            NbStoreInstIDs(&NetBlock.pFBlockIDs.InstID[0]);                         /* store InstIDs in nonvolatile memory */
            #endif
            report = OP_NO_REPORT;
            break;

        default:                                                                    /* FBlockID or old InstID failed */
            report = OP_NO_REPORT;                                                  /* return(OP_NO_REPORT), GET operation will report unchanged FBlock list */
            break;
    }

    return (report);
    #else
    return(OP_NO_REPORT);
    #endif

    #else
    return(NBFBlockIDsSet(Tx_Ptr, Rx_Ptr));           /* call application to change InstID */
    #endif
}
#endif





/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_FBlockIDs_Get()                                         */
/* Description : Get FBlockIDs and InstIDs (except FBlock: NetBlock)        */
/*               FBlockIDList::=<FBlockID1>,<InstID1>,<FBlockID2>,.....     */
/*                                                                          */
/* Parameter(s): ptr on tx message, ptr on rx message                       */
/*               Rx_Ptr = NULL, if function is called by NTFS module.       */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I21
byte NB_FBlockIDs_Get(pTMsgTx Tx_Ptr, pTMsgRx Rx_Ptr)
{
    #ifndef NB_CB9
    byte *ptr_data;
    word counter;
    bool overflow;
    #if (NUM_FBLOCKS>0)
    byte i;
    #endif
    #endif


    #ifndef CMD_ADD8
    if (NULL != Rx_Ptr) /* function called by CMD (Rx_Ptr != NULL) or by NTFS (Rx_Ptr == NULL)  */
    {
                                                                    /* Length check only in case of OP_GET,  */
        if (OP_GET == Rx_Ptr->Operation)                            /* but not in case of OP_SETGET */
        {
            if ((byte)0 != Rx_Ptr->Length)
            {
                return( CmdErrorMsg(Tx_Ptr, ERR_LENGTH) );              /* return(OP_ERROR) */
            }
        }
    }
    #endif

    if (NULL != Rx_Ptr)
    {
        if ((byte)0xFF == Rx_Ptr->Inst_ID)                          /* injected message? */
        {
            Tx_Ptr->MidLevelRetries = DEF_MID_LEVEL_RETRIES_FBLOCKIDS;
        }
    }

    #ifndef NB_CB9  /* Response message is prepared by NetServices */
    ptr_data  = &Tx_Ptr->Data[0];
    counter   = (byte)0;
    overflow  = MNS_FALSE;

    #if (NUM_FBLOCKS>0)
    for (i=(byte)0;(i<NUM_FBLOCKS) && (MNS_FALSE == overflow);i++)
    {
        if (   ((byte)0xF0 >  NetBlock.pFBlockIDs.FBlockID[i])      /* Filter supplier specific FBlocks */
            && (FBLOCK_ET  != NetBlock.pFBlockIDs.FBlockID[i]) )    /* and FBlock ET */
        {
            if (counter < (word)(MAX_MSG_TX_DATA - 1))
            {
                *ptr_data++ =  NetBlock.pFBlockIDs.FBlockID[i];
                *ptr_data++ =  NetBlock.pFBlockIDs.InstID[i];
                counter     += (byte)2;
            }
            else
            {
                overflow = MNS_TRUE;
            }
        }
    }
    #endif
    if (MNS_FALSE != overflow)
    {
        return(CmdErrorMsg(Tx_Ptr, ERR_PROCESSING)); /* Processing Error */
    }
    else
    {
        Tx_Ptr->Length = (word)counter;
        return(OP_STATUS);
    }
    #endif /* #ifndef NB_CB9  */

    #ifdef NB_CB9   /* Response message is prepared by Application */
    Tx_Ptr->Operation = OP_STATUS;      /* default, can be overwritten, by using OP_NO_REPORT */
    Tx_Ptr->Length    = (word)0;        /* length and data must be set by application */
    NBFBlockIDsGet(Tx_Ptr);             /* call application to prepare the response */
    return(Tx_Ptr->Operation);
    #endif
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_FBlockIDs_Status()                                      */
/* Description : Handle FBlockIDs Status Message                            */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I2C
byte NB_FBlockIDs_Status(pTMsgRx Rx_Ptr)
{
    #ifdef AH_5
    AddrHCheckFBlockIDs(Rx_Ptr);
    #endif

    #ifdef NB_CBS2
    NbFBlockIDsStatus(Rx_Ptr);                      /* Transport to Application */
    #endif

    #if !(defined AH_5) && !(defined NB_CBS2)
    (void) Rx_Ptr;
    #endif

    return(OP_NO_REPORT);
}
#endif

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_FBlockIDs_Error()                                       */
/* Description : Handle FBlockIDs Error Message                             */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I2F
byte NB_FBlockIDs_Error(pTMsgRx Rx_Ptr)
{
    #ifdef NB_CBE2
    NbFBlockIDsError(Rx_Ptr);                       /* Transport to Application */
    #endif

    return(OP_NO_REPORT);
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_DeviceInfo_Get()                                        */
/* Description : Get DeviceInfo                                             */
/* Parameter(s): ptr on tx message, ptr on rx message                       */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I31
byte NB_DeviceInfo_Get(pTMsgTx Tx_Ptr, pTMsgRx Rx_Ptr)
{
    byte rx_id;
    byte returnvalue;
    byte* tgt_ptr;

    #ifndef CMD_ADD8
    if (Rx_Ptr->Length != (word)1)
        return( CmdErrorMsg(Tx_Ptr, ERR_LENGTH) );              /* return(OP_ERROR) */
    #endif

    rx_id           = Rx_Ptr->Data[0];
    Tx_Ptr->Data[0] = rx_id;                                    /* Data[0] contains ID !! */
    returnvalue     = OP_STATUS;

    switch (rx_id)
    {
        /*--------------------------- */
        /* Transceiver Revision Date */
        /* NetServices Version */
        /* NetServices Revision */
        /*--------------------------- */
        case 0xA0:

            MostGetRevision(&Tx_Ptr->Data[1]);                  /* Data[1]: Day of Transceiver Revision Date */
                                                                /* Data[2]: Month of Transceiver Revision Date */
                                                                /* Data[3]: Year of Transceiver Revision Date */
            tgt_ptr = &Tx_Ptr->Data[4];
            *tgt_ptr++ = ((byte)0x03);                           /* Data[4]..[6]:  NetServices Version */
            *tgt_ptr++ = ((byte)0x00);
            *tgt_ptr++ = ((byte)0x04);
            *tgt_ptr++ = ((byte)0x00);                           /* Data[7]..[10]: NetServices Revision           */
            *tgt_ptr++ = ((byte)0x18);                           /*           */
            *tgt_ptr++ = ((byte)0xFF);                           /*                       */
            *tgt_ptr   = ((byte)0x2A);                           /*                       */

            Tx_Ptr->Length = (word)11;
            break;


        /*--------------------------- */
        /* Handled by application */
        /*--------------------------- */
        default:
            Tx_Ptr->Length = (word)0;                           /* gives the possibility to recognize if the application has created an own error message */ 
            returnvalue = NbGetDeviceInfo(rx_id, Tx_Ptr);       /* Call Application, whenever request cannot  */
                                                                /* be answered by NetServices directly */

            if (   (OP_ERROR == returnvalue)                    /* unknown ID ? */
                && ((word)0  == Tx_Ptr->Length))                /* did application create an error message ? */
            {
                (void)CmdErrorParamWrong( Tx_Ptr, (byte)1, &Rx_Ptr->Data[0], (byte)1);    /* prepare error message   */
            }
            break;
    }

    return(returnvalue);
}
#endif






/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_DeviceInfo_Status()                                     */
/* Description : Handle DeviceInfo Status Message                           */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I3C
byte NB_DeviceInfo_Status(pTMsgRx Rx_Ptr)
{
    #ifdef NB_CBS3
    NbDeviceInfoStatus(Rx_Ptr);                     /* Transport to Application */
    #endif
    return(OP_NO_REPORT);
}
#endif

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_DeviceInfo_Error()                                      */
/* Description : Handle DeviceInfo Error Message                            */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I3F
byte NB_DeviceInfo_Error(pTMsgRx Rx_Ptr)
{
    #ifdef NB_CBE3
    NbDeviceInfoError(Rx_Ptr);                      /* Transport to Application */
    #endif
    return(OP_NO_REPORT);
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_ShutDown_Start()                                        */
/* Description : Start method ShutDown and get flag suspend                 */
/* Parameter(s): ptr on tx message, ptr on rx message                       */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I70
byte NB_ShutDown_Start(pTMsgTx Tx_Ptr, pTMsgRx Rx_Ptr)
{
    byte ret;

    #ifndef CMD_ADD8
    if (Rx_Ptr->Length != (word)1)
    {
        return( CmdErrorMsg(Tx_Ptr, ERR_LENGTH) );          /* return(OP_ERROR) */
    }
    #endif

    ret = OP_NO_REPORT;                                         /* default: No Result required */

    switch (Rx_Ptr->Data[0])
    {
        /*---------------------------------- */
        /*  NetBlock.ShutDown.Start(Query) */
        /*---------------------------------- */
        case NB_SHUTDOWN_QUERY:
            NetBlock.mShutDown.Suspend = NbShutDown();                  /* Call Application and get new status  */

            if (MNS_FALSE != NetBlock.mShutDown.Suspend)                     /* Application not ready for shut down process ? */
            {
                Tx_Ptr->Data[0] = NB_SHUTDOWN_SUSPEND;                  /* Answer: Suspend */
                Tx_Ptr->Length  = (word)1;
                ret = OP_RESULT;
            }
            break;


        /*---------------------------------- */
        /*  NetBlock.ShutDown.Start(Execute) */
        /*---------------------------------- */
        case NB_SHUTDOWN_EXECUTE:
            NbShutDownExecute();                                        /* Call Application */
                                            /* --> The decentral device registry is not stored on each state transition NetOn->NetOff, */
            #ifdef AH_11                    /*     but only when the telegram "NetBlock.ShutDown.Start(Execute)" has been received. */
            AddrHStoreDevTabAH();                                       /* Store decentral registry in non-volatile menory area. */
            #endif                                                      /* But check first, if table has been initialized */

            #ifdef NTF_0
            NtfInit();                                                  /* initialize Notification Service */
            #endif

            #ifdef ENABLE_CFG_MSG_FILTER
            SetConfigStateFilterStatus(MNS_ON);
            #endif
            break;

        case NB_SHUTDOWN_DEVICE:
            if (MNS_FALSE == NbShutDownDevice())
            {
                ret = CmdErrorParamWrong(Tx_Ptr, (byte)1, &Rx_Ptr->Data[0], (byte)1);           /* Prepare Error Message */
            }
            break;

        /*---------------------------------- */
        /*  wrong parameter */
        /*---------------------------------- */
        default:
            ret = CmdErrorParamWrong(Tx_Ptr, (byte)1, &Rx_Ptr->Data[0], (byte)1);           /* Prepare Error Message */
            break;
    }
    return(ret);
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_ShutDown_Result()                                       */
/* Description : Handle ShutDown Result Message                             */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I7C
byte NB_ShutDown_Result(pTMsgRx Rx_Ptr)
{
    #ifdef NB_CBS7
    NbShutDownResult(Rx_Ptr);                           /* Transport to Application */
    #endif
    return(OP_NO_REPORT);
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_ShutDown_Error()                                        */
/* Description : Handle ShutDown Error Message                              */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I7F
byte NB_ShutDown_Error(pTMsgRx Rx_Ptr)
{
    #ifdef NB_CBE7
    NbShutDownError(Rx_Ptr);                            /* Transport to Application */
    #endif
    return(OP_NO_REPORT);
}
#endif




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_NodeAddr_Status()                                       */
/* Description : Handle NodeAddress Status Message                          */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I1C
byte NB_NodeAddr_Status(pTMsgRx Rx_Ptr)
{
    #ifdef MSV2_10
    if (MSG_SRC_INIC == Rx_Ptr->Src_Adr)
    {
        NodeAddrChangeComplete();
    }
    #endif

    #ifdef NB_CBS1
    NbNodeAddrStatus(Rx_Ptr);                       /* Transport to Application */
    #endif
    return(OP_NO_REPORT);
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_NodeAddr_Error()                                        */
/* Description : Handle NodeAddress Error Message                           */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I1F
byte NB_NodeAddr_Error(pTMsgRx Rx_Ptr)
{
    #ifdef NB_CBE1
    NbNodeAddrError(Rx_Ptr);                        /* Transport to Application */
    #endif
    return(OP_NO_REPORT);
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_GroupAddr_Status()                                      */
/* Description : Handle GroupAddress Status Message                         */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I4C
byte NB_GroupAddr_Status(pTMsgRx Rx_Ptr)
{
    #ifdef NB_CBS4
    NbGroupAddrStatus(Rx_Ptr);                      /* Transport to Application */
    #endif
    return(OP_NO_REPORT);
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_GroupAddr_Error()                                       */
/* Description : Handle GroupAddress Error Message                          */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I4F
byte NB_GroupAddr_Error(pTMsgRx Rx_Ptr)
{
    #ifdef NB_CBE4
    NbGroupAddrError(Rx_Ptr);                       /* Transport to Application */
    #endif
    return(OP_NO_REPORT);
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_Version_Get()                                           */
/* Description : Handle Version Get Message                                 */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I151
byte NB_Version_Get(pTMsgTx tx_ptr)
{
  tx_ptr->Data[0] = NB_VERSION_MAJOR;
  tx_ptr->Data[1] = NB_VERSION_MINOR;
  tx_ptr->Data[2] = NB_VERSION_BUILD;
  tx_ptr->Length  = (word)3;
  return(OP_STATUS);
}
#endif

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_Version_Status()                                        */
/* Description : Handle Version Status Message                              */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I15C
byte NB_Version_Status(pTMsgRx Rx_Ptr)
{
    #ifdef NB_CBS15
    NbVersionStatus(Rx_Ptr);                      /* Transport to Application */
    #endif
    return(OP_NO_REPORT);
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_Version_Error()                                         */
/* Description : Handle Version Error Message                               */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I15F
byte NB_Version_Error(pTMsgRx Rx_Ptr)
{
    #ifdef NB_CBE15
    NbVersionError(Rx_Ptr);                       /* Transport to Application */
    #endif
    return(OP_NO_REPORT);
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_FBlockInfo_Get()                                        */
/* Description : Handle FBlockInfo Get Message                              */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OpType                                                     */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I171
byte NB_FBlockInfo_Get(pTMsgTx Tx_Ptr, pTMsgRx Rx_Ptr)
{
    word rx_id;
    byte name[] = "NetBlock";
    byte i;
    byte version[3];
    byte return_value;
    bool answer_prepared;

    #ifndef CMD_ADD8
    if (Rx_Ptr->Length != (word)2)                  /* parameter ID is required */
    {
        return( CmdErrorMsg(Tx_Ptr, ERR_LENGTH) );  /* return(OP_ERROR) */
    }
    #endif

    rx_id = (word)((word)Rx_Ptr->Data[0] << 8);     /* Data[0] contains HB of ID */
    rx_id += Rx_Ptr->Data[1];                       /* Data[1] contains LB of ID */

    Tx_Ptr->Data[0] = Rx_Ptr->Data[0];              /* prepare TX message */
    Tx_Ptr->Data[1] = Rx_Ptr->Data[1];
    Tx_Ptr->Length  = (word)2;

    answer_prepared = MNS_TRUE;

    if (rx_id < (word)0x1000)                             /* FktID ?*/
    {
        Tx_Ptr->Length  = (word)3;

        switch (rx_id)
        {
            case FUNC_FBLOCKIDS:
            case FUNC_IMPLFBLOCKIDS:
                #ifdef NB_ADD2
                Tx_Ptr->Data[2] = MAT_INTERFACE_ONLY;
                #else
                Tx_Ptr->Data[2] = MAT_FULLY_IMPLEMENTED_VERIFIED;
                #endif
                break;

            case FUNC_DEVICEINFO:
            case FUNC_SHUTDOWN:
            case FUNC_FBLOCKINFO:
                Tx_Ptr->Data[2] = MAT_PARTLY_IMPLEMENTED_VERIFIED;
                break;

            case FUNC_NODEPOSITION:
            case FUNC_NODEADDRESS:
            case FUNC_GROUPADDRESS:
            case FUNC_RETRYPARAMETERS:
            case FUNC_SAMPLINGFREQUENCY:
            case FUNC_SHUTDOWNREASON:
            case FUNC_VERSION:
            case FUNC_EUI48:
            case FUNC_NB_RBDRESULT:
            case FUNC_NB_BOUNDARY:
                Tx_Ptr->Data[2] = MAT_FULLY_IMPLEMENTED_VERIFIED;
                break;

            default:
                Tx_Ptr->Data[2] = MAT_UNKNOWN;
                answer_prepared = MNS_FALSE;
                break;
        }
    }
    else
    {
        switch (rx_id)
        {
            case FBI_FBLOCK_NAME:
                Tx_Ptr->Data[2] = FRMT_ISO8859;
                for(i=(byte)0; i<(byte)(sizeof(name)/sizeof(name[0])); ++i)
                {
                    Tx_Ptr->Data[i+3] = (byte)name[i];
                }
                Tx_Ptr->Length = (word)i + (word)3;
                break;

            case FBI_FBLOCK_VERSION:
                NbGetVersion(&version[0]);

                Tx_Ptr->Data[2] = FRMT_ISO8859;

                i = MsgVersionToISO8859(&version[0], &Tx_Ptr->Data[3]);

                Tx_Ptr->Length  = (word)i + (word)3;
                break;

            case FBI_MOST_VERSION:
                NbGetMOSTVersion(&version[0]);

                Tx_Ptr->Data[2] = FRMT_ISO8859;

                i = MsgVersionToISO8859(&version[0], &Tx_Ptr->Data[3]);

                Tx_Ptr->Length  = (word)i + (word)3;
                break;

            case FBI_FBLOCK_TYPE:
                Tx_Ptr->Data[2] = FRMT_ISO8859;
                Tx_Ptr->Data[3] = (byte)'\0';
                Tx_Ptr->Length  = (word)4;
                break;

            default :
                answer_prepared = MNS_FALSE;
                break;

        }
    }

    /* Call Application, because request cannot be answered by NetServices completely      */
    if (MNS_FALSE == NbFBlockInfoGet(rx_id, answer_prepared, Tx_Ptr))           /* unknown ID ? */
    {
        return_value = CmdErrorParamWrong( Tx_Ptr, (byte)1, &Rx_Ptr->Data[0], (byte)2);  /* prepare error message   */
    }
    else
    {
        return_value = OP_STATUS;
    }

    return(return_value);
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_FBlockInfo_Status()                                     */
/* Description : Handle FBlockInfo Status Message                           */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I17C
byte NB_FBlockInfo_Status(pTMsgRx Rx_Ptr)
{
    #ifdef NB_CBS17
    NbFBlockInfoStatus(Rx_Ptr);                     /* Transport to Application */
    #endif
    return(OP_NO_REPORT);
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_FBlockInfo_Error()                                      */
/* Description : Handle FBlockInfo Error Message                            */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I17F
byte NB_FBlockInfo_Error(pTMsgRx Rx_Ptr)
{
    #ifdef NB_CBE17
    NbFBlockInfoError(Rx_Ptr);                      /* Transport to Application */
    #endif
    return(OP_NO_REPORT);
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_ImplFBlocks_Get()                                       */
/* Description : Handle FBlockInfo Get Message                              */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I181
byte NB_ImplFBlocks_Get(pTMsgTx Tx_Ptr)
{
    #ifndef NB_CB21
    byte   *ptr_data;
    dword   counter;          /* avoid overflow if MAX_MSG_TX_DATA == 65535 */
    bool    overflow;
    word    i;
    word    fblock_index;

    ptr_data     = &Tx_Ptr->Data[0];
    counter      = (dword)0;
    overflow     = MNS_FALSE;
    fblock_index = (word)0;

    #if (NUM_FBLOCKS>0)
    for (i=(word)1; (i<1+NUM_FBLOCKS+CMD_NUM_WILDCARDS) && (MNS_FALSE==overflow); i++)      /* start after NetBlock */
    {
        if ((byte)0xF0 > FBlocks[i].FBlock)                                   /* Filter supplier specific FBlocks */
        {
            if (counter < MAX_MSG_TX_DATA - 1)
            {
                if (FBLOCK_ET != FBlocks[i].FBlock)                     /* and FBlock ET */
                {
                    #if (CMD_NUM_WILDCARDS > 0)
                    if ( FBlocks[i].Inst_Tab_Ptr != (byte*)0 )          /* Forget about wildcard "INST ID don't care" in FBLOCK table of Command Interpreter */
                    #endif
                    {
                        *ptr_data++ =  FBlocks[i].FBlock;
                        *ptr_data++ =  InstIDsDefault[fblock_index];     /* Wildcard entries have no entry in InstIDsDefault */
                        fblock_index++;
                        counter     += (byte)2;
                    }
                }
                else
                {
                    fblock_index++;
                }
            }
            else
            {
                overflow = MNS_TRUE;
            }
        }
    }
    #endif

    if (MNS_FALSE != overflow)
    {
        return(CmdErrorMsg(Tx_Ptr, ERR_PROCESSING)); /* Processing Error */
    }
    else
    {
        Tx_Ptr->Length = (word)counter;
        return(OP_STATUS);
    }
    #endif

    #ifdef NB_CB21
    Tx_Ptr->Operation = OP_STATUS;      /* default, can be overwritten, by using OP_NO_REPORT */
    Tx_Ptr->Length    = (word)0;        /* length and data must be set by application */
    NbImplFBlocksGet(Tx_Ptr);           /* call application to prepare the response */
    return(Tx_Ptr->Operation);
    #endif
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_ImplFBlocks_Status()                                    */
/* Description : Handle ImplFBlocks Status Message                          */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I18C
byte NB_ImplFBlocks_Status(pTMsgRx Rx_Ptr)
{
    #ifdef NB_CBS18
    NbImplFBlocksStatus(Rx_Ptr);                     /* Transport to Application */
    #endif
    return(OP_NO_REPORT);
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_ImplFBlocks_Error()                                     */
/* Description : Handle ImplFBlocks Error Message                           */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I18F
byte NB_ImplFBlocks_Error(pTMsgRx Rx_Ptr)
{
    #ifdef NB_CBE18
    NbImplFBlocksError(Rx_Ptr);                      /* Transport to Application */
    #endif
    return(OP_NO_REPORT);
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_EUI48_Status()                                          */
/* Description : Handles EUI48.Status Message                               */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I19C
byte NB_EUI48_Status(pTMsgRx Rx_Ptr)
{
    #ifdef NB_CBS19
    NbEUI48Status(Rx_Ptr);                     /* Transport to Application */
    #endif
    return(OP_NO_REPORT);
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_EUI48_Error()                                           */
/* Description : Handle EUI48 Error Message                                 */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I19F
byte NB_EUI48_Error(pTMsgRx Rx_Ptr)
{
    #ifdef NB_CBE19
    NbEUI48Error(Rx_Ptr);                      /* Transport to Application */
    #endif
    return(OP_NO_REPORT);
}
#endif




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_NodePosition_Status()                                   */
/* Description : Handle NodePosition Status Message                         */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I9C
byte NB_NodePosition_Status(pTMsgRx Rx_Ptr)
{
    #ifdef NB_CBS9
    NbNodePositionStatus(Rx_Ptr);                       /* Transport to Application */
    #endif
    return(OP_NO_REPORT);
}
#endif

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_NodePosition_Error()                                    */
/* Description : Handle NodePosition Error Message                          */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I9F
byte NB_NodePosition_Error(pTMsgRx Rx_Ptr)
{
    #ifdef NB_CBE9
    NbNodePositionError(Rx_Ptr);                        /* Transport to Application */
    #endif
    return(OP_NO_REPORT);
}
#endif




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_RetryParameters_Status()                                */
/* Description : Handle RetryParameters Status Message                      */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I10C
byte NB_RetryParameters_Status(pTMsgRx Rx_Ptr)
{
    #ifdef NB_CBS10
    NbRetryParametersStatus(Rx_Ptr);                        /* Transport to Application */
    #endif
    return(OP_NO_REPORT);
}
#endif

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_RetryParameters_Error()                                 */
/* Description : Handle RetryParameters Error Message                       */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I10F
byte NB_RetryParameters_Error(pTMsgRx Rx_Ptr)
{
    #ifdef NB_CBE10
    NbRetryParametersError(Rx_Ptr);                     /* Transport to Application */
    #endif
    return(OP_NO_REPORT);
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_SamplingFrequency_Status()                              */
/* Description : Handle SamplingFrequency Status Message                    */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I11C
byte NB_SamplingFrequency_Status(pTMsgRx Rx_Ptr)
{
    #ifdef NB_CBS11
    NbSamplingFrequencyStatus(Rx_Ptr);                      /* Transport to Application */
    #endif
    return(OP_NO_REPORT);
}
#endif

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_SamplingFrequency_Error()                               */
/* Description : Handle SamplingFrequency Error Message                     */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I11F
byte NB_SamplingFrequency_Error(pTMsgRx Rx_Ptr)
{
    #ifdef NB_CBE11
    NbSamplingFrequencyError(Rx_Ptr);                       /* Transport to Application */
    #endif
    return(OP_NO_REPORT);
}
#endif





/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_Boundary_SetGet()                                       */
/* Description : Set Boundary value                                         */
/* Parameter(s): ptr on tx message, ptr on rx message                       */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I142
byte NB_Boundary_SetGet(pTMsgTx Tx_Ptr, pTMsgRx Rx_Ptr)
{
    byte boundary;

    #ifndef CMD_ADD8
    if ((word)1 != Rx_Ptr->Length)                                /* parameter length check */
    {
        return(CmdErrorMsg(Tx_Ptr, ERR_LENGTH));
    }
    #endif

    boundary = Rx_Ptr->Data[0];

    if (    ((MNS_FALSE != MostIsSupported(NSF_MOST_25)) && ((boundary > (byte)15) || (boundary < (byte)6)))
         || ((MNS_FALSE != MostIsSupported(NSF_MOST_50)) &&  (boundary > (byte)29) ) )
    {
        return(CmdErrorParamWrong(Tx_Ptr, (byte)1, &Rx_Ptr->Data[0], (byte)1));
    }

    if (MNS_FALSE != NbBoundarySetQuery(boundary))
    {
        pTMsgTx     my_ptr;

        my_ptr = MsgGetTxPtr();
        if ((NULL != my_ptr) && (MNS_FALSE == BoundaryGet_Pending))
        {
            BoundaryGet_Pending = MNS_TRUE;
            BoundaryPtr         = Tx_Ptr;                   /* save Tx ptr for late use  */

            my_ptr->Tgt_Adr     = MSG_TGT_INIC;             /* forward message to INIC   */
            my_ptr->FBlock_ID   = Rx_Ptr->FBlock_ID;
            my_ptr->Inst_ID     = Rx_Ptr->Inst_ID;
            my_ptr->Func_ID     = Rx_Ptr->Func_ID;
            my_ptr->Operation   = Rx_Ptr->Operation;
            my_ptr->Length      = Rx_Ptr->Length;
            my_ptr->Data[0]     = Rx_Ptr->Data[0];

            AmsMsgSend(my_ptr);
        }
        else
        {                                                   /* message could not be sent to INIC */
            return(CmdErrorMsg(Tx_Ptr, ERR_PROCESSING));    /* report error                      */
        }

        return(CMD_TX_RETAIN);
    }
    else
    {
        return(CmdErrorMsg(Tx_Ptr, ERR_NOTAVAILABLE));
    }
}
#endif

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_Boundary_Status()                                       */
/* Description : Service NetBlock.Boundary.Status                           */
/* Parameter(s): ptr on rx msg                                              */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I14C
byte NB_Boundary_Status(pTMsgRx Rx_Ptr)
{
    if (MSG_SRC_INIC == Rx_Ptr->Src_Adr)                            /* message from INIC ? */
    {
        if (MNS_FALSE != BoundaryGet_Pending)
        {
            BoundaryGet_Pending    = MNS_FALSE;

            BoundaryPtr->Operation = OP_STATUS;                     /* send status message using retained msg pointer */
            BoundaryPtr->Length    = (word)1;
            BoundaryPtr->Data[0]   = Rx_Ptr->Data[0];               /* new boundary value */
            AmsMsgSend(BoundaryPtr);
        }
    }
    else
    {
        #ifdef NB_CBS14
        NbBoundaryStatus(Rx_Ptr);
        #endif
    }
    return(OP_NO_REPORT);
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_Boundary_Error()                                        */
/* Description : Service NetBlock.Boundary.Error                            */
/* Parameter(s): ptr on rx msg                                              */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I14F
byte NB_Boundary_Error(pTMsgRx Rx_Ptr)
{
    byte i;

    if (MSG_SRC_INIC == Rx_Ptr->Src_Adr)                    /* message from INIC ? */
    {
        if (MNS_FALSE != BoundaryGet_Pending)               /* boundary value could not be changed or Deallocation All failed */
        {
            BoundaryGet_Pending     = MNS_FALSE;

            BoundaryPtr->Operation  = OP_ERROR;             /* send error message using retained msg pointer */
            BoundaryPtr->Length     = Rx_Ptr->Length;       /* copy error info received from INIC */

            for (i = (byte)0; i < BoundaryPtr->Length; ++i)
            {
                BoundaryPtr->Data[i] = Rx_Ptr->Data[i];
            }
            AmsMsgSend(BoundaryPtr);
        }
    }
    else
    {
        #ifdef NB_CBE14
        NbBoundaryError(Rx_Ptr);
        #endif
    }
    return(OP_NO_REPORT);
}
#endif





/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_ShutDownReason_Set()                                    */
/* Description : Handle ShutDownReason Set Message                          */
/* Parameter(s): ptr on tx message                                          */
/*               ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I160
byte NB_ShutDownReason_Set(pTMsgTx Tx_Ptr, pTMsgRx Rx_Ptr)
{
    #ifndef CMD_ADD8
    if (Rx_Ptr->Length != (word)1)
    {
        return( CmdErrorMsg(Tx_Ptr, ERR_LENGTH) );
    }
    #endif

    if (Rx_Ptr->Data[0] != (byte)0x00)
    {
        return(CmdErrorParamWrong(Tx_Ptr, (byte)0x01, &Rx_Ptr->Data[0], (byte)0x01));
    }

    /* reset shutdown reason */
    VmsvResetSSOResult();

    return(OP_NO_REPORT);
}
#endif

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_ShutDownReason_Status()                                 */
/* Description : Handle ShutDownReason Status Message                       */
/* Parameter(s): ptr on tx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I161
byte NB_ShutDownReason_Get(pTMsgTx Tx_Ptr)
{
    Tx_Ptr->Length = VmsvGetSSOResult(Tx_Ptr);

    return(OP_STATUS);
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_ShutDownReason_Status()                                 */
/* Description : Handle ShutDownReason Status Message                       */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I16C
byte NB_ShutDownReason_Status(pTMsgRx Rx_Ptr)
{
    #ifdef NB_CBS16
    NbShutDownReasonStatus(Rx_Ptr);                       /* Transport to Application */
    #endif
    return(OP_NO_REPORT);
}
#endif

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NB_ShutDownReason_Error()                                        */
/* Description : Handle ShutDownReason Error Message                     */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_I16F
byte NB_ShutDownReason_Error(pTMsgRx Rx_Ptr)
{
    #ifdef NB_CBE16
    NbShutDownReasonError(Rx_Ptr);                        /* Transport to Application */
    #endif
    return(OP_NO_REPORT);
}
#endif





/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*  Further Functions */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbGetFBlockIndex()                                         */
/* Description : Get local FBlock Index of FBlockID.InstID                  */
/* Parameter(s): FBlockID, InstID                                           */
/* Returns     : Local FBlock Index                                         */
/*                  0xFF means: FBlockID.InstID not found                   */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_4
byte NbGetFBlockIndex(byte fblock_id, byte inst_id)
{
  #if (NUM_FBLOCKS>0)
    byte index;

    index = (byte)0;

    do
    {
        if (NetBlock.pFBlockIDs.FBlockID[index] == fblock_id)
        {
            if ( (NetBlock.pFBlockIDs.InstID[index] == inst_id) || ((byte)0 == inst_id) )
            {
                return(index);
            }
        }

        index++;

    }
    while ( index < (byte)NUM_FBLOCKS );
  #endif

    return ((byte)0xFF);              /* FBlockID.InstID not found */
}
#endif




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbRefreshNodePos()                                         */
/* Description : Refresh current pNodePosition                              */
/* Parameter(s): FBlockID, InstID                                           */
/* Returns     : Local FBlock Index                                         */
/*                  0xFF means: FBlockID.InstID not found                   */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_5
void NbRefreshNodePos(void)
{
    if ( (byte)0xFF == MostGetNodePos() )
    {
        NodePositionValid = MNS_FALSE;
    }
    else
    {
        NodePositionValid = MNS_TRUE;
    }
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbGoNetOff()                                               */
/* Description : Is called by Supervisor (Basic Layer) whenever             */
/*               NetInterface transfers from state NET_ON to NET_OFF        */
/* Parameter(s): none                                                       */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_6
void NbGoNetOff(void)
{
    #ifdef AH_14
    AddrHClearTasks();
    #endif
}
#endif





/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbSetNodeAddr()                                            */
/* Description : Set Property NodeAddress in FBlock NetBlock                */
/*               Set NodeAddress in Transceiver Registers (NAH,NAL)         */
/*               Store new address in non-volatile memory area              */
/*                                                                          */
/* Parameter(s): new node address                                           */
/* Returns     : MNS_TRUE: Address changed                                  */
/*               MNS_FALSE: Change denied                                   */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_7
bool NbSetNodeAddr(word addr)
{
    MostSetNodeAdr( addr );                                         /* Set Property in INIC */
    return(MNS_TRUE);
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbSetGetNodeAddr()                                         */
/* Description : Set Property NodeAddress in FBlock NetBlock                */
/*               Forces status message from INIC                            */
/*                                                                          */
/* Parameter(s): new node address                                           */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_15
void NbSetGetNodeAddr(word addr)
{
    MostSetGetNodeAdr( addr );                                         /* Set Property in INIC */
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbSetGroupAddr()                                           */
/* Description : Set Property GroupAddress in FBlock NetBlock               */
/*               Set Transceiver Register (GA)                              */
/*               Store new address in non-volatile memory area              */
/*                                                                          */
/* Parameter(s): LowByte of new group address                               */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_8
void NbSetGroupAddr(byte addr)
{
    MostSetGroupAdr( addr );                                        /* Set Transceiver Register */
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbSetShadowInstID()                                        */
/* Description : Set InstID of a certain Shadow FBlock                      */
/*               <FBlockID>,<InstOld>,<InstNew>                             */
/*                                                                          */
/* Parameter(s): ptr on tx message, ptr on rx message                       */
/* Returns     : result                                                     */
/*                  0 - successful change                                   */
/*                  1 - old InstID not found                                */
/*                  2 - FBlockID not found                                  */
/*                  3 - no shadow FBlocks available                         */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_9
byte NbSetShadowInstID(byte FBlock, byte InstOld, byte InstNew)
{
    #if (NUM_FBLOCKS_SHADOW>0)
    byte i;
    byte SearchStatus;

    i = CMD_SIZE_FBLOCK_TABLE - NUM_FBLOCKS_SHADOW;

    SearchStatus = (byte)0;                                     /* SearchStatus: */
                                                                /*    0x00 := FBlock.Inst found not yet */
                                                                /*    0x01 := FBlock found */
                                                                /*    0x02 := FBlock.Inst found */
    do
    {
        if (FBlocks[i].FBlock == FBlock)                        /* Check FBlockID */
        {
            SearchStatus = (byte)0x01;
            if (FBlocks[i].Inst_Tab_Ptr != 0)                   /* no pointer, but INST_ANY */
            {
                if (*FBlocks[i].Inst_Tab_Ptr == InstOld)            /* Check old InstID */
                {
                    *FBlocks[i].Inst_Tab_Ptr = InstNew;             /* Set new InstID */
                    SearchStatus = (byte)0x02;
                }
                else
                {
                    i++;
                }
            }
            else
            {
                i++;
            }
        }
        else
        {
            i++;
        }

    }while( ((byte)i<CMD_SIZE_FBLOCK_TABLE) && (SearchStatus < (byte)2) );

    switch(SearchStatus)
    {
        case 0x02:                                              /* no error occurred */
            #ifdef NB_CB14
            NbStoreShadowInstIDs(&InstIDShadow[0]);             /* store InstIDs in nonvolatile memory */
            #endif
            return((byte)0);

        case 0x01:                                              /* old InstID failed */
            return((byte)1);

        default:                                                /* FBlockID failed */
            return((byte)2);
    }

    #else
    return((byte)3);
    #endif
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbCheckPosValid()                                          */
/* Description : Check if answer to the incoming message depends on         */
/*               node position and if node position value is valid          */
/*                                                                          */
/* Parameter(s): ptr on rx message or NULL pointer                          */
/* Returns     : result                                                     */
/*                  0 - node position is valid                              */
/*                      or no answer will be sent                           */
/*                      or answer does not depend on node position          */
/*                  1 - node position value is invalid                      */
/*                      and message has to be discarded (FBlockIDS.Get)     */
/*                  2 - node position value is invalid                      */
/*                      and answer can be delayed                           */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_10
byte NbCheckPosValid(pTMsgRx rx_ptr)
{
    byte fblock;

    if (NULL == rx_ptr)
    {
        if (MNS_FALSE != NodePositionValid)
        {
            return((byte)0);                                /* ok */
        }
        else
        {
            return((byte)2);                                /* delay  */
        }
    }
    else
    {
        if (MNS_FALSE != NodePositionValid)
        {
            return((byte)0);                                /* ok */
        }
        else
        {
            fblock = rx_ptr->FBlock_ID;

            if (   (FBLOCK_NETBLOCK == fblock)              /* NetBlock message ? */
               &&  (rx_ptr->Operation < OP_REPORTS) )       /* command or report message ? */
            {
                if (FUNC_FBLOCKIDS == rx_ptr->Func_ID)      /* FBlockIDs command? */
                    return((byte)1);                        /* discard message */
                else
                    return((byte)2);                        /* delay answer */
            }
            else if (  (FBLOCK_ET  == fblock)               /* FBlock ET ? */
                   && (rx_ptr->Operation < OP_REPORTS))     /* command or report message ? */
            {
                return((byte)2);                            /* delay answer */
            }
            else
            {
                return ((byte)0);                           /* ok */
            }
        }
    }
}
#endif




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbGetVersion()                                             */
/* Description : deliver NetBlock Version to array                          */
/* Parameter(s): ptr to array                                               */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_13
void NbGetVersion(byte *version)
{
    *version++ = NB_VERSION_MAJOR;
    *version++ = NB_VERSION_MINOR;
    *version   = NB_VERSION_BUILD;
    return;
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbGetMOSTVersion()                                         */
/* Description : deliver MOST Spec Version to array                         */
/* Parameter(s): ptr to array                                               */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NB_14
void NbGetMOSTVersion(byte *version)
{
    *version++ = GFB_VERSION_MAJOR;
    *version++ = GFB_VERSION_MINOR;
    *version   = GFB_VERSION_BUILD;
    return;
}
#endif



