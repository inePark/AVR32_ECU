/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         MOST NetService Kernel (Application Socket)
File:           mns2.c
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

#ifdef MNS2_MIN
    #include "mns2.h"
#endif

#ifdef AH_MIN
    #include "ah.h"
#endif

    #include "nbehc.h"

#ifdef CMD_MIN
    #include "cmd.h"
#endif

#ifdef MSV2_MIN
    #include "msv2.h"
#endif

#ifdef NTF_MIN
    #include "ntfs.h"
#endif



#ifdef NS_INC_TGT_SPEC
#include NS_INC_TGT_SPEC        /* include the file which is defined in the configuration file */
#endif

#ifdef NS_INC_MNS2
#include NS_INC_MNS2
#endif


#ifdef MNS2_MIN
    #ifndef NS_MNS_MNS2
    #error NS_MNS_MNS2 must be defined in adjust1.h (Layer I), if module MNS2 (Layer II) is used !
    #endif
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

#ifdef NETWORKMASTER_SHADOW
extern word NetworkMaster_Addr;
#endif



/*
------------------------------------------------------------------------------
        Local variables and buffers
------------------------------------------------------------------------------
*/

#ifdef MNS2_OPT_1
word MnsPending2;
#endif




/*
------------------------------------------------------------------------------
        Local Function Prototypes (not available for the outside)
------------------------------------------------------------------------------
*/


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : InitNetServicesLayer2()                                    */
/* Description : Initializes all modules of the NetServices Layer II        */
/* Parameter(s): None                                                       */
/* Returns     : Nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef MNS2_0
void InitNetServicesLayer2(void)
{
    #ifdef MNS2_OPT_1
    MnsPending2 = (word)0x0000;                             /* Clear all Request Flags */
    #endif

    #ifdef NB_MIN
    NbInit();                                               /* Init NetBlock */
    #endif

    #ifdef AH_MIN
    AddrHInit();                                            /* Init Address Search Handler */
    #endif

    #ifdef CMD_MIN
    CmdInit();                                              /* Init MOST Command Interpreter */
    #endif

    #ifdef MSV2_MIN
    MostSupervisor2Init();                                  /* Init MOST Supervisor Layer II */
    #endif

    #ifdef NTF_0
    NtfInit();                                              /* Init Notification Service */
    #endif

    #ifdef ET_MIN
    ET_Init();
    #endif

    #ifdef NETWORKMASTER_SHADOW                             /* Only if NetworkMaster Shadow available */
    NetworkMaster_Addr = (word)0x0000;                      /* Init NetworkMaster Address */
    #endif
}
#endif




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : MostServiceLayer2()                                        */
/* Description : Trigger function of MOST NetServices Layer II              */
/* Parameter(s): options                                                    */
/* Returns     : - Request Flags, if MNS2_OPT_1 is defined in adjust2.h     */
/*               - 0xFFFF, if MNS2_OPT_1 is not defined in adjust2.h        */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef MNS2_1
word MostServiceLayer2(word opt, word events)
{
    #ifdef MNS2_OPT_1
    word ret;
    #endif

    #ifdef MNS2_OPT_1
    MnsPending2 |= (events & (MNS2_E_MASK));    /* capture relevant events */
    #endif

    if (MNS_NET_ON == MostGetState())       /* Check state of MOST Supervisor */
    {
        #ifdef MSV2_MIN
        #ifdef MNS2_OPT_1
        MnsPending2 &= MNS2_P_MASK;     /* mask used flags */
        MnsPending2 &= ~(word)(MNS2_P_MSV2_STATE | MNS2_P_MSV2_TIMEOUT);
        #endif
        #endif


        #ifdef MSV2_MIN                 /* only if Supervisor Layer II implemented */
        #ifdef MSV2_4
        if (ConfigState)
        {
            ConfigCheckStateMaster();
        }
        #endif
        #endif

        #ifdef NB_MIN
        NbService();
        #endif

        #ifdef MNS2_OPT_2
        if (!(opt&MNS2_O_NO_AH))
        #endif
        {
            #ifdef MNS2_OPT_1
            MnsPending2 &= ~(word)(MNS2_P_AH_STATE | MNS2_P_AH_TIMEOUT);
            #endif

            #ifdef AH_3
            AddrHService();                             /* Address Search Handler */
            #endif
        }

        #ifdef MNS2_OPT_2
        if (!(opt&MNS2_O_NO_NTFS))
        #endif
        {
            #ifdef MNS2_OPT_1
            MnsPending2 &= ~MNS2_P_NTFS;
            #endif

            #ifdef NTF_1
            NtfService();                               /* Notification Service */
            #endif
        }

    }
    #ifdef MNS2_OPT_1
    else
    {
        MnsPending2 = (word)0;                          /* Clear request flags, since not in MNS_NET_ON state */
    }
    #endif

    #ifdef MNS2_OPT_1
    ret = MnsPending2;                                  /* Return Pending Flags */
    return(ret);
    #else
    return (0xFFFF);
    #endif
}
#endif




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : MnsPending2Set()                                           */
/* Description : Set one or several request flags in variable MnsPending2   */
/*                                                                          */
/* Parameter(s): flags                                                      */
/* Returns     : Nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef MNS2_10
void MnsPending2Set(word flags)
{
    MnsPending2 |= flags;
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : MnsPending2SetAndCall()                                    */
/* Description : Set one or several request flags in variable MnsPending2   */
/*               and call application via callback function                 */
/*                                                                          */
/* Parameter(s): flags                                                      */
/* Returns     : Nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef MNS2_11
void MnsPending2SetAndCall(word flags)
{
    MnsPending2 |= flags;
    flags = MnsPending2;
    MnsRequestLayer2(flags);
}
#endif
