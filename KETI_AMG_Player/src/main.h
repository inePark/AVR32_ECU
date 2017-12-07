/*******************************************************************************
 * SMSC Automotive Infotainment Systems Group
 *
 * (c)2002-2010 Standard Microsystems Corporation and its subsidiaries ("SMSC").
 * The copyright of the computer program(s) herein is the property of SMSC.
 * The program(s) may be used and copied only with written permission from SMSC
 * or in accordance with the terms and conditions stipulated in the
 * agreement under which the program(s) have been supplied.
 ******************************************************************************/

/*****************************************************

 Module  : Main module, header file
 Version : 1.0
 File    : main.h
 Date    : 03/10/2010
 Author  : Gary Warren
 Company : SMSC-AIS


 Modifications:

 03/10/2010  GW      Created

 *****************************************************/
#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>

/****************************************************************************
 Function definitions
 ****************************************************************************/
void app_on_init_complete(void);
void app_go_net_on(void);
void app_go_net_off(void);
void app_start_check_timer(uint16_t timeout);
//void SW1_ButtonEvent();
//void SW2_ButtonEvent();
//void SW3_ButtonEvent();



#endif /* MAIN_H_ */
