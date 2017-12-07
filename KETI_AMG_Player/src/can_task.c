/**
 * \file
 *
 * \brief Application CAN Task Management.
 *
 * Copyright (c) 2011 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
#include <asf.h>
#include <stddef.h>
#include <stdio.h>
//#include <unistd.h>
#include <avr32/io.h>
#include "compiler.h"
#include "board.h"
#include "power_clocks_lib.h"
#include "gpio.h"
#include "gui.h"
#include "pm_uc3c.h"
#include "scif_uc3c.h"
#include "print_funcs.h"
#include "can_task.h"
#include "conf_can_task.h"
#include "dsp.h"
#include "conf_demo.h"
#include "defines.h"


// Extern Variables
extern fw_state_t gFwState;

//! Local Mob Declaration
can_msg_t mob_ram_ch0[NB_MOB_CHANNEL];
can_msg_t mob_ram_ch1[NB_MOB_CHANNEL];

//! Local function to prepare RX and TX buffers
void can_example_prepare_data_to_send(void);
void can_example_prepare_data_to_send2(unsigned char);
void can_example_prepare_data_to_receive(void);
void can_out_callback_channel0(U8 handle, U8 event);
void can_out_callback_channel1(U8 handle, U8 event);
//! External reference on ADC Current Conversion
extern volatile uint16_t adc_current_conversion;

unsigned char process_ecu_comm (U32 can_id, U64 input_data);


//! Buffer to store ADC Conversions
A_ALIGNED dsp16_t signal3_buf[BUFFER_LENGTH];
A_ALIGNED unsigned char version_flag;
A_ALIGNED unsigned counter_;

//! Boolean for message transmitted on CAN channel 1
volatile bool message_transmitted_on_channel1 = false;
/*! \brief CAN Call Back when message is transmitted
 *
 */
void can_out_callback_channel1(U8 handle, U8 event)
{
	gpio_tgl_gpio_pin(LED1_GPIO);
	// Transmission Only
	can_mob_free(1,handle);
	message_transmitted_on_channel1 = true;
}

//! Boolean for message transmitted on CAN channel 0
volatile bool message_received_on_channel0 = false;
/*! \brief CAN Call Back when message is received
 */
void can_out_callback_channel0(U8 handle, U8 event)
{
	gpio_tgl_gpio_pin(LED3_GPIO);
	// Reception Only
	pCANMOB_message2[0].can_msg->data.u64 = can_get_mob_data(0,handle).u64;
	pCANMOB_message2[0].can_msg->id = can_get_mob_id(0,handle);
	pCANMOB_message2[0].dlc = can_get_mob_dlc(0,handle);
	pCANMOB_message2[0].status = event;
	can_mob_free(0,handle);
	message_received_on_channel0 = true;
	
	//printf ("recv data via can0\n");
}

void can_task_init(void)
{
	// Setup the generic clock for CAN
	scif_gc_setup(AVR32_SCIF_GCLK_CANIF,
		SCIF_GCCTRL_OSC0,
		AVR32_SCIF_GC_NO_DIV_CLOCK,
		0);
	// Now enable the generic clock
	scif_gc_enable(AVR32_SCIF_GCLK_CANIF);
	static const gpio_map_t CAN_GPIO_MAP = {
		{AVR32_CANIF_RXLINE_0_0_PIN, AVR32_CANIF_RXLINE_0_0_FUNCTION},
		{AVR32_CANIF_TXLINE_0_0_PIN, AVR32_CANIF_TXLINE_0_0_FUNCTION}
	};
	// Assign GPIO to CAN.
	gpio_enable_module(CAN_GPIO_MAP,
		sizeof(CAN_GPIO_MAP) / sizeof(CAN_GPIO_MAP[0]));
	/*	can_example_prepare_data_to_send();
	*/	can_example_prepare_data_to_receive();

}

/*! \brief CAN Task:
 *        - Check if messages are correctly transmitted/received
 *        - Update message for transmission/reception
 */
void can_task(void)
{
	static bool rcvFlag = false;
	static unsigned tx_counter_ = 0;

	if (!rcvFlag)	{
		can_example_prepare_data_to_receive();
		rcvFlag = true;
		printf ("can recv prepare\n");
	}		

	if (counter_ ==  55){
		counter_ = 0;
		tx_counter_ ++;

		pCANMOB_message0[0].can_msg->id = 0x6FC;
		pCANMOB_message0[0].can_msg->data.u64 = (0x1234FFFFFFFFFF01 + tx_counter_);
		can_example_prepare_data_to_send();						
	}

	if (message_received_on_channel0 == true) {
		message_received_on_channel0 = false;
		rcvFlag = false;
		
		printf ("RECV from ECU mng via CAN\n");
		 
		process_ecu_comm(pCANMOB_message2[0].can_msg->id, pCANMOB_message2[0].can_msg->data.u64);						
	}
}

/*! \brief CAN Prepare Data to Send
 *        - Allocate one MOB in transmission
 *        - Fill the MOB with the correct DATA
 *        - Start the transmission
 */
/* modified by park to communicate via channel 1 */
void can_example_prepare_data_to_send(void)
{
	//static unsigned counter_ = 0;
	// Initialize channel 1
	can_init(0, ((U32)&mob_ram_ch1[0]), CANIF_CHANNEL_MODE_NORMAL, can_out_callback_channel1);
	// Allocate one mob for TX
	pCANMOB_message0[0].handle = can_mob_alloc(0);

	// Check return if no mob are available
	if (pCANMOB_message0[0].handle==CAN_CMD_REFUSED) {
		while(true);
	}
		
	can_tx(0, pCANMOB_message0[0].handle, pCANMOB_message0[0].dlc, pCANMOB_message0[0].req_type, pCANMOB_message0[0].can_msg);
}

/*! \brief CAN Prepare Data to Receive
 *        - Allocate one MOB in reception
 *        - Start the reception
 */
void can_example_prepare_data_to_receive(void)
{
	// Initialize channel 0
	can_init(0, ((U32)&mob_ram_ch0[0]), CANIF_CHANNEL_MODE_NORMAL, can_out_callback_channel0);

	// Allocate one mob for RX
	pCANMOB_message2[0].handle = can_mob_alloc(0);

	can_rx(0, pCANMOB_message2[0].handle, pCANMOB_message2[0].req_type, pCANMOB_message2[0].can_msg);
}

/*! \brief CAN Prepare Data to Send
 *        - Allocate one MOB in transmission
 *        - Fill the MOB with the correct DATA
 *        - Start the transmission
 */
void can_example_prepare_data_to_send2(unsigned char order_)
{
	can_init(1, ((U32)&mob_ram_ch1[0]), CANIF_CHANNEL_MODE_NORMAL, can_out_callback_channel1);
						
	pCANMOB_message0[0].handle = can_mob_alloc(1);
	if (pCANMOB_message0[0].handle==CAN_CMD_REFUSED) {
		while(true);
	}
			
	pCANMOB_message0[0].can_msg->id = 0x06FF;
				
	switch (order_)	{
		case UPDATE:
			//pCANMOB_message0[0].can_msg->id = 0x06FF;
			pCANMOB_message0[0].can_msg->data.u64 = 0x55ffffffffffffff;
			break;
		case DOWNGRADE:
			//pCANMOB_message0[0].can_msg->id = 0x06FF;
			pCANMOB_message0[0].can_msg->data.u64 = 0x44ffffffffffffff;
			break;
		case FRONT_UP:
			//pCANMOB_message0[0].can_msg->id = 0x6FF;
			pCANMOB_message0[0].can_msg->data.u64 = 0x4675ffffffffffff;
			break;
		case FRONT_DOWN:
			//pCANMOB_message0[0].can_msg->id = 0x06FF;
			pCANMOB_message0[0].can_msg->data.u64 = 0x4664ffffffffffff;
			break;
		case REAR_UP:
			//pCANMOB_message0[0].can_msg->id = 0x6FF;
			pCANMOB_message0[0].can_msg->data.u64 = 0x5275ffffffffffff;
			break;
		case REAR_DOWN:
//			pCANMOB_message0[0].can_msg->id = 0x6FF;
			pCANMOB_message0[0].can_msg->data.u64 = 0x5264ffffffffffff;
			break;
		default:
//			pCANMOB_message0[0].can_msg->id = 0x6FF;
			pCANMOB_message0[0].can_msg->data.u64 = 0xffffffffffffffff;
			break;
	}
			

	can_tx(1, pCANMOB_message0[0].handle, pCANMOB_message0[0].dlc, pCANMOB_message0[0].req_type, pCANMOB_message0[0].can_msg);
}



unsigned char process_ecu_comm (const U32 can_id, const U64 input_data)	{

	/*no care checksum.... we will add checksum macro at later.. */
	
	unsigned char return_value = 0;
	unsigned char buffer_[64];
	unsigned char buffer_2 [64];
	FILE* fp;
	
	switch (can_id)		{
		case 0x6FA:	/* request or order packet*/
			//temp_data >>
			if ((input_data & 0xFFFFFFFFFFFFFF00) == 0)	{
				version_flag = (input_data & 0x00000000000000FF);
				return_value = 0;
			}
			switch (input_data & 0xFF000000)	{ /* command */
				case 0x10000000: /* ECUM request to confirm connection */
					if ((input_data & 0xFFFF00) == 0xFF00 /* temporal ECU ID*/)	{
						pCANMOB_message0[0].can_msg->id = 0x6FB;	/* ECU confirm the connection */
						pCANMOB_message0[0].can_msg->data.u64 = 0x00FF100000000000;
						can_example_prepare_data_to_send();
						return_value = 1;
					}
					else
						return_value = 11;
					break;
				case 0x11: /* ECUM notify to start TX of data */
					if ((input_data & 0xFFFF00) == 0)	{
						pCANMOB_message0[0].can_msg->id = 0x6FB;	/* ECU replay */
						pCANMOB_message0[0].can_msg->data.u64 = 0x00FF110010000000;
						can_example_prepare_data_to_send();
						return_value = 2;
					}
					else
						return_value = 12;
					break;
			case 0x12: /*ECUM notify to end TX of data */
				if (((input_data & 0xFFFF00) == 0) || ((input_data & 0xFFFF00) == 0XFF))	{
					/* normal(0) or irregulary(0xFF) end */
					pCANMOB_message0[0].can_msg->id = 0x6FB;	/* ECU replay */
					pCANMOB_message0[0].can_msg->data.u64 = 0x00FF120010000000;
					can_example_prepare_data_to_send();
					return_value = 3;
				}
				else
					return_value = 13;
				break;
			case 0x21000000: /*ECUM request information of SW version*/
				if ((input_data & 0xFFFF00) == 0xFF00 /* temporal ECU ID*/)	{
					pCANMOB_message0[0].can_msg->id = 0x6FB;	/* ECU replay */
					//if (version_flag == 1)
					U64 temp_u64 = 0xFF000000;
					temp_u64 &= version_flag <<24;
					temp_u64 |= 0xFFFFFFFF00FFFFFF;
					pCANMOB_message0[0].can_msg->data.u64 = 0x00FF2100FF000000 & temp_u64;
					can_example_prepare_data_to_send();
					return_value = 4;
				}
				else
					return_value = 14;
				break;
			case 0x22000000: /*ECUM request state of SW update */
				if ((input_data & 0xFFFF00) == 0xFF00 /* temporal ECU ID*/)	{
					version_flag = (input_data & 0xFF00000000000000) >> 56;
					pCANMOB_message0[0].can_msg->id = 0x6FB;	/* ECU replay */
					pCANMOB_message0[0].can_msg->data.u64 = 0x00FF220064000000;
					can_example_prepare_data_to_send();
					return_value = 5;
				}
				else
					return_value = 15;
				break;
			default:
				return_value = 123;
				break;
		}
		//		return_value = input_data;
		break;

	case 0x6FC:	/* binary control data */
		if ((input_data & 0xFFFF000000000000) == 0xFF /* temporal ECU ID*/)	{
			pCANMOB_message0[0].can_msg->id = 0x6FB;	/* ECU replay */
			pCANMOB_message0[0].can_msg->data.u64 = 0x00FF300000000000;
			can_example_prepare_data_to_send();
			return_value = 10;
		}
		else
			return_value = 234;
		break;

	case 0x6FF:	/* binary-content data-2 */
		fp = fopen ("asdf","wb");
		fwrite(&input_data, 1, 16, fp);
		fclose(fp);

		fp = fopen ("asdf", "rb");
		fread (buffer_, 1, 16, fp);
		fclose (fp);
		memset (buffer_2, 0x0, 64);
		//strncmp (buffer_2, buffer_, 2);
		//return_value = atoi (buffer_2);
		
		version_flag = 1;
		
		pCANMOB_message0[0].can_msg->id = 0xFFFF;
		pCANMOB_message0[0].can_msg->data.u64 = 0x44ffffffffffffff;
		//can_example_prepare_data_to_send2(DOWNGRADE);
		gFwState.fwVerStored = 1;
			
		can_example_prepare_data_to_send();
		
		return_value = 10;
		
		//for (unsigned p = 0; p < 100000000; p ++);
		//sleep(1);
		delay_ms(1000);
		//gui_init(FCPU_HZ, FHSB_HZ, FPBB_HZ, FPBA_HZ);
		gui_clear_view();
		break;
		
	case 0x6FE:	/* binary-content data-1 */
		/* processing binary data-1 in this area */
		fp = fopen ("recv_file","wb");
		fwrite(&input_data, 1, 16, fp);
		fclose(fp);

		fp = fopen ("recv_file", "rb");
		fread (buffer_, 1, 16, fp);
		fclose (fp);
		memset (buffer_2, 0x0, 64);
		//strncmp (buffer_2, buffer_, 2);
		//return_value = atoi (buffer_2);
		
		version_flag = 2;
		

		pCANMOB_message0[0].can_msg->id = 0xFFFF;
		pCANMOB_message0[0].can_msg->data.u64 = 0x55ffffffffffffff;
		//can_example_prepare_data_to_send2(UPDATE);
		gFwState.fwVerStored = 2;
		can_example_prepare_data_to_send();
		
		return_value = 10;
		
		//for (unsigned p = 0; p < 100000000; p ++);
		//sleep(1);
		delay_ms(1000);
		//gui_init(FCPU_HZ, FHSB_HZ, FPBB_HZ, FPBA_HZ);
		gui_clear_view();
		
		break;
		
	default:
		return_value = 255; /* No such a(an) request of order  */
		break;
		
	}
	
	return return_value;
}