/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
#include <asf.h>
#include <stdint.h>
#include "defines.h"
#include "timers.h"
#include "con_mgr.h"
#include "i2c_driver.h"
#include "i2c_lld.h"
#include "board_utils.h"
#include "codec.h"
#include "main.h"

#include "mostns.h"
#include "ns_cb_l1.h"
#include "nbehc.h"
#include "mnstrace.h"

#include "fblock_pv_amp.h"
#include "fblock_pv_auxin.h"


/* ---------------------- add from here *////////////////// park
#include "compiler.h"
#include "board.h"
#include "power_clocks_lib.h"
#include "dsp.h"
#include "gui.h"
#include "controller.h"
#include "gpio.h"
#include "print_funcs.h"
#include "flashc.h"
#include "adcifa.h"
#include "twim.h"
#include "conf_at42qt1060.h"
#include "can_task.h"
#include "conf_demo.h"

A_ALIGNED dsp16_t signal1_buf[BUFFER_LENGTH];
A_ALIGNED dsp16_t signal4_buf[BUFFER_LENGTH];
extern unsigned char check_flag;
extern unsigned char check_flag_rear;
extern unsigned char version_flag;
extern unsigned counter_;
volatile U16 adc_current_conversion;
/*! \brief Initializes the MCU system clocks.
 */

volatile bool input_fft_view = false;
volatile bool output_fft_view = false;
volatile bool zoom_view = false;
volatile int32_t zoom_view_id;

enum state_master {
	STATE_IDLE,
	STATE_SOURCE1,
	STATE_OUTPUT1,
	STATE_OUTPUT2,
	STATE_OUTPUT3
};

enum state_function {
	STATE_FCT_IDLE,
	STATE_FCT_FUNCTION1,
	STATE_FCT_FUNCTION2,
	STATE_FCT_FUNCTION3,
	STATE_FCT_FUNCTION4,
	STATE_FCT_ZOOM
};

static enum state_master state = STATE_IDLE;
static enum state_function state_fct = STATE_FCT_IDLE;
static bool new_state_fct = false;

/*! \brief Global State Machine:
 *        - Function Idle
 *        - Function Zoom
 */
static bool state_machine_global(int source_id, enum state_function *state)
{
	switch (*state) {
		case STATE_FCT_IDLE:
			
			if (version_flag == 2)	{	//after update
				if (source_id == GUI_SOURCE1_ID) {
					if (new_state_fct) {
						gui_set_selection(GUI_SOURCE1_ID);
					}
				}
				else if (source_id == GUI_OUTPUT1_ID) {
					if (new_state_fct) {
						gui_set_selection(GUI_OUTPUT1_ID);
					}
				}
				else if (source_id == GUI_OUTPUT2_ID) {
					if (new_state_fct) {
						gui_set_selection(GUI_OUTPUT2_ID);
					}
				}
				else if (source_id == GUI_OUTPUT3_ID) {
					if (new_state_fct) {
						gui_set_selection(GUI_OUTPUT3_ID);
					}
				}
			}
			else
			{
				if (source_id == GUI_OUTPUT2_ID) {
					if (new_state_fct) {
						gui_set_selection(GUI_OUTPUT2_ID);
					}
				}
				else if (source_id == GUI_OUTPUT3_ID) {
					if (new_state_fct) {
						gui_set_selection(GUI_OUTPUT3_ID);
					}
				}
				
			} //else
					
			break;
		// Not Implemented
		case STATE_FCT_FUNCTION1:
			break;
		// Not Implemented
		case STATE_FCT_FUNCTION2:
			break;
		// Not Implemented
		case STATE_FCT_FUNCTION3:
			break;
		// Not Implemented
		case STATE_FCT_FUNCTION4:
			break;
		// Zoom
		case STATE_FCT_ZOOM:
			if (new_state_fct) {
				zoom_view = true;
//				printf("2");
				if (source_id == GUI_SOURCE1_ID)
					zoom_view_id = GUI_SOURCE1_ID;
				else if (source_id == GUI_OUTPUT1_ID)
					zoom_view_id = GUI_OUTPUT1_ID;
				else if (source_id == GUI_OUTPUT2_ID)
					zoom_view_id = GUI_OUTPUT2_ID;
				else if (source_id == GUI_OUTPUT3_ID)
					zoom_view_id = GUI_OUTPUT3_ID;
			}
			break;
	}
	return true;
}

/*! \brief Navigation State Machine:
 *        - STATE_SOURCE1, STATE_OUTPUT1, STATE_OUTPUT2, OUTPUT3
 *
 */
static void state_machine_task(void)
{
	//printf (".");
	// Set function state
	if (controller_key_fct5()) {
		
		state_fct = STATE_FCT_ZOOM;
		new_state_fct = true;
	}
	else if (controller_key_fct1()) {
		state_fct = STATE_FCT_IDLE;
		state = STATE_SOURCE1;
		new_state_fct = true;
	}
	else if (controller_key_fct2()) {
		state_fct = STATE_FCT_IDLE;
		state = STATE_OUTPUT1;
		new_state_fct = true;
	}
	else if (controller_key_fct3()) {
		state_fct = STATE_FCT_IDLE;
		state = STATE_OUTPUT2;
		new_state_fct = true;
	}
	else if (controller_key_fct4()) {
		state_fct = STATE_FCT_IDLE;
		state = STATE_OUTPUT3;
		new_state_fct = true;
	}
	// Clear Zoom state if on and a key is pressed
	if (zoom_view && !controller_key_fct5()) {
		zoom_view = false;
		gui_clear_view();
		new_state_fct = true;
		state_fct = STATE_FCT_IDLE;
	}

	switch (state) {
		case STATE_IDLE:
			break;
		case STATE_SOURCE1:
			if (!state_machine_global(GUI_SOURCE1_ID, &state_fct))
				return;
			break;
		case STATE_OUTPUT1:
			if (!state_machine_global(GUI_OUTPUT1_ID, &state_fct))
				return;
			break;
		case STATE_OUTPUT2:
			if (!state_machine_global(GUI_OUTPUT2_ID, &state_fct))
				return;
			break;
		case STATE_OUTPUT3:
			if (!state_machine_global(GUI_OUTPUT3_ID, &state_fct))
				return;
			break;
	}
	new_state_fct = false;
}


static const gpio_map_t USART_GPIO_MAP =
{
	{AVR32_USART0_TXD_0_1_PIN, AVR32_USART0_TXD_0_1_FUNCTION},
	{AVR32_USART0_RXD_0_1_PIN, AVR32_USART0_RXD_0_1_FUNCTION},
};

// USART options.
static const usart_options_t USART_OPTIONS =
{
	.baudrate     = 115200,
	.charlength   = 8,
	.paritytype   = USART_NO_PARITY,
	.stopbits     = USART_1_STOPBIT,
	.channelmode  = USART_NORMAL_CHMODE
};

fw_state_t gFwState;

const char CompanyName[] =      "KETI";
const char ProductName[] =      "AMG_Player";
const char ProductVersion[] =   "AVN_MECU V1.1.1";

uint8_t masterStartFlag = 0;
uint8_t initFlag = 0;
uint8_t MnsServiceRequest;

uint8_t tmp;
TNetServicesConfig *cfg_ptr;
TNetServicesConfig cfg;
TMostVersionInfo version_info;
TIMER_T mostCheckTimer;

int main (void)
{
	/* Clock Init */	
	sysclk_init();
	
	/* Delay Init */
	delay_init(FOSC0);
	
	/* Debug UART Init */
	gpio_enable_module(USART_GPIO_MAP, sizeof(USART_GPIO_MAP) / sizeof(USART_GPIO_MAP[0]));
	stdio_serial_init(&AVR32_USART0, &USART_OPTIONS);
	
	
	//GPIO Init
	init_board_gpio();
	
	// Insert application code here, after the board has been initialized.
	printf("AMG Player Started\n");
	
	// GUI, Controller and DSP process init
	gui_init(FCPU_HZ, FHSB_HZ, FPBB_HZ, FPBA_HZ);
	printf ("GUI INIT\n");
		
	MyNode.NetworkFreq = 48;
	
	/* INIC INIT */
	inic_HoldReset();
	delay_ms(10);
	inic_ReleaseReset();

	/* Interrupt related Functions  Init*/
	Disable_global_interrupt();
	INTC_init_interrupts();
	
	/* i2c Init */
	i2c_master_init();

	// Timer Init
	TmrInit();
	printf("Timer Init\n");

		
	//Controller Init
	controller_init(FCPU_HZ, FHSB_HZ, FPBA_HZ, FPBB_HZ);
	printf ("Controller INIT\n");
			
	// Initialize CAN Interface
	can_task_init();
	printf ("CAN INIT\n");
			
	// Enable global interrupts
	Enable_global_interrupt(); // time starts now
	/* Interrupt related Functions  Init End*/
	
	// give multichannel codec a reset after power up (release before initializing)
	codec_HoldReset();
	codec_EnableI2c();    // enable codec's I2C mux - will run I2C at 100kHz always - so mux can stay enabled.
	codec_activeI2c();
	delay_ms(5);    // takes a few ms for mux to switch
	ConmgrInit();
	//DacInit();

	MnsTraceInit();
	
	/* now start NetService */
	LOG_NOR("%u Starting NetServices...\n", TmrGetMasterTick_Long());
	cfg_ptr = GetNetServicesConfig();
	prepare_mns_config(cfg_ptr); // set up NetServices configuration
	InitNetServices(); // NetServices get INIC to 'attached' state
	LOG_NOR("%u InitNetServices has returned \n", TmrGetMasterTick_Long()); // See how long NS took to start up
	//PWROFF_UC_INACTIVE(); // now let INIC manage power
	

	InitializeTimer(&mostCheckTimer); // setup timer structure - nex_min_timeout call back runs this timer
	
	gFwState.fVolCur = AVN_DEFAULT_VOLUME;
	gFwState.fVolSet = AVN_DEFAULT_VOLUME;
	gFwState.rVolCur = AVN_DEFAULT_VOLUME;
	gFwState.rVolSet = AVN_DEFAULT_VOLUME;
	gFwState.fwVerCur = 1;
	gFwState.fwVerStored = 1;
	
	//check_flag = 10;
	//version_flag = 1;
	//counter_ = 0;

	
	
	/* ---------------------- insert to here *////////////////////////////
	
	
	
	/* Loop Phase */

	while (1) {
		
		/* ---------------------- insert from here *////////////////////////////
		// Call CAN Task for communication management
		can_task();
		// Call Gui Task for display update
		gui_task();
		// Call Controller Task for control Update
		controller_task();
		// Here add the other concurrent process
		state_machine_task();
		/* ---------------------- insert from here *////////////////////////////
		
				
		/* run NetServices - event driven */
		if (MnsServiceRequest == TRUE)
		{
			MnsServiceRequest = FALSE;
			MostService();
		}

		if (TIMER_EXPIRED == mostCheckTimer.flag) // see if time for a CheckTimers call
		{
			mostCheckTimer.flag = TIMER_IDLE; // timer has expired - not running now
			MostCheckTimers(); // call the NetServices function
		}

		check_for_port_message(); // always checking for INIC traffic, handles it if ready
		connection_manager(); // run connection manager state machine
		TmrTask(); // run all timers
		
		//MsgBlinkSM(); // LED blinker state machine
		//ErrBlinkSM(); // Error blinking state machine
		//DebounceSwitches(); // Pushbutton switch debounce & event generator

		// check for DAC events
		#if 0
		if (DAC_INT_ASSERTED())
		{
			DacGetStatus(); // get new status - that will clear Alert Bit.
			NtfPropertyChangedFkt(FBLOCK_AMP, 0, AMP_EVENTS); // trigger notification
		}
		#endif
				
		/* Check FW Update */
		if(gFwState.fwVerCur != gFwState.fwVerStored) {
			LOG_NOR("FIRMWARE Changed(%d -> %d)\n", gFwState.fwVerCur, gFwState.fwVerStored);
			switch(gFwState.fwVerStored) {
				case 1 :
					gFwState.fVolCur = AVN_DEFAULT_VOLUME;
					gFwState.fVolSet = AVN_DEFAULT_VOLUME;
					gFwState.rVolCur = 0;
					gFwState.rVolSet = 0;
					CODEC_setVolume(0, gFwState.fVolSet);
					CODEC_setVolume(1, gFwState.rVolSet);
					gFwState.fwVerCur = gFwState.fwVerStored;
					break;
					
				case 2:
					gFwState.fVolCur = AVN_DEFAULT_VOLUME;
					gFwState.fVolSet = AVN_DEFAULT_VOLUME;
					gFwState.rVolCur = AVN_DEFAULT_VOLUME;
					gFwState.rVolSet = AVN_DEFAULT_VOLUME;
					CODEC_setVolume(0, gFwState.fVolSet);
					CODEC_setVolume(1, gFwState.rVolSet);
					gFwState.fwVerCur = gFwState.fwVerStored;
					break;
				default :
					LOG_ERR("Invalid Firmware, ver=%d\n", gFwState.fwVerStored);
					gFwState.fwVerStored = gFwState.fwVerCur;
					break;		
			}
		}
		
		if(gFwState.fVolCur != gFwState.fVolSet) {
			CODEC_setVolume(0, gFwState.fVolSet);
			gFwState.fVolCur = gFwState.fVolSet;	
		}
		if(gFwState.rVolCur != gFwState.rVolSet) {
			CODEC_setVolume(1, gFwState.rVolSet);
			gFwState.rVolCur = gFwState.rVolSet;
		}
				
	}//while
}//main


/*
*  FUNCTION:       app_on_init_complete
*
*  PARAMETERS:     none
*
*  DESCRIPTION:    A callback from Most Net Services when the initialization is complete.
*                  No net services functions should be called before this routine executes.
*
*  RETURNS:        nothing
*
*/
void app_on_init_complete(void)
{
	LOG_NOR("%u Most NetServices initialization complete \n", TmrGetMasterTick_Long());              // See how long NS took to start up
	MostSelectClockOutput(RMCK_256);                        // start clock for codec - needed before talking to codec
	InitPorts();                                            // sets up Streaming port - network not needed
	//masterStartFlag = 1;
	initFlag = 1;
}
/*
*  FUNCTION:       app_go_net_on
*
*  PARAMETERS:     none
*
*  DESCRIPTION:    A callback from Most Net Services when the network has locked and is available
*                  No messages can be sent until this call is received
*
*  RETURNS:        nothing
*
*/
void app_go_net_on(void)
{
	uint8_t NodePos;

	LOG_NOR("Network has reached state NetOn! \n");
	MyNode.Net_State = NISTATE_NET_ON;
	MostGetVersionInfo(&version_info);
	LOG_NOR("Hardware Version is %02X\n", version_info.hw);
	LOG_NOR("Product Version is %d.%d.%d\n", version_info.fw[0], version_info.fw[1], version_info.fw[2]);
	
//	PwrStopPowerDownTimer();                                      // we're locked, stay powered up
	// Now set our InstID's to Node Position
	NodePos = MostGetNodePos();                                 // get our current node position
	NetBlock.pFBlockIDs.InstID[AMP_INDEX]   = NodePos;
	NetBlock.pFBlockIDs.InstID[AUXIN_INDEX] = NodePos;
	masterStartFlag = 0;
	GREEN_LED_ON();
}

/*
*  FUNCTION:       app_go_net_off
*
*  PARAMETERS:     none
*
*  DESCRIPTION:    A callback from Most Net Services when the network has unlocked and is not available
*
*  RETURNS:        nothing
*
*/
void app_go_net_off(void)
{
	LOG_NOR("Got Net Off event, killing network connections \n");
	MyNode.Net_State = NISTATE_NET_OFF;                     // global flag that other routines check
	kill_net_connections();                                 // reset all network side connections to "not connected" state
	ET_Go_Net_Off();                                        // Trigger ET's net off in case tester is running
	//    PwrStartPowerDownTimer();                                 // if we stay unlocked for 10 sec, power down
}


/*
*  FUNCTION:       app_start_check_timer
*
*  PARAMETERS:     timeout - 16 bit value, specifies how long to wait before calling MostCheckTimers
*
*  DESCRIPTION:    A callback from Most Net Services when the network has unlocked and is not available
*
*  RETURNS:        nothing
*
*/
void app_start_check_timer(uint16_t timeout)
{
	uint16_t ticks_left;

	if (TIMER_IDLE == mostCheckTimer.flag)                  // see if timer is already running
	{
		StartTimer(&mostCheckTimer, timeout, TIMER_ONE_SHOT);
	}
	else                                                    // timer is already running
	{
		ticks_left =  TmrTicksLeft(mostCheckTimer.handle);
		if (timeout < ticks_left)                           // wants service sooner than previously asked for
		{
			TmrNewTime(mostCheckTimer.handle, timeout);     // reset timer
		}
	}
}