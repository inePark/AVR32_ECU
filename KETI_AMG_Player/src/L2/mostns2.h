/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Header File of MOST NetServices API (Layer II, Appl.Socket)
File:           MostNS2.h
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

#ifndef _MOSTNS2_H
#define _MOSTNS2_H




#include "mostdef2.h"

#include "protocol.h"

#ifdef ET_MIN
    #include "et.h"
#endif

/********************************************************************************************************************

                            GLOBAL DEFINITIONS OF MOST NETSERVICES (LAYER II)

********************************************************************************************************************/

#define NB_VERSION_MAJOR        ((byte) 0x03)
#define NB_VERSION_MINOR        ((byte) 0x00)
#define NB_VERSION_BUILD        ((byte) 0x01)

#define GFB_VERSION_MAJOR       ((byte) 0x03)
#define GFB_VERSION_MINOR       ((byte) 0x00)
#define GFB_VERSION_BUILD       ((byte) 0x02)

#define ET_VERSION_MAJOR     ((byte) 0x03)
#define ET_VERSION_MINOR     ((byte) 0x00)
#define ET_VERSION_BUILD     ((byte) 0x01)


/*-----------------------------------------------------------------*/
/*  Definitions of MOST NetService Kernel Layer II (MNS2)          */
/*-----------------------------------------------------------------*/

/* Flags of MnsPending2:  */
/*---------------------- */
#define MNS2_P_MSV2_STATE       (word)0x0001        /* Supervisor LayerII:  State of State Machine changed */
#define MNS2_P_MSV2_TIMEOUT     (word)0x0002        /* Supervisor LayerII:  Timeout event */
#define MNS2_P_AH_STATE         (word)0x0004        /* Address Handler:     State of State Machine changed */
#define MNS2_P_AH_TIMEOUT       (word)0x0008        /* Address Handler:     Timeout event  */
#define MNS2_P_NTFS             (word)0x0010        /* Notification Service */

/* Define group of flags: */
/*------------------------ */
#ifdef MSV2_MIN
 #define MNS2_P_MASK_MSV2       ( (MNS2_P_MSV2_STATE) | (MNS2_P_MSV2_TIMEOUT) )
#else
 #define MNS2_P_MASK_MSV2       0
#endif

#ifdef AH_MIN
 #define MNS2_P_MASK_AH         ( (MNS2_P_AH_STATE) | (MNS2_P_AH_TIMEOUT) )
#else
 #define MNS2_P_MASK_AH         0
#endif

#ifdef NTF_MIN
 #define MNS2_P_MASK_NTFS       ( MNS2_P_NTFS )
#else
 #define MNS2_P_MASK_NTFS       0
#endif

/* Mask of used flags: */
/*--------------------- */
#define MNS2_P_MASK             (MNS2_P_MASK_MSV2 | MNS2_P_MASK_AH | MNS2_P_MASK_NTFS)



/* Options for MostServiceLayer2():  */
/*---------------------------------- */
#define MNS2_O_NO_AH                0x0001      /* don't service the Address Handler module */
#define MNS2_O_NO_NTFS              0x0002      /* don't service the Notification Service */
#define MNS2_O_ALL                  0x0000      /* service all pending requests */


/* Events for MostServiceLayer2():  */
/*---------------------------------- */
#define MNS2_E_TIMER                ((MNS2_P_MSV2_TIMEOUT)|(MNS2_P_AH_TIMEOUT)) /* event forced by MnsRequestLayer2Timer() */
#define MNS2_E_PEN                  0x8000                                      /* event forced by return value of MostServiceLayer2() */
#define MNS2_E_REQ                  0x4000                                      /* event forced by MnsRequestLayer2() */
#define MNS2_E_MASK                 MNS2_E_TIMER                                /* Mask for events, which must be captured by "MnsPending2" */





/*-----------------------------------------------------------------*/
/*  Definitions of MOST Command Interpreter (CMD)                  */
/*-----------------------------------------------------------------*/
#ifdef CMD_MIN
/*  Terminations of tables */
/*------------------------ */
#define FUNC_TERMINATION    ((word)0xFFFF)                                          /* termination of Function_ID table          */
#define OP_TERMINATION      ((byte)0xFF)                                            /* termination of Operation table */


/*  Flags in OP_Type table */
/*------------------------ */
#define OP_W_NIL            ((byte)0x00)                                            /* no function call */
#define OP_W_IND_TXRX       ((byte)0xD0)                                            /* individual call with tx_ptr and rx_ptr */
#define OP_W_IND_NONE       ((byte)0x10)                                            /* individual call without parameter */
#define OP_W_IND_RX         ((byte)0x50)                                            /* individual call only with rx_ptr */
#define OP_W_IND_TX         ((byte)0x90)                                            /* individual call only with tx_ptr */
#define OP_W_MHP_EXT        ((byte)0xF0)                                            /* MOST High Protocol Extension */

#ifdef NS_CMD_MHP
 #define MHP_OP_FLAGS_DATA  ((byte)0xF1)                                            /* data pointer */
 #define MHP_OP_FLAGS_FUNC  ((byte)0xF2)                                            /* function pointer */
#endif

#define OP_W_MASK           ((byte)0xF0)                                            /* Mask Write Flags */

#define OP_R_NIL            ((byte)0x0)                                             /* no function call */
#define OP_R_IND_TX         ((byte)0x1)                                             /* individual call only with tx_ptr */
#define OP_R_IND_TXRX       ((byte)0x2)                                             /* individual call with tx_ptr and rx_ptr */
#define OP_R_BOOL           ((byte)0x3)                                             /* get Boolean type */
#define OP_R_BYTE           ((byte)0x4)                                             /* get Byte type */
#define OP_R_WORD           ((byte)0x5)                                             /* get Word type */
#define OP_R_LONG           ((byte)0x6)                                             /* get Long type */
#define OP_R_INTF           ((byte)0x7)                                             /* get interface description */
#define OP_R_MASK           ((byte)0xF)                                             /* Mask Read Flags */

#define OP_NO_REPORT        ((byte)0x00)                                    /* No report message required */

/* flags that can be combined with the OP_Type to be returned: */
#define CMD_PASS            ((byte) 0x10)                                            /* Flag signing message to pass on */
#define CMD_TX_RETAIN       ((byte) 0x40)                                            /* Flag signing to retain tx buffer */
#define CMD_RX_RETAIN       ((byte) 0x20)                                            /* Flag signing to retain rx buffer */
#define CMD_RX_REPEAT       ((byte) 0x80)                                            /* repeat same received message again */


#ifdef CMD_ADD8
  #define CMD_LEN_NO  ((word)0x00)
  #define CMD_LEN_EQ  ((word)0x01)
  #define CMD_LEN_LE  ((word)0x02)
  #define CMD_LEN_GE  ((word)0x03)

  #define LC_SHIFT ((word)6)
  #define LC_MASK  ((byte)0x3F)                                             /* parameter values up to 63 */

  #define LC_NO    (byte)(CMD_LEN_NO)                                       /* no length check */
  #define LC_EQ(a) ((byte)(CMD_LEN_EQ << LC_SHIFT) | ((byte)a & LC_MASK))   /* equal */
  #define LC_LE(a) ((byte)(CMD_LEN_LE << LC_SHIFT) | ((byte)a & LC_MASK))   /* less or equal */
  #define LC_GE(a) ((byte)(CMD_LEN_GE << LC_SHIFT) | ((byte)a & LC_MASK))   /* greater or equal */

#else
  #define LC_NO
  #define LC_EQ(a)
  #define LC_LE(a)
  #define LC_GE(a)
#endif




#endif  /* #ifdef CMD_MIN */



/*-----------------------------------------------------------------*/
/*  Definitions of Notification Service (NTF)                      */
/*-----------------------------------------------------------------*/
                                                                            /* Flags used in property table */
                                                                            /* of Notification Service: */
#define NTF_BOOL            ((word)0x1000)                                          /* Type: Bool  */
#define NTF_BYTE            ((word)0x2000)                                          /* Type: Byte */
#define NTF_WORD            ((word)0x3000)                                          /* Type: Word */
#define NTF_LONG            ((word)0x4000)                                          /* Type: Long */
#define NTF_IND             ((word)0x5000)                                          /* Service function is called (Tx ptr only) */
#define NTF_INDTXRX         ((word)0x6000)                                          /* Service function is called (Tx ptr, Rx ptr = NULL) */
#define NTF_EXT_FUNC        ((word)0x8000)                                          /* Additional Pointer is a function pointer */
#define NTF_EXT_DATA        ((word)0x0000)                                          /* Additional Pointer is a data pointer */
#define NTF_TERMINATION     ((word)0xFFFF)

#if (NTF_SIZE_DEVICE_TAB > 255)
    #undef  NTF_SIZE_DEVICE_TAB
    #define NTF_SIZE_DEVICE_TAB 255
#endif

#if (NTF_SIZE_DEVICE_TAB <= 32)
#define NTF_DEV_INDEX_FREE  0x1F                                            /* Init value for notification matrix */
#else
#define NTF_DEV_INDEX_FREE  0x00FF
#endif

/* Parameters of Telegram FBlockID.Notification.Set */
/*------------------------------------------------- */
#define NTF_SET_ALL         ((byte)0x00)                                            /* Set notification in all properties */
#define NTF_SET_FUNC        ((byte)0x01)                                            /* Set notification in some (following) properties */
#define NTF_CLR_ALL         ((byte)0x02)                                            /* Clear notification in all properties */
#define NTF_CLR_FUNC        ((byte)0x03)                                            /* Clear notification in some (following) properties */


/*-----------------------------------------------------------------*/
/*  Definitions of MOST Supervisor Layer II (MSV2)                 */
/*-----------------------------------------------------------------*/

/* error ids */
#define ERROR_TIMEOUT_CONFIG_STATUS 0xF1                                    /* Timeout error, if no status message from NetworkMaster  */




/********************************************************************************************************************

                            GLOBAL TYPE DECLARATIONS OF MOST NETSERVICES (LAYER II)

********************************************************************************************************************/

#ifdef PTR_FUNCTION_VOID
    #define NS_F_V (void (*) (void))
#else
    #define NS_F_V
#endif



/*-----------------------------------------------------------------*/
/*  Type Declaration of MOST Command Interpreter                   */
/*-----------------------------------------------------------------*/

#ifdef CMD_MIN
typedef _CONST struct FBlock_L_Type                 /* Line of the FBlock table */
{
    _CONST byte FBlock;                             /* FBlock ID */
    byte *Inst_Tab_Ptr;                             /* Ptr on Inst ID (RAM) of certain FBlock */
    _CONST struct Func_L_Type *Func_Tab_Ptr;        /* Ptr on function table */
} TFBlockL, *pTFBlockL;
#endif



#ifdef CMD_MIN
typedef _CONST struct Func_L_Type                   /* Line of a Function_ID table */
{
    _CONST word Func;                               /* Function ID */
    _CONST struct Op_L_Type *Op_Tab_Ptr;            /* Ptr on operation table */

} TFuncL, *pTFuncL;
#endif


#ifdef CMD_MIN
#ifdef PTR_UNION_VOID
typedef union
{
    byte (*FPtr)();                                 /* function pointer */
    byte *DPtr;                                     /* data pointer */
} TOpLPtrUnion;
#endif
#endif



#ifdef CMD_MIN
typedef _CONST struct Op_L_Type                     /* Line of a Operation Type table */
{
    _CONST byte Operation;                          /* Operation Type */
    _CONST byte Flags;                              /* kind of function call */

    #ifdef PTR_UNION_VOID
    TOpLPtrUnion Write;
    TOpLPtrUnion Read;
    #else
    #ifdef PTR_FUNCTION_VOID
    void (*Write_Ptr)(void);                        /* Ptr for write access (function ptr or data ptr) */
    void (*Read_Ptr)(void);                         /* Ptr for read access (function ptr or data ptr) */
    #else
    void *Write_Ptr;                                /* Ptr for write access (function ptr or data ptr) */
    void *Read_Ptr;                                 /* Ptr for read access (function ptr or data ptr) */
    #endif
    #endif

    #ifdef CMD_ADD8
    _CONST byte LengthCheck;                        /* relation and reference value */
    #endif

} TOpL, *pTOpL;
#endif




/*-----------------------------------------------------------------*/
/*  Type Declaration of Address Handler                            */
/*-----------------------------------------------------------------*/

#if (ADDRH_SIZE_DEVICE_TAB > 0)
typedef struct DevTab_T                             /* decentral device registry */
{
    byte FBlock_ID;                                 /* FBlockID */
    byte Inst_ID;                                   /* InstID */
    word Most_Addr;                                 /* Device Address */
}TDevTab, *pTDevTab;
#endif



/*-----------------------------------------------------------------*/
/*  Type Declaration of Notification Service                       */
/*-----------------------------------------------------------------*/
#ifdef NTF_MIN

#if (NTF_SIZE_DEVICE_TAB <=32)
typedef byte TNtfMatrix, *pTNtfMatrix;      /* Element of the Notification Matrix ( Flags+DeviceIndex <0..31> )  */
#else
typedef word TNtfMatrix, *pTNtfMatrix;      /* Element of the Notification Matrix ( Flags+DeviceIndex <0..255> ) */
#endif

#ifdef PTR_UNION_VOID
typedef union
{
    byte (*FPtr)();                         /* function pointer */
    void *DPtr;                             /* data pointer */
} TNtfPtrUnion;
#endif


typedef _CONST struct Ntf_Prop_L_Type       /* Line of the Notification Service Property Table */
{
    _CONST word Type_FuncID;                /* Notification Type and Function ID */
    pTNtfMatrix PtrNtfMatrixCol;            /* Ptr at Notification Matrix Column (Array of device indices with flags) (RAM) */
    #ifdef PTR_UNION_VOID
    TNtfPtrUnion Prop;
    #else
    #ifdef PTR_FUNCTION_VOID
    void (*PtrProperty)(void);              /* Ptr at Property (current value) */
    #else
    void *PtrProperty;                      /* Ptr at Property (current value) */
    #endif
    #endif

    #ifdef NTF_EXT
    #ifdef PTR_UNION_VOID
    TNtfPtrUnion PropExt1;
    #else
    #ifdef PTR_FUNCTION_VOID
    void (*PtrPropertyExt1)(void);          /* Ptr for extended use (only if NTF_EXT defined) */
    #else
    void *PtrPropertyExt1;                  /* Ptr for extended use (only if NTF_EXT defined) */
    #endif
    #endif
    #endif
} TNtfPropL, *pTNtfPropL;


typedef _CONST struct Ntf_FBlock_L_Type     /* Line of the Notification Service FBlock Table */
{
    _CONST byte FBlockIndex;                /* Local FBlock Index  */
    _CONST byte NumDev;                     /* Maximum number of devices to notify */
    pTNtfPropL PtrPropTab;                  /* Pointer at Property Table */
} TNtfFBlockL, *pTNtfFBlockL;

#endif





/********************************************************************************************************************

                            GLOBAL VARIABLES OF MOST NETSERVICES (LAYER II)

********************************************************************************************************************/






/********************************************************************************************************************

                            API FUNCTIONS OF MOST NETSERVICES (LAYER II)

********************************************************************************************************************/


#ifdef __cplusplus
extern "C"
{
#endif


/*-----------------------------------------------------------------*/
/*  API Functions of MOST NetService Kernel Layer II (MNS2)        */
/*-----------------------------------------------------------------*/

#ifdef MNS2_0
void InitNetServicesLayer2(void);                                   /* Init all modules of NetServices Layer II */
#endif


#ifdef MNS2_1
word MostServiceLayer2(word opt, word events);                      /* Trigger function for MOST NetServices Layer II  */
#endif




/*-----------------------------------------------------------------*/
/*  API Functions of MOST Supervisor Layer II (MSV2)               */
/*-----------------------------------------------------------------*/

#ifdef MSV2_0
void MostSupervisor2Init(void);                                     /* Init MOST Supervisor Layer II */
#endif

#ifdef MSV2_5
void CentralRegistryCheckComplete(byte NewStatus);                  /* Termination of System Configuration Check */
#endif                                                              /* Parameter: 0: NotOk */
                                                                    /*            1: Ok */
                                                                    /*            2: Invalid */
                                                                    /*            4: NewExt */

#ifdef MSV2_6
void CentralRegistryMprChanged(byte mpr_new, byte mpr_old);         /* must be called on Network Change Event */
#endif



#ifdef MSV2_7
void CentralRegistryTriggerJustBorn(void);                          /* Triggers NetworkMaster.Configuration.Status.NotOk */
#endif                                                              /* after first address initialisation (device is just born) */





/*-----------------------------------------------------------------*/
/*  API Functions of MOST Command Interpreter (CMD)                */
/*-----------------------------------------------------------------*/

#ifdef CMD_0
void CmdInit(void);                                                 /* Init the MOST command interpreter */
#endif

#ifdef CMD_2
byte CmdRxInterpreter(pTMsgTx tx_ptr, pTMsgRx rx_ptr);
#endif

#ifdef CMD_3
void CmdEncodeWord(byte *ptr_tgt, word *ptr_src);                   /* copy a 16 bit value to tx buffer */
#endif

#ifdef CMD_4
void CmdDecodeWord(word *ptr_tgt, byte *ptr_src);                   /* copy a 16 bit value from rx buffer */
#endif


#ifdef CMD_5
byte CmdErrorMsg(pTMsgTx tx_ptr, byte error_id);        /* Prepare data field and length of a standard */
#endif                                                              /* error message. Can be used if no parameters */
                                                                    /* have to send back. */
#ifdef CMD_6
byte CmdErrorByte(pTMsgTx tx_ptr, byte error_id, byte param);
#endif                                                              /* Prepare data field and length of an error */
                                                                    /* message, which will transport one  */
                                                                    /* parameter of type Byte. */
#ifdef CMD_12
byte CmdErrorParamAll(pTMsgTx tx_ptr, pTMsgRx rx_ptr, byte error_id);
#endif                                                              /* Prepare data field and length of an error */
                                                                    /* message, which will transport all */
                                                                    /* received parameters */
#ifdef CMD_14
byte CmdErrorParamWrong(pTMsgTx tx_ptr, byte index, byte* param_ptr, byte num);
#endif                                                              /* Prepare data field and length of an error */
                                                                    /* message, when a parameter is wrong */

#ifdef CMD_15
byte CmdErrorParamNotAvailable(pTMsgTx tx_ptr, byte index, byte* param_ptr, byte num);
#endif                                                              /* Prepare data field and length of an error     */
                                                                    /* message, when a parameter is not available                                                                                                      */


#ifdef CMD_8
void CmdEncodeStrRom(pTMsgTx tx_ptr, const byte *src_ptr);
#endif                                                              /* Copy a string or an interface description */
                                                                    /* from ROM to Tx Buffer */
                                                                    /* Stringformat:  */
                                                                    /* HB of length, LB of length, data[0]..data[n] */

#ifdef CMD_13
void CmdEncodeStrRam(pTMsgTx tx_Ptr, byte *src_ptr);
#endif                                                              /* Copy a string or an interface description */
                                                                    /* from RAM to Tx Buffer */
                                                                    /* Stringformat:  */
                                                                    /* HB of length, LB of length, data[0]..data[n] */


#ifdef CMD_9
void CmdEncodeLong(byte *ptr_tgt, dword *ptr_src);                  /* copy a 32 bit value to tx buffer */
#endif

#ifdef CMD_10
void CmdDecodeLong(dword *ptr_tgt, byte *ptr_src);                  /* copy a 32 bit value from rx buffer */
#endif


#ifdef CMD_11
void CmdCopyTxRx(pTMsgTx tx_ptr, pTMsgRx rx_ptr);
#endif                                                              /* copy data and length field from rx to tx buffer */


#ifdef CMD_20
byte CmdInsertSenderHandle(pTMsgTx tx_ptr, byte handle_hb, byte handle_lb);
#endif

#ifdef CMD_21
byte CmdInsertSenderHandlePtr(pTMsgTx tx_ptr, pTMsgRx rx_ptr);
#endif

#ifdef CMD_23
byte CmdGetFunctionIds(pTMsgTx tx_ptr, pTFuncL func_ptr);
#endif

/*-----------------------------------------------------------------*/
/*  API Functions of module NetBlock (NB)                          */
/*-----------------------------------------------------------------*/

#ifdef NB_0
void NbInit(void);
#endif

#ifdef NB_4
byte NbGetFBlockIndex(byte fblock_id, byte inst_id);
#endif

#ifdef NB_7
bool NbSetNodeAddr(word addr);
#endif

#ifdef NB_8
void NbSetGroupAddr(byte addr);
#endif

#ifdef NB_9
byte NbSetShadowInstID(byte FBlock, byte InstOld, byte InstNew);
#endif

#ifdef NB_13
void NbGetVersion(byte *version);
#endif

#ifdef NB_14
void NbGetMOSTVersion(byte *version);
#endif

#ifdef NB_15
void NbSetGetNodeAddr(word addr);
#endif





/*-----------------------------------------------------------------*/
/*  API Functions of Address Handler (AH)                          */
/*-----------------------------------------------------------------*/
#ifdef AH_0
void AddrHInit(void);
#endif

#ifdef AH_01
void AddrHDevTabInit(bool ClearDevTab);
#endif


#ifdef AH_8
word AddrHDevTabGet(byte FBlockID, byte InstID);
#endif

#ifdef AH_9
void AddrHDevTabSet(word MostAddr, byte FBlockID, byte InstID);
#endif

#ifdef AH_13
bool AddrHDevTabDelEntry(byte FBlockID, byte InstID);
#endif



/*-----------------------------------------------------------------*/
/*  API Functions of Notification Service (NTF)                    */
/*-----------------------------------------------------------------*/
#ifdef NTF_0
void NtfInit(void);
#endif

#ifdef NTF_7
byte NtfInitSingleMatrix(byte fblock_id, byte inst_id);
#endif

#ifdef NTF_10
byte NtfSetNotificationMatrix(pTMsgTx tx_ptr, pTMsgRx rx_ptr);
#endif

#ifdef NTF_11
byte NtfGetNotificationMatrix(pTMsgTx tx_ptr, pTMsgRx rx_ptr);
#endif

#ifdef NTF_12
byte NtfSetSingleNotification(pTNtfMatrix device_index_tab_ptr, byte num_entries, word device_id);
#endif

#ifdef NTF_13
byte NtfReadMatrixCol(pTNtfMatrix col_ptr, byte num_lines, word* ptr_device_buffer);
#endif

#ifdef NTF_40
void NtfPropertyChangedRx(pTMsgRx  rx_ptr);
#endif

#ifdef NTF_41
void NtfPropertyChanged(pTNtfMatrix device_index_tab_ptr, byte num_entries);
#endif

#ifdef NTF_42
void NtfPropertyError(pTNtfMatrix device_index_tab_ptr, byte num_entries);
#endif

#ifdef NTF_43
void NtfExtEvent(pTNtfMatrix device_index_tab_ptr, byte num_entries);
#endif

#ifdef NTF_44
void NtfPropertyChangedFkt(byte fblock_id, byte inst_id, word fkt_id);
#endif

#ifdef NTF_45
void NtfPropertyChangedIdx(byte fblock_index, word fkt_id);
#endif

#ifdef NTF_61
byte NtfCheck(pTMsgTx tx_ptr, pTMsgRx rx_ptr);
#endif

#ifdef NTF_63
byte NtfDelDevIDSingleColumn(word device_id, byte fblock_id, byte inst_id, word fkt_id);
#endif

#ifdef NTF_64
byte NtfDelDevIDSingleMatrix(word device_id, byte fblock_id, byte inst_id);
#endif

#ifdef NTF_65
byte NtfDelDevIDAllMatrices(word device_id);
#endif




/********************************************************************************************************************

                            CALLBACK FUNCTIONS OF MOST NETSERVICES (LAYER II)

********************************************************************************************************************/


/*-----------------------------------------------------------------*/
/* Callback Functions of MOST NetServices Kernel Layer II (MNS2)   */
/* Can be an empty function block if the application has not to    */
/* react to this calls.                                            */
/*-----------------------------------------------------------------*/
#ifdef MNS2_CB1
void MnsRequestLayer2(word flags);
#endif

#ifdef MNS2_CB2
void MnsRequestLayer2Timer(word flags);
#endif






/*-----------------------------------------------------------------*/
/* Callback Functions of MOST CommandInterpreter (CMD)             */
/* These functions MUST work along the discription since needed    */
/* by this Service.                                                */
/*-----------------------------------------------------------------*/

#ifdef CMD_CB1
byte CmdUnknownFBlockShadow(pTMsgTx tx_ptr, pTMsgRx rx_ptr);
#endif

#ifdef CMD_CB2
byte CmdRxFilter(pTMsgTx tx_ptr, pTMsgRx rx_ptr);
#endif

#ifdef CMD_CB3
byte CmdTxFilter(pTMsgTx tx_ptr, pTMsgRx rx_ptr);
#endif

#ifdef CMD_CB4
void CmdTxNoResult(pTMsgRx rx_ptr);
#endif






/*-----------------------------------------------------------------*/
/*                                                                 */
/* Callback Functions of module NetBlock (NB)                      */
/*                                                                 */
/*-----------------------------------------------------------------*/

#ifdef NB_CBS1
void NbNodeAddrStatus(pTMsgRx prbuf);
#endif

#ifdef NB_CBE1
void NbNodeAddrError(pTMsgRx prbuf);
#endif


#ifdef NB_CBS2
void NbFBlockIDsStatus(pTMsgRx prbuf);
#endif

#ifdef NB_CBE2
void NbFBlockIDsError(pTMsgRx prbuf);
#endif


#ifdef NB_CBS3
void NbDeviceInfoStatus(pTMsgRx prbuf);
#endif

#ifdef NB_CBE3
void NbDeviceInfoError(pTMsgRx prbuf);
#endif


#ifdef NB_CBS4
void NbGroupAddrStatus(pTMsgRx prbuf);
#endif

#ifdef NB_CBE4
void NbGroupAddrError(pTMsgRx prbuf);
#endif



#ifdef NB_CBS7
void NbShutDownResult(pTMsgRx prbuf);
#endif

#ifdef NB_CBE7
void NbShutDownError(pTMsgRx prbuf);
#endif


#ifdef NB_CBS8
void NbPowerStatus(pTMsgRx prbuf);
#endif

#ifdef NB_CBE8
void NbPowerError(pTMsgRx prbuf);
#endif


#ifdef NB_CBS9
void NbNodePositionStatus(pTMsgRx prbuf);
#endif

#ifdef NB_CBE9
void NbNodePositionError(pTMsgRx prbuf);
#endif


#ifdef NB_CBS10
void NbRetryParametersStatus(pTMsgRx prbuf);
#endif

#ifdef NB_CBE10
void NbRetryParametersError(pTMsgRx Rx_Ptr);
#endif

#ifdef NB_CBS11
void NbSamplingFrequencyStatus(pTMsgRx Rx_Ptr);
#endif

#ifdef NB_CBE11
void NbSamplingFrequencyError(pTMsgRx Rx_Ptr);
#endif

#ifdef NB_CBS14
void NbBoundaryStatus(pTMsgRx prbuf);
#endif

#ifdef NB_CBE14
void NbBoundaryError(pTMsgRx prbuf);
#endif

#ifdef NB_CBS15
void NbVersionStatus(pTMsgRx prbuf);
#endif

#ifdef NB_CBE15
void NbVersionError(pTMsgRx prbuf);
#endif

#ifdef NB_CBS16
void NbShutDownReasonStatus(pTMsgRx prbuf);
#endif

#ifdef NB_CBE16
void NbShutDownReasonError(pTMsgRx prbuf);
#endif

#ifdef NB_CBS17
void NbFBlockInfoStatus(pTMsgRx prbuf);
#endif

#ifdef NB_CBE17
void NbFBlockInfoError(pTMsgRx prbuf);
#endif

#ifdef NB_CBS18
void NbImplFBlocksStatus(pTMsgRx prbuf);
#endif

#ifdef NB_CBE18
void NbImplFBlocksError(pTMsgRx prbuf);
#endif

#ifdef NB_CBS19
void NbEUI48Status(pTMsgRx prbuf);
#endif

#ifdef NB_CBE19
void NbEUI48Error(pTMsgRx prbuf);
#endif

/*-----------------------------------------------------------------*/
/* Callback Functions of module NetBlock (NB)                      */
/* These functions MUST work along the discription since needed    */
/* by this Service.                                                */
/*-----------------------------------------------------------------*/
#ifdef NB_CB1
bool NbShutDown(void);                                                  /* Callback function to indicate query of shut down process */
#endif                                                                  /* ReturnValue: 1: suspending */
                                                                        /*              0: not suspending  */


#ifdef NB_CB6
byte NbGetDeviceInfo(byte id, pTMsgTx tx_ptr);                          /* Service request for Device Info */
#endif


#ifdef NB_CB8
void NbShutDownExecute(void);                                           /* Callback function to indicate execution  */
#endif                                                                  /* of shut down process */

#ifdef NB_CB9
void NBFBlockIDsGet(pTMsgTx Tx_Ptr);
#endif

#ifdef NB_CB12
void NbStoreInstIDs(byte *InstIDs);
#endif

#ifdef NB_CB13
byte NbRestoreInstIDs(byte *InstIDs);
#endif

#ifdef NB_CB14
void NbStoreShadowInstIDs(byte *ShadowInstIDs);
#endif

#ifdef NB_CB15
byte NbRestoreShadowInstIDs(byte *ShadowInstIDs);
#endif

#ifdef NB_CB16
byte NBFBlockIDsSet(pTMsgTx Tx_Ptr, pTMsgRx Rx_Ptr);
#endif

#ifdef NB_CB17
bool NbShutDownDevice(void);
#endif

#ifdef NB_CB19
bool NbBoundarySetQuery(byte boundary);
#endif

#ifdef NB_CB20
bool NbFBlockInfoGet(word id, bool answer_prepared, pTMsgTx tx_ptr);
#endif

#ifdef NB_CB21
void NbImplFBlocksGet(pTMsgTx tx_ptr);
#endif



/*-----------------------------------------------------------------*/
/* Callback Functions of Address Handler (AH)                      */
/* These functions MUST work along the discription since needed    */
/* by this Service.                                                */
/*-----------------------------------------------------------------*/
#ifdef AH_CB1
void AddrHSearchFailed(pTMsgTx ptbuf);                                  /* Address searching process failed */
#endif

#ifdef AH_CB2
void AddrHResult(word MostAddr, byte FBlockID, byte InstID);            /* Reporting result of a searching proccess */
#endif

#ifdef AH_CB3
void AddrHStoreDevTab(pTDevTab SrcPtr);                                 /* Store decentral registry into non-volatile  */
#endif                                                                  /* memory area */

#ifdef AH_CB4
void AddrHRestoreDevTab(pTDevTab TgtPtr);                               /* Restore decentral registry from non-volatile */
#endif                                                                  /* memory area */



/*-----------------------------------------------------------------*/
/*                                                                 */
/* Callback Functions of module NetworkMaster (NM)                 */
/*                                                                 */
/*-----------------------------------------------------------------*/

#ifdef NM_CBS1
void NmConfigurationStatus(pTMsgRx prbuf);
#endif

#ifdef NM_CBE1
void NmConfigurationError(pTMsgRx prbuf);
#endif


#ifdef NM_CBS2
void NmCentralRegStatus(pTMsgRx prbuf);
#endif

#ifdef NM_CBE2
void NmCentralRegError(pTMsgRx prbuf);
#endif


#ifdef NM_CBR4
void NmSaveConfigResult(pTMsgRx prbuf);
#endif

#ifdef NM_CBE4
void NmSaveConfigError(pTMsgRx prbuf);
#endif

#ifdef NM_CBRA4
void NmSaveConfigResultAck(pTMsgRx prbuf);
#endif

#ifdef NM_CBEA4
void NmSaveConfigErrorAck(pTMsgRx prbuf);
#endif

#ifdef NM_CBS5
void NmFktIDsStatus(pTMsgRx prbuf);
#endif

#ifdef NM_CBE5
void NmFktIDsError(pTMsgRx prbuf);
#endif

#ifdef NM_CBS6
void NmSystemAvailStatus(pTMsgRx prbuf);
#endif

#ifdef NM_CBE6
void NmSystemAvailError(pTMsgRx prbuf);
#endif

#ifdef NM_CBS7
void NmFBlockInfoStatus(pTMsgRx prbuf);
#endif

#ifdef NM_CBE7
void NmFBlockInfoError(pTMsgRx prbuf);
#endif

#ifdef NM_CBS8
void NmVersionStatus(pTMsgRx prbuf);
#endif

#ifdef NM_CBE8
void NmVersionError(pTMsgRx prbuf);
#endif

#ifdef NM_CBRA9
void NmOwnConfigInvalidResultAck(pTMsgRx prbuf);
#endif

#ifdef NM_CBEA9
void NmOwnConfigInvalidErrorAck(pTMsgRx prbuf);
#endif



/*-----------------------------------------------------------------*/
/* Callback Functions of MOST Supervisor Layer II (MSV2)           */
/* Can be an empty function block if the application has not to    */
/* react to this calls.                                            */
/*-----------------------------------------------------------------*/
#ifdef MSV2_CB1
void SystemCommunicationInit(byte ConfigStatusOk);
#endif

#ifdef MSV2_CB2
void CentralRegistryCheckStart(void);
#endif

#ifdef MSV2_CB3
void CentralRegistryClear(void);
#endif

#ifdef MSV2_CB4
void Store_Error_Info2(byte error_id);
#endif

#ifdef MSV2_CB5
void FBlockIDsChanged(pTMsgRx Rx_Ptr);
#endif



/*-----------------------------------------------------------------*/
/*                                                                 */
/* Callback Functions of module DebugMessages (DM)                 */
/*                                                                 */
/*-----------------------------------------------------------------*/
#ifdef DM_CB0
void DmAdjAppDbgMsgStatus(pTMsgRx prbuf);
#endif

#ifdef DM_CB1
void DmAdjAppDbgMsgError(pTMsgRx prbuf);
#endif



#ifdef __cplusplus
}
#endif


#endif /* _MOSTNS2_H */


