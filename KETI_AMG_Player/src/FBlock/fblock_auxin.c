/*****************************************************

Module  : FBlock - for AuxIn Function
Version : 2.0
File    : fblock_auxin.c
Date    : 11/28/2006
Author  : Gary Warren
Company : SMSC - AIS
Comments:

Description:
    Implements the function block AuxIn.
All AuxIn functions are here also


Modifications
~~~~~~~~~~~~~
Date            By      Description

10/11/2010  GW  Update for MOST Spec 3 and ACK OpTypes.
                Connection logic is moved into here from con_mgr.


==============================================================================
*/
#include <stdint.h>
#include <asf.h>
#include "mostns.h"

#include "defines.h"
#include "board_utils.h"

#include "fblock_pb_auxin.h"
#include "fblock_pv_auxin.h"
#include "con_mgr.h"

/*
------------------------------------------------------------------------------
    Local Definitions
------------------------------------------------------------------------------
*/
#define AUXIN_VERSION_MAJOR   1
#define AUXIN_VERSION_MINOR   0
#define AUXIN_VERSION_BUILD   0

/*
------------------------------------------------------------------------------
    Local variables and buffers
------------------------------------------------------------------------------
*/

TAuxin Auxin;              					// Structure containing all properties and methods

TNtfMatrix  NtfMatrix_Auxin[NUM_NTF_PROP_AUXIN][NTF_SIZE_DEVICE_TAB+1]; 			// Notification Matrix of FBlock
struct Msg_Tx_Type *AuxinRetained_Tx_Ptr; // global for delayed responses
static uint16_t SenderHandle;

CONNECTION ADC_connection;			// AuxIn properties
uint16_t scm_buf[32];

/*
------------------------------------------------------------------------------
    Local Function Prototypes
------------------------------------------------------------------------------
*/


/*
------------------------------------------------------------------------------
    External Function Prototypes
------------------------------------------------------------------------------
*/


/*
------------------------------------------------------------------------------
    Tables
------------------------------------------------------------------------------
*/

#include "t_fblock_auxin.tab"      				// Table containing all FUNC_IDS and all OP_TYPES of FBlock



/****************************************************************************
*  Function:       Auxin_Init
*
*  Description:    Init the FBlock
*
*  Input(s):       -none-
*
*  Side-effect(s): -none-
*
*  Return(s):      -none-
*
****************************************************************************/
void Init_Auxin(void)
{
	uint8_t i;
	uint8_t j;

	Auxin.SrcActivity = 0x02;
	Auxin.pConnectionLabel = 0xFFFF;
	Auxin.pNodeDelay = 0;

	ADC_connection.INIC_handle = INVALID_SOCKET;
	Auxin_KillConnections();



	#ifdef NTF_MIN
	// Init Notification Matrix
	//------------------------------------------
	for (i=0;i<NUM_NTF_PROP_AUXIN;i++) // Number of Properties

	{
		for (j=0;j<NTF_SIZE_DEVICE_TAB+1;j++) // Number of entries in the Device Index Table for each property
		NtfMatrix_Auxin[i][j] = (TNtfMatrix)NTF_DEV_INDEX_FREE; // clear whole notification matrix of FBlock
	}
	#endif
}

/*
*  FUNCTION:       Auxin_KillConnections
*
*  PARAMETERS:     none
*
*  DESCRIPTION:    Sets all network side connection parameters for Stereo Aux In to "not connected".
*                  when network dies, these connections die automatically - flag that they are invalid
*                  Also used at startup to flag all network connections are "not connected"
*
*  RETURNS:        nothing
*
*/
void Auxin_KillConnections(void)
{
	ADC_connection.Network_handle = INVALID_SOCKET;
	ADC_connection.ConnectionLabel = INVALID_LABEL; // MOST50
	ADC_connection.ConnectionHandle = INVALID_CONNECTION; // MOST25 & MOST50 - same
	ADC_connection.Connecting = FALSE; // not currently working on connecting
	ADC_connection.LocalCommand = FALSE; // not doing stuff on our own
	GREEN_LED_OFF(); // ADC connection LED should be off

	AuxinRetained_Tx_Ptr = NULL; // no messages waiting for response
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Section needed by Rx Command Interpreter
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++




/****************************************************************************
*  Function:       FuncIDs_Get
*
*  Description:    Get the FunctionID's
*
*  Input(s):       ptr on tx message
*
*  Side-effect(s): -none-
*
*  Return(s):      OP_Type
*
****************************************************************************/
uint8_t Auxin_FuncIDs_Get(struct Msg_Tx_Type *Tx_Ptr)
{
	CmdGetFunctionIds(Tx_Ptr, Func_Auxin);
	return (OP_STATUS);
}



/****************************************************************************
*  Function:       	Notification_Set
*
*  Description:    	Set the Notification-List
*
*  Input(s):       	ptr on tx message
*					ptr on rx message
*
*  Side-effect(s): 	-none-
*
*  Return(s):      	OP_Type
*
****************************************************************************/
uint8_t Auxin_Notification_Set(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{
	#ifdef NTF_MIN
	return(NtfSetNotificationMatrix(Tx_Ptr,Rx_Ptr));
	#else
	return (OP_NO_REPORT);
	#endif
}



/****************************************************************************
*  Function:       	Notification_Get
*
*  Description:		Get the Notification-List
*
*  Input(s):       	ptr on tx message
*					ptr on rx message
*
*  Side-effect(s): 	-none-
*
*  Return(s):      	OP_Type
*
****************************************************************************/
uint8_t Auxin_Notification_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{
	#ifdef NTF_MIN
	return(NtfGetNotificationMatrix(Tx_Ptr,Rx_Ptr));
	#else
	return (OP_NO_REPORT);
	#endif
}



/****************************************************************************
*  Function:       	NotificationCheck_Get
*
*  Description:		Get the Notification-List
*
*  Input(s):       	ptr on tx message
*					ptr on rx message
*
*  Side-effect(s): 	-none-
*
*  Return(s):      	OP_Type
*
****************************************************************************/
uint8_t Auxin_NotificationCheck_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{
	#ifdef NTF_MIN
	return(NtfCheck(Tx_Ptr,Rx_Ptr));
	#else
	return (OP_NO_REPORT);
	#endif
}



/****************************************************************************
*  Function:        Auxin_FBlockInfo_Get(struct Msg_Tx_Type *Tx_Ptr);
*
*  Description:     Returns information about our FBlock
*
*  Input(s):        ptr on tx message
*                   ptr on rx message
*
*  Side-effect(s):  -none-
*
*  Return(s):       Status - depending on index passed in
*
****************************************************************************/
uint8_t Auxin_FBlockInfo_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{
	uint16_t rx_id;
	uint8_t  name[] = "AuxIn";
	uint8_t  i;
	uint8_t  version[3];

	CmdDecodeWord(&rx_id, &Rx_Ptr->Data[0]);

	Tx_Ptr->Data[0] = Rx_Ptr->Data[0];              /* prepare TX message */
	Tx_Ptr->Data[1] = Rx_Ptr->Data[1];
	Tx_Ptr->Length  = (u16_t)2;


	if (rx_id < 0x1000)                             /* FktID ?*/
	{
		Tx_Ptr->Length  = (u16_t)3;

		switch (rx_id)
		{

			case FUNC_SOURCEINFO:
			case FUNC_ALLOC:
			case FUNC_DEALLOC:
			case FUNC_SOURCEACT:
			case FUNC_SOURCENAME:
			case FUNC_SYNCDATAINFO:
			Tx_Ptr->Data[2] = MAT_FULLY_IMPLEMENTED_VERIFIED;
			break;


			default:
			Tx_Ptr->Data[2] = MAT_UNKNOWN;
			break;
		}
	}
	else
	{
		switch (rx_id)
		{
			case FBI_FBLOCK_NAME:
			Tx_Ptr->Data[2] = FRMT_ISO8859;
			for(i=(byte)0; i<(byte)(sizeof(name)); ++i)
			{
				Tx_Ptr->Data[i+3] = (byte)name[i];
			}
			Tx_Ptr->Length = (word)i + (word)3;
			break;

			case FBI_SUPPLIER_VERSION:
			version[0] = AUXIN_VERSION_MAJOR;
			version[1] = AUXIN_VERSION_MINOR;
			version[2] = AUXIN_VERSION_BUILD;

			Tx_Ptr->Data[2] = FRMT_ISO8859;

			i = MsgVersionToISO8859(&version[0], &Tx_Ptr->Data[3]);

			Tx_Ptr->Length  = (word)((word)i + (word)3);
			break;

			case FBI_FBLOCK_VERSION:
			version[0] = AUXIN_VERSION_MAJOR;
			version[1] = AUXIN_VERSION_MINOR;
			version[2] = AUXIN_VERSION_BUILD;

			Tx_Ptr->Data[2] = FRMT_ISO8859;

			i = MsgVersionToISO8859(&version[0], &Tx_Ptr->Data[3]);

			Tx_Ptr->Length  = (word)((word)i + (word)3);
			break;

			case FBI_MOST_VERSION:
			version[0]  = GFB_VERSION_MAJOR;
			version[1]  = GFB_VERSION_MINOR;
			version[2]  = GFB_VERSION_BUILD;

			Tx_Ptr->Data[2] = FRMT_ISO8859;

			i = MsgVersionToISO8859(&version[0], &Tx_Ptr->Data[3]);

			Tx_Ptr->Length  = (word)((word)i + (word)3);
			break;

			case FBI_SYSTEM_INTEGRATOR:
			Tx_Ptr->Data[2] = FRMT_ISO8859;
			Tx_Ptr->Data[3] = (u8_t)'S';
			Tx_Ptr->Data[4] = (u8_t)'M';
			Tx_Ptr->Data[5] = (u8_t)'S';
			Tx_Ptr->Data[6] = (u8_t)'C';
			Tx_Ptr->Length  = (u16_t)7;
			break;

			case FBI_FBLOCK_TYPE:
			Tx_Ptr->Data[2] = FRMT_ISO8859;
			Tx_Ptr->Data[3] = (u8_t)'S';
			Tx_Ptr->Data[4] = (u8_t)'t';
			Tx_Ptr->Data[5] = (u8_t)'e';
			Tx_Ptr->Data[6] = (u8_t)'r';
			Tx_Ptr->Data[7] = (u8_t)'e';
			Tx_Ptr->Data[8] = (u8_t)'o';
			Tx_Ptr->Length  = (u16_t)9;
			break;

			default :
			return(CmdErrorParamWrong(Tx_Ptr, 1, &Rx_Ptr->Data[0], 2));
			break;
		}
	}

	return(OP_STATUS);
}
/****************************************************************************
*  Function:       	SourceInfo_Get
*
*  Description:		Return our audio settings and connections if connected
*
*  Input(s):       	ptr on tx message
*					ptr on rx message
*
*  Side-effect(s): 	-none-
*
*  Return(s):      	OP_Type
*
****************************************************************************/
uint8_t Auxin_SourceInfo_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{
	// parameter checking - 1 parameter -> SinkNr
	if (Rx_Ptr->Data[0] != AUXIN_SOURCE_NR)
	return(CmdErrorParamWrong(Tx_Ptr, 1, &Rx_Ptr->Data[0], 1));

	// Can call Auxin_SourceInfo_N here - code from here is the same
	return (Auxin_SourceInfo_N(Tx_Ptr, Rx_Ptr));
}

/****************************************************************************
*  Function:       	SourceInfo_Get
*
*  Description:		Called from above, or from Notification Matrix
*                   Return our audio settings and connections if connected
*
*  Input(s):       	ptr on tx message
*					ptr on rx message
*
*  Side-effect(s): 	-none-
*
*  Return(s):      	OP_Type
*
****************************************************************************/
uint8_t Auxin_SourceInfo_N(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{

	Tx_Ptr->Data[0] = AUXIN_SOURCE_NR;                          // SourceNr
	Tx_Ptr->Data[1] = 0;                                        // MSB of blockwidth
	Tx_Ptr->Data[2] = AUDIO_STEREO * RES_16BIT;                 // LSB of blockwitdh - only support stereo (4 bytes)
	Tx_Ptr->Data[3] = (u8_t) (ADC_connection.ConnectionLabel>>8);   // MSB of connection label
	Tx_Ptr->Data[4] = (u8_t) ADC_connection.ConnectionLabel;        // LSB of connection label
	Tx_Ptr->Data[5] = 0;                                        // Transmission Class 0 = Synchronous
	Tx_Ptr->Data[6] = 0;                                        // Content Protection 0 = None
	Tx_Ptr->Data[7] = 0;                                        // Content Type 0 = Audio
	Tx_Ptr->Data[8] = 2;                                        // Content Description, short stream 2 bytes follow
	Tx_Ptr->Data[9] = AUDIO_STEREO;                             // AudioChannels 2 = Stereo
	Tx_Ptr->Data[10] = RES_16BIT;                                // Resolution 2 = 16-Bit Audio

	Tx_Ptr->Length  = 11;
	return (OP_RESULT);
}


/****************************************************************************
*  Function:       	Allocate_StartResult
*
*  Description:    	Allocates bandwidth and puts the ADC data on the MOST Network
*					by creating Netork Socket (OUT) and connecting socket to ADC
*  Input(s):       	ptr on tx message
*					ptr on rx message
*
*  Side-effect(s): 	-none-
*
*  Return(s):      	OP_Type
*
****************************************************************************/
uint8_t Auxin_Allocate_StartResult(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{
	TMnsResult result;

	/* parameters:  SenderHandle (word  0:1)
	SourceNr, (byte) 2
	*/

	if (Rx_Ptr->Data[2] != AUXIN_SOURCE_NR)
	return(CmdErrorParamWrong(Tx_Ptr, 2, &Rx_Ptr->Data[2], 1));

	// Cmd OK - use connection manager to connect
	DECODE_WORD(&SenderHandle, &Rx_Ptr->Data[0]);  // Save SenderHandle for later reply
	if (ADC_connection.ConnectionHandle == INVALID_CONNECTION) // Make sure we're not already allocated and connected
	{
		if (ConmgrGetMutex(300, Auxin_AllocateTimeout))
		{
			ADC_connection.ConnectType = SCM_MOST_FLAGS_SOURCE_ALLOCATE;
			result = ADC_INIC_CreateSocket(); // starts allocation process
			if (ERR_NO == result)
			{
				AuxinRetained_Tx_Ptr = Tx_Ptr; // keep global pointer to return message - returned later
				return (CMD_TX_RETAIN); // sending result later in Auxin_Allocate_Result
			}
			else
			{
				ConmgrReturnMutex();  //we're quitting, return mutex, stop timeout timer
				LOG_ERR("Error starting ADC->INIC socket creation: result = %02X\n", result);
				Tx_Ptr->Data[0] = ERR_PROCESSING; // processing error
				Tx_Ptr->Length = 1; //
				return (OP_ERRORACK);  // Return with error - allocate failed
			}
		}
		else
		{
			return (CmdErrorMsg(Tx_Ptr, ERR_BUSY)); // can't do it now - return error - done
		}
	}
	else // we are already connected, just send back current info as if we just connected
	{ // here, we will pretend we just returned from a successful connect sequence
		Tx_Ptr->Data[0] = AUXIN_SOURCE_NR; // send back our source #
		Tx_Ptr->Data[1] = 0; // MSB of blockwidth
		Tx_Ptr->Data[2] = AUDIO_STEREO*RES_16BIT; // LSB of blockwidth
		Tx_Ptr->Data[3] = HB(ADC_connection.ConnectionLabel);
		Tx_Ptr->Data[4] = LB(ADC_connection.ConnectionLabel);
		Tx_Ptr->Length = 5; // Both MOST25 and MOST50/150 end up w/ same length for 4 byte stereo
		CmdInsertSenderHandle(Tx_Ptr, HB(SenderHandle), LB(SenderHandle));  // Moves parameters over and fixes length.
		return (OP_RESULTACK); // send response & done
	}
}

void Auxin_AllocateTimeout(void)
{
	LOG_ERR("Timeout while allocating and connecting stereo ADC\n");
	Auxin_Allocate_Result(NSR_E_FAILED);
}

/****************************************************************************
*  Function:       	AuxInAllocate_Result
*
*  Description:	    Called once SCM manager tasks are done creating/connecting socket
*                   Called from the ADC_ConnectSockets_CB function or
*					from ADC_Network_CreateSocket_CB if there was an error
*
*  Input(s):       	result from Socket Connection Manager
*
*  Side-effect(s): 	Sends response message from StartResult
*					Triggers Notification of SourceInfo
*
*  Return(s):      	-none-
*
****************************************************************************/
void Auxin_Allocate_Result(uint16_t result)
{

	if (AuxinRetained_Tx_Ptr == NULL) // sanity check - should have valid pointer when we get here
	{
		LOG_ERR("Tried to respond to AuxInConnect, but had no valid message pointer\n");
		return;
	}

	if (result == NSR_S_OK) // return is OK
	{
		AuxinRetained_Tx_Ptr->Operation = OP_RESULTACK; // response to start/result
		AuxinRetained_Tx_Ptr->Data[0] = AUXIN_SOURCE_NR; // send back our source #
		AuxinRetained_Tx_Ptr->Data[1] = 0; // MSB of blockwidth
		AuxinRetained_Tx_Ptr->Data[2] = AUDIO_STEREO*RES_16BIT; // LSB of blockwidth
		AuxinRetained_Tx_Ptr->Data[3] = HB(ADC_connection.ConnectionLabel);
		AuxinRetained_Tx_Ptr->Data[4] = LB(ADC_connection.ConnectionLabel);
		AuxinRetained_Tx_Ptr->Length = 5; // Both MOST25 and MOST50 end up w/ same length for 4 byte stereo
	}
	else // got some error during allocate
	{
		LOG_ERR("Allocate failed with error code : %02X\n", result);
		AuxinRetained_Tx_Ptr->Operation = OP_ERRORACK; // response to get
		if ( (NSR_E_CRS_NOT_ENOUGH_CHANNELS == result) || (NSR_E_CRS_SOCKET_TABLE_FULL == result) || (NSR_E_CRS_INVALID_BLOCKWIDTH == result))
		{
			AuxinRetained_Tx_Ptr->Data[0]  = ERR_FUNC_SPECIFIC;          // not enough bandwidth error response
			AuxinRetained_Tx_Ptr->Data[1]  = AUXIN_SOURCE_NR;
			AuxinRetained_Tx_Ptr->Data[2]  = 0;
			AuxinRetained_Tx_Ptr->Data[3]  = AUDIO_STEREO*RES_16BIT;     // LSB of blockwidth
			AuxinRetained_Tx_Ptr->Length   = 4;                          //
		}
		else
		{
			AuxinRetained_Tx_Ptr->Data[0]  = ERR_PROCESSING;             // processing error
			AuxinRetained_Tx_Ptr->Length  = 1;                           //
		}
	}
	CmdInsertSenderHandle(AuxinRetained_Tx_Ptr, HB(SenderHandle), LB(SenderHandle));  // Moves parameters over and fixes length.
	MsgSend(AuxinRetained_Tx_Ptr); // send response now
	AuxinRetained_Tx_Ptr = NULL; // pointer no longer valid - MsgSend will free the message after sending
	NtfPropertyChanged(&NtfMatrix_Auxin[0][0], 1); // trigger notification, we've allocated
}



/****************************************************************************
*  Function:       	DeAllocate_StartResult
*
*  Description:	    Takes the ADC off of the Network by Disconnecting ADC
*                   sockets and then Destroying the ADC Network Socket.
*
*  Input(s):       	ptr on tx message
*					ptr on rx message
*
*  Side-effect(s): 	-none-
*
*  Return(s):      	OP_Type
*
****************************************************************************/
uint8_t Auxin_DeAllocate_StartResult(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{
	uint8_t result;

	/* parameters:  SenderHandle (word  0:1)
	SourceNr, (byte) 2
	*/

	if (Rx_Ptr->Data[2] != AUXIN_SOURCE_NR)
	return(CmdErrorParamWrong(Tx_Ptr, 2, &Rx_Ptr->Data[2], 1));

	// Cmd OK - use connection manager to dis-connect
	DECODE_WORD(&SenderHandle, &Rx_Ptr->Data[0]);
	if (ADC_connection.ConnectionHandle != INVALID_CONNECTION) // Checking to see if we are "allocated" first
	{
		if (ConmgrGetMutex(300, Auxin_DeAllocateTimeout))
		{
			ADC_connection.LocalCommand = FALSE; // we're commanded to de-allocate (not doing on our own)
			result = ADC_DisConnectSockets(); // starts de-allocation process
			if (ERR_NO == result)
			{
				AuxinRetained_Tx_Ptr = Tx_Ptr; // keep global pointer to return message - returned later
				return (CMD_TX_RETAIN); // sending result later in Auxin_Allocate_Result
			}
			else
			{
				ConmgrReturnMutex();  //we're quitting, return mutex, stop timeout timer
				LOG_ERR("Error starting ADC disconnect sockets process: result = %02X\n", result);
				Tx_Ptr->Data[0] = ERR_PROCESSING; // processing error
				Tx_Ptr->Length = 1; //
				return (OP_ERRORACK);  // Return with error - allocate failed
			}
		}
		else
		{
			return (CmdErrorMsg(Tx_Ptr, ERR_BUSY));
		}
	}
	else // Not connected, so just reply that its done
	{
		Tx_Ptr->Data[0] = AUXIN_SOURCE_NR; // this is the only parameter we send back
		Tx_Ptr->Length = 1; //
		CmdInsertSenderHandle(Tx_Ptr, HB(SenderHandle), LB(SenderHandle));  // Moves parameters over and fixes length.
		return (OP_RESULTACK); // no other return needed
	}
}

void Auxin_DeAllocateTimeout(void)
{
	LOG_ERR("Timeout while de-allocating and dis-connecting stereo ADC\n");
	Auxin_DeAllocate_Result(NSR_E_FAILED);
}

/****************************************************************************
*  Function:       	AuxInDeAllocate_Result
*
*  Description:	    Called once SCM manager tasks are done destroying socket
*                   Called from the ADC_Network_DestroySocket_CB function or
*					from ADC_DisConnectSockets_CB if there was an error
*
*  Input(s):       	result from Socket Connection Manager
*
*  Side-effect(s): 	Sends response message from StartResult
*					Triggers Notification of SourceInfo
*
*  Return(s):      	-none-
*
****************************************************************************/
void Auxin_DeAllocate_Result(uint16_t result)
{
	if (!ADC_connection.LocalCommand)
	{
		if (AuxinRetained_Tx_Ptr == NULL) // sanity check - should have valid pointer when we get here
		{
			LOG_ERR("Tried to respond to AuxInDisConnect, but had no valid message pointer\n");
			return;
		}
		if (NSR_S_OK == result) // return is OK
		{
			AuxinRetained_Tx_Ptr->Operation = OP_RESULTACK; // response to start/result
			AuxinRetained_Tx_Ptr->Data[0] = AUXIN_SOURCE_NR; // this is the only parameter we send back
			AuxinRetained_Tx_Ptr->Length = 1; //
		}
		else
		{
			AuxinRetained_Tx_Ptr->Operation = OP_ERRORACK; // response to get
			AuxinRetained_Tx_Ptr->Data[0] = ERR_PROCESSING; // error processing this request
			AuxinRetained_Tx_Ptr->Length = 1; //
		}
		CmdInsertSenderHandle(AuxinRetained_Tx_Ptr, HB(SenderHandle), LB(SenderHandle));  // Moves parameters over and fixes length.
		MsgSend(AuxinRetained_Tx_Ptr); // send response now
		AuxinRetained_Tx_Ptr = NULL; // pointer no longer valid - MsgSend will free the message after sending
		NtfPropertyChanged(&NtfMatrix_Auxin[0][0], 1); // trigger notification, we've allocated
	}
	ADC_connection.LocalCommand = FALSE;  // no longer doing local connection (if we were)
}


/****************************************************************************
*  Function:       	Auxin_SourceActivity_StartResult
*
*  Description:
*
*  Input(s):       	ptr on tx message
*					ptr on rx message
*
*  Side-effect(s): 	-none-
*
*  Return(s):      	OP_Type
*
****************************************************************************/
uint8_t Auxin_SourceActivity_StartResult(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{

	/* parameters:  SenderHandle (word  0:1)
	SourceNr, (byte) 2
	Activity, (byte) 3
	*/

	if (Rx_Ptr->Data[2] != AUXIN_SOURCE_NR)
	return(CmdErrorParamWrong(Tx_Ptr, 2, &Rx_Ptr->Data[2], 1));

	DECODE_WORD(&SenderHandle, &Rx_Ptr->Data[0]);
	if (ADC_connection.ConnectionHandle != INVALID_CONNECTION) // are we connected?
	{
		switch (Rx_Ptr->Data[3])
		// switch on activity parameter
		{
			case SRC_ACTIVITY_OFF:
			{
				Auxin.SrcActivity = SRC_ACTIVITY_OFF;
				ScmMuteConnection(ADC_connection.ConnectionHandle, NULL); // mute - no callback
				break;
			}
			case SRC_ACTIVITY_ON:
			{
				Auxin.SrcActivity = SRC_ACTIVITY_ON;
				ScmDemuteConnection(ADC_connection.ConnectionHandle, NULL);
				break;
			}
			default:
			return (CmdErrorParamWrong(Tx_Ptr, 3, &Rx_Ptr->Data[3], 1));
			break;
		}
	}
	else
	{
		Auxin.SrcActivity = SRC_ACTIVITY_OFF; // not connected, therefore OFF
	}

	Tx_Ptr->Data[0] = AUXIN_SOURCE_NR;
	Tx_Ptr->Data[1] = Auxin.SrcActivity;
	Tx_Ptr->Length = 0x02;
	CmdInsertSenderHandle(Tx_Ptr, HB(SenderHandle), LB(SenderHandle));  // Moves parameters over and fixes length.

	return (OP_RESULTACK);
}

/****************************************************************************
*  Function:       	Auxin_SourceName_Get
*
*  Description:
*
*  Input(s):       	ptr on tx message
*					ptr on rx message
*
*  Side-effect(s): 	-none-
*
*  Return(s):      	OP_Type
*
****************************************************************************/
uint8_t Auxin_SourceName_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{

	if (Rx_Ptr->Data[0] != AUXIN_SOURCE_NR)
	return(CmdErrorParamWrong(Tx_Ptr, 1, &Rx_Ptr->Data[0], 1));

	Tx_Ptr->Data[0] = AUXIN_SOURCE_NR;
	Tx_Ptr->Data[1] = ISO8859;
	Tx_Ptr->Data[2] = 'A';
	Tx_Ptr->Data[3] = 'u';
	Tx_Ptr->Data[4] = 'd';
	Tx_Ptr->Data[5] = 'i';
	Tx_Ptr->Data[6] = 'o';
	Tx_Ptr->Data[7] = ' ';
	Tx_Ptr->Data[8] = 'I';
	Tx_Ptr->Data[9] = 'n';
	Tx_Ptr->Data[10] = 0x00;
	Tx_Ptr->Length = 11;

	return (OP_STATUS);
}

/****************************************************************************
*  Function:       	Auxin_SyncDataInfo_Get
*
*  Description:
*
*  Input(s):       	ptr on tx message
*					ptr on rx message
*
*  Side-effect(s): 	-none-
*
*  Return(s):      	OP_Type
*
****************************************************************************/
uint8_t Auxin_SyncDataInfo_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{

	Tx_Ptr->Data[0] = 0x01; // Source Count
	Tx_Ptr->Data[1] = AUXIN_SOURCE_NR; // SrcNr
	Tx_Ptr->Data[1] = 0x00; // Sink Count
	Tx_Ptr->Length = 0x03;

	return (OP_STATUS);
}



///////////////////////////////////////////////////////////////////////////////////



/********************************************************************************/
/************************ ADC Connection Process ********************************/
/********************************************************************************/
/*******************************************************************************
* function: ADC_INIC_CreateSocket
*
* descrip.: Create a socket from ADC to INIC
* params  : none
* returns : --
* effects : NetServices triggers ADC_INIC_CreateSocket_CB when done
*******************************************************************************/
uint8_t ADC_INIC_CreateSocket(void)
{
	TScmSocketDesc SocketDesc;
	uint8_t result;

	// create socket from INIC to ADC
	LOG_NOR("Creating ADC to INIC socket...\n");
	SocketDesc.port_id = SCM_PORT_ID_STREAM;
	SocketDesc.direction = SCM_IN;
	SocketDesc.datatype = SCM_TYPE_SYNC;
	SocketDesc.blockwidth = 0x0004;
	SocketDesc.streaming.interface_id = SCM_STREAM_INTERFACE_SR0;
	SocketDesc.streaming.offset = 0x00;
	result = ScmCreateSocket(&SocketDesc, ADC_INIC_CreateSocket_CB);
	return (result);
}

/*******************************************************************************
* function: ADC_INIC_CreateSocket_CB
*
* descrip.: MNS callback function
* params  : result of socket creation process, respective socket handle, length
*           of channel list and pointer to channel list
* returns : --
* effects : moves state machine on to CREATE_ADC_INIC state
*******************************************************************************/
void ADC_INIC_CreateSocket_CB(TMnsResult cb_result, uint8_t handle, uint8_t list_len, uint16_t *list_ptr)
{

	uint8_t result;

	if (NSR_S_OK == cb_result)
	{
		LOG_NOR("ADC->INIC Socket created OK\n");
		ADC_connection.INIC_handle = handle; // our socket handle for INIC side
		// won't have a list_len, or list_ptr for INIC socket - only a handle
		RED_LED_OFF(); // turn off error light if it was on

		/* now go create the network side socket */
		result = ADC_Network_CreateSocket();
		if (ERR_NO != result)
		{
			ConmgrReturnMutex();  //we're quitting (failed), return mutex, stop timeout timer
			LOG_ERR("Error starting ADC->Network socket creation: result = %02X\n", result);
			Auxin_Allocate_Result(NSR_E_FAILED);
		}

	}
	else
	{
		ConmgrReturnMutex();  //we're quitting (failed), return mutex, stop timeout timer
		LOG_ERR("Error Creating ADC->INIC Socket: Error Code = %04X\n", cb_result);
		ADC_connection.INIC_handle = INVALID_SOCKET;
		RED_LED_ON(); // turn on error light
		Auxin_Allocate_Result(NSR_E_FAILED);
	}
}



/*******************************************************************************
* function: ADC_Network_CreateSocket
*
* descrip.: Create a socket from INIC to Network for ADC streaming data
* params  : none
* returns : --
* effects : NetServices triggers ADC_Network_CreateSocket_CB when done
*******************************************************************************/
uint8_t ADC_Network_CreateSocket(void)
{
	uint8_t result;
	TScmSocketDesc SocketDesc;

	LOG_NOR("Creating ADC INIC to Network socket...\n");
	SocketDesc.port_id = SCM_PORT_ID_MOST;
	SocketDesc.direction = SCM_OUT;
	SocketDesc.datatype = SCM_TYPE_SYNC;
	SocketDesc.blockwidth = 0x0004;
	SocketDesc.most.flags = ADC_connection.ConnectType;
	SocketDesc.most.result_list_ptr = scm_buf;

	result = ScmCreateSocket(&SocketDesc, ADC_Network_CreateSocket_CB);
	return (result);
}

/*******************************************************************************
* function: ADC_Network_CreateSocket_CB
*
* descrip.: MNS callback function
* params  : result of socket creation process, respective socket handle, length
*           of channel list and pointer to channel list
* returns : --
* effects : if socket created successfully, moves state machine on to CONNECT_ADC when done
*******************************************************************************/
void ADC_Network_CreateSocket_CB(TMnsResult cb_result, uint8_t handle, uint8_t list_len, uint16_t *list_ptr)
{
	uint8_t result;

	if (NSR_S_OK == cb_result)
	{
		LOG_NOR("ADC->NETWORK Socket created OK\n");
		RED_LED_OFF(); // turn off error light if it was on

		if (list_ptr)
		{
			ADC_connection.Network_handle = handle; // our socket handle for NETWORK side
			ADC_connection.ConnectionLabel = *list_ptr; // get connection label from list
			LOG_NOR("Connection Label for AuxIn is: %04X\n", ADC_connection.ConnectionLabel);

			Auxin.pConnectionLabel = ADC_connection.ConnectionLabel; // device's copy

			/* now go connect the sockets */
			result = ADC_ConnectSockets();
			if (ERR_NO != result)
			{
				ConmgrReturnMutex();  //we're quitting (failed), return mutex, stop timeout timer
				LOG_ERR("Error starting ADC->Network connect sockets: result = %02X\n", result);
				Auxin_Allocate_Result(NSR_E_FAILED);
			}
		}
		else
		{
			ConmgrReturnMutex();  //we're quitting (failed), return mutex, stop timeout timer
			LOG_ERR("List Pointer is NULL - can't get Connection Label \n");
			Auxin_Allocate_Result(NSR_E_FAILED);
		}
	}
	else
	{
		ConmgrReturnMutex();  //we're quitting (failed), return mutex, stop timeout timer
		LOG_ERR("Error Creating ADC->NETWORK Socket: Error Code = %04X\n", cb_result);
		ADC_connection.Network_handle = INVALID_SOCKET;
		ADC_connection.ConnectionLabel = INVALID_LABEL;

		Auxin_Allocate_Result(cb_result); // callback to FBlock AuxIn that will send "failed" response
		RED_LED_ON(); // turn on error light
	}
}


/* ----------------------------------------------- Connect Sockets -------------------------------------------------------------*/

/*******************************************************************************
* function: ADC_ConnectSockets
*
* descrip.: Connects the ADC to the MOST Network - INIC & NETWORK sockets must
*           already be created
* params  : none
* returns : --
* effects : NetServices triggers ADC_ConnectSockets_CB when done
*******************************************************************************/
uint8_t ADC_ConnectSockets(void)
{
	uint8_t result;

	/* connect ADC sockets */
	LOG_NOR("Connecting ADC to Network...\n");
	result = ScmConnectSockets(ADC_connection.INIC_handle, ADC_connection.Network_handle, ADC_ConnectSockets_CB);
	return (result);
}


/*******************************************************************************
* function: ADC_ConnectSockets_CB
*
* descrip.: MNS callback function
* params  : result of socket connection process and respective connection handle
* returns : --
* effects : moves state machine back to IDLE when done
*******************************************************************************/
void ADC_ConnectSockets_CB(TMnsResult cb_result, uint8_t connection_handle)
{
	ConmgrReturnMutex();  //we're done, (pass or fail), return mutex, stop timeout timer
	if (NSR_S_OK == cb_result) // sockets connected OK
	{
		LOG_NOR("ADC Sockets Connected OK\n");
		ADC_connection.ConnectionHandle = connection_handle;
		GREEN_LED_ON();
		RED_LED_OFF(); // turn off error light if it was on
	}
	else
	{
		LOG_ERR("Error Connecting ADC Sockets: Error Code = %04X\n", cb_result);
		ADC_connection.ConnectionHandle = INVALID_CONNECTION;
		RED_LED_ON(); // turn on error light
	}
	Auxin_Allocate_Result(cb_result); // callback to FBlock AuxIn that will send response
}



/* ----------------------------------------------- DisConnect Sockets -------------------------------------------------------------*/

/*******************************************************************************
* function: ADC_DisConnectSockets
*
* descrip.: DisConnects the ADC from the MOST Network - killing the connection
*           stops the streaming data - INIC & NETWORK sockets still exist
* params  : none
* returns : Pass / Fail code
* effects : NetServices triggers ADC_DisConnectSockets_CB when done
*******************************************************************************/
uint8_t ADC_DisConnectSockets(void)
{
	uint8_t result;

	LOG_NOR("Dis-connecting ADC from Network...\n");
	result = ScmDisconnectSockets(ADC_connection.ConnectionHandle, ADC_DisConnectSockets_CB); // only needs handle to disconnect socket
	return (result);
}

/*******************************************************************************
* function: ADC_DisConnectSockets_CB
*
* descrip.: MNS callback function
* params  : result of socket dis connect process
* returns : --
* effects : moves state machine on to DESTROY_NET_OUT when done
*******************************************************************************/
void ADC_DisConnectSockets_CB(TMnsResult cb_result)
{
	uint8_t result;

	if (NSR_S_OK == cb_result) // sockets dis-connected OK
	{
		LOG_NOR("ADC Sockets DisConnected OK\n");
		ADC_connection.ConnectionHandle = INVALID_CONNECTION;
		GREEN_LED_OFF();
		result = ADC_Network_DestroySocket(); // now release the bandwidth
		if (ERR_NO != result)
		{
			ConmgrReturnMutex();  //we're quitting (failed), return mutex, stop timeout timer
			LOG_ERR("Error starting ADC->Network socket destruction: result = %02X\n", result);
			Auxin_DeAllocate_Result(NSR_E_FAILED);
		}
		RED_LED_OFF(); // turn off error light if it was on
	}
	else
	{
		ConmgrReturnMutex();  //we're quitting (failed), return mutex, stop timeout timer
		LOG_ERR("Error DisConnecting ADC Sockets: Error Code = %04X\n", cb_result);
		Auxin_DeAllocate_Result(cb_result); // callback to FBlock AuxIn to send "failed" result
		RED_LED_ON(); // turn on error light
	}
}


/*******************************************************************************
* function: ADC_Network_DestroySocket
*
* descrip.: Kills the Network socket which releases the bandwidth used by ADC
*
* params  : none
* returns : result pass/fail
* effects : NetServices triggers ADC_Network_DestroySocket_CB when done
*******************************************************************************/
uint8_t ADC_Network_DestroySocket(void)
{
	uint8_t result;

	LOG_NOR("Destroying ADC to Network socket...\n");
	result = ScmDestroySocket(ADC_connection.Network_handle, ADC_Network_DestroySocket_CB);
	return (result);
}

/*******************************************************************************
* function: ADC_Network_DestroySocket_CB
*
* descrip.: MNS callback function
* params  : result of destroy socket process
* returns : --
* effects : moves state machine back to IDLE when done
*******************************************************************************/
void ADC_Network_DestroySocket_CB(TMnsResult cb_result)
{

	ConmgrReturnMutex();  //we're done now (pass or fail), return mutex, stop timeout timer
	if (NSR_S_OK == cb_result) // return is OK
	{
		LOG_NOR("ADC Network Socket Destroyed OK\n");
		ADC_connection.Network_handle = INVALID_SOCKET; // socket killed
		ADC_connection.ConnectionLabel = INVALID_LABEL; // AND connection label is also dead
		RED_LED_OFF(); // turn off error light if it was on
	}
	else
	{
		LOG_ERR("Error Destroying ADC Network Socket: Error Code = %04X\n", cb_result);
		RED_LED_ON(); // turn on error light
	}
	Auxin_DeAllocate_Result(cb_result); // callback to FBlock AuxIn to send appropriate result
}


/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*                          									            */
/* General-Functions                                						*/
/*                                        									*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/


void AuxNotifyService(void)
{
	if (Auxin.nConnectionLabel != Auxin.pConnectionLabel)
	{
		NtfPropertyChanged(&NtfMatrix_Auxin[0][0], 1);
		Auxin.nConnectionLabel = Auxin.pConnectionLabel;
	}
}

