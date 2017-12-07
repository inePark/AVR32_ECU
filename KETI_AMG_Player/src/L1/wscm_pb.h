/*
==============================================================================

Project:        MOST NetServices V3 for INIC
Module:         Public API of the Socket Connection Manager Wrapper (WSCM)
File:           wscm_pb.h
Version:        3.0.x-SR-1  
Language:       C
Author(s):      S.Kerber, S.Semmler
Date:           05.January.2011

FileGroup:      Layer I
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

/*! \file
  * \brief      Public API of the Socket Connection Manager Wrapper (WSCM)
  */

#ifndef _WSCM_PB_H
#define _WSCM_PB_H

#include "mostdef1.h"   /* Definition File for MOST NetServices Basic Layer */

/*
==============================================================================
    WSCM Service flags (public only for trace purposes)
==============================================================================
*/

#define WSCM_P_NONE              ((word) 0x0000)         /* general 'none' */

#define WSCM_P_GO_PROTECTED      ((word) 0x0001)
#define WSCM_P_NTF_COMPLETE      ((word) 0x0002)
#define WSCM_P_PM                ((word) 0x0004)
#define WSCM_P_PM_DONE           ((word) 0x0008)
#define WSCM_P_SRV_CHECK         ((word) 0x0010)
/*
#define WSCM_P_FREE6             ((word) 0x0020)
#define WSCM_P_FREE7             ((word) 0x0040)
#define WSCM_P_FREE8             ((word) 0x0080)
#define WSCM_P_FREE9             ((word) 0x0100)
#define WSCM_P_FREE10            ((word) 0x0200)
#define WSCM_P_FREE11            ((word) 0x0400)
#define WSCM_P_FREE12            ((word) 0x0800)
#define WSCM_P_FREE13            ((word) 0x1000)
#define WSCM_P_FREE14            ((word) 0x2000)
#define WSCM_P_FREE15            ((word) 0x4000)
#define WSCM_P_FREE16            ((word) 0x8000)
*/
#define WSCM_P_FIRST             WSCM_P_GO_PROTECTED
#define WSCM_P_LAST              WSCM_P_SRV_CHECK

/*
==============================================================================
    API
==============================================================================
*/

/*! \brief Port ID MediaLB */
#define SCM_PORT_ID_MEDIALB                         ((byte) 0x00)
/*! \brief Port ID of the control port */
#define SCM_PORT_ID_CPSER                           ((byte) 0x01)
/*! \brief Port ID of the MOST Network Port */
#define SCM_PORT_ID_MOST                            ((byte) 0x02)
/*! \brief Port ID of the Streaming Port */
#define SCM_PORT_ID_STREAM                          ((byte) 0x03)
/*! \brief Port ID of the SPI Port */
#define SCM_PORT_ID_SPI                             ((byte) 0x04)
/*! \brief Port ID of instance 0 of the Transport Stream Interface Port */
#define SCM_PORT_ID_TRANSPORT                       ((byte) 0x05)
/*! \brief Port ID of instance 1 of the Transport Stream Interface Port */
#define SCM_PORT_ID_TRANSPORT_1                     ((byte) 0x25)
/*! \brief Port ID of the Remote Clock Port */
#define SCM_PORT_ID_RMCK                            ((byte) 0x06)

/*! \brief A mask to prescind the port ID from the instance  */
#define SCM_PORT_ID_INST_MASK                       ((byte) 0x1F)

#define SCM_QUEUE_ID_ALL                            ((byte) 0x00)
#define SCM_QUEUE_ID_ICM                            ((byte) 0x01)
#define SCM_QUEUE_ID_MCM                            ((byte) 0x02)
#define SCM_QUEUE_ID_MDP                            ((byte) 0x03)


/* MediaLB port configuration: 'interface_mode' */
#define MEDIALB_AUTO                                0xFF

/* MediaLB port configuration: 'port_mode' */
#define SCM_PORT_CFG_MLB_MODE_CTL                   ((byte) 0x00)

/* MediaLB port configuration: 'clock_config' */
#define SCM_PORT_CFG_MLB_256_FS                     ((byte) 0x00)
#define SCM_PORT_CFG_MLB_512_FS                     ((byte) 0x01)
#define SCM_PORT_CFG_MLB_1024_FS                    ((byte) 0x02)
#define SCM_PORT_CFG_MLB_2048_FS                    ((byte) 0x03)
#define SCM_PORT_CFG_MLB_3072_FS                    ((byte) 0x04)
#define SCM_PORT_CFG_MLB_4096_FS                    ((byte) 0x05)
#define SCM_PORT_CFG_MLB_6144_FS                    ((byte) 0x06)
#define SCM_PORT_CFG_MLB_8192_FS                    ((byte) 0x07)
/* Socket Description: 'direction' */
#define SCM_IN                                      ((byte) 0x00)
#define SCM_OUT                                     ((byte) 0x01)

/* Socket Description: 'datatype' */
#define SCM_TYPE_STREAM                             ((byte) 0x00) /* deprecated: kept for legacy reason */
#define SCM_TYPE_SYNC                               ((byte) 0x00)
#define SCM_TYPE_PACKET                             ((byte) 0x01)
#define SCM_TYPE_CONTROL                            ((byte) 0x02)
#define SCM_TYPE_ISOPACKET                          ((byte) 0x03)
#define SCM_TYPE_ISODFPHASE                         ((byte) 0x04)
#define SCM_TYPE_ISODFDATA                          ((byte) 0x05)
#define SCM_TYPE_QOS_IP_STREAM                      ((byte) 0x06)

/* Socket Description: 'most_flags' */
#define SCM_MOST_FLAGS_DEFAULT                      ((byte) 0x00)
/* - flags for  info 'preallocated' */
#define SCM_MOST_FLAGS_SOURCE_ALLOCATE              ((byte) 0x00)
#define SCM_MOST_FLAGS_SOURCE_CONNECT               ((byte) 0x01)
/* - flags for  info 'preallocated' */
/*   applicable only for streaming "in" sockets */
#define SCM_MOST_FLAGS_SINGLE_CONNECTION            ((byte) 0x00)
#define SCM_MOST_FLAGS_MULTIPLE_CONNECTIONS         ((byte) 0x02)

/* Arguments for ScmMuteMode() */
#define SCM_MUTE_MODE_AUTO                          ((byte) 0x00)
#define SCM_MUTE_MODE_OFF                           ((byte) 0x01)
#define SCM_MUTE_MODE_OFF_EXT                       ((byte) 0x02)

/* Streaming port config ... clock drive mode */
#define SCM_PORT_CFG_STREAM_SCKFSY_INIC             ((byte) 0x00)
#define SCM_PORT_CFG_STREAM_SCKFSY_EXTERNAL         ((byte) 0x01)
#define SCM_PORT_CFG_STREAM_SCKFSY_DF_INIC          ((byte) 0x02)
#define SCM_PORT_CFG_STREAM_SCKFSY_DF_EXTERNAL      ((byte) 0x03)


/* Streaming port config ... port mode */
#define SCM_PORT_CFG_STREAM_IN_OUT                  ((byte) 0x00)
#define SCM_PORT_CFG_STREAM_DUAL_IN                 ((byte) 0x01)
#define SCM_PORT_CFG_STREAM_DUAL_OUT                ((byte) 0x02)
#define SCM_PORT_CFG_STREAM_FULL                    ((byte) 0x03)
#define SCM_PORT_CFG_STREAM_WILDCARD                ((byte) 0xFF)

/*!\brief  Streaming Data format */
#define SCM_STREAM_FRMT_DEL_64FS_16BIT              ((byte) 0x00)
#define SCM_STREAM_FRMT_DEL_64FS_24BIT              ((byte) 0x01)
#define SCM_STREAM_FRMT_DEL_128FS_16BIT             ((byte) 0x02)
#define SCM_STREAM_FRMT_DEL_128FS_24BIT             ((byte) 0x03)
#define SCM_STREAM_FRMT_DEL_64FS_SEQ                ((byte) 0x04)
#define SCM_STREAM_FRMT_DEL_128FS_SEQ               ((byte) 0x05)
#define SCM_STREAM_FRMT_DEL_256FS_SEQ               ((byte) 0x06)
#define SCM_STREAM_FRMT_RIGHT_64FS_16BIT            ((byte) 0x07)
#define SCM_STREAM_FRMT_RIGHT_64FS_24BIT            ((byte) 0x08)
#define SCM_STREAM_FRMT_64FS_SEQ                    ((byte) 0x0B)
#define SCM_STREAM_FRMT_128FS_SEQ                   ((byte) 0x0C)
#define SCM_STREAM_FRMT_256FS_SEQ                   ((byte) 0x0D)
#define SCM_STREAM_FRMT_LEFT_64FS_16BIT             ((byte) 0x0E)
#define SCM_STREAM_FRMT_LEFT_64FS_24BIT             ((byte) 0x0F)
#define SCM_STREAM_FRMT_LEFT_128FS_16BIT            ((byte) 0x10)
#define SCM_STREAM_FRMT_LEFT_128FS_24BIT            ((byte) 0x11)
#define SCM_STREAM_FRMT_RIGHT_128FS_16BIT           ((byte) 0x15)
#define SCM_STREAM_FRMT_RIGHT_128FS_24BIT           ((byte) 0x16)
#define SCM_STREAM_FRMT_DEL_512FS_SEQ               ((byte) 0x17)
#define SCM_STREAM_FRMT_512FS_SEQ                   ((byte) 0x18)
#define SCM_STREAM_FRMT_DEL_8FS_SEQ                 ((byte) 0x19)
#define SCM_STREAM_FRMT_8FS_SEQ                     ((byte) 0x1A)
#define SCM_STREAM_FRMT_DEL_16FS_SEQ                ((byte) 0x1B)
#define SCM_STREAM_FRMT_16FS_SEQ                    ((byte) 0x1C)
#define SCM_STREAM_FRMT_DEL_32FS_SEQ                ((byte) 0x1D)
#define SCM_STREAM_FRMT_32FS_SEQ                    ((byte) 0x1E)

/* Streaming socket interface id's */
#define SCM_STREAM_INTERFACE_SR0                    ((byte) 0x00)
#define SCM_STREAM_INTERFACE_SX0                    ((byte) 0x01)
#define SCM_STREAM_INTERFACE_SR1                    ((byte) 0x02)
#define SCM_STREAM_INTERFACE_SX1                    ((byte) 0x03)
#define SCM_STREAM_INTERFACE_SR2                    ((byte) 0x04)
#define SCM_STREAM_INTERFACE_SX2                    ((byte) 0x05)
#define SCM_STREAM_INTERFACE_SR3                    ((byte) 0x06)
#define SCM_STREAM_INTERFACE_SX3                    ((byte) 0x07)
#define SCM_STREAM_INTERFACE_FSYFSK                 ((byte) 0x10)

/*!\brief Streaming Socket frequency reference value */
#define SCM_DSFR_44100                              ((byte) 0x01)
#define SCM_DSFR_48000                              ((byte) 0x02)
#define SCM_DSFR_96000                              ((byte) 0x03)
#define SCM_DSFR_192000                             ((byte) 0x04)
#define SCM_DSFR_8000                               ((byte) 0x05)
#define SCM_DSFR_16000                              ((byte) 0x06)
#define SCM_DSFR_176400                             ((byte) 0x07)
#define SCM_DSFR_88200                              ((byte) 0x08)

/*!\brief RMCK divider */
#define SCM_RMCK_DIVIDER_DISABLED                   ((byte) 0x00)
#define SCM_RMCK_DIVIDER_64_FS                      ((byte) 0x01)
#define SCM_RMCK_DIVIDER_128_FS                     ((byte) 0x02)
#define SCM_RMCK_DIVIDER_256_FS                     ((byte) 0x03)
#define SCM_RMCK_DIVIDER_384_FS                     ((byte) 0x04)
#define SCM_RMCK_DIVIDER_512_FS                     ((byte) 0x05)
#define SCM_RMCK_DIVIDER_768_FS                     ((byte) 0x06)
#define SCM_RMCK_DIVIDER_1024_FS                    ((byte) 0x07)
#define SCM_RMCK_DIVIDER_1536_FS                    ((byte) 0x08)

/* SPI port configuration: 'port_mode' */
#define SCM_PORT_CFG_SPI_SLAVE                      ((byte) 0x00)

#define SCM_PM_NONE                                 ((byte) 0x00)
#define SCM_PM_AUTO                                 ((byte) 0x02)

/* Transport Stream port config ... port mode */
#define SCM_PORT_CFG_TRANSPORT_SLAVE                ((byte) 0x00)
#define SCM_PORT_CFG_TRANSPORT_MASTER               ((byte) 0x01)

#define SCM_ISO_PACKET_SIZE_188                     ((word) 0x00BC)
#define SCM_ISO_PACKET_SIZE_196                     ((word) 0x00C4)
#define SCM_ISO_PACKET_SIZE_206                     ((word) 0x00CE)

/*!\brief Configuration for RMCK port \a clock_source  */
#define SCM_RMCK_NETWORK_SYSTEM_CLOCK               ((byte) 0x00)
/*!\brief Configuration for RMCK port \a clock_source  */
#define SCM_RMCK_ISOC_PHASE                         ((byte) 0x01)
/*!\brief Configuration for RMCK port \a clock_source  */
#define SCM_RMCK_ISOC_LINKED_TO_PORT                ((byte) 0x02)

/*!\brief Configuration for RMCK port \a drive_mode  */
#define SCM_RMCK_SCKFSY_INIC_OUT                    ((byte) 0x00)
#define SCM_RMCK_SCKFSY_INIC_IN                     ((byte) 0x01)

typedef struct TScmTransportStreamPortConfig
{
    byte port_mode;

} TScmTransportStreamPortConfig;

typedef struct TScmStreamingPortConfig
{
    /*! \brief   Who is driving the SCK/FSY signal.
     *  \details Possible values are:
     *           \li SCM_PORT_CFG_STREAM_SCKFSY_INIC: The INIC.
     *           \li SCM_PORT_CFG_STREAM_SCKFSY_EXTERNAL: Driven external.
     *           \li SCM_PORT_CFG_STREAM_SCKFSY_DF_INIC: The INIC with discrete frame clock.
     *           \li SCM_PORT_CFG_STREAM_SCKFSY_DF_EXTERNAL: Driven external with discrete frame clock.
     */
    byte clock_drive_mode;
    /*! \brief port mode of the streaming port decription.
     *  \details Possible values are:
     *           \li SCM_PORT_CFG_STREAM_IN_OUT: Partial streaming mode IN/OUT
     *           \li SCM_PORT_CFG_STREAM_DUAL_IN: Partial streaming mode dual IN
     *           \li SCM_PORT_CFG_STREAM_DUAL_OUT : Partial streaming mode dual OUT.
     *           \li SCM_PORT_CFG_STREAM_FULL: Full streaming mode.
     *           \li SCM_PORT_CFG_STREAM_WILDCARD: Use in case of OS81110
     */
    byte port_mode;
    /*! \brief Data format of streaming port description.
     *  \details Possible values are:
     *  \li SCM_STREAM_FRMT_DEL_64FS_16BIT: Delayed-bit streaming format, 64 x Fs, 16-bit channels, legacy
     *  \li SCM_STREAM_FRMT_DEL_64FS_24BIT: Delayed-bit streaming format, 64 x Fs, 24-bit channels, legacy
     *  \li SCM_STREAM_FRMT_DEL_128FS_16BIT: Delayed-bit streaming format, 128 x Fs, 16-bit channels, legacy (MOST50 only)
     *  \li SCM_STREAM_FRMT_DEL_128FS_24BIT: Delayed-bit streaming format, 128 x Fs, 24-bit channels, legacy (MOST50 only)
     *  \li SCM_STREAM_FRMT_DEL_64FS_SEQ: Delayed-bit streaming format, 64 x Fs, sequence
     *  \li SCM_STREAM_FRMT_DEL_128FS_SEQ: Delayed-bit streaming format, 128 x Fs, sequence
     *  \li SCM_STREAM_FRMT_DEL_256FS_SEQ: Delayed-bit streaming format, 256 x Fs, sequence
     *  \li SCM_STREAM_FRMT_RIGHT_64FS_16BIT: Rigth-justified Streaming format, 64 x Fs, 16-bit channels,legacy
     *  \li SCM_STREAM_FRMT_RIGHT_64FS_24BIT: Rigth-justified Streaming format, 64 x Fs, 24-bit channels,legacy
     *  \li SCM_STREAM_FRMT_64FS_SEQ: Sequential Streaming format, 64 x Fs
     *  \li SCM_STREAM_FRMT_128FS_SEQ: Sequential Streaming format, 128 x Fs
     *  \li SCM_STREAM_FRMT_256FS_SEQ: Sequential Streaming format, 256 x Fs
     *  \li SCM_STREAM_FRMT_LEFT_64FS_16BIT: Left-justified Streaming Format, 64 x Fs, 16-bit channels,legacy
     *  \li SCM_STREAM_FRMT_LEFT_64FS_24BIT: Left-justified Streaming Format, 64 x Fs, 24-bit channels,legacy
     *  \li SCM_STREAM_FRMT_LEFT_128FS_16BIT: Left-justified Streaming Format, 128 x Fs, 16-bit channels,legacy (MOST50 only)
     *  \li SCM_STREAM_FRMT_LEFT_128FS_24BIT: Left-justified Streaming Format, 128 x Fs, 24-bit channels,legacy (MOST50 only)
     *  \li SCM_STREAM_FRMT_RIGHT_128FS_16BIT: Right-justified Streaming Format, 128 x Fs, 16-bit channels,legacy (MOST50 only)
     *  \li SCM_STREAM_FRMT_RIGHT_128FS_24BIT: Right-justified Streaming Format, 128 x Fs, 24-bit channels,legacy (MOST50 only)
     */
    byte data_format;

} TScmStreamingPortConfig;

typedef struct TScmMediaLBPortConfig
{

    byte port_mode;

    byte clock_config;

} TScmMediaLBPortConfig;

#ifdef _OS81110_ISO
typedef struct TScmRMCKPortConfig
{
    byte divider;
    byte drive_mode;
    byte clock_source;
    byte linked_to_port_id;
}TScmRMCKPortConfig;
#endif

#ifdef _OS81110_SPI
/*!
  * \brief   Configuration of the SPI Port
  *
  */
typedef struct TScmSpiPortConfig
{
    /*! \brief      Indicates the port mode of the SPI Port
     *  \details    Possible values are:
     *              \li SCM_PORT_CFG_SPI_SLAVE
     *              Further values may be available in future versions
     */
    byte port_mode;

    /*! \brief      Indicates the clock mode of the SPI Port
     *  \details    Possible values are:
     *              \li (0x00) SCLK is low when idle. Data switches on falling edge,
     *                  needs to be captured on the rising edge.
     *              \li (0x01) SCLK is low when idle. Data switches on rising edge,
     *                  needs to be captured on the falling edge.
     *              \li (0x02) SCLK is high when idle. Data switches on rising edge,
     *                  needs to be captured on the falling edge.
     *              \li (0x03) SCLK is high when idle. Data switches on falling edge,
     *                  needs to be captured on the rising edge.
     */
    byte clock_mode;

    /*! \brief      Indicates the so called interrupt threshold
     *  \details    It denotes the number of bytes the SPI-bus master is allowed to
     *              access after the SPI-bus slave shows "not ready" using /SINTn.
     *              Valid interrupt_thresholds are 0x00 up to 0x0F.
     */
    byte interrupt_threshold;

}TScmSpiPortConfig;
#endif


typedef union TScmPortConfig
{
    TScmStreamingPortConfig streaming;
    TScmMediaLBPortConfig   medialb;
    #ifdef _OS81110_ISO
    TScmTransportStreamPortConfig transport;
    TScmRMCKPortConfig rmck;
    #endif

    #ifdef _OS81110_SPI
    TScmSpiPortConfig spi;
    #endif

} TScmPortConfig;
typedef struct TScmPortDesc
{
    byte           port_id;
    TScmPortConfig config;

} TScmPortDesc;

typedef struct TScmSocketDesc
{
    byte  port_id;
    byte  direction;
    byte  datatype;
    word  blockwidth;
    struct TScmMediaLBSocketDesc
    {
        /*! \brief The MediaLB channel */
        word channel_addr;
        /*! \brief The offset to the start of the socket. */
        byte offset;
        /*! \brief The total blockwidth to be allocated on the MediaLB */
        byte channel_blockwidth;
        #ifdef _OS81110_ISO
        /*! \brief The size of data packets in the isochronous channel. */
        word iso_packet_size;
        /*! \brief Denotes a reference value to be the closest sample frequency to the real one.
         *  \details Possible values are:
         *           \li (0x00)  32000 Hz
         *           \li (0x01)  41100 Hz
         *           \li (0x02)  48000 Hz
         *           \li (0x03)  96000 Hz
         *           \li (0x04) 192000 Hz
         */
        byte data_sample_freq_ref;
        /*! \brief Enables the flow-control for the MediaLB channel. */
        bool flow_control_enable;
        #endif
    } medialb;

    /*! \brief      Holds the properties of a MOST Network socket.
      * \attention  The arguments of the structure are only relevant if the port id corresponds
      *             to the MOST Network port. For all other ports the structure can be omitted.
      */
    struct TScmMostSocketDesc
    {
        byte  flags;
        byte  list_len;
        word *list_ptr;
        word *result_list_ptr;
        #ifdef _OS81110_ISO
        word iso_packet_size;
        /*! \brief Denotes the number of data bytes per isochronous frame.
         */
        byte data_frame_blockwidth;
        /*! \brief Denotes a reference value to be the closest sample frequency to the real one.
         *  \details Possible values are:
         *           \li (0x00)  32000 Hz
         *           \li (0x01)  41100 Hz
         *           \li (0x02)  48000 Hz
         *           \li (0x03)  96000 Hz
         *           \li (0x04) 192000 Hz
         */
        byte data_sample_freq_ref;
        #endif

    } most;
    struct TScmStreamingSocketDesc
    {
        byte interface_id;
        byte offset;

        #ifdef _OS81110_ISO
        /*! \brief Denotes a reference value closest to the sample rate of the intended source.
          */
        byte data_sample_freq_ref;
        #endif
    } streaming;

    #ifdef _OS81110_ISO
    /*! \brief Holds the properties of a transport stream interface (TSI) socket.
      * \attention The arguments of the structure are only relevant if the port id corresponds
      *            to the TSI port. For all other ports the structure can be omitted.
      */
    struct TScmTransportStreamSocketDesc
    {
        word iso_packet_size;

    } transport;
    #endif

    #ifdef _OS81110_ISO
    struct TScmRMCKSocketConfig
    {
        byte data_sample_freq_ref;
    }rmck;
    #endif
} TScmSocketDesc;

typedef void TScmListCB(TMnsResult result, byte list_len, word *list_ptr);

#ifdef SCM_5
    byte ScmSendHandleMsg(byte handle, TMnsStdCB *cb_ptr, word func_id);
#endif

#ifdef SCM_6
    byte ScmSetByteByMsg(byte value, TMnsStdCB *cb_ptr, word func_id);
#endif

#ifdef SCM_7
    byte ScmOpenPort(TScmPortDesc *desc_ptr, TMnsStdCB *cb_ptr);
#endif

#ifdef SCM_8
    typedef void TScmCreateSocketCB(TMnsResult result, byte socket_handle,
                                   byte list_len, word *list_ptr);
    byte ScmCreateSocket(TScmSocketDesc *desc_ptr, TScmCreateSocketCB *cb_ptr);
#endif

#ifdef SCM_10
    byte ScmGetMuteMode(void);
#endif

#ifdef SCM_11
    byte ScmGetBoundary(void);
#endif

#ifdef SCM_15
    typedef void TScmAllocOnlyMlbCB(TMnsResult result, byte bandwith);
    byte ScmAllocOnlyMlb(word channel_addr, byte bandwidth,
                         TScmAllocOnlyMlbCB *cb_ptr);
#endif

#ifdef SCM_16
    byte ScmDeallocOnlyMlb(word channel_addr, TMnsStdCB *cb_ptr);
#endif

#ifdef SCM_MIN
typedef struct TScmBoundaryInfo
    {
        byte preset;
        byte current;
        byte total;
        word free;
        byte packet;
        byte used;

    } TScmBoundaryInfo;
#endif

#ifdef SCM_30
    byte ScmGetBoundaryExt(TScmBoundaryInfo *info_ptr);
#endif

#ifdef SCM_31
    byte ScmDestroySocketExt(byte socket_handle, word *result_list_ptr,
                             TScmListCB *cb_ptr);
#endif

#ifdef SCM_MIN
    typedef void TScmConnectSocketsCB(TMnsResult result,
                                  byte connection_handle);
    #define ScmConnectSockets(input_handle, output_handle, cb_ptr) ScmConnectSocketsExt(input_handle, output_handle, MNS_FALSE, cb_ptr)
#endif

#ifdef SCM_32
    byte ScmConnectSocketsExt( byte input_handle, byte output_handle,
                               bool default_mute,
                               TScmConnectSocketsCB *cb_ptr );
#endif

#ifdef SCM_MIN
    typedef void TScmGetSourceCB(TMnsResult result, word node_address,
                                 byte group_address, byte node_position);
#endif

#ifdef SCM_35
    byte ScmGetSource(word connection_label, TScmGetSourceCB *cb_ptr);
#endif

#ifdef SCM_MIN
    typedef void TSCDemuteCB(byte* sc_status, byte length);
#endif


#ifdef SCM_MIN
    /*! \brief     Wrapper to ScmSendHandleMsg().
     *  \details   API function macro.
     *
     *  \param[in] connection_handle: Identifies the connection.
     *  \param[in] cb_ptr the callback will return the results.
     */
    #define ScmDisconnectSockets(connection_handle, cb_ptr) ScmSendHandleMsg(connection_handle, cb_ptr, FUNCID_INIC_DISCONNECTSOCKETS)


    /*! \brief     Wrapper to ScmSendHandleMsg().
     *  \details   API function macro.
     *
     *  \param[in] socket_handle: Identifies the socket.
     *  \param[in] cb_ptr the callback will return the results.
     */
    #define ScmDestroySocket(socket_handle, cb_ptr) ScmSendHandleMsg(socket_handle, cb_ptr, FUNCID_INIC_DESTROYSOCKET)

    #define ScmMuteConnection(connection_handle, cb_ptr) ScmSendHandleMsg(connection_handle, cb_ptr, FUNCID_INIC_MUTECONNECTION)

    #define ScmDemuteConnection(connection_handle, cb_ptr) ScmSendHandleMsg(connection_handle, cb_ptr, FUNCID_INIC_DEMUTECONNECTION)

    #define ScmSetMuteMode(mode, cb_ptr) ScmSetByteByMsg(mode, cb_ptr, FUNCID_INIC_MUTEMODE)

    #define ScmSetBoundary(sbc, cb_ptr) ScmSetByteByMsg(sbc, cb_ptr, FUNCID_INIC_BANDWIDTH)

    #define ScmClosePort(port_id, cb_ptr) ScmSendHandleMsg(port_id, cb_ptr, FUNCID_INIC_CLOSEPORT)
#endif

#endif /* _WSCM_PB_H */
