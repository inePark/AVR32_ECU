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
* Project, Module: 	AppWave demo project
*				   	Realization of FBlock Amplifier
*
* Filename: 		fblock_pv_amp.h
*
* Description: 		private Header of Amplifier module
*
* Author(s): 		Adapted by GW
*
**************************************************************************/


//----------------------------------------------------------------------
// FBlock Amplifier
//----------------------------------------------------------------------
#include <stdint.h>

#define FBLOCK_AMP				0x22	//
#define INST_AMP				0x01	//
#define AMP_INDEX				0x00	// Change if position in t_fblock.tab table is changed
#define NUM_NTF_PROP_AMP		0x05	// How many properties do a notification
#define I2C_ERROR				0x21	// could be put in mostdef1.h with all other errors

#define AUDIO					0x00 	//
#define RES_16BIT				0x02 	// Resolution 2	= 16-Bit Audio
#define AUDIO_MONO				0x01 	// AudioChann 1 = Mono
#define AUDIO_STEREO			0x02 	// AudioChann 2 = Stereo
#define SINK_DELAY              0x00

#define UNICODE					0x00 	// ID's for Strings
#define ASCII					0x03 	//
#define MMUTE					0xF8 	// For Connect / DisConnect

extern TNtfPropL NtfPropTabAmp[];


//-------------------------------------------------------------------
// FunctionIDs of FBlock Amplifier
//-------------------------------------------------------------------

//----- MOST Cooperation Function Catalogue -------------------------
#define AMP_VOLUME				0x400	// Property:	Volume
#define AMP_MUTE                0x113
#define AMP_BASS                0x202
#define AMP_TREBLE              0x203
#define AMP_EVENTS	  			0xC03   /* Amplifier Events/Status - custom property */
#define AMP_TEMPERATURE			0xC04
