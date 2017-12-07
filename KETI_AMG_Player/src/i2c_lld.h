/*****************************************************

Module  : NetServices I2C Low Level Driver, header file
Version : 1.0
File    : i2c_lld.h
Date    : 01/12/2006
Author  : Gary Warren
Company : Oasis Sillicon Systems
Comments:


Modifications:

01/12/2006	GW		Creation of driver

*****************************************************/

/****************************************************************************
  Function definitions
****************************************************************************/
#ifndef _I2C_LLD_H_
#define _I2C_LLD_H_

#include <stdint.h>
#include "mbm_pb.h"

void lld_start_interfaces(void);
void lld_reset(void);
void lld_on_buf_freed(void);
uint8_t lld_i2c_transmit(HMBMBUF handle, uint8_t *add_ptr, uint16_t add_size);
void i2c_rx_callback(void);
void check_for_port_message(void);

#endif

