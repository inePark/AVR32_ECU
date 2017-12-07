
/*****************************************************
Module  : FBlock - for Amplifier Function
Version : 2.0
File    : fblock_amp.c
Date    : 11/28/2006
Author  : Gary Warren
Company : SMSC - AIS
Comments:

Description:
    Implements the function block Amplifier.
All Amplifier specific functions are here also.


Modifications:

10/08/2010  GW  Update for MOST Spec 3 and ACK OpTypes.
                Connection logic is moved into here from con_mgr.

03/10/2010  GW  Modified for Eval110 board - does not have EEPROM for saving High
                and Low temperature limits. Save in Data Storage registers of MPM85000.
                These registers are non-volatile through power cycles, but lost when
                12V main power is removed.

10/25/2007	GW	Took out Bass / Treble functions for Eval50 ST board -
				Power Management boards use MAXIM DAC - doesn't have bass/treble
				Must be removed from notification / fblock table

------------------------------------------------------------------------------
    Include Files
------------------------------------------------------------------------------
*/

#include <stdint.h>
#include <asf.h>
#include "defines.h"
#include "mostns.h"

#include "board_utils.h"
#include "i2c_driver.h"

#include "codec.h"
#include "dac_max9850.h"

#include "fblock_pb_amp.h"
#include "fblock_pv_amp.h"
#include "con_mgr.h"


/*
------------------------------------------------------------------------------
    Local Definitions
------------------------------------------------------------------------------
*/

// Amp specific data
//------------------------
#define AMP_SINK_NR       	0x01		    //
#define AMP_CHANNELS      	0x04      		// Number of Audio-Channels
#define AMP_CLABEL          0x02

#define AMP_VERSION_MAJOR   1
#define AMP_VERSION_MINOR   0
#define AMP_VERSION_BUILD   0

#define TEMP_LO_ADDR     (uint8_t) 0x10
#define TEMP_HI_ADDR     (uint8_t) 0x11

#define DEFAULT_VOLUME   0x0a



// Amp.Volume                 				//
//------------------------            		//
#define VOLMAX          	40   			//	direct mapping to DAC

/*
------------------------------------------------------------------------------
    Local variables and buffers
------------------------------------------------------------------------------
*/

TAmp Amp; // Structure containing all properties and methods
CONNECTION DAC_connection;
static uint16_t SenderHandle;

TNtfMatrix NtfMatrix_Amp[NUM_NTF_PROP_AMP][NTF_SIZE_DEVICE_TAB + 1]; // Notification Matrix of FBlock
struct Msg_Tx_Type *AmpRetained_Tx_Ptr; // global for delayed responses


/*
------------------------------------------------------------------------------
    Local Function Prototypes
------------------------------------------------------------------------------
*/

void Send_Amp_Result(struct Msg_Tx_Type *Tx_Ptr);
void Set_Volume(void);
void Amp_ConnectTimeout(void);
void Amp_DisconnectTimeout(void);

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

#include "t_fblock_amp.tab"      				// Table containing all FUNC_IDS and all OP_TYPES of FBlock



/****************************************************************************
*  Function:       void Amp_Init(void);
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
void Init_Amp(void)
{
	uint8_t i;
	uint8_t j;

	/* Initialize connection properties */
	Amp_KillConnections();
	DAC_connection.INIC_handle = INVALID_SOCKET;

	// Init pMute
	Amp.pMute = FALSE;
	CODEC_demute();
	//DacDemute();

	// Init pVolume
	//Amp.pVolume = DEFAULT_VOLUME; // start off -12dB
	Amp.pVolume = AVN_DEFAULT_VOLUME;	//netbugger AVN default Volume
	Set_Volume();

	// Init pConnectionLabel
	Amp.pConnectionLabel = INVALID_LABEL;

	// Init Amp Temperature varaibles
	Amp.O_Temp = FALSE;


	#ifdef NTF_MIN
	// Init Notification Matrix
	for (i=0;i<NUM_NTF_PROP_AMP;i++) // Number of Properties

	{
		for (j=0;j<NTF_SIZE_DEVICE_TAB+1;j++) // Number of entries in the Device Index Table for each property
		NtfMatrix_Amp[i][j] = (TNtfMatrix)NTF_DEV_INDEX_FREE; // clear whole notification matrix of FBlock
	}
	#endif
}


/*
*  FUNCTION:       Amp_KillConnections
*
*  PARAMETERS:     none
*
*  DESCRIPTION:    Sets all network side connection parameters for Stereo Amp to "not connected".
*                  when network dies, these connections die automatically - flag that they are invalid
*                  Also used at startup to flag all network connections are "not connected"
*
*  RETURNS:        nothing
*
*/
void Amp_KillConnections(void)
{
	LOG_NOR("Initializing Amp connections\n");
	DAC_connection.Network_handle = INVALID_SOCKET;
	DAC_connection.ConnectionLabel = INVALID_LABEL; // MOST50
	DAC_connection.ConnectionHandle = INVALID_CONNECTION; // MOST25 & MOST50 - same
	DAC_connection.Connecting = FALSE; // not currently working on connecting
	DAC_connection.LocalCommand = FALSE; // not doing stuff on our own
	YELLOW2_LED_OFF(); // DAC connection LED should be off

	AmpRetained_Tx_Ptr = NULL; // no messages waiting for response
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Section needed by Rx Command Interpreter
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


/****************************************************************************
*  Function:       uint8_t FuncIDs_Get(struct Msg_Tx_Type *Tx_Ptr);
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
uint8_t Amp_FuncIDs_Get(struct Msg_Tx_Type *Tx_Ptr)
{
	CmdGetFunctionIds(Tx_Ptr, Func_Amp);
	return (OP_STATUS);
}


/****************************************************************************
*  Function:       	uint8_t Notification_Set(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
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
uint8_t Amp_Notification_Set(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{
	#ifdef NTF_MIN
	return(NtfSetNotificationMatrix(Tx_Ptr,Rx_Ptr));
	#else
	return (NIL);
	#endif
}


/****************************************************************************
*  Function:       	uint8_t Notification_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
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
uint8_t Amp_Notification_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{
	#ifdef NTF_MIN
	return(NtfGetNotificationMatrix(Tx_Ptr,Rx_Ptr));
	#else
	return (NIL);
	#endif
}


/****************************************************************************
*  Function:       	NotificationCheck_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
*
*  Description:    	Check if device is in the Notification-List
*
*  Input(s):       	ptr on tx message
*					ptr on rx message
*
*  Side-effect(s): 	-none-
*
*  Return(s):      	OP_Type
*
****************************************************************************/
uint8_t Amp_NotificationCheck_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{
	#ifdef NTF_MIN
	return(NtfCheck(Tx_Ptr,Rx_Ptr));
	#else
	return (NIL);
	#endif
}


/****************************************************************************
*  Function:        Amp_FBlockInfo_Get(struct Msg_Tx_Type *Tx_Ptr);
*
*  Description:     Returns information about our FBlock
*
*  Input(s):       	ptr on tx message
*					ptr on rx message
*
*  Side-effect(s): 	-none-
*
*  Return(s):       Status - depending on index passed in
*
****************************************************************************/
uint8_t Amp_FBlockInfo_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{
	uint16_t rx_id;
	uint8_t  name[] = "AudioAmp";
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

			case FUNC_SINKINFO:
			case FUNC_CONNECT:
			case FUNC_DISCONNECT:
			case FUNC_MUTE:
			case FUNC_SINKNAME:
			case FUNC_SYNCDATAINFO:
			case AMP_VOLUME:
			case AMP_EVENTS:
			case AMP_TEMPERATURE:
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
			version[0] = AMP_VERSION_MAJOR;
			version[1] = AMP_VERSION_MINOR;
			version[2] = AMP_VERSION_BUILD;

			Tx_Ptr->Data[2] = FRMT_ISO8859;
			i = MsgVersionToISO8859(&version[0], &Tx_Ptr->Data[3]);
			Tx_Ptr->Length  = (word)((word)i + (word)3);
			break;

			case FBI_FBLOCK_VERSION:
			version[0] = AMP_VERSION_MAJOR;
			version[1] = AMP_VERSION_MINOR;
			version[2] = AMP_VERSION_BUILD;

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
*  Function:       	uint8_t SinkInfo_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
*
*  Description:    	Get the SinkInfo
*
*  Input(s):       	ptr on tx message
*					ptr on rx message
*
*  Side-effect(s): 	-none-
*
*  Return(s):      	OP_Type
*
****************************************************************************/
uint8_t Amp_SinkInfo_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{
	// parameter checking - 1 parameter -> SinkNr
	if (Rx_Ptr->Data[0] != AMP_SINK_NR)
	return(CmdErrorParamWrong(Tx_Ptr, 1, &Rx_Ptr->Data[0], 1));

	return (Amp_SinkInfo_Get_N(Tx_Ptr)); // Let the notification function do the work below
}

/****************************************************************************
*  Function:       	uint8_t SinkInfo_Get_N(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
*
*  Description:    	Get the SinkInfo - called from above, or from Notification Matrix
*
*  Input(s):       	ptr on tx message
*					ptr on rx message
*
*  Side-effect(s): 	-none-
*
*  Return(s):      	OP_Type
*
****************************************************************************/
uint8_t Amp_SinkInfo_Get_N(struct Msg_Tx_Type *Tx_Ptr)
{

	Tx_Ptr->Data[0] = AMP_SINK_NR; // SinkNumber
	Tx_Ptr->Data[1] = 0;                                        // MSB of blockwidth
	Tx_Ptr->Data[2] = AUDIO_STEREO * RES_16BIT;                 // LSB of blockwitdh - only support stereo (4 bytes)
	Tx_Ptr->Data[3] = (u8_t) (DAC_connection.ConnectionLabel>>8);   // MSB of connection label
	Tx_Ptr->Data[4] = (u8_t) DAC_connection.ConnectionLabel;        // LSB of connection label
	Tx_Ptr->Data[5] = 0;                                        // Transmission Class 0 = Synchronous
	Tx_Ptr->Data[6] = 0;                                        // Content Protection 0 = None
	Tx_Ptr->Data[7] = 0;                                        // Content Type 0 = Audio
	Tx_Ptr->Data[8] = 2;                                        // Content Description, short stream 2 bytes follow
	Tx_Ptr->Data[9] = AUDIO_STEREO;                             // AudioChannels 2 = Stereo
	Tx_Ptr->Data[10] = RES_16BIT;                                // Resolution 2 = 16-Bit Audio

	Tx_Ptr->Length  = 11;
	return (OP_STATUS);
}


/****************************************************************************
*  Function:       	uint8_t Amp_Connect_StartResult(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
*
*  Description:    	Connect Most Network Channels to Amplifier
*
*  Input(s):       	ptr on tx message
*					ptr on rx message
*
*  Side-effect(s): 	-none-
*
*  Return(s):      	OP_Type
*
****************************************************************************/
uint8_t Amp_Connect_StartResult(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{
	uint8_t result;
	uint16_t newConnection;
	uint16_t blockWidth;

	/* parameters:  SenderHandle (word  0:1)
	SinkNr, (byte) 2
	BlockWidth (word) 3:4
	ConnectionLabel (word) 5:6
	*/

	if (Rx_Ptr->Data[0] != AMP_SINK_NR)
	return(CmdErrorParamWrong(Tx_Ptr, 0, &Rx_Ptr->Data[0], 1));

	//DECODE_WORD(&SenderHandle, &Rx_Ptr->Data[0]);
	DECODE_WORD(&blockWidth, &Rx_Ptr->Data[2]);
	DECODE_WORD(&newConnection, &Rx_Ptr->Data[4]);
	LOG_NOR("Connection Label for Stereo Amp is: %04X\n", newConnection);
	
	if (AUDIO_STEREO*RES_16BIT != blockWidth)
	{
		return (CmdErrorParamWrong(Tx_Ptr, 2, &Rx_Ptr->Data[2], 2));
	}

	if (newConnection != DAC_connection.ConnectionLabel)
	{
		if (ConmgrGetMutex(500, Amp_ConnectTimeout))
		{
			/* We're establishing new connection or changing connections */
			DAC_connection.ConnectionLabel = newConnection;
			DAC_connection.Connecting = TRUE; // flag that we're in connection process in case we need to disconnect first
			if (INVALID_CONNECTION == DAC_connection.ConnectionHandle) // already connected?
			{ // if not, just do the connection
				result = DAC_INIC_CreateSocket();  // that starts connection process -
			}
			else
			{ // if so disconnect - then disconnect will check "Connecting" flag & contiune w/ connection
				result = DAC_DisConnectSockets(); // must disconnect first - then connect
			}

			if (ERR_NO == result)
			{
				AmpRetained_Tx_Ptr = Tx_Ptr; // keep global pointer to return message - returned later
				return (CMD_TX_RETAIN); // sending result later in Auxin_Allocate_Result
			}
			else
			{
				ConmgrReturnMutex();  //we're quitting, return mutex, stop timeout timer
//				RED_LED_ON();
				DAC_connection.ConnectionLabel = INVALID_LABEL;  // not connected to newConnection
				LOG_NOR("Error starting DAC->INIC socket creation: result = %02X\n", result);
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
	else
	{
		/* already connected to that connection label */
		Tx_Ptr->Operation = OP_RESULT; // response to start/result
		Tx_Ptr->Data[0] = AMP_SINK_NR; // this is the only parameter we send back
		Tx_Ptr->Length = 1; //
		//CmdInsertSenderHandle(Tx_Ptr, HB(SenderHandle), LB(SenderHandle));  // Moves parameters over and fixes length.
		return (OP_RESULT);  // Just report connection was OK
	}
}

/****************************************************************************
*  Function:       	uint8_t Amp_Connect_StartResultACK(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
*
*  Description:    	Connect Most Network Channels to Amplifier
*
*  Input(s):       	ptr on tx message
*					ptr on rx message
*
*  Side-effect(s): 	-none-
*
*  Return(s):      	OP_Type
*
****************************************************************************/
uint8_t Amp_Connect_StartResultACK(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{
	uint8_t result;
	uint16_t newConnection;
	uint16_t blockWidth;

	/* parameters:  SenderHandle (word  0:1)
	SinkNr, (byte) 2
	BlockWidth (word) 3:4
	ConnectionLabel (word) 5:6
	*/

	if (Rx_Ptr->Data[2] != AMP_SINK_NR)
	return(CmdErrorParamWrong(Tx_Ptr, 2, &Rx_Ptr->Data[2], 1));

	DECODE_WORD(&SenderHandle, &Rx_Ptr->Data[0]);
	DECODE_WORD(&blockWidth, &Rx_Ptr->Data[3]);
	DECODE_WORD(&newConnection, &Rx_Ptr->Data[5]);
	LOG_NOR("Connection Label for Stereo Amp is: %04X\n", newConnection);
	
	if (AUDIO_STEREO*RES_16BIT != blockWidth)
	{
		return (CmdErrorParamWrong(Tx_Ptr, 3, &Rx_Ptr->Data[3], 2));
	}

	if (newConnection != DAC_connection.ConnectionLabel)
	{
		if (ConmgrGetMutex(500, Amp_ConnectTimeout))
		{
			/* We're establishing new connection or changing connections */
			DAC_connection.ConnectionLabel = newConnection;
			DAC_connection.Connecting = TRUE; // flag that we're in connection process in case we need to disconnect first
			if (INVALID_CONNECTION == DAC_connection.ConnectionHandle) // already connected?
			{ // if not, just do the connection
				result = DAC_INIC_CreateSocket();  // that starts connection process -
			}
			else
			{ // if so disconnect - then disconnect will check "Connecting" flag & contiune w/ connection
				result = DAC_DisConnectSockets(); // must disconnect first - then connect
			}

			if (ERR_NO == result)
			{
				AmpRetained_Tx_Ptr = Tx_Ptr; // keep global pointer to return message - returned later
				return (CMD_TX_RETAIN); // sending result later in Auxin_Allocate_Result
			}
			else
			{
				ConmgrReturnMutex();  //we're quitting, return mutex, stop timeout timer
//				RED_LED_ON();
				DAC_connection.ConnectionLabel = INVALID_LABEL;  // not connected to newConnection
				LOG_ERR("Error starting DAC->INIC socket creation: result = %02X\n", result);
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
	else
	{
		/* already connected to that connection label */
		Tx_Ptr->Operation = OP_RESULT; // response to start/result
		Tx_Ptr->Data[0] = AMP_SINK_NR; // this is the only parameter we send back
		Tx_Ptr->Length = 1; //
		CmdInsertSenderHandle(Tx_Ptr, HB(SenderHandle), LB(SenderHandle));  // Moves parameters over and fixes length.
		return (OP_RESULTACK);  // Just report connection was OK
	}
}

void Amp_ConnectTimeout(void)
{
	LOG_NOR("Timeout while connecting stereo DAC\n");
	Amp_Connect_Result(NSR_E_FAILED);
}

/****************************************************************************
*  Function:       	uint8_t AmpConnect_Result(word result);
*
*  Description:    	Callback from connection manager after response from INIC
*
*  Input(s):       	result of Connection request
*
*  Side-effect(s): 	-none-
*
*  Return(s):      	-none-
*
****************************************************************************/
void Amp_Connect_Result(uint16_t result)
{

	if (NULL == AmpRetained_Tx_Ptr) // sanity check - should have valid pointer when we get here
	{
		LOG_ERR("Tried to respond to AmpConnect, but had no valid message pointer\n");
		return;
	}

	if (NSR_S_OK == result) // return is OK
	{
		AmpRetained_Tx_Ptr->Operation = OP_RESULTACK; // response to start/result
		AmpRetained_Tx_Ptr->Data[0] = AMP_SINK_NR; // this is the only parameter we send back
		AmpRetained_Tx_Ptr->Length = 1; //
	}
	else
	{
		AmpRetained_Tx_Ptr->Operation = OP_ERRORACK; // response to get
		AmpRetained_Tx_Ptr->Data[0] = ERR_PROCESSING; // processing error
		AmpRetained_Tx_Ptr->Length = 1; //
	}
	DAC_connection.Connecting = FALSE; // done connecting now
	CmdInsertSenderHandle(AmpRetained_Tx_Ptr, HB(SenderHandle), LB(SenderHandle));  // Moves parameters over and fixes length.
	MsgSend(AmpRetained_Tx_Ptr); // send response now
	AmpRetained_Tx_Ptr = NULL; // pointer no longer valid - MsgSend will free the message after sending
	NtfPropertyChangedFkt(FBLOCK_AMP, 0, FUNC_SINKINFO); // trigger notification

}


/****************************************************************************
*  Function:       	uint8_t Amp_DisConnect_StartResult(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
*
*  Description:    	DisConnect Most Network Channels to DSP Inputs
*
*  Input(s):       	ptr on tx message
*					ptr on rx message
*
*  Side-effect(s): 	-none-
*
*  Return(s):      	OP_Type
*
****************************************************************************/
uint8_t Amp_DisConnect_StartResult(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{
	uint8_t result;

	if (Rx_Ptr->Data[0] != AMP_SINK_NR)
	return(CmdErrorParamWrong(Tx_Ptr, 0, &Rx_Ptr->Data[0], 1));

	// parameters OK
	//DECODE_WORD(&SenderHandle, &Rx_Ptr->Data[0]);
	if (DAC_connection.ConnectionHandle != INVALID_CONNECTION) // IF socket is connected - then disconnect
	{
		if (ConmgrGetMutex(300, Amp_DisconnectTimeout))
		{
			DAC_connection.Connecting = FALSE; // make sure this is false
			result = DAC_DisConnectSockets(); // start disconnect process
			if (ERR_NO == result)
			{
				AmpRetained_Tx_Ptr = Tx_Ptr; // keep global pointer to return message - returned later
				return (CMD_TX_RETAIN); // sending result later in Auxin_Allocate_Result
			}
			else
			{
				ConmgrReturnMutex();  //we're quitting, return mutex, stop timeout timer
				LOG_ERR("Error starting DAC disconnect sockets process: result = %02X\n", result);
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
		Tx_Ptr->Data[0] = AMP_SINK_NR; // this is the only parameter we send back
		Tx_Ptr->Length = 1; //
		//CmdInsertSenderHandle(Tx_Ptr, HB(SenderHandle), LB(SenderHandle));  // Moves parameters over and fixes length.
		return (OP_RESULT); // We can send the result now - we are already connected
	}
}

/****************************************************************************
*  Function:       	uint8_t Amp_DisConnect_StartResultACK(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
*
*  Description:    	DisConnect Most Network Channels to DSP Inputs
*
*  Input(s):       	ptr on tx message
*					ptr on rx message
*
*  Side-effect(s): 	-none-
*
*  Return(s):      	OP_Type
*
****************************************************************************/
uint8_t Amp_DisConnect_StartResultACK(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{
	uint8_t result;

	if (Rx_Ptr->Data[2] != AMP_SINK_NR)
	return(CmdErrorParamWrong(Tx_Ptr, 2, &Rx_Ptr->Data[2], 1));

	// parameters OK
	DECODE_WORD(&SenderHandle, &Rx_Ptr->Data[0]);
	if (DAC_connection.ConnectionHandle != INVALID_CONNECTION) // IF socket is connected - then disconnect
	{
		if (ConmgrGetMutex(300, Amp_DisconnectTimeout))
		{
			DAC_connection.Connecting = FALSE; // make sure this is false
			result = DAC_DisConnectSockets(); // start disconnect process
			if (ERR_NO == result)
			{
				AmpRetained_Tx_Ptr = Tx_Ptr; // keep global pointer to return message - returned later
				return (CMD_TX_RETAIN); // sending result later in Auxin_Allocate_Result
			}
			else
			{
				ConmgrReturnMutex();  //we're quitting, return mutex, stop timeout timer
				LOG_ERR("Error starting DAC disconnect sockets process: result = %02X\n", result);
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
		Tx_Ptr->Data[0] = AMP_SINK_NR; // this is the only parameter we send back
		Tx_Ptr->Length = 1; //
		CmdInsertSenderHandle(Tx_Ptr, HB(SenderHandle), LB(SenderHandle));  // Moves parameters over and fixes length.
		return (OP_RESULTACK); // We can send the result now - we are already connected
	}
}

void Amp_DisconnectTimeout(void)
{
	LOG_ERR("Timed out while disconnecting stereo DAC from Network\n");
	Amp_DisConnect_Result(NSR_E_FAILED);
}


/****************************************************************************
*  Function:       	uint8_t AmpDisConnect_Result(uint16_t result);
*
*  Description:    	Callback from connection manager after response from INIC
*
*  Input(s):       	result of DisConnect request
*
*  Side-effect(s): 	-none-
*
*  Return(s):      	-none-
*
****************************************************************************/
void Amp_DisConnect_Result(uint16_t result)
{

	if (DAC_connection.LocalCommand) // if we're doing this on our own (not commanded to)
	{ // ... then don't reply
		DAC_connection.LocalCommand = FALSE; // no longer working on local command (if we were)
	}
	else // reply to whoever told us to disconnect
	{
		if (AmpRetained_Tx_Ptr == NULL) // sanity check - should have valid pointer when we get here
		{
			LOG_ERR("Tried to respond to AmpDisConnect, but had no valid message pointer\n");
			return;
		}

		if (result == NSR_S_OK) // return is OK
		{
			DAC_connection.ConnectionLabel = INVALID_LABEL; // this label no longer in use
			AmpRetained_Tx_Ptr->Operation = OP_RESULTACK; // response to start/result
			AmpRetained_Tx_Ptr->Data[0] = AMP_SINK_NR; // this is the only parameter we send back
			AmpRetained_Tx_Ptr->Length = 1; //
		}
		else
		{
			AmpRetained_Tx_Ptr->Operation = OP_ERRORACK; // response to get
			AmpRetained_Tx_Ptr->Data[0] = ERR_PROCESSING; // error processing this request
			AmpRetained_Tx_Ptr->Length = 1; //
		}

		CmdInsertSenderHandle(AmpRetained_Tx_Ptr, HB(SenderHandle), LB(SenderHandle));  // Moves parameters over and fixes length.
		MsgSend(AmpRetained_Tx_Ptr); // send response now
		AmpRetained_Tx_Ptr = NULL; // pointer no longer valid - MsgSend will free the message after sending
	}
	NtfPropertyChangedFkt(FBLOCK_AMP, 0, FUNC_SINKINFO); // trigger notification
}


/****************************************************************************
*  Function:       	uint8_t Amp_Mute_Set(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
*
*  Description:    	Set the Mute-Info
*
*  Input(s):       	ptr on tx message
*					ptr on rx message
*
*  Side-effect(s): 	-none-
*
*  Return(s):      	OP_Type
*
****************************************************************************/
uint8_t Amp_Mute_Set(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{

	if (Rx_Ptr->Data[0] != AMP_SINK_NR)
	return(CmdErrorParamWrong(Tx_Ptr, 1, &Rx_Ptr->Data[0], 1));

	if (DAC_connection.ConnectionHandle != INVALID_CONNECTION)
	{
		if (Rx_Ptr->Data[1] == 0x00) // Read Status
		{
			Amp.pMute = FALSE; // Set Flag Mute Off
			ScmDemuteConnection(DAC_connection.ConnectionHandle, NULL);
		}
		else if (Rx_Ptr->Data[1] == 0x01)
		{
			Amp.pMute = TRUE; // Set Flag for Mute
			ScmMuteConnection(DAC_connection.ConnectionHandle, NULL);
		}
		else
		{
			return (CmdErrorParamWrong(Tx_Ptr, 2, &Rx_Ptr->Data[1], 1));
		}
	}

	NtfPropertyChangedFkt(FBLOCK_AMP, 0, FUNC_MUTE); // trigger notification
	return (OP_NO_REPORT);

}



/****************************************************************************
*  Function:       	uint8_t Amp_Mute_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
*
*  Description:    	Get the Mute-Info
*
*  Input(s):       	ptr on tx message
*					ptr on rx message
*
*  Side-effect(s): 	-none-
*
*  Return(s):      	OP_Type
*
****************************************************************************/
uint8_t Amp_Mute_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{

	if (Rx_Ptr->Data[0] != AMP_SINK_NR)
	return(CmdErrorParamWrong(Tx_Ptr, 1, &Rx_Ptr->Data[0], 1));


	Tx_Ptr->Data[0] = AMP_SINK_NR; // SinkNumber
	Tx_Ptr->Data[1] = Amp.pMute; // Status
	Tx_Ptr->Length = 2;
	return (OP_STATUS);
}



/****************************************************************************
*  Function:       	void Mute_Get_N(struct Msg_Tx_Type *Tx_Ptr);
*
*  Description:    	Get the Mute-Info (Notification)
*
*  Input(s):       	ptr on tx message
*
*  Side-effect(s): 	-none-
*
*  Return(s):      	OP_Type
*
****************************************************************************/
uint8_t Amp_Mute_Get_N(struct Msg_Tx_Type *Tx_Ptr)
{
	Tx_Ptr->Data[0] = AMP_SINK_NR; // SinkNumber
	Tx_Ptr->Data[1] = Amp.pMute; // Status
	Tx_Ptr->Length = 2;
	return (OP_STATUS);
}



/****************************************************************************
*  Function:       	uint8_t Amp_SinkName_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
*
*  Description:    	Get the SinkName
*
*  Input(s):       	ptr on tx message
*					ptr on rx message
*
*  Side-effect(s): 	-none-
*
*  Return(s):      	OP_Type
*
****************************************************************************/
uint8_t Amp_SinkName_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{

	if (Rx_Ptr->Data[0] != AMP_SINK_NR)
	return(CmdErrorParamWrong(Tx_Ptr, 1, &Rx_Ptr->Data[0], 1));

	Tx_Ptr->Data[0] = AMP_SINK_NR;
	Tx_Ptr->Data[1] = ISO8859;
	Tx_Ptr->Data[2] = 'A';
	Tx_Ptr->Data[3] = 'u';
	Tx_Ptr->Data[4] = 'd';
	Tx_Ptr->Data[5] = 'i';
	Tx_Ptr->Data[6] = 'o';
	Tx_Ptr->Data[7] = ' ';
	Tx_Ptr->Data[8] = 'O';
	Tx_Ptr->Data[9] = 'u';
	Tx_Ptr->Data[10] = 't';
	Tx_Ptr->Data[11] = 0x00;
	Tx_Ptr->Length = 12;
	return (OP_STATUS);
}


/****************************************************************************
*  Function:       	Amp_SyncDataInfo_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
*
*  Description:    	Get the SinkName
*
*  Input(s):       	ptr on tx message
*					ptr on rx message
*
*  Side-effect(s): 	-none-
*
*  Return(s):      	OP_Type
*
****************************************************************************/
uint8_t Amp_SyncDataInfo_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{

	Tx_Ptr->Data[0] = 0; // No source resources for FBlock Amp
	Tx_Ptr->Data[1] = 1; // FBlock Amp has 1 sink connection
	Tx_Ptr->Data[2] = AMP_SINK_NR; // ...and that's the SinkNr

	Tx_Ptr->Length = 3; //

	return (OP_STATUS);

}

/****************************************************************************
*  Function:       	uint8_t Amp_Volume_Set(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
*
*  Description:    	Set the Volume to defined Value
*
*  Input(s):       	ptr on tx message
*					ptr on rx message
*
*  Side-effect(s): 	-none-
*
*  Return(s):      	OP_Type
*
****************************************************************************/
uint8_t Amp_Volume_Set(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{
	// Parameter checking - 1 parameter - volume setting
	if (Rx_Ptr->Data[0] > VOLMAX)
	return (CmdErrorParamWrong(Tx_Ptr, 1, &Rx_Ptr->Data[0], 1));

	if (MyNode.CodecPresent)
	{
		Amp.pVolume = Rx_Ptr->Data[0]; // Get the Volume from message
		Set_Volume();
		return (OP_NO_REPORT);
	}
	else
	{
		return (CmdErrorMsg(Tx_Ptr, I2C_ERROR));
	}
}



/****************************************************************************
*  Function:       	uint8_t Amp_Volume_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
*
*  Description:    	Get the current Volume setting
*
*  Input(s):       	ptr on tx message
*					ptr on rx message
*
*  Side-effect(s): 	-none-
*
*  Return(s):      	OP_Type
*
****************************************************************************/
uint8_t Amp_Volume_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{

	Tx_Ptr->Data[0] = Amp.pVolume; // Read the Volume
	Tx_Ptr->Length = 1;

	return (OP_RESULT);
}


/****************************************************************************
*  Function:       	uint8_t Amp_Volume_Inc(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
*
*  Description:    	Increment the Volume-Value
*
*  Input(s):       	ptr on tx message
*					ptr on rx message
*
*  Side-effect(s): 	Changes volume
*
*  Return(s):      	OP_Type
*
****************************************************************************/
uint8_t Amp_Volume_Inc(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{

	// step parameter is ignored - always inc 1 step
	if (MyNode.CodecPresent)
	{
		if (Amp.pVolume != VOLMAX)
		{
			Amp.pVolume += 1;
			Set_Volume();
		}
		return (OP_NO_REPORT);
	}
	else
	{
		return (CmdErrorMsg(Tx_Ptr, I2C_ERROR));
	}
}



/****************************************************************************
*  Function:       	uint8_t Amp_Volume_Dec(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
*
*  Description:    	Decrement the Volume-Value
*
*  Input(s):       	ptr on tx message
*					ptr on rx message
*
*  Side-effect(s): 	-none-
*
*  Return(s):      	OP_Type
*
****************************************************************************/
uint8_t Amp_Volume_Dec(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{

	// step parameter is ignored - always dec 1dB
	if (MyNode.CodecPresent)
	{
		if (Amp.pVolume != 0)
		{
			Amp.pVolume -= 1;
			Set_Volume();
		}

		return (OP_NO_REPORT);
	}
	else
	{
		return (CmdErrorMsg(Tx_Ptr, I2C_ERROR));
	}
}


/*-------------------------------------------------------------------------------------------------------------------*/
/*
*  FUNCTION: 		Amp_Events_Set
*  PARAMETERS:		rx_handle - pointer to received message that we are processing
*                  tx_handle - pointer to a tx message for response to this command if we need to respond
*  DESCRIPTION:	Has 3 parameters - Amp_Events to enable, and the hi/lo temperature values for temp alarms
*                  There is no response to a Set command
*  RETURNS:		Nothing
*/
/*-------------------------------------------------------------------------------------------------------------------*/
uint8_t Amp_Events_Set(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{
	// Parameter checking - 3 parameters - EventEnable, TempHi, TempLo

	// Cmd OK -
	Amp.EventMask = Rx_Ptr->Data[0]; // just read and save the mask - check against when we get events
	DacSetEvents(Amp.EventMask); // bits are in correct positions already -
//	PM_SetTempLimit(Rx_Ptr->Data[1], PM_TLIMHI_REG); // Set upper limit (could do error checking on input value)
//	PM_SetTempLimit(Rx_Ptr->Data[2], PM_TLIMLO_REG); // Set lower limit (could do error checking on input value)
	// Now read current Temperature limits
//	drvi2c_ByteAddrByteRead(PM_ADDR, PM_TLIMHI_REG, &Amp.HiTemp);
	LOG_NOR("Hi temperature limit: ");
	Amp_PrintTemp(Amp.HiTemp);
//	drvi2c_ByteAddrByteRead(PM_ADDR, PM_TLIMLO_REG, &Amp.LoTemp);
	LOG_NOR("Low temperature limit: ");
	Amp_PrintTemp(Amp.LoTemp);

	Rx_Ptr->Length = 0; // In case this message gets passed on the Events_Get - fix the length
	return (OP_NO_REPORT);
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
*  FUNCTION: 		Amp_Events_Get
*  PARAMETERS:		rx_handle - pointer to received message that we are processing
*                  tx_handle - pointer to a tx message for response to this command if we need to respond
*  DESCRIPTION:	Return Events, Amp Status, EventEnable
*                  Events are mostly from DAC Status A register (w/ temp status OR'ed in)
*                  Amp Status is DAC Status B
*                  EventEnable is what was sent in Amp_Events_Set command
*  RETURNS:		Nothing
*/
/*-------------------------------------------------------------------------------------------------------------------*/
uint8_t Amp_Events_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{
	// Parameter checking - no parameters

	return (Amp_Events_Get_N(Tx_Ptr)); // let the notification command do the work
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
*  FUNCTION: 		Amp_Events_Get_N
*  PARAMETERS:		tx_handle - pointer to a tx message for Events response message
*  DESCRIPTION:	Return Events, Amp Status, EventEnable
*                  Events are mostly from DAC Status A register (w/ temp status OR'ed in)
*                  Amp Status is DAC Status B
*                  EventEnable is what was sent in Amp_Events_Set command
*  RETURNS:		Nothing
*/
/*-------------------------------------------------------------------------------------------------------------------*/
uint8_t Amp_Events_Get_N(struct Msg_Tx_Type *Tx_Ptr)
{

	uint8_t temp = 0;

	//temp = DacGetStatus(); // gets Status A
	temp &= (1 << LCK | 1 << SHPS | 1 << VMIN | 1 << IOHL | 1 << IOHR); // only allow these sources - also clears bit 2 for Temp bit
	if (Amp.O_Temp)
	{
		temp |= TEMP_EVENT; // set bit 2
	}
	Tx_Ptr->Data[0] = temp; // That's Events
	//temp = DacGetStatusB(); //
	Tx_Ptr->Data[1] = temp; // Amp Status is direct from DAC
	Tx_Ptr->Data[2] = Amp.EventMask; // Just returning our current mask value - user sent earlier
	Tx_Ptr->Length = 3;
	return (OP_STATUS);
}


/********************************************************************************/
/************************ DAC Connection Process ********************************/
/********************************************************************************/


/*******************************************************************************
* function: DAC_INIC_CreateSocket
*
* descrip.: Create a socket from DAC to INIC
* params  : none
* returns : --
* effects : NetServices triggers DAC_INIC_CreateSocket_CB when done
*******************************************************************************/
uint8_t DAC_INIC_CreateSocket(void)
{
	uint8_t result;
	TScmSocketDesc SocketDesc;

	// create socket from INIC to DAC
	LOG_NOR("Creating DAC to INIC socket...\n");
	SocketDesc.port_id = SCM_PORT_ID_STREAM;
	SocketDesc.direction = SCM_OUT;
	SocketDesc.datatype = SCM_TYPE_STREAM;
	SocketDesc.blockwidth = 0x0004;
	SocketDesc.streaming.interface_id = SCM_STREAM_INTERFACE_SX2;
	SocketDesc.streaming.offset = 0x00;

	result = ScmCreateSocket(&SocketDesc, DAC_INIC_CreateSocket_CB);
	return (result);
}

/*******************************************************************************
* function: DAC_INIC_CreateSocket_CB
*
* descrip.: MNS callback function
* params  : result of socket creation process, respective socket handle, length
*           of channel list and pointer to channel list
* returns : --
* effects : if successful, calls DAC_Network_CreateSocket
*******************************************************************************/
void DAC_INIC_CreateSocket_CB(TMnsResult cb_result, uint8_t handle, uint8_t list_len, uint16_t *list_ptr)
{
	uint8_t result;

	if (NSR_S_OK == cb_result)
	{
		LOG_NOR("DAC->INIC Socket created OK\n");
		DAC_connection.INIC_handle = handle; // our socket handle for INIC side
		// won't have a list_len, or list_ptr for INIC socket - only a handle
		RED_LED_OFF(); // turn off error light if it was on
		result = DAC_Network_CreateSocket(); // INIC socket done, go create network socket
		if (ERR_NO != result)
		{
			ConmgrReturnMutex();  //we're quitting (failed), return mutex, stop timeout timer
			RED_LED_ON(); // turn on error light
			LOG_ERR("Error starting DAC->Network socket creation: result = %02X\n", result);
			Amp_Connect_Result(NSR_E_FAILED);
		}
	}
	else
	{
		LOG_ERR("Error Creating DAC->INIC Socket: Error Code = %04X\n", cb_result);
		DAC_connection.INIC_handle = INVALID_SOCKET;
		RED_LED_ON(); // turn on error light
		ConmgrReturnMutex();  //we're done, return mutex, stop timeout timer
		Amp_Connect_Result(NSR_E_FAILED);
	}
}

/*******************************************************************************
* function: DAC_Network_CreateSocket
*
* descrip.: Create a socket from Network to INIC for DAC streaming data
* params  : none
* returns : --
* effects : NetServices triggers DAC_Network_CreateSocket_CB when done
*******************************************************************************/
uint8_t DAC_Network_CreateSocket(void)
{
	uint8_t result;
	TScmSocketDesc SocketDesc;

	LOG_NOR("Creating DAC Network -> INIC socket...\n");
	SocketDesc.port_id = SCM_PORT_ID_MOST;
	SocketDesc.direction = SCM_IN;
	SocketDesc.datatype = SCM_TYPE_SYNC;
	SocketDesc.blockwidth = 0x0004;
	SocketDesc.most.list_len = 0x01;
	SocketDesc.most.list_ptr = &DAC_connection.ConnectionLabel; // has to be filled in correctly before we get here

	result = ScmCreateSocket(&SocketDesc, DAC_Network_CreateSocket_CB);
	return (result);
}

/*******************************************************************************
* function: DAC_Network_CreateSocket_CB
*
* descrip.: MNS callback function
* params  : result of socket creation process, respective socket handle, length
*           of channel list and pointer to channel list
* returns : --
* effects : if socket created successfully, calls DAC ConnectSockets
*******************************************************************************/
void DAC_Network_CreateSocket_CB(TMnsResult cb_result, uint8_t handle, uint8_t list_len, uint16_t *list_ptr)
{
	uint8_t result;

	if (NSR_S_OK == cb_result)
	{
		LOG_NOR("DAC Network -> INIC Socket created OK\n");
		DAC_connection.Network_handle = handle; // our socket handle for NETWORK side
		// only get a handle for an "IN" socket
		RED_LED_OFF(); // turn off error light if it was on
		result = DAC_ConnectSockets(); // connect the two sockets now
		if (ERR_NO != result)
		{
			ConmgrReturnMutex();  //we're quitting (failed), return mutex, stop timeout timer
			RED_LED_ON(); // turn on error light
			LOG_ERR("Error starting DAC->Network connect sockets: result = %02X\n", result);
			Amp_Connect_Result(NSR_E_FAILED);
		}
	}
	else
	{
		ConmgrReturnMutex();  //we're done, return mutex, stop timeout timer
		RED_LED_ON(); // turn on error light
		LOG_ERR("Error Creating DAC Network -> INIC Socket: Error Code = %04X\n", cb_result);
		DAC_connection.Network_handle = INVALID_SOCKET;
		DAC_connection.ConnectionLabel = INVALID_LABEL;
		Amp_Connect_Result(NSR_E_FAILED);
	}
}

/*******************************************************************************
* function: DAC_ConnectSockets
*
* descrip.: Connects the DAC to the MOST Network - INIC & NETWORK sockets must
*           already be created
* params  : none
* returns : --
* effects : NetServices triggers DAC_ConnectSockets_CB when done
*******************************************************************************/
uint8_t DAC_ConnectSockets(void)
{
	uint8_t result;
	LOG_NOR("Connecting Network to DAC...\n");
	result = ScmConnectSockets(DAC_connection.Network_handle, DAC_connection.INIC_handle, DAC_ConnectSockets_CB);
	return (result);
}


/*******************************************************************************
* function: DAC_ConnectSockets_CB
*
* descrip.: MNS callback function
* params  : result of socket connection process and respective connection handle
* returns : --
* effects : moves state machine back to IDLE when done
*******************************************************************************/
void DAC_ConnectSockets_CB(TMnsResult cb_result, uint8_t connection_handle)
{

	ConmgrReturnMutex();  //we're done, return mutex, stop timeout timer
	if (NSR_S_OK == cb_result)
	{
		LOG_NOR("DAC Sockets Connected OK\n");
		DAC_connection.ConnectionHandle = connection_handle;
		DAC_connection.Connecting = FALSE; // no longer connecting - done
		YELLOW2_LED_ON(); // indicate data is streaming now
		RED_LED_OFF(); // turn off error light if it was on
	}
	else
	{
		LOG_ERR("Error Connecting DAC Sockets: Error Code = %04X\n", cb_result);
		DAC_connection.ConnectionHandle = INVALID_CONNECTION;
		RED_LED_ON(); // turn on error light
	}
	/* pass or fail, we're done now */
	Amp_Connect_Result(cb_result); // send reply now
}


/********************************************************************************/
/************************ DAC Dis-connect Process *******************************/
/********************************************************************************/

/*******************************************************************************
* function: DAC_DisConnectSockets
*
* descrip.: DisConnects the DAC from the MOST Network - killing the connection
*           stops the streaming data - INIC & NETWORK sockets still exist
* params  : none
* returns : --
* effects : NetServices triggers DAC_DisConnectSockets_CB when done
*******************************************************************************/
uint8_t DAC_DisConnectSockets(void)
{
	uint8_t result;

	LOG_NOR("Dis-connecting DAC from Network...\n");
	result = ScmDisconnectSockets(DAC_connection.ConnectionHandle, DAC_DisConnectSockets_CB); // only needs handle to disconnect socket
	return (result);
}

/*******************************************************************************
* function: DAC_DisConnectSockets_CB
*
* descrip.: MNS callback function
* params  : result of socket dis connect process
* returns : --
* effects : moves state machine on to DESTROY_NET_IN when done
*******************************************************************************/
void DAC_DisConnectSockets_CB(TMnsResult cb_result)
{
	uint8_t result;

	if (NSR_S_OK == cb_result)
	{
		LOG_NOR("DAC Sockets DisConnected OK\n");
		DAC_connection.ConnectionHandle = INVALID_CONNECTION;
		YELLOW2_LED_OFF(); // audio no longer streaming
		RED_LED_OFF(); // turn off error light if it was on
		result = DAC_INIC_DestroySocket(); // go ahead and kill the INIC socket - not being used now
		if (ERR_NO != result)
		{
			ConmgrReturnMutex();  //we're quitting (failed), return mutex, stop timeout timer
			//RED_LED_ON();
			LOG_ERR("Error starting DAC->INIC disconnect sockets: result = %02X\n", result);
			if (DAC_connection.Connecting) // were we disconnecting so we could connect to another source?
			{
				Amp_Connect_Result(NSR_E_FAILED); // connection failed - can't disconnect
			}
			else  // doing disconnect only
			{
				Amp_DisConnect_Result(NSR_E_FAILED); // dis-connect has failed
			}
		}
	}
	else
	{
		LOG_ERR("Error DisConnecting DAC Sockets: Error Code = %04X\n", cb_result);
		if (DAC_connection.Connecting) // were we disconnecting so we could connect to another source?
		{
			Amp_Connect_Result(cb_result); // connection failed - can't disconnect
		}
		else  // doing disconnect only
		{
			Amp_DisConnect_Result(cb_result); // dis-connect has failed
		}
		RED_LED_ON(); // turn on error light
		ConmgrReturnMutex();  //we're quitting, return mutex, stop timeout timer
	}
}

/*******************************************************************************
* function: DAC_INIC_DestroySocket
*
* descrip.: Kills the Network socket - ready to create new one to another channel
*
* params  : none
* returns : --
* effects : NetServices triggers DAC_Network_DestroySocket_CB when done
*******************************************************************************/
uint8_t DAC_INIC_DestroySocket(void)
{
	uint8_t result;
	LOG_NOR("Destroying DAC -> INIC Socket...\n");
	result = ScmDestroySocket(DAC_connection.INIC_handle, DAC_INIC_DestroySocket_CB);
	return(result);
}

/*******************************************************************************
* function: DAC_INIC_DestroySocket_CB
*
* descrip.: MNS callback function
* params  : result of destroy socket process
* returns : --
* effects : moves state machine back to IDLE when done
*******************************************************************************/
void DAC_INIC_DestroySocket_CB(TMnsResult cb_result)
{
	uint8_t result;

	if (NSR_S_OK == cb_result)
	{
		LOG_NOR("DAC -> INIC Socket Destroyed OK\n");
		DAC_connection.INIC_handle = INVALID_SOCKET; // socket killed
		RED_LED_OFF(); // turn off error light if it was on
		result = DAC_Network_DestroySocket();  // now kill the network socket to complete disconnect
		if (ERR_NO != result)
		{
			ConmgrReturnMutex();  //we're quitting (failed), return mutex, stop timeout timer
			RED_LED_ON();
			LOG_ERR("Error starting DAC->Network destroy sockets: result = %02X\n", result);
			if (DAC_connection.Connecting) // were we disconnecting so we could connect to another source?
			{
				Amp_Connect_Result(NSR_E_FAILED); // connection failed - can't disconnect
			}
			else  // doing disconnect only
			{
				Amp_DisConnect_Result(NSR_E_FAILED); // dis-connect has failed
			}
		}
	}
	else
	{
		LOG_ERR("Error Destroying DAC -> INIC Socket: Error Code = %04X\n", cb_result);
		if (DAC_connection.Connecting) // were we disconnecting so we could connect to another source?
		{
			Amp_Connect_Result(cb_result); // connection failed - can't disconnect
		}
		else  // doing disconnect only
		{
			Amp_DisConnect_Result(cb_result); // dis-connect has failed
		}
		RED_LED_ON(); // turn on error light
		ConmgrReturnMutex();  //we're quitting, return mutex, stop timeout timer
	}
}
/*******************************************************************************
* function: DAC_Network_DestroySocket
*
* descrip.: Kills the Network socket - ready to create new one to another channel
*
* params  : none
* returns : --
* effects : NetServices triggers DAC_Network_DestroySocket_CB when done
*******************************************************************************/
uint8_t DAC_Network_DestroySocket(void)
{
	uint8_t result;
	LOG_NOR("Destroying DAC Network -> INIC socket...\n");
	result = ScmDestroySocket(DAC_connection.Network_handle, DAC_Network_DestroySocket_CB);
	return (result);
}

/*******************************************************************************
* function: DAC_Network_DestroySocket_CB
*
* descrip.: MNS callback function
* params  : result of destroy socket process
* returns : --
* effects : moves state machine back to IDLE when done
*******************************************************************************/
void DAC_Network_DestroySocket_CB(TMnsResult cb_result)
{
	uint8_t result;

	if (NSR_S_OK == cb_result)
	{
		LOG_NOR("DAC Network -> INIC Socket Destroyed OK\n");
		DAC_connection.Network_handle = INVALID_SOCKET; // socket killed
		if (DAC_connection.Connecting) // were we disconnecting so we could connect to another source?
		{
			result = DAC_INIC_CreateSocket(); // process started by "Connect" so now get on with connecting
			if (ERR_NO != result)
			{
				ConmgrReturnMutex();  //we're quitting (failed), return mutex, stop timeout timer
				RED_LED_ON(); // turn on error light
				LOG_ERR("Error starting DAC connection process after disconnect: result = %02X\n", result);
				Amp_Connect_Result(NSR_E_FAILED); // connection failed - can't disconnect
			}
		} // don't respond yet - will do when connection is finished
		else
		{
			DAC_connection.ConnectionLabel = INVALID_LABEL; // this label no longer in use
			Amp_DisConnect_Result(cb_result); // normal exit of disconnect process
			RED_LED_OFF(); // turn off error light if it was on
			ConmgrReturnMutex();  //we're done, return mutex, stop timeout timer
		}
	}
	else
	{
		LOG_ERR("Error Destroying DAC Network -> INIC Socket: Error Code = %02X\n", cb_result);
		if (DAC_connection.Connecting) // were we disconnecting so we could connect to another source?
		{
			Amp_Connect_Result(cb_result); // connection failed - can't disconnect
		}
		else  // doing disconnect only
		{
			Amp_DisConnect_Result(cb_result); // dis-connect has failed
		}
		RED_LED_ON(); // turn on error light
		ConmgrReturnMutex();  //we're quitting, return mutex, stop timeout timer
	}
}


/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*                          									            */
/* General-Functions                                						*/
/*                                        									*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/


/****************************************************************************
*  Function:       	void Set_Volume(void);
*
*  Description:    	Sets the volume of the UDA1380 codec based on Amp.pVolume
*					which is set by a VolumeSet, VolumeInc, or VolumeDec command
*					Scales volume from MOST (0 = Mute -> 32 = MAX) range to
*					Codec range (00 = Max, FC = mute)
*
*  Input(s):       	Volume from Amp.pVolume - Must range from 0 to 32
*
*  Side-effect(s): 	Changes volume
*
*  Return(s):      	-none-
*
****************************************************************************/
void Set_Volume(void)
{

	//    uint8_t attn;

	// Volume to attenuation mapping - simply 40 (0x28) - value
	//   attn = 40 - Amp.pVolume; // 0 in -> 0x28 out (-73dB), 40 in -> 00 out (+6 dB)
	//   DacSetAttenuation(attn);
	CODEC_setVolume(0, Amp.pVolume);
	CODEC_setVolume(1, 0);
	//CODEC_setAttenuation(2, attn);
	
	if (Amp.pVolume != Amp.nVolume)
	{
		NtfPropertyChangedFkt(FBLOCK_AMP, 0, AMP_VOLUME); // trigger notification
		Amp.nVolume = Amp.pVolume;
	}

}

void Amp_SetVolumeDefault (void)
{
	Amp_SetVolumeLocal(DEFAULT_VOLUME);
}

void Amp_SetVolumeLocal (uint8_t volume)
{
	Amp.pVolume = volume; // Set the property value
	Set_Volume();
}

void Amp_VolumeIncrementLocal (void)
{
	if (Amp.pVolume != VOLMAX)
	{
		Amp.pVolume += 1;
		Set_Volume();
	}

}

void Amp_VolumeDecrementLocal (void)
{
	if (Amp.pVolume != 0)
	{
		Amp.pVolume -= 1;
		Set_Volume();
	}
}


void Amp_UpdateTemp(void)
{
//	Amp.pTemperature = PM_GetTemp(); // update Amp's Temperature property

	// Notification
	if (Amp.pTemperature != Amp.nTemperature) // see if new value is different
	{
		Amp.nTemperature = Amp.pTemperature; // make reference & actual the same
		NtfPropertyChangedFkt(FBLOCK_AMP, 0, AMP_TEMPERATURE); // trigger notification
		Amp_PrintTemp(Amp.pTemperature);
	}
}

void Amp_TempEvent_CB(uint8_t Temp)
{
	LOG_NOR("*** Temp limit crossed *** : ");
	Amp_PrintTemp(Temp);
	if (Temp >= Amp.HiTemp)
	{
		Amp.O_Temp = TRUE;
	}
	else
	{
		Amp.O_Temp = FALSE;
	}
	NtfPropertyChangedFkt(FBLOCK_AMP, 0, AMP_EVENTS); // trigger notification
}

void Amp_PrintTemp(uint8_t Temp)
{
	LOG_NOR("Temp is ");
	if (0 != (0x80 & Temp)) // High order bit set ==> negative 2's compliment
	{
		LOG_NOR("-");
		Temp = (0xFF - Temp) + 1; // 2 comp invert
	}
	LOG_NOR("%d", Temp);
	LOG_NOR(" deg C\n");
}

