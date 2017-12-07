/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Outside Module containing all Callback Functions of
                MOST High Protocol Service Module
File:           NS_CB_MH.C
Version:        3.0.2.Alpha.1
Language:       C
Author(s):      S.Kerber
Date:           03.September.2009

FileGroup:      Layer II Extension: MOST High Protocol Service
Customer ID:    <None; as non-released alpha version>
FeatureCode:    FCR1
------------------------------------------------------------------------------

                (c) Copyright 1998-2009
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




/*----------------------------------------------------------------------------- */
#ifdef MHP_CB1
byte MhpTxStatus(byte status, pTMhpTx tx_ptr)
{
    pTMhpTx dummy_ptr;

    dummy_ptr = tx_ptr;                         /* tx_ptr can be used to indicate message */

    switch (status)
    {
        /* Block was succesfully transmitted */
        /*------------------------------------ */
        case MHP_TX_SUCCESS:

            /* The application has to decide, if further blocks (segments) must be transmitted, */
            /* or if the connection should be closed */

            return(0x00);       /* Shut down connection after timeout, since no further blocks available */
         /* return(0x01); */    /* tx_buf was prepared for next block (segment); connection will not be closed */



        /* Transmission failed */
        /*------------------------------------ */
        default:
            break;

    }

    return(0);
}
#endif





/*----------------------------------------------------------------------------- */
#ifdef MHP_CB2
void MhpRxComplete(byte status, pTMhpRx rx_ptr)
{
    pTMhpRx dummy_ptr;
    dummy_ptr = rx_ptr;                         /* rx_ptr must be used to indicate message */

    switch (status)
    {
        case MHP_RX_SUCCESS:

            /* Packet has been successfully received */

            break;




        default:

            /* Packet could not be received, since any error occurred */

            break;

    }

}
#endif



/*----------------------------------------------------------------------------- */
#ifdef MHP_CB5
bool MhpGetBufIntf(pTMhpRx buf_ptr)
{
    /* Check requested connection:                                                  */
    /*  - buf_ptr->Src_Adr                                                          */
    /*  - buf_ptr->FBlock_ID                                                        */
    /*  - buf_ptr->Inst_ID                                                          */
    /*  - buf_ptr->Func_ID                                                          */
    /*  - buf_ptr->Operation                                                        */

    /* Depending on these parameters, the respective buffer                         */
    /* interface description must be returned to the                                */
    /* MostHighProtocol Service.                                                    */

    /* The following function can be used to set the required values:               */

    /*      MhpSetBufIntf( flags, ptr, buf_size );                                  */

    /*          flags:      0x01: ptr points at data buffer                         */
    /*                      0x02: ptr points at respective service function         */

    /*          ptr:        pointer at data buffer or service function              */
    /*          buf_size:   size of data buffer in bytes                            */

    /* This callback function has to return "0x00", if the function MhpSetBufIntf() */
    /* is directly called by the application (within this callback function !).     */

    /* When the Command Interpreter of Layer II is used, the return value depends   */
    /* on the availablities of internal AMS buffers: */


    /*---------------------------------------------------*/
    /* The following functionality can be used, if the   */
    /* Command Interpreter of Layer II is implemented:   */
    /*---------------------------------------------------*/

    pTMsgTx msg_tx_ptr;
    pTMsgRx msg_rx_ptr;

    if ( (msg_tx_ptr = MsgGetTxPtr() ) != NULL )
    {
        if ( (msg_rx_ptr = MsgGetRxInPtr() ) != NULL )
        {
            msg_rx_ptr->Src_Adr     = buf_ptr->Src_Adr;
            msg_rx_ptr->FBlock_ID   = buf_ptr->FBlock_ID;
            msg_rx_ptr->Inst_ID     = buf_ptr->Inst_ID;
            msg_rx_ptr->Func_ID     = buf_ptr->Func_ID;
            msg_rx_ptr->Operation   = buf_ptr->Operation;

            CmdRxInterpreter(msg_tx_ptr,msg_rx_ptr);    /* MhpSetBufIntf() is called within this function */

            MsgFreeRxMsg(msg_rx_ptr);
            return (MNS_TRUE); /* CmdRxInterpreter successfully called, but does not mean that even the table entry has been found    */
        }
        else
        {
            MsgTxUnused(msg_tx_ptr);
            return (MNS_FALSE); /* no free RX buffer available */
        }
    }
    else
    {
        return (MNS_TRUE); /* no free TX buffer available */
    }
}
#endif


/*----------------------------------------------------------------------------- */
#ifdef MHP_CB30
bool MhpTxDelay(word delay, pTMhpTx tx_ptr)
{
  /* Insert a short delay !                                                           */
  /* 'tx_ptr' can be used to persive the respective connection identifier if desired. */

  /* The delay time ('time_delay') should be about: time_delay = delay_cnt * time_1   */

  /* 'delay_cnt':   Argument 'delay'                                                  */
  /* 'time_1':      Average time between two MOST High data packets on your           */
  /*                platform, when the transmission is not delayed.                   */
  /*                This time depends on your software and hardware architecture.     */



  #if 1
    /* First possibility:                                       */

    /* Insert the delay time within this callback function and  */
    /* return MNS_TRUE:                                         */

    /* insert delay..... (time_delay)                           */

    return (MNS_TRUE);      /* Delay time is passed */

  #else

    /* Second possibility:                                                              */

    /* The delay time is realized outside of this callback function.                    */
    /* As soon as the delay time is passed, MostServiceHigh() must                      */
    /* be called by using argument MNSH_E_TX_EVENT. Return from callback                */
    /* function by using value MNS_FALSE:                                               */

    /* start your high performance timer..... (time_delay)                              */

    return (MNS_FALSE); /* Delay time is not yet passed.                                */
                        /* End of delay will be indicated by calling MostServiceHigh()  */
                        /* using argument MNSH_E_TX_EVENT                               */
  #endif
}
#endif



/*----------------------------------------------------------------------------- */
#ifdef MHP_CB31
word MhpTxEstablishDelay(word air_tx, word air_rx, word ratio, pTMhpTx tx_ptr)
{
    /* Here you have the possibility to establish the delay window if desired: */
    /* tx_ptr->MinDelay = ... ; */
    /* tx_ptr->MaxDelay = ... ; */

    /* Here you have the possibility to set the start delay value: */
    return(0xFFFF); /* 0xFFFF: "automatic", start value is calculated by NetServices */
}
#endif


#ifdef MHP_CB32
bool MhpHighResTimerStart(word delay, pTMhpTx buf_ptr)
{
    word    dummy_delay;
    pTMhpTx dummy_buf_ptr;

    /* An external timer has to be started here which provides      */
    /* a delay of "delay" microseconds. When it is finished,        */
    /* the API function MhpHighResTimeout() has to be called        */
    /* with parameter buf_ptr.                                      */

    dummy_delay     = delay;
    dummy_buf_ptr   = buf_ptr;

    return(MNS_TRUE);
}
#endif


#ifdef MHP_CB33
word MhpTxControlRange(word air_rx, pTMhpTx tx_ptr)
{
    /* Here you have the possibility to establish the delay window if desired: */
    /* tx_ptr->MinDelay = ... ; */
    /* tx_ptr->MaxDelay = ... ; */
    /* MinDelay and MaxDelay are given in microseconds */

    /* Here you have the possibility to set the start delay value: */
    return(0xFFFF); /* 0xFFFF: "automatic", start value is calculated by NetServices */
}
#endif





/*----------------------------------------------------------------------------- */
#ifdef MHP_DBG_TX
void MhpDebugInfoTx(pTMhpTx buf_ptr, byte* info_ptr, byte info_length)
{
    byte    dummy_length;               /* Just prevent compiler warnings */
    pTMhpTx dummy_buf_ptr;
    byte*   dummy_info_ptr;

    dummy_length    = info_length;
    dummy_buf_ptr   = buf_ptr;
    dummy_info_ptr  = info_ptr;
}
#endif



/*----------------------------------------------------------------------------- */
#ifdef MHP_DBG_RX
void MhpDebugInfoRx(pTMhpRx buf_ptr, byte* info_ptr, byte info_length)
{
    byte    dummy_length;               /* Just prevent compiler warnings */
    pTMhpRx dummy_buf_ptr;
    byte*   dummy_info_ptr;

    dummy_length    = info_length;
    dummy_buf_ptr   = buf_ptr;
    dummy_info_ptr  = info_ptr;
}
#endif



/*----------------------------------------------------------------------------- */
#ifdef MHP_DBG_TXTEL
void MhpDebugInfoTxTel(byte status, pTMhpTxTel tel_ptr)
{
    byte dummy_status;                  /* Just prevent compiler warnings */
    pTMhpTxTel dummy_tel_ptr;

    dummy_status  = status;
    dummy_tel_ptr = tel_ptr;
}
#endif




/*----------------------------------------------------------------------------- */
#ifdef MHP_DBG_RXTEL
void MhpDebugInfoRxTel(pTMhpRxTel tel_ptr)
{
    pTMhpRxTel dummy_tel_ptr;           /* Just prevent compiler warnings */

    dummy_tel_ptr = tel_ptr;
}
#endif
