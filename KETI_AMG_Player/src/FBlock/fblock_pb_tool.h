#ifndef _FBLOCK_PB_TOOL_H_
#define _FBLOCK_PB_TOOL_H_
#include <stdint.h>
#include "mostns.h"

/*
------------------------------------------------------------------------------
		Variables and buffers
------------------------------------------------------------------------------
*/
extern const struct Func_L_Type Func_TOOL[];	  	// Table containing all FktIDs

/*
------------------------------------------------------------------------------
		Function Prototypes
------------------------------------------------------------------------------
*/

int Init_TOOL(void);

//==== MOST Function Catalog ==========================================================

uint8_t TOOL_FuncIDs_Get(struct Msg_Tx_Type *Tx_Ptr);
uint8_t TOOL_FBlockInfo_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
uint8_t TOOL_GW_Outgoing(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);


#endif
