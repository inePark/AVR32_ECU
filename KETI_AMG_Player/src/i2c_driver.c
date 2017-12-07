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

 Module  : I2C Master driver
 Version : 2.0
 File    : i2c_driver.c
 Date    : 7/7/2006
 Author  : Gary Warren
 Company : SMSC - AIS
 Comments:


 Description:
 A driver for I2C master mode on Atmel AVR using
 the TWI hardware in a polled mode.


 Modifications:

 11/03/2006  GW      Modified to use DEBUG_PRINT... macros for all printing

 07/07/2006  GW    Created from I2C Driver used in Big Ring
 project AppWave


 *****************************************************/
#include <stdint.h>
#include <asf.h>
#include "defines.h"
#include "i2c_driver.h"
#include "timers.h"

// TODO:  wha?
//extern static int twi_set_speed(volatile avr32_twi_t *twi, unsigned int speed, unsigned long pba_hz);

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 *  FUNCTION:     i2c_master_init
 *  PARAMETERS:      none -
 *  DESCRIPTION:  Robust I2C initializer routine - first checks state of I2C
 *                  bus via GPIO checks on I2C lines.  If not both high, tries
 *                  to clear bus by clocking bits and generating stop conditions.
 *                  If this fails, it will reset all I2C perepherals that it can
 *                  (CODEC, and INIC) and try again.
 *                  Finally if I2C lines are are high, initializes the TWI hardware on ATMEGA1281
 *             I2C Interrupts are not enabled, so this is for polling mode
 *                  I2C Master Mode - 400 kHz
 *  RETURNS:      nothing
 */
/*-------------------------------------------------------------------------------------------------------------------*/
Type_Result i2c_master_init(void)
{
	Type_Result status;
    static const gpio_map_t TWIM_GPIO_MAP = { 
		{ AVR32_TWIMS0_TWCK_1_PIN, AVR32_TWIMS0_TWCK_FUNCTION },
		{ AVR32_TWIMS0_TWD_1_PIN, AVR32_TWIMS0_TWD_FUNCTION } 
	};
	static const twim_options_t TWIM_OPTIONS = {
		.pba_hz = FOSC0,
		.speed	= DRVI2C_SPEED_CODEC,
		.chip	= 0,
		.smbus	= false,
	};
    
	// TWI gpio pins configuration
    gpio_enable_module(TWIM_GPIO_MAP, sizeof(TWIM_GPIO_MAP) / sizeof(TWIM_GPIO_MAP[0]));

    // initialize TWI driver with options
    status = twim_master_init(&AVR32_TWIM0, &TWIM_OPTIONS);

    // check init result
	/*
    if (status != STATUS_OK)
    {
        status = ERR_DRVI2C_BASE - status;
        LOG_ERR("ERROR - I2C init failed\r\n");
    }
	*/

    return status;
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
int16_t DRVI2C_pv_TraceData(twi_package_t *package_ptr, TYPE_BOOL read)
{
	uint16_t i;

	LOG_DBG("[%d]\n", TmrGetMasterTick_Long());
	
	LOG_DBG("[%02X]", (uint8_t)(package_ptr->chip) << 1);

	if (read) {
		LOG_DBG("[R]");
	} else {
		LOG_DBG("[W]");
	}
	if (package_ptr->addr_length != 0) {
		for(i=0; i<package_ptr->addr_length; i++) {
			LOG_DBG("%02X", (uint8_t)package_ptr->addr[i]);
		}
		LOG_DBG("s");
	}
	for(i=0; i < package_ptr->length; i++) {
		LOG_DBG("%02X.", *((uint8_t*)(package_ptr->buffer)+i));
	}
	LOG_DBG("\n");

	return ERR_NONE;
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
int16_t inic_read(uint8_t *buf, int buflen)
{
    volatile avr32_twi_t *twi = &AVR32_TWIM0;
	twi_package_t packet;
	status_code_t	status;
    int16_t length;

	packet.chip = I2C_INIC_ADDR;
	packet.addr_length = 0;
	packet.length = buflen;	//PML Length
	packet.buffer = (void*)buf;
	status = twi_master_read (twi, &packet);
	if(status != STATUS_OK) {
		LOG_ERR("twi_master_read fail, status=%d\n", status);
		return ERR_DRVI2C_READ;
	}
	
	length = ((uint16_t)(buf[0]) << 8);
	length |= buf[1];	
	packet.length = length+2;
	
	DRVI2C_pv_TraceData(&packet,TRUE);

    return ERR_NONE;
}

int16_t dac_read(uint8_t *buf, int length, uint8_t addr)
{
	volatile avr32_twi_t *twi = &AVR32_TWIM0;
	twi_package_t packet;
	status_code_t	status;

	packet.chip = CODEC_ADDR;
	packet.addr_length = 1;
	packet.addr[0] = addr;
	packet.length = 1;
	packet.buffer = (void*)buf;
	
	status = twi_master_read (twi, &packet);
	if(status != STATUS_OK) {
		LOG_ERR("twi_master_read fail, status=%d\n", status);
		return ERR_DRVI2C_READ;
	}
	
	DRVI2C_pv_TraceData(&packet,TRUE);

	return ERR_NONE;

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
int16_t inic_write(uint8_t *buf, int buflen)
{
	volatile avr32_twi_t *twi = &AVR32_TWIM0;
	twi_package_t packet;
	status_code_t	status;

	packet.chip = I2C_INIC_ADDR;
	packet.addr_length = 0;
	packet.length = buflen;
	packet.buffer = (void*)buf;
	status = twi_master_write (twi, &packet);
	if(status != STATUS_OK) {
		LOG_ERR("twi_master_write fail, status=%d\n", status);
		return ERR_DRVI2C_WRITE;
	}

	DRVI2C_pv_TraceData(&packet, FALSE);

	return ERR_NONE;
}

int16_t dac_write(uint8_t *buf, int buflen, int8_t addr)
{
	volatile avr32_twi_t *twi = &AVR32_TWIM0;
	twi_package_t packet;
	status_code_t	status;

	packet.chip = CODEC_ADDR;
	packet.addr_length = 1;
	packet.addr[0] = addr;
	packet.length = buflen;
	packet.buffer = (void*)buf;
	status = twi_master_write (twi, &packet);
	if(status != STATUS_OK) {
		LOG_ERR("twi_master_write fail, status=%d\n", status);
		return ERR_DRVI2C_WRITE;
	}

	DRVI2C_pv_TraceData(&packet,FALSE);

	return ERR_NONE;
}


/*-------------------------------------------------------------------------------------------------------------------*/