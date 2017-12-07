/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         FBlock DebugMessages (Adjust_Application_DebugMessage)
File:           dm.c
Version:        3.0.x-SR-1  
Language:       C
Author(s):      R.Hanke
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
Date            By      Description

==============================================================================
*/

/*! \file
  * \brief      FBlock DebugMessages (Part Adjust_Application_DebugMessage)
  *
  * \details    Implementation of functions needed by FBlock DebugMessages
  *             especially by function Adjust_Application_DebugMessage.
  *             The function Adjust_Application_DebugMessage is provided for
  *             adjusting the debug level for application-related debug
  *             events.
  *
  * \remarks    The FBlock DebugMessages is part of the MOST Debug Messages
  *             Module (MDM) which is part of MNS Layer I.
  *
  */


/*
==============================================================================
    Includes
==============================================================================
*/

#include "mostns.h"
#include "dm.h"


#ifdef DM_MIN
#include "t_dm.tab"
#endif


/*
==============================================================================
    Module Internal Definitions
==============================================================================
*/

#define MDM_CFG_APP_NUM_FUNC_IDS        ((word)MDM_APP_NUM_FUNC_IDS)


/*
==============================================================================
==============================================================================
    Module Implementation
==============================================================================
==============================================================================
*/

#ifdef DM_0
/*!
  * \brief      Set the debug level of a application-related debug event.
  * \param[out] tx_ptr  Message Tx Pointer
  * \param[in]  rx_ptr  Message Rx Pointer
  * \return     status/error
  *                 OP_NO_REPORT: Everything is in order
  *                 OP_ERROR:     Error occurred
  *
  */
byte DM_AdjAppDbgMsg_Set(pTMsgTx tx_ptr, pTMsgRx rx_ptr)            /*lint -esym( 818, rx_ptr ) no use of const data type in API functions*/
{
    byte ret_val;

    #ifndef CMD_ADD8
    if (rx_ptr->Length != (word)3)                                  /* Check message length */
    {
        ret_val = CmdErrorMsg(tx_ptr, ERR_LENGTH);                  /* Error: Invalid length */
    }
    else                                                            /* Length check is ok ! */
    #endif  /* #ifndef CMD_ADD8 */
    {
        word func_id;
        byte debug_level;
        byte res;

        DECODE_WORD(&func_id, rx_ptr->Data);                        /* Decode function id */
        debug_level = rx_ptr->Data[2];                              /* Extract debug level */

        if((word)0x0FFF != func_id)                                 /* Single function id addressed ? */
        {
            res = MdmSetDbgLevelApp(func_id, debug_level);          /* Set debug level */

            if(ERR_NO == res)                                       /* Operation succeed ? */
            {
                ret_val = OP_NO_REPORT;
            }
            else
            {
                ret_val = CmdErrorParamWrong(tx_ptr, (byte)0x01, &(rx_ptr->Data[0]), (byte)0x02);       /* Error: Operation failed */
            }
        }
        else                                                        /* All application debug function ids addressed */
        {
            word dbg_level_list[MDM_APP_NUM_FUNC_IDS];
            word length;
            word i;
            word fid;

            ret_val = OP_NO_REPORT;

            res = MdmGetDbgLevelListApp(dbg_level_list,             /* Get list of function id/debug level pairs */
                                        MDM_CFG_APP_NUM_FUNC_IDS,
                                        &length);

            if(ERR_NO == res)                                       /* At least one function id registered ? */
            {
                for(i=(word)0; i<length; i++)
                {
                    fid=(word)(dbg_level_list[i] & (word)0x0FFF);   /* Extract MOST function id */
                    res = MdmSetDbgLevelApp(fid, debug_level);      /* Set debug level */

                    if(ERR_PARAM == res)
                    {
                        ret_val = CmdErrorParamWrong(tx_ptr, (byte)0x02, &(rx_ptr->Data[2]), (byte)0x01);   /* Error: Operation failed (invalid debug level) */
                        break;
                    }
                }
            }
        }
    }

    return(ret_val);
}                                                                   /*lint +esym( 818, rx_ptr ) no use of const data type in API functions*/
#endif  /* #ifdef DM_0 */



#ifdef DM_1
/*!
  * \brief      Get the debug level of a application-related debug event.
  * \param[out] tx_ptr  Message Tx Pointer
  * \param[in]  rx_ptr  Message Rx Pointer
  * \return     status/error
  *                 OP_STATUS: Everything is in order
  *                 OP_ERROR:  Error occurred
  *
  */
byte DM_AdjAppDbgMsg_Get(pTMsgTx tx_ptr, pTMsgRx rx_ptr)        /*lint -esym( 818, rx_ptr ) no use of const data type in API functions*/
{
    byte ret_val;

    #ifndef CMD_ADD8
    if ( (OP_GET == rx_ptr->Operation) &&                           /* Length check only in case of OP_GET, */
            (rx_ptr->Length != (word)2) )                              /* but not in case of OP_SETGET */
    {
        ret_val = CmdErrorMsg(tx_ptr, ERR_LENGTH);                  /* Error: Invalid message length */
    }
    else                                                            /* Length check is ok ! */
    #endif  /* #ifndef CMD_ADD8 */
    {
        word func_id;
        byte res;

        DECODE_WORD(&func_id, rx_ptr->Data);                        /* Decode function id */

        if((word)0x0FFF != func_id)                                 /* Single function id addressed ? */
        {
            word tmp;

            res = MdmGetDbgLevelApp(func_id);                       /* Get associated debug level */

            if((byte)0xFF != res)                                   /* Function id registered ? */
            {
                /*
                    * Store debug level and function id using one word variable
                    * Bit 15...12: Debug level
                    * Bit 11...0: Function id
                    */
                tmp = (word)((word)res << 12);
                tmp |= (word)((word)0x0FFF & func_id);
                tx_ptr->Data[0] = HB(tmp);
                tx_ptr->Data[1] = LB(tmp);
                tx_ptr->Length = (word)2;
                ret_val = OP_STATUS;
            }
            else
            {
                ret_val = CmdErrorMsg(tx_ptr, ERR_FKTID);           /* Error: Function id unknown */
            }
        }
        else                                                        /* All application debug function ids addressed */
        {
            word dbg_level_list[MDM_APP_NUM_FUNC_IDS];
            word length;
            word i;
            word j;

            res = MdmGetDbgLevelListApp(dbg_level_list,             /* Get list of function id/debug level pairs */
                                        MDM_CFG_APP_NUM_FUNC_IDS,
                                        &length);

            if(ERR_NO == res)                                       /* At least one function id registered ? */
            {
                j=(word)0;
                for(i=(word)0; i<length; i++)                       /* Fill tx pointer with debug-level-list */
                {
                    tx_ptr->Data[j++] = HB(dbg_level_list[i]);
                    tx_ptr->Data[j++] = LB(dbg_level_list[i]);
                }
                tx_ptr->Length = (word)(length << 1);               /* Set message length (number of table entries * 2 byte) */
            }
            else
            {
                tx_ptr->Length = (word)0;                           /* Set message length to 0 (no elements in list) */
            }

            ret_val = OP_STATUS;
        }
    }

    return(ret_val);
}                                                               /*lint +esym( 818, rx_ptr ) no use of const data type in API functions*/
#endif  /* #ifdef DM_1 */



#ifdef DM_2
/*!
  * \brief      Handle FBlock DebugMessages status message.
  * \param[in]  rx_ptr  Message Rx Pointer
  * \return     OP_NO_REPORT
  *
  */
byte DM_AdjAppDbgMsg_Status(pTMsgRx rx_ptr)         /*lint -esym( 818, rx_ptr ) no use of const data type in API functions*/
{
    #ifdef DM_CB0
    DmAdjAppDbgMsgStatus(rx_ptr);                   /* Transport to application */
    #endif

    return(OP_NO_REPORT);
}                                                   /*lint +esym( 818, rx_ptr ) no use of const data type in API functions*/
#endif  /* #ifdef DM_2 */



#ifdef DM_3
/*!
  * \brief      Handle FBlock DebugMessages error message.
  * \param[in]  rx_ptr  Message Rx Pointer
  * \return     OP_NO_REPORT
  *
  */
byte DM_AdjAppDbgMsg_Error(pTMsgRx rx_ptr)          /*lint -esym( 818, rx_ptr ) no use of const data type in API functions*/
{
    #ifdef DM_CB1
    DmAdjAppDbgMsgError(rx_ptr);                    /* Transport to application */
    #endif

    return(OP_NO_REPORT);
}                                                   /*lint +esym( 818, rx_ptr ) no use of const data type in API functions*/
#endif  /* #ifdef DM_3 */


/*
==============================================================================
    End Of File
==============================================================================
*/
