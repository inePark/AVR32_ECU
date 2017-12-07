/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Core Application Framework (Layer I Callbacks)
File:           ns_cb_l1.c
Version:        2.0.3
Language:       C
Author(s):      S.Semmler
Date:           20.May.2005

FileGroup:      Layer I
Customer ID:    C058FE02020003.P.OSSGER.MODULES
FeatureCode:    FCR1.G1.0000.G2.0000.G7.0000
------------------------------------------------------------------------------

                (c) Copyright 1998-2005
                Oasis SiliconSystems AG
                All Rights Reserved

------------------------------------------------------------------------------



Modifications
~~~~~~~~~~~~~
Date            By      Description
12/09/2009      GW      Added SCError handling for source drop

11/17/2009      GW      Adapted for Eval92 and MSV3

01/10/2006		GW		Start adapting to AppWave project

==============================================================================
*/

#include <asf.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include "defines.h"
#include "mostns.h"
#include "mnstrace.h"

#include "board_utils.h"
#include "i2c_lld.h"

#include "timers.h"
#include "con_mgr.h"
//#include "pwr_mgr.h"
#include "main.h"
//#include "trace.h"
//#include "srv_l1.h"

/* -----------------------------------------------------------------------------
 * forward declarations of callback functions used by Layer I
 * -----------------------------------------------------------------------------
 */

/* kernel */
static void mns_service_request(void);
static void mns_on_init_complete(void);
static void mns_on_error(TMnsResult result);


static void mns_next_min_timeout(word timeout);


/* application message service wrapper */
#ifdef AMS_MIN
    static byte msg_tx_status(byte status, TMsgTx *msg_ptr);
    static byte msg_tx_filter(TMsgTx *msg_ptr);
    static byte msg_rx_complete(TMsgRx *msg_ptr);
    static void msg_retry_config_adjusted(TMsgRetryConfig *config_ptr);

    #ifdef AMS_TX_BYPASS_FILTER
    static bool msg_tx_bypass(TMsgTx *msg_ptr);
    #endif

    #ifdef MSG_RX_USER_PAYLOAD
    static word msg_get_payload_buf(TMsgRx *msg_ptr, word size);
    static void msg_free_payload_buf(TMsgRx *msg_ptr);
    static word msg_reallocate_payload_buf(TMsgRx *msg_ptr, word size);
    #endif

    #ifdef ENABLE_CFG_MSG_FILTER
    static void msg_tx_buffer_flushed(word num);
    static void msg_tx_msg_flushed(TMsgTx *msg_ptr);
    #endif
#endif

/* asynchronous data service wrapper */
#ifdef ADS_MIN
    static void data_tx_status(TDataTx *msg_ptr);
    static byte data_rx_complete(TDataRx *msg_ptr);
    static byte data_rx_filter(TDataRx *msg_ptr);
    #ifdef DATA_TX_USER_PAYLOAD
    static void data_tx_msg_flushed(TDataTx *msg_ptr);
    #endif
#endif

/* virtual MOST supervisor */
#ifdef VMSV_MIN
    static void msval_state_changed(byte state);
    static void msval_error(byte error, byte *info_ptr);
    static void msval_event(byte event, byte *info_ptr);
    static void msval_diag_result(byte event, byte *info_ptr);
    static void pmistate_changed(byte state, byte events);

    #ifdef MSV_DIAG_RESULT_MSG
        static void msval_store_rbd_result(byte rbd_status, byte length, byte *diag_id);
    #endif

#endif
static byte diag_id[] = "Eval110";

/* Socket Connection Manager */
#ifdef SCM_MIN
    static void scm_error(TMnsResult code, byte handle);
#endif

extern u8_t MnsServiceRequest;

/** Function to prepare a TNetServicesConfig structure.
  *
  * @param cfg_ptr is a pointer to the config structure to use.
  * @see   InitNetServices
  */
void prepare_mns_config(TNetServicesConfig *cfg_ptr)
{
	/* general system callbacks */
	cfg_ptr->general.most_service_request_fptr  = mns_service_request;
	cfg_ptr->general.on_init_complete_fptr      = mns_on_init_complete;
	cfg_ptr->general.on_stop_netservices_fptr   = NULL;
	cfg_ptr->general.on_error_fptr              = mns_on_error;
	cfg_ptr->general.on_buf_freed_fptr          = NULL;
	cfg_ptr->general.get_tick_count_fptr        = TmrGetMasterTick_Word;
	cfg_ptr->general.next_min_timeout_fptr      = mns_next_min_timeout;

	cfg_ptr->general.watchdogmode.overwrite     = MNS_TRUE;
	cfg_ptr->general.watchdogmode.reset         = MNS_FALSE;
	cfg_ptr->general.watchdogmode.timeout       = MNS_TM_WD_DISABLE;
	cfg_ptr->general.watchdogmode.auto_shutdown_delay = 0xFFFF;

	cfg_ptr->general.burst                      = 1;

	/* LLD interface callbacks and configuration */
	cfg_ptr->lld.start_interfaces_fptr          = lld_start_interfaces;
	cfg_ptr->lld.reset_fptr                     = lld_reset;
	cfg_ptr->lld.on_buf_freed_fptr              = lld_on_buf_freed;
	cfg_ptr->lld.i2c_tx_fptr                    = lld_i2c_transmit;     // using I2C for control / packet(if any)
	cfg_ptr->lld.ctrl_tx_fptr                   = NULL;
	cfg_ptr->lld.data_tx_fptr                   = NULL;
	cfg_ptr->lld.data_interface_id              = PMS_IFACE_I2C;

	/* application (control) message service wrapper callbacks */
	#ifdef AMS_MIN
	cfg_ptr->msg.tx_status_fptr             = msg_tx_status;
	cfg_ptr->msg.tx_filter_fptr             = msg_tx_filter;
	cfg_ptr->msg.rx_complete_fptr           = msg_rx_complete;
	#ifdef AMS_TX_BYPASS_FILTER
	cfg_ptr->msg.tx_bypass_fptr             = msg_tx_bypass;
	#endif
	#ifdef MSG_RX_USER_PAYLOAD
	cfg_ptr->msg.get_payload_buf_fptr       = msg_get_payload_buf;
	cfg_ptr->msg.free_payload_buf_fptr      = msg_free_payload_buf;
	cfg_ptr->msg.reallocate_payload_buf_fptr= msg_reallocate_payload_buf;
	#endif
	#ifdef MSG_TX_USER_PAYLOAD
	cfg_ptr->msg.free_tx_payload_buf_fptr   = msg_free_tx_payload_buf;
	#endif
	#ifdef NS_AMS_MSV2
	cfg_ptr->msg.tx_buffer_flushed_fptr     = msg_tx_buffer_flushed;
	cfg_ptr->msg.tx_msg_flushed_fptr        = msg_tx_msg_flushed;
	#endif
	#ifndef PMS_RX_OPT4
	cfg_ptr->msg.rx_direct                  = MNS_FALSE;
	#endif
	cfg_ptr->msg.retry_config_adjusted_fptr = msg_retry_config_adjusted;
	cfg_ptr->msg.rx_burst                   = 1;
	#endif

	/* pms configuration */
	cfg_ptr->pms.rx_burst                       = 4;


	/* asynchronous (packet) data service wrapper callbacks */
	#ifdef ADS_MIN
	cfg_ptr->wads.tx_status_fptr            = data_tx_status;
	cfg_ptr->wads.rx_complete_fptr          = data_rx_complete;
	cfg_ptr->wads.rx_filter_fptr            = data_rx_filter;
	#ifdef DATA_TX_USER_PAYLOAD
	cfg_ptr->msg.free_data_tx_payload_fptr  = data_free_tx_payload_buf;
	#endif
	cfg_ptr->wads.rx_direct                 = MNS_FALSE;
	cfg_ptr->wads.rx_burst                  = 1;
	#endif

	/* virtual MOST supervisor service callbacks */
	#ifdef VMSV_MIN
	cfg_ptr->vmsv.msval_state_changed_fptr      = msval_state_changed;
	cfg_ptr->vmsv.msval_error_fptr              = msval_error;
	cfg_ptr->vmsv.msval_event_fptr              = msval_event;
	cfg_ptr->vmsv.msval_diag_result_fptr        = msval_diag_result;
	cfg_ptr->vmsv.pmistate_changed_fptr         = pmistate_changed;
	#endif

	/* diag results callbacks */
	#ifdef MSV_DIAG_RESULT_MSG
	cfg_ptr->vmsv.rbd_store_rbd_result_fptr     = msval_store_rbd_result;
	cfg_ptr->vmsv.diag_id.length = sizeof(diag_id);
	cfg_ptr->vmsv.diag_id.stream = diag_id;
	cfg_ptr->vmsv.sso_restore_ssoresult_fptr = NULL;
	#endif


	/* Socket Connection Manager service callbacks */
	#ifdef SCM_MIN
	cfg_ptr->scm.on_error_fptr                  = scm_error;
	#endif
}

/** Whenever this callback is called the application should call MostService()
* asynchronously.
*
* @see   MostService
*/
static void mns_service_request(void)
{
	MnsServiceRequest = TRUE;
}

/** Whenever this callback is called the NetServices have completed their
* initialization (triggered by InitNetServices() or a re-attach scenario).
* The application is now allowed to call NetServices API functions (again).
*
* @see   InitNetServices, on_error
*/
static void mns_on_init_complete(void)
{
	app_on_init_complete();
}

/*! The MOST NetServices call this function to signal any kind of error
* identified by the TMnsResult code.
*
* @param result contains the code identifing the error cause
*/
static void mns_on_error(TMnsResult result)
{
	switch(result)
	{
		case NSR_E_REATTACH:
		LOG_ERR("mns_on_error(NSR_E_REATTACH)\n");
		break;

		case NSR_E_NO_COMM:
		LOG_ERR("mns_on_error(NSR_E_RESET_INIC)\n");
		break;

		case NSR_E_INIC_VERSION:
		LOG_ERR("mns_on_error(NSR_E_INIC_VERSION)\n");
		break;

		case NSR_E_INVALID_RETRY_CONFIG:
		LOG_ERR("mns_on_error(NSR_E_INVALID_RETRY_CONFIG)\n");
		break;

		case NSR_E_INVALID_REMOTE_ACCESS_CONFIG:
		LOG_ERR("mns_on_error(NSR_E_INVALID_REMOTE_ACCESS_CONFIG)\n");
		break;

		case NSR_E_BIST_MEM_CONTENT:
		LOG_ERR("mns_on_error(NSR_E_BIST_MEM_CONTENT)\n");
		break;

		case NSR_E_BIST_MEMORY:
		LOG_ERR("mns_on_error(NSR_E_BIST_MEMORY)\n");
		break;

		case NSR_E_BIST_CONF_STRING:
		LOG_ERR("mns_on_error(NSR_E_BIST_CONF_STRING)\n");
		break;

		case NSR_E_BIST_PCODE:
		LOG_ERR("mns_on_error(NSR_E_BIST_PCODE)\n");
		break;

		case NSR_E_BIST_DATA_MEMORY:
		LOG_ERR("mns_on_error(NSR_E_BIST_DATA_MEMORY)\n");
		break;

		case NSR_EI_RESET:
		LOG_ERR("mns_on_error(NSR_EI_RESET)\n");
		break;

		case NSR_EI_SYNC_FAILED:
		LOG_ERR("mns_on_error(NSR_EI_SYNC_FAILED)\n");
		break;

		case NSR_EI_SYNC:
		LOG_ERR("mns_on_error(NSR_EI_SYNC)\n");
		break;

		case NSR_E_INVALID_SEG_MODE_CONFIG:
		LOG_ERR("mns_on_error(NSR_E_INVALID_SEG_MODE_CONFIG)\n");
		break;

		default:
		/* unknown error code */
		LOG_ERR("mns_on_error, unknown error code: %04X\n", result);
		
		/*assert(MNS_FALSE);*/
	}
}

/*! This function is called to communicate the timeout till the next call to
* MostCheckTimers() is needed.
*/
static void mns_next_min_timeout (word timeout)
{
	app_start_check_timer(timeout);
}

/*    *******************      AMS Section    *******************      */

#ifdef AMS_MIN
/** Whenever this callback is called the NetServices have compeleted the
* transmission of a control message. Check the status and choose one of the
* following return values ...
*
* MSG_TX_FREE                 The message will automatically be freed.
*                             (In case of XMIT_SUCCESS/XMIT_NO_ICM or un-
*                             recoverable errors the only sense-making
*                             choice).
*
* MSG_TX_RETRY                The AMS will retry to send this segment.
* MSG_TX_POSTPONE             The AMS will not retry. You are considered to
*                             pass the message pointer to MsgSend() again.
*                             (In case of segmented messages this means that
*                             the entire message will be sent again).
*
* The application is not required to implement the callback, it is for your
* information only.
*
* @param  status is one of the XMIT_xxx constants explained in the manual.
*             XMIT_SUCCESS means success, every other status reports an error.
* @param  msg_ptr is a pointer to the message transmitted.
* @return MSG_TX_FREE, MSG_TX_RETRY or MSG_TX_POSTPONE
* @see    MsgGetTxPtrExt, MsgSend, MsgTxRetry, MsgTxPostpone
*/
static u8_t msg_tx_status(u8_t status, TMsgTx *msg_ptr)
{
	u8_t result = MSG_TX_FREE;

	switch (status)
	{
		case XMIT_CRC:
		result = MSG_TX_RETRY;
		break;

		default:
		break;
	}
	//    TRACE(TRACE_ON_MSG_TX_STATUS, status, result);

	return(result);
}

/** Whenever this callback is called the NetServices are going to transmit
* a control message. It can be used for debugging or other filtering purposes.
*
* You are not required to implement this callback, but if you choose to do so
* you have to return one of the following values ...
*
* MSG_TX_FILTER_DEFAULT   -> (Default) The message will be sent normaly.
* MSG_TX_FILTER_CANCEL    -> The message will not be sent but freed.
* MSG_TX_FILTER_BUSY      -> Since the application is busy, the AMS will not
*                            yet sent the message but call this callback
*                            again.
*
* @param  msg_ptr is a pointer to the message transmitted.
* @see    MsgGetTxPtrExt, MsgSend
*/
static u8_t msg_tx_filter(TMsgTx *msg_ptr)
{
	//    TRACE(TRACE_ON_MSG_TX_FILTER, 0, 0);
	return(MSG_TX_FILTER_DEFAULT);
}

/** Whenever this callback is called the NetServices have received a control
* message. The application is meant to handle it inside this callback.
*
* You are not required to implement this callback, but if you choose to do so
* you have to return one of the following values ...
*
* MSG_RX_TAKE            -> The application takes the responsibility for
*                           freeing the message via MsgFreeRxMsg().
* MSG_RX_FREE            -> (Default) The message will be freed by the AMS.
* MSG_RX_BUSY            -> The callback will be called again with this
*                           message. The UsageCnt is untouched.
* MSG_RX_INC_USAGE_CNT   -> Combination of MSG_RX_TAKE and MSG_RX_BUSY, but
*                           the UsageCnt is increased. That means that
*                           MsgFreeRxMsg() has to be called as often as this
*                           value was returned.
*
* @param  msg_ptr is a pointer to the message received.
* @see    msg_rx_error
*/
static u8_t msg_rx_complete(TMsgRx *msg_ptr)
{
	//    TRACE(TRACE_ON_MSG_RX_COMPLETE, msg_ptr->FBlock_ID, msg_ptr->Func_ID);

	//    if ((0xAA == msg_ptr->FBlock_ID) && (0xAAA == msg_ptr->Func_ID))
	//    {
	//        TRACE(TRACE_ON_EXIT_MSG, 0, 0);
	//        EXIT_APP();
	//    }
	return(MSG_RX_FREE);
}

/*! This function is called whenever the retry configuration is changed either
* by the application itself or by another network device.
*
* The parameter config_ptr denotes the current retry configuration.
*/
static void msg_retry_config_adjusted(TMsgRetryConfig *config_ptr)
{
	LOG_NOR("msg_retry_config_adjusted()\n");
}
#endif

#ifdef AMS_TX_BYPASS_FILTER
/*! This callback allows to decide if a message should
* bypass the control message queue.
*
* @param  msg_ptr is a pointer to the message transmitted.
*/
static bool msg_tx_bypass(TMsgTx *msg_ptr)
{
	LOG_NOR("msg_tx_bypass()\n");

	return;
}
#endif

#ifdef MSG_RX_USER_PAYLOAD
/*! callback function to request a buffer for receiving MOST control messages.
* It is mandatory, if MSG_RX_USER_PAYLOAD is enabled.
*
* @param  msg_ptr is a pointer to the message transmitted.
*
* @param  size is the number of bytes to be provided.
*/
static word msg_get_payload_buf(TMsgRx *msg_ptr, word size)
{
	LOG_NOR("msg_get_payload_buf(%d)\n", size);

	return;
}

/*! callback function is called  whenever a message buffer
* with external payload has to be freed .
* It is mandatory, if MSG_RX_USER_PAYLOAD is enabled.
*
* @param  msg_ptr is a pointer to the message transmitted.
*/
static void msg_free_payload_buf(TMsgRx *msg_ptr)
{
	LOG_NOR("msg_free_payload_buf()\n");

	return;
}

/*! callback function to request a new buffer for receiving MOST control
* messages. It is optional even if MSG_RX_USER_PAYLOAD is enabled.
*
* @param  msg_ptr is a pointer to the message transmitted.
*
* @param  size is the number of bytes to be provided.
*/
static word msg_reallocate_payload_buf(TMsgRx *msg_ptr, word size)
{
	LOG_NOR("msg_reallocate_payload_buf(%d)\n", size);

	return;
}

#endif

#ifdef ENABLE_CFG_MSG_FILTER
/*! Whenever this callback is called the NetServices have flushed
* the AMS tx message buffer due to a Configuration state change..
*
* The parameter num shows how many messages were deleted
* you have to return one of the following values ...
*
*
* @param  num is the number of deleted messages
*/
static void msg_tx_buffer_flushed(word num)
{
	LOG_NOR("msg_tx_buffer_flushed(%d)\n", num);

	return;
}
#endif


#ifdef ENABLE_CFG_MSG_FILTER
/*! This function is called whenever the application tries to send a message
* when the Configuration State is NotOk and the message must not be sent or
* prior to a reinitialization of NS when TX messages can have user allocated
* payload (MSG_TX_USER_PAYLOAD need to be defined to get this notification).
*
* The parameter msg_ptr denotes the respective message.
*
*/
static void msg_tx_msg_flushed(TMsgTx *msg_ptr)
{
	// This callback function is called, if a message is filtered by the
	// Configuration State Based Message Filter or if it is flushed from
	// a TX FIFO prior to a NS reinit.

	LOG_NOR("msg_tx_msg_flushed()\n");

	return;
}
#endif


/*    *******************      ADS Section    *******************      */
#ifdef ADS_MIN

/** Whenever this callback is called the NetServices have compeleted the
* transmission of a data packet. You can have a last look into the message
* before it is automatically freed. If you are using handles you might want
* to check it here.
*
* The application is not required to implement the callback, it is for your
* information only.
*
* @param  msg_ptr is a pointer to the message transmitted.
* @see    DataGetTxPtrExt, DataSend
*/
static void data_tx_status(TDataTx *msg_ptr)
{
	//    TRACE(TRACE_ON_DATA_TX_COMPLETE, 0, 0);
}

/** Whenever this callback is called the NetServices have received a data
* packet.
*
* The application is not required to implement the callback, but if it does
* it has to return one of the following constants:
*
* DATA_RX_COMPLETE_BUSY   - The application is busy an can not handle the
*                           callback at this moment. Therefor the NetServices
*                           will call this callback again with this message
*                           next time the ADS is serviced again.
* DATA_RX_COMPLETE_FREE   - (Default) The message has been handled by the
*                           application and may now be freed by the
*                           NetServices.
*
* @param  msg_ptr is a pointer to the message received.
* @return one of the above constants.
* @see    data_rx_filter
*/
static u8_t data_rx_complete(TDataRx *msg_ptr)
{
	//    static u8_t temp_buf[50];
	//    int cnt = 0;
	//
	//    for (;cnt < msg_ptr->Length ; cnt++)
	//    {
	//        temp_buf[cnt] = msg_ptr->Data[cnt];
	//    }
	//
	//    TRACE(TRACE_ON_DATA_RX_COMPLETE, msg_ptr->Length, (int) &temp_buf);
	return(DATA_RX_COMPLETE_FREE);
}

/** This callback will be called by the NetServices, if a data packet was
* received. It is called before data_rx_complete will be called. The callback
* can be used i.e. to fill a timestamp into the extended data member of the
* message.
*
* Note: The MOSThigh protocol implementatin of the NetServices use this
* function to filter incoming MHP telegrams.
*
* The application is not required to implement the callback, but if it does
* it has to return one of the following constants:
*
* DATA_RX_FILTER_BUSY     - The application is busy an can not handle the
*                           callback at this moment. Therefor the NetServices
*                           will call this callback again with this message
*                           next time the ADS is serviced again.
* DATA_RX_FILTER_FREE     - The message has been handled by the application
*                           and may now be freed by the NetServices. The
*                           callback data_rx_complete() will not be called in
*                           this case!
* DATA_RX_FILTER_DEFAULT  - (Default) The callback data_rx_complete() will be
*                           called next.
*
* @param  msg_ptr is a pointer to the message received.
* @return one of the above constants.
* @see    data_rx_complete
*/
static u8_t data_rx_filter(TDataRx *msg_ptr)
{
	//    TRACE(TRACE_ON_DATA_RX_FILTER, 0, 0);
	return(DATA_RX_FILTER_DEFAULT);
}
#endif

/*    *******************      Most Supervisor Section    *******************      */
#ifdef VMSV_MIN

static void msval_state_changed(u8_t state)
{
	LOG_NOR("NetServices Supervisor State: ");
	switch (state)
	{
		case MSVAL_S_OFF:
			LOG_NOR("MSVAL_S_OFF\n");
			app_go_net_off();
			break;
		case MSVAL_S_INIT:
			LOG_NOR("MSVAL_S_INIT\n");
			//PwrStopPowerDownTimer();                      // stay powered up now
			break;
		case MSVAL_S_RBD:
			LOG_ERR("MSVAL_S_RBD\n");
			//PwrStopPowerDownTimer();                      // stay powered up in RBD
			break;
		case MSVAL_S_ON:
			LOG_NOR("MSVAL_S_ON\n");
			app_go_net_on();
			break;
		case MSVAL_S_RBDRES:
			LOG_NOR("MSVAL_S_RBDRES\n");
			break;
		default:
			LOG_ERR("State code unknown = %02X\n", state);
			break;
	}
	//    TRACE(TRACE_ON_MSVAL_STATE_CHANGED, state, 0);
}

/*! The MOST NetServices call this function to communicate errors on the
* network interface. Possible error codes are MSVAL_ERR_INIT_ERROR,
* MSVAL_ERR_UNLOCK_CRITICAL and MSVAL_ERR_UNLOCK_SHORT.
*
* @param error is the code identifing the error
* @param info_ptr is not used (always NULL)
*/
static void msval_error(byte error, byte *info_ptr)
{
	LOG_ERR("NetServices msval_error: ");
	switch (error)
	{
		case MSVAL_ERR_UNLOCK_SHORT:
			LOG_ERR("MSVAL_ERR_UNLOCK_SHORT\n");
			break;
		case MSVAL_ERR_UNLOCK_CRITICAL:
			LOG_ERR("MSVAL_ERR_UNLOCK_CRITICAL\n");
			break;
		case MSVAL_ERR_INIT_ERROR:
			LOG_ERR("MSVAL_ERR_INIT_ERROR\n");
			break;
		case MSVAL_ERR_STARTUP_FAILED:
			LOG_ERR("MSVAL_ERR_STARTUP_FAILED\n");
			break;
		case MSVAL_ERR_STARTUP_BUSY:
			LOG_ERR("MSVAL_ERR_STARTUP_BUSY\n");
			break;
		case MSVAL_ERR_SHUTDOWN_FAILED:
			LOG_ERR("MSVAL_ERR_SHUTDOWN_FAILED\n");
			break;
		case MSVAL_ERR_SHUTDOWN_BUSY:
			LOG_ERR("MSVAL_ERR_SHUTDOWN_BUSY\n");
			break;
		default:
			LOG_ERR("Err code unknown = %02X\n", error);
			break;
	}
}

/*! The MOST NetServices call this function to communicate events on the
* network interface.
*
* @param event is the code identifing the event
* @param info_ptr contains additional information or NULL
*/
static void msval_event(byte event, byte *info_ptr)
{
	switch (event)
	{
		case MSVAL_E_UNLOCK:
			LOG_NOR("msval_event(MSVAL_E_UNLOCK)\n");
			break;
		case MSVAL_E_LOCKSTABLE:
			LOG_NOR("msval_event(MSVAL_E_LOCKSTABLE)\n");
			break;
		case MSVAL_E_MPR:
			LOG_NOR("msval_event(MSVAL_E_MPR)\n");
			break;
		case MSVAL_E_MPRDEL_INC:
			LOG_NOR("msval_event(MSVAL_E_MPRDEL_INC)\n");
			break;
		case MSVAL_E_MPRDEL_DEC:
			LOG_NOR("msval_event(MSVAL_E_MPRDEL_DEC)\n");
			break;
		case MSVAL_E_MPRDEL_EQUAL:
			LOG_NOR("msval_event(MSVAL_E_MPRDEL_EQUAL)\n");
			break;
		case MSVAL_E_NETON:
			LOG_NOR("msval_event(MSVAL_E_NETON)\n");
			break;
		case MSVAL_E_SHUTDOWN:
			LOG_NOR("msval_event(MSVAL_E_SHUTDOWN)\n");
			break;
		case MSVAL_E_NPR:
			LOG_NOR("msval_event(MSVAL_E_NPR)\n");
			break;
		case MSVAL_E_NET_ACTIVITY:
			LOG_NOR("msval_event(MSVAL_E_NET_ACTIVITY)\n");
			break;
		default:
			LOG_NOR("Unknown msval_event, event code is: %02X\n", event);
			break;
	}
}

static void msval_diag_result(u8_t event, u8_t *info_ptr)
{
	LOG_NOR("RBD Result is ");
	ET_Store_Diag_Result(event, info_ptr);                      // forward on to ET which saves it for tester
	switch (event)                                              // now print results
	{
		case MSVAL_DIAG_OK:
			LOG_NOR("MSVAL_DIAG_OK\n");
			break;
		case MSVAL_DIAG_POS:
			LOG_NOR("MSVAL_DIAG_POS = %02X\n", info_ptr[1]);
			break;
		case MSVAL_DIAG_FAILED:
			LOG_NOR("MSVAL_DIAG_FAILED: ");
			switch (info_ptr[1])
			{
			case 0x00:
				LOG_NOR("MultiMaster\n");
				break;
			case 0x01:
				LOG_NOR("AllSlave\n");
				break;
			case 0x02:
				LOG_NOR("Diag_Poor\n");
				break;
			}
			break;
		case MSVAL_DIAG_SIGNAL_NO_LOCK:
			LOG_NOR("MSVAL_DIAG_SIGNAL_NO_LOCK\n");
			break;
		default:
			LOG_NOR("Unknown RBD result, result code is: %02X\n", event);
			break;
	}
	//    TRACE(TRACE_ON_MSVAL_DIAG_RESULT, event, info_ptr[1]);
}

/*! The MOST NetServices call this function to deliver the Power Management
* Interface state.
*/
static void pmistate_changed(byte state, byte events)
{
	LOG_NOR("PMI State: ");
	switch (state)
	{
		case PMI_STATE_NORMAL:
			LOG_NOR("PMI_STATE_NORMAL\n");
			break;
		case PMI_STATE_CRITICAL:
			LOG_NOR("PMI_STATE_CRITICAL\n");
			break;
		case PMI_STATE_OFF:
			LOG_NOR("PMI_STATE_OFF\n");
			break;
	}
	LOG_NOR("PMI Events: PMI_REQPWROFF is ");
	if (events & 0x01)
	{
		LOG_NOR("True, ready to power down\n");
//		if (PWR_MGMT_ENABLED())
//		{
//			PwrShutDown();  // let power manager shut down our node
//		}
//		else
//		{
			LOG_NOR("DIP Sw6 is on - can't shut down\n");
//		}
	}
	else
	{
		LOG_NOR("False, staying powered up");
	}
	LOG_NOR(" - PMI_STP, Switch to Power is  ");
	if (events & 0x02)
	{
		LOG_NOR("True\n");
	}
	else
	{
		LOG_NOR("False\n");
	}
}

#endif


#ifdef MSV_DIAG_RESULT_MSG
void msval_store_rbd_result(byte rbd_status, byte length, byte *diag_id)
{
	int i;
	LOG_ERR("Got RBD result message from network:\n    RBD status: ");
	switch(rbd_status)
	{
		case 0x01:
			LOG_ERR("Activity, but no lock\n");
			break;
		case 0x02:
			LOG_ERR("No activity\n");
			break;
		case 0xEE:
			LOG_ERR("Invalid RBD result\n");
			break;
		default:
			LOG_ERR("Unknown rbd_status value\n");
			break;
	}
	LOG_ERR("Received DiagId stream is ");
	for(i=0; i<length; i++) {
		LOG_ERR("%02X ", diag_id[i]);
	}
	LOG_ERR("\n");
}
#endif


#ifdef SCM_MIN
/*! The MOST NetServices call this function to delivier
* Socket Connection Manager errors
*/
static void scm_error(TMnsResult code, byte handle)
{
	LOG_ERR("SCM Error: ");
	switch (code)
	{
		case NSR_E_SOURCE_DROP:
			LOG_ERR("SOURCE_DROP\n");
			SCError_CB(handle);                         // go destroy connections/sockets w/ error
			break;
		case NSR_E_MOST_BANDWIDTH:
			LOG_ERR("MOST Bandwidth Error\n");
			break;
		case NSR_E_MEDIALB_BANDWIDTH:
			LOG_ERR("MediaLB Channel Address / Bandwidth Error\n");
			break;
		case NSR_E_MOST_DEALLOCATED_ALL:
			LOG_ERR("All Network bandwidth is de-allocated\n");
			break;
	}
	LOG_ERR("SCM handle is: %02X\n", handle);
}
#endif


/*! The MOST NetServices call this function to pass trace information generated
* by the library.
*
* @param service identifier of the service
* @param event   identifier of the event
* @param pcnt    number of parameters following
*/
void mns_trace(int service, int event, int pcnt, ...)
{
	va_list ap;
	int     cnt = 0;
	int     params[NST_MAX_PCNT];

	va_start(ap, pcnt);
	for (;cnt < pcnt; cnt++)
	{
		params[cnt] = va_arg(ap, int);
	}
	va_end(ap);

	MnsTraceParser(service, event, TmrGetMasterTick_Word(), pcnt, params);
}

/* -----------------------------------------------------------------------------
* callback functions used by TraceLib
* -----------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : PrintTraceLine()                                           */
/* Description : Callback function which prints a debug message             */
/*                                                                          */
/* Parameter(s): str  pointer to message to print                           */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void PrintTraceLine(const char *str)
{
	LOG_NOR("%s\n", str);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : PrintErrorLine()                                           */
/* Description : Callback function which prints the parser's error          */
/*               and assert debug messagess                                 */
/* Parameter(s): str  pointer to message to print                           */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void PrintErrorLine(const char *str)
{
	LOG_ERR("%s\n", str);
}

/*****************************************************************************
* NetServices V3 assumes an operating system environment and calls the functions
* below to get and return exclusive control of the processor.  Since we don't have
* an OS - these functions just return.
**********************************************************************************/
void mns_take (int mutex_id)
{
	int dummy;

	dummy = mutex_id;

}

void mns_give (int mutex_id)
{
	int dummy;

	dummy = mutex_id;

}
