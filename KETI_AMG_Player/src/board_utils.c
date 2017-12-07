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

 Module  : Board Utilities
 Version : 1.0
 File    : board_utils.c
 Date    : 11/02/2006
 Author  : Gary Warren
 Company : SMSC - AIS
 Comments: Has drivers for some of the boards peripherals.
 : This is where several miscelaneous routines end up


 Description:
 for timed blinking of the message and error LED's




 Modifications:
05/27/2011      GW  Added debounce of the 3 pushbuttons and call event routines
                    in main() when a button is pushed. main() decides what to do.

04/17/2009      GW  Took out Temperature routines - temp sensor in MPM85000, so
                    temp routines are there.
                    Took out ConfigFreq - frequency is fixed on Eval110.

11/02/2006      GW  Created

 *****************************************************/
#include <asf.h>
#include <stdint.h>
#include "defines.h"
#include "board_utils.h"
#include "board_gpio.h"

const u8_t hex_table[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
const char hexDigits[16] = "0123456789ABCDEF";


void codec_HoldReset(void)
{
	gpio_clr_gpio_pin(GPIO_CODEC_RESET);
}

void codec_ReleaseReset(void)
{
	gpio_set_gpio_pin(GPIO_CODEC_RESET);
}

void inic_HoldReset(void)
{
	gpio_clr_gpio_pin(GPIO_INIC_RESET);
}

void inic_ReleaseReset(void)
{
	gpio_set_gpio_pin(GPIO_INIC_RESET);
}


void codec_EnableI2c(void)
{
	gpio_clr_gpio_pin(GPIO_CODEC_I2C_EN);
}
void codec_activeI2c(void)
{
	gpio_set_gpio_pin(GPIO_CODEC_I2C_RDY);
}


/*----------------------------------------------------------------------------*/
/*!
 * \brief Brief Description - few words
 *
 * Detailed description goes here. Follows the brief description separated by
 * an empty line
 *
 * \param p1 - the first parameter
 * \param p2 - the second parameter
 *
 * \return Return value description
 *
 * \remarks Special instructions for using the function/side-effects
 *
 * \see Any reference for more details
 *
 * \todo
 *    -# To-do item 1.
 *    -# To-do item 2.
 */
/*----------------------------------------------------------------------------*/
TYPE_U8 *utoa( TYPE_U16 nbr, TYPE_U8 *str, TYPE_S16  sz)
{
  TYPE_S16  lowbit;

  if(sz>0) str[--sz]='\0';
  else if(sz<0) sz = -sz;
  else while(str[sz]!='\0') ++sz;
  while(sz) {
    lowbit=nbr&1;
    nbr=(nbr>>1)&32767;  /* divide by 2 */
    str[--sz]=((nbr%5)<<1)+lowbit+'0';
    if((nbr=nbr/5)==0) break;
    }
  while(sz) str[--sz]='0';
  return str;
  }

/*----------------------------------------------------------------------------*/
/*!
 * \brief Brief Description - few words
 *
 * Detailed description goes here. Follows the brief description separated by
 * an empty line
 *
 * \param p1 - the first parameter
 * \param p2 - the second parameter
 *
 * \return Return value description
 *
 * \remarks Special instructions for using the function/side-effects
 *
 * \see Any reference for more details
 *
 * \todo
 *    -# To-do item 1.
 *    -# To-do item 2.
 */
/*----------------------------------------------------------------------------*/
char * ultoa (TYPE_U32 value, char *string, int radix)
{
  char *dst;
  char digits[32];
  int i, n;

  dst = string;
  if (radix < 2 || radix > 36)
    {
      *dst = 0;
      return string;
    }
  i = 0;
  do
    {
      n = value % radix;
      digits[i++] = (n < 10 ? (char)n+'0' : (char)n-10+'a');
      value /= radix;
    } while (value != 0);
  while (i > 0)
    *dst++ = digits[--i];
  *dst = 0;
  return string;
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 *  FUNCTION:     hex_upper_nibble
 *  PARAMETERS:      value - byte for which we want the ASCII character for upper nibble
 *  DESCRIPTION:  returns the ASCII character of the upper nibble of input byte
 *                  used with hex_lower_nibble to print a full hex byte.
 *  RETURNS:      nothing
 */
/*-------------------------------------------------------------------------------------------------------------------*/
TYPE_U8 hex_upper_nibble (TYPE_U8 val)
{
      return(hex_table[(val & 0xf0)>>4]);
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 *  FUNCTION:     hex_lower_nibble
 *  PARAMETERS:      value - byte for which we want the ASCII character for lower nibble
 *  DESCRIPTION:  returns the ASCII character of the lower nibble of input byte
 *                  used with hex_upper_nibble to print a full hex byte.
 *  RETURNS:      nothing
 */
/*-------------------------------------------------------------------------------------------------------------------*/
TYPE_U8 hex_lower_nibble (TYPE_U8 val)
{
      return(hex_table[(val & 0x0f)]);
}




void init_board_gpio()
{
	uint8_t pin = 0;
	uint8_t gpioList[] = {
		GPIO_SW_4,
//		GPIO_SW_5
		GPIO_CODEC_RESET,
		GPIO_CODEC_I2C_EN,
		GPIO_CODEC_I2C_RDY,
		GPIO_INIC_INT_N,
		GPIO_INIC_RESET,
		LED0_GPIO,
		LED1_GPIO,
		LED2_GPIO,
		LED3_GPIO
	};
	
	for(pin = 0; pin < sizeof(gpioList); pin++) {
		gpio_enable_gpio_pin(gpioList[pin]);
	}
	
	/* LED INIT */
	gpio_set_gpio_pin(LED3_GPIO);
	gpio_set_gpio_pin(LED2_GPIO);
	gpio_set_gpio_pin(LED1_GPIO);
	gpio_set_gpio_pin(LED0_GPIO);
}