/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Adjustment for MOST NetServices Application Socket
File:           adjust2.h
Version:        3.0.2.Alpha.1
Language:       C
Author(s):      S.Kerber
Date:           03.September.2009

FileGroup:      Layer II
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


#ifndef _ADJUST2_H
#define _ADJUST2_H



/*------------------------------------------------------------------- */
/*  Adjustment of Services of NetServices Layer II */
/*------------------------------------------------------------------- */


/*--------------------------- */
/*  NetService Kernel LayerII */
/*--------------------------- */
    #define MNS2_MIN                                /* MOST NetServices Layer II Kernel Module */



/*-------------------------- */
/*  Command Interpreter      */
/*-------------------------- */
    #define CMD_MIN                                 /* minimum requirement of MOST Command Interpreter */

/*    #define CMD_ADD1  */                          /* provides a function which copies whole data and length field of */
                                                    /* a received message to the message entry to transmit */

/*    #define CMD_ADD2  */                          /* Provides an external command interpreter for further FBlock Shadows, */
                                                    /* which are are not registered in the FBlock Table */

/*    #define CMD_ADD3  */                          /* Provides an additional callback function, which filters a received message */
                                                    /* before calling the NetServices Command Interpreter */

/*  #define CMD_ADD3_OPT1 */                        /* provides a callback function which is called if no Tx pointer is available */
                                                    /* and therefore the NetServices Command Interpreter cannot be called. */

/*  #define CMD_ADD4      */                        /* Provides an additional callback function, which is called whenever */
                                                    /* the command interpreter completes the result message to transmit. */
                                                    /* The application is able to monitor or modify each result message. */

/*  #define CMD_ADD5      */                        /* Provides an additional callback function, which is called */
                                                    /* whenever there is no result message to transmit */

    #define CMD_ADD7                                /* enables CmdGetFunctionIds() */

    #define CMD_ADD8                                /* enables automatic length check of received messages */

/*  #define CMD_LEN_CHECK_14BIT */                  /* sets the maximum number which can be used in a length check macro to 16383 (= 2^14 - 1).  */
                                                    /* If this macro is not set this maximum number is 63 (= 2^6 - 1). */

    #define CMD_NUM_WILDCARDS       0               /* Number of wildcard entries in the FBlock Table (T_FBlock.tab). */
                                                    /* i.e. Nullpointer in the second column (received InstID don't care) */

                                                    /* Please note: */
                                                    /*     FBlock 'NetBlock' (first table entry) must not be taken into account !!! */


    #define PTR_FUNCTION_VOID                       /* Avoids compiler errors if pointers to functions and data have different sizes */
                                                    /* When defined, macro NS_F_V can be used to cast data  and function pointers */
                                                    /* in the Command Interpreter and Notification Service tables */



/*-------------------------- */
/*  MOST Supervisor LayerII  */
/*-------------------------- */
    #define MSV2_MIN                                /* MOST Supervisor Layer II */

/*  #define NETWORKMASTER_INSTID    0x01 */         /* InstID of Networkmaster (NWM).  */
                                                    /* This macro is used only, if the NWM is implemented in my device. */
                                                    /* If the macro is not defined a default */
                                                    /* value of 0x01 is used (according to MOST Spec. 2V2) */
                                                    /* It can be changed to 0x00, if my NWM has to be compatible */
                                                    /* to MOST Spec. 2V1 and earlier. */

    #define CONFIGSTATE_NOTOK_DELAY_TIME    0           /* defines the time (in ms) between receiving a NB.Shutdown(Execute) command */
                                                        /* and point in time when system state is regarded as NotOK.                 */

    #define CFG_STATE_FILTER_EXCEPTION_LIST  0x22, 0x24, 0x0E /* Messages addressed to these FBlocks are sent independent         */
                                                        /* of the Configuration Status. Items have to be separated by comma */
                                                        /* Attention: The list must not be empty!                           */

/*-------------------------- */
/*  NetBlock Module          */
/*-------------------------- */
    #define NB_MIN                                      /* minimum requirement of MOST NetBlock */

/*  #define NB_ADD2     */                              /* enable user replacement for NB_FBlockIDs_Get() and NB_FBlockIDs_Set() */



/*  #define NB_ADD5     */                              /* enable possibility to save InstIDs in nonvolatile memory */
                                                        /* and to change InstID of Shadow FBlocks from application */

/*  #define NB_ADD6     */                              /* enable Device ShutDown  */

/*  #define NB_ADD7     */                              /* enable property Boundary for NetBlock */

/*  #define NB_ADD8     */                              /* enable property ImplFBlocks for NetBlock */


                                                        /* The following switches enable the receive section of */
                                                        /* status and result messages of the NetBlock. */
                                                        /* If your application has to react on these messages, */
                                                        /* you have to enable the respective switch(es): */

/*  #define CB_NB_FBLOCKIDS_STATUS          */          /* NetBlock.FBlockIDs.Status */
/*  #define CB_NB_DEVICEINFO_STATUS         */          /* NetBlock.DeviceInfo.Status */
/*  #define CB_NB_PERMISSIONTOWAKE_STATUS   */          /* NetBlock.PermissionToWake.Status */
/*  #define CB_NB_SHUTDOWN_RESULT           */          /* NetBlock.ShutDown.Result and NetBlock.ShutDown.ResultAck*/
/*  #define CB_NB_NODEPOSITION_STATUS       */          /* NetBlock.NodePosition.Status */
/*  #define CB_NB_NODEADDRESS_STATUS        */          /* NetBlock.NodeAddress.Status */
/*  #define CB_NB_GROUPADDRESS_STATUS       */          /* NetBlock.GroupAddress.Status */
/*  #define CB_NB_RETRYPARAMETERS_STATUS    */          /* NetBlock.RetryParameters.Status */
/*  #define CB_NB_SAMPLINGFREQ_STATUS       */          /* NetBlock.SamplingFrequency.Status */
/*  #define CB_NB_NOTIFICATION_STATUS       */          /* NetBlock.Notification.Status */
/*  #define CB_NB_NOTIFICATIONCHECK_STATUS  */          /* NetBlock.NotificationCheck.Status */
/*  #define CB_NB_BOUNDARY_STATUS           */          /* NetBlock.Boundary.Status */
/*  #define CB_NB_VERSION_STATUS            */          /* NetBlock.Version.Status */
/*  #define CB_NB_SHUTDOWNREASON_STATUS     */          /* NetBlock.ShutDownReason.Status */
/*  #define CB_NB_FBLOCKINFO_STATUS         */          /* NetBlock.FBlockInfo.Status */
/*  #define CB_NB_IMPLFBLOCKS_STATUS        */          /* NetBlock.ImplFBlocks.Status */
/*  #define CB_NB_EUI48_STATUS              */          /* NetBlock.EUI48.Status */

                                                        /* The following switches enable the receive section of */
                                                        /* error messages of the NetBlock. */
                                                        /* If your application has to react on these messages, */
                                                        /* you have to enable the respective switch(es): */

/*  #define CB_NB_FBLOCKIDS_ERROR       */              /* NetBlock.FBlockIDs.Error */
/*  #define CB_NB_DEVICEINFO_ERROR      */              /* NetBlock.DeviceInfo.Error */
/*  #define CB_NB_PERMISSIONTOWAKE_ERROR   */           /* NetBlock.PermissionToWake.Error */
/*  #define CB_NB_SHUTDOWN_ERROR        */              /* NetBlock.ShutDown.Error and NetBlock.ShutDown.ErrorAck*/
/*  #define CB_NB_NODEPOSITION_ERROR    */              /* NetBlock.NodePosition.Error */
/*  #define CB_NB_NODEADDRESS_ERROR     */              /* NetBlock.NodeAddress.Error */
/*  #define CB_NB_GROUPADDRESS_ERROR    */              /* NetBlock.GroupAddress.Error */
/*  #define CB_NB_RETRYPARAMETERS_ERROR */              /* NetBlock.RetryParameters.Error */
/*  #define CB_NB_SAMPLINGFREQ_ERROR    */              /* NetBlock.SamplingFrequency.Error */
/*  #define CB_NB_NOTIFICATION_ERROR    */              /* NetBlock.Notification.Error */
/*  #define CB_NB_NOTIFICATIONCHECK_ERROR   */          /* NetBlock.NotificationCheck.Error */
/*  #define CB_NB_BOUNDARY_ERROR*/                      /* NetBlock.Boundary.Error */
/*  #define CB_NB_VERSION_ERROR         */              /* NetBlock.Version.Error */
/*  #define CB_NB_SHUTDOWNREASON_ERROR  */              /* NetBlock.ShutDownReason.Error */
/*  #define CB_NB_FBLOCKINFO_ERROR      */              /* NetBlock.FBlockInfo.Error */
/*  #define CB_NB_IMPLFBLOCKS_ERROR     */              /* NetBlock.ImplFBlocks.Error */
/*  #define CB_NB_EUI48_ERROR   */                      /* NetBlock.EUI48.Error */


/*-------------------------- */
/*  Addr Search Handler      */
/*-------------------------- */
/*  #define AH_MIN   */                             /* Symbolic addressing */
                                                    /* This feature is only useful for devices which control */
                                                    /* other devices. */


    /* Configuration of AddressHandler: */
    /*--------------------------------- */
    #define ADDRH_SIZE_DEVICE_TAB     3             /* number of addresses of devices which can be stored (decentral registry) */
                                                    /* If set to zero, there is no decentral registry available. */

    #define ADDRH_SEARCH_METHOD     1               /* Search method of the Address Handler */
                                                    /*    1: central registry method  only */
                                                    /*    2: never using central registry */
                                                    /*    3: automatically */

    #define ADDRH_RETRY             3               /* Highlevel retry of the address search handler */
    #define ADDRH_TIMEOUT           200             /* Timeout of each try (value in ms) */

    #define ADDRH_DEVICE_TAB_NONVOLATILE            /* Decentral Registry is stored non-volatile if this switch is defined */


/*----------------------------- */
/*  NetworkMaster Shadow Module */
/*----------------------------- */


                                                    /* Please select one of the following two items: */

/*  #define NETWORKMASTER_LOCAL */                     /* a) NetworkMaster is implemented in my device, as part of my application. */

    #define NETWORKMASTER_SHADOW                    /* b) NetworkMaster Shadow (only an image of the NetworkMaster) is */
                                                    /* implemented in the my device, as part of NetServices. */

                                                    /* The following switches enable the receive section of */
                                                    /* status and result messages from the NetworkMaster. */
                                                    /* If your application has to react on these messages, */
                                                    /* you have to enable the respective switch(es): */

/*  #define CB_NM_FKTIDS_STATUS         */             /* NetworkMaster.FktIDs.Status  */
/*  #define CB_NM_VERSION_STATUS        */             /* NetworkMaster.Version.Status  */
/*  #define CB_NM_FBLOCKINFO_STATUS     */             /* NetworkMaster.FBlockInfo.Result */
/*  #define CB_NM_CONFIGURATION_STATUS  */             /* NetworkMaster.Configuration.Status */
/*  #define CB_NM_CENTRALREG_STATUS     */             /* NetworkMaster.CentralRegistry.Status */
/*  #define CB_NM_SAVECONFIG_RESULT     */             /* NetworkMaster.SaveConfiguration.Result and NetworkMaster.SaveConfiguration.ResultAck*/
/*  #define CB_NM_SYSTEMAVAIL_STATUS    */             /* NetworkMaster.SystemAvail.Result */


                                                    /* The following switches enable the receive section of */
                                                    /* error messages from the NetworkMaster. */
                                                    /* If your application has to react on these messages, */
                                                    /* you have to enable the respective switch(es): */

/*  #define CB_NM_FKTIDS_ERROR          */             /* NetworkMaster.FktIDs.Error */
/*  #define CB_NM_VERSION_ERROR         */             /* NetworkMaster.Version.Error  */
/*  #define CB_NM_FBLOCKINFO_ERROR      */             /* NetworkMaster.FBlockInfo.Error */
/*  #define CB_NM_CONFIGURATION_ERROR   */             /* NetworkMaster.Configuration.Error */
/*  #define CB_NM_CENTRALREG_ERROR      */             /* NetworkMaster.CentralRegistry.Error */
/*  #define CB_NM_SAVECONFIG_ERROR      */             /* NetworkMaster.SaveConfiguration.Error and NetworkMaster.SaveConfiguration.ErrorAck*/
/*  #define CB_NM_SYSTEMAVAIL_ERROR     */             /* NetworkMaster.SystemAvail.Error */


/*-------------------------- */
/*  Notification Service     */
/*-------------------------- */

    #define NTF_MIN                                /* Notification Service minimum requirement */

    #define NTF_EXT                                 /* extends the capabilities of the  Notification Service by following features: */
                                                    /*   - notify special events  (extended notification mechanism)                 */
                                                    /*   - notify "Error: Function not available"                                   */
                                                    /*   - advanced announcement of property changes                                */
                                                    /*   - API functions to delete DevIDs out of Notification Matrices              */

/*  #define NTF_ADD5   */                          /* NtfService searches Notification Table until notification is found */
                                                   /* or complete table is looked through                                */


    /* Configuration of NTFS: */
    /*--------------------------------- */
    #define NTF_MAX_FBLOCKS         2              /* Number of FBlocks supported by the Notification Service */

    #define NTF_SIZE_DEVICE_TAB     5              /* Maximum number of different addresses (physical or group addr.) */
                                                   /* which can be serviced by the Notification Service */

    #define NTF_EXT_OPTYPE          0xE            /* only relevant, if NTF_EXT is defined: */
                                                   /* This OP Type will be used, whenever a string has to be send */
                                                   /* using the extended notification mechanism */


/*-------------------------- */
/* FBlock ET implementation  */
/*-------------------------- */

#define ET_MIN                                   /* minimal FBlock ET support */

/* #define ET_ADD6 */                            /* adds ET support for Network Master */
/* #define ET_ADD7 */                            /* adds ET support in Timing Master */




/*------------------------------------------------------------------- */
/*  Number of implemented FBlocks and FBlock Shadows */
/*------------------------------------------------------------------- */

#ifdef SCOTT_LEE_3_0_3
#ifdef ET_MIN
    #define NUM_FBLOCKS         4                   /* Enter the number of FBlocks ( additional to NetBlock ! ) */
#else                                               /* Please Note: */
    #define NUM_FBLOCKS         3                   /* FBlock ET must be considered explicitely ! */
#endif

#ifdef NETWORKMASTER_SHADOW                         /* Number of FBlocks which can receive report messages only. */
    #define NUM_FBLOCKS_SHADOW  1                   /* These FBlocks are not really implemented but they are */
#else                                               /* images of the respective FBlocks. */
    #define NUM_FBLOCKS_SHADOW  0
#endif                                              /* Please note: */
                                                    /* The NetworkMaster Shadow Module must be considered explicitly ! */
#else

#define NUM_FBLOCKS_APP			3
#define NUM_FBLOCKS_SHADOW_APP  0

#endif

/*------------------------------------------------------------------- */
/*  Interface to MOST High Protocol Service */
/*------------------------------------------------------------------- */

/*  #define NS_CMD_MHP  */                             /* Provides interface between Command Interpreter */
                                                    /* and MOST High Protocol Service */

/*  #define NS_AH_MHP   */                             /* Provides Interface between Address Handler */
                                                    /* and MOST High Protocol Service */

/*  #define NS_MSV2_MHP */                          /* Provides interface between Supervisor Layer II */
                                                    /* and MOST High Protocol Service */






/*------------------------------------------------------------------- */
/*  User specific Include Files */
/*------------------------------------------------------------------- */

/* The following files are included in the respective NetServices modules: */

/*  #define NS_INC_AH       "my_file.h" */     /* File is included in module ah.c */
/*  #define NS_INC_CMD      "my_file.h" */     /* File is included in module cmd.c */
/*  #define NS_INC_MNS2     "my_file.h" */     /* File is included in module mns2.c */
/*  #define NS_INC_MSV2     "my_file.h" */     /* File is included in module msv2.c */
/*  #define NS_INC_NB       "my_file.h" */     /* File is included in module nb.c */
/*  #define NS_INC_NETWM_S  "my_file.h" */     /* File is included in module netwm_s.c */
/*  #define NS_INC_NTFS     "my_file.h" */     /* File is included in module ntfs.c */





#endif /* _ADJUST2_H */
