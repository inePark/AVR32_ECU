/*
==============================================================================

Project:        INIC Audio Interface
Module:         FBlock AuxIn
File:           fblock_pb_auxin.h
Version:
Language:       C
Author(s):      C.GROMM
Date:           15.Aug.2005

------------------------------------------------------------------------------

                (c) Copyright 1998-2005
                Oasis SiliconSystems AG
                All Rights Reserved

------------------------------------------------------------------------------


Modifications
~~~~~~~~~~~~~
Date            By      Description

==============================================================================
*/

/*
------------------------------------------------------------------------------
	Type Declaration
------------------------------------------------------------------------------
*/
#ifndef _FBLOCK_AUXIN_H_
#define _FBLOCK_AUXIN_H_
#include <stdint.h>

// Auxin specific data
//------------------------
#define AUXIN_SOURCE_NR       	0x01		    //
#define AUXIN_CHANNELS      	0x04      		// Number of Audio-Channels
#define SRC_ACTIVITY_ON         0x02
#define SRC_ACTIVITY_OFF        0x00


typedef struct								// Definition of FBlock AuxinBlock
{
	uint8_t     SrcActivity;
	uint8_t     pNodeDelay;
	uint8_t		pGain;
	uint16_t    pConnectionLabel;
	uint16_t    nConnectionLabel;
}TAuxin;



/*
------------------------------------------------------------------------------
		Variables and buffers
------------------------------------------------------------------------------
*/
extern TAuxin Auxin;								// Struct AuxinBlock
extern const struct Func_L_Type Func_Auxin[];	  	// Table containing all FktIDs


/*
------------------------------------------------------------------------------
		Function Prototypes
------------------------------------------------------------------------------
*/

void Init_Auxin(void);
void AuxNotifyService(void);
void Auxin_KillConnections(void);

//==== MOST Function Catalog ==========================================================

uint8_t Auxin_FuncIDs_Get(struct Msg_Tx_Type *Tx_Ptr);
uint8_t Auxin_Notification_Set(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
uint8_t Auxin_Notification_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
uint8_t Auxin_NotificationCheck_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
uint8_t Auxin_FBlockInfo_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
uint8_t Auxin_SourceInfo_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
uint8_t Auxin_SourceInfo_N(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
uint8_t Auxin_Allocate_StartResult(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
void    Auxin_AllocateTimeout(void);
void    Auxin_Allocate_Result (uint16_t result);
uint8_t Auxin_Allocate_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
uint8_t Auxin_DeAllocate_StartResult(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
void    Auxin_DeAllocateTimeout(void);
void    Auxin_DeAllocate_Result (uint16_t result);
uint8_t Auxin_SourceActivity_StartResult(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
uint8_t Auxin_SourceName_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
uint8_t Auxin_SyncDataInfo_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);

uint8_t ADC_INIC_CreateSocket(void);
void    ADC_INIC_CreateSocket_CB(TMnsResult cb_result, uint8_t handle, uint8_t list_len, uint16_t *list_ptr);
uint8_t ADC_Network_CreateSocket(void);
void    ADC_Network_CreateSocket_CB(TMnsResult cb_result, uint8_t handle, uint8_t list_len, uint16_t *list_ptr);
uint8_t ADC_ConnectSockets(void);
void    ADC_ConnectSockets_CB(TMnsResult cb_result, uint8_t connection_handle);
uint8_t ADC_DisConnectSockets(void);
void    ADC_DisConnectSockets_CB(TMnsResult cb_result);
uint8_t ADC_Network_DestroySocket(void);
void    ADC_Network_DestroySocket_CB(TMnsResult cb_result);



#endif