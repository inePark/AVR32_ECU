/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Outside Module containing all Callback Functions of Layer II
File:           NS_CB_L2.c
Version:        3.0.3
Language:       C
Author(s):      S.Kerber
Date:           11.June.2010

FileGroup:      Layer II
Customer ID:    <None; as non-released alpha version>
FeatureCode:    FCR1
------------------------------------------------------------------------------

                (c) Copyright 1998-2010
                SMSC
                All Rights Reserved

------------------------------------------------------------------------------



Modifications
~~~~~~~~~~~~~
Date        By      Description
09/01/2010  GW      Edited for NSV3 IOC Demo project

11/17/2009  GW      Edited for Eval92 MSV3 Demo project
==============================================================================
*/






/*
------------------------------------------------------------------------------
        Include Files
------------------------------------------------------------------------------
*/
#include <stdint.h>
#include <string.h>
#include <asf.h>

#include "defines.h"
#include "mostns.h"
#include "ams.h"


#include "board_utils.h"
#include "i2c_lld.h"
#include "timers.h"
#include "con_mgr.h"

#include "nbehc.h"
#include "fblock_pv_amp.h"
#include "fblock_pv_auxin.h"
#include "fblock_pb_amp.h"
#include "fblock_pb_auxin.h"
/*
------------------------------------------------------------------------------
        Local Definitions
------------------------------------------------------------------------------
*/

// from et.c
#define ET_SHUTDOWN_IF_POWERMASTER                               ((byte) 0x00)
#define ET_SHUTDOWN_SIM_SHUTDOWN_TEMPERATURE                     ((byte) 0x01)
#define ET_SHUTDOWN_SIM_DEAD_TEMPERATURE                         ((byte) 0x02)
#define SDN_SUCCESS                                              ((byte) 0x00)
#define SDN_UNABLE_TO_PROCESS                                    ((byte) 0x01)
#define SDN_NOT_POWERMASTER                                      ((byte) 0x02)
#define ATT_DISABLED                                             ((byte) 0x00)
#define ATT_ENABLED                                              ((byte) 0x01)


/*
------------------------------------------------------------------------------
        Local variables and buffers
------------------------------------------------------------------------------
*/
extern u8_t PM_Version[];
u8_t        print_buff[40];

TMostVersionInfo MostVersion;



byte GroupAddr_NonVolatile;                         /* Should be in non-volatile memory segment if available */

#if (NUM_FBLOCKS>0)
byte InstID_NonVolatile[NUM_FBLOCKS];               /* Should be in non-volatile memory segment if available */
#endif

#if (NUM_FBLOCKS_SHADOW>0)
byte InstIDShadow_NonVolatile[NUM_FBLOCKS_SHADOW];  /* Should be in non-volatile memory segment if available */
#endif

#ifdef AH_CB3
TDevTab DeviceTab_NonVolatile[ADDRH_SIZE_DEVICE_TAB]; /* Should be in non-volatile memory segment if available */
#endif


/*------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------*/
/*                                                                                                      */
/*                      Callback Functions of MOST Supervisor Layer II                                  */
/*                                                                                                      */
/*------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------*/




/*----------------------------------------------------------------------------- */
#ifdef MSV2_CB1
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : SystemCommunicationInit()                                  */
/* Description : indicates the completion of the configuration process      */
/* CB Type     : A                                                          */
/* Parameter(s): ConfigStatusOk - Configuration.Status                      */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void SystemCommunicationInit(byte ConfigStatusOk)
{
	byte dummy;
	dummy = ConfigStatusOk;
}
#endif




/*----------------------------------------------------------------------------- */
#ifdef MSV2_CB2
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CentralRegistryCheckStart()                                */
/* Description : starts the system configuration check                      */
/* CB Type     : B                                                          */
/* Parameter(s): none                                                       */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void CentralRegistryCheckStart(void)        /* NetworkMaster Only */
{
	/* The Application has to check the System Configuration (Central Registry). */

	/* After completion, the application has to report the result using */
	/* the API function CentralRegistryCheckComplete(bool). */

	/*  -> CentralRegistryCheckComplete(1);     */ /* If System Configuration Ok */
	/*  -> CentralRegistryCheckComplete(0);     */ /* If System Configuration NotOk */

	/* Please note:  */
	/* CentralRegistryCheckComplete() function can not be called within this callback function. */
}
#endif




/*----------------------------------------------------------------------------- */
#ifdef MSV2_CB3
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CentralRegistryClear()                                     */
/* Description : central registry has to be cleared                         */
/* CB Type     : B                                                          */
/* Parameter(s): none                                                       */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void CentralRegistryClear(void)             /* NetworkMaster Only */
{
}
#endif


/*----------------------------------------------------------------------------- */
#ifdef MSV2_CB4
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : Store_Error_Info2()                                        */
/* Description : indicates an error event                                   */
/* CB Type     : A                                                          */
/* Parameter(s): error, type of error                                       */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void Store_Error_Info2(byte error)
{
	byte Dummy;

	Dummy = error;
}
#endif



/*----------------------------------------------------------------------------- */
#ifdef MSV2_CB5
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : FBlockIDsChanged()                                         */
/* Description : new FBlockIDs  or existing ones disappeared                */
/* CB Type     : A                                                          */
/* Parameter(s): received Configuration.Status msg                          */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void FBlockIDsChanged(pTMsgRx Rx_Ptr)
{
	switch (Rx_Ptr->Data[0])
	{
		case NWM_CFG_STATUS_INVALID:

		break;

		case NWM_CFG_STATUS_NEW:

		break;

		case NWM_CFG_STATUS_NEWEXT:

		break;

		default:
		break;
	}
}
#endif




/*------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------*/
/*                                                                                                      */
/*                      Callback Functions of MOST Command Interpreter                                  */
/*                                                                                                      */
/*------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------- */
#ifdef CMD_CB1
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CmdUnknownFBlockShadow()                                   */
/* Description : FBlock shadow was not found in table                       */
/* CB Type     : B                                                          */
/* Parameter(s): rx_ptr - pointer to received nessage                       */
/* Parameter(s): tx_ptr - pointer to reserved tx message                    */
/* Returns     : information how to handle the tx message                   */
/*                                                                          */
/*--------------------------------------------------------------------------*/
byte CmdUnknownFBlockShadow(pTMsgTx tx_ptr, pTMsgRx rx_ptr)
{
	pTMsgTx dummy1;
	pTMsgRx dummy2;

	dummy1 = tx_ptr;
	dummy2 = rx_ptr;
	LOG_ERR("Received unknow shadow message from node %04X\n", rx_ptr->Src_Adr);
	LOG_ERR("   message to FBlock:InstId [%02X:%02X]\n",rx_ptr->FBlock_ID, rx_ptr->Inst_ID);
	LOG_ERR("   Function ID is %04X\n", rx_ptr->Func_ID);

	return (OP_NO_REPORT);
}
#endif



/*----------------------------------------------------------------------------- */
#ifdef CMD_CB2
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CmdRxFilter()                                              */
/* Description : possibility to filter or modify incoming messages          */
/* CB Type     : B                                                          */
/* Parameter(s): rx_ptr - pointer to received nessage                       */
/* Parameter(s): tx_ptr - pointer to reserved tx message                    */
/* Returns     : information how to handle the rx and tx messages           */
/*                                                                          */
/*--------------------------------------------------------------------------*/
byte CmdRxFilter(pTMsgTx tx_ptr, pTMsgRx rx_ptr)
{
	blink_msg_led();   // causes yellow LED to blink each time we get a message

	#ifdef ET_MIN
	byte retval;

	if (NULL != tx_ptr)                 /* call ET_RxFilter only if tx_ptr is available */
	{
		retval = ET_RxFilter(tx_ptr, rx_ptr);

		if (0x00 != retval)
		{
			return(retval);
		}
	}
	#else
	pTMsgTx dummy1;
	pTMsgRx dummy2;

	dummy1 = tx_ptr;
	dummy2 = rx_ptr;
	#endif

	if (NULL == tx_ptr)                 /* no tx_ptr available */
	{
		return(0x03);                   /* force retrigger */
	}
	else
	{
		return(0x00);                   /* call Command Interpreter */
	}
	/* Following returning values are possible: */
	/*  0x00:  The received message was checked (modified). The Command Interpreter of the NetServices */
	/*         must be called after returning from callback function. Tx buffer was not used. */
	/*  0x01:  The received message must not be interpreted by the Command Interpreter. Tx buffer was not used. */
	/*  0x02:  The received message must not be interpreted by the Command Interpreter. Tx buffer was used and */
	/*         must be send by NetSevices. */
	/*  0x03:  message was not interpreted and must be retriggered. Tx buffer was not used. */
	/*          */
}
#endif





/*----------------------------------------------------------------------------- */
#ifdef CMD_CB5
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CmdRxFilter2()                                             */
/* Description : possibility to filter or modify incoming messages          */
/*               if no tx buffer is available                               */
/* CB Type     : B                                                          */
/* Parameter(s): rx_ptr - pointer to received nessage                       */
/* Parameter(s): tx_ptr - pointer to reserved tx message                    */
/* Returns     : information how to handle the rx message                   */
/*                                                                          */
/*--------------------------------------------------------------------------*/
byte CmdRxFilter2(pTMsgTx tx_ptr, pTMsgRx rx_ptr)
{
	#ifdef ET_MIN
	byte retval;

	if (NULL != tx_ptr)                 /* call ET_RxFilter only if tx_ptr is available */
	{
		retval = ET_RxFilter(tx_ptr, rx_ptr);

		if (0x00 != retval)
		{
			return(retval);
		}
	}
	#else
	pTMsgRx dummy;

	dummy = rx_ptr;
	#endif

	if (NULL == tx_ptr)                 /* no tx_ptr available */
	{
		return(0x03);                   /* force retrigger */
	}
	else
	{
		return(0x00);                   /* call Command Interpreter */
	}


	/* Following returning values are possible: */
	/*  0x00:  The received message was checked (modified). The Command Interpreter of the NetServices */
	/*         must be called after returning from callback function. Tx buffer was not used. */
	/*  0x01:  The received message must not be interpreted by the Command Interpreter. Tx buffer was not used. */
	/*  0x02:  The received message must not be interpreted by the Command Interpreter. Tx buffer was used and */
	/*         must be send by NetSevices. */
	/*  0x03:  message was not interpreted and must be retriggered. Tx buffer was not used. */
	/*          */
	/*  PLEASE NOTE: in case no tx_ptr is available only return values 0x01 and 0x03 are allowed. */
	/*               All other return values are treated as 0x03.              */
}
#endif


/*----------------------------------------------------------------------------- */
#ifdef CMD_CB3
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CmdTxFilter()                                              */
/* Description : filters outgoing result messages                           */
/* CB Type     : B                                                          */
/* Parameter(s): rx_ptr - pointer to received message which                 */
/*                        caused the result message                         */
/* Parameter(s): tx_ptr - pointer to reserved tx message                    */
/* Returns     : information how to handle the tx message                   */
/*                                                                          */
/*--------------------------------------------------------------------------*/
byte CmdTxFilter(pTMsgTx tx_ptr, pTMsgRx rx_ptr)
{
	pTMsgTx dummy1;
	pTMsgRx dummy2;

	dummy1 = tx_ptr;
	dummy2 = rx_ptr;

	return (MNS_TRUE);
	/* Following returning values are possible: */
	/*  == 0x00:  The result message will not be transmitted, the tx buffer is released by NetServices. */
	/*          */
	/*  != 0x00:  The result message will be transmitted in an ordinary way.  */

}
#endif



/*----------------------------------------------------------------------------- */
#ifdef CMD_CB4
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : CmdTxNoResult()                                            */
/* Description : get an event when no result message is caused              */
/* CB Type     : A                                                          */
/* Parameter(s): rx_ptr - original received message                         */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void CmdTxNoResult(pTMsgRx rx_ptr)
{
	pTMsgRx dummy;

	dummy = rx_ptr;
}
#endif










/*------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------*/
/*                                                                                                      */
/*                      Callback Functions of NetBlock Module                                           */
/*                                                                                                      */
/*------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------- */
#ifdef NB_CB1
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbShutDown()                                               */
/* Description : method ShutDown is started with parameter "Query"          */
/* CB Type     : B                                                          */
/* Parameter(s): none                                                       */
/* Returns     : suspend result required or not                             */
/*                                                                          */
/*--------------------------------------------------------------------------*/
bool NbShutDown(void)
{
	/* Query: -> return "Suspend" if necessary */

	return (MNS_FALSE);         /* no suspend result required  */

	/*  return (MNS_TRUE);       */ /* suspending */

}
#endif


/*----------------------------------------------------------------------------- */
#ifdef NB_CB8
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbShutDownExecute()                                        */
/* Description : method ShutDown is started with parameter "Execute"        */
/* CB Type     : A                                                          */
/* Parameter(s): none                                                       */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbShutDownExecute(void)
{
	/* save your application signals and wait for light off */
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NB_CB17
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbShutDownDevice()                                         */
/* Description : method ShutDown is started with parameter "DeviceShutdown" */
/* CB Type     : A                                                          */
/* Parameter(s): none                                                       */
/* Returns     : MNS_TRUE  - DeviceShutdown will be done                    */
/*               MNS_FALSE - DeviceShutdown is denied                       */
/*                                                                          */
/*--------------------------------------------------------------------------*/
bool NbShutDownDevice(void)
{
	/* perform a device shutdown */
	return (MNS_TRUE);
}
#endif


/*----------------------------------------------------------------------------- */
#ifdef NB_CB18
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbShutDownWakeup()                                         */
/* Description : method ShutDown is started with 							*/
/*				 parameter "WakeFromDeviceShutdown"        					*/
/* CB Type     : A                                                          */
/* Parameter(s): none                                                       */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbShutDownWakeup(void)
{
	/* wake up the device */
}
#endif









/*----------------------------------------------------------------------------- */
#ifdef NB_CB6
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbGetDeviceInfo()                                          */
/* Description : a request could not be answered directly                   */
/* CB Type     : B                                                          */
/* Parameter(s): id     - request ID                                        */
/* Parameter(s): tx_ptr - reserved Tx message                               */
/* Returns     : OP_STATUS or OP_ERROR                                      */
/*                                                                          */
/*--------------------------------------------------------------------------*/
byte NbGetDeviceInfo(byte id, pTMsgTx tx_ptr)
{
	byte returnvalue;

	returnvalue = OP_STATUS;
	tx_ptr->Data[0]  = id;           				        // Return the ID sent in
	tx_ptr->Data[1]  = ISO8859;       				        // Format Identifier ISO8859

	switch (id)
	{
		case 0x00:                                          // Company Name
		{
			strcpy_P((char *)&tx_ptr->Data[2], &CompanyName[0]);
			tx_ptr->Length   = 7;							//

			break;
		}

		case 0x01:                                          // Product Name
		{
			strcpy_P((char *)&tx_ptr->Data[2], &ProductName[0]);
			tx_ptr->Length   = 9;							//
			break;
		}

		case 0x02:                                          // Product Version
		{
			strcpy_P((char *)&tx_ptr->Data[2], &ProductVersion[0]);
			tx_ptr->Length  = 15;
			break;
		}

		case 0x21:                                          // NetServices Version
		{
			MostGetVersionInfo(&MostVersion);               // get all the current version information
			//sprintf(&tx_ptr->Data[2], "%d.%d.%d", MostVersion.ns[0], MostVersion.ns[1], MostVersion.ns[2]);

			tx_ptr->Data[2]  = 'V';          				// Start of string
			tx_ptr->Data[3]  = hex_upper_nibble(MostVersion.ns[0]);  // Version numbers are in hex
			tx_ptr->Data[4]  = hex_lower_nibble(MostVersion.ns[0]);
			tx_ptr->Data[5]  = '.';
			tx_ptr->Data[6]  = hex_upper_nibble(MostVersion.ns[1]);  //
			tx_ptr->Data[7]  = hex_lower_nibble(MostVersion.ns[1]);
			tx_ptr->Data[8]  = '.';
			tx_ptr->Data[9]  = hex_upper_nibble(MostVersion.ns[2]);  //
			tx_ptr->Data[10] = hex_lower_nibble(MostVersion.ns[2]);
			tx_ptr->Data[11]  = 0x00;			    		// string terminator
			tx_ptr->Length  = 12;
			break;
		}

		case 0x22:                                          // NetServices Revision
		{
			MostGetVersionInfo(&MostVersion);                   // get the revision, use print_buff for temp storage
			//sprintf(&tx_ptr->Data[2], "%d.%d.%d", MostVersion.fw_date[0], MostVersion.fw_date[1], MostVersion.fw_date[0]);
			tx_ptr->Data[2]  = 'V';          				// Start of string
			tx_ptr->Data[3]  = hex_upper_nibble(MostVersion.fw_date[0]);  // version numbers are in hex
			tx_ptr->Data[4]  = hex_lower_nibble(MostVersion.fw_date[0]);
			tx_ptr->Data[5]  = '.';
			tx_ptr->Data[6]  = hex_upper_nibble(MostVersion.fw_date[1]);  //
			tx_ptr->Data[7]  = hex_lower_nibble(MostVersion.fw_date[1]);
			tx_ptr->Data[8]  = '.';
			tx_ptr->Data[9]  = hex_upper_nibble(MostVersion.fw_date[2]);  //
			tx_ptr->Data[10] = hex_lower_nibble(MostVersion.fw_date[2]);
			tx_ptr->Data[11]  = 0x00;			    		// string terminator
			tx_ptr->Length  = 12;
			break;
		}

		case 0x23:                                          // INIC Firmware Version
		{
			MostGetVersionInfo(&MostVersion);               // get all the current version information
			//sprintf(&tx_ptr->Data[2], "%d.%d.%d", MostVersion.fw[0], MostVersion.fw[1], MostVersion.fw[2]);
			tx_ptr->Data[2]  = 'V';          				// Start of string
			tx_ptr->Data[3]  = hex_upper_nibble(MostVersion.fw[0]);  // INIC version numbers are in hex
			tx_ptr->Data[4]  = hex_lower_nibble(MostVersion.fw[0]);
			tx_ptr->Data[5]  = '.';
			tx_ptr->Data[6]  = hex_upper_nibble(MostVersion.fw[1]);  // INIC version numbers are in hex
			tx_ptr->Data[7]  = hex_lower_nibble(MostVersion.fw[1]);
			tx_ptr->Data[8]  = '.';
			tx_ptr->Data[9]  = hex_upper_nibble(MostVersion.fw[2]);  // INIC version numbers are in hex
			tx_ptr->Data[10] = hex_lower_nibble(MostVersion.fw[2]);
			tx_ptr->Data[11]  = 0x00;			    		// string terminator
			tx_ptr->Length  = 12;
			break;
		}

		case 0x24:                                          // Power Manager Version
		{
			/*
			tx_ptr->Data[1]  = PM_Version[1];               // Returning HEX values, not ASCII string
			tx_ptr->Data[2]  = PM_Version[2];
			*/
			tx_ptr->Length   = 3;
			break;
		}

		case 0xAD:                                          // WakeInfo
		{
			/*
			if ( (PM_Version[3] & (1<<PM_IR_POR) || PM_Version[3] & (1<<PM_IR_ONSW)))  // ON/SW & POR - both 'local events'
			{
				tx_ptr->Data[1] = 0x01; // local or 'internal' wakeup
			}
			else // 'external event - activity or ECL - which one?
			{
				if (PM_Version[3] & (1<<PM_IR_EPHY))  // note: Ephy or Ophy set bit 5 ('activity') in PM_Version
				{
					tx_ptr->Data[1] = 0x02; // 'external' - wakeup by activity
				}
				else if (PM_Version[3] & (1<<PM_IR_LIN))
				{
					tx_ptr->Data[1] = 0x03; // 'external' - wakeup by ECL
				}
				else // if none of those bits are set, we don't know why we powered up
				{
					tx_ptr->Data[1] = 0x00; // 'not initialized' - don't know
				}
			}
			tx_ptr->Data[2]  = PM_Version[3];               // Direct WakeInfo byte collected during startup
			*/
			tx_ptr->Length   = 3;
			break;
		}

		default:                            /* id not available ? */
		returnvalue = OP_ERROR;         /* -> error message will be prepared by the NetServices ! */
		break;                          /*    You just have to return OP_ERROR */
	}

	return(returnvalue);
}
#endif




/*----------------------------------------------------------------------------- */
#ifdef NB_CB9
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NBFBlockIDsGet()                                           */
/* Description : Get FBlockIDs and InstIDs (except FBlock: NetBlock)        */
/*               FBlockIDList::=<FBlockID1>,<InstID1>,<FBlockID2>,.....     */
/*                                                                          */
/* Parameter(s): ptr on tx message                                          */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NBFBlockIDsGet(pTMsgTx Tx_Ptr)
{
	/* Please note: */
	/* The operation type of the response message must be filled too ! */

	Tx_Ptr->Data[0]     = FBLOCK_AMP;                                   /* FBlockID:    Amp     0x22 */
	Tx_Ptr->Data[1]     = NetBlock.pFBlockIDs.InstID[AMP_INDEX]         /* InstID:              0x01 */
	Tx_Ptr->Data[0]     = FBLOCK_AUXIN;                                 /* FBlockID:    AuxIn   0x24 */
	Tx_Ptr->Data[1]     = NetBlock.pFBlockIDs.InstID[AUXIN_INDEX];      /* InstID:              0x01 */
	Tx_Ptr->Length      = 4;            /* */
	Tx_Ptr->Operation   = OP_STATUS;    /* */
	return;
}
#endif



/*----------------------------------------------------------------------------- */
#ifdef NB_CB16
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NBFBlockIDsSet()                                           */
/* Description : Change the InstID of a certain FBlockID                    */
/*                                                                          */
/* Parameter(s): ptr on rx message                                          */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
byte NBFBlockIDsSet(pTMsgTx Tx_Ptr, pTMsgRx Rx_Ptr)
{
	return(OP_NO_REPORT);
}
#endif




/*----------------------------------------------------------------------------- */
#ifdef NB_CB12
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbStoreInstIDs()                                           */
/* Description : Trigger to store your FBlock's InstIDs                     */
/*               (NetBlock.FBlockIDs) in non-volatile memory                */
/* CB Type     : B                                                          */
/* Parameter(s): pointer to array of InstIDs                                */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbStoreInstIDs(byte *InstIDs)
{
	byte i;
	for (i = 0; i < NUM_FBLOCKS; ++i)
	InstID_NonVolatile[i] = InstIDs[i];
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NB_CB13
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbRestoreInstIDs()                                         */
/* Description : return the stored InstIDs                                  */
/* CB Type     : B                                                          */
/* Parameter(s): pointer to array of InstIDs                                */
/* Returns     : MNS_TRUE  - InstIDs were restored from nonvolatile memory  */
/*               MNS_FALSE - InstIDs were not restored                      */
/*                                                                          */
/*--------------------------------------------------------------------------*/
byte NbRestoreInstIDs(byte *InstIDs)
{
	byte i;

	for (i = 0; i < NUM_FBLOCKS; ++i)
	{
		if (InstID_NonVolatile[i] != 0)                         /* valid ? */
		InstIDs[i] = InstID_NonVolatile[i];
	}
	return(MNS_TRUE);
}
#endif



/*----------------------------------------------------------------------------- */
#ifdef NB_CB14
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbStoreShadowInstIDs()                                     */
/* Description : Trigger to store your Shadow FBlock InstIDs                */
/*               (InstIDShadow) in non-volatile memory                      */
/* CB Type     : B                                                          */
/* Parameter(s): pointer to array of InstIDs                                */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbStoreShadowInstIDs(byte *ShadowInstIDs)
{
	byte i;
	for (i = 0; i < NUM_FBLOCKS_SHADOW; ++i)
	InstIDShadow_NonVolatile[i] = ShadowInstIDs[i];
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NB_CB15
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbRestoreShadowInstIDs()                                   */
/* Description : return the stored Shadow FBlock InstIDs                    */
/* CB Type     : B                                                          */
/* Parameter(s): pointer to array of InstIDs                                */
/* Returns     : MNS_TRUE  - InstIDs were restored from nonvolatile memory  */
/*               MNS_FALSE - InstIDs were not restored                      */
/*                                                                          */
/*--------------------------------------------------------------------------*/
byte NbRestoreShadowInstIDs(byte *ShadowInstIDs)
{
	byte i;

	for (i = 0; i < NUM_FBLOCKS_SHADOW; ++i)
	{
		if (InstIDShadow_NonVolatile[i] != 0)                   /* valid ? */
		ShadowInstIDs[i] = InstIDShadow_NonVolatile[i];
	}
	return(MNS_TRUE);

}
#endif



/*----------------------------------------------------------------------------- */
#ifdef NB_CB19
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbBoundarySetQuery()                                       */
/* Description : announces change of boundary value                         */
/*               with the possibility to refuse the change                  */
/* CB Type     : B                                                          */
/* Parameter(s): boundary value                                             */
/* Returns     : MNS_TRUE    change of boundary is allowed                  */
/*               MNS_FALSE   change of boundary is not allowed              */
/*                                                                          */
/*--------------------------------------------------------------------------*/
bool NbBoundarySetQuery(byte boundary)
{
	byte dummy;

	dummy = boundary;

	return(MNS_TRUE);
}
#endif



/*----------------------------------------------------------------------------- */
#ifdef NB_CB20
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbFBlockInfoGet()                                          */
/* Description : get information on FBlock NetBlock                         */
/* CB Type     : B                                                          */
/* Parameter(s): id              - request ID                               */
/*               answer_prepared - MNS_TRUE if NB has prepared the answer   */
/*               tx_ptr          - reserved Tx message                      */
/* Returns     : MNS_TRUE   if the request could be serviced                */
/*               MNS_FALSE  if the request could not be serviced            */
/*                                                                          */
/*--------------------------------------------------------------------------*/
bool NbFBlockInfoGet(word id, bool answer_prepared, pTMsgTx tx_ptr)
{
	byte name[] = "SMSC";
	byte i;
	byte version[3];


	if (answer_prepared)
	{
		return(MNS_TRUE);
	}
	else
	{
		switch (id)
		{
			case FBI_SUPPLIER_VERSION:
			version[0] = 1;     // out first version of this FBlock
			version[1] = 0;
			version[2] = 0;

			tx_ptr->Data[2] = FRMT_ISO8859;

			i = MsgVersionToISO8859(&version[0], &tx_ptr->Data[3]);

			tx_ptr->Length  = (word)i + (word)3;
			answer_prepared = MNS_TRUE;
			break;
			case FBI_SYSTEM_INTEGRATOR:
			tx_ptr->Data[2] = FRMT_ISO8859;
			for(i=(byte)0; i<(byte)(sizeof(name)/sizeof(name[0])); ++i)
			{
				tx_ptr->Data[i+3] = (byte)name[i];
			}
			tx_ptr->Length = (word)i + (word)3;
			answer_prepared = MNS_TRUE;
			break;
			default:
			answer_prepared = MNS_FALSE;
			break;

		}
		return(answer_prepared);
	}

}
#endif



/*----------------------------------------------------------------------------- */
#ifdef NB_CB21
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbImplFBlocksGet()                                         */
/* Description : get information on implemented FBlocks                     */
/* CB Type     : B                                                          */
/* Parameter(s): tx_ptr - reserved Tx message                               */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbImplFBlocksGet(pTMsgTx tx_ptr)
{
	/* Please note: */
	/* The operation type of the response message must be filled too ! */

	/* Example: */

	tx_Ptr->Data[0]     = 0xAB;         /* FBlockID:    0xAB */
	tx_Ptr->Data[1]     = 0x01;         /* InstID:      0x01 */
	tx_Ptr->Data[2]     = 0xCD;         /* FBlockID:    0xCD */
	tx_Ptr->Data[3]     = 0x01;         /* InstID:      0x01 */
	tx_Ptr->Length      = 4;
	tx_Ptr->Operation   = OP_STATUS;
	return;
}
#endif


/*----------------------------------------------------------------------------- */
#ifdef NB_CBS1
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbNodeAddrStatus()                                         */
/* Description : NetBlock.NodeAddress.Status message was received           */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received status message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbNodeAddrStatus(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NB_CBE1
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbNodeAddrError()                                          */
/* Description : NetBlock.NodeAddress.Error message was received            */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received error message                  */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbNodeAddrError(pTMsgRx prbuf)
{
}
#endif



/*----------------------------------------------------------------------------- */
#ifdef NB_CBS2
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbFBlockIDsStatus()                                        */
/* Description : NetBlock.FBlockIDs.Status message was received             */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received status message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbFBlockIDsStatus(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NB_CBE2
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbFBlockIDsError()                                         */
/* Description : NetBlock.FBlockIDs.Error message was received              */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received error message                  */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbFBlockIDsError(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NB_CBS3
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbDeviceInfoStatus()                                       */
/* Description : NetBlock.DeviceInfo.Status message was received            */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received status message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbDeviceInfoStatus(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NB_CBE3
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbDeviceInfoError()                                        */
/* Description : NetBlock.DeviceInfo.Error message was received             */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received error message                  */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbDeviceInfoError(pTMsgRx prbuf)
{
}
#endif



/*----------------------------------------------------------------------------- */
#ifdef NB_CBS4
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbGroupAddrStatus()                                        */
/* Description : NetBlock.GroupAddress.Status message was received          */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received status message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbGroupAddrStatus(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NB_CBE4
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbGroupAddrError()                                         */
/* Description : NetBlock.GroupAddress.Error message was received           */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received error message                  */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbGroupAddrError(pTMsgRx prbuf)
{
}
#endif


/*----------------------------------------------------------------------------- */
#ifdef NB_CBS7
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbShutDownResult()                                         */
/* Description : NetBlock.ShutDown.Result message was received              */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received status message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbShutDownResult(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NB_CBE7
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbShutDownError()                                          */
/* Description : NetBlock.ShutDown.Error message was received               */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received error message                  */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbShutDownError(pTMsgRx prbuf)
{
}
#endif


/*----------------------------------------------------------------------------- */
#ifdef NB_CBS9
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbNodePositionStatus()                                     */
/* Description : NetBlock.NodePosition.Status message was received          */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received status message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbNodePositionStatus(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NB_CBE9
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbNodePositionError()                                      */
/* Description : NetBlock.NodePosition.Error message was received           */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received error message                  */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbNodePositionError(pTMsgRx prbuf)
{
}
#endif



/*----------------------------------------------------------------------------- */
#ifdef NB_CBS10
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbRetryParametersStatus()                                  */
/* Description : NetBlock.RetryParameters.Status message was received       */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received status message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbRetryParametersStatus(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NB_CBE10
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbRetryParametersError()                                   */
/* Description : NetBlock.RetryParameters.Error message was received        */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received status message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbRetryParametersError(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NB_CBS11
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbSamplingFrequencyStatus()                                */
/* Description : NetBlock.SamplingFrequency.Status message was received     */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received status message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbSamplingFrequencyStatus(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NB_CBE11
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbSamplingFrequencyError()                                 */
/* Description : NetBlock.SamplingFrequency.Error message was received      */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received status message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbSamplingFrequencyError(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NB_CBS12
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbNotificationStatus()                                     */
/* Description : NetBlock.Notification.Status message was received          */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received status message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbNotificationStatus(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NB_CBE12
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbNotificationError()                                      */
/* Description : NetBlock.Notification.Error message was received           */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received status message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbNotificationError(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NB_CBS13
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbNtfCheckStatus()                                         */
/* Description : NetBlock.NotificationCheck.Status message was received     */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received status message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbNtfCheckStatus(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NB_CBE13
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbNtfCheckError()                                          */
/* Description : NetBlock.NotificationCheck.Error message was received      */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received status message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbNtfCheckError(pTMsgRx prbuf)
{
}
#endif


/*----------------------------------------------------------------------------- */
#ifdef NB_CBS14
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbBoundaryStatus()                                         */
/* Description : NetBlock.Boundary.Status message was received              */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received status message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbBoundaryStatus(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NB_CBE14
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbBoundaryError()                                          */
/* Description : NetBlock.Boundary.Error message was received               */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received error message                  */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbBoundaryError(pTMsgRx prbuf)
{
}
#endif


/*----------------------------------------------------------------------------- */
#ifdef NB_CBS15
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbVersionStatus()                                          */
/* Description : NetBlock.Version.Status message was received               */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received status message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbVersionStatus(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NB_CBE15
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbVersionError()                                           */
/* Description : NetBlock.Version.Error message was received                */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received error message                  */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbVersionError(pTMsgRx prbuf)
{
}
#endif


/*--------------------------------------------------------------------------*/
#ifdef NB_CBS16
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbShutDownReasonStatus()                                   */
/* Description : NetBlock.ShutDownReason.Status message was received        */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received status message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbShutDownReasonStatus(pTMsgRx prbuf)
{
	LOG_NOR("ShutDownReason Status message received\n");

}
#endif

/*--------------------------------------------------------------------------*/
#ifdef NB_CBE16
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbShutDownReasonError()                                    */
/* Description : NetBlock.ShutDownReason.Error message was received         */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received error message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbShutDownReasonError(pTMsgRx prbuf)
{
}
#endif


/*----------------------------------------------------------------------------- */
#ifdef NB_CBS17
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbFBlockInfoStatus()                                       */
/* Description : NetBlock.FBlockInfo.Status message was received            */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received status message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbFBlockInfoStatus(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NB_CBE17
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbFBlockInfoError()                                        */
/* Description : NetBlock.FBlockInfo.Error message was received             */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received error message                  */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbFBlockInfoError(pTMsgRx prbuf)
{
}
#endif


/*----------------------------------------------------------------------------- */
#ifdef NB_CBS18
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbImplFBlocksStatus()                                       */
/* Description : NetBlock.ImplFBlocks.Status message was received            */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received status message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbImplFBlocksStatus(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NB_CBE18
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbImplFBlocksError()                                        */
/* Description : NetBlock.ImplFBlocks.Error message was received             */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received error message                  */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbImplFBlocksError(pTMsgRx prbuf)
{
}
#endif



/*----------------------------------------------------------------------------- */
#ifdef NB_CBS19
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbEUI48Status()                                            */
/* Description : NetBlock.EUI48.Status message was received                 */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received status message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbEUI48Status(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NB_CBE19
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NbEUI48Error()                                             */
/* Description : NetBlock.EUI48.Error message was received                  */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received error message                  */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NbEUI48Error(pTMsgRx prbuf)
{
}
#endif





/*------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------*/
/*                                                                                                      */
/*                      Callback Functions of  NetworkMaster Shadow Module                              */
/*                                                                                                      */
/*------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------- */
#ifdef NM_CBS1
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NmConfigurationStatus()                                    */
/* Description : NetworkMaster.Configuration.Status message was received    */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received status message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NmConfigurationStatus(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NM_CBE1
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NmConfigurationError()                                     */
/* Description : NetworkMaster.Configuration.Error message was received     */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received error message                  */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NmConfigurationError(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NM_CBS2
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NmCentralRegStatus()                                       */
/* Description : NetworkMaster.CentralRegistry.Status message was received  */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received status message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NmCentralRegStatus(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NM_CBE2
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NmCentralRegError()                                        */
/* Description : NetworkMaster.CentralRegistry.Error message was received   */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received error message                  */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NmCentralRegError(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NM_CBR4
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NmSaveConfigResult()                                       */
/* Description : NetworkMaster.SaveConfiguration.Result message was received*/
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received result message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NmSaveConfigResult(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NM_CBE4
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NmSaveConfigError()                                        */
/* Description : NetworkMaster.SaveConfiguration.Error message was received */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received error message                  */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NmSaveConfigError(pTMsgRx prbuf)
{
}
#endif



/*----------------------------------------------------------------------------- */
#ifdef NM_CBRA4
/*------------------------------------------------------------------------------*/
/*                                                                              */
/* Function    : NmSaveConfigResultAck()                                        */
/* Description : NetworkMaster.SaveConfiguration.ResultAck message received     */
/* CB Type     : C                                                              */
/* Parameter(s): prbuf - pointer to received result message                     */
/* Returns     : nothing                                                        */
/*                                                                              */
/*------------------------------------------------------------------------------*/
void NmSaveConfigResultAck(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NM_CBEA4
/*------------------------------------------------------------------------------*/
/*                                                                              */
/* Function    : NmSaveConfigErrorAck()                                         */
/* Description : NetworkMaster.SaveConfiguration.ErrorAck message was received  */
/* CB Type     : C                                                              */
/* Parameter(s): prbuf - pointer to received error message                      */
/* Returns     : nothing                                                        */
/*                                                                              */
/*------------------------------------------------------------------------------*/
void NmSaveConfigErrorAck(pTMsgRx prbuf)
{
}
#endif



/*----------------------------------------------------------------------------- */
#ifdef NM_CBS5
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NmFktIDsStatus()                                           */
/* Description : NetworkMaster.FktIDs.Status message was received           */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received result message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NmFktIDsStatus(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NM_CBE5
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NmFktIDsError()                                            */
/* Description : NetworkMaster.FktIDs.Error message was received            */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received error message                  */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NmFktIDsError(pTMsgRx prbuf)
{
}
#endif


/*----------------------------------------------------------------------------- */
#ifdef NM_CBS6
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NmSystemAvailStatus()                                      */
/* Description : NetworkMaster.SystemAvail.Status message was received      */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received result message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NmSystemAvailStatus(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NM_CBE6
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NmSystemAvailError()                                       */
/* Description : NetworkMaster.SystemAvail.Error message was received       */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received error message                  */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NmSystemAvailError(pTMsgRx prbuf)
{
}
#endif


/*----------------------------------------------------------------------------- */
#ifdef NM_CBS7
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NmFBlockInfoStatus()                                       */
/* Description : NetworkMaster.FBlockInfo.Status message was received       */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received result message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NmFBlockInfoStatus(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NM_CBE7
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NmFBlockInfoError()                                        */
/* Description : NetworkMaster.FBlockInfo.Error message was received        */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received error message                  */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NmFBlockInfoError(pTMsgRx prbuf)
{
}
#endif


/*----------------------------------------------------------------------------- */
#ifdef NM_CBS8
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NmVersionStatus()                                          */
/* Description : NetworkMaster.Version.Status message was received          */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received result message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NmVersionStatus(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NM_CBE8
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NmVersionError()                                           */
/* Description : NetworkMaster.Version.Error message was received           */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received error message                  */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NmVersionError(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NM_CBRA9
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NmOwnConfigInvalidResultAck()                              */
/* Description : NetworkMaster.OwnConfigInvalid.ResultAck was received      */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received result message                 */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NmOwnConfigInvalidResultAck(pTMsgRx prbuf)
{
}
#endif

/*----------------------------------------------------------------------------- */
#ifdef NM_CBEA9
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NmOwnConfigInvalidErrorAck()                               */
/* Description : NetworkMaster.OwnConfigInvalid.ErrorAck was received       */
/* CB Type     : C                                                          */
/* Parameter(s): prbuf - pointer to received error message                  */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void NmOwnConfigInvalidErrorAck(pTMsgRx prbuf)
{
}
#endif





/*------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------*/
/*                                                                                                      */
/*                      Callback Functions of Address Handler                                           */
/*                                                                                                      */
/*------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------- */
#ifdef AH_CB1
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : AddrHSearchFailed()                                        */
/* Description : FBlock could not be found                                  */
/* CB Type     : A                                                          */
/* Parameter(s): ptbuf - pointer to causing message                         */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void AddrHSearchFailed(pTMsgTx ptbuf)
{
	pTMsgTx DummyPtr;

	DummyPtr = ptbuf;           /* (just prevent a compiler warning) */

	/* ...  */
	/* ... Check Tx Msg */
	/* ... */

}
#endif


/*----------------------------------------------------------------------------- */
#ifdef AH_CB2
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : AddrHResult()                                              */
/* Description : Fblock has been found                                      */
/* CB Type     : A                                                          */
/* Parameter(s): MostAddr - MOST address of FBlock                          */
/* Parameter(s): FBlockID                                                   */
/* Parameter(s): InstID                                                     */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void AddrHResult(word MostAddr, byte FBlockID, byte InstID)
{
	word Dummy1;
	byte Dummy2, Dummy3;

	Dummy1 = MostAddr;          /* (just prevent a compiler warning)             */
	Dummy2 = FBlockID;
	Dummy3 = InstID;
}
#endif


/*----------------------------------------------------------------------------- */
#ifdef AH_CB3
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : AddrHStoreDevTab()                                         */
/* Description : store the decentral registry into non-volatile memory area */
/* CB Type     : B                                                          */
/* Parameter(s): SrcPtr - first entry in the decentral registry             */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void AddrHStoreDevTab(pTDevTab SrcPtr)
{
	pTDevTab TgtPtr;
	byte i;

	TgtPtr = DeviceTab_NonVolatile;                     /* load target pointer */

	for (i=0;i<ADDRH_SIZE_DEVICE_TAB;i++)
	{
		TgtPtr->FBlock_ID = SrcPtr->FBlock_ID;          /* Copy decentral registry into */
		TgtPtr->Inst_ID   = SrcPtr->Inst_ID;            /* non-volatile memory area */
		TgtPtr->Most_Addr = SrcPtr->Most_Addr;
		TgtPtr++;
		SrcPtr++;
	}
}
#endif



/*----------------------------------------------------------------------------- */
#ifdef AH_CB4
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : AddrHRestoreDevTab()                                       */
/* Description : restore the decentral registry from non-volatile           */
/*               memory area                                                */
/* CB Type     : B                                                          */
/* Parameter(s): TgtPtr - first entry in the decentral registry             */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void AddrHRestoreDevTab(pTDevTab TgtPtr)
{
	pTDevTab SrcPtr;
	byte i;

	SrcPtr = DeviceTab_NonVolatile;                     /* load target pointer */

	for (i=0;i<ADDRH_SIZE_DEVICE_TAB;i++)
	{
		TgtPtr->FBlock_ID = SrcPtr->FBlock_ID;          /* Copy decentral registry into */
		TgtPtr->Inst_ID   = SrcPtr->Inst_ID;            /* non-volatile memory area */
		TgtPtr->Most_Addr = SrcPtr->Most_Addr;
		TgtPtr++;
		SrcPtr++;
	}
}
#endif



/*------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------*/
/*                                                                                                      */
/*                   Callback Functions of FBlock ET                                                    */
/*                                                                                                      */
/*------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------*/

#ifdef ET_MIN

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : ET_NetInterfaceState_Query()                               */
/* Description : The FBlock ET asks the application about its NetInterface- */
/*               State via this callback.                                   */
/* CB Type     : B                                                          */
/* Parameter(s): -                                                          */
/* Returns     : 0x00  - Normal operation                                   */
/*               0x01  - Standby                                            */
/*                                                                          */
/*--------------------------------------------------------------------------*/
byte ET_NetInterfaceState_Query(void)
{
	return(OP_NO_REPORT);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : ET_Shutdown_Request()                                      */
/* Description : The FBlock ET triggers the application to perform a        */
/*               certain shutdown scenario. The application should do so    */
/*               asynchronously (after the callback returned).              */
/* CB Type     : B                                                          */
/* Parameter(s): The shutdown scenario to use ...                           */
/*               0x00 - Normal shutdown (only if this is the PowerMaster!)  */
/*               0x01 - Shutdown temperature scenario                       */
/*               0x02 - Dead temperature scenario                           */
/* Returns     : 0x00 - The application is able to process the request.     */
/*               0x01 - The application is not able to process the request. */
/*               0x02 - The device is not PowerMaster (in case type==0x00). */
/*                                                                          */
/*--------------------------------------------------------------------------*/
byte ET_Shutdown_Request(byte type)
{
	byte retval;

	switch (type)
	{
		case ET_SHUTDOWN_IF_POWERMASTER:
		{
			retval = SDN_NOT_POWERMASTER;
			break;
		}
		case ET_SHUTDOWN_SIM_SHUTDOWN_TEMPERATURE:
		{                           // send broadcast - requesting network shutdown
			TMsgTx *msg_ptr = MsgGetTxPtrExt(0x01);  // Only 1 byte of data to send
			if (msg_ptr)
			{
				msg_ptr->Tgt_Adr = MSG_TGT_BROADCAST_BLOCKING;
				msg_ptr->FBlock_ID = FBLOCK_NETBLOCK;
				msg_ptr->Inst_ID = INST_ANY;
				msg_ptr->Func_ID = FUNC_SHUTDOWN;
				msg_ptr->Operation = OP_RESULT;
				msg_ptr->Data[0] = 0x03;            // That's TemperatureShutDown

				MsgSend(msg_ptr);
				retval = SDN_SUCCESS;               // done
			}
			else
			{
				retval = SDN_UNABLE_TO_PROCESS;     // can't do it right now
			}
			break;
		}
		case ET_SHUTDOWN_SIM_DEAD_TEMPERATURE:
		{                           // turn off network and power down
			LOG_ERR("Emergency Power Down Simulation\n");
			//PM_OverrideEphy();          // ignore that we're locked
			//PM_OverrideOphy();          // ignore that we're locked
			MostShutDown();
			StopNetServices();          // go to protected mode - should shut down.
			delay_ms(50);           // waiting for NetServices to stop
			HOLD_DUT_IN_RESET();           // resetting INIC causes it to release PWROFF instantly
			//PWROFF_UC_INACTIVE();          // Make sure EHC is not holding power
			retval = SDN_SUCCESS;       // done

			break;
		}
		default:
		{
			retval = SDN_UNABLE_TO_PROCESS;     // got a parameter we don't know
			break;
		}
	}

	return(retval);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : ET_SharedRxTxMsgBuf_Query()                                */
/* Description : The FBlock ET asks the application if shared Rx and Tx     */
/*               buffering is used.                                         */
/* CB Type     : B                                                          */
/* Parameter(s): -                                                          */
/* Returns     : 0x00 - If the shared buffering is not used.                */
/*               0x01 - If the shared buffering is used.                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
byte ET_SharedRxTxMsgBuf_Query(void)
{
	return(0x00);
}


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : ET_Reset_Request()                                         */
/* Description : This function should trigger a device reset                */
/* CB Type     : B                                                          */
/* Parameter(s): None                                                       */
/* Returns     : None                                                       */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void ET_Reset_Request(void)
{
	/* MPM and INIC features used to reset EHC & INIC */
	TMsgTx *msg_ptr;

//	PM_SetHold();                                           // Tell MPM to hold power on while we reset INIC and EHC

	/* reset using INIC command to reset both INIC and EHC - requires INIC's RSOUT connected to EHC_Reset */
	msg_ptr = MsgGetTxPtrExt(1);

	if (msg_ptr)
	{
		msg_ptr->Tgt_Adr   = MSG_TGT_INIC;
		msg_ptr->FBlock_ID = FBLOCK_INIC;
		msg_ptr->Func_ID   = FUNCID_INIC_RESET;
		msg_ptr->Operation = OP_START;
		msg_ptr->Data[0]   = 3;
		msg_ptr->Length    = 1;

		MsgSend3(msg_ptr);
	}
	else
	{
		LOG_ERR("No buffer for ET.Reset\n");
	}

	//    DEBUG_PRINT_STRING("Resetting INIC...\n");
	//   lld_reset();                                            // resets INIC for 70 ms
	//    DEBUG_PRINT_STRING("Now resetting self...\n");
	//    delay_ms(50);
	//    wdt_reset_mcu();                                        // enable watchdog - should time out in ~15msec
	// doesn't "return" this should RESET the Atmel uC.
}


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : ET_AutoWakeup_Request()                                    */
/* Description : The FBlock ET triggers a wakeup via this callback.         */
/* CB Type     : B                                                          */
/* Parameter(s): delay is the time in seconds to wait before perfoming the  */
/*               wakeup. If diag is MNS_TRUE the MostStartUp() function     */
/*               should be called using the DIAGNOSIS flag. If attenuation  */
/*               is MNS_TRUE the -3db attenuation should be used during     */
/*               that wakeup.                                               */
/*               The callback should return immediatly (don't wait / sleep  */
/*               inside the callback)!                                      */
/*               Duration describes the wake up active time.                */
/* Returns     : -                                                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void ET_AutoWakeup_Request(byte delay, bool diag, byte duration)
{
	//    PWROFF_UC_ACTIVE();                        // keep Eval board on during wait & restart
//	PwrAutoWakeupReq(delay, diag, duration);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : ET_FBlockInfo_Query()                                      */
/* Description : get information on FBlock ET                               */
/* CB Type     : B                                                          */
/* Parameter(s): id              - request ID                               */
/*               answer_prepared - MNS_TRUE if NB has prepared the answer   */
/*               tx_ptr          - reserved Tx message                      */
/* Returns     : MNS_TRUE   if the request could be serviced                */
/*               MNS_FALSE  if the request could not be serviced            */
/*                                                                          */
/*--------------------------------------------------------------------------*/
bool ET_FBlockInfo_Query(word id, bool answer_prepared, pTMsgTx Tx_Ptr)
{
	byte integrator_name[] = "SMSC";
	byte i;
	byte version[3];

	if (MNS_FALSE != answer_prepared)
	{
		/* code here if app needs to change any of NetServices answers */
		return(MNS_TRUE);
	}
	else
	{
		/* code here to answer ID's that NetServices does not answer */
		answer_prepared = MNS_TRUE;
		switch (id)
		{
			case FBI_SYSTEM_INTEGRATOR:
			Tx_Ptr->Data[2] = FRMT_ISO8859;
			for(i=(byte)0; i<(byte)(sizeof(integrator_name)); ++i)
			{
				Tx_Ptr->Data[i+3] = (byte)integrator_name[i];
			}
			Tx_Ptr->Length = (word)i + (word)3;
			break;

			case FBI_SUPPLIER_VERSION:
			version[0] = 1;
			version[1] = 0;
			version[2] = 0;

			Tx_Ptr->Data[2] = FRMT_ISO8859;

			i = MsgVersionToISO8859(&version[0], &Tx_Ptr->Data[3]);

			Tx_Ptr->Length  = (word)((word)i + (word)3);
			break;

			default :
			answer_prepared = MNS_FALSE;
			break;

		}

		return(answer_prepared);
	}

}
#endif  /* ET_MIN */



#ifdef ET_ADD6
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : ET_CentralRegistrySize_Query()                             */
/* Description : This function should provide the maximal size of the       */
/*               Central Registry.                                          */
/* CB Type     : B                                                          */
/* Parameter(s): None                                                       */
/* Returns     : 0x00 - No fixed size.                                      */
/*               Others - The max. size of the Central Registry.            */
/*                                                                          */
/*--------------------------------------------------------------------------*/

word ET_CentralRegistrySize_Query(void)
{
	return (0x3C8);  /* Example */
}
#endif

#ifdef ET_ADD7
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : ET_ActivateSlaveMode_Request()                             */
/* Description : This function should trigger the shutdown and prepare      */
/*               the device mode for the next startup.                      */
/* CB Type     : B                                                          */
/* Parameter(s): None                                                       */
/* Returns     : MNS_TRUE - device will perform shutdown                    */
/*               MNS_FALSE - shutdown cannot be done                        */
/*                                                                          */
/*--------------------------------------------------------------------------*/

bool ET_ActivateSlaveMode_Request(void)
{
	/* trigger shutdown                     */
	/* prepare device mode for next startup */
	/* considerAutoWakeUp settings          */
	return(MNS_TRUE);
}
#endif



#ifdef FUNCID_ET_MOSTREMOTERESET
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : ET_MOSTRemoteReset_Request()                               */
/* Description : The FBlock ET calls this function everytime an             */
/*               ET.MOSTRemoteReset.Set was received. Start your work after */
/*               the callback returned!                                     */
/* CB Type     : B                                                          */
/* Parameter(s): The address of the sender and the settings parameter       */
/* Returns     : -                                                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void ET_MOSTRemoteReset_Request(word src_adr, byte settings)
{
}
#endif

#ifdef FUNCID_ET_PHYSICALLAYERTEST
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : ET_PhysicalLayerTest_Status()                              */
/* Description : CB function informs application about activation           */
/*               of PhysicalLayerTest                                       */
/* CB Type     : B                                                          */
/* Parameter(s): running  indicates if test is started resp. finished       */
/* Returns     : -                                                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void ET_PhysicalLayerTest_Status(bool running)
{
}
#endif

#ifdef FUNCID_ET_DSO
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : ET_DSO_Request()                                           */
/* Description : This function should open a new MHP connection             */
/* CB Type     : B                                                          */
/* Parameter(s): num_packets - number of MOSTHigh packets that shall be sent*/
/*               next_packet_method - what to do after a TX_SUCCESS event   */
/*               ack_mode - acknowledge method                              */
/*               fblock_id - target FBlockID                                */
/*               inst_id - target InstID                                    */
/*               fkt_id - target FktID                                      */
/*               op_type - target OpType                                    */
/* Returns     : MNS_TRUE - device will perform shutdown                    */
/*               MNS_FALSE - shutdown cannot be done                        */
/*                                                                          */
/*--------------------------------------------------------------------------*/
byte ET_DSO_Request(word num_packets, byte next_packet_method, byte ack_mode,
byte fblock_id, byte inst_id, word fkt_id, byte op_type)
{
	/* open an MHP connection using */

	/*
	possible return values:
	ET_DSO_OK        (0x00): connection could be opened
	ET_DSO_BUSY:     (0x01): maximum number of connections is reached
	or there is already a connection
	ET_DSO_NO_MEMORY (0x02): not enough memory available to allocate the buffers
	*/

	return(ET_DSO_OK);
}
#endif


#ifdef FUNCID_ET_DSIHOLD
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : ET_DSIHold_Request()                                       */
/* Description : set certain DSI connection to hold                         */
/* CB Type     : B                                                          */
/* Parameter(s): hold - set hold of DSI connection on / off                 */
/*               dsi_idx - number of the DSI connection, offset to 0x400    */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void ET_DSIHold_Request(byte hold, byte dsi_idx)
{
	/*
	If hold is set to enable, the application must stop clearing receive buffer
	of certain DSI connection. Hint for implementation: the callback function
	has to indicate that no buffer memory is available.
	*/
}
#endif

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : ET_DSIDSOCount_Request()                                   */
/* Description : Omits the values for MAX_MHP_RX and MAX_MHP_TX set in the  */
/*               packet driving host.                                       */
/*               This function could also signal the packet driving host to */
/*               send a message with the DSI/DSO count values to the source */
/*               address included in the rx_ptr. In this case the return    */
/*               value should be OP_NO_REPORT                               */
/* CB Type     : B                                                          */
/* Parameter(s): rx_ptr, tx_ptr                                             */
/* Returns     : OP_STATUS when providing the values synchron               */
/*               or OP_NO_REPORT if the answer will be given by the packet  */
/*               driving host.                                              */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef PACKET_ON_SECOND_HOST
byte ET_DSIDSOCount_Request(pTMsgRx rx_ptr, pTMsgTx tx_ptr)
{
	(void) rx_ptr;
	tx_ptr->Data[0] = 0;  /* Dummy value for DSICount RX */
	tx_ptr->Data[1] = 0;  /* Dummy value for DSICount TX */
	tx_ptr->length  = 2;

	return OP_STATUS;
}
#endif


#ifdef FUNCID_ET_ECLTRIGGER
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : ET_ECLTrigger_Request()                                    */
/* Description : trigger the ECL                                            */
/* CB Type     : B                                                          */
/* Parameter(s): tgt_addr   - address of requesting device                  */
/*               ecl_action - action to be performed                        */
/*               length     - length of ecl_data                            */
/*               ecl_data   - additional data for requested ecl_action      */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void ET_ECLTrigger_Request(word tgt_addr, byte ecl_action, byte length, byte* ecl_data)
{

	/*  trigger the requested ECLAction   */

	return;
}
#endif

#ifdef FUNCID_ET_ECLINITIATORSTATE
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : ET_ECLInitiatorState_Query()                               */
/* Description : asks for last state of the ECL                             */
/* CB Type     : B                                                          */
/* Parameter(s): node_class - ECL node class of a device                    */
/*               comm_state - communication status of ECL                   */
/*               alive      - ECL alive result                              */
/*               signal     - ECL signal result                             */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void ET_ECLInitiatorState_Query (word node_class, byte* comm_state, byte* alive, byte* signal)
{

	*comm_state = 0x00;         /* 0x00:  Init   */
	/* 0x01:  OK     */
	/* 0x02: Invalid */

	*alive      = 0x01;         /* 0x00: device is not alive */
	/* 0x01: device is alive     */

	*signal     = 0x00;         /* 0x00: 0 reported, interpretation depends on performed test */
	/* 0x01: 1 reported                                           */

	return;
}
#endif








