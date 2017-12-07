/*******************************************************************************
 * SMSC Automotive Infotainment Systems Group
 *
 * (c)2002-2007 Standard Microsystems Corporation and its subsidiaries ("SMSC").
 * The copyright of the computer program(s) herein is the property of SMSC.
 * The program(s) may be used and copied only with written permission from SMSC
 * or in accordance with the terms and conditions stipulated in the
 * agreement under which the program(s) have been supplied.
******************************************************************************/

/****************************************************************************
Module  : Timer setup
Version : 2.0
File    : timers.h
Date    : 07/12/2006
Author  : Gary Warren
Company : SMSC - AIS
Comments:


Modifications:

07/12/2006	GW		Created from AppWave

****************************************************************************/
#ifndef __TIMERS__
#define __TIMERS__
#include <stdint.h>

/****************************************************************************
  Global Definitions
****************************************************************************/
#define MAX_TIMERS      10          // number of timers active at one time
#define TIMER_TICK      10          // timer tick in msec

#define TIMER_CLEAR     0x00        // clears all flags - no timer in this slot
#define TIMER_ONE_SHOT  0x01        // timer goes away when it times out
#define TIMER_PERIODIC  0x02        // timer sets flag, then restarts its count
#define TIMER_ACTIVE    0x80        // timer is using this slot
#define TIMER_ENABLE    0x40        // enable timer for counting
#define NO_SLOTS        0xFF        // indicator that no empty timer slots are available

#define TIMER_IDLE      0x00        // timer flag value while not running
#define TIMER_RUNNING   0x01        // timer flag value while running
#define TIMER_EXPIRED   0x02        // timer flag value when timer has timed out
#ifndef NULL
#define NULL 	0x00
#endif

struct TIMER
{
    uint8_t TmrStatus;
    uint16_t TmrCount;
    uint16_t TmrInit;
    uint8_t *TmrFlag;
};

typedef struct Timer_t
{
	uint8_t handle;
	uint8_t flag;
} TIMER_T;

#define AMG_PLAYER_TC				(&AVR32_TC0)
#define AMG_PLAYER_TC_CHANNEL		0
#define AMG_PLAYER_TC_IRQ			AVR32_TC0_IRQ0
#define AMG_PLAYER_TC_IRQ_PRIORITY	AVR32_INTC_INT0

/****************************************************************************
  Function definitions
****************************************************************************/

void TmrInit (void);
u32_t TmrGetMasterTick_Long (void);
uint16_t TmrGetMasterTick_Word (void);

uint8_t TmrGetHandle (void);
void TmrStartTimer (uint8_t tmr, uint16_t ticks, uint8_t mode, uint8_t *tmr_flag);
void TmrReStartTimer (uint8_t tmr);
void TmrNewTime (uint8_t tmr, uint16_t ticks);
void TmrPause (uint8_t tmr);
void TmrResume (uint8_t tmr);
void TmrKill (uint8_t tmr);
uint16_t TmrTicksLeft (uint8_t tmr);
void TmrTask (void);

void InitializeTimer(TIMER_T *tmr);
uint8_t StartTimer(TIMER_T *tmr, uint16_t time, uint8_t t_mode);
void StopTimer(TIMER_T *tmr);


#endif
