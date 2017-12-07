/*****************************************************

Module  : Net Services I2C low level driver (LLD)
Version : 1.0
File    : i2c_lld.c
Date    : 01/12/2006
Author  : Gary Warren
Company : SMSC - AIS
Comments:


Description:
	Has the NetServices call back functions for the I2C
	low level driver


Modifications:


01/12/2006	GW		Creation of driver

*****************************************************/
#include <stdint.h>
#include "defines.h"
#include "mostns.h"

#include "board_utils.h"
#include "timers.h"
#include "i2c_driver.h"
#include "i2c_lld.h"



uint8_t i2c_temp_buff[64];
uint8_t i2c_pm_buff[64];


uint8_t lld_running;						// NetServices can accept messages when TRUE
uint8_t lld_i2c_rx_retry = FALSE;			// NetServices was busy/buffers full - need to resend last message
uint8_t lld_ready;						// local buffer ready to accept new port message from INIC


/**************************************************************************
*	This first section is the part of the driver that
*	NetServices assumes - from the sample lld.c file
*
**************************************************************************/

//#define HOLD_DUT_IN_RESET()	
//#define RELEASE_DUT_RESET()
void lld_start_interfaces (void)
{
	lld_running = TRUE;
	lld_ready = TRUE;
}

void lld_reset (void)
{
	HOLD_DUT_IN_RESET();
	delay_ms(70);
	RELEASE_DUT_RESET();

}

void lld_on_buf_freed(void)
{
    if (lld_running)
    {
        if (lld_i2c_rx_retry)					// do we have a buffer waiting to send to net services?
		{
		    i2c_rx_callback();
		}
    }
}

/* This LDD function will be called by the PMS to send a certain control
  * message via I²C.
  *
  * @param  handle   of the message buffer containing at least the header of
  *                  the control message. Normally also the payload.
  * @param  add_ptr  points to the extra payload to append, otherwise NULL.
  * @param  add_size is the size of the extra payload referenced by add_ptr.
  * @return TRUE if the message buffer is going to be transmitted, otherwise
  *         FALSE if for any reason a transmission is not possible now.
  *
  */
uint8_t lld_i2c_transmit(HMBMBUF handle, uint8_t *add_ptr, uint16_t add_size)
{
	uint8_t *tgt_ptr;
	uint8_t *src_ptr;
    uint8_t success;
	uint8_t copy_count;

	uint16_t size;
    uint16_t length;

	success = FALSE;                                        //

	if (lld_running)
	{
		tgt_ptr = &i2c_temp_buff[0];                       // this is our local buffer
		size = MbmGetBufLength(handle);
		src_ptr = MbmGetBufData(handle);                    // pointer to NetServices copy of the message
		copy_count = add_ptr ? 2 : 1;				        // copying 2 buffers or just 1?
		length = size + add_size;

		while (copy_count != 0)						        // copy all data into our own buffer
		{
			while (size != 0)
			{
				*(tgt_ptr++) = *(src_ptr++);
				size--;
			}
			if (--copy_count)						        // if extra data, move src pointer and adjust size - copy additional data
			{
				src_ptr = add_ptr;
				size = add_size;
			}
		}
		inic_write(i2c_temp_buff, length);
		PmsTxRelease(handle);                               // signal to NetServices that we're done with the message buffer
		success = TRUE;
	}
    return(success);
}

/** This function is a callback from the assumed I²C device implementation
  * to notify the LLD that data was received on the control channel and stored
  * in the corresponding RX buffer.
  *
  ** assumes i2c_pm_buff has the port message
  *
  * @see lld_running
  * @see lld_start_interfaces()
  * @see lld_on_buf_freed()
  * @see i2c_rx_ptr
  * @see i2c_enable_rx()
  * @see lld_i2c_rx_retry
  */
void i2c_rx_callback(void)
{
    if (lld_running)
    {

        uint16_t size = ((i2c_pm_buff[0] << 8) | // PML + 2 is total size of message
                          i2c_pm_buff[1]) + 2;

        HMBMBUF handle = PmsGetRxBuf(size);

        lld_i2c_rx_retry = handle ? FALSE : TRUE; // if we got a buffer, retry will be FALSE - we can pass NS the message now
        // if no buffer avail, retry is TRUE - we hold the message until NS has buffer
        // signaled by callback lld_on_buf_freed.
        if (!lld_i2c_rx_retry)
        {
            uint8_t *src_ptr = i2c_pm_buff;
            uint8_t *tgt_ptr = MbmGetBufData(handle);

            while (size) // copy message into NS buffer
            {
                *(tgt_ptr++) = *(src_ptr++);
                size--;
            }

            PmsRx(handle); // tell NetServices it has a new message to handle
        }
    }

    /* RX is going to be (re-)enabled if lld_start_interfaces() was not yet
     * called and we ignore incoming data or if we successfully copied the data
     * into a message buffer and gave it to PmsRx().
     */
    if (!lld_i2c_rx_retry)
    {
        lld_ready = TRUE;
    }
}


/*
 *  FUNCTION: 		check_for_port_message
 *
 *  PARAMETERS:		none
 *
 *  DESCRIPTION:	This is the interface routine between the Atmel low level driver and the
 *					NetServices LLD.  Must be called from main loop, or periodically (often)
 *					via operating system timer or state machine.
 *
 *  RETURNS:		nothing - side effects - triggers port message read if data available
 *
 */
void check_for_port_message (void)
{
	s16_t pm_size;

	if (lld_running)										// NetServices ready to accept message
	{
		if (lld_ready)										// not currently holding a previous message
		{
			if (DUT_INT_ASSERTED())	 			    // wait for INT to go low - INIC wants to talk to us
			{
				inic_read(i2c_pm_buff, sizeof(i2c_pm_buff));
				pm_size = (i2c_pm_buff[0] << 8) | (i2c_pm_buff[1]);
				if (pm_size > 0 )							// if 0 or less, then we got an i2c error
				{
					lld_ready = FALSE;						// our i2c_pm_buffer is in use now - cleared when message passed on to NS
					i2c_rx_callback ();
				}
			}
		}
	}
}


