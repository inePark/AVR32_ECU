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

Module  : Board Utilities, header file
Version : 1.0
File    : board_utils.h
Date    : 11/02/2006
Author  : Gary Warren
Company : SMSC - AIS
Comments: Has drivers for some of the boards peripherals.
        : This is where several miscelaneous routines end up


Modifications:

11/02/2006  GW      Created

*****************************************************/
#ifndef __BOARD_UTILS__
#define __BOARD_UTILS__

#include <asf.h>
#include "board_gpio.h"

// for temperature sensor
#define START_CONVERT		0x51
#define STOP_CONVERT		0x22
#define READ_TEMP			0xAA
#define TEMP_HIGH			0xA1
#define TEMP_LOW			0xA2
#define TEMP_CONFIG			0xAC
#define TEMP_POR			0x54

#define TEMP_EVENT			0x04			// mask for bit in Amp.EventMask

#define HOLD_DUT_IN_RESET()	inic_HoldReset()
#define RELEASE_DUT_RESET()	inic_ReleaseReset()
void ConfigFreq (void);
void codec_HoldReset(void);
void codec_ReleaseReset(void);
void codec_EnableI2c(void);
void codec_activeI2c(void);
void inic_HoldReset(void);
void inic_ReleaseReset(void);

#define GREEN_LED_ON()
#define RED_LED_ON()
#define YELLOW_LED_ON()
#define YELLOW2_LED_ON()
#define GREEN_LED_OFF()
#define RED_LED_OFF()
#define YELLOW_LED_OFF()
#define YELLOW2_LED_OFF()
#define blink_msg_led()

#define DUT_INT_ASSERTED()    (!gpio_get_pin_value(GPIO_INIC_INT_N))

//__attribute__((__interrupt__))void sw_handler(void);
TYPE_U8 *utoa( TYPE_U16 nbr, TYPE_U8 *str, TYPE_S16  sz);
char * ultoa (TYPE_U32 value, char *string, int radix);
TYPE_U8 hex_upper_nibble (TYPE_U8 val);
TYPE_U8 hex_lower_nibble (TYPE_U8 val);
void init_board_gpio(void);


#endif
