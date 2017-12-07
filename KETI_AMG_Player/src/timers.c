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

Module  : Atmel Timers
Version : 2.0
File    : timers.c
Date    : 07/11/2006
Author  : Gary Warren
Company : SMSC - AIS
Comments:



Description:
Initialization and interrupt routines for timers
Only Timer2 is used now for a 1 ms tick timer.
Other timers set to	default settings.
Module has a generic set of timer
routines for application code.


Modifications:


*****************************************************/

/*------------------------------------------------------------------------------
Include Files
------------------------------------------------------------------------------*/

#include <asf.h>
#include "defines.h"
#include "timers.h"

/*------------------------------------------------------------------------------
Local definitions
------------------------------------------------------------------------------*/
//#define DEBUG_TIMERS                                    // if defined printes when timers are created/destroyed

#define TIMER_CLOCK 	(F_CPU/64l)						// always use 64 for prescaler
#define TIMER_FREQ		1000							// want 1 msec ticks or 1kHz
#define TICK_COUNT 		TIMER_CLOCK/TIMER_FREQ


/*------------------------------------------------------------------------------
Global Variables
------------------------------------------------------------------------------*/
static volatile u32_t MasterTick;
static volatile uint8_t TimerTick;
static volatile uint8_t TimerTickDone;

struct TIMER TmrTable[MAX_TIMERS];
uint8_t NumTimersInUse;
uint8_t MaxTimersUsed;

/*------------------------------------------------------------------------------
Local Function Prototypes
------------------------------------------------------------------------------*/
void TmrTableInit( void );

__attribute__((__interrupt__))
void tc_irq(void)
{
	MasterTick++;
	TimerTick--;
	if (TimerTick == 0)
	{
		TimerTick = TIMER_TICK;                     // re-initialize ticker
		TimerTickDone = TRUE;
	}

	// clear the interrupt flag
	tc_read_sr(AMG_PLAYER_TC, AMG_PLAYER_TC_CHANNEL);
}


/*-------------------------------------------------------------------------------------------------------------------*/
/*
*  FUNCTION: 		TmrInit
*  PARAMETERS:		None
*  DESCRIPTION:	Sets up Timer2 for 1ms interrupts.
*					All other timers are turned off
*                  Initializes the timer table
*  RETURNS:		Nothing
*/
/*-------------------------------------------------------------------------------------------------------------------*/

/* Change RTC -> TC on AT32UC3C0512C by netbugger */

#if 0
void TmrInit (void) 
{

	// Register the RTC interrupt handler to the interrupt controller.
	INTC_register_interrupt(&rtc_irq, AVR32_RTC_IRQ, AVR32_INTC_INT0);

	// Initialize the RTC for 1ms tick

	// fRTC = 2^(-(PSEL+2)) * 32768
	// PSEL = 4 -> fRTC = 1024 -> 0.977us, so we'll get about 58.6s actual
	//  "seconds" per minute
	//
	if (!rtc_init(&AVR32_RTC, RTC_OSC_32KHZ, 4))
	{
		LOG_DBG("Error initializing the RTC\r\n");
		SYSTEM_HALT();
	}
	// Set top value to 0 to generate an interrupt every RTC tick */
	rtc_set_top_value(&AVR32_RTC, 0);
	// Enable the interrupts
	rtc_enable_interrupt(&AVR32_RTC);
	// Enable the RTC
	rtc_enable(&AVR32_RTC);

	TmrTableInit();                                 // init the timer blocks
	MasterTick = 0;                                 // Starting time now
	TimerTick = TIMER_TICK;                         // init count down timer for 10ms ticks
	TimerTickDone = FALSE;
}
#else
void TmrInit (void)
{

	
	// Options for waveform generation.
	static const tc_waveform_opt_t waveform_opt = {
		// Channel selection.
		.channel  = AMG_PLAYER_TC_CHANNEL,
		// Software trigger effect on TIOB.
		.bswtrg   = TC_EVT_EFFECT_NOOP,
		// External event effect on TIOB.
		.beevt    = TC_EVT_EFFECT_NOOP,
		// RC compare effect on TIOB.
		.bcpc     = TC_EVT_EFFECT_NOOP,
		// RB compare effect on TIOB.
		.bcpb     = TC_EVT_EFFECT_NOOP,
		// Software trigger effect on TIOA.
		.aswtrg   = TC_EVT_EFFECT_NOOP,
		// External event effect on TIOA.
		.aeevt    = TC_EVT_EFFECT_NOOP,
		// RC compare effect on TIOA.
		.acpc     = TC_EVT_EFFECT_NOOP,
		/*
		 * RA compare effect on TIOA.
		 * (other possibilities are none, set and clear).
		 */
		.acpa     = TC_EVT_EFFECT_NOOP,
		/*
		 * Waveform selection: Up mode with automatic trigger(reset)
		 * on RC compare.
		 */
		.wavsel   = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER,
		// External event trigger enable.
		.enetrg   = false,
		// External event selection.
		.eevt     = 0,
		// External event edge selection.
		.eevtedg  = TC_SEL_NO_EDGE,
		// Counter disable when RC compare.
		.cpcdis   = false,
		// Counter clock stopped with RC compare.
		.cpcstop  = false,
		// Burst signal selection.
		.burst    = false,
		// Clock inversion.
		.clki     = false,
		// Internal source clock 3, connected to fPBA / 8.
		.tcclks   = TC_CLOCK_SOURCE_TC3
	};
	
	// Options for enabling TC interrupts
	static const tc_interrupt_t tc_interrupt = {
		.etrgs = 0,
		.ldrbs = 0,
		.ldras = 0,
		.cpcs  = 1, // Enable interrupt on RC compare alone
		.cpbs  = 0,
		.cpas  = 0,
		.lovrs = 0,
		.covfs = 0
	};
	
	// Register the RTC interrupt handler to the interrupt controller.
	INTC_register_interrupt(&tc_irq, AMG_PLAYER_TC_IRQ, AMG_PLAYER_TC_IRQ_PRIORITY);
	
	TmrTableInit();                                 // init the timer blocks
	MasterTick = 0;                                 // Starting time now
	TimerTick = TIMER_TICK;                         // init count down timer for 10ms ticks
	TimerTickDone = FALSE;

	// Initialize the timer/counter.
	tc_init_waveform(AMG_PLAYER_TC, &waveform_opt);
	
	/*
	 * Set the compare triggers.
	 * We configure it to count every 1 milliseconds.
	 * We want: (1 / (fPBA / 8)) * RC = 1 ms, hence RC = (fPBA / 8) / 1000
	 * to get an interrupt every 10 ms.
	 */
	tc_write_rc(AMG_PLAYER_TC, AMG_PLAYER_TC_CHANNEL, (sysclk_get_pba_hz() / 8 / 1000));
	// configure the timer interrupt
	tc_configure_interrupts(AMG_PLAYER_TC, AMG_PLAYER_TC_CHANNEL, &tc_interrupt);
	// Start the timer/counter.
	tc_start(AMG_PLAYER_TC, AMG_PLAYER_TC_CHANNEL);	
}
#endif


/*-------------------------------------------------------------------------------------------------------------------*/
/*
*  FUNCTION: 		TmrGetMasterTick_Long
*  PARAMETERS:		None
*  DESCRIPTION:	Returns the current value of the full 32 bit MasterTick
*                  MasterTick resolution is 1ms
*  RETURNS:		unsigned long MasterTick
*/
/*-------------------------------------------------------------------------------------------------------------------*/
u32_t TmrGetMasterTick_Long (void)
{
	return(MasterTick);
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
*  FUNCTION: 		TmrGetMasterTick_Word
*  PARAMETERS:		None
*  DESCRIPTION:	Returns the low order 16 bits of current value of MasterTick
*  RETURNS:		unsigned int MasterTick
*/
/*-------------------------------------------------------------------------------------------------------------------*/
uint16_t TmrGetMasterTick_Word (void)
{
	return( (uint16_t) MasterTick);
}


/******************************************************************************
************************ Multiple Timer Utility *******************************

The code below implements a timer block that can have multple countdown
timers.  The intended use is for timing inside of state machines - when
you need to wait some time before going on to the next state.
Currently set to have up to 10 timers (MAX_TIMERS) each with 10 ms
ticks (TIMER_TICK).  Each timer is a WORD, so max time is 655 seconds or more
than 10 minutes.
To use a timer - first get a handle - TmrGetHandle().  If
some handle other than 0xFF (NO_SLOTS) is returned, then a free timer is
available to use.  To use a timer, call TmrStartTimer with the handle,
the number of ticks, a Mode flag, and a pointer to a flag byte.  The Mode flag
can be either ONE_SHOT_TIMER or PERIODIC_TIMER.  The flag byte is a user
variable that should be set to 0 before calling TmrStartTimer - when the
timer expires, the flag byte will be set to 0xFF by the timer routines.  User
code should be checking the state of the flag to see when the timer expires.
If the Mode flag was PERIODIC timer, then the timer is started over with
the same time value.  Users code must recognize and clear the flag byte before
it times out again.  If Mode flag was ONE_SHOT_TIMER, then the timer slot is
cleared and becomes available again.
******************************************************************************/


/*-------------------------------------------------------------------------------------------------------------------*/
/*
*  FUNCTION: 		TmrTableInit
*  PARAMETERS:		None
*  DESCRIPTION:	Sets all Timer slots to "Available" i.e. cleared
*  RETURNS:		Nothing
*/
/*-------------------------------------------------------------------------------------------------------------------*/
void TmrTableInit( void )
{
	uint8_t i;

	NumTimersInUse = MAX_TIMERS;
	MaxTimersUsed = 0;
	for (i=0; i<MAX_TIMERS ; i++ )
	{
		TmrKill(i);                     // clears timer slot and decrements NumTimersInUse
	}
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
*  FUNCTION: 		TmrGetHandle
*  PARAMETERS:		None
*  DESCRIPTION:	Searches through timer table looking for an
*                  empty slot.  Returns the index of the first
*                  available slot.
*  RETURNS:		Handle = index of first empty slot in timer table
*                  or 0xFF (NO_SLOTS) if all are busy
*/
/*-------------------------------------------------------------------------------------------------------------------*/
uint8_t TmrGetHandle ( void )
{
	uint8_t i;
	struct TIMER *ptmr;
	uint8_t timer_handle;

	ptmr = &TmrTable[0];
	timer_handle = NO_SLOTS;                        // assume we're all full

	for (i=0 ; i<MAX_TIMERS; i++)
	{
		if (ptmr->TmrStatus == TIMER_CLEAR)
		break;                                  // (make sure your compiler does not inc i when breaking out of loop!)
		ptmr++;                                     // next timer slot
	}

	if (i < MAX_TIMERS)                             // found an empty slot before the end?
	{
		timer_handle = i;                           // timer slot index is handle
		NumTimersInUse++;
		if (NumTimersInUse > MaxTimersUsed)
		MaxTimersUsed++;

		#ifdef DEBUG_TIMERS
		DEBUG_PRINT_TIME(TmrGetMasterTick_Long());              // start with time stamp
		DEBUG_PRINT_STRING("Timer # ");
		DEBUG_PRINT_DECIMAL(i, 3);
		DEBUG_PRINT_STRING(" allocated, ");
		DEBUG_PRINT_DECIMAL(NumTimersInUse, 3);
		DEBUG_PRINT_STRING(" timers currently in use\n");
		#endif
	}
	else
	LOG_ERR("No Timer Slots available\n");

	return(timer_handle);
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
*  FUNCTION: 		TmrStartTimer
*  PARAMETERS:		tmr = timer handle from TmrGetHandle
*                  ticks = number of ticks to count ( default = 10ms ticks)
*                  mode = ONE_SHOT_TIMER or PERIODIC_TIMER
*                  tmr_flag = pointer to byte which will be set to FF when timer expires
*  DESCRIPTION:	Initializes and starts a particular timer.
*  RETURNS:		Nothing
*/
/*-------------------------------------------------------------------------------------------------------------------*/
void TmrStartTimer (uint8_t tmr, uint16_t ticks, uint8_t mode, uint8_t *tmr_flag)
{
	struct TIMER *ptmr;

	if (tmr < MAX_TIMERS)
	{
		ptmr = &TmrTable[tmr];
		ptmr->TmrCount = ticks;
		ptmr->TmrInit = ticks;
		ptmr->TmrStatus = (TIMER_ACTIVE | TIMER_ENABLE | mode);
		ptmr->TmrFlag = tmr_flag;
		*tmr_flag = TIMER_RUNNING;                  // user should do this before calling - MUST do it for periodic interrupts
	}
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
*  FUNCTION: 		TmrNewTime
*  PARAMETERS:		tmr = timer handle from TmrGetHandle
*                  ticks = new tick interval to use
*  DESCRIPTION:	Changes the timout of a running timer
*                  Used to change the rate of a periodic timer after its been going
*  RETURNS:		Nothing
*/
/*-------------------------------------------------------------------------------------------------------------------*/
void TmrNewTime (uint8_t tmr, uint16_t ticks)
{
	struct TIMER *ptmr;

	if (tmr < MAX_TIMERS)                           // sanity check on parameter
	{
		ptmr = &TmrTable[tmr];                      // pointer to timer slot
		if (ptmr->TmrStatus && TIMER_ACTIVE)        // make sure timer is in use
		{
			ptmr->TmrInit = ticks;                  // sets new init value
			ptmr->TmrCount = ticks;                 // AND essentially restarts time over at new value
		}
	}
}


/*-------------------------------------------------------------------------------------------------------------------*/
/*
*  FUNCTION: 		TmrReStartTimer
*  PARAMETERS:		tmr = timer handle from TmrGetHandle
*  DESCRIPTION:	Causes a running timer to start over with its initial count
*                  Used to extend the time of a timer
*  RETURNS:		Nothing
*/
/*-------------------------------------------------------------------------------------------------------------------*/
void TmrReStartTimer (uint8_t tmr)
{
	struct TIMER *ptmr;

	if (tmr < MAX_TIMERS)                           // sanity check on parameter
	{
		ptmr = &TmrTable[tmr];                      // pointer to timer slot
		if (ptmr->TmrStatus && TIMER_ACTIVE)        // make sure timer is in use
		{
			ptmr->TmrCount = ptmr->TmrInit;         // start time over at initial value
		}
	}
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
*  FUNCTION: 		TmrPause
*  PARAMETERS:		tmr = timer handle for a particular timers
*  DESCRIPTION:	Causes the timer count to stop decrementing.  The
*                  timer is suspended
*  RETURNS:		Nothing
*/
/*-------------------------------------------------------------------------------------------------------------------*/
void TmrPause (uint8_t tmr)
{
	struct TIMER *ptmr;

	if (tmr < MAX_TIMERS)
	{
		ptmr = &TmrTable[tmr];
		if (ptmr->TmrStatus && TIMER_ACTIVE)        // make sure timer is in use
		{
			ptmr->TmrStatus &= !TIMER_ENABLE;
		}
	}
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
*  FUNCTION: 		TmrResume
*  PARAMETERS:		tmr = timer handle for a particular timer
*  DESCRIPTION:	Restarts a suspended timer.  No effect if timer was not
*                  suspended or if slot is empty
*  RETURNS:		Nothing
*/
/*-------------------------------------------------------------------------------------------------------------------*/
void TmrResume (uint8_t tmr)
{
	struct TIMER *ptmr;

	if (tmr < MAX_TIMERS)
	{
		ptmr = &TmrTable[tmr];
		if (ptmr->TmrStatus && TIMER_ACTIVE)        // make sure timer is in use
		{
			ptmr->TmrStatus |= TIMER_ENABLE;
		}
	}
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
*  FUNCTION: 		TmrKill
*  PARAMETERS:		tmr = timer handle for a particular timer
*  DESCRIPTION:	Clears a particular timer slot - making it available again
*  RETURNS:		Nothing
*/
/*-------------------------------------------------------------------------------------------------------------------*/
void TmrKill (uint8_t tmr)
{
	struct TIMER *ptmr;

	if (tmr < MAX_TIMERS)
	{
		ptmr = &TmrTable[tmr];
		ptmr->TmrStatus = TIMER_CLEAR;
		ptmr->TmrCount = 0;
		ptmr->TmrInit = 0;
		ptmr->TmrFlag = NULL;
		if (NumTimersInUse > 0)
		{
			NumTimersInUse--;
			#ifdef DEBUG_TIMERS
			DEBUG_PRINT_TIME(TmrGetMasterTick_Long());              // start with time stamp
			DEBUG_PRINT_STRING ("Released Timer # ");
			DEBUG_PRINT_DECIMAL( tmr, 3);
			DEBUG_PRINT_STRING (". Number of timers currently in use is ");
			DEBUG_PRINT_DECIMAL(NumTimersInUse, 3);
			DEBUG_PRINT_CRLF;
			#endif
		}
	}
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
*  FUNCTION: 		TmrTicksLeft
*  PARAMETERS:		tmr = timer handle for a particular timer
*  DESCRIPTION:	Reports how many ticks are left for a particular timer
*  RETURNS:		Number of ticks left before timeout
*/
/*-------------------------------------------------------------------------------------------------------------------*/
uint16_t TmrTicksLeft (uint8_t tmr)
{
	struct TIMER *ptmr;
	uint16_t time_left;

	time_left = 0xFFFF;                             // invalid timer count
	if (tmr < MAX_TIMERS)
	{
		ptmr = &TmrTable[tmr];
		time_left = ptmr->TmrCount;
	}
	return(time_left);

}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
*  FUNCTION: 		TmrTask
*  PARAMETERS:		None
*  DESCRIPTION:	Main timer task called by timer interrupt routine.
*                  Searches through timer table, decrements all enabled
*                  timers, and if any time out, sets the flag byte.  If
*                  a timer has timed out and its mode is ONE_SHOT_TIMER, then
*                  TmrKill is called to free up that slot.  If the mode was
*                  TIMER_PERIODIC, then the timer is started over
*  RETURNS:		Nothing
*/
/*-------------------------------------------------------------------------------------------------------------------*/
void TmrTask (void)
{
	uint8_t i;
	struct TIMER *ptmr;

	if (TimerTickDone)												// check flag to see if its time to update timers
	{
		TimerTickDone = FALSE;										// reset flag for next tick
		ptmr = &TmrTable[0];
		for (i=0; i<MAX_TIMERS ; i++ )
		{
			if (ptmr->TmrStatus & TIMER_ACTIVE)                     // if this timer slot is used
			{
				if (ptmr->TmrStatus & TIMER_ENABLE)				    // and if this timer slot is enabled
				{
					ptmr->TmrCount--;                               // decrement timer count
					if (ptmr->TmrCount == 0)                        // if its 0, then timer has expired
					{
						*ptmr->TmrFlag = TIMER_EXPIRED;             // set this timer's flag
						if (ptmr->TmrStatus & TIMER_PERIODIC)       // reset timer?
						{
							ptmr->TmrCount = ptmr->TmrInit;
						}
						else                                        // this was one shot timer
						{
							TmrKill(i);                             // release this timer slot
						}
					}
				}
			}
			ptmr++;                                                 // check next timer slot
		}
	}
}


void InitializeTimer(TIMER_T *tmr)
{
	tmr->flag = TIMER_IDLE;                         // timer is not currently running
	tmr->handle = NO_SLOTS;                         // invalid handle
}

uint8_t StartTimer(TIMER_T *tmr, uint16_t time, uint8_t t_mode)
{
	uint8_t result = TRUE;

	if (TIMER_IDLE == tmr->flag)                    // sanity check - don't start another timer if its already running
	{
		tmr->handle = TmrGetHandle();
		if (tmr->handle != NO_SLOTS)
		{
			tmr->flag = TIMER_RUNNING;              // init timer variable
			TmrStartTimer (tmr->handle, time/TIMER_TICK, t_mode, &tmr->flag);
		}
		else
		{
			result = FALSE;
			LOG_ERR("No timer slots available\n");
		}
	}
	else
	{
		result = FALSE;
		LOG_ERR("Timer is already running\n");
	}

	return(result);
}

void StopTimer(TIMER_T *tmr)
{
	TmrKill(tmr->handle);                           // stop counting, get rid of timer of timer slot
	tmr->flag = TIMER_IDLE;                         // timer is not running
	tmr->handle = NO_SLOTS;                         // invalid handle
}

/*    Timer Test Code
Pop this in to main routine just after initialzing interrupts


uint8_t led_on_handle, led_on_flag;
uint8_t led_off_handle, led_off_flag;
uint8_t yellow_led_handle, yellow_led_flag, yellow_led_on;
uint8_t green_led_handle, green_led_flag, green_led_on;


led_on_flag = TIMER_EXPIRED;
yellow_led_handle = TmrGetHandle();
YELLOW_LED_ON
yellow_led_on = TRUE;
TmrStartTimer(yellow_led_handle, 250/TIMER_TICK, TIMER_PERIODIC, &yellow_led_flag);
green_led_handle = TmrGetHandle();
GREEN_LED_ON
green_led_on = TRUE;
TmrStartTimer(green_led_handle, 100/TIMER_TICK, TIMER_PERIODIC, &green_led_flag);
while (1)
{
if (led_on_flag == TIMER_EXPIRED)
{
RED_LED_ON();
led_on_flag = FALSE;
led_off_handle = TmrGetHandle();
TmrStartTimer(led_off_handle, 200/TIMER_TICK, TIMER_ONE_SHOT, &led_off_flag);
}

if (led_off_flag == TIMER_EXPIRED)
{
RED_LED_OFF();
led_off_flag = FALSE;
led_on_handle = TmrGetHandle();             //
TmrStartTimer(led_on_handle, 500/TIMER_TICK, TIMER_ONE_SHOT, &led_on_flag);
}

if (yellow_led_flag == TIMER_EXPIRED)
{
yellow_led_flag = TIMER_RUNNING;            // must reset flag for periodic timer
if (yellow_led_on)
{
YELLOW_LED_OFF
yellow_led_on = FALSE;
}
else
{
YELLOW_LED_ON
yellow_led_on = TRUE;
}
}

if (green_led_flag == TIMER_EXPIRED)
{
green_led_flag = TIMER_RUNNING;            // must reset flag for periodic timer
if (green_led_on)
{
GREEN_LED_OFF
green_led_on = FALSE;
}
else
{
GREEN_LED_ON
green_led_on = TRUE;
}
}
TmrTask();                              // run all timers
}

*/
