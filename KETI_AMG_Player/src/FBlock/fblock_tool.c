/* 
 * MOST TOOL : 
 * Author : netbugger
 * Date   : 2013.03.18
 * Contents : queue for dual inic 
 */
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <asf.h>
#include "mostns.h"
#include "defines.h"
#include "fblock_pv_tool.h"
#include "fblock_pb_tool.h"
#include "t_fblock_tool.tab"	// Table containing all FUNC_IDS and all OP_TYPES of FBlock
/*
   ------------------------------------------------------------------------------
   Local Definitions
   ------------------------------------------------------------------------------
*/
#define TOOL_VERSION_MAJOR   1
#define TOOL_VERSION_MINOR   0
#define TOOL_VERSION_BUILD   0

/*
   ------------------------------------------------------------------------------
   Local variables and buffers
   ------------------------------------------------------------------------------
*/

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




/****************************************************************************
 *  Function:       Init_TOOL
 *
 *  Description:    Init the FBlock(Make Queue)
 *
 *  Input(s):       -none-
 *
 *  Side-effect(s): -none-
 *
 *  Return(s):      -none-
 *
 ****************************************************************************/
int Init_TOOL(void)
{
	return 0;
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
uint8_t TOOL_FuncIDs_Get(struct Msg_Tx_Type *Tx_Ptr)
{
	CmdGetFunctionIds(Tx_Ptr, Func_TOOL);
	return (OP_STATUS);
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
uint8_t TOOL_FBlockInfo_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr)
{
	uint16_t rx_id;
	uint8_t  name[] = "TOOL";
	uint8_t  i;
	uint8_t  version[3];

	CmdDecodeWord(&rx_id, &Rx_Ptr->Data[0]);

	Tx_Ptr->Data[0] = Rx_Ptr->Data[0];	/* prepare TX message */
	Tx_Ptr->Data[1] = Rx_Ptr->Data[1];
	Tx_Ptr->Length  = (uint16_t)2;


	if (rx_id < 0x1000)					/* FktID ?*/
	{
		Tx_Ptr->Length  = (uint16_t)3;

		switch (rx_id)
		{
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
				version[0] = TOOL_VERSION_MAJOR;
				version[1] = TOOL_VERSION_MINOR;
				version[2] = TOOL_VERSION_BUILD;

				Tx_Ptr->Data[2] = FRMT_ISO8859;

				i = MsgVersionToISO8859(&version[0], &Tx_Ptr->Data[3]);

				Tx_Ptr->Length  = (word)((word)i + (word)3);
				break;

			case FBI_FBLOCK_VERSION:
				version[0] = TOOL_VERSION_MAJOR;
				version[1] = TOOL_VERSION_MINOR;
				version[2] = TOOL_VERSION_BUILD;

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
				Tx_Ptr->Data[3] = (uint8_t)'K';
				Tx_Ptr->Data[4] = (uint8_t)'E';
				Tx_Ptr->Data[5] = (uint8_t)'T';
				Tx_Ptr->Data[6] = (uint8_t)'I';
				Tx_Ptr->Length  = (uint16_t)7;
				break;

			case FBI_FBLOCK_TYPE:
				Tx_Ptr->Data[2] = FRMT_ISO8859;
				Tx_Ptr->Data[3] = (uint8_t)'G';
				Tx_Ptr->Data[4] = (uint8_t)'W';
				Tx_Ptr->Data[5] = (uint8_t)'I';
				Tx_Ptr->Data[6] = (uint8_t)'F';
				Tx_Ptr->Data[7] = (uint8_t)'T';
				Tx_Ptr->Data[8] = (uint8_t)'O';
				Tx_Ptr->Data[9] = (uint8_t)'O';
				Tx_Ptr->Data[10] = (uint8_t)'L';
				Tx_Ptr->Length  = (uint16_t)11;
				break;

			default :
				return(CmdErrorParamWrong(Tx_Ptr, 1, &Rx_Ptr->Data[0], 2));
				break;
		}
	}

	return(OP_STATUS);
}


uint8_t TOOL_GW_Outgoing(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *RxPtr)
{
	int16_t i;
	LOG_NOR("Received from Optical : \n");
	for(i=0; i<RxPtr->Length; i++) {
		LOG_NOR("%02X ", RxPtr->Data[i]);
	}
	LOG_NOR("\n");
	
	return (OP_NO_REPORT);
}
