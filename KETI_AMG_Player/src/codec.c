/*******************************************************************************
 * SMSC Automotive Infotainment Systems Group
 *
 * (c)2002-2009 Standard Microsystems Corporation and its subsidiaries ("SMSC").
 * The copyright of the computer program(s) herein is the property of SMSC.
 * The program(s) may be used and copied only with written permission from SMSC
 * or in accordance with the terms and conditions stipulated in the
 * agreement under which the program(s) have been supplied.
 ******************************************************************************/

/*****************************************************

 Module  : Driver for Cirrus CS42448 8 channel CODEC
 Version : 1.0
 File    : codec.c
 Date    : 09/16/2009
 Author  : Gary Warren
 Company : SMSC - AIS
 Comments:

 Description:
 A driver for the Cirrus Logkc CS42448 Codec

 Modifications:

 09/16/2009  GW    Creation of driver

 *****************************************************/
#include <stdint.h>
#include "defines.h"
#include "mostns.h"
#include "codec.h"
#include "i2c_driver.h"
#include "board_utils.h"
#include "timers.h"
#include "con_mgr.h"
#include <asf.h>


//
//------------------------------------------
#define CODEC_INIT_REGS 6
const u8_t cs42448_defaults[2 * CODEC_INIT_REGS] =
   {  CODEC_POWER_CON,    (1 << PDN), // Set PDN before initializing
      CODEC_FORMATS,      (0 << FREEZE | 1 << AUX_DIF | 0 << DAC_DIF2 | 0 << DAC_DIF1 | 1<< DAC_DIF0 | 0 << ADC_DIF2 | 0 << ADC_DIF1 | 1 << ADC_DIF0), // I2S formats
      CODEC_ADC_CONT,     (1 << ADC1_SINGLE)|(1 << ADC2_SINGLE) | (1 << ADC3_SINGLE),
      CODEC_TRANS_CONT,   (0 << DAC_SNG_VOL | 1 << DAC_SZC1 | 1 << DAC_SZC0 | 1 << ADC_SNG_VOL | 1<< ADC_SZC1 | 1 << ADC_SZC0), // single vol/gain control - slew enabled
      CODEC_STATUS_CONT,  (0 << STAT_INT_1 | 1 << STAT_INT_0), // CODEC Int output is active low
      CODEC_POWER_CON,    (0 << PDN) // Now clear PDN codec initialized and running
      };
// CODEC_ADC1_GAIN,    12  // give adc's +6dB gain

/*
 Type_DrvI2cPackage i2cPackage;
 i2cPackage.chip = DAC_ADDR;
 i2cPackage.addr = map;
 i2cPackage.addr_length = 1;
 i2cPackage.buffer = &data_out;
 i2cPackage.length = 1;

 */

Type_Result codec_RegisterRead(TYPE_U8 regAddr, TYPE_U8 * regData_ptr)
{
	Type_Result result;

	result = dac_read(regData_ptr, 1, regAddr);
	

	if (0 <= result)
	{
		return ERR_NONE;
	}

	return result;
}

Type_Result codec_RegisterWrite(TYPE_U8 regAddr, TYPE_U8 regData)
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
 *  FUNCTION:     CODEC_setDefaultValues
 *  PARAMETERS:      none -
 *  DESCRIPTION:  A "debug" version of initializing CODEC
 *                  Prints each register's initial and final value as it programs them
 *  RETURNS:      >0 = success, <0 is one of the I2C errors
 */
/*-------------------------------------------------------------------------------------------------------------------*/
s16_t CODEC_setDefaultValues(void)
{
	u8_t map;
	u8_t data_out;
	u8_t data_in;
	u8_t i;
	s16_t ret;

	// codec being held in reset till now, let it go
	codec_ReleaseReset();
	LOG_NOR("Initializing CS42448 CODEC ... \n");
	delay_ms(5);

	for (i = 0; i < CODEC_INIT_REGS; i++) // Setting register defaults of MAX9850
	{

		map = (cs42448_defaults[i * 2 + 0]); // Register adr.
		data_out = cs42448_defaults[i * 2 + 1]; // Register value (byte)

		ret = codec_RegisterWrite(map, data_out);
		if (ERR_NONE == ret)
		{
			MyNode.CodecPresent = TRUE; // codec responded
			delay_ms(1); // codec must have some time between commands
			codec_RegisterRead(map, &data_in);
			LOG_NOR("CS42448 Register, Value written, Value read: %02X %02X %02X\n", map, data_out, data_in);
		} else {
			MyNode.CodecPresent = FALSE; // can't talk to codec
			LOG_ERR("Error writing to Codec\n");
			break;
		}
	}
	// if everything was OK, now enable DAC (take out of Shutdown)
	if (ERR_NONE == ret)
	{
		LOG_NOR("Initializing done - CODEC enabled now ... \n");
	}

	return (ret);
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 *  FUNCTION:     CODEC_setDefaultValuesFast
 *  PARAMETERS:      none -
 *  DESCRIPTION:  Same as above, but does not print each register's value
 *  RETURNS:      >0 = success, <0 is one of the I2C errors
 */
/*-------------------------------------------------------------------------------------------------------------------*/
s16_t CODEC_setDefaultValuesFast(void)
{
   u8_t map;
   u8_t data_out;
   u8_t i;
   s16_t ret;

#ifdef DEV_USE_BUB85650
   return ERR_NONE;
#endif

   // codec being held in reset till now, let it go
   codec_ReleaseReset();
   LOG_NOR("Initializing CS42448 CODEC ... \n");
   for (i = 0; i < CODEC_INIT_REGS; i++) // Setting register defaults of MAX9850
   {

      map = (cs42448_defaults[i * 2 + 0]); // Register adr.
      data_out = cs42448_defaults[i * 2 + 1]; // Register value (byte)

      ret = codec_RegisterWrite(map, data_out);
      if (ERR_NONE == ret)
      {
         MyNode.CodecPresent = TRUE; // codec responded
         delay_ms(1); // codec must have some time between commands
      } else
      {
         MyNode.CodecPresent = FALSE; // can't talk to codec
         break;
      }
   }

   return (ret);
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 *  FUNCTION:     CODEC_mute
 *  PARAMETERS:      none -
 *  DESCRIPTION:  Mutes the DAC output - used by Amp.Mute command
 *  RETURNS:      >0 = success, <0 is one of the I2C errors
 */
/*-------------------------------------------------------------------------------------------------------------------*/
s16_t CODEC_mute(void)
{
   s16_t result;

   LOG_NOR("Muting Codec ... \n");
   result = codec_RegisterWrite(CODEC_DAC_MUTE, 0xFF);

   return result;

} // Mute()

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 *  FUNCTION:     CODEC_demute
 *  PARAMETERS:      none -
 *  DESCRIPTION:  Un-Mutes the DAC output - used by Amp.Mute command
 *  RETURNS:      >0 = success, <0 is one of the I2C errors
 */
/*-------------------------------------------------------------------------------------------------------------------*/
s16_t CODEC_demute(void)
{
   s16_t result;

#ifdef DEV_USE_BUB85650
   return ERR_NONE;
#endif

   LOG_NOR("DeMuting Codec ... \n");
   result = codec_RegisterWrite(CODEC_DAC_MUTE, 0x00);
   return result;
} // DeMute()

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 *  FUNCTION:     CODEC_isMuted
 *  PARAMETERS:      none -
 *  DESCRIPTION:  Check mute setting of codec, returns status
 *  RETURNS:      boolean - TRUE if muted, FALSE if not
 */
/*-------------------------------------------------------------------------------------------------------------------*/
u8_t CODEC_isMuted(void)
{
   u8_t reg_value;
   s16_t result;

   result = codec_RegisterRead(CODEC_DAC_MUTE, &reg_value);
   if (ERR_NONE == result)
   {
      if (0x00 == reg_value) // current driver mutes all or none so 0x00 => no mute bits set, not muted
         return FALSE;
      else
         return TRUE;
   } else
   {
      return (FALSE);
   }
} // CODEC_isMuted()


/*-------------------------------------------------------------------------------------------------------------------*/
/*
 *  Function:        void CODEC_setVolume( value);
 *  Description:     Sets the volume of the CS42448 CODEC
 *                   Scales volume from MOST (0 = Mute -> 40 = MAX) range to
 *                   Codec range (00 = Max, FF = -127.5dB(mute) )
 *  Input(s):        Volume value passed in - Must range from 0 to 40
 *  Side-effect(s):  Changes volume
 *  Return(s):       -none-
 */
/*-------------------------------------------------------------------------------------------------------------------*/
void CODEC_setVolume(u8_t ch, u8_t value)
{

   u8_t attn;

   // Volume to attenuation mapping, map 40 steps into 256, invert sense
   // some values    volume   attn
   //                  0       FF (255)  mute - special case
   //                  1       4E ( 78)  -39 dB
   //                  5       46 ( 70)  -35 dB
   //                 10       1E ( 60)  -30 dB
   //                 20       28 ( 40)  -20 dB
   //                 30       14 ( 20)  -10 dB
   //                 38       04 (  4)  - 2 dB
   //                 39       02 (  2)  - 1 dB
   //                 40       00 (  0)    0 dB
   if (0 == value)      // special case, 0 causes attn to be bigger than byte
   {
      attn = 0xFF;                  // full attenuation
   }
   else
   {
      attn = (40 - value)*2;     // change sense & make each step 1 dB
   }
   CODEC_setAttenuation(ch, attn);
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 *  Function:        u8_t CODEC_getVolume(void);
 *  Description:     Reads the attenuation setting of the CS42448 CODEC and
 *                   scales it back to the MOST volume range
 *                   Scales from Codec range (00 = Max, 0xFF = mute)
 *                   to MOST range (0 = Mute -> 0x28 = MAX)
 *  Input(s):        None
 *  Side-effect(s):  None
 *  Return(s):       8 bit MOST volume range from 0 to 40
 */
/*-------------------------------------------------------------------------------------------------------------------*/

u8_t CODEC_getVolume(u8_t ch)
{

   u8_t volume = 0;
   s16_t interimVol;       // do calculations at 16 bits.
   // attenuation to volume mapping, inverse of mapping table above

   interimVol = CODEC_getAttenuation(ch); // read attenuator setting
   if (interimVol >= 0)    // <0 ==> read error
   {
      volume = (u8_t) interimVol;
      if (0xFF == volume)
      {
         volume = 0;
      }
      else
      {
         volume = 40 - (volume/2);
      }
   }
   return (volume);
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 *  FUNCTION:     CODEC_setAttenuation
 *  PARAMETERS:      value - the value to set the volume to
 *  DESCRIPTION:  Set the volume of the DAC output - used by Amp.Volume command
 *                  0x00 (loud) ... 0x28 (quiet)
 *                  sets left and right to same value
 *                  an outside routine should map MOST volume values to Codec Attn values
 *  RETURNS:      >0 = success, <0 is one of the I2C errors
 */
/*-------------------------------------------------------------------------------------------------------------------*/
s16_t CODEC_setAttenuation(u8_t ch, u8_t value)
{
   s16_t ret;
   u8_t regAddr;
   
   regAddr = (ch *2) + CODEC_DAC1_VOL;

   LOG_NOR("Setting Volume ...\n");
   ret = codec_RegisterWrite(regAddr, value);
   ret = codec_RegisterWrite((regAddr+1), value);
   return ret;
} // SetAttenaution()

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 *  FUNCTION:     CODEC_getAttenuation
 *  PARAMETERS:      none -
 *  DESCRIPTION:  Reads the atten register & returns the value for displaying volume
 *                  returns single attn value, left and right always set the same
 *                  an outside routine should map Codec Attn values to MOST volume values
 *  RETURNS:      8 bit attenuation value
 */
/*-------------------------------------------------------------------------------------------------------------------*/
s16_t CODEC_getAttenuation(u8_t ch)
{
	u8_t regAddr;
	u8_t reg_value;
	s16_t ret;

	regAddr = (ch*2) + CODEC_DAC1_VOL;
	ret = codec_RegisterRead(regAddr, &reg_value);
	if (ERR_NONE == ret)
	{
		return ((s16_t) reg_value);
	}
	else
	{
		return (ret);
	}
} // CODEC_getAttenuation()

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 *  FUNCTION:     CODEC_getStatus
 *  PARAMETERS:      none -
 *  DESCRIPTION:  Reads the STATUS A register on the DAC
 *  RETURNS:      status register & prints out the status - call backs can be triggered here for notification
 */
/*-------------------------------------------------------------------------------------------------------------------*/
s16_t CODEC_getStatus(void)
{
   u8_t status;
   s16_t ret = ERR_NONE;
   
   LOG_NOR("\nReading DAC Status A... \n");

   ret = codec_RegisterRead(CODEC_STATUS_REG, &status);
   if (ERR_NONE == ret)
   {
      // go through bits 1 at a time to see what's set
      // here we are just printing results - could add callbacks to take action for each event
      return ((s16_t) status);

   } else
      return (ret);
}


