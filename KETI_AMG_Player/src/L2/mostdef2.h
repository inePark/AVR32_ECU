/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Definition File for MOST NetServices Application Socket
File:           MostDef2.h
Version:        3.0.x-SR-1  
Language:       C
Author(s):      S.Kerber
Date:           05.January.2011

FileGroup:      Layer II
Customer ID:    0018FF2A0300xx.N.KETI
FeatureCode:    FCR1
------------------------------------------------------------------------------

                (c) Copyright 1998-2011
                SMSC
                All Rights Reserved

------------------------------------------------------------------------------



Modifications
~~~~~~~~~~~~~
Date    By      Description

==============================================================================
*/


#ifndef _MOSTDEF2_H
#define _MOSTDEF2_H




/*------------------------------------------------------------------- */
/*  Define dummy macros, if not otherwise defined */
/*------------------------------------------------------------------- */
#ifndef CMD_NUM_WILDCARDS
    #define CMD_NUM_WILDCARDS   0
#endif




/*------------------------------------------------------------------- */
/*  General Definitions and Buffer Sizes */
/*------------------------------------------------------------------- */

#define AH_SIZE_ADDR_HNDL_BUF      ((byte)10)                  /* buffer size of address handler (taskbuffer) */




/*------------------------------------------------------------------- */
/*  Define Switches depending on your choice in adjust2.h */
/*------------------------------------------------------------------- */

#ifdef AH_MIN
    #ifndef ADDRH_SEARCH_METHOD
        #define ADDRH_SEARCH_METHOD 1
    #else
        #if (ADDRH_SEARCH_METHOD != 1)
            #error  ADDRH_SEARCH_METHOD must have value 1
        #endif   
    #endif
#endif




#if (defined NETWORKMASTER_LOCAL) && (defined NETWORKMASTER_SHADOW)
    #undef NETWORKMASTER_LOCAL
#endif

#if (defined NETWORKMASTER_LOCAL) && (defined AH_MIN)
    #undef ADDRH_SEARCH_METHOD
    #define ADDRH_SEARCH_METHOD 1

    #define CR_AVAILABLE                                            /* Central Registry available (local !!!) */
#endif


#ifdef NETWORKMASTER_LOCAL                                          /* no decentral registry available, if NetworkMaster */
    #undef ADDRH_SIZE_DEVICE_TAB                                    /* is implemented local */
#endif

#if (ADDRH_SEARCH_METHOD != 2) && (defined NETWORKMASTER_SHADOW)
    #define CR_AVAILABLE                                            /* Central Registry available (not local !!!) */
#endif

#if (ADDRH_SEARCH_METHOD == 2) || (ADDRH_SEARCH_METHOD == 3)        /* Search using FBlockID method */
    #define ADDRH_SEARCH_FBLOCKIDS
#endif


#ifndef NUM_FBLOCKS
    #ifndef NUM_FBLOCKS_APP
        #define NUM_FBLOCKS_APP     0
    #endif
    /* Calculation of number of FBlocks */
    #if (!defined ET_MIN) && (!defined DM_MIN)
        #define NUM_FBLOCKS     (NUM_FBLOCKS_APP)
    #elif (!defined ET_MIN) || (!defined DM_MIN)
        #define NUM_FBLOCKS     (NUM_FBLOCKS_APP + 1)
    #else
        #define NUM_FBLOCKS     (NUM_FBLOCKS_APP + 2)
    #endif
#else
    #error NUM_FBLOCKS must no longer be defined by the application. NUM_FBLOCKS_APP must be used instead.
#endif

#ifndef NUM_FBLOCKS_SHADOW
    #ifndef NUM_FBLOCKS_SHADOW_APP
        #define NUM_FBLOCKS_SHADOW_APP     0
    #endif
    /* Calculation of number of shadow FBlocks */
    #if (!defined NETWORKMASTER_SHADOW) 
        #define NUM_FBLOCKS_SHADOW     (NUM_FBLOCKS_SHADOW_APP)
    #else
        #define NUM_FBLOCKS_SHADOW     (NUM_FBLOCKS_SHADOW_APP + 1)
    #endif
#else
    #error NUM_FBLOCKS_SHADOW must no longer be defined by the application. NUM_FBLOCKS_SHADOW_APP must be used instead.
#endif


#define CMD_SIZE_FBLOCK_TABLE   ((byte)(1+NUM_FBLOCKS+NUM_FBLOCKS_SHADOW+CMD_NUM_WILDCARDS)) /* Number of entries in the FBlock Table */


#ifdef NS_MNS2_MHP
    #error NS_MNS2_MHP must not be defined in version 2.x
#endif

#if (!defined DISABLE_CFG_MSG_FILTER) && ((defined NETWORKMASTER_SHADOW) || (defined NETWORKMASTER_LOCAL))
    #define ENABLE_CFG_MSG_FILTER
#endif

#if (defined ENABLE_CFG_MSG_FILTER) && (!defined MSV2_MIN)
    #error If MSV2_MIN is not defined, DISABLE_CFG_MSG_FILTER must be defined!
#endif

#if (defined NS_AMS_MSV2) && (defined DISABLE_CFG_MSG_FILTER)
    #error NS_AMS_MSV2 must not be defined if DISABLE_CFG_MSG_FILTER is defined.
#endif

#if !(defined NS_AMS_MSV2) && (defined ENABLE_CFG_MSG_FILTER)
    #error NS_AMS_MSV2 must be defined if DISABLE_CFG_MSG_FILTER is not defined.
#endif


typedef TMostTimer TMostTimerHL;


/*------------------------------------------------------------------- */
/*  Macros to set a request for MostServiceLayer2() */
/*------------------------------------------------------------------- */
#ifdef MNS2_OPT_1
#define MNS2_REQUEST_SET(flags)         MnsPending2Set(flags);
#define MNS2_REQUEST_CALL(flags)        MnsPending2SetAndCall(flags);
#else
#define MNS2_REQUEST_SET(flags)
#define MNS2_REQUEST_CALL(flags)
#endif






/*------------------------------------------------------------------- */
/*  Circumference of Net Services Layer II */
/*  (will be prepared automatically after adjusting the Layer II) */
/*------------------------------------------------------------------- */


/* MOST Net Service Kernel Layer II */
/*------------------------------------ */
#ifdef MNS2_MIN                                         /* minimum requirement of MOST NetServices Layer II */
        #define MNS2_0
        #define MNS2_1
        #define MNS2_12

        #ifdef AH_MIN
         #define MNS2_21                                        /* Software Timer of Address Handler */
        #endif

#ifdef MNS2_OPT_1                                       /* Option 1: Request Flags */
        #define MNS2_10
        #define MNS2_11
        #define MNS2_CB1
        #define MNS2_CB2
#endif

#endif /* MNS2_MIN */


/* MOST Supervisor Layer II */
/*------------------------------------ */
#ifdef MSV2_MIN
        #define MSV2_0
        #define MSV2_8

        #ifdef NETWORKMASTER_LOCAL                      /* NetworkMaster Config */
            #define MSV2_4
            #define MSV2_5
            #define MSV2_6
            #define MSV2_7
            #define MSV2_CB2
            #define MSV2_CB3
            #define MSV2_10
        #else                                           /* NetworkSlave Config */
            #ifdef NETWORKMASTER_SHADOW
                #define MSV2_3
                #define MSV2_CB5
                #define MSV2_9
            #endif
        #endif

        #define MSV2_CB1
        #define MSV2_CB4


        #ifdef ENABLE_CFG_MSG_FILTER
            #define MSV2_11
            #define MSV2_12
            #define MSV2_13
        #endif

#endif





/* MOST Rx Command Interpreter */
/*------------------------------------ */
#ifdef CMD_MIN                                          /* minimum requirement of Rx Command Interpreter */
        #define CMD_0
        #define CMD_1
        #define CMD_2
        #define CMD_3
        #define CMD_4
        #define CMD_5
        #define CMD_6
        #define CMD_8
        #define CMD_9
        #define CMD_10
        #define CMD_11
        #define CMD_12
        #define CMD_13
        #define CMD_14
        #define CMD_15
        #define CMD_16
        #define CMD_20
        #define CMD_21
        #define CMD_22


        #define CMD_CB1                                 /* report unknown FBlock Shadow */
        #define CMD_CB2                                 /* filtering received messages */
        #define CMD_CB3                                 /* filtering the resulting messages */
        #define CMD_CB4                                 /* event is fired, whenever no result is transmitted */

#ifdef CMD_ADD7
    #define CMD_23                                      /* provides CmdGetFunctionIds() */
#endif

#endif /* CMD_MIN */




/* MOST NetBlock */
/*------------------------------------ */

#ifdef NB_MIN                                           /* minimum requirement of NetBlock */
        #define NB_0
        #define NB_4
        #define NB_5
        #define NB_6
        #define NB_7
        #define NB_8
        #define NB_10
        #define NB_12
        #define NB_13
        #define NB_14
        #define NB_15

        #define AMS_T23



                                                       /* NetBlock.NodeAddress */
        #define NB_I1C
        #define NB_CBS1
        #define NB_I1F
        #define NB_CBE1


        #define NB_I20                                  /* NetBlock.FBlockIDs */
        #define NB_I21
        #define NB_I2C
        #define NB_CBS2
        #define NB_I2F
        #define NB_CBE2


        #define NB_I31                                  /* NetBlock.DeviceInfo */
        #define NB_I3C
        #define NB_CBS3
        #define NB_I3F
        #define NB_CBE3


        #define NB_I4C
        #define NB_CBS4
        #define NB_I4F
        #define NB_CBE4



        #define NB_I70                                  /* NetBlock.ShutDown*/
        #define NB_I7C
        #define NB_CBS7
        #define NB_I7F
        #define NB_CBE7


                    /* NetBlock.Nodeposition */
        #define NB_I9C
        #define NB_CBS9
        #define NB_I9F
        #define NB_CBE9

        #define NB_I10C
        #define NB_CBS10
        #define NB_I10F
        #define NB_CBE10


        #define NB_I11C
        #define NB_CBS11
        #define NB_I11F
        #define NB_CBE11


    #ifdef NB_ADD7
        #define NB_I142                                 /* NetBlock.Boundary */
    #endif
        #define NB_I14C
        #define NB_CBS14
        #define NB_I14F
        #define NB_CBE14

        #define NB_I151                                 /* NetBlock.Version */
        #define NB_I15C
        #define NB_CBS15
        #define NB_I15F
        #define NB_CBE15


    #ifdef _OS81110_SSO
        #define NB_I160
        #define NB_I161
        #define NB_I16C
        #define NB_CBS16
        #define NB_I16F
        #define NB_CBE16
    #endif


        #define NB_CB20
        #define NB_I171
        #define NB_I17C
        #define NB_CBS17
        #define NB_I17F
        #define NB_CBE17


    #ifdef NB_ADD8
        #define NB_I181
    #endif
        #define NB_I18C
        #define NB_CBS18
        #define NB_I18F
        #define NB_CBE18

        #define NB_I19C
        #define NB_CBS19
        #define NB_I19F
        #define NB_CBE19

    #define NB_CB1

    #define NB_CB4
    #define NB_CB5
    #define NB_CB6
    #define NB_CB7
    #define NB_CB8
    #define NB_CB17


    #ifdef NB_ADD2
        #define NB_CB9
        #define NB_CB16
      #ifdef NB_ADD8
        #define NB_CB21
      #endif
    #endif


    #ifdef NB_ADD5
      #define NB_9

      #if (NUM_FBLOCKS > 0)
        #define NB_CB12
        #define NB_CB13
      #endif

      #if (NUM_FBLOCKS_SHADOW > 0)
        #define NB_CB14
        #define NB_CB15
      #endif
    #endif

    #ifdef NB_ADD7
        #define NB_CB19
    #endif


    #define MOST_GET_NODE_POS() MostGetNodePos()

#endif  /* NB_MIN */





/* Address Search Handler */
/*------------------------------------ */
#ifdef AH_MIN                                           /* minimum requirement of address search handler */
        #define AH_0
        #define AH_1
        #define AH_2
        #define AH_3
        #define AH_4
        #define AH_11
        #define AH_14

        #ifdef CR_AVAILABLE                                 /* if central registry available (not local !) */
            #define AH_7
            #define AH_10
        #endif

        #ifdef ADDRH_SEARCH_FBLOCKIDS                       /* if search method via FBlockIDs possible */
            #define AH_5
            #define AH_6
        #endif

        #if (ADDRH_SIZE_DEVICE_TAB > 0)                        /* decentral registry */
            #define AH_01
            #define AH_8
            #define AH_9
            #define AH_12
            #define AH_13
        #endif

        #define AH_CB1
        #define AH_CB2

        #if (defined ADDRH_DEVICE_TAB_NONVOLATILE) && (ADDRH_SIZE_DEVICE_TAB > 0)
            #define AH_CB3                                  /* store/restore decentral registry */
            #define AH_CB4
        #endif

#endif  /* AH_MIN */



/* NetworkMaster Shadow Module */
/*------------------------------------ */
#ifdef NETWORKMASTER_SHADOW

    #define NM_1
                                                        /* NetworkMaster.Configuration */
    #define NM_I1C                                      /*      since coniguration status report is needed by */
                                                        /*      configuration process of the NetServices */
    #define NM_CBS1
    #define NM_I1F
    #define NM_CBE1

                                                        /* NetworkMaster.CentralRegistry */
    #define NM_I2C
    #define NM_CBS2
    #define NM_I2F
    #define NM_CBE2


    #define NM_I4C                                  /* NetworkMaster.SaveConfiguration */
    #define NM_I4CA
    #define NM_CBR4
    #define NM_CBRA4
    #define NM_I4F
    #define NM_I4FA
    #define NM_CBE4
    #define NM_CBEA4

    #define NM_I5C
    #define NM_CBS5
    #define NM_I5F
    #define NM_CBE5

    #define NM_I6C
    #define NM_CBS6
    #define NM_I6F
    #define NM_CBE6

    #define NM_I7C
    #define NM_CBS7
    #define NM_I7F
    #define NM_CBE7

    #define NM_I8C
    #define NM_CBS8
    #define NM_I8F
    #define NM_CBE8

    #define NM_I9CA
    #define NM_CBRA9
    #define NM_I9FA
    #define NM_CBEA9


#endif  /* NETWORKMASTER_SHADOW */

#if (defined NETWORKMASTER_LOCAL) && (defined AH_MIN)
    #define NM_I2C                                  /*      status and error report is needed by the Addr Handler (AH) */
    #define NM_I2F
#endif

#ifdef NETWORKMASTER_INSTID
    #error Macro NETWORKMASTER_INSTID must not be set by the user.
#else
    #define NETWORKMASTER_INSTID    0x01            /* InstID of Networkmaster (NWM).  */
                                                    /* This macro is used only, if the NWM is implemented in my device. */
#endif



/* Notification Service */
/*------------------------------------ */

#ifdef NTF_MIN                                          /* minimum requirement of Notification Service */

    #if (NUM_FBLOCKS == 0)
        #error Notification Service is only available if NUM_FBLOCKS > 0
    #endif

    #define NTF_0
    #define NTF_1
    #define NTF_2
    #define NTF_3
    #define NTF_4
    #define NTF_5

    #define NTF_7
    #define NTF_8
    #define NTF_9
    #define NTF_10
    #define NTF_11
    #define NTF_12
    #define NTF_13
    #define NTF_20
    #define NTF_21
    #define NTF_22
    #define NTF_30
    #define NTF_31
    #define NTF_40
    #define NTF_41
    #define NTF_50
    #define NTF_51
    #define NTF_52
    #define NTF_53
    #define NTF_54
    #define NTF_55
    #define NTF_60
    #define NTF_61

#ifdef NTF_EXT
    #define NTF_42                                     /* use Notification Service to send error message */
    #define NTF_43                                     /* use Notification Service to send interface description */
    #define NTF_44                                     /* advanced announcement of property changes */
    #define NTF_45

    #define NTF_62                                     /* API functions to delete DevIDs out of Notification Matrices */
    #define NTF_63
    #define NTF_64
    #define NTF_65
    #define NTF_66
    #define NTF_67
#endif


#ifdef MNS2_OPT_1
    #define NTF_6                                       /* needed by MOST NetService Kernel Layer II, to calculate the number of elements */
#endif

#endif /* NTF_MIN */


/* FBlock ET                           */
/*------------------------------------ */
#ifdef ET_MIN

  #ifndef NS_MSV_ET
    #error NS_MSV_ET needs to be defined if ET_MIN is defined!
  #endif

  #ifndef VMSV_MIN
    #error VMSV_MIN must be defined, if ET_MIN is defined!
  #endif

  #if (defined MAX_MSG_TX_DATA) && (!defined AMS_TX_NOSEG)
    #if MAX_MSG_TX_DATA <= PMS_CTRL_MAX_PAYLOAD
    #error MAX_MSG_TX_DATA needs to be bigger than 45
    #endif
  #endif

#else

  #ifdef NS_MSV_ET
    #error NS_MSV_ET must not be defined if ET_MIN is not defined!
  #endif

#endif


/* FBlock DebugMessages                */
/*------------------------------------ */
#ifdef DM_MIN
  #ifndef MDM_MIN
    #error MDM_MIN must be defined, if DM_MIN is defined!
  #endif

    #define DM_0        /* DM_AdjAppDbgMsg_Set */
    #define DM_1        /* DM_AdjAppDbgMsg_Get */
    #define DM_2        /* DM_AdjAppDbgMsg_Status */
    #define DM_3        /* DM_AdjAppDbgMsg_Error */

    #define DM_CB0      /* DmAdjAppDbgMsgStatus */
    #define DM_CB1      /* DmAdjAppDbgMsgError */
#endif


#endif /* _MOSTDEF2_H */
