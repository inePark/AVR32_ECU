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

Module  : Driver for MAX9850 DAC, header file
Version : 1.0
File    : dac_max9850.h
Date    : 09/25/2007
Author  : Gary Warren
Company : SMSC-AIS


Modifications:

09/25/2007	GW		Creation of driver

*****************************************************/
#ifndef __CODEC__
#define __CODEC__
#include <stdint.h>

/****************************************************************************
  Function definitions
****************************************************************************/
int16_t 	DacInit(void);
int16_t 	DacInitFast(void);

int16_t 	DacMute(void);
int16_t 	DacDemute(void);
uint8_t 	DacIsMuted(void);

int16_t 	DacSetAttenuation(uint8_t value);
int16_t 	DacGetAttenuation(void);

int16_t	    DacGetStatus(void);
int16_t 	DacGetStatusB(void);
int16_t 	DacSetEvents(uint8_t int_events);

Type_Result DacRegisterRead(uint8_t regAddr, uint8_t * regData_ptr);
Type_Result DacRegisterWrite(uint8_t regAddr, uint8_t regData);


/****************************************************************************
  Register and bit definitions for DAC9850
****************************************************************************/


#define STATUS_A	0x00		// Read Only - several status bits
#define ALERT		7			// "interrupt" signal
#define SGPIO		6			// State of GPIO pin
#define LCK			5			// PLL Locked
#define SHPS		4			// HeadPhone connected status
#define VMIN		3			// Volume at Min
#define IOHL		1			// Headphone overcurrent Left
#define IOHR		0			// Headphone overcurrent Right

#define STATUS_B	0x01		// Read only - more status bits
#define SMONO		4			// Headphone mono status
#define SHP			3			// HeadPhones operating
#define SLO			2			// Line out outputs enabled
#define SLI			1			// Line in inputs enabled
#define SDAC		0			// DAC operating - valid start sequence

#define VOLUME		0x02		// Volume register (R/W) - 6 volume bits 2 control bits
#define MUTE		7			// Mute/Unmute headphone outputs
#define SLEW		6			// Enable volume slew control
#define VOL_5		5			// MSB of 6 bit volume value

#define DAC_GP		0x03		// General Purpose Reg (R/W)
#define GPIOM_1		7			// msb of GM(1:0) GPIO mode control
#define GPIOM_0		6			// lsb of GM(1:0) GPIO mode control
#define GPD			5			// GPIO Direction
#define DBDEL_1		4			// msb of Debounce Delay
#define DBDEL_0		3			// lsb of Debounce Delay
#define MONO		2			// Headphone mono control
#define ZDEN		0			// Zero crossing enable - changes happen on zero crossings

#define INT_ENABLE	0x04		// Interrupt Enable register (R/W)
#define ISGPIO		6			// Change on GPIO Input causes ALERT - only for GPIO in
#define ICLK		5			// Change in PPL Lock causes ALERT
#define ISHPS		4			// Change in headphone stats (SHPS) causes ALERT
#define IVMIN		3			// Change in VMIN causes ALERT
#define IIOH		0			// Change in IOHR or IOHL causes ALERT

#define DAC_ENABLE	0x05		// Enable control bits (R/W)
#define SHDN		7			// 0 = ShutDown, 1 = powered up
#define MCLKEN		6			// MCLK is active
#define CPEN_1		5			// Enable Charge Pump
#define CPEN_0		4			// Enable Charge Pump (Both =1 to enable)
#define HPEN		3			// HeadPhone Enable
#define LNOEN		2			// LineOut enabled (must be for headphones to be enabled)
#define LNIEN		1			// LineIn enabled
#define DACEN		0			// DAC enable - enables all digital audio

#define DAC_CLOCK	0x06		// Clock divider bits
#define IC_1		3			// msb of Clock divider
#define IC_0 		2			// lsb of Clock divider	IC(1:0) 00=1, 01=2, 02=3, 03=4

#define CHARGE_PUMP	0x07		// Charge Pump Register
#define SR_1		7			// msb of Slew Rate Control
#define SR_0		6			// lsb of Slew Rate Control
#define CP_4		4			// msb of CP(4:0) - charge pump clock divider

#define LRCLK_MSB	0x08		// LR clock divider - 15 bits
#define LRCLK_INT	7			// INT bit, 1 - divider ratio is integer
// MSB(14:8) take up remaining 7 bits

#define LRCLK_LSB	0x09		// LR clock divider - lower 8 bits
// LSB(7:0) use all 8 bits

#define DIG_AUDIO	0x0A		// Digital Audio Settings - I2S selection
#define MASTER		7			// DAC sources clocks (or not)
#define INV			6			// swap Left / Right INV=0 for I2S
#define BCINV		5			// Bit Clock invert = 0 for I2S
#define LSF			4			// Least Significant bit first  = 0 for I2S
#define SD_DELAY	3			// Audio latched on 1st or 2nd clock = 1 for I2S
#define RJT			2			// Right justified data	= 0 for I2S
#define WLS_1		1			// msb of word legnth select
#define WLS_0		0			// lsb of word length select = 00 for 16 bit

#endif


