/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Network Master Shadow Module
File:           NetwM_S.c
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




/*
------------------------------------------------------------------------------
        Include Files
------------------------------------------------------------------------------
*/


#include "mostns.h"

#ifdef MSV2_MIN
    #include "msv2.h"
#endif

#ifdef AH_MIN
    #include "ah.h"
#endif
#include "netwm_s.h"

#ifdef NS_INC_NETWM_S
#include NS_INC_NETWM_S
#endif




/*
------------------------------------------------------------------------------
        Local Definitions
------------------------------------------------------------------------------
*/




/*
------------------------------------------------------------------------------
        Macro Definitions
------------------------------------------------------------------------------
*/



/*
------------------------------------------------------------------------------
        Type Declaration
------------------------------------------------------------------------------
*/





/*
------------------------------------------------------------------------------
        Global variables / imported variables
------------------------------------------------------------------------------
*/





/*
------------------------------------------------------------------------------
        Local variables and buffers
------------------------------------------------------------------------------
*/


#ifdef NETWORKMASTER_SHADOW                     /* Only if NetworkMaster Shadow available */
word NetworkMaster_Addr;                        /* Address of NetworkMaster device, which contains the */
                                                /* central registry */
#endif



/*
------------------------------------------------------------------------------
        Local Function Prototypes
------------------------------------------------------------------------------
*/





/*
------------------------------------------------------------------------------
        Tables
------------------------------------------------------------------------------
*/



#ifdef NETWORKMASTER_SHADOW         /* Only if NetworkMaster Shadow available */
    #include "t_nm_s.tab"           /* Table containing all FUNC_IDs and OP_TYPES of the NetworkMaster Shadow */
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NmGetNWMAddr()                                             */
/* Description : returns address of NetworkMaster                           */
/* Parameter(s): none                                                       */
/* Returns     : NetworkMaster Address                                      */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NM_1
word NmGetNWMAddr(void)
{
    return(NetworkMaster_Addr);
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NM_Configuration_Status()                                  */
/* Description : Service NetworkMaster.Configuration.Status                 */
/* Parameter(s): ptr on rx msg                                              */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NM_I1C
byte NM_Configuration_Status(pTMsgRx Rx_Ptr)
{
    #ifdef AH_MIN
    if (Rx_Ptr->Src_Adr != (word)0xFFFF)
    {
        NetworkMaster_Addr = Rx_Ptr->Src_Adr;       /* Get Address of Device, which contains the FBlock NetworkMaster */
    }
    #endif


    #ifdef NM_CBS1
    NmConfigurationStatus(Rx_Ptr);
    #endif

    #ifdef MSV2_3
    ConfigStatusChanged(Rx_Ptr);
    #endif

    return(OP_NO_REPORT);
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NM_Configuration_Error()                                   */
/* Description : Service NetworkMaster.Configuration.Error                  */
/* Parameter(s): ptr on rx msg                                              */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NM_I1F
byte NM_Configuration_Error(pTMsgRx Rx_Ptr)
{
    #ifdef NM_CBE1
    NmConfigurationError(Rx_Ptr);
    #endif
    return(OP_NO_REPORT);
}
#endif





/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NM_CentralReg_Status()                                     */
/* Description : Service NetworkMaster.CentralRegistry.Status               */
/* Parameter(s): ptr on rx msg                                              */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NM_I2C
byte NM_CentralReg_Status(pTMsgRx Rx_Ptr)
{
    #ifdef AH_7
    AddrHCheckCentralRegStatus(Rx_Ptr);
    #endif

    #ifdef NM_CBS2
    NmCentralRegStatus(Rx_Ptr);
    #endif
    return(OP_NO_REPORT);
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NM_CentralReg_Error()                                      */
/* Description : Service NetworkMaster.CentralRegistry.Error                */
/* Parameter(s): ptr on rx msg                                              */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NM_I2F
byte NM_CentralReg_Error(pTMsgRx Rx_Ptr)
{
    #ifdef AH_10
    AddrHCheckCentralRegError(Rx_Ptr);
    #endif

    #ifdef NM_CBE2
    NmCentralRegError(Rx_Ptr);
    #endif
    return(OP_NO_REPORT);
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NM_SaveConfig_Result()                                     */
/* Description : Service NetworkMaster.SaveConfiguration.Result             */
/* Parameter(s): ptr on rx msg                                              */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NM_I4C
byte NM_SaveConfig_Result(pTMsgRx Rx_Ptr)
{
    #ifdef NM_CBR4
    NmSaveConfigResult(Rx_Ptr);
    #endif
    return(OP_NO_REPORT);
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NM_SaveConfig_ResultAck()                                  */
/* Description : Service NetworkMaster.SaveConfiguration.ResultAck          */
/* Parameter(s): ptr on rx msg                                              */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NM_I4CA
byte NM_SaveConfig_ResultAck(pTMsgRx Rx_Ptr)
{
    #ifdef NM_CBRA4
    NmSaveConfigResultAck(Rx_Ptr);
    #endif
    return(OP_NO_REPORT);
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NM_SaveConfig_Error()                                      */
/* Description : Service NetworkMaster.SaveConfiguration.Error              */
/* Parameter(s): ptr on rx msg                                              */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NM_I4F
byte NM_SaveConfig_Error(pTMsgRx Rx_Ptr)
{
    #ifdef NM_CBE4
    NmSaveConfigError(Rx_Ptr);
    #endif
    return(OP_NO_REPORT);
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NM_SaveConfig_ErrorAck()                                   */
/* Description : Service NetworkMaster.SaveConfiguration.ErrorAck           */
/* Parameter(s): ptr on rx msg                                              */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NM_I4FA
byte NM_SaveConfig_ErrorAck(pTMsgRx Rx_Ptr)
{
    #ifdef NM_CBEA4
    NmSaveConfigErrorAck(Rx_Ptr);
    #endif
    return(OP_NO_REPORT);
}
#endif




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NM_FktIDs_Status()                                         */
/* Description : Service NetworkMaster.FktIDs.Status                        */
/* Parameter(s): ptr on rx msg                                              */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NM_I5C
byte NM_FktIDs_Status(pTMsgRx Rx_Ptr)
{
    #ifdef NM_CBS5
    NmFktIDsStatus(Rx_Ptr);
    #endif
    return(OP_NO_REPORT);
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NM_FktIDs_Error()                                          */
/* Description : Service NetworkMaster.FktID.Error                          */
/* Parameter(s): ptr on rx msg                                              */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NM_I5F
byte NM_FktIDs_Error(pTMsgRx Rx_Ptr)
{
    #ifdef NM_CBE5
    NmFktIDsError(Rx_Ptr);
    #endif
    return(OP_NO_REPORT);
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NM_SystemAvail_Status()                                    */
/* Description : Service NetworkMaster.SystemAvail.Status                   */
/* Parameter(s): ptr on rx msg                                              */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NM_I6C
byte NM_SystemAvail_Status(pTMsgRx Rx_Ptr)
{
    #ifdef NM_CBS6
    NmSystemAvailStatus(Rx_Ptr);
    #endif
    return(OP_NO_REPORT);
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NM_SystemAvail_Error()                                     */
/* Description : Service NetworkMaster.SystemAvail.Error                    */
/* Parameter(s): ptr on rx msg                                              */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NM_I6F
byte NM_SystemAvail_Error(pTMsgRx Rx_Ptr)
{
    #ifdef NM_CBE6
    NmSystemAvailError(Rx_Ptr);
    #endif
    return(OP_NO_REPORT);
}
#endif




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NM_FBlockInfo_Status()                                     */
/* Description : Service NetworkMaster.FBlockInfo.Status                    */
/* Parameter(s): ptr on rx msg                                              */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NM_I7C
byte NM_FBlockInfo_Status(pTMsgRx Rx_Ptr)
{
    #ifdef NM_CBS7
    NmFBlockInfoStatus(Rx_Ptr);
    #endif
    return(OP_NO_REPORT);
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NM_FBlockInfo_Error()                                      */
/* Description : Service NetworkMaster.FBlockInfo.Error                     */
/* Parameter(s): ptr on rx msg                                              */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NM_I7F
byte NM_FBlockInfo_Error(pTMsgRx Rx_Ptr)
{
    #ifdef NM_CBE7
    NmFBlockInfoError(Rx_Ptr);
    #endif
    return(OP_NO_REPORT);
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NM_Version_Status()                                        */
/* Description : Service NetworkMaster.Version.Status                       */
/* Parameter(s): ptr on rx msg                                              */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NM_I8C
byte NM_Version_Status(pTMsgRx Rx_Ptr)
{
    #ifdef NM_CBS8
    NmVersionStatus(Rx_Ptr);
    #endif
    return(OP_NO_REPORT);
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NM_Version_Error()                                         */
/* Description : Service NetworkMaster.Version.Error                        */
/* Parameter(s): ptr on rx msg                                              */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NM_I8F
byte NM_Version_Error(pTMsgRx Rx_Ptr)
{
    #ifdef NM_CBE8
    NmVersionError(Rx_Ptr);
    #endif
    return(OP_NO_REPORT);
}
#endif





/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NM_OwnConfigInvalid_ResultAck()                            */
/* Description : Service NetworkMaster.OwnConfigInvalid.ResultAck           */
/* Parameter(s): ptr on rx msg                                              */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NM_I9CA
byte NM_OwnConfigInvalid_ResultAck(pTMsgRx Rx_Ptr)
{
    #ifdef NM_CBRA9
    NmOwnConfigInvalidResultAck(Rx_Ptr);
    #endif
    return(OP_NO_REPORT);
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NM_OwnConfigInvalid_ErrorAck()                             */
/* Description : Service NetworkMaster.OwnConfigInvalid.ErrorAck            */
/* Parameter(s): ptr on rx msg                                              */
/* Returns     : OP_Type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NM_I9FA
byte NM_OwnConfigInvalid_ErrorAck(pTMsgRx Rx_Ptr)
{
    #ifdef NM_CBEA9
    NmOwnConfigInvalidErrorAck(Rx_Ptr);
    #endif
    return(OP_NO_REPORT);
}
#endif
