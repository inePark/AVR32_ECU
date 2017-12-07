/**************************************************************************
* Oasis SiliconSystems (c) 2003
*
* The copyright of the computer program(s) herein is the property
* of Oasis SiliconSystems, Germany.
* The program(s) may be used and copied only with written permission
* from Oasis SiliconSystems, or in accordance with the terms and
* conditions stipulated in the agreement under which the program(s) have
* been supplied.
*
* Project, Module: 	AppWave demo projetc
*				   	Realization of FBlock Amplifier
*
* Filename: 		fblock_pb_amp.h
*
* Description: 		public Header of Amplifier module
*
* Author(s): 		Adapted by GW
*
**************************************************************************/



/*
------------------------------------------------------------------------------
	Type Declaration
------------------------------------------------------------------------------
*/
#include <stdint.h>

typedef struct // Definition of FBlock AmpBlock
{
    uint8_t pVolume; // property value
    uint8_t nVolume; // notification value
    uint8_t pBass;
    uint8_t pTreble;
    uint8_t pTemperature;
    uint8_t nTemperature;
    uint8_t pMute;
    uint16_t pConnectionLabel;
    uint8_t O_Temp; // TRUE /FALSE based on Over temp condition
    uint8_t HiTemp; // current alarm limits
    uint8_t LoTemp;
    uint8_t EventMask;
} TAmp;



/*
------------------------------------------------------------------------------
		Variables and buffers
------------------------------------------------------------------------------
*/
extern TAmp Amp;								// Struct AmpBlock
extern const struct Func_L_Type Func_Amp[];	  	// Table containing all FktIDs


/*
------------------------------------------------------------------------------
		Function Prototypes
------------------------------------------------------------------------------
*/

void Init_Amp(void);
void Set_Volume(void);
void Amp_KillConnections(void);


//==== MOST Function Catalog ==========================================================

uint8_t Amp_FuncIDs_Get(struct Msg_Tx_Type *Tx_Ptr);

uint8_t Amp_Notification_Set(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
uint8_t Amp_Notification_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
uint8_t Amp_NotificationCheck_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
uint8_t Amp_Version_Get(struct Msg_Tx_Type *Tx_Ptr);
uint8_t Amp_FBlockInfo_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);

uint8_t Amp_SinkInfo_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
uint8_t Amp_SinkInfo_Get_N(struct Msg_Tx_Type *Tx_Ptr);
uint8_t Amp_SyncDataInfo_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);

uint8_t Amp_Connect_StartResult(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
uint8_t Amp_Connect_StartResultACK(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
void    Amp_Connect_Result (uint16_t result);

uint8_t Amp_DisConnect_StartResult(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
uint8_t Amp_DisConnect_StartResultACK(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
void    Amp_DisConnect_Result(uint16_t result);

uint8_t Amp_Mute_Set(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
uint8_t Amp_Mute_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
uint8_t Amp_Mute_Get_N(struct Msg_Tx_Type *Tx_Ptr);

uint8_t Amp_SinkName_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);

uint8_t Amp_Volume_Set(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
uint8_t Amp_Volume_Inc(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
uint8_t Amp_Volume_Dec(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);

uint8_t Amp_Events_Set (struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
uint8_t Amp_Events_Get (struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);
uint8_t Amp_Events_Get_N (struct Msg_Tx_Type *Tx_Ptr);

uint8_t DAC_INIC_CreateSocket(void);
void    DAC_INIC_CreateSocket_CB(TMnsResult cb_result, uint8_t handle, uint8_t list_len, uint16_t *list_ptr);
uint8_t DAC_Network_CreateSocket(void);
void    DAC_Network_CreateSocket_CB(TMnsResult cb_result, uint8_t handle, uint8_t list_len, uint16_t *list_ptr);
uint8_t DAC_ConnectSockets(void);
void    DAC_ConnectSockets_CB(TMnsResult cb_result, uint8_t connection_handle);
uint8_t DAC_DisConnectSockets(void);
void    DAC_DisConnectSockets_CB(TMnsResult cb_result);
uint8_t DAC_INIC_DestroySocket(void);
void    DAC_INIC_DestroySocket_CB(TMnsResult cb_result);
uint8_t DAC_Network_DestroySocket(void);
void    DAC_Network_DestroySocket_CB(TMnsResult cb_result);

void Amp_UpdateTemp(void);
void Amp_TempEvent_CB( uint8_t Temp);
void Amp_PrintTemp (uint8_t Temp);

void Amp_SetVolumeDefault (void);
void Amp_SetVolumeLocal (uint8_t volume);
void Amp_VolumeIncrementLocal (void);
void Amp_VolumeDecrementLocal (void);


uint8_t Amp_Volume_Get(struct Msg_Tx_Type *Tx_Ptr, struct Msg_Rx_Type *Rx_Ptr);



