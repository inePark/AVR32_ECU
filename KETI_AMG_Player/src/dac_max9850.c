/*******************************************************************************
 * SMSC Automotive Infotainment Systems Group
 *
 * (c)2002-2007 Standard Microsystems Corporation and its subsidiaries ("SMSC").
 * The copyright of the computer program(s) herein is the property of SMSC.
 * The program(s) may be used and copied only with written permission from SMSC
 * or in accordance with the terms and conditions stipulated in the
 * agreement under which the program(s) have been supplied.
 ******************************************************************************/

/*****************************************************

 Module  : Driver for MAX9850 DAC
 Version : 1.0
 File    : dac_max9850.c
 Date    : 09/25/2007
 Author  : Gary Warren
 Company : SMSC - AIS
 Comments:

 Description:
 A driver for the Maxim NAX9850 DAC

 Modifications:

 09/25/2007	GW		Creation of driver

 *****************************************************/
#include <stdint.h>
#include "defines.h"
#include "con_mgr.h"
#include "dac_max9850.h"
#include "i2c_driver.h"
#include "board_utils.h"
#include "timers.h"

//#define M150_DEMO_LEFT_64FS_16BIT

uint8_t DAC_Int; // global flag gets set when pin change interrupt is detected.


// Default settings for DAC MAX9850
// Settings are:
//	VOLUME:		Not muted, volume slew enabled, initial volume = -11.5 dB
//  DAC_GP:		GPIO used as output and is the ALERT signal (INT), Headphone detection debounced, changes occor on zero crossings
//  INT_ENABLE:	All disabled - user uses FBlockAmp.Events to enable
//	DAC_ENABLE:	All enabled except Line Inputs, an initially, SHDN.
//	DAC_CLOCK:	Internal divider is 1
//	CHARGE_PMP:	Slew rate control = 125ms, CP(4:0)=0 to enable internal charge pump oscillator
//	LRCLK_MSB:	Divisor is an integer, MSB not used
//	LRCLK_LSB:	Using RMCK = 256Fs, divisor becomes 256Fs/16*Fs or 16
//  DIG_AUDIO:	Digital format - setup for 16 bit delayed I2S

//
//------------------------------------------
#define NUM_DEFAULT_REGS 9
const uint8_t
        dac9850_defaults[2 * NUM_DEFAULT_REGS] = {
                                                   VOLUME,
                                                   (0 << MUTE | 1 << SLEW | 0x10), // Enable slew - initial volume to -11.5 dB
                                                   DAC_GP,
                                                   (1 << GPIOM_1 | 1 << GPIOM_0 | 1 << GPD | 0 << DBDEL_1 | 1 << DBDEL_0 | 0
                                                           << MONO | 1 << ZDEN),
                                                   INT_ENABLE,
                                                   (0 << ISGPIO | 0 << ICLK | 0 << ISHPS | 0 << IVMIN | 0 << IIOH),
                                                   DAC_ENABLE,
                                                   (0 << SHDN | 1 << MCLKEN | 1 << CPEN_1 | 1 << CPEN_0 | 1 << HPEN | 1 << LNOEN
                                                           | 0 << LNIEN | 1 << DACEN),
                                                   DAC_CLOCK,
                                                   (0 << IC_1 | 0 << IC_0),
                                                   CHARGE_PUMP,
                                                   (0 << SR_1 | 1 << SR_0),
                                                   LRCLK_MSB,
                                                   (1 << LRCLK_INT),
                                                   LRCLK_LSB,
                                                   (16),
#ifdef M150_DEMO_LEFT_64FS_16BIT
                                                    DIG_AUDIO, ( 0<<MASTER | 0<<INV | 0<<BCINV | 0<<LSF | 0<<SD_DELAY | 0<<RJT | 0<<WLS_1 | 0<<WLS_0)
#else
                                                   DIG_AUDIO, (0 << MASTER | 0 << INV | 0 << BCINV | 0 << LSF | 1 << SD_DELAY | 0
                                                           << RJT | 0 << WLS_1 | 0 << WLS_0)
#endif
        };

Type_Result DacRegisterRead(uint8_t regAddr, uint8_t * regData_ptr)
{
	Type_Result result;
	
	result = dac_read(regData_ptr, 1, regAddr);
	
	if (0 <= result)
	{
		return ERR_NONE;
	}

	return result;
}

Type_Result DacRegisterWrite(uint8_t regAddr, uint8_t regData)
{
	Type_Result result;

	result = dac_write(&regData, 1, regAddr);
	
	if (0 <= result)
	{
		return ERR_NONE;
	}

	return result;
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
*  FUNCTION: 		DacInit
*  PARAMETERS:		none -
*  DESCRIPTION:	A "debug" version of initializing CODEC
*                  Prints each register's initial and final value as it programs them
*  RETURNS:		>0 = success, <0 is one of the I2C errors
*/
/*-------------------------------------------------------------------------------------------------------------------*/
int16_t DacInit(void)
{
	uint8_t map;
	uint8_t data_out;
	uint8_t data_in;
	uint8_t i;
	int16_t ret;

	#ifdef DEV_USE_BUB85650
	return ERR_NONE;
	#endif

	LOG_NOR("Initializing MAX9850 DAC ... \n");

	for (i = 0; i < NUM_DEFAULT_REGS; i++) // Setting register defaults of MAX9850
	{

		map = (dac9850_defaults[i * 2 + 0]); // Register adr.
		data_out = dac9850_defaults[i * 2 + 1]; // Register value (byte)

		ret = DacRegisterWrite(map, data_out);
		if (ERR_NONE == ret)
		{
			MyNode.CodecPresent = TRUE; // codec responded
			delay_ms(1); // codec must have some time between commands
			DacRegisterRead(map, &data_in);
			LOG_NOR("MAX9850 Register, Value written, Value read: ");
			LOG_NOR("%02X %02X %02X\n", map, data_out, data_in);
		}
		else
		{
			MyNode.CodecPresent = FALSE; // can't talk to codec
			LOG_ERR("Error writing to Codec\n");
			break;
		}
	}
	// if everything was OK, now enable DAC (take out of Shutdown)
	if (ERR_NONE == ret)
	{
		LOG_NOR("Initializing done - enabling DAC now ... \n");
		delay_ms(1); // short delay before enable
		data_out = (1 << SHDN | 1 << MCLKEN | 1 << CPEN_1 | 1 << CPEN_0 | 1 << HPEN | 1 << LNOEN | 0 << LNIEN | 1 << DACEN);
		DacRegisterWrite(DAC_ENABLE, data_out);
		DAC_Int = FALSE; // init flag to none
	}

	return (ret);
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
*  FUNCTION: 		DacInitFast
*  PARAMETERS:		none -
*  DESCRIPTION:	Same as above, but does not print each register's value
*  RETURNS:		>0 = success, <0 is one of the I2C errors
*/
/*-------------------------------------------------------------------------------------------------------------------*/
int16_t DacInitFast(void)
{
	uint8_t map;
	uint8_t data_out;
	uint8_t i;
	int16_t ret;

	#ifdef DEV_USE_BUB85650
	return ERR_NONE;
	#endif

	LOG_NOR("Initializing MAX9850 DAC ... \n");
	for (i = 0; i < NUM_DEFAULT_REGS; i++) // Setting register defaults of MAX9850
	{

		map = (dac9850_defaults[i * 2 + 0]); // Register adr.
		data_out = dac9850_defaults[i * 2 + 1]; // Register value (byte)

		ret = DacRegisterWrite(map, data_out);
		if (ERR_NONE == ret)
		{
			MyNode.CodecPresent = TRUE; // codec responded
			delay_ms(1); // codec must have some time between commands
		}
		else
		{
			MyNode.CodecPresent = FALSE; // can't talk to codec
			break;
		}
	}

	// if everything was OK, now enable DAC (take out of Shutdown)
	if (ERR_NONE == ret)
	{
		delay_ms(1);
		data_out = (1 << SHDN | 1 << MCLKEN | 1 << CPEN_1 | 1 << CPEN_0 | 1 << HPEN | 1 << LNOEN | 0 << LNIEN | 1 << DACEN);
		DacRegisterWrite(DAC_ENABLE, data_out);

		DAC_Int = FALSE; // init flag to none
	}

	return (ret);

}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
*  FUNCTION: 		DacMute
*  PARAMETERS:		none -
*  DESCRIPTION:	Mutes the DAC output - used by Amp.Mute command
*  RETURNS:		>0 = success, <0 is one of the I2C errors
*/
/*-------------------------------------------------------------------------------------------------------------------*/
int16_t DacMute(void)
{
	uint8_t dataIo;
	int16_t result;

	#ifdef DEV_USE_BUB85650
	return ERR_NONE;
	#endif

	LOG_NOR("Muting Codec ... \n");

	result = DacRegisterRead(VOLUME, &dataIo);
	if (ERR_NONE == result)
	{
		dataIo |= (1 << MUTE); // set mute bit
		DacRegisterWrite(VOLUME, dataIo);
	}

	return result;

} // Mute()

/*-------------------------------------------------------------------------------------------------------------------*/
/*
*  FUNCTION: 		DacDemute
*  PARAMETERS:		none -
*  DESCRIPTION:	Un-Mutes the DAC output - used by Amp.Mute command
*  RETURNS:		>0 = success, <0 is one of the I2C errors
*/
/*-------------------------------------------------------------------------------------------------------------------*/
int16_t DacDemute(void)
{
	uint8_t dataIo;
	int16_t result;

	#ifdef DEV_USE_BUB85650
	return ERR_NONE;
	#endif

	LOG_NOR("DeMuting Codec ... \n");

	result = DacRegisterRead(VOLUME, &dataIo);
	if (ERR_NONE == result)
	{
		dataIo &= ~(1 << MUTE); // clear mute bit
		DacRegisterWrite(VOLUME, dataIo);
	}

	return result;
} // DeMute()

/*-------------------------------------------------------------------------------------------------------------------*/
/*
*  FUNCTION: 		DacIsMuted
*  PARAMETERS:		none -
*  DESCRIPTION:	Check mute setting of codec, returns status
*  RETURNS:		boolean - TRUE if muted, FALSE if not
*/
/*-------------------------------------------------------------------------------------------------------------------*/
uint8_t DacIsMuted(void)
{
	uint8_t reg_value;
	int16_t result;

	#ifdef DEV_USE_BUB85650
	return FALSE;
	#endif

	result = DacRegisterRead(VOLUME, &reg_value);
	if (ERR_NONE == result)
	{
		if ((reg_value & (1 << MUTE)) == 0) // report state of MASTER bit
		return FALSE;
		else
		return TRUE;
	}
	else
	{
		return (FALSE);
	}
} // DacIsMuted()


/*-------------------------------------------------------------------------------------------------------------------*/
/*
*  FUNCTION: 		DacSetAttenuation
*  PARAMETERS:		value - the value to set the volume to
*  DESCRIPTION:	Set the volume of the DAC output - used by Amp.Volume command
*                  0x00 (loud) ... 0x28 (quiet)
*                  sets left and right to same value
*                  an outside routine should map MOST volume values to Codec Attn values
*  RETURNS:		>0 = success, <0 is one of the I2C errors
*/
/*-------------------------------------------------------------------------------------------------------------------*/
int16_t DacSetAttenuation(uint8_t value)
{
	uint8_t reg_value;
	int16_t ret;

	#ifdef DEV_USE_BUB85650
	return ERR_NONE;
	#endif

	LOG_NOR("Setting Volume ...\n");
	ret = DacRegisterRead(VOLUME, &reg_value);
	if (ERR_NONE == ret)
	{
		reg_value &= (1 << MUTE | 1 << SLEW); // get rid of all volume bits, leaving MUTE and SLEW
		reg_value |= value; // add in new volume setting
		DacRegisterWrite(VOLUME, reg_value);
	}

	return ret;
} // SetAttenaution()

/*-------------------------------------------------------------------------------------------------------------------*/
/*
*  FUNCTION: 		DacGetAttenuation
*  PARAMETERS:		none -
*  DESCRIPTION:	Reads the atten register & returns the value for displaying volume
*                  returns single attn value, left and right always set the same
*                  an outside routine should map Codec Attn values to MOST volume values
*  RETURNS:		8 bit attenuation value
*/
/*-------------------------------------------------------------------------------------------------------------------*/
int16_t DacGetAttenuation(void)
{
	uint8_t reg_value;
	int16_t ret;

	#ifdef DEV_USE_BUB85650
	return 0;
	#endif

	ret = DacRegisterRead(VOLUME, &reg_value);
	if (ERR_NONE == ret)
	{
		reg_value &= ~(1 << MUTE | 1 << SLEW); // get rid of MUTE and SLEW - leaving only attn bits
		return ((int16_t) reg_value);
	}
	else
	return (ret);

} // DacGetAttenuation()

/*-------------------------------------------------------------------------------------------------------------------*/
/*
*  FUNCTION: 		DacGetStatus
*  PARAMETERS:		none -
*  DESCRIPTION:	Reads the STATUS A register on the DAC
*  RETURNS:		status register & prints out the status - call backs can be triggered here for notification
*/
/*-------------------------------------------------------------------------------------------------------------------*/
int16_t DacGetStatus(void)
{
	uint8_t status;
	int16_t ret;

	LOG_NOR("\nReading DAC Status A... \n");

	ret = DacRegisterRead(STATUS_A, &status);
	if (ERR_NONE == ret)
	{
		// go through bits 1 at a time to see what's set
		// here we are just printing results - could add callbacks to take action for each event
		LOG_NOR("DAC PLL: ");
		if (status & (1 << LCK))
		{
			LOG_NOR("locked\n");
		}
		else
		{
			LOG_NOR("unlocked\n");
		}

		LOG_NOR("Headphone status: ");
		if (status & (1 << SHPS))
		{
			LOG_NOR("connected\n");
		}
		else
		{
			LOG_NOR("disconnected\n");
		}

		if (status & (1 << IOHL))
		LOG_NOR("Left Headphone channel shorted\n");
		if (status & (1 << IOHR))
		LOG_NOR("Right Headphone channel shorted\n");

		return ((int16_t) status);

	}
	else
	return (ret);
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
*  FUNCTION: 		DacGetStatusB
*  PARAMETERS:		none -
*  DESCRIPTION:	Reads the STATUS B register on the DAC
*  RETURNS:		status B - just how the DAC is configured - no interrupt events here
*/
/*-------------------------------------------------------------------------------------------------------------------*/
int16_t DacGetStatusB(void)
{
	uint8_t status;
	int16_t ret;

	LOG_NOR("\nReading DAC Status B... \n");
	ret = DacRegisterRead(STATUS_B, &status);
	if (ERR_NONE == ret)
	{
		return ((int16_t) (status));
	}
	return (ret);
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
*  FUNCTION: 		DacSetEvents
*  PARAMETERS:		byte to set the INT_ENABLE events
*  DESCRIPTION:	Sets the INT_ENABLE register in DAC with value passed in
*  RETURNS:		>0 = success, <0 is one of the I2C errors
*/
/*-------------------------------------------------------------------------------------------------------------------*/
int16_t DacSetEvents(uint8_t int_events)
{
	LOG_NOR("Setting DAC Int Enable Events ... \n");

	return (DacRegisterWrite(INT_ENABLE, int_events));
}

