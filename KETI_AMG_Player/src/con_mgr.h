/*****************************************************

Module  : Connection Manager, header file
Version : 2.0
File    : con_mgr.h
Date    : 11/07/2006
Author  : Gary Warren
Company : SMSC - AIS
Comments:


Modifications:


*****************************************************/
#ifndef CON_MGR_H_
#define CON_MGR_H_

/****************************************************************************
  Constants
****************************************************************************/
#include <stdint.h>

#define INVALID_SOCKET 			0xFF
#define INVALID_CONNECTION 		0xFF
#define INVALID_CHANNEL			0xF8
#define INVALID_LABEL			0x0000

/* Streaming Data Formats */
#define DELAY_64FS_16BIT							((uint8_t) 0x00)	// Delayed-bit Streaming format, 64xFs 16 bit channels, legacy
#define DELAY_64FS_24BIT							((uint8_t) 0x01)	// Delayed-bit Streaming format, 64xFs 24 bit channels, legacy
#define DELAY_128FS_16BIT							((uint8_t) 0x02)	// Delayed-bit Streaming format, 128xFs 16 bit channels, legacy
#define DELAY_128FS_24BIT							((uint8_t) 0x03)	// Delayed-bit Streaming format, 128xFs 24 bit channels, legacy
#define DELAY_64FS_SEQ								((uint8_t) 0x04)	// Delayed-bit Streaming format, 64xFs  sequential
#define DELAY_128FS_SEQ								((uint8_t) 0x05)	// Delayed-bit Streaming format, 128xFs sequential
#define DELAY_256FS_SEQ								((uint8_t) 0x06)	// Delayed-bit Streaming format, 256xFs sequential
#define RIGHT_64FS_16BIT							((uint8_t) 0x07)	// Right justified Streaming format, 64xFs 16 bit channels, legacy
#define RIGHT_64FS_24BIT							((uint8_t) 0x08)	// Right justified Streaming format, 64xFs 24 bit channels, legacy
#define RIGHT_128FS_16BIT							((uint8_t) 0x09)	// Right justified Streaming format, 128xFs 16 bit channels, legacy
#define RIGHT_128FS_24BIT							((uint8_t) 0x0A)	// Right justified Streaming format, 128xFs 24 bit channels, legacy
#define SEQ_64FS									((uint8_t) 0x0B)	// Sequential Streaming format, 64xFs
#define SEQ_128FS									((uint8_t) 0x0C)	// Sequential Streaming format, 128xFs
#define SEQ_256FS									((uint8_t) 0x0D)	// Sequential Streaming format, 256xFs
#define LEFT_64FS_16BIT								((uint8_t) 0x07)	// Left justified Streaming format, 64xFs 16 bit channels, legacy
#define LEFT_64FS_24BIT								((uint8_t) 0x08)	// Left justified Streaming format, 64xFs 24 bit channels, legacy
#define LEFT_128FS_16BIT							((uint8_t) 0x09)	// Left justified Streaming format, 128xFs 16 bit channels, legacy
#define LEFT_128FS_24BIT							((uint8_t) 0x0A)	// Left justified Streaming format, 128xFs 24 bit channels, legacy

// Source / Sink Data types
#define PCM_AUDIO             0x00             //
#define CDROM                 0x01
#define SPDIF                 0x02
#define MPEG1_SYSTEM          0x20
#define MPEG2_PROGRAM         0x21
#define MPEG2_TRANSPORT       0x22
#define MPEG1_SYSTEM_DTCP     0x40
#define MPEG2_PROGRAM_DTCP    0x41
#define MPEG2_TRANSPORT_DTCP  0x42


#define RES_16BIT             0x02  // Resolution 2 = 16-Bit Audio
#define AUDIO_MONO            0x01  // AudioChann 1 = Mono
#define AUDIO_STEREO          0x02  // AudioChann 2 = Stereo
#define SEVEN_DOT_ONE         0x08  // 8 channels for 7.1 audio
#define FIVE_DOT_ONE          0x06  // 6 channels for 5.1 audio


//-------------------------------------------------------------------
// FunctionIDs of FBlock Amplifier
//-------------------------------------------------------------------

//----- MOST Cooperation Function Catalogue -------------------------
#define FBLOCK_AMP              0x22    //
#define AMP_VOLUME              0x400   // Property:    Volume
#define AMP_MUTE                0x113
#define AMP_BASS                0x202
#define AMP_TREBLE              0x203
#define AMP_EVENTS              0xC03   /* Amplifier Events/Status - custom property */
#define AMP_TEMPERATURE         0xC04

//-------------------------------------------------------------------
// FunctionIDs of FBlock Auxin
//-------------------------------------------------------------------
#define FBLOCK_AUXIN            0x24    //

#define SRC_ACTIVITY_ON         0x02
#define SRC_ACTIVITY_OFF        0x00


enum CONNECTION_STATE
{
    CON_IDLE=0,
    SCM_PENDING
};

struct Connection
{
	uint8_t  StreamingPort;            // TRUE / FALSE open or not
	uint8_t  INIC_handle;              // Streaming port handle when socket created
	uint8_t  Network_handle;           // Network port handle when socket created
	uint16_t ConnectionLabel;			// Used for MOST50
	uint8_t  ConnectionHandle;			//
	uint8_t  Connecting;				// flag, set when we must disconnect before connecting
	uint8_t  ConnectType;				// flag, for SourceConnect vs. Allocate
	uint8_t  LocalCommand;				// flag, if commanded (need response) or doing on our own (no response)
};
typedef struct Connection CONNECTION;


typedef void  (*Type_ScmTimeout_fptr)(void);

typedef struct _ScmCmd
{
   uint8_t              apiLocked;
   Type_ScmTimeout_fptr ScmTimeout_fptr;
}  Type_ScmCmd;



typedef struct DeviceState
{
	uint8_t    MPR;
	uint8_t    Net_State;
	uint8_t    Net_State_Previous;
	uint8_t    DeviceMode;
	uint8_t    NodeDelay;
	uint16_t   NodePositionAddress;
	uint16_t   NodeAddress;
	uint16_t   GroupAddress;
	uint8_t    AbilityToWake;
	uint8_t    RetryTime;
	uint8_t    RetryNumber;
	uint8_t    LockState;
	uint8_t    EHCI_State;
	uint8_t    NetworkFreq;
    uint8_t    INIC_Sockets_Done;
    uint8_t    StreamingPort;
    uint8_t    MediaLBPort;
	uint8_t    ICM_fifo_pending;
	uint8_t    MCM_fifo_pending;
	uint8_t    MDP_fifo_pending;
	uint8_t    Attenuation;
	uint8_t    TempSensorPresent;
	uint8_t    CodecPresent;
	uint16_t   ChatPartner;
} DEVICE_STATE;

extern DEVICE_STATE MyNode;
extern struct Msg_Tx_Type *Retained_Tx_Ptr;                            // global for delayed responses


void    ConmgrInit ( void );
void    kill_net_connections (void);
void    connection_manager (void);
void    InitPorts (void);
void    InitPorts_Timeout (void);
uint8_t OpenSerialPort(void);
void    OpenSerialPort_CB(uint16_t cb_result);


void    SCError_CB(uint8_t handle);
void    SCError_Timeout(void);
uint8_t ConmgrGetMutex(uint16_t timeout, Type_ScmTimeout_fptr Timeout_fn);
void    ConmgrReturnMutex(void);

#endif
