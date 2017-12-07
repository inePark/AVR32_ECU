/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Notification Service
File:           ntfs.c
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
Date            By      Description

==============================================================================
*/







/*
------------------------------------------------------------------------------
        Include Files
------------------------------------------------------------------------------
*/


    #include "mostns.h"

#ifdef NB_MIN
    #include "nbehc.h"
#endif

#ifdef NTF_MIN
    #include "ntfs.h"
#endif

#ifdef MNS2_MIN
    #include "mns2.h"
#endif

#ifdef NS_INC_NTFS
    #include NS_INC_NTFS
#endif



#ifdef NTF_ADD5
    #ifndef NS_MNS_MNS2
        #error NS_MNS_MNS2 must be defined in adjust1.h (Layer I), if NTF_ADD5 is defined in adjust2.h !
    #endif
#endif


/*
------------------------------------------------------------------------------
        Local Definitions
------------------------------------------------------------------------------
*/

#define MASK_TYPE       ((byte) 0x7)             /* Mask for Flags in Bits: 12..14 of Flagfield */
#define MASK_TYPE_EXT   ((byte) 0x8)             /* Mask for Flag in Bit 15 of Flagfield */

#define NTF_TYPE_BOOL       (NTF_BOOL>>12)
#define NTF_TYPE_BYTE       (NTF_BYTE>>12)
#define NTF_TYPE_WORD       (NTF_WORD>>12)
#define NTF_TYPE_LONG       (NTF_LONG>>12)
#define NTF_TYPE_IND        (NTF_IND>>12)
#define NTF_TYPE_INDTXRX    (NTF_INDTXRX>>12)

#define NTF_TYPE_EXT_DATA (NTF_EXT_DATA>>12)
#define NTF_TYPE_EXT_FUNC (NTF_EXT_FUNC>>12)

#define PROP_TERMINATION (NTF_TERMINATION & (word)0x0FFF)

#if (NTF_SIZE_DEVICE_TAB <= 32)
#define MASK_DEV_INDEX      ((TNtfMatrix)0x1F)      /* Device Index     (bits: 4..0) */
#define MASK_FLAG_EXT1      ((TNtfMatrix)0x20)      /* LineFlag: EXT1   (bit: 5) */
#define MASK_FLAG_NTF       ((TNtfMatrix)0x40)      /* LineFlag: NTF    (bit: 6) */
#define MASK_FLAG_SINGLE    ((TNtfMatrix)0x80)      /* LineFlag: SINGLE (bit: 7) */
#else /*  32 < NTF_SIZE_DEVICE_TAB <= 255 : */
#define MASK_DEV_INDEX      ((TNtfMatrix)0x00FF)    /* Device Index     (bits: 8..0) */
#ifdef NTF_EXT
#define MASK_FLAG_EXT1      ((TNtfMatrix)0x2000)    /* LineFlag: EXT1   (bit: 13) */
#endif
#define MASK_FLAG_NTF       ((TNtfMatrix)0x4000)    /* LineFlag: NTF    (bit: 14) */
#define MASK_FLAG_SINGLE    ((TNtfMatrix)0x8000)    /* LineFlag: SINGLE (bit: 15) */
#endif

#ifdef NTF_EXT
#define MASK_FLAG_ERROR     ((TNtfMatrix)0x01)      /* ColumnFlag: ERROR (inverted!)                         */
#endif


/* ErrorInfo for Notification.Error: */
#define NTF_ERR_FBLOCK_NOTAVAIL     ((byte)0x20)    /* FBlock.Inst is not registered in the NtfService */
#define NTF_ERR_DEVTABLE_OVERFLOW   ((byte)0x21)    /* Device Table overflow; Notification.Set is denied */
#define NTF_ERR_MATRIX              ((byte)0x10)    /* Single entries cannot be entered, since matrix overflow, */
                                                    /* or respective property is not registered */
#define NTF_ERR_DEVICEID_NOTAVAIL   ((byte)0x08)    /* DeviceID not entered in DeviceTab */

#define NTF_ERR_MSG_BUF_OVERFLOW    ((byte)0x01)    /* Message length exceeds buffer size (MAX_MSG_TX_DATA) */

#define NTF_SINGLE_PREV                             /* Prevent multiple identical single notification entries */




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

#ifdef NTF_MIN
pTNtfFBlockL NtfFBlockPtr;                  /* Pointer at table containing all FBlocks to service */

byte NtfFBlockIndex;                        /* local FBlock index of respective FBlock  */
                                            /* (position in the FBlock table of the command interpreter) */

byte NtfNumDev;                             /* maximum number of devices to service ( of the respective FBlock ) */


pTNtfPropL   NtfPropTabPtr;                 /* Pointer at property table of respective FBlock */

byte  NtfType;                              /* notification type of respective property */
word  NtfFuncID;                            /* Function ID of respective property */

pTNtfMatrix NtfMatrixColPtr;                /* pointer at device index table (column of notification matrix) of respective property */
TNtfMatrix  NtfPropFlags;                   /* Flags of respective property (ColumnFlags) */

#ifdef PTR_FUNCTION_VOID
void (*NtfPropertyPtr)(void);               /* pointer at respective property */
#else
void* NtfPropertyPtr;                       /* pointer at respective property */
#endif

#ifdef NTF_EXT
#ifdef PTR_FUNCTION_VOID
void (*NtfPropertyExt1Ptr)(void);           /* extended pointer of respective property */
#else
void* NtfPropertyExt1Ptr;                   /* extended pointer of respective property */
#endif
#endif

byte NtfIndexNext;                          /* index for device index table */


word NtfDeviceTab[NTF_SIZE_DEVICE_TAB];     /* Device Table of Notification Service */
byte NtfDeviceTabWriteIndex;
word NtfDeviceTabCnt[NTF_SIZE_DEVICE_TAB];  /* Array containing counters for each DeviceTab entry */
#endif

#ifdef NTF_6
word NtfNumElements;                        /* Number of all matrix elements */
word NtfLoopCounter;                        /* Loop Counter which indicates demand for re-trigger */
#endif



/*
------------------------------------------------------------------------------
        Local Function Prototypes
------------------------------------------------------------------------------
*/


#ifdef NTF_2
void NtfLoadFBlockDescription(void);
#endif

#ifdef NTF_3
void NtfLoadPropDescription(void);
#endif

#ifdef NTF_4
void NtfIncFBlockPtr(void);
#endif

#ifdef NTF_5
void NtfIncPropTabPtr(void);
#endif

#ifdef NTF_6
word NtfCalcNum(void);
#endif

#ifdef NTF_8
void NtfInitAllMatrices(void);
#endif

#ifdef NTF_9
byte NtfInitOneMatrix(byte ntf_fblock_index, bool init_mode);
#endif

#ifdef NTF_20
byte NtfWriteDeviceIndex(pTNtfMatrix device_index_tab_ptr, byte num_entries, byte device_index);
#endif

#ifdef NTF_21
byte NtfClearDeviceIndex(pTNtfMatrix device_index_tab_ptr, byte num_entries, byte device_index);
#endif

#ifdef NTF_22
byte NtfScanDeviceIndexTab(pTNtfMatrix* device_index_tab_ptrptr, byte num_entries, byte device_index);
#endif

#ifdef NTF_30
byte NtfWriteDeviceTab(word device_addr);
#endif

#ifdef NTF_31
byte NtfScanDeviceTab(word device_addr);
#endif

#ifdef NTF_50
byte NtfLoadFBlockDescriptionRx(pTMsgRx rx_ptr, byte* num_dev_ptr, pTNtfPropL* prop_tab_baseptrptr);
#endif

#ifdef NTF_51
pTNtfFBlockL NtfGetFBlockTabPtr(byte fblock_index);
#endif

#ifdef NTF_52
pTNtfPropL NtfGetPropTabPtr(pTNtfPropL prop_tab_ptr, word func_id);
#endif

#ifdef NTF_53
void NtfChangeAllFlags(pTNtfMatrix device_index_tab_ptr, byte num_entries, TNtfMatrix flags_set);
#endif

#ifdef NTF_54
void NtfErrorAppendFuncID(byte *ptr_num_failed_func, byte **pptr_tgt, word func_id);
#endif

#ifdef NTF_55
byte NtfErrorComplFuncID(byte num, pTMsgTx tx_ptr);
#endif

#ifdef NTF_60
byte NtfConvertFBlockIndex(byte fblock_index);
#endif

#ifdef NTF_62
byte NtfDelNotification(pTNtfMatrix device_index_tab_ptr, byte num_entries, byte device_index);
#endif

#ifdef NTF_66
byte NtfPrepVars(pTNtfPropL* prop_tab_ptrptr, byte* num_dev, byte* device_index,
                 word device_id, byte fblock_id, byte inst_id);
#endif

#ifdef NTF_67
byte NtfDelDevIDMatrix(pTNtfPropL prop_tab_ptr, byte num_dev, byte device_index);
#endif




/*
------------------------------------------------------------------------------
        Tables
------------------------------------------------------------------------------
*/

#ifdef NTF_MIN
#include "t_notify.tab"                     /* include FBlock Table of Notification Service  */
#endif


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*  Initialisation  -  Section */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfInit()                                                  */
/* Description : Init the Notification Service                              */
/* Parameter(s): none                                                       */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_0
void NtfInit(void)
{
    byte i;

    for (i=(byte)0;i<NTF_SIZE_DEVICE_TAB;i++)
    {
        NtfDeviceTab[i]    = (word)0x0000;  /* Init Device Table */
        NtfDeviceTabCnt[i] = (word)0x0000;  /* Init Counter Array */
    }
    NtfDeviceTabWriteIndex = (byte)0;

    NtfInitAllMatrices();                   /* Complete initialisation of all matrices */

    #ifdef NTF_6
    NtfNumElements = NtfCalcNum();          /* Calculate number of all matrix elements */
                                            /* This is neccessary only, if service will not be polled  */
    NtfLoopCounter = (word)0;
    #endif

    NtfFBlockPtr = NtfFBlockTab;            /* Init pointer at table containing all FBlocks to service */

    NtfLoadFBlockDescription();             /* load first FBlock description */
    NtfLoadPropDescription();               /* load first Property description of first FBlock */
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfCalcNum()                                               */
/* Description : This function calculates the number of all possible        */
/*               matrix elements which must be serviced by this service.    */
/*               It is only required, if the service is not polled          */
/*               cyclically.                                                */
/* Parameter(s): none                                                       */
/* Returns     : Number of Matrix Elements                                  */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_6
word NtfCalcNum(void)
{
    word num_elements;
    byte i;

    num_elements = (word)0;

    NtfFBlockPtr = NtfFBlockTab;

    for (i=(byte)0;i<NTF_MAX_FBLOCKS;i++)
    {
        NtfLoadFBlockDescription();

        while(NtfPropTabPtr->Type_FuncID != NTF_TERMINATION)
        {
            num_elements += (word)NtfNumDev;
            NtfPropTabPtr++;
        }

        NtfFBlockPtr++;
    }

    return(num_elements);
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfInitSingleMatrix()    [API function]                    */
/* Description : Initialize the NTF Matrix of one certain FBlock.           */
/* Parameter(s): FBlockID and InstID of respective FBlock                   */
/*                                                                          */
/* Returns     : 0x00: succeeded                                            */
/*               0x01: matrix not initialized, due to wrong index           */
/*               0x02: function not supported                               */
/*               0x03: NetInterface is not is state MNS_NET_ON              */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_7
byte NtfInitSingleMatrix(byte fblock_id, byte inst_id)
{
    byte fblock_index;      /* index in table of Command Interpreter */
    byte ntf_fblock_index;  /* index in table of Notification Service */


    if (MostGetState() != MNS_NET_ON)   /* Re-initialization only allowed in state MNS_NET_ON */
        return(0x03);


    fblock_index = NbGetFBlockIndex( fblock_id, inst_id);   /* get index of entry in table of Command Interpreter */
    if ((byte)0xFF == fblock_index)
        return((byte)0x01);


    /* Convert 'fblock_index' (regarding T_FBlock.tab) to 'ntf_fblock_index' (regarding T_Notify.tab): */

    ntf_fblock_index = NtfConvertFBlockIndex(fblock_index);

    if (ntf_fblock_index != (byte)0xFF)   /* entry found */
    {
        return( NtfInitOneMatrix(ntf_fblock_index, MNS_FALSE) );
    }
    else
    {
        return((byte)0x01);   /* FBlock not registered in Notification Service      */
    }
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfInitAllMatrices()     [no API function]                 */
/* Description : Initialize the NTF Matrices of all FBlocks.                */
/*               Must only be called when complete NTFS is initialized,     */
/*               that means only when NtfInit() is executed.                */
/*                                                                          */
/* Parameter(s): none                                                       */
/* Returns     : nothing                                                    */
/*--------------------------------------------------------------------------*/
#ifdef NTF_8
void NtfInitAllMatrices(void)
{
    byte i;

    for (i=(byte)0;i<NTF_MAX_FBLOCKS;i++)      /* Init NtfMatrix of all FBlocks */
    {
        (void)NtfInitOneMatrix(i, MNS_TRUE);                /* first or complete initialization          */
    }
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfInitOneMatrix()   [no API function]                     */
/* Description : Initialize the NTF Matrix of one certain FBlock.           */
/* Parameter(s): ntf_fblock_index: Index of entry of the FBlock table of    */
/*               the Notification Service.                                  */
/*               init_mode: MNS_FALSE: re-initialization of a single matrix */
/*                          MNS_TRUE:  first or complete initialization     */
/*                                                                          */
/* Returns     : 0x00: succeeded                                            */
/*               0x01: matrix not initialized, due to wrong index           */
/*               0x02: init_mode not supported                              */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_9
byte NtfInitOneMatrix(byte ntf_fblock_index, bool init_mode)
{
    pTNtfFBlockL ptrFBlockTab;  /* pointer at FBlock table */
    byte i;
    byte num_lines;             /* number of lines (devices) per column (property) */
    pTNtfPropL ptrPropTab;      /* pointer at property table */
    pTNtfMatrix ptrMatrix;      /* pointer at notification matrix (contains flags and device index)          */
    TNtfMatrix flags_device_index;
    byte device_index;


    if (ntf_fblock_index < NTF_MAX_FBLOCKS)      /* valid index ?  (0 = first entry in the table) */
    {
        ptrFBlockTab = &NtfFBlockTab[ntf_fblock_index];     /* load ptr at FBlock table */
        num_lines   = (byte)(ptrFBlockTab->NumDev + 1);             /* +1 due to the flag field */
        ptrPropTab  = ptrFBlockTab->PtrPropTab;             /* load pointer at property table */


        if (MNS_FALSE != init_mode)  /* as part of first or complete initialization : */
        {
            while(ptrPropTab->Type_FuncID != NTF_TERMINATION)   /* loop for all properties of this FBlock */
            {
                ptrMatrix = ptrPropTab->PtrNtfMatrixCol;                /* load base ptr at column (one property) */

                for (i=(byte)0; i<num_lines; i++)                             /* number of lines per property */
                {
                    *ptrMatrix++ = (TNtfMatrix)NTF_DEV_INDEX_FREE;      /* clear entry in matrix */
                }

                ptrPropTab++;                                           /* next property */
            }
        }
        else    /* re-initialization of a single matrix */
        {
            #if 0
            return(0x02);   /*  re-initialization not supported */
            #endif

            while(ptrPropTab->Type_FuncID != NTF_TERMINATION)   /* loop for all properties of this FBlock */
            {
                ptrMatrix = ptrPropTab->PtrNtfMatrixCol;                /* load base ptr at column (one property) */

                *ptrMatrix++ = (TNtfMatrix)NTF_DEV_INDEX_FREE;          /* clear flagfield (first line) in the matrix */

                for (i=(byte)1; i<num_lines; i++)                             /* number of lines per property */
                {
                    flags_device_index = *ptrMatrix;
                    if (flags_device_index != NTF_DEV_INDEX_FREE)       /* entry already in use */
                    {
                        device_index = (byte)(flags_device_index & MASK_DEV_INDEX);

                        *ptrMatrix = NTF_DEV_INDEX_FREE;        /* clear entry */

                        if ((word)0 < NtfDeviceTabCnt[device_index])
                        {
                            NtfDeviceTabCnt[device_index]--;                /* decrement respective counter of Device Index Table */
                        }

                    }
                    ptrMatrix++;
                }

                ptrPropTab++;                                           /* next property */
            }
        }

        return(0x00);   /* function call succeeded */
    }
    else
    {
        return(0x01);   /* function call failed due to wrong index */
    }
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfConvertFBlockIndex()                                    */
/* Description : Convert the index of entry in the table of the Command     */
/*               Interpreter to the index regarding the table of the        */
/*               Notification Service.                                      */
/* Parameter(s): index in table of Command Interpreter                      */
/*                                                                          */
/* Returns     : == 0xFF:  FBlock not found                                 */
/*               <> 0xFF:  Index of entry in the table of the NTFS          */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_60
byte NtfConvertFBlockIndex(byte fblock_index)
{
    byte ntf_fblock_index;  /* index in table of Notification Service */

    for (ntf_fblock_index=(byte)0; ntf_fblock_index<NTF_MAX_FBLOCKS; ntf_fblock_index++)
    {
        if (fblock_index == NtfFBlockTab[ntf_fblock_index].FBlockIndex)
        {
            return( ntf_fblock_index );
        }
    }

    return (0xFF);  /* FBlock not registered in the Notification Service  */
}
#endif




/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*  Notification Service  -  Section */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfService()                                               */
/* Description : Notification Service Trigger Function                      */
/* Parameter(s): none                                                       */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_1
void NtfService(void)
{
    pTMsgTx     tx_ptr;             /* pointer at tx message entry */

    TNtfMatrix  flags_device_index; /* LineFlags and DeviceIndex */
    TNtfMatrix  new_flags_device_index; /* updated value of flags_device_index to avoid multi-thread-conflicts */
    byte        device_index;
    pTNtfMatrix device_index_ptr;   /* pointer at device index table (notification matrix) */

    byte        notify_required;    /* 0x00: No Notification */
                                    /* 0x01: Status Notification */
                                    /* 0x02: Error Notification */
                                    /* 0x03: Ext1 Notification */

    #ifdef NTF_ADD5
    bool        exit_loop = MNS_FALSE;
    #endif

    /*------------------------------*/


    #ifdef NTF_ADD5
    /* loop until notification is found or complete table is looked through */
    while ((0 < NtfLoopCounter) && (MNS_FALSE == exit_loop))
    #else
    tx_ptr = MsgGetTxPtr();

    if (tx_ptr)                     /* tx message entry free ? */
    #endif
    {
        notify_required = (byte)0x00;                                   /* init flag */

        device_index_ptr = NtfMatrixColPtr + (TNtfMatrix)NtfIndexNext;  /* prepare pointer at device index table */

        flags_device_index = *device_index_ptr;                         /* read flags and device index */

        device_index = (byte)(flags_device_index & MASK_DEV_INDEX);

        #ifndef NTF_ADD5
        #ifdef AMS_TX_ADD9
        tx_ptr->MidLevelRetries = DEF_MID_LEVEL_RETRIES_INT_PROC;       /* set number of mid level retries */
        #endif
        #endif


        if ( device_index != (byte)NTF_DEV_INDEX_FREE)
        {                                                               /* Entry in Device Index Table available ? */

            #ifdef NTF_EXT
            if (NtfPropertyExt1Ptr && (flags_device_index & MASK_FLAG_EXT1))                /* Flag EXT1 set ?                       */
            {
                notify_required = (byte)0x03;
            }
            else
            #endif

            #ifdef NTF_EXT
            if ( !(NtfPropFlags & MASK_FLAG_ERROR) )                                        /* Flag ERROR (inverted!) set ?          */
            {
                if (flags_device_index & MASK_FLAG_NTF)                                     /* Flag NTF set ? */
                    notify_required = (byte)0x02;
            }
            else
            #endif
            {
                if (flags_device_index & MASK_FLAG_NTF)             /* has NTF flag been set (announced event)  */
                {
                    notify_required = (byte)0x01;
                }
            }

        } /* if ( device_index != (byte)NTF_DEV_INDEX_FREE) */

        if (notify_required)
        {
            #ifdef NTF_ADD5
            /* if notification is required obtain tx pointer */
            tx_ptr = MsgGetTxPtr();

            if (tx_ptr)
            {
                #ifdef AMS_TX_ADD9
                tx_ptr->MidLevelRetries = DEF_MID_LEVEL_RETRIES_INT_PROC;       /* set number of mid level retries */
                #endif
            #endif

                #ifdef NTF_DEBUG_1                                      /* if debug mode: */
                tx_ptr->Tgt_Adr = (word)0x0100;                         /* send all messages to same device */
                #else                                                   /* and put target address into data field */
                tx_ptr->Tgt_Adr = NtfDeviceTab[device_index];
                #endif

                #ifdef AMS_TX_ADD9
                if (0x0300 == (tx_ptr->Tgt_Adr & 0xFF00))               /* set number of mid level retries to 0 */
                {                                                       /* in case of Group- or Broadcast message */
                    tx_ptr->MidLevelRetries = (byte)0;
                }
                #endif

                #if (NUM_FBLOCKS>0)
                tx_ptr->FBlock_ID   = NetBlock.pFBlockIDs.FBlockID[NtfFBlockIndex];
                tx_ptr->Inst_ID     = NetBlock.pFBlockIDs.InstID[NtfFBlockIndex];
                #endif

                tx_ptr->Func_ID     = NtfFuncID;

                switch (notify_required)
                {
                    case 0x01:

                        new_flags_device_index = *device_index_ptr; /* read content of matrix field again, since flags could be modified by another process */

                        if (new_flags_device_index & MASK_FLAG_SINGLE)
                        {
                            *device_index_ptr = NTF_DEV_INDEX_FREE;                     /* clear entry in device index table */
                            if ((word)0 < NtfDeviceTabCnt[device_index])
                            {
                                NtfDeviceTabCnt[device_index]--;                        /* decrement respective Device Table counter */
                            }
                        }
                        else
                        {
                            *device_index_ptr = (TNtfMatrix)(new_flags_device_index & ~MASK_FLAG_NTF);  /* clear flag NTF */
                        }

                        tx_ptr->Operation   = OP_STATUS;

                        switch (NtfType & MASK_TYPE)
                        {
                            case NTF_TYPE_BOOL:

                                #ifdef AMS_TX_ADD9
                                tx_ptr->MidLevelRetries = (byte)0;                  /* set number of mid level retries to 0 for messages completely generated by NTFS*/
                                #endif

                                #ifdef NTF_DEBUG_1
                                CmdEncodeWord(&tx_ptr->Data[0],&NtfDeviceTab[device_index]);
                                tx_ptr->Data[2] = (byte)(*(bool*)NtfPropertyPtr);   /* prepare data field */
                                tx_ptr->Length = (word)3;                           /* prepare data length */
                                #else
                                tx_ptr->Data[0] = (byte)(*(bool*)NtfPropertyPtr);   /* prepare data field */
                                tx_ptr->Length = (word)1;                           /* prepare data length */
                                #endif
                                break;

                            case NTF_TYPE_BYTE:

                                #ifdef AMS_TX_ADD9
                                tx_ptr->MidLevelRetries = (byte)0;                  /* set number of mid level retries to 0 for messages completely generated by NTFS*/
                                #endif

                                #ifdef NTF_DEBUG_1
                                CmdEncodeWord(&tx_ptr->Data[0],&NtfDeviceTab[device_index]);
                                tx_ptr->Data[2] = *(byte*)NtfPropertyPtr;
                                tx_ptr->Length  = (word)3;
                                #else
                                tx_ptr->Data[0] = *(byte*)NtfPropertyPtr;
                                tx_ptr->Length  = (word)1;
                                #endif
                                break;

                            case NTF_TYPE_WORD:

                                #ifdef AMS_TX_ADD9
                                tx_ptr->MidLevelRetries = (byte)0;                      /* set number of mid level retries to 0 for messages completely generated by NTFS*/
                                #endif

                                #ifdef NTF_DEBUG_1
                                CmdEncodeWord(&tx_ptr->Data[0],&NtfDeviceTab[device_index]);
                                CmdEncodeWord(&tx_ptr->Data[2],(word*)NtfPropertyPtr);
                                tx_ptr->Length = (word)4;
                                #else
                                CmdEncodeWord(&tx_ptr->Data[0],(word*)NtfPropertyPtr);
                                tx_ptr->Length = (word)2;
                                #endif
                                break;

                            case NTF_TYPE_LONG:

                                #ifdef AMS_TX_ADD9
                                tx_ptr->MidLevelRetries = (byte)0;                      /* set number of mid level retries to 0 for messages completely generated by NTFS*/
                                #endif

                                #ifdef NTF_DEBUG_1
                                CmdEncodeWord(&tx_ptr->Data[0],&NtfDeviceTab[device_index]);
                                CmdEncodeLong(&tx_ptr->Data[2],(dword*)NtfPropertyPtr);
                                tx_ptr->Length = (word)6;
                                #else
                                CmdEncodeLong(&tx_ptr->Data[0],(dword*)NtfPropertyPtr);
                                tx_ptr->Length = (word)4;
                                #endif
                                break;

                            case NTF_TYPE_IND:                                      /* callback function with TX Ptr only */

                                #ifdef PTR_UNION_VOID
                                tx_ptr->Operation = ( *(byte(*)(pTMsgTx )) NtfPropTabPtr->Prop.FPtr ) (tx_ptr);
                                #else
                                tx_ptr->Operation = ( *(byte(*)(pTMsgTx )) NtfPropertyPtr ) (tx_ptr);
                                #endif
                                break;                                          /* function pointer is stored in NtfPropertyPtr */


                            case NTF_TYPE_INDTXRX:                                 /* callback function with TX Ptr (<> NULL) and RX Ptr (=NULL) */

                                #ifdef PTR_UNION_VOID
                                tx_ptr->Operation = ( *(byte(*)(pTMsgTx, pTMsgRx)) NtfPropTabPtr->Prop.FPtr ) (tx_ptr, NULL);
                                #else
                                tx_ptr->Operation = ( *(byte(*)(pTMsgTx, pTMsgRx)) NtfPropertyPtr ) (tx_ptr, NULL);
                                #endif
                                break;                                          /* function pointer is stored in NtfPropertyPtr */
                        }
                        break;

                    #ifdef NTF_EXT
                    case 0x02:

                        new_flags_device_index = *device_index_ptr; /* read content of matrix field again  */
                        *device_index_ptr = (TNtfMatrix)(new_flags_device_index & ~MASK_FLAG_NTF);        /* clear flag NTF */

                        tx_ptr->Operation   = OP_ERROR;

                        #ifdef AMS_TX_ADD9
                        tx_ptr->MidLevelRetries = (byte)0;                      /* set number of mid level retries to 0 for messages completely generated by NTFS*/
                        #endif

                        #ifdef NTF_DEBUG_1
                        CmdEncodeWord(&tx_ptr->Data[0],&NtfDeviceTab[device_index]);
                        tx_ptr->Data[2] = ERR_NOTAVAILABLE;
                        tx_ptr->Length  = (word)3;
                        #else
                        CmdErrorMsg(tx_ptr, ERR_NOTAVAILABLE);
                        #endif
                        break;
                    #endif

                    #ifdef NTF_EXT
                    case 0x03:

                        new_flags_device_index = *device_index_ptr; /* read content of matrix field again, since flags could be modified by another process */
                        *device_index_ptr = (TNtfMatrix)(new_flags_device_index & ~MASK_FLAG_EXT1);

                        switch (NtfType & MASK_TYPE_EXT)
                        {
                            case NTF_TYPE_EXT_FUNC:
                                #ifdef PTR_UNION_VOID
                                tx_ptr->Operation = ( *(byte(*)(pTMsgTx )) NtfPropTabPtr->PropExt1.FPtr ) (tx_ptr);
                                #else
                                tx_ptr->Operation = ( *(byte(*)(pTMsgTx )) NtfPropertyExt1Ptr ) (tx_ptr);
                                #endif
                                break;

                            case NTF_TYPE_EXT_DATA:
                                #ifdef AMS_TX_ADD9
                                tx_ptr->MidLevelRetries = (byte)0;                      /* set number of mid level retries to 0 for messages completely generated by NTFS*/
                                #endif

                                CmdEncodeStrRam(tx_ptr,(byte*)NtfPropertyExt1Ptr);
                                tx_ptr->Operation   = NTF_EXT_OPTYPE;
                                break;
                        }

                        #ifdef NTF_DEBUG_1
                        CmdEncodeWord(&tx_ptr->Data[0],&NtfDeviceTab[device_index]);    /* Overwrite Data[0]..[1] with Target Address */
                        #endif
                        break;
                    #endif
                }


                if (tx_ptr->Operation >= OP_REPORTS)        /* valid notification ? */
                {
                    AmsMsgSend(tx_ptr);
                    #ifdef NTF_ADD5
                    exit_loop = MNS_TRUE;
                    #endif
                }
                else
                {
                    MsgTxUnused(tx_ptr);
                }

                #ifdef NTF_ADD5
                NtfIndexNext++;
                if (NtfIndexNext > NtfNumDev)                 /* last entry of device index table ?  */
                {
                    NtfIncPropTabPtr();                         /*       next property, first device */
                }

                #ifdef MNS2_OPT_1
                if (NtfLoopCounter)                             /* Decrement Loop Counter, since one element was serviced */
                {
                    NtfLoopCounter--;
                }
                #endif
                #endif /* #ifdef NTF_ADD5 */

            } /* if (notify_required) */
        #ifndef NTF_ADD5
            else
            {
                MsgTxUnused(tx_ptr);                        /* release message entry                         */
            }
        #else /*#ifndef NTF_ADD5*/
            else    /* else to "if (tx_ptr)" */
            {
                exit_loop = MNS_TRUE;   /* this causes loop exit and hence buffer transmission/release instead of infinite spin  */
            }
        }   /* if notify required */
        else
        {
        /* if no notify required then move on */
        #endif /*#ifndef NTF_ADD5*/

            NtfIndexNext++;
            if (NtfIndexNext > NtfNumDev)                 /* last entry of device index table ?  */
            {
                NtfIncPropTabPtr();                         /*       next property, first device */
            }                                               /* else: same property, next device */

            #ifdef MNS2_OPT_1
            if (NtfLoopCounter)                             /* Decrement Loop Counter, since one element was serviced */
            {
                NtfLoopCounter--;
            }
            #endif

        #ifdef NTF_ADD5
        }
        #endif

    } /* if (tx_ptr) */

    #ifdef MNS2_OPT_1
    if (NtfLoopCounter)
    {
        MNS2_REQUEST_SET(MNS2_P_NTFS)                   /* Set Request Flag, if not all elements has been serviced */
    }
    #endif
}
#endif




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfLoadFBlockDescription()                                 */
/* Description : Load description of respective FBlock                      */
/* Parameter(s): none                                                       */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_2
void NtfLoadFBlockDescription(void)
{
    NtfFBlockIndex = NtfFBlockPtr->FBlockIndex;     /* read local fblock index */
    NtfNumDev      = NtfFBlockPtr->NumDev;          /* read number of devices of this device table (column of notif.matrix) */
    NtfPropTabPtr  = NtfFBlockPtr->PtrPropTab;      /* Load base address of property table */
}
#endif




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfLoadPropDescription()                                   */
/* Description : Load description of respective property                    */
/* Parameter(s): none                                                       */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_3
void NtfLoadPropDescription(void)
{
    word type_funcid;

    type_funcid = NtfPropTabPtr->Type_FuncID;           /* read Type and FuncID of current property */
    NtfType = (byte)(type_funcid >> 12);                /* prepare Type */
    NtfFuncID = (word)(type_funcid & 0x0FFF);           /* prepare FuncID */

    NtfMatrixColPtr =  NtfPropTabPtr->PtrNtfMatrixCol;  /* read base pointer on notification matrix column */

    #ifdef PTR_UNION_VOID
    NtfPropertyPtr    = NtfPropTabPtr->Prop.DPtr;       /* read data ptr at property             */
    #else
    NtfPropertyPtr    = NtfPropTabPtr->PtrProperty;     /* read ptr (function ptr or data ptr)           */
    #endif


    #ifdef NTF_EXT
    #ifdef PTR_UNION_VOID
    NtfPropertyExt1Ptr = NtfPropTabPtr->PropExt1.DPtr;  /* read data ptr for extended use                                        */
    #else
    NtfPropertyExt1Ptr = NtfPropTabPtr->PtrPropertyExt1;/* read ptr for extended use (function ptr or data ptr)                  */
    #endif
    #endif

    NtfIndexNext = (byte)1;                             /* Init index for device index table (column of notification matrix) */
                                                        /* But forget about Flag field (first entry, index=0)                    */

    NtfPropFlags = *NtfMatrixColPtr;                    /* read property flags (ColumnFlags, first entry in Column)              */
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfIncFBlockPtr()                                          */
/* Description : Increment NtfFBlockPtr modulo table size                   */
/* Parameter(s): none                                                       */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_4
void NtfIncFBlockPtr(void)
{
    if (NtfFBlockPtr == &NtfFBlockTab[NTF_MAX_FBLOCKS-1] )     /* inc pointer modulo table size */
        NtfFBlockPtr =  &NtfFBlockTab[0];
    else
        NtfFBlockPtr++;

    NtfLoadFBlockDescription();                                 /* load description of next FBlock */
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfIncPropTabPtr()                                         */
/* Description : Increment NtfPropTabPtr modulo table size                  */
/*               Load NtfRomPtr and NtfRamPtr                               */
/* Parameter(s): none                                                       */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_5
void NtfIncPropTabPtr(void)
{
    NtfPropTabPtr++;                                            /* increment pointer at property table */

    if (NTF_TERMINATION == NtfPropTabPtr->Type_FuncID)          /* termination of property table reached ? */
        NtfIncFBlockPtr();                                      /* next FBlock */

    NtfLoadPropDescription();                                   /* load description of next property */
}
#endif







/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*  Notification Matrix  -  Section */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfSetNotificationMatrix()                                 */
/* Description : Set Notification Matrix of respective FBlock               */
/* Parameter(s): ptr at tx msg, ptr at rx msg                               */
/* Returns     : op_type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_10
byte NtfSetNotificationMatrix(pTMsgTx tx_ptr, pTMsgRx rx_ptr)
{
    word rx_length;
    byte fblock_error;
    byte control;
    word device_id;
    byte device_index = 0;

    byte i;

    byte num_dev;
    pTNtfPropL prop_tab_ptr, prop_tab_baseptr;
    pTNtfMatrix device_index_tab_baseptr;

    word func_id;
    word func_id_tab;

    byte* error_msg_ptr;
    byte num_func_failed;


    rx_length = rx_ptr->Length;

    /*-------------- Check Length ----------------------------- */
    #ifndef CMD_ADD8
    if ( rx_length < 0x03)
        return( CmdErrorMsg(tx_ptr, ERR_LENGTH) );   /* return(OP_ERROR) */
    #endif


    /*-------------- Load FBlock Description ------------------- */

    fblock_error = NtfLoadFBlockDescriptionRx(rx_ptr,&num_dev,&prop_tab_baseptr);       /* load max. number of device_index entries */
                                                                                        /* and base pointer at property table */

    if (fblock_error)
        return( CmdErrorByte(tx_ptr, ERR_FUNC_SPECIFIC, NTF_ERR_FBLOCK_NOTAVAIL) );
                                                                /* Error: FBlockID.InstID not registered */
                                                                /*        in Notification Service ! */
                                                                /*        --> return(OP_ERROR) */


    /*-------------- Read Parameters --------------------------- */

    control = rx_ptr->Data[0];                              /* get param: Control */
    CmdDecodeWord(&device_id,&rx_ptr->Data[1]);             /* get param: DeviceID */



    /*-------------- Get Device Index -------------------------- */

    if (control <= NTF_SET_FUNC )                           /* set all or set func */
    {
        device_index = NtfWriteDeviceTab(device_id);        /* check if already existing and write new entry if necessary */

        if ((byte)0xFF == device_index)                           /* device table overflow ? */
            return( CmdErrorByte(tx_ptr, ERR_FUNC_SPECIFIC, NTF_ERR_DEVTABLE_OVERFLOW) ); /* Error: Device Table Overflow */
                                                                                        /*        --> return(OP_ERROR) */
    }
    else if (control <= NTF_CLR_FUNC)                        /* clr all or clr func */
    {
        device_index = NtfScanDeviceTab(device_id);
        if ((byte)0xFF == device_index)                      /* address not found ? */
        {
            return (OP_NO_REPORT);
        }
        else if ((word)0 == NtfDeviceTabCnt[device_index])        /* address found, but not used in any Notification Matrix */
        {
            return (OP_NO_REPORT);
        }
    }

    /*---------------------------------------------------------- */


    error_msg_ptr   = &tx_ptr->Data[2];                         /* init pointer to data field storing failed func ids    */
    num_func_failed = (byte)0;                                  /* number of failed func ids */


    switch (control)
    {
        /*----------------------------------------- */
        case NTF_SET_ALL:
        /*----------------------------------------- */

            prop_tab_ptr = prop_tab_baseptr;                            /* set pointer at first Property Table entry */

            do
            {
                func_id_tab = (word)(prop_tab_ptr->Type_FuncID & 0x0FFF);/* read FunctionID in Property Table */
                if (func_id_tab != (word)PROP_TERMINATION)
                {
                    if (func_id_tab < 0xF00)  /* ignore supplier specific functions */
                    {
                        device_index_tab_baseptr = prop_tab_ptr->PtrNtfMatrixCol;       /* read base pointer at device index table */
                        /*lint -e644 the variable is initialized! Otherwise the switch selects default case! */
                        if ( (byte)2 == NtfWriteDeviceIndex(device_index_tab_baseptr+1, num_dev, device_index) )
                        {
                            NtfErrorAppendFuncID(&num_func_failed, &error_msg_ptr, func_id_tab);/* Error: Device Index Table of */
                                                                                                /* respective property overflow */
                        }
                    }

                    prop_tab_ptr++;                                     /* next property */
                }

            }while (func_id_tab != (word)PROP_TERMINATION);


            if (num_func_failed)                                            /* at least one entry failed */
            {                                                               /* Error: Device Index Table of */
                                                                            /* respective property overflow */
                return ( NtfErrorComplFuncID(num_func_failed, tx_ptr) );    /* --> return(OP_ERROR) */
            }
            break;


        /*----------------------------------------- */
        case NTF_SET_FUNC:
        /*----------------------------------------- */

            for ( i=(byte)3; i<(byte)rx_length; i+=(byte)2 )
            {
                CmdDecodeWord(&func_id,&rx_ptr->Data[i]);                   /* Read Func ID of property to service */
                prop_tab_ptr = NtfGetPropTabPtr(prop_tab_baseptr, func_id);  /* Get pointer at corresponding line of */
                                                                            /* the Property Table  */
                if (prop_tab_ptr != NULL)
                {
                    device_index_tab_baseptr = prop_tab_ptr->PtrNtfMatrixCol;/* read base pointer at device index table */

                    if ( (byte)2 == NtfWriteDeviceIndex(device_index_tab_baseptr+1, num_dev, device_index) )
                    {
                        NtfErrorAppendFuncID(&num_func_failed, &error_msg_ptr, func_id);    /* Error: Device Index Table of   */
                    }                                                                       /* respective property overflow */
                }
                else
                {
                        NtfErrorAppendFuncID(&num_func_failed, &error_msg_ptr, func_id);    /* Error: Property not registered */
                }                                                                           /* in Notification Service */
            }

            if (num_func_failed)                                            /* at least one entry failed */
            {
                return ( NtfErrorComplFuncID(num_func_failed, tx_ptr) );    /* --> return(OP_ERROR) */
            }
            break;




        /*----------------------------------------- */
        case NTF_CLR_ALL:
        /*----------------------------------------- */

            prop_tab_ptr = prop_tab_baseptr;
            do
            {
                func_id_tab = (word)(prop_tab_ptr->Type_FuncID & 0x0FFF);       /* read FunctionID in Property Table */
                if (func_id_tab != (word)PROP_TERMINATION)
                {
                    device_index_tab_baseptr = prop_tab_ptr->PtrNtfMatrixCol;   /* read base pointer at device index table */

                    (void)NtfClearDeviceIndex(device_index_tab_baseptr+1, num_dev, device_index);
                    prop_tab_ptr++;                                             /* next property */
                }
            }while (func_id_tab != (word)PROP_TERMINATION);
            break;

        /*----------------------------------------- */
        case NTF_CLR_FUNC:
        /*----------------------------------------- */

            for ( i=(byte)3; i<(byte)rx_length; i+=(byte)2 )
            {
                CmdDecodeWord(&func_id,&rx_ptr->Data[i]);                   /* Read Func ID of property to service */
                prop_tab_ptr = NtfGetPropTabPtr(prop_tab_baseptr, func_id);  /* Get pointer at corresponding line of */
                                                                            /* the Property Table  */

                if (prop_tab_ptr != NULL)                                    /* property registered in Notification Service ? */
                {
                    device_index_tab_baseptr = prop_tab_ptr->PtrNtfMatrixCol;/* read base pointer at device index table */

                    (void)NtfClearDeviceIndex(device_index_tab_baseptr+1, num_dev, device_index);
                }                                                           /* else: no error report ! */
            }
            break;

        /*----------------------------------------- */
        default:
        /*----------------------------------------- */

            return( CmdErrorParamWrong(tx_ptr, (byte)1, &rx_ptr->Data[0], (byte)1) );    /* Error: wrong parameter control */
    }                                                                        /*        --> return(OP_ERROR) */

    return(OP_NO_REPORT);                               /* no error occurred */
}
#endif




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfGetNotificationMatrix()                                 */
/* Description : Get Column of the Notification Matrix of respective FBlock */
/* Parameter(s): ptr at tx msg, ptr at rx msg                               */
/* Returns     : op_type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_11
byte NtfGetNotificationMatrix(pTMsgTx tx_ptr, pTMsgRx rx_ptr)
{
    byte        fblock_error;
    byte        num_dev;
    pTNtfPropL  prop_tab_ptr;
    pTNtfPropL  prop_tab_baseptr;
    word        func_id;
    pTNtfMatrix device_index_tab_ptr;
    word        tx_length;
    byte        i;
    TNtfMatrix  device_index_flags;
    byte        device_index;
    byte*       tx_data_ptr;
    word        device_addr;



    /*-------------- Check Length ----------------------------- */
    #ifndef CMD_ADD8
    if ( rx_ptr->Length != (word)0x02)
        return( CmdErrorMsg(tx_ptr, ERR_LENGTH) );               /* return(OP_ERROR) */
    #endif


    /*-------------- Load FBlock Description ------------------- */

    fblock_error = NtfLoadFBlockDescriptionRx(rx_ptr,&num_dev,&prop_tab_baseptr);

    if (fblock_error)
        return( CmdErrorByte(tx_ptr, ERR_FUNC_SPECIFIC, NTF_ERR_FBLOCK_NOTAVAIL) );
                                                               /* Error: FBlockID.InstID not registered */
                                                               /*        in Notification Service ! */
                                                               /*        --> return(OP_ERROR) */


    /*-------------- Get Parameter (FuncID) -------------------- */

    CmdDecodeWord(&func_id,&rx_ptr->Data[0]);                   /* read parameter (Function ID) */


    /*---------------------------------------------------------- */


    prop_tab_ptr = NtfGetPropTabPtr(prop_tab_baseptr, func_id); /* get pointer at corresponding line of the property table */

    if (prop_tab_ptr != NULL)
    {
        device_index_tab_ptr = prop_tab_ptr->PtrNtfMatrixCol+1; /* set pointer at first entry in device index table              */
                                                                /* (column of notification matrix) */

        CmdEncodeWord(&tx_ptr->Data[0],&func_id);               /* first parameter: Function ID  */
        tx_length   = (word)2;                                  /* init length of tx message */
        tx_data_ptr = &tx_ptr->Data[2];                         /* init pointer at parameter field containing device ids */

        for (i=(byte)0;i<num_dev;i++)                           /* scan whole column */
        {
            device_index_flags = *device_index_tab_ptr++;       /* get device index and flags from table and increment  */
                                                                /* pointer at index table */
            device_index = (byte)(device_index_flags & MASK_DEV_INDEX);
            device_addr  = NtfDeviceTab[device_index];

            if ( (device_index != (byte)NTF_DEV_INDEX_FREE) && !(device_index_flags & MASK_FLAG_SINGLE) )
            {                                                   /* valid device index ? But forget about single notification */
                                                                /* entries */
                CmdEncodeWord(tx_data_ptr,&device_addr);        /* copy device id to tx message */
                tx_data_ptr += 2;                               /* set pointer to next parameter (device id) */
                tx_length   = (word)(tx_length+(word)2);        /* increment length of data to transmit */
            }
        }

        tx_ptr->Length = tx_length;                             /* set length of data to transmit */
    }
    else
    {
     /* return( CmdErrorByte(tx_ptr, ERR_FUNC_SPECIFIC, 0x04) );   */
        return( CmdErrorParamNotAvailable( tx_ptr, (byte)1, &rx_ptr->Data[0], (byte)2) );
                                                                /* Error: Property not registered */
                                                                /*        in Notification Service ! */
                                                                /*        --> return(OP_ERROR) */
    }

    return (OP_STATUS);
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfReadMatrixCol()                                         */
/* Description : Get Column of the Notification Matrix of respective        */
/*               FBlock.InstID.FuncID                                       */
/*                                                                          */
/* Parameter(s): ptr at matrix column, number of lines (without flagfield,  */
/*               ptr at result buffer                                       */
/*               The result buffer will contain all devices, after          */
/*               returning from this function                               */
/*                                                                          */
/* Returns     : number of devices, which are registered in matrix          */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_13
byte NtfReadMatrixCol(pTNtfMatrix col_ptr, byte num_lines, word* ptr_device_buffer)
{
    TNtfMatrix device_index_flags, device_index;
    byte counter;
    byte i;
    word device_addr;

    counter = (byte)0x00;

    col_ptr++;                                                      /* forget about flag field */

    for (i=(byte)0;i<num_lines;i++)                                 /* scan whole column */
    {
        device_index_flags = *col_ptr++;                            /* get device index and flags from table and increment  */
                                                                    /* pointer at index table */

        device_index = (byte)(device_index_flags & MASK_DEV_INDEX);
        device_addr  = NtfDeviceTab[device_index];                  /* read device id from device table */

        if ( (device_index != (byte)NTF_DEV_INDEX_FREE) && !(device_index_flags & MASK_FLAG_SINGLE) )
        {                                                           /* valid device index ? But forget about single notification */
                                                                    /* entries */
            if (ptr_device_buffer != NULL)
                *ptr_device_buffer++ = device_addr;                 /* Copy Device Address into result buffer if requested */

            counter++;
        }

    }

    return (counter);
}
#endif




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfSetSingleNotification()                                 */
/* Description : Set entry in the device index table for a single           */
/*               notification after scanning the device index table for     */
/*               already existing device index                              */
/* Parameter(s): ptr at device index table, size of index table, device id  */
/* Returns     : status                                                     */
/*                      0x00: no error occurred                             */
/*                      0x01: device table overflow                         */
/*                      0x02: device index table overflow                   */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_12
byte NtfSetSingleNotification(pTNtfMatrix device_index_tab_ptr, byte num_entries, word device_id)
{
    byte scan_status;
    byte device_index;

    device_index = NtfWriteDeviceTab(device_id);
    if ((byte)0xFF == device_index)       /* device table overflow ? */
        return ((byte)0x01);

    device_index_tab_ptr++;         /* forget about flag field                                                       */

    scan_status = NtfScanDeviceIndexTab(&device_index_tab_ptr, num_entries, device_index);

    if (scan_status)                /* device index already existing ? */
    {
        return(0);
    }

    scan_status = NtfScanDeviceIndexTab(&device_index_tab_ptr, num_entries, (byte)NTF_DEV_INDEX_FREE);

    if ((byte)0 == scan_status)               /* device index table overflow ? */
    {
        return(2);
    }

    *device_index_tab_ptr = (TNtfMatrix)(device_index | MASK_FLAG_SINGLE);      /* enter single notification entry */


    NtfDeviceTabCnt[device_index]++;                                            /* increment respective Device Table counter */

    return(0);
}
#endif






/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*  Device Index Table  -  Section */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfWriteDeviceIndex()                                      */
/* Description : write a certain device index into the notification device  */
/*               index table of a respective property, after scanning the   */
/*               index table for already existing device index              */
/* Parameter(s): base pointer at device index table,                        */
/*               number of table entries,                                   */
/*               device_index                                               */
/*                                                                          */
/* Returns     : status:                                                    */
/*                          0: device index written                         */
/*                          1: device index already existing                */
/*                          2: no free table entry                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_20
byte NtfWriteDeviceIndex(pTNtfMatrix device_index_tab_ptr, byte num_entries, byte device_index)
{
    byte scan_status;
    TNtfMatrix flags_device_index;


    scan_status = NtfScanDeviceIndexTab(&device_index_tab_ptr, num_entries, device_index);

    if (scan_status)                                            /* device index already existing ? */
    {
        flags_device_index = *device_index_tab_ptr;                 /* get existing entry */

        #ifdef NTF_SINGLE_PREV
        flags_device_index &= (TNtfMatrix)~MASK_FLAG_SINGLE;        /* clear flag of 'Single Notification' */
        #endif

        *device_index_tab_ptr = flags_device_index | MASK_FLAG_NTF; /* set flag NTF at existing entry */

        #ifdef MNS2_OPT_1
        NtfLoopCounter = NtfNumElements;                        /* Load counter to indicate demand for re-trigger */
        #endif



        #ifdef MNS2_OPT_1
        MNS2_REQUEST_CALL(MNS2_P_NTFS)                          /* Set Request Flag and call application */
        #endif

        return((byte)1);
    }

    scan_status = NtfScanDeviceIndexTab(&device_index_tab_ptr, num_entries, NTF_DEV_INDEX_FREE);

    if ((byte)0 == scan_status)                                           /* no free table entry ? */
    {
        return((byte)2);
    }

    *device_index_tab_ptr = (TNtfMatrix)(device_index | MASK_FLAG_NTF);     /* write device_index into first free table entry, since  */
                                                                            /* device_index not already existing */
                                                                            /* and set flag NTF */

    #ifdef MNS2_OPT_1
    NtfLoopCounter = NtfNumElements;                            /* Load counter to indicate demand for re-trigger */
    #endif



    NtfDeviceTabCnt[device_index]++;                            /* increment respective counter of Device Index Table */

    #ifdef MNS2_OPT_1
    MNS2_REQUEST_CALL(MNS2_P_NTFS)                              /* Set Request Flag and call application */
    #endif

    return((byte)0);                                                  /* report success */
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfClearDeviceIndex()                                      */
/* Description : Clear a certain device index in the notification device    */
/*               index table of a respective property after scanning the    */
/*               device index table                                         */
/* Parameter(s): base pointer at device index table,                        */
/*               number of table entries,                                   */
/*               device_index                                               */
/*                                                                          */
/* Returns     : status:                                                    */
/*                          0: device_index cleared                         */
/*                          1: device_index not available                   */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_21
byte NtfClearDeviceIndex(pTNtfMatrix device_index_tab_ptr, byte num_entries, byte device_index)
{
    byte scan_status;

    scan_status = NtfScanDeviceIndexTab(&device_index_tab_ptr, num_entries, device_index);

    #ifdef NTF_SINGLE_PREV
    if ((byte)1 == scan_status)               /* device index available ? (ignore 'Single Notification' !) */
    #else
    if (scan_status)                    /* device index available ? */
    #endif
    {
        *device_index_tab_ptr = NTF_DEV_INDEX_FREE;     /* clear entry */

        if ((word)0 < NtfDeviceTabCnt[device_index])
        {
            NtfDeviceTabCnt[device_index]--;                /* decrement respective counter of Device Index Table */
        }

        return ((byte)0);
    }


    return((byte)1);                          /* device index not available */
}
#endif







/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfScanDeviceIndexTab()                                    */
/* Description : search the notification device index table of a respective */
/*               property for a certain device index                        */
/* Parameter(s): ptr at ptr at device index table (changed by function),    */
/*               number of table entries, device index                      */
/*                                                                          */
/* Returns     : status:                                                    */
/*                          0: device index not found                       */
/*                          1: notification entry found, ptr at device      */
/*                             index table changed                          */
/*                          2: single notification entry found,             */
/*                             ptr at device index table changed            */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_22
byte NtfScanDeviceIndexTab(pTNtfMatrix* device_index_tab_ptrptr, byte num_entries, byte device_index)
{
    byte i;
    pTNtfMatrix device_index_tab_ptr;
    TNtfMatrix device_index_flags;          /* device index and flags */

    device_index_tab_ptr = *device_index_tab_ptrptr;


    for (i=(byte)0;i<num_entries;i++)
    {
        device_index_flags = *device_index_tab_ptr;         /* read device index and flags out of table */

        #ifdef NTF_SINGLE_PREV
        if ( ((device_index_flags & MASK_DEV_INDEX) == (TNtfMatrix)device_index) ) /* valid device index ? */
        #else
        if ( ((device_index_flags & MASK_DEV_INDEX) == (TNtfMatrix)device_index) && !(device_index_flags & MASK_FLAG_SINGLE) )
        #endif
        {
            *device_index_tab_ptrptr = device_index_tab_ptr;

            #ifdef NTF_SINGLE_PREV
            if (device_index_flags & MASK_FLAG_SINGLE)
                return(2);
            #endif

            return((byte)1);
        }

        device_index_tab_ptr++;
    }

    return((byte)0);
}
#endif







/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*  Device Table  -  Section */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfWriteDeviceTab()                                        */
/* Description : set device in device table of notification service         */
/*               and return index after scanning the device table for       */
/*               always existing device id                                  */
/* Parameter(s): device_addr                                                */
/* Returns     : device_index (0xFF: no entry free)                         */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_30
byte NtfWriteDeviceTab(word device_addr)
{
    byte scan_index;
    byte return_value;
    bool search_flag;
    byte search_cnt;

    scan_index = NtfScanDeviceTab(device_addr) ;

    if (scan_index != (byte)0xFF)                       /* device address already existing ? */
        return (scan_index);                            /* return index of Device Table */

    search_flag = MNS_FALSE;
    search_cnt  = NTF_SIZE_DEVICE_TAB;
    return_value = (byte)0xFF;                          /* will be changed if free entry found */

    do
    {
        if  ((word)0 == NtfDeviceTabCnt[NtfDeviceTabWriteIndex])       /* free entry ? */
        {
            search_flag = MNS_TRUE;
            NtfDeviceTab[NtfDeviceTabWriteIndex] = device_addr;  /* set entry in Device Table */
            return_value = NtfDeviceTabWriteIndex;               /* save index as return value */
        }
        else
            search_cnt--;                                        /* decrement loop counter */

        NtfDeviceTabWriteIndex++;                           /* increment index modulo array size */
        if (NTF_SIZE_DEVICE_TAB == NtfDeviceTabWriteIndex)
            NtfDeviceTabWriteIndex = (byte)0x00;

    }while (search_cnt && (MNS_FALSE == search_flag));

    return(return_value);               /* 0xFF: no free entry */
                                        /* other value: respective index of Device Table */
}
#endif






/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfScanDeviceTab()                                         */
/* Description : search the notification device table for a certain         */
/*               device address or group address                            */
/* Parameter(s): device_addr                                                */
/* Returns     : device_index (0xFF: device_addr not found)                 */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_31
byte NtfScanDeviceTab(word device_addr)
{
    byte i;

    for (i=(byte)0;i<NTF_SIZE_DEVICE_TAB;i++)
    {
        if (NtfDeviceTab[i] == device_addr)
            return ((byte)i);
    }
    return ((byte)0xFF);
}
#endif






/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*  Property Changed  -  Section */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfPropertyChangedRx()                                     */
/* Description : Set flag NTF in device index table of the respective       */
/*               property                                                   */
/*               (second level function)                                    */
/* Parameter(s): pointer at received message                                */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_40
void NtfPropertyChangedRx(pTMsgRx rx_ptr)
{
    byte fblock_error;
    byte num_dev;                           /* number of devices in the notification matrix */
    pTNtfPropL   prop_tab_baseptr;          /* base pointer at property table of respective FBlock */

    pTNtfPropL   prop_tab_ptr;              /* Pointer at corresponding line in Ntf Property Table */
    pTNtfMatrix  device_index_tab_baseptr;  /* base pointer at device index table (column of notification matrix) */

    /*---------------------------------------- */

    fblock_error = NtfLoadFBlockDescriptionRx(rx_ptr, &num_dev, &prop_tab_baseptr);

    if ((byte)0 == fblock_error)                                              /* fblock index available ? */
    {
        prop_tab_ptr = NtfGetPropTabPtr(prop_tab_baseptr, rx_ptr->Func_ID);  /* prepare pointer at corresponding property line */

        if (prop_tab_ptr != NULL)                                            /* property available ? */
        {
            device_index_tab_baseptr = prop_tab_ptr->PtrNtfMatrixCol;

            NtfPropertyChanged(device_index_tab_baseptr, num_dev);                   /* call first level function */
        }
    }
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfPropertyChanged()                                       */
/* Description : Set flag NTF in device index table of the respective       */
/*               property                                                   */
/*               (first level function)                                     */
/* Parameter(s): Pointer at device index table, number of devices           */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_41
void NtfPropertyChanged(pTNtfMatrix device_index_tab_ptr, byte num_entries)
{
    #ifdef NTF_EXT
    TNtfMatrix property_flags;

    property_flags = *device_index_tab_ptr;
    *device_index_tab_ptr   = property_flags | MASK_FLAG_ERROR;                         /* clear flag ERROR (inverted!)          */
    #endif

    NtfChangeAllFlags(device_index_tab_ptr, num_entries, MASK_FLAG_NTF);
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfPropertyChangedFkt()                                    */
/* Description : Set flag NTF in device index table of the respective       */
/*               property                                                   */
/*                                                                          */
/* Parameter(s): fblock_id,                                                 */
/*               inst_id,                                                   */
/*               fkt_id,                                                    */
/*                                                                          */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_44
void NtfPropertyChangedFkt(byte fblock_id, byte inst_id, word fkt_id)
{
    byte         fblock_index;

    fblock_index = NbGetFBlockIndex( fblock_id, inst_id);
    if ((byte)0xFF == fblock_index)
        return;

    NtfPropertyChangedIdx(fblock_index, fkt_id);
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfPropertyChangedIdx()                                    */
/* Description : Set flag NTF in device index table of the respective       */
/*               property                                                   */
/*                                                                          */
/* Parameter(s): idx,                                                       */
/*               fkt_id,                                                    */
/*                                                                          */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_45
void NtfPropertyChangedIdx(byte fblock_index, word fkt_id)
{
    byte         num_dev;
    pTNtfFBlockL fblock_tab_ptr;
    pTNtfPropL   prop_tab_ptr, prop_tab_baseptr;
    pTNtfMatrix  device_index_tab_baseptr;


    fblock_tab_ptr = NtfGetFBlockTabPtr(fblock_index);

    if (fblock_tab_ptr != NULL)                          /* fblock index found ? */
    {
        num_dev = fblock_tab_ptr->NumDev;               /* read maximum count of devices to service          */
        prop_tab_baseptr = fblock_tab_ptr->PtrPropTab;  /* read base (!) pointer at property table */

        prop_tab_ptr = NtfGetPropTabPtr(prop_tab_baseptr, fkt_id);       /* prepare pointer at corresponding property line */

        if (prop_tab_ptr != NULL)                                        /* property available ? */
        {
            device_index_tab_baseptr = prop_tab_ptr->PtrNtfMatrixCol;

            NtfPropertyChanged(device_index_tab_baseptr, num_dev);       /* call first level function */
        }
    }
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfPropertyError()                                         */
/* Description : Set flags ERROR + NTF in device index table of the         */
/*               respective property                                        */
/*               (first level function)                                     */
/* Parameter(s): Pointer at device index table, number of devices           */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_42
void NtfPropertyError(pTNtfMatrix device_index_tab_ptr, byte num_entries)
{
    TNtfMatrix property_flags;

    property_flags = *device_index_tab_ptr;
    *device_index_tab_ptr = (TNtfMatrix)(property_flags & ~MASK_FLAG_ERROR);                          /* set property flag ERROR (inverted!)   */

    NtfChangeAllFlags(device_index_tab_ptr, num_entries, MASK_FLAG_NTF);
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfExtEvent()                                              */
/* Description : Set flags EXT1 + NTF in device index table of the          */
/*               respective property                                        */
/*               (first level function)                                     */
/* Parameter(s): Pointer at device index table, number of devices           */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_43
void NtfExtEvent(pTNtfMatrix device_index_tab_ptr, byte num_entries)
{
    NtfChangeAllFlags(device_index_tab_ptr, num_entries, MASK_FLAG_EXT1);                 /* set flags NTF + EXT1                  */
}
#endif









/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*  Useful Functions */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfLoadFBlockDescriptionRx()                               */
/* Description : Prepare num_dev (number of entries in device index table)  */
/*               and prop_tab_baseptr (base pointer at Property Table)      */
/*               for a respective FBlock                                    */
/*                                                                          */
/* Parameter(s): ptr at received message,                                   */
/*               ptr at num_entries (will be changed),                      */
/*               ptr at prop_tab_baseptr (will be changed)                  */
/* Returns     : error                                                      */
/*               0x00 no error occurred                                     */
/*               0x01 FBlockID.InstID not registered in Notification        */
/*                    Service                                               */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_50
byte NtfLoadFBlockDescriptionRx(pTMsgRx rx_ptr, byte* num_dev_ptr, pTNtfPropL* prop_tab_baseptrptr)
{
    byte fblock_index;
    pTNtfFBlockL fblock_tab_ptr;

    fblock_index = NbGetFBlockIndex( rx_ptr->FBlock_ID, rx_ptr->Inst_ID);
                                                    /* get local FBlock index of FBlockID.InstID */
    if ((byte)0xFF == fblock_index)
        return((byte)0x01);

    fblock_tab_ptr = NtfGetFBlockTabPtr(fblock_index);

    if (fblock_tab_ptr != NULL)                          /* fblock index found ? */
    {
        *num_dev_ptr = fblock_tab_ptr->NumDev;              /* read maximum count of devices to service          */
        *prop_tab_baseptrptr = fblock_tab_ptr->PtrPropTab;  /* read base (!) pointer at property table */
        return((byte)0x00);
    }
    else                                                /* fblock index not found ! */
        return ((byte)0x01);
}
#endif







/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfGetFBlockTabPtr()                                       */
/* Description : Get pointer at corresponding line in the Notification      */
/*               FBlock Table                                               */
/* Parameter(s): local FBlock index                                         */
/* Returns     : pointer at corresponding line in the Ntf FBlock Table      */
/*               NULL: FBlock index not registered in the Notification       */
/*                    Service                                               */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_51
pTNtfFBlockL NtfGetFBlockTabPtr(byte fblock_index)
{
    pTNtfFBlockL fblock_tab_ptr;
    byte i;

    fblock_tab_ptr = NtfFBlockTab;      /* set pointer at first entry in the Notification FBlock Table */

    for (i=(byte)0;i<NTF_MAX_FBLOCKS;i++)
    {
        if (fblock_tab_ptr->FBlockIndex == fblock_index)
            return (fblock_tab_ptr);
        else
            fblock_tab_ptr++;
    }
    return (NULL);
}
#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfGetPropTabPtr()                                         */
/* Description : Get pointer at corresponding line in the Notification      */
/*               Property Table                                             */
/* Parameter(s): Basepointer at corresponding Property Table, Function ID   */
/* Returns     : pointer at corresponding line in the Property Table        */
/*               NULL: Property not registered in the Notification Service   */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_52
pTNtfPropL NtfGetPropTabPtr(pTNtfPropL prop_tab_ptr, word func_id)
{
    word func_id_tab;

    if (func_id < 0x0FFF)
        do
        {
            func_id_tab = (word)(prop_tab_ptr->Type_FuncID & 0x0FFF);
            if (func_id_tab == func_id)
                return(prop_tab_ptr);
            else
                prop_tab_ptr++;

        }while (func_id_tab != (word)PROP_TERMINATION);

    return(NULL);
}
#endif




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfChangeAllFlags()                                        */
/* Description : Set flag NTF in device index table of the respective       */
/*               property                                                   */
/*               (zero level function)                                      */
/* Parameter(s): Pointer at device index table, number of devices, Flags    */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_53
void NtfChangeAllFlags(pTNtfMatrix device_index_tab_ptr, byte num_entries, TNtfMatrix flags_set)
{
    byte i;
    TNtfMatrix flags_device_index;

    device_index_tab_ptr++;                                         /* forget about column flag field */

    for (i=(byte)0;i<num_entries;i++)
    {
        flags_device_index = *device_index_tab_ptr;                 /* read flags and device index */
        *device_index_tab_ptr++ = flags_device_index | flags_set;   /* set flag(s) */
    }

    #ifdef MNS2_OPT_1
    NtfLoopCounter = NtfNumElements;                                /* Load counter to indicate demand for re-trigger */
    #endif

    #ifdef MNS2_OPT_1
    MNS2_REQUEST_CALL(MNS2_P_NTFS)                                  /* Set Request Flag and call application */
    #endif
}
#endif




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfErrorAppendFuncID()                                     */
/* Description : Pepare parameter "failed func id" in the error message     */
/*                                                                          */
/* Parameter(s): Pointer at number of failed func ids (will be modified),   */
/*               pointer at pointer at target data field (will be modified),*/
/*               failed func id                                             */
/*                                                                          */
/* Returns     : nothing                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_54
void NtfErrorAppendFuncID(byte *ptr_num, byte **pptr_tgt, word func_id)
{
    #if (MAX_MSG_TX_DATA < 512)
    if ( *ptr_num < (byte)((MAX_MSG_TX_DATA-2)/2)  )        /* Do not overflow limit of AMS TX buffer  */
    #endif
    {
        (*ptr_num)++;                               /* increment number of failed func ids                                           */

        #ifdef AMS_TX_ADD3
        MsgTxDataWord(pptr_tgt, &func_id);          /* Prepare parameter in the error msg */
        #else
        *((*pptr_tgt)++) = HB(func_id);
        *((*pptr_tgt)++) = LB(func_id);
        #endif
    }
}
#endif

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfErrorComplFuncID()                                      */
/* Description : Complete error message containing failed func ids          */
/*               Data[2]..Data[num*2+1] was already prepared by             */
/*               NtfErrorAppendFuncID()                                     */
/*                                                                          */
/* Parameter(s): number of failed func ids,                                 */
/*               pointer at error msg                                       */
/*                                                                          */
/* Returns     : OP_ERROR                                                   */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_55
byte NtfErrorComplFuncID(byte num, pTMsgTx tx_ptr)
{
    tx_ptr->Data[0] = ERR_FUNC_SPECIFIC;    /* Prepare Data[0]..Data[1] in the error msg  */
    tx_ptr->Data[1] = NTF_ERR_MATRIX;
    tx_ptr->Length  = (word)((num << 1) + 2);
    return (OP_ERROR);
}
#endif







/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfCheck()                                                 */
/* Description : looks for a certain DeviceID in all Notification matrices  */
/*                                                                          */
/* Parameter(s): ptr at tx msg, ptr at rx msg                               */
/* Returns     : op_type                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_61
byte NtfCheck(pTMsgTx tx_ptr, pTMsgRx rx_ptr)
{
    byte i;
    word num = (word)0;
    bool success;

    word device_id;
    byte device_index;
    byte fblock_error;
    word func_id;

    byte num_dev;
    pTNtfPropL prop_tab_ptr, prop_tab_baseptr;
    pTNtfMatrix device_index_tab_ptr;

    #ifndef CMD_ADD8
    if (rx_ptr->Length != (word)0x02)                         /* invalid length */
        return( CmdErrorMsg(tx_ptr, ERR_LENGTH) );       /* return(OP_ERROR) */
    #endif

    CmdDecodeWord(&device_id,&rx_ptr->Data[0]);
    device_index = NtfScanDeviceTab(device_id);
    if ((byte)0xFF == device_index)
    {
        return( CmdErrorByte(tx_ptr, ERR_FUNC_SPECIFIC, NTF_ERR_DEVICEID_NOTAVAIL) );    /* Error: DeviceID not entered in DeviceTab */
    }
    else if ((word)0 == NtfDeviceTabCnt[device_index])
    {
        return( CmdErrorByte(tx_ptr, ERR_FUNC_SPECIFIC, NTF_ERR_DEVICEID_NOTAVAIL) );    /* Error: DeviceID not entered in DeviceTab */
    }

    /*-------------- Load FBlock Description ------------------- */

    fblock_error = NtfLoadFBlockDescriptionRx(rx_ptr,&num_dev,&prop_tab_baseptr);       /* load max. number of device_index entries */
                                                                                        /* and base pointer at property table */

    if (fblock_error)
        return( CmdErrorByte(tx_ptr, ERR_FUNC_SPECIFIC, NTF_ERR_FBLOCK_NOTAVAIL) );
                                                                /* Error: FBlockID.InstID not registered */
                                                                /*        in Notification Service ! */
                                                                /*        --> return(OP_ERROR) */

    CmdEncodeWord(&tx_ptr->Data[0],&device_id);                 /* prepare Tx message */
    tx_ptr->Length = (word)2;

    #ifdef AMS_TX_ADD9
    tx_ptr->MidLevelRetries = (byte)0;                          /* internally generated messages don't use MLR */
    #endif

    prop_tab_ptr = prop_tab_baseptr;

    while (prop_tab_ptr->Type_FuncID != NTF_TERMINATION)
    {
        device_index_tab_ptr = prop_tab_ptr->PtrNtfMatrixCol+1; /* set pointer at first entry in device index table */
                                                                /* (column of notification matrix) */
        success = MNS_FALSE;                                    /* used to stop search if entry is found */
        for (i=(byte)0; (i < num_dev) && (MNS_FALSE == success); i++, device_index_tab_ptr++)
        {
            if (device_index == (*device_index_tab_ptr & MASK_DEV_INDEX))
            {
                if (MAX_MSG_TX_DATA < tx_ptr->Length + (word)2)     /* message length exceeds buffer size (MAX_MSG_TX_DATA) */
                {
                    return( CmdErrorByte(tx_ptr, ERR_FUNC_SPECIFIC, NTF_ERR_MSG_BUF_OVERFLOW) );
                }
                else
                {
                    num++;                                          /* entry found */
                    success = MNS_TRUE;
                    func_id = (word)(prop_tab_ptr->Type_FuncID & 0x0FFF);
                    CmdEncodeWord(&tx_ptr->Data[2*num], &func_id);
                    tx_ptr->Length += (word)2;
                }
            }
        }

        ++prop_tab_ptr;                                         /* next property */
    }

    return(OP_STATUS);
}

#endif



/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfDelNotification()                                       */
/* Description : Deletes the device index from a column of a                */
/*               notification matrix.                                       */
/*                                                                          */
/* Parameter(s): pointer to notification matrix,                            */
/*               number of entries,                                         */
/*               device index,                                              */
/*                                                                          */
/* Returns     : error                                                      */
/*               0x00 device_index not entered in DeviceTab                 */
/*               0x01 no error occurred, entry deleted                      */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_62
byte NtfDelNotification(pTNtfMatrix device_index_tab_ptr, byte num_entries, byte device_index)
{
    byte i;
    byte success = (byte)0;

    for (i=(byte)0; (i < num_entries) && ((byte)0 == success); ++i, ++device_index_tab_ptr)
    {
        if (device_index == (*device_index_tab_ptr & MASK_DEV_INDEX))
        {
            success = (byte)0x01;
            *device_index_tab_ptr = (TNtfMatrix)NTF_DEV_INDEX_FREE;
            /* decrement NtfDeviceTab usage counter */
            if ((word)0 < NtfDeviceTabCnt[device_index])
            {
                --NtfDeviceTabCnt[device_index];
            }
        }
    }
    return(success);
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfDelDevIDSingleColumn()                                  */
/* Description : Deletes the entry for a device_id in one column of a       */
/*               notification matrix. The notification matrix is specified  */
/*               by fblock_id and inst_id.                                  */
/*                                                                          */
/* Parameter(s): device_id,                                                 */
/*               fblock_id,                                                 */
/*               inst_id,                                                   */
/*               fkt_id                                                     */
/*                                                                          */
/* Returns     : error                                                      */
/*               0x00 no error occurred, entry deleted                      */
/*               0x01 DeviceID not entered in DeviceTab                     */
/*               0x02 FBlockID/InstID not entered in table FBlocks          */
/*               0x03 entry not found in Notification Matrix                */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_63
byte NtfDelDevIDSingleColumn(word device_id, byte fblock_id, byte inst_id, word fkt_id)
{
    byte        num_dev;
    byte        count = (byte)0;
    pTNtfPropL  prop_tab_ptr;
    pTNtfMatrix device_index_tab_ptr;

    byte        device_index;
    byte        ret;

    ret = NtfPrepVars(&prop_tab_ptr, &num_dev, &device_index, device_id, fblock_id, inst_id);
    if (ret != (byte)0)
        return(ret);

    while (prop_tab_ptr->Type_FuncID != NTF_TERMINATION  && count == (byte)0)
    {
        if ((prop_tab_ptr->Type_FuncID & 0x0FFF) == fkt_id)     /* column found ? */
        {
            device_index_tab_ptr = prop_tab_ptr->PtrNtfMatrixCol+1;
            count = NtfDelNotification(device_index_tab_ptr, num_dev, device_index);
        }
        ++prop_tab_ptr;
    }

    if (count > 0)
        return((byte)0);
    else
        return((byte)0x03);
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfDelDevIDSingleMatrix()                                  */
/* Description : Deletes the device index for device_id from a notification */
/*               matrix. The notification matrix is specified  by fblock_id */
/*               and inst_id.                                               */
/*                                                                          */
/* Parameter(s): device_id,                                                 */
/*               fblock_id,                                                 */
/*               inst_id,                                                   */
/*                                                                          */
/* Returns     : error                                                      */
/*               0x00 no error occurred, entry deleted                      */
/*               0x01 DeviceID not entered in DeviceTab                     */
/*               0x02 FBlockID/InstID not entered in table FBlocks          */
/*               0x03 entry not found in Notification Matrix                */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_64
byte NtfDelDevIDSingleMatrix(word device_id, byte fblock_id, byte inst_id)
{
    byte        num_dev;
    byte        device_index;
    byte        ret;
    pTNtfPropL  prop_tab_ptr;


    ret = NtfPrepVars(&prop_tab_ptr, &num_dev, &device_index, device_id, fblock_id, inst_id);
    if (ret != (byte)0)
        return(ret);

    ret = NtfDelDevIDMatrix(prop_tab_ptr, num_dev, device_index);

    if (ret > 0)
        return((byte)0);
    else
        return((byte)0x03);
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfDelDevIDAllMatrices()                                   */
/* Description : Deletes the device index for device_id from all            */
/*               notification matrices.                                     */
/*                                                                          */
/* Parameter(s): device_id                                                  */
/*                                                                          */
/* Returns     : error                                                      */
/*               0x00 no error occurred, entry deleted                      */
/*               0x01 DeviceID not entered in DeviceTab                     */
/*               0x03 entry not found in Notification Matrix                */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_65
byte NtfDelDevIDAllMatrices(word device_id)
{
    byte        num_dev;
    byte        device_index;
    byte        i;
    word        count = (word)0;
    pTNtfPropL  prop_tab_ptr;


    device_index = NtfScanDeviceTab(device_id);
    if (device_index == (byte)0xFF)
    {
        return((byte)0x01);                   /* Error: DeviceID not entered in DeviceTab */
    }

    /*  delete all single matrices */
    for (i=(byte)0;i<NTF_MAX_FBLOCKS;i++)
    {
        prop_tab_ptr = NtfFBlockTab[i].PtrPropTab;
        num_dev      = NtfFBlockTab[i].NumDev;

        count += NtfDelDevIDMatrix(prop_tab_ptr, num_dev, device_index);
    }

    if (count > (word)0)
    {
        return((byte)0);
    }
    else
    {
        return((byte)0x03);
    }
}
#endif




/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfPrepVars()                                              */
/* Description : finds property table pointer, number of entries and        */
/*               device id.                                                 */
/*                                                                          */
/* Parameter(s): pointer to property table,                                 */
/*               number of possible entries,                                */
/*               device_index,                                              */
/*                                                                          */
/* Returns     : error                                                      */
/*               0x00 no error occurred,                                    */
/*               0x01 DeviceID not entered in DeviceTab                     */
/*               0x02 FBlockID/InstID not entered in table FBlocks          */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_66
byte NtfPrepVars(pTNtfPropL* prop_tab_ptrptr, byte* num_dev, byte* device_index,
                 word device_id, byte fblock_id, byte inst_id)
{
    byte        fblock_index;           /* index in table of Command Interpreter */
    byte        ntf_fblock_index;       /* index in table of Notification Service */

    *device_index = NtfScanDeviceTab(device_id);
    if (*device_index == (byte)0xFF)
        return((byte)0x01);                   /* Error: DeviceID not entered in DeviceTab */

    fblock_index = NbGetFBlockIndex(fblock_id, inst_id); /* get index of entry in table of Command Interpreter   */
    if (fblock_index == (byte)0xFF)
        return((byte)0x02);               /* Error: FBlockID/InstID not entered in table of Command Interpreter */

    /* Convert 'fblock_index' (regarding T_FBlock.tab) to 'ntf_fblock_index' (regarding T_Notify.tab): */
    ntf_fblock_index = NtfConvertFBlockIndex(fblock_index);

    *prop_tab_ptrptr = NtfFBlockTab[ntf_fblock_index].PtrPropTab;
    *num_dev         = NtfFBlockTab[ntf_fblock_index].NumDev;

    return ((byte)0);
}
#endif


/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Function    : NtfDelDevIDMatrix()                                        */
/* Description : Deletes the device index for device_id from a              */
/*               notification matrix.                                       */
/*                                                                          */
/* Parameter(s): pointer to property table,                                 */
/*               number of possible entries,                                */
/*               device_index,                                              */
/*                                                                          */
/* Returns     : error                                                      */
/*               0x00 entry not found in Notification Matrix                */
/*               0x01 no error occurred, entry deleted                      */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifdef NTF_67
byte NtfDelDevIDMatrix(pTNtfPropL prop_tab_ptr, byte num_dev, byte device_index)
{
    word        count = (word)0;
    pTNtfMatrix device_index_tab_ptr;


    while (prop_tab_ptr->Type_FuncID != NTF_TERMINATION)
    {
        device_index_tab_ptr = prop_tab_ptr->PtrNtfMatrixCol+1;
        count += NtfDelNotification(device_index_tab_ptr, num_dev, device_index);
        ++prop_tab_ptr;
    }

    if (count > (word)0)
        return((byte)0x01);
    else
        return((byte)0x00);
}
#endif





