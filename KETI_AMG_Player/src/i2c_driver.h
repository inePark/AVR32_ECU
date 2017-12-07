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

 Module  : I2C Master driver, header file
 Version : 1.0
 File    : i2c_driver.h
 Date    : 9/8/2005
 Author  : Gary Warren
 Company : Oasis Sillicon Systems
 Comments:


 Modifications:


 09/09/2005	GW		Creation of driver

 *****************************************************/
#ifndef __I2C_DRIVER__
#define __I2C_DRIVER__

#include <stdint.h>
#include <asf.h>

#define ERR_DRVI2C_I2C_RTN_OKAY                 (ERR_NONE)
#define ERR_DRVI2C_UNKNOWN                      (ERR_DRVI2C_BASE-1)
#define ERR_DRVI2C_BUS_HELD                     (ERR_DRVI2C_BASE-2)
#define ERR_DRVI2C_TIMEOUT                      (ERR_DRVI2C_BASE-3)
#define ERR_DRVI2C_UNEXPECTED_NAK               (ERR_DRVI2C_BASE-4)
#define ERR_DRVI2C_CLOCK_STRETCH_TIMEOUT        (ERR_DRVI2C_BASE-5)
#define ERR_DRVI2C_READ                         (ERR_DRVI2C_BASE-6)
#define ERR_DRVI2C_WRITE                        (ERR_DRVI2C_BASE-7)
#define ERR_DRVI2C_GENERAL                      (ERR_DRVI2C_BASE-8)
#define ERR_DRVI2C_INVALID_ARGUMENT             (ERR_DRVI2C_BASE-9)
#define ERR_DRVI2C_ARBITRATION_LOST             (ERR_DRVI2C_BASE-10)
#define ERR_DRVI2C_NO_CHIP_FOUND                (ERR_DRVI2C_BASE-11)
#define ERR_DRVI2C_OVERRRUN                     (ERR_DRVI2C_BASE-12)
#define ERR_DRVI2C_ALREADY_INITIALIZED          (ERR_DRVI2C_BASE-13)
#define ERR_DRVI2C_NOT_INITIALIZED              (ERR_DRVI2C_BASE-14)
#define ERR_DRVI2C_ENABLE_TIMEOUT               (ERR_DRVI2C_BASE-15)
#define ERR_DRVI2C_DISABLE_TIMEOUT              (ERR_DRVI2C_BASE-16)
#define ERR_DRVI2C_INVALID_PORT                 (ERR_DRVI2C_BASE-17)

#define I2C_TIMEOUT     1000                                // Wait up to 1 second w/ clock line stuck low before resetting
/****************************************************************************
 Function definitions
 ****************************************************************************/

Type_Result i2c_master_init(void);
int16_t inic_read(uint8_t *buf, int buflen);
int16_t dac_read(uint8_t *buf, int length, uint8_t addr);
int16_t inic_write(uint8_t *buf, int buflen);
int16_t dac_write(uint8_t *buf, int buflen, int8_t addr);

int16_t DRVI2C_pv_TraceData(twi_package_t *package_ptr, TYPE_BOOL read);

#endif
