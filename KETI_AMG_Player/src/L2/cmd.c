/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         MOST Command Interpreter
File:           cmd.c
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

#ifdef MSV2_MIN
    #include "msv2.h"
#endif

#ifdef NS_INC_CMD
    #include NS_INC_CMD
#endif



/*
------------------------------------------------------------------------------
        Local Definitions
------------------------------------------------------------------------------
*/


/* return values of CmdRxInterpreter(), stored in CmdRx_RetVal: */
/*------------------------------------------------------------- */

  #define CMD_F_OK          ((byte)0x00)    /* message can be cleared (no flag stored in CmdRx_RetVal) */
  #define CMD_F_RETRIGGER   CMD_RX_REPEAT   /* retrigger same message again */
  #define CMD_F_RX_RETAIN   CMD_RX_RETAIN   /* retain last received message */

  #define CMD_F_MASK        (CMD_RX_REPEAT | CMD_RX_RETAIN) /* These flags within 'Report' (returned value of CB function) */
                                                            /* are used (and combined) as return value of CmdInterpreter. */
                                                            /* They are used to indicate the right return value of MsgRxOutMsg(). */




/* local switches: */
/*---------------- */
#ifndef CMD_ADD8                /* Perform basic length check if parametric length check is not enabled */
#define CMD_LEN_CHECK_ZERO      /* Command Interpreter will perform a length check, */
                                /* whenever the property is read directly */
                                /* by data pointer. */
                                /* If this macro is not defined, the length field */
                                /* is ignored. */
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

#ifdef CMD_MIN

    #if NUM_FBLOCKS_SHADOW > 0
    byte InstIDShadow[NUM_FBLOCKS_SHADOW];
    #endif

    /* Support of application groupcast (FBlockID==0xFF and/or InstID==0xFF) */
    _CONST struct FBlock_L_Type *Cmd_FBlock_Tab_Ptr_Holder;  /* these two variables are place holders so */
    byte Cmd_FBlock_Position_Holder;                         /* when rerunning CmdRxInterpreter() we start in the right place */

    #ifdef CMD_ADD7
        #define CMD_EVEN_POS ((byte) 0)
        #define CMD_ODD_POS  ((byte) 1)
    #endif

#endif  /* CMD_MIN */




/*
------------------------------------------------------------------------------
        Local Function Prototypes
------------------------------------------------------------------------------
*/

#ifdef CMD_16
byte CmdErrorParam_W_NA(pTMsgTx tx_ptr, byte index, byte* param_ptr, byte num);
#endif

#ifdef CMD_22
void CmdAutoInsertSenderHandlePtr(pTMsgTx tx_ptr, pTMsgRx rx_ptr);
#endif


/*
------------------------------------------------------------------------------
        Tables
------------------------------------------------------------------------------
*/

#ifdef CMD_MIN
#include "t_fblock.tab"             /* Table containing all FBlocks, FBlock Shadows and default InstIDs */
#endif






/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CmdInit()                                                  */
/* Description : Init the Command Interpreter                               */
/* Parameter(s): none                                                       */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef CMD_0
void CmdInit(void)
{
    #if (NUM_FBLOCKS_SHADOW > 0)
    byte i;
    #endif

    /* Init InstIDShadow[] */
    /*------------------------------------------ */
    #if (NUM_FBLOCKS_SHADOW > 0)
    for (i=(byte)0;i<(byte)NUM_FBLOCKS_SHADOW;i++)
    {
        InstIDShadow[i] = InstIDsShadowDefault[i];              /* Get InstIDs default values (RomRead) */
    }
    #ifdef NB_CB15
    NbRestoreShadowInstIDs(&InstIDShadow[0]);
    #endif
    #endif

    Cmd_FBlock_Tab_Ptr_Holder  = NULL;                            /* the values need to be initialized */
    Cmd_FBlock_Position_Holder = (byte)0;
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : MsgRxOutMsg()                                               */
/* Description : Callbackfunction of the AMS                                */
/*               This function is called when a message has been received   */
/* Parameter(s): ptr at received message                                    */
/* Returns     : acknowledge if message was interpreted                     */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef CMD_1
byte MsgRxOutMsg(pTMsgRx rx_ptr)
{
    byte reaction;
    pTMsgTx tx_ptr;

    if (   ((word)0x0300 == (rx_ptr->Src_Adr & (word)0xFF00))           /* ignore messages sent from invalid addresses */
        || ((word)0x0400 == (rx_ptr->Src_Adr & (word)0xFF00)) )
    {
        #ifdef NETWORKMASTER_LOCAL
        if (FBLOCK_NETBLOCK != rx_ptr->FBlock_ID)
        #endif
        {
            MsgFreeRxMsg(rx_ptr);
            return(MSG_RX_TAKE);                            /* discard message */
        }
    }


    #ifdef NB_MIN
    if ((byte)1 == NbCheckPosValid(rx_ptr))                 /* FBlockIDs.Get and node position invalid? */
    {
        MsgFreeRxMsg(rx_ptr);
        return(MSG_RX_TAKE);                                /* discard message */
    }
    #endif

    #ifdef NB_MIN
    if (    ((byte)2 == NbCheckPosValid(rx_ptr))            /* node position invalid and result depends on it? */
        #ifdef MSV2_MIN
         || (MNS_FALSE == CheckConfigState())               /* node address temporarily invalid? */
        #endif
        )
    {
        return(MSG_RX_BUSY);                                /* force retrigger */
    }
    #endif


    tx_ptr = MsgGetTxPtr();

    if (tx_ptr)
    {
        #ifdef AMS_TX_ADD9
        tx_ptr->MidLevelRetries = (byte)DEF_MID_LEVEL_RETRIES_INT_PROC;     /* set number of mid level retries */
        #endif

        #ifdef CMD_CB2
        if (NULL == Cmd_FBlock_Tab_Ptr_Holder)                  /* avoid multiple calls of CmdRxFilter() in case of wildcard addressing */
        {
            reaction = CmdRxFilter(tx_ptr, rx_ptr);             /* callback application to filter message */
                                                                /*   reaction:  0x03: message was not interpreted and must be retriggered */
                                                                /*              0x02: message was interpreted and tx buffer used */
                                                                /*              0x01: message was interpreted, but no tx msg required */
                                                                /*              0x00: message was checked (modified), */
                                                                /*                    and CmdRxInterpreter() must be called now */
                                                                /*                    (no tx msg required) */
            switch(reaction)
            {
                case 0x03:
                    MsgTxUnused(tx_ptr);                        /* release tx message entry */
                    return(MSG_RX_BUSY);                        /* force retrigger */

                case 0x02:
                    AmsMsgSend(tx_ptr);                            /* send tx message */
                    MsgFreeRxMsg(rx_ptr);                       /* release rx message entry, */
                    return(MSG_RX_TAKE);                        /* pointer was grabbed and message was interpreted */

                case 0x01:
                    MsgTxUnused(tx_ptr);                        /* release tx message entry */
                    MsgFreeRxMsg(rx_ptr);                       /* release rx message entry, */
                    return(MSG_RX_TAKE);                        /* pointer was grabbed and message was interpreted */

                default:
                    break;
            }
        }
        #endif

        reaction = CmdRxInterpreter(tx_ptr, rx_ptr);

        switch(reaction)
        {
            case CMD_F_OK:
                MsgFreeRxMsg(rx_ptr);                       /* release message entry, */
                return(MSG_RX_TAKE);                        /* pointer was grabbed and message was interpreted */

            case CMD_F_RETRIGGER:                           /* repeat same received message again, but do not increment the usage cnt */
                return(MSG_RX_BUSY);

            case CMD_F_RX_RETAIN:                           /* message was interpreted, but application */
                return(MSG_RX_TAKE);                        /* retains rx buffer */

            case (CMD_F_RX_RETAIN | CMD_F_RETRIGGER):       /* same message must be repeated, but message has been */
                return(MSG_RX_INC_USAGE_CNT);               /* also retained */

            default:
                break;
        }
    }
    #ifdef CMD_CB2
    else
    {                                                       
        reaction = CmdRxFilter((pTMsgTx)NULL, rx_ptr);      /* callback application to filter message             */
                                                            /*   reaction:  0x03: message was not interpreted and must be retriggered */
                                                            /*              0x01: message was interpreted,  */
        switch(reaction)
        {
            case 0x03:
                return (MSG_RX_BUSY);                       /* force retrigger */

            case 0x01:
                MsgFreeRxMsg(rx_ptr);                       /* release rx message entry, */
                return(MSG_RX_TAKE);                        /* pointer was grabbed and message was interpreted */

            default:
                break;
        }
    }
    #endif

    return(MSG_RX_BUSY);                                    /* force retrigger, since there is no free tx buffer */
}
#endif




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CmdRxInterpreter()                                         */
/* Description : MOST Rx Message Command Interpreter                        */
/*               This interpreter expects a rx message handled by the       */
/*               Application Message Service (AMS)                          */
/*               This interpreter is called only if there is at least       */
/*               one free tx buffer entry.                                  */
/* Parameter(s): ptr on transmit buffer                                     */
/*               ptr on received message                                    */
/*                                                                          */
/* Returns     : flags:                                                     */
/*                          0x00: No retrigger required                     */
/*                          0x01: Retrigger forced, since tx buffer         */
/*                                required but not free                     */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef CMD_2
byte CmdRxInterpreter(pTMsgTx Tx_Ptr, pTMsgRx Rx_Ptr)
{
    byte Tab_Counter;                                                   /* counter while searching for FBlock.InstID */

    byte FBlock_Rx;                                                     /* FBlockID: received value and value from table */
    byte FBlock_Tab;
    byte Inst_Rx;                                                       /* InstID:   received value and value from table */
    byte Inst_Tab;
    word Func_Rx;                                                       /* FuncID:   received value and value from table */
    word Func_Tab;
    byte Op_Rx;                                                         /* OpType:   received value and value from table */
    byte Op_Tab;
    #ifdef CMD_ADD8
    word Len_Rx;
    #endif

    _CONST struct FBlock_L_Type *FBlock_Tab_Ptr;                        /* Ptr on table containing all FBlocks */
    _CONST struct Func_L_Type *Func_Tab_Ptr;                            /* Ptr on table containing all Func_IDs */
    byte *Inst_Tab_Ptr;                                                 /* Ptr on corresponding InstID (RAM) */
    _CONST struct Op_L_Type *Op_Tab_Ptr;                                /* Ptr on table containing all OpTypes */

    #ifndef PTR_UNION_VOID
    #ifdef PTR_FUNCTION_VOID
    void (*Write_Ptr_Tab)(void);                                        /* Ptr for write access */
    void (*Read_Ptr_Tab)(void);                                         /* Ptr for read access */
    #else
    void *Write_Ptr_Tab;                                                /* Ptr for write access */
    void *Read_Ptr_Tab;                                                 /* Ptr for read access */
    #endif
    #endif

    byte Flags;                                                         /* indicates kind of pointer and number of parameters */
    byte Inst_Check;                                                    /* status flags while searching for FBlock.InstID */

    byte CmdRx_RetVal;                                                  /* Returnvalue stores different flags, e.g. flag if application retains rx buffer */
                                                                        /* This value is used to indicate the right return value of MsgRxOutMsg() */
    #ifdef CMD_LEN_CHECK_ZERO
    bool check_len;                                                     /* flag, if the len field must be checked by the */
    #endif                                                              /* command interpreter */

    byte Report;                                                        /* contains kind of report (bit0..3) and flags (bit4..7) */

                                                                        /* bit 0..3: */
                                                                        /* 0:   no report / no error occurred */
                                                                        /* A-E: any report depending on OP_Type */
                                                                        /* F:   error report */

                                                                        /* bit 4: CMD_PASS_ON */
                                                                        /* bit 5: RX_RETAIN */

    bool cmd_repeat_int;                                                /* indicates if a cmd search has to be repeated */

    #ifdef CMD_ADD8
    byte lcFlags;
    byte lcValue;
    #endif

    /*----------------------------- */



    FBlock_Rx = Rx_Ptr->FBlock_ID;                                      /* get FBlock_ID from Rx buffer */
    Inst_Rx   = Rx_Ptr->Inst_ID;                                        /* get Inst_ID from Rx buffer */
    Func_Rx   = Rx_Ptr->Func_ID;                                        /* get Function_ID from Rx buffer */
    Op_Rx     = Rx_Ptr->Operation;                                      /* get Operation from Rx buffer */
    #ifdef CMD_ADD8
    Len_Rx    = Rx_Ptr->Length;
    #endif


    /*----------------------------- */
    /*   Prepare Header for Tx_Msg */
    /*----------------------------- */
    Tx_Ptr->Tgt_Adr     = Rx_Ptr->Src_Adr;
    Tx_Ptr->FBlock_ID   = FBlock_Rx;
    Tx_Ptr->Func_ID     = Func_Rx;

    if (FBLOCK_NETBLOCK == FBlock_Rx)                                   /* InstID of FBlock NetBlock */
    {
        Tx_Ptr->Inst_ID = MOST_GET_NODE_POS();                          /* has to be the NodePosition */
    }
    else                                                                /* when answering the report message */
    {
        Tx_Ptr->Inst_ID = Inst_Rx;
    }

    #ifdef CTRL_FILTER_ID
    Tx_Ptr->Filter_ID   = Rx_Ptr->Filter_ID;
    #endif


    Report          = (byte)0;                                          /* clear report status */
    CmdRx_RetVal    = CMD_F_OK;                                         /* clear all flags that will be returned to MsgRxOutMsg */

    cmd_repeat_int  = MNS_FALSE;                                            /* init internal flag */

    Inst_Check = ERR_FBLOCKID;                                          /* set search flags */
                                                                        /*   1 = FBlock found not yet */
                                                                        /*   2 = FBlock found, but Inst found not yet */
                                                                        /*   0 = FBlock.InstID ok */


    if (   ((FBLOCK_ALL == FBlock_Rx) || (INST_ALL == Inst_Rx))         /* if the InstanceID is 0xFF and we are not looking at this */
        &&  (Cmd_FBlock_Tab_Ptr_Holder) )                               /* message for the first time enter where we left last time */
    {
        FBlock_Tab_Ptr = Cmd_FBlock_Tab_Ptr_Holder;
        Tab_Counter = Cmd_FBlock_Position_Holder;  /* when reexamining the message we also need to make sure that Tab_Counter is properly decremented. */
    }
    else    /* origin behaviour: */
    {
        FBlock_Tab_Ptr = FBlocks;                                           /* init pointer on FBlock_ID table */

        if (Op_Rx >= OP_REPORTS)
        {
            Tab_Counter = CMD_SIZE_FBLOCK_TABLE;                            /* load counter with number of table entries */
        }
        else
        {
            Tab_Counter = CMD_SIZE_FBLOCK_TABLE - (byte)NUM_FBLOCKS_SHADOW;     /* load counter, but forget about FBlock Shadows */
        }
    }




    /*-------------------------- */
    /*   Check: FBlockID.InstID */
    /*-------------------------- */
    if (Tab_Counter)
    {
        do
        {
            FBlock_Tab = FBlock_Tab_Ptr->FBlock;                            /* read FBlock_ID from table (Romread) */

            if (    ( (FBLOCK_ALL       == FBlock_Rx)                       /* FBLOCK_ALL addresses all FBlocks but not      */
                   && (FBLOCK_NETBLOCK  != FBlock_Tab)                      /* the NetBlock and no supplier specific FBlocks */
                   && ((byte)0xF0       >  FBlock_Tab) )
                 ||   (FBlock_Tab       == FBlock_Rx) )
            {
                Tx_Ptr->FBlock_ID = FBlock_Tab;                             /* set correct FBlockID for response message */
                if ((byte)0 == Inst_Rx)                                     /* InstID == 0x00 ? */
                {                                                           /* Received InstID 0x00 is treated as don't care */
                    if (FBlock_Tab_Ptr->Inst_Tab_Ptr)
                    {
                        Tx_Ptr->Inst_ID = *(FBlock_Tab_Ptr->Inst_Tab_Ptr);  /* then prepare own InstID for reponse message, if valid InstID stored in table */
                    }
                    Inst_Check = (byte)0;                                   /* -> FBlock.InstID ok. */
                }
                else if (INST_ALL == Inst_Rx)                               /* otherwise check InstID */
                /* if InstID is 0xFF and the FBlockID already checks out then save the correct InstID in Tx_Ptr and set Inst_Check to 0 to leave the loop */
                {
                    if (FBlock_Tab_Ptr->Inst_Tab_Ptr)
                    {
                        Tx_Ptr->Inst_ID = *(FBlock_Tab_Ptr->Inst_Tab_Ptr);
                    }
                    else if (FBlock_Tab_Ptr->FBlock != FBLOCK_NETBLOCK)
                    {
                        Tx_Ptr->Inst_ID = (byte)0x00;                       /* Set InstID (TX) to zero, but only when FBlockID not ID of NetBlock */
                    }

                    Inst_Check = (byte)0;
                }
                else
                {
                    Inst_Tab_Ptr = FBlock_Tab_Ptr->Inst_Tab_Ptr;            /* (Romread) */

                    if (Inst_Tab_Ptr)
                    {
                        Inst_Tab = *(Inst_Tab_Ptr);                         /* read Inst_ID from RAM table */

                        if (Inst_Tab == Inst_Rx)                            /* InstID ok ? */
                        {
                            Inst_Check = (byte)0;                           /* -> FBlock.InstID ok. */
                        }
                        else
                        {
                            Inst_Check = ERR_INSTID;                        /* -> FBlock ok, but InstID failed */
                        }
                    }
                    else
                    {                                                       /* No Inst_ID available (eg. NetBlock) */
                        Inst_Check = (byte)0;
                    }
                }
            }

            if (Inst_Check)                                                 /* if FBlock.InstID found not yet */
            {
                FBlock_Tab_Ptr++;
                Tab_Counter--;
            }

        }while (((byte)0 != Inst_Check) && ((byte)0 != Tab_Counter));       /* repeat until required FBlock.InstID has been found */
                                                                            /* or end of table has been reached */
    }

    if ((INST_ALL == Inst_Rx) ||(FBLOCK_ALL == FBlock_Rx))      /* Application groupcast ? */
    {
        if ((byte)0 == Tab_Counter)                 /* this indicates that we finished searching the FBlock table without finding the sought FBlock */
        {
            Cmd_FBlock_Position_Holder = (byte)0;   /* so Cmd_FBlock_Position_Holder needs to be reset to 0 */
            if (NULL != Cmd_FBlock_Tab_Ptr_Holder)  /* if Cmd_FBlock_Tab_Ptr_Holder is not NULL then we found a matching FBlock on a previous run of the CmdRxInterpreter */
            {
                Cmd_FBlock_Tab_Ptr_Holder =  NULL;  /* so this value also needs to be reset to NULL */
            }
            /* if no FBlocks are found and this was the first time through the table, then not setting Inst_Check to 0 will cause a search for FBlock_Shaddows */
        }
        else  /* otherwise we found a matching FBlock, but we're not done searching the table */
        {
            Cmd_FBlock_Tab_Ptr_Holder  = FBlock_Tab_Ptr;           /* save the current position */
            Cmd_FBlock_Position_Holder = Tab_Counter;              /* save the current Tab_Counter */
            cmd_repeat_int             = MNS_TRUE;
        }
    }



    if ((byte)0 != Inst_Check)                                     /* if corresponding FBlockID.InstID not found */
    {
        if (Op_Rx >= OP_REPORTS)                                    /* Unknown FBlock Shadow ? */
        {                                                           /* --> Call Application and get result */
            Report = CmdUnknownFBlockShadow(Tx_Ptr, Rx_Ptr);

            CmdRx_RetVal |= (byte)(Report & CMD_F_MASK);                          /* save  CMD_RX_RETAIN and CMD_RX_REPEAT flag */
            Report &= (byte)(~CMD_F_MASK);                                          /* clear CMD_RX_RETAIN and CMD_RX_REPEAT flag */


            if ((byte)0 == Report )
            {
                MsgTxUnused(Tx_Ptr);                                        /* Set tx buffer line free since there is no message to send */
                return(CmdRx_RetVal);
            }
        }
        else /* Op_Rx < OP_REPORTS */
        {
            (void)CmdErrorMsg(Tx_Ptr, Inst_Check);
            Report = OP_ERROR;
        }
    }

    /*------------------------------------------------ */

    if ((byte)0 == Report)                                        /* if no error occurred */
    {
        Func_Tab_Ptr = FBlock_Tab_Ptr->Func_Tab_Ptr;                    /* read pointer on Function_ID table (Romread) */

        /*------------------------ */
        /*   Check: Function ID */
        /*------------------------ */
        do
        {
            Func_Tab = Func_Tab_Ptr->Func;                              /* read Function_ID from table (Romread) */

            Func_Tab_Ptr++;

            if (FUNC_TERMINATION == Func_Tab)                           /* end of table ? */
            {
                (void)CmdErrorMsg(Tx_Ptr, ERR_FKTID);                   /* FuncID not found */
                Report = OP_ERROR;
            }
        }while ((Func_Tab != Func_Rx) && ((byte)0 == Report));          /* repeat until required FuncID has been found */
                                                                        /* or end of table has been reached */
    /*------------------------------------------------ */


        if ((byte)0 == Report)                                              /* if no error occurred */
        {
            Func_Tab_Ptr--;

            Op_Tab_Ptr = Func_Tab_Ptr->Op_Tab_Ptr;                          /* read pointer on Operation-Type table (Romread) */

            /*------------------------ */
            /*   Check: Operation Type */
            /*------------------------ */
            do
            {
                Op_Tab = Op_Tab_Ptr->Operation;                             /* read Operation-Type from table (Romread) */

                Op_Tab_Ptr++;

                if (OP_TERMINATION == Op_Tab)                               /* end of table ? */
                {
                    (void)CmdErrorByte(Tx_Ptr, ERR_OPTYPE, Op_Rx);          /* OpType not found */
                    Report = OP_ERROR;
                }

            }while ((Op_Tab != Op_Rx) && ((byte)0 == Report));              /* repeat until required OpType has been found */
                                                                            /* or end of table has been reached */


            /*------------------------------------------------ */
            #ifdef CMD_ADD8
            if ((byte)0 == Report)                                          /* if no error occurred */
            {
                Op_Tab_Ptr--;

                lcFlags = (Op_Tab_Ptr->LengthCheck) >> LC_SHIFT;            /* read flags from table (Romread) */
                lcValue = (Op_Tab_Ptr->LengthCheck) &  LC_MASK;

                switch (lcFlags)
                {
                    case CMD_LEN_EQ:                                        /* equal */
                        if (Len_Rx != lcValue)
                        {
                            Report = OP_ERROR;
                        }
                        break;

                    case CMD_LEN_LE:                                        /* less or equal */
                        if (Len_Rx > lcValue)
                        {
                            Report = OP_ERROR;
                        }
                        break;

                    case CMD_LEN_GE:                                        /* less or equal */
                        if (Len_Rx < lcValue)
                        {
                            Report = OP_ERROR;
                        }
                        break;

                    default:                                                /* no check */
                        break;
                }

                if (OP_ERROR == Report)
                {
                    CmdErrorMsg(Tx_Ptr, ERR_LENGTH);                        /* create error message */
                }
             }
             #endif


            /*------------------------ */

            if ((byte)0 == Report)                                          /* if no error occurred */
            {
                #ifndef CMD_ADD8
                Op_Tab_Ptr--;
                #endif

                Flags = Op_Tab_Ptr->Flags;                                  /* read flags from table (Romread) */

                /*------------------------------- */
                /*   Set Property / Start Method */
                /*------------------------------- */
                if (Flags & OP_W_MASK)                                      /* write access required ? */
                {
                    #ifndef PTR_UNION_VOID
                    Write_Ptr_Tab = Op_Tab_Ptr->Write_Ptr;                          /* read pointer for write access (Romread) */
                    #endif

                    #ifdef CMD_LEN_CHECK_ZERO
                    check_len = MNS_FALSE;                                              /* perform no length check */
                    #endif

                    switch (Flags & OP_W_MASK)                                      /* switch by kind of function call */
                    {
                        case OP_W_IND_TXRX:                                         /* individual function, parameters: Tx_Ptr, Rx_Ptr */

                            #ifdef PTR_UNION_VOID
                            Report = ( *(byte(*)(pTMsgTx, pTMsgRx)) Op_Tab_Ptr->Write.FPtr ) (Tx_Ptr, Rx_Ptr);
                            #else
                            Report = ( *(byte(*)(pTMsgTx, pTMsgRx)) Write_Ptr_Tab ) (Tx_Ptr, Rx_Ptr);
                            #endif
                            break;

                        case OP_W_IND_NONE:                                         /* individual function, parameter: none */

                            #ifdef PTR_UNION_VOID
                            Report = ( *(byte(*)(void)) Op_Tab_Ptr->Write.FPtr ) ();
                            #else
                            Report = ( *(byte(*)(void)) Write_Ptr_Tab ) ();
                            #endif
                            break;

                        case OP_W_IND_RX:                                           /* individual function, parameter: Rx_Ptr */

                            #ifdef PTR_UNION_VOID
                            Report = ( *(byte(*)(pTMsgRx)) Op_Tab_Ptr->Write.FPtr ) (Rx_Ptr);
                            #else
                            Report = ( *(byte(*)(pTMsgRx)) Write_Ptr_Tab ) (Rx_Ptr);
                            #endif
                            break;

                        case OP_W_IND_TX:                                           /* individual function, parameter: Tx_Ptr */

                            #ifdef PTR_UNION_VOID
                            Report = ( *(byte(*)(pTMsgTx)) Op_Tab_Ptr->Write.FPtr ) (Tx_Ptr);
                            #else
                            Report = ( *(byte(*)(pTMsgTx)) Write_Ptr_Tab ) (Tx_Ptr);
                            #endif
                            break;

                        #if (defined MHP_R4) && (defined NS_CMD_MHP)
                        case OP_W_MHP_EXT:

                            #ifdef PTR_UNION_VOID
                            MhpSetBufIntf(Flags & (byte)0x0F,(void*)(Op_Tab_Ptr->Write.FPtr),(word)(Op_Tab_Ptr->Read.DPtr));
                            #else
                            MhpSetBufIntf(Flags & (byte)0x0F,(void*)Write_Ptr_Tab,(word)(int)(Op_Tab_Ptr->Read_Ptr));
                            #endif
                            break;                                                  /* call MOST High Protocol Service */
                        #endif

                    }

                    CmdRx_RetVal |= (byte)(Report & CMD_F_MASK);                          /* save  CMD_RX_RETAIN and CMD_RX_REPEAT flag */
                    Report &= (byte)(~CMD_F_MASK);                                          /* clear CMD_RX_RETAIN and CMD_RX_REPEAT flag */

                }
                #ifdef CMD_LEN_CHECK_ZERO
                else
                {
                    check_len = MNS_TRUE;                                               /* perform length check before read access, */
                                                                                    /* since there was no write access */
                }
                #endif


                /*--------------------------------------- */
                /*   Report Property / Result / Interface */
                /*--------------------------------------- */
                if (Flags >= OP_W_MHP_EXT)                   /* Does Flags sign a Mhp entry ? */
                {
                    #ifdef NS_CMD_MHP
                    /* Forget about read access, if flags sign a Mhp entry. */
                    /* It was already completed in write access procedure, by calling MhpSetBufIntf(). */
                    #else
                    /* Message is ignored, since the NS_CMD_MHP interface is not enabled. */
                    /* The message is therefore not serviced correctly.  */
                    /* If MHP entries are used in the Operation Tables of the CMD module, */
                    /* the respective interface (NS_CMD_MHP) must be enabled ! */
                    #endif
                }
                else if (((byte)0 == Report) && (Flags & OP_R_MASK) )    /* if no error occurred and reading access required */
                {
                    #ifdef CMD_LEN_CHECK_ZERO
                    if (MNS_FALSE != check_len)
                    {
                        if ( (Flags&OP_R_MASK) != OP_R_IND_TXRX )
                        {
                            if (Rx_Ptr->Length != (word)0)
                                Report = CmdErrorMsg(Tx_Ptr, ERR_LENGTH);
                        }
                    }
                    if ((byte)0 == Report)
                    #endif
                    {
                        #ifndef PTR_UNION_VOID
                        Read_Ptr_Tab = Op_Tab_Ptr->Read_Ptr;                        /* read pointer for read access (Romread) */
                        #endif

                        switch (Flags & OP_R_MASK)                                  /* switch by kind of reading access */
                        {
                            case OP_R_IND_TX:                                       /* individual function, parameter: Tx_Ptr */

                                #ifdef PTR_UNION_VOID
                                Report = ( *(byte(*)(pTMsgTx)) Op_Tab_Ptr->Read.FPtr ) (Tx_Ptr);
                                #else
                                Report = ( *(byte(*)(pTMsgTx)) Read_Ptr_Tab ) (Tx_Ptr);
                                #endif
                                break;

                            case OP_R_IND_TXRX:                                     /* individual function, parameters: Tx_Ptr, Rx_Ptr */

                                #ifdef PTR_UNION_VOID
                                Report = ( *(byte(*)(pTMsgTx, pTMsgRx)) Op_Tab_Ptr->Read.FPtr ) (Tx_Ptr, Rx_Ptr);
                                #else
                                Report = ( *(byte(*)(pTMsgTx, pTMsgRx)) Read_Ptr_Tab ) (Tx_Ptr, Rx_Ptr);
                                #endif
                                break;

                            case OP_R_BYTE:                                         /* read access on Byte type */

                                #ifdef PTR_UNION_VOID
                                Tx_Ptr->Data[0] = *(byte*)Op_Tab_Ptr->Read.DPtr;
                                #else
                                Tx_Ptr->Data[0] = *(byte*)Read_Ptr_Tab;
                                #endif
                                Tx_Ptr->Length  = (word)1;
                                Report          = OP_STATUS;
                                break;

                            case OP_R_BOOL:                                         /* read access on Bool type */

                                #ifdef PTR_UNION_VOID
                                Tx_Ptr->Data[0] = (byte)(*(bool*)Op_Tab_Ptr->Read.DPtr);
                                #else
                                Tx_Ptr->Data[0] = (byte)(*(bool*)Read_Ptr_Tab);
                                #endif
                                Tx_Ptr->Length  = (word)1;
                                Report          = OP_STATUS;
                                break;

                            case OP_R_WORD:                                         /* read access on Word type */

                                #ifdef PTR_UNION_VOID
                                CmdEncodeWord(&Tx_Ptr->Data[0], (word *)Op_Tab_Ptr->Read.DPtr);
                                #else
                                CmdEncodeWord(&Tx_Ptr->Data[0], (word *)Read_Ptr_Tab);
                                #endif
                                Tx_Ptr->Length  = (word)2;
                                Report          = OP_STATUS;
                                break;

                            case OP_R_LONG:                                         /* read access on DWord type */

                                #ifdef PTR_UNION_VOID
                                CmdEncodeLong(&Tx_Ptr->Data[0], (dword *)Op_Tab_Ptr->Read.DPtr);
                                #else
                                CmdEncodeLong(&Tx_Ptr->Data[0], (dword *)Read_Ptr_Tab);
                                #endif
                                Tx_Ptr->Length  = (word)4;
                                Report          = OP_STATUS;
                                break;

                            case OP_R_INTF:                                         /* read access on String type */

                                #ifdef PTR_UNION_VOID
                                CmdEncodeStrRom(Tx_Ptr, (byte *)Op_Tab_Ptr->Read.DPtr);
                                #else
                                CmdEncodeStrRom(Tx_Ptr, (byte *)Read_Ptr_Tab);
                                #endif
                                Report = OP_INTERFACE;
                                break;

                        }  /* switch (Flags & OP_R_MASK) */

                        CmdRx_RetVal |= (byte)(Report & CMD_F_MASK);                      /* save  CMD_RX_RETAIN and CMD_RX_REPEAT flag */
                        Report &= (byte)(~CMD_F_MASK);                                      /* clear CMD_RX_RETAIN and CMD_RX_REPEAT flag */


                    }  /* if ((byte)0 == Report) */

                }  /* else if (((byte)0 == Report) && (Flags & OP_R_MASK) )  */
            }
        }
    }

    /*------------------------------- */
    /*  Complete Tx Message */
    /*------------------------------- */
    if ( (byte)0 == (Report & CMD_TX_RETAIN) )          /* Flag not set, which signs that tx message will be completed by application ? */
    {
        if ( (Report & (byte)0xF) == OP_ERROR)          /* Is the response an error message ? */
        {
            if (!(Report & CMD_PASS))                   /* Avoid all error message on following conditions, but only if not passed through: */
            {
                if (   (MSG_RCV_TYPE_BROADCAST == Rx_Ptr->Rcv_Type)     /* BroadcastMessage ? */
                    || (MSG_RCV_TYPE_GROUPCAST == Rx_Ptr->Rcv_Type)     /* GroupcastMessage ? */
                    || (FBLOCK_ALL             == FBlock_Rx)            /* GroupcastMessage on application level ? */
                    || (INST_ALL               == Inst_Rx) )
                {
                    Report = OP_NO_REPORT;
                }
            }
        }   /* if 'error message' */

        if (Report && ( (Op_Rx<OP_REPORTS)||(Report&CMD_PASS) ) )   /* Send message (answer, result, error or message passed on) */
        {                                                           /* but no error report on a report message */
            Tx_Ptr->Operation   = Report & (byte)0xF;             /* Forget about Flags (bits 4..7) */

            CmdAutoInsertSenderHandlePtr(Tx_Ptr, Rx_Ptr);           /* insert sender handle automatically if necessary */

            if ( CmdTxFilter(Tx_Ptr, Rx_Ptr) )       /* Callback to view result message */
            {
                AmsMsgSend(Tx_Ptr);
            }
            else
            {
                MsgTxUnused(Tx_Ptr);
            }
        }
        else
        {
            CmdTxFilter((pTMsgTx)NULL, Rx_Ptr);     /* No result message -> Callback with NULL-Pointer and "rx ptr" */

            CmdTxNoResult(Rx_Ptr);                  /* No result message -> Callback only with "rx ptr" */

            MsgTxUnused(Tx_Ptr);                    /* Set tx buffer line free since there is no message to send */
        }
    }
    else
    {
        CmdTxFilter((pTMsgTx)NULL, Rx_Ptr);         /* result message not yet. Has been retained by application */
                                                    /*  -> Callback with NULL-Pointer and "rx ptr" */
    }


    if ( ((byte)0    == (CmdRx_RetVal & CMD_F_RETRIGGER)) &&
         (MNS_FALSE != cmd_repeat_int) )      /* if a repeat was called by the application, then don't increment the position holders */
    {                                       /* otherwise if a repeat is needed by the Command Interperter alone, then set the repeat flag and increment the position holders */
        CmdRx_RetVal |= CMD_F_RETRIGGER;    /* set the global repeat flag */
        Cmd_FBlock_Tab_Ptr_Holder++;        /* increment it so we start with the next position in the table */
        Cmd_FBlock_Position_Holder --;      /* decrement it so we adjust for are above increment */
    }

    return(CmdRx_RetVal);
}
#endif






/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CmdEncodeWord()                                            */
/* Description : copy a 16 bit Word into tx buffer                          */
/* Parameter(s): ptr on target buffer, ptr on source variable               */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef CMD_3
void CmdEncodeWord(byte *ptr_tgt, word *ptr_src)
{
    *ptr_tgt++  = (byte)(*ptr_src >> 8);            /* prepare HB */
    *ptr_tgt    = (byte)(*ptr_src & (word)0xFF);    /* prepare LB */
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CmdDecodeWord()                                            */
/* Description : copy a 16 bit Word from rx buffer to a variable            */
/* Parameter(s): ptr on target variable, ptr on source buffer               */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef CMD_4
void CmdDecodeWord(word *ptr_tgt, byte *ptr_src)
{
    *ptr_tgt    = (word)((word)(*ptr_src++) << 8);  /* prepare HB */
    *ptr_tgt   += (*ptr_src);                       /* prepare LB */
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CmdEncodeLong()                                            */
/* Description : copy a 32 bit DWord into tx buffer                         */
/* Parameter(s): ptr on target buffer, ptr on source variable               */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef CMD_9
void CmdEncodeLong(byte *ptr_tgt, dword *ptr_src)
{
    word  high_part;
    word  low_part;
    dword help;
    byte i;

    help = *ptr_src;

    for (i=(byte)0;i<(byte)16;i++)
    {
        help = help >> 1;
    }
    high_part = (word)help;

    CmdEncodeWord(ptr_tgt,&high_part);

    ptr_tgt += 2;
    low_part = (word)(*ptr_src);

    CmdEncodeWord(ptr_tgt,&low_part);
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CmdDecodeLong()                                            */
/* Description : copy a 32 bit DWord from rx buffer to a variable           */
/* Parameter(s): ptr on target variable, ptr on source buffer               */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef CMD_10
void CmdDecodeLong(dword *ptr_tgt, byte *ptr_src)
{
    word high_part;
    word low_part;
    dword help;
    byte i;

    CmdDecodeWord(&high_part, ptr_src);
    ptr_src += 2;
    CmdDecodeWord(&low_part, ptr_src);

    help = (dword)high_part;

    for (i=(byte)0;i<(byte)16;i++)
    {
        help = help << 1;
    }

    *ptr_tgt = help | (dword)low_part;
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CmdErrorMsg()                                              */
/* Description : Prepare data field and length of a STANDARD error message  */
/* Parameter(s): ptr on tx message buffer, Error ID                         */
/*               Can be used if NO parameters have to send back             */
/* Returns     : OpType                                                     */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef CMD_5
byte CmdErrorMsg(pTMsgTx Tx_Ptr, byte ErrorID)
{
    Tx_Ptr->Data[0] = ErrorID;
    Tx_Ptr->Length  = (word)1;
    return(OP_ERROR);
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CmdErrorByte()                                             */
/* Description : Prepare data field and length of an error message          */
/*               which will transport one parameter of type Byte            */
/* Parameter(s): ptr on tx message buffer, Error ID, parameter              */
/* Returns     : OpType                                                     */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef CMD_6
byte CmdErrorByte(pTMsgTx Tx_Ptr, byte ErrorID, byte param)
{
    Tx_Ptr->Data[0] = ErrorID;
    Tx_Ptr->Data[1] = param;
    Tx_Ptr->Length  = (word)2;
    return(OP_ERROR);
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CmdErrorParamAll()                                         */
/* Description : Prepare data field and length of an error message          */
/*               which will transport back all parameters                   */
/* Parameter(s): ptr on tx message buffer, Error ID, parameter              */
/* Returns     : OpType                                                     */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef CMD_12
byte CmdErrorParamAll(pTMsgTx tx_ptr, pTMsgRx rx_ptr, byte error_id)
{
    byte* dest_ptr;
    byte* src_ptr;
    word  rx_length;
    word  i;

    dest_ptr = &tx_ptr->Data[0];
    src_ptr  = &rx_ptr->Data[0];

    rx_length = rx_ptr->Length;

    *dest_ptr++ = error_id;                             /* Data[0] = ErrorCode */
    for (i=(word)0;i<rx_length;i++)
    {
        *dest_ptr++ = *src_ptr++;                       /* Data[1]..[n] = all received parameters */
    }

    tx_ptr->Length = rx_length + (word)1;

    return(OP_ERROR);
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CmdErrorParamWrong()                                       */
/* Description : Prepare data field and length of an error message          */
/*               whenever a parameter was wrong                             */
/* Parameter(s): pointer at tx message buffer, index of parameter,          */
/*               pointer at first Byte of parameter, number of bytes        */
/* Returns     : OpType                                                     */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef CMD_14
byte CmdErrorParamWrong(pTMsgTx tx_ptr, byte index, byte* param_ptr, byte num)
{
    #if (MAX_MSG_TX_DATA < 256)
    if (num > (byte)(MAX_MSG_TX_DATA-2))
    {
        return(OP_ERROR);
    }
    #endif

    tx_ptr->Data[0] = ERR_PARAM_WRONG;                              /* Data[0] */

    return( CmdErrorParam_W_NA(tx_ptr, index, param_ptr, num) );    /* prepare Data[1] .. Data[num+1] and length */
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CmdErrorParamNotAvailable()                                */
/* Description : Prepare data field and length of an error message          */
/*               whenever a parameter is not available                      */
/* Parameter(s): pointer at tx message buffer, index of parameter,          */
/*               pointer at first Byte of parameter, number of bytes        */
/* Returns     : OpType                                                     */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef CMD_15
byte CmdErrorParamNotAvailable(pTMsgTx tx_ptr, byte index, byte* param_ptr, byte num)
{
    #if (MAX_MSG_TX_DATA < 256)
    if (num > (byte)(MAX_MSG_TX_DATA-2))
    {
        return(OP_ERROR);
    }
    #endif

    tx_ptr->Data[0] = ERR_PARAM_NOTAVAILABLE;                       /* Data[0] */

    return ( CmdErrorParam_W_NA(tx_ptr, index, param_ptr, num) );   /* prepare Data[1] .. Data[num+1] and length */
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CmdErrorParam_W_NA()                                       */
/* Description :                                                            */
/* Parameter(s): pointer at tx message buffer, index of parameter,          */
/*               pointer at first Byte of parameter, number of bytes        */
/* Returns     : OpType                                                     */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef CMD_16
byte CmdErrorParam_W_NA(pTMsgTx tx_ptr, byte index, byte* param_ptr, byte num)
{
    byte* dest_ptr;
    byte i;

    dest_ptr = &tx_ptr->Data[1];

    *dest_ptr++ = index;                /* Data[1] */

    for (i=(byte)0;i<num;i++)                 /* Data[2] .. Data[num+1] */
    {
        *dest_ptr++ = *param_ptr++;
    }

    tx_ptr->Length = (word)2 + (word)num;

    return(OP_ERROR);
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CmdEncodeStrRom()                                          */
/* Description : Copy a string or interface description from ROM            */
/*               to Tx buffer                                               */
/*               Stringformat:                                              */
/*               HB of length, LB of Length, Data[0].... Data[n]            */
/* Parameter(s): ptr on Tx buf, ptr on string/interface description         */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef CMD_8
void CmdEncodeStrRom(pTMsgTx Tx_Ptr, const byte *SrcPtr)
{
    byte *DestPtr;
    word Length;


    Length = (word)*SrcPtr++;           /* Length HB (Romread) */
    Length = (word)(Length << 8);
    Length += (word)*SrcPtr++;          /* Length LB */


    Tx_Ptr->Length = (word)Length;
    DestPtr = &Tx_Ptr->Data[0];


    while (Length)
    {
        *DestPtr++ = *SrcPtr++;         /* (Romread) */
        Length--;
    }
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CmdEncodeStrRam()                                          */
/* Description : Copy a string or interface description from RAM            */
/*               to Tx buffer                                               */
/*               Stringformat:                                              */
/*               HB of length, LB of Length, Data[0].... Data[n]            */
/* Parameter(s): ptr on Tx buf, ptr on string/interface description         */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef CMD_13
void CmdEncodeStrRam(pTMsgTx Tx_Ptr, byte *SrcPtr)
{
    byte *DestPtr;
    word Length;


    Length = (word)*SrcPtr++;           /* Length HB */
    Length = (word)(Length << 8);
    Length += (word)*SrcPtr++;          /* Length LB */


    Tx_Ptr->Length = (word)Length;
    DestPtr = &Tx_Ptr->Data[0];


    while (Length)
    {
        *DestPtr++ = *SrcPtr++;
        Length--;
    }
}
#endif






/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CmdCopyTxRx()                                              */
/* Description : Copy the length and data field from a received message     */
/*               to the message entry reserved for the tx message           */
/* Parameter(s): ptr at tx message, ptr at rx message                       */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef CMD_11
void CmdCopyTxRx(pTMsgTx tx_ptr, pTMsgRx rx_ptr)
{
    byte* dest_ptr;
    byte* src_ptr;
    word  rx_length;
    word  i;

    dest_ptr = &tx_ptr->Data[0];
    src_ptr  = &rx_ptr->Data[0];

    rx_length = rx_ptr->Length;

    for (i=(word)0;i<rx_length;i++)
    {
        *dest_ptr++ = *src_ptr++;
    }

    tx_ptr->Length = rx_length;
}
#endif




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CmdInsertSenderHandle()                                    */
/* Description : Insert a sender handle in an existing report message       */
/*               which must be returned caused by an StartAck operation     */
/*                                                                          */
/* Parameter(s): ptr at tx message without handle, HB of handle,            */
/*               LB of handle                                               */
/* Returns     : 0x00: ok                                                   */
/*               0x01: handle not copied, since buffer too small            */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef CMD_20
byte CmdInsertSenderHandle(pTMsgTx tx_ptr, byte handle_hb, byte handle_lb)
{
    byte* dest_ptr;
    byte* src_ptr;
    word length;
    word i;

    length   = tx_ptr->Length;

    if (length > ((word)MAX_MSG_TX_DATA - (word)2))
    {
        return((byte)1);                          /* Error:  Buffer too small */
    }

    if (length)
    {
        src_ptr  = &tx_ptr->Data[length-(word)1];
        dest_ptr = src_ptr + 2;

        for (i=(word)0;i<length;i++)
        {
            *dest_ptr-- = *src_ptr--;
        }
    }
    else
    {
        dest_ptr = &tx_ptr->Data[1];
    }

    *dest_ptr-- = handle_lb;
    *dest_ptr   = handle_hb;

    tx_ptr->Length = length + (word)2;

    return((byte)0);
}
#endif

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CmdInsertSenderHandlePtr()                                 */
/* Description : Insert a sender handle in an existing report message       */
/*               which must be returned caused by an StartAck operation     */
/*                                                                          */
/* Parameter(s): ptr at tx message without handle,                          */
/*               ptr at rx message which contains the received sender handle*/
/*                                                                          */
/* Returns     : 0x00: ok                                                   */
/*               0x01: handle not copied, since buffer too small            */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef CMD_21
byte CmdInsertSenderHandlePtr(pTMsgTx tx_ptr, pTMsgRx rx_ptr)
{
    return( CmdInsertSenderHandle( tx_ptr, rx_ptr->Data[0], rx_ptr->Data[1] ) );
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CmdAutoInsertSenderHandlePtr()                             */
/* Description : Insert the sender handle automatically if necessary        */
/*                                                                          */
/* Parameter(s): ptr at tx message                                          */
/*               ptr at rx message                                          */
/*                                                                          */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef CMD_22
void CmdAutoInsertSenderHandlePtr(pTMsgTx tx_ptr, pTMsgRx rx_ptr)
{
    byte op_tx;             /* operation type of tx message */
    byte op_rx;             /* operation type of received message */
    bool ins_handle;        /* insert sender handle ? */

    op_rx = rx_ptr->Operation;

    if  (                                       /* insert the sender handle only, in case of... */
          ( (OP_STARTACK       == op_rx)        /* a) Operation StartAck */
         || (OP_ABORTACK       == op_rx)        /* b) Operation AbortAck */
         || (OP_STARTRESULTACK == op_rx) )      /* c) Operation StartResultAck */
         && ((word)2  <= rx_ptr->Length)        /* check if rx message contains a sender handle */
        )
    {
        op_tx = tx_ptr->Operation;
        ins_handle = MNS_FALSE;

        switch (op_tx)                          /* check for operation type of tx msg */
        {
            case OP_ERROR:                      /* ERROR -> ERROR ACK */
                op_tx = OP_ERRORACK;
                ins_handle = MNS_TRUE;
                break;

            case OP_PROCESSING:                 /* PROCESSING -> PROCESSING ACK */
                op_tx = OP_PROCESSINGACK;
                ins_handle = MNS_TRUE;
                break;

            case OP_RESULT:                     /* RESULT -> RESULT ACK */
                op_tx = OP_RESULTACK;
                ins_handle = MNS_TRUE;
                break;
        }

        if (MNS_FALSE != ins_handle)
        {
            (void)CmdInsertSenderHandlePtr(tx_ptr, rx_ptr);     /* copy sender handle from rx msg to tx msg */
            tx_ptr->Operation = op_tx;                          /* write new operation type of tx msg */
        }
    }
}
#endif

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CmdGetFunctionIds()                                        */
/* Description : Write Id-List in RLE format into the Msg structure.        */
/*                                                                          */
/* Parameter(s): Address of Tx-Structure and address of FBlock              */
/*                                                                          */
/* Returns     : OP_STATUS or OP_ERROR                                      */
/*                                                                          */
/* Note        : The function table requires to be sorted in                */
/*               ascending order for CmdGetFunctionIds() to work.           */
/*                                                                          */
/*--------------------------------------------------------------------------*/

#ifdef CMD_23
byte CmdGetFunctionIds( pTMsgTx tx_ptr, pTFuncL func_ptr )
{
    byte position;
    bool changed;
    byte tempValue;
    byte tempByte;
    bool overflow;
    word lastFunc;
    word i;
    word counter;                       /* Start with notification              */
    bool presenceStatus;                /* According to RLE format starts list  */
                                        /* with presentation status 1           */


    func_ptr++;

    lastFunc        = (word)0;
    i               = (word)0;
    counter         = (word)0x0001;
    presenceStatus  = MNS_TRUE;
    tempValue       = (byte)0x00;
    tempByte        = (byte)0x00;
    position        = CMD_EVEN_POS;
    overflow        = MNS_FALSE;

                                        /* Read all functions from list until */
                                        /* the termination mark is visible    */



    /* search till end of table (0xFFFF) or till first supplier specific function found */
    while( (func_ptr->Func < (word)0xF00) && (MNS_FALSE == overflow) )
    {
                                    /* RLE-Format-> only changes are visible  */
        changed = MNS_FALSE;
        if(func_ptr->Func < counter)
        {
            /* table not sorted ! */
            return(CmdErrorMsg(tx_ptr, ERR_DEVICE_MALFUNC));
        }
        else if( (func_ptr->Func == counter) && (MNS_FALSE == presenceStatus))
        {
            presenceStatus = MNS_TRUE;
            changed = MNS_TRUE;
        }
        else if( (func_ptr->Func != counter) && (MNS_FALSE != presenceStatus))
        {
            presenceStatus = MNS_FALSE;
            changed = MNS_TRUE;
        }

        if( MNS_FALSE != changed )
        {
            switch(position)
            {
                case CMD_EVEN_POS:
                                      /* 8 of 12 bit can be written and the   */
                                      /* last 4 bit must be stored until the  */
                                      /* next function Id                     */
                    tempValue = (byte)(counter >> 4);
                    tempByte  = (byte)((word)(counter << 4));
                    if (i < (word)MAX_MSG_TX_DATA)
                    {
                        tx_ptr->Data[i] = tempValue;
                        i++;
                    }
                    else
                    {
                        overflow = MNS_TRUE;
                    }

                    position = CMD_ODD_POS;
                    break;

                case CMD_ODD_POS:
                                        /* 4 Bit of former function plus      */
                                        /* 4 Bit of the current one. At least */
                                        /* the 8 Bit LSB in the next pos.     */
                    tempValue = (byte)(counter >> 8);
                    tempByte |= tempValue;
                    tempValue = (byte)counter;
                    if (i < (word)MAX_MSG_TX_DATA)
                    {
                        tx_ptr->Data[i] = tempByte;
                        i++;
                    }
                    else
                    {
                        overflow = MNS_TRUE;
                    }

                    if (i < (word)MAX_MSG_TX_DATA)
                    {
                        tx_ptr->Data[i] = tempValue;
                        i++;
                    }
                    else
                    {
                        overflow = MNS_TRUE;
                    }

                    position = CMD_EVEN_POS;
                    break;

            }
        }

        if (counter >= func_ptr->Func)
        {
            lastFunc = func_ptr->Func + (word)1;
            func_ptr++;
        }

        counter++;

    }

    if (i < (word)MAX_MSG_TX_DATA)
    {
        tx_ptr->Data[i] = (byte)(lastFunc >> 4);
        i++;
    }
    else
    {
        overflow = MNS_TRUE;
    }

    if (i < (word)MAX_MSG_TX_DATA)
    {
        tx_ptr->Data[i] = (byte)((word)(lastFunc << 4));
        i++;
    }
    else
    {
        overflow = MNS_TRUE;
    }

    if (MNS_FALSE != overflow)
    {
        return(CmdErrorMsg(tx_ptr, ERR_PROCESSING)); /* Processing Error */
    }
    else
    {
        tx_ptr->Length = i;
        return(OP_STATUS);
    }
}
#endif
