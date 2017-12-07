/*****************************************************

Module  : Connection Manager
Version : 2.0
File    : con_mgr.c
Date    : 11/07/2006
Author  : Gary Warren
Company : SMSC - AIS
Comments:

Description:
	State Machines to manage the sync connections
	by creating sockets & connections.  This state
    machine runs on its own & must be called by
    the main loop periodically in order to run.
    It is controlled by other routines changing the
    state to get the desired action done.

Modifications:

03/08/2010  GW      Updated for Eval110 board

12/09/2009  GW      Added SCError handling for source drop

11/07/2006	GW		Created from non-NS Eval Board code

*****************************************************/

#include <stdint.h>
#include <asf.h>
#include "defines.h"
#include "mostns.h"

#include "i2c_lld.h"
#include "timers.h"
#include "board_utils.h"
#include "codec.h"
#include "con_mgr.h"
#include "fblock_pb_amp.h"
#include "fblock_pb_auxin.h"
#include "fblock_pv_amp.h"
#include "fblock_pv_auxin.h"
#include "board_utils.h"


#define TEMP_POLLING_TIME   60000 // normal polling time 60 seconds
//#define TEMP_POLLING_TIME   30000 // normal polling time 30 seconds
#define FAST_TEMP_POLLING_TIME  5000   // fast polling time 5 seconds

enum CONNECTION_STATE connection_state; // current state of connection manager
//CONNECTION ADC_connection, DAC_connection; // connection parameters for ADC and DAC

DEVICE_STATE MyNode; // global info about our device on the network
Type_ScmCmd ScmCmd;

word scm_buf[32];

TIMER_T TemperatureTimer;
TIMER_T ScmCmdTimer;                                            // timeout for SCM commands

extern CONNECTION DAC_connection;
extern CONNECTION ADC_connection;

/*
 *  FUNCTION: 		ConmgrInit
 *
 *  PARAMETERS:		none
 *
 *  DESCRIPTION:	Initializes all the parameters for the ADC and DAC connections,
 *					after reset, nothing is connected.
 *
 *  RETURNS:		nothing
 *
 */
void ConmgrInit(void)
{
    connection_state = CON_IDLE; // connection manager initial state
    MyNode.StreamingPort = FALSE; // streaming port not open
    MyNode.MediaLBPort = FALSE;  // MediaLB not open yet

	/* initializing codecs(CS42448) assumes that RMCK is already running - i.e. set in config string */
	CODEC_setDefaultValues();
    /* initializing codecs assumes that RMCK is already running - i.e. set in config string */
//    DacInitFast(); // init Maxim 9850 DAC

    /* initialize fblocks */
    Init_Amp(); // initialize amplifier parameters - incl gain/mute
    Init_Auxin(); // initialize aux in parameters

    InitializeTimer(&TemperatureTimer);
    InitializeTimer(&ScmCmdTimer);

}


/*
 *  FUNCTION:       ConmgrGetMutex
 *
 *  PARAMETERS:     timeout - how long the mutex is valid in milliseconds
 *                  fn_ptr - routine to call if timeout expires before Mutex is returned
 *
 *  DESCRIPTION:    When a routine needs to call an INIC SocketConnectionManager (SCM) function
 *                  (or a series of SCM functions) it must call this routine first to get a
 *                  mutex for the SCM.  If SCM is already busy with another request, the routine
 *                  will return false.  If not busy, a TRUE is returned and the function may call
 *                  the SCM function.  When the SCM function returns with its callback, the calling
 *                  routine must then return the Mutex.  A timeout specifies an upper bound on how long the calling
 *                  routine expects the SCM to take.  If the timeout expires, it is assumed that the
 *                  SCM had a problem and timed out - did not respond. A callback specified by  the calling function
 *                  is done to indicate the timeout.
 *
 *  RETURNS:        TRUE - mutex granted / FALSE mutex denied, SCM is already busy
 *
 */
uint8_t ConmgrGetMutex(uint16_t timeout, Type_ScmTimeout_fptr Timeout_fn)
{
    if(!ScmCmd.apiLocked)
    {
        StartTimer(&ScmCmdTimer, timeout, TIMER_ONE_SHOT);  // start the timeout timer
        ScmCmd.apiLocked = TRUE;
        ScmCmd.ScmTimeout_fptr = Timeout_fn;
        connection_state = SCM_PENDING; // INIC's connection manager is busy
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
}

/*
 *  FUNCTION:       ConmgrReturnMutex
 *
 *  PARAMETERS:     none
 *
 *  DESCRIPTION:    When a routine has checked out the SCM Mutex and is done (SCM has returned)
 *                  this routine is called to return the Mutex.  If this routine is not called before
 *                  the timeout period, the timeout error callback is called
 *                  is done to indicate the timeout.
 *
 *  RETURNS:        nothing
 *
 */
void ConmgrReturnMutex(void)
{
    StopTimer(&ScmCmdTimer); // command sequence did not time out
    ScmCmd.apiLocked = FALSE; // open up for new commands
    ScmCmd.ScmTimeout_fptr = NULL;
    connection_state = CON_IDLE; // done - not waiting on anything now
}

/*
 *  FUNCTION: 		kill_net_connections
 *
 *  PARAMETERS:		none
 *
 *  DESCRIPTION:	Sets all network side connection parameters for ADC and DAC to "not connected".
 *					when network dies, these connections die automatically - flag that they are invalid
 *                  Also used at startup to flag all network connections are "not connected"
 *
 *  RETURNS:		nothing
 *
 */
void kill_net_connections(void)
{
    Amp_KillConnections();
    Auxin_KillConnections();
}


/*
 *  FUNCTION: 		InitPorts
 *
 *  PARAMETERS:		none
 *
 *  DESCRIPTION:	Called by main routine once NetServices has finished its startup.
 *					Starts the connection manager in OPEN_PORT state which will open the streaming port
 *                  and the create INIC side sockets for the ADC and DAC.  The network does not need to be running
 *                  and these sockets can remain in existance forever.
 *
 *  RETURNS:		nothing
 *
 */
void InitPorts (void)
{
	uint8_t result;

	if (ConmgrGetMutex(200, InitPorts_Timeout))
	{
		result = OpenSerialPort();  // starts Open Port process
		if (ERR_NO != result)
		{
			ConmgrReturnMutex();  //we're quitting, return mutex, stop timeout timer
			LOG_ERR("Error starting Open Port process: result = %02X\n", result);
			RED_LED_ON();
		}
	}
	else
	{
		LOG_ERR("Could not open ports because Connection Manager is busy\n");
	}

	if (TIMER_IDLE == TemperatureTimer.flag) // sanity check - don't start another timer if its already running
	{
		LOG_NOR("Starting Temperature polling timer\n");
		// *netbugger* StartTimer(&TemperatureTimer, TEMP_POLLING_TIME, TIMER_PERIODIC);
	}
}

void InitPorts_Timeout (void)
{
    LOG_ERR("Timed out during Open Port process\n");
}

/*
 *  FUNCTION: 		connection_manager
 *
 *  PARAMETERS:		none
 *
 *  DESCRIPTION:    Only checking a couple of timers. The connect / disconnect processes
 *                  are now handled entirely in the fblocks.
 *
 *  RETURNS:		nothing
 *
 */
void connection_manager(void)
{
	switch (connection_state)
	{
		case CON_IDLE:
		{
			if (TIMER_EXPIRED == TemperatureTimer.flag) // see if temperature timer has expired
			{
				TemperatureTimer.flag = TIMER_RUNNING; // if so, reset flag (periodic timer)
			//	if (FAST_TEMP_UPDATE_ENABLED())        // DIP Sw 7 controls how fast we poll for temperature
			//	{
			//		TmrNewTime(TemperatureTimer.handle, FAST_TEMP_POLLING_TIME/TIMER_TICK);
			//	}
			//	else
			//	{
					TmrNewTime(TemperatureTimer.handle, TEMP_POLLING_TIME/TIMER_TICK);
			//	}
				Amp_UpdateTemp(); // call FBlock Amp function to do the work
			}
			break; // just waiting for someone to request a SCM action
		}

		case SCM_PENDING:
		{
			if (TIMER_EXPIRED == ScmCmdTimer.flag) // check if command timer has expired
			{
				StopTimer(&ScmCmdTimer); // kill the timer
				LOG_ERR("Error: Pending Socket Connection Manager operation has timed out - retry...\n");
				ScmCmd.apiLocked = FALSE; // open up for new commands
				RED_LED_ON(); // turn on error light
				if(NULL != ScmCmd.ScmTimeout_fptr)
				{
					ScmCmd.ScmTimeout_fptr();  // call the timeout callback
				}
				else
				{
					LOG_ERR("Tried to call SCM Timeout function, but pointer was null\n");
				}
				connection_state = CON_IDLE; // done - not waiting on anything now
			}
			break;
		}
	}
}




/* ----------------------------------------------Open Port -------------------------------------------------------------------*/


/*******************************************************************************
 * function: OpenSerialPort
 *
 * descrip.: Open the streaming port for codec
 * params  : none
 * returns : --
 * effects : NetServices triggers OpenSerialPort_CB when done
 *******************************************************************************/
uint8_t OpenSerialPort(void)
{
    TScmPortDesc PortDesc, *pPortDesc;
    uint8_t result;

    LOG_NOR("Opening Streaming Port...\n");

    pPortDesc = &PortDesc;
    pPortDesc->port_id = SCM_PORT_ID_STREAM; // Open Streaming Port
    pPortDesc->config.streaming.clock_drive_mode = SCM_PORT_CFG_STREAM_SCKFSY_INIC; // INIC is source of SCK and FSY
    pPortDesc->config.streaming.port_mode = SCM_PORT_CFG_STREAM_IN_OUT; // Full Streaming Mode
    pPortDesc->config.streaming.data_format = DELAY_64FS_16BIT; // Delayed bit 64Fs 16 bit I2S format
    result = ScmOpenPort(pPortDesc, OpenSerialPort_CB);
    return (result);

}

/*******************************************************************************
 * function: OpenSerialPort_CB
 *
 * descrip.: MNS callback function
 * params  : result of open port process
 * returns : --
 * effects : if successful, moves state machine on to CREATE_ADC_INIC state
 *******************************************************************************/
void OpenSerialPort_CB(uint16_t cb_result)
{

    ConmgrReturnMutex();  //we're done (pass or fail), return mutex, stop timeout timer
    if (NSR_S_OK == cb_result)
    {
        LOG_NOR("Serial Streaming Port Opened OK\n");
        MyNode.StreamingPort = TRUE;
        RED_LED_OFF(); // turn off error light if it was on
    }
    else
    {
        LOG_ERR("Error opening serial streaming port: Error Code = %04X\n", cb_result);
        
        MyNode.StreamingPort = FALSE; // signal port is not open (should have FALSE already anyway)
        RED_LED_ON(); // turn on error light
    }
}


/*-------------------------------------------------------------------------------------------------------------------*/
/*
 *  FUNCTION:     SCError_CB
 *  PARAMETERS:      ConnectionHandle - the handle of the connection that has caused an error
 *  DESCRIPTION:  Gets called from INIC FBlock shadow when INIC sends an SCError status message
 *                  If this handle matches our ADC connection handle, then destroy and re-allocate
 *                  out current information
 *  RETURNS:      nothing
 */
/*-------------------------------------------------------------------------------------------------------------------*/
void SCError_CB(uint8_t handle)
{
	uint8_t result;

	if (handle == DAC_connection.ConnectionHandle) // stereo source we were listening to went away
	{
		// then this connection had an error - kill sockets & notify source
		if (DAC_connection.ConnectionLabel != INVALID_LABEL) // sanity check, making sure we had a valid connection before error
		{
			LOG_ERR("Dis-connecting stereo sink connection\n");
			DAC_connection.LocalCommand = TRUE; // destroy own sockets w/o being commanded, no response
			if (ConmgrGetMutex(300, SCError_Timeout))
			{
				result = DAC_DisConnectSockets(); // starts disconnect process
				if (result != ERR_NO)
				{
					ConmgrReturnMutex();  //we're quitting, return mutex, stop timeout timer
					LOG_ERR("Error starting ADC disconnect sockets process following SCError: result = %02X\n", result);
					
				}
			}
		}
	}
	else if (handle == ADC_connection.ConnectionHandle)
	{
		// then this connection had an error - destroy and rebuild - just like changing channels
		if (ADC_connection.ConnectionLabel != INVALID_LABEL) // making sure we had a valid connection before error
		{
			LOG_ERR("Disconnec and de-allocate stereo source (AuxIn)\n");
			ADC_connection.LocalCommand = TRUE; // flag that we don't want to send a response
			if (ConmgrGetMutex(300, SCError_Timeout))
			{
				result = ADC_DisConnectSockets(); // starts deallocation process
				if (result != ERR_NO)
				{
					ConmgrReturnMutex();  //we're quitting, return mutex, stop timeout timer
					LOG_ERR("Error starting ADC disconnect sockets process following SCError: result = %02X\n", result);
				}
			}
		}
	}

	else
	{
		LOG_ERR("Handle reported by SCError was not a valid connection handle for this node\n");
	}
}


void SCError_Timeout(void)
{
    LOG_ERR("Timed out while trying to automatically disconnect /destroy sockets after SCError\n");
}







