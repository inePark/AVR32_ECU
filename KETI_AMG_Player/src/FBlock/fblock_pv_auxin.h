/*
==============================================================================

Project:        INIC Audio Interface
Module:         FBlock AuxIn
File:           fblock_pv_auxin.h
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
#define FBLOCK_AUXIN			0x24	//
#define INST_AUXIN				0x01	//
#define AUXIN_INDEX				0x01	// Change if Location in FBlock.tab is changed

#define NUM_NTF_PROP_AUXIN		0x03	// How many properties do a notification

#define GAINMAX          		24 		// in dB - codec supports 3dB steps - so actually only 8 gain settings
#define I2C_ERROR				0x21	// could be put in mostdef1.h with all other errors

#define AUDIO					0x00 	//
#define RES_16BIT				0x02 	// Resolution 2	= 16-Bit Audio
#define AUDIO_MONO				0x01 	// AudioChann 1 = Mono
#define AUDIO_STEREO			0x02 	// AudioChann 2 = Stereo
#define SOURCE_DELAY            0x00

#define UNICODE					0x00 	// ID's for Strings
#define ASCII					0x03 	//
#define MMUTE					0xF8 	// For Connect / DisConnect

extern TNtfPropL NtfPropTabAuxin[];


//-------------------------------------------------------------------
// FunctionIDs of FBlock AuxiliaryInput
//-------------------------------------------------------------------
#define FUNC_INPUT_GAIN			0x437	// InputGainOffset



void Set_Input_Gain (void);
