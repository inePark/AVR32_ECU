/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Adjustment for MOST High Protocol Service Module
File:           adjustmh.h
Version:        3.0.2.Alpha.1
Language:       C
Author(s):      S.Kerber
Date:           03.September.2009

FileGroup:      Layer II Extension: MOST High Protocol Service
Customer ID:    <None; as non-released alpha version>
FeatureCode:    FCR1
------------------------------------------------------------------------------

                (c) Copyright 1998-2009
                SMSC
                All Rights Reserved

------------------------------------------------------------------------------



Modifications
~~~~~~~~~~~~~
Date    By      Description

==============================================================================
*/

#ifndef _ADJUSTMH_H
#define _ADJUSTMH_H





/*-------------------------------------------------------------------*/
/*  MOST High Protocol Service Kernel */
/*-------------------------------------------------------------------*/


/*  #define MHP_AVRG_INT_RATE_DEFAULT   500 */      /* This macro can be used to disclose the performance category */
                                                    /* of the hardware layer for telegrams to be transmitted.  */
                                                    /* The performance category is measured in the average interval of */
                                                    /* time [in microseconds] between two TX events. */
                                                    /* Examples: */
                                                    /* a) using a main-loop architecture with an average cycle time */
                                                    /*    of 1 ms: */
                                                    /*    --> set value to 1000 */
                                                    /* b) using an event triggered architecture with an average possible */
                                                    /*    rate of 500 calls of NetServices Kernel per second: */
                                                    /*    --> set value to 2000 */
                                                    /* c) using the alternative packet driver interface with an */
                                                    /*    interrupt handler, that is able to service about 2000 interrupts */
                                                    /*    per second. */
                                                    /*    --> set macro to 500 */

                                                    /* If this macro is not set, one of the following default values is */
                                                    /* used by MHP Service: */
                                                    /* a) using ADS interface:  1000 */
                                                    /* b) using alternative packet interface: 500 */


/*  #define MHP_AVRG_INT_RATE_RX_DEFAULT   1000 */  /* This macro can be used to disclose the performance category */
                                                    /* of the hardware layer for telegrams to be received.         */
                                                    /* The performance category is measured in the average interval of */
                                                    /* time [in microseconds] between two RX events. */

                                                    /* If this macro is not set, one of the following default values is */
                                                    /* used by MHP Service: */
                                                    /* a) using ADS interface:  2000 */
                                                    /* b) using alternative packet interface: 1000 */



/*-------------------------------------------------------------------*/
/*  MOST High Protocol Service TX Section */
/*-------------------------------------------------------------------*/
    #define MHP_TX_MIN                              /* Enable MOST High Protocol Service Tx Section */


    #define MAX_MHP_TX          1                   /* Max. number of simultaneous Tx Connections */

    #define MHP_NO_COPY_ENABLE                      /* provides the message payload by pointer instead of copying into buffer */
                                                    /* saves one copy procedure of the MHP payload                            */

    #define MAX_MHP_TX_HANDLE   4                   /* size of tx handle (number of additional bytes) */

/*  #define MHP_DBG_TX */                           /* Enable Debug Mode of Tx Section */

/*  #define MHP_DELAY_RANGE_EN */                   /* Enables the possibility to set a range for the transmission delay. */
                                                    /* Usually this delay is fully controlled by the MHP service itself. */
                                                    /* By using this feature, the application can set a maximum and minimum */
                                                    /* value of delay. */

/*  #define MHP_ADDRESS_RESOLUTION */               /* Provides the possibility to build a connection to an unknown  */
                                                    /* Device ID (0xFFFF). It requires the availability of the Address Handler */
                                                    /* module (Layer II). */

/*  #define MHP_TX_OPT_COPY2 */                     /* Provides the possibility to save a copy process in the Tx section. In */
                                                    /* this case the pointer to the user payload is given to the ADS service. */


/*------------------------------------------------------------------- */
/*  MOST High Protocol Service RX Section */
/*------------------------------------------------------------------- */

    #define MHP_RX_MIN                              /* Enable MOST High Ptotocol Service Rx Section */

    #define MAX_MHP_RX          1                   /* Number of simultaneous Rx Connections */

    #define MAX_MHP_RX_HANDLE   4                   /* size of rx handle (number of additional bytes) */

/*  #define MHP_DBG_RX */                           /* Enable Debug Mode of Rx Section */

/*  #define MHP_RX_OPT_COPY1 */                     /* Provides the possibility to save a copy process in the Rx section. */


/*------------------------------------------------------------------- */
/*  MOST High Protocol Service Interface */
/*------------------------------------------------------------------- */

    #define MHP_MAX_PACKET_SIZE         1014        /* Maximum payload size of MOST Data Packets (MDP),             */
                                                    /* depends on used MDP interface: CP (I2C) or MediaLB interface */
                                                    /* If using CP (I2C): select 48                                 */
                                                    /* If using MediaLB : select up to 1014 (1014 is recommended!)  */


/*------------------------------------------------------------------- */
/*  Additional Debug Switches */
/*------------------------------------------------------------------- */

/*  #define MHP_DBG_TXTEL */                        /* Enable debug function to spy Tx telegrams */

/*  #define MHP_DBG_RXTEL */                        /* Enable debug function to spy Rx telegrams */


/*------------------------------------------------------------------- */
/*  Miscellaneous Switches */
/*------------------------------------------------------------------- */


/*
------------------------------------------------------------------------------
Memory Copy Function

   By defining this macro it is possible to select a user defined memory copy
   function. This function can be adapted to the used hardware (e.g. DMA, Cache 
   Alignments, DWord instead of byte copy loop).

   The function signature must be:
   - MHP_MEM_CPY: void FunctionName(byte* targetPointer, byte* sourcePointer, word size)

   If the macro is not defined, it will be mapped to MNS_MEM_CPY (adjust1.h)
------------------------------------------------------------------------------
*/

/*  #define MHP_MEM_CPY        MyMhpMemCpy  */







/*------------------------------------------------------------------- */
/*  User specific Include Files  */
/*------------------------------------------------------------------- */

/* The following files are included in the respective NetServices modules: */

/*  #define NS_INC_MHP          "my_file.h"  */ /* File is included in module mhp.c (in front of local definitions) */
/*  #define NS_INC_MHP_BEHIND   "my_file2.h" */ /* File is included in module mhp.c (behind local definitions) */









#endif /* _ADJUSTMH_H */




