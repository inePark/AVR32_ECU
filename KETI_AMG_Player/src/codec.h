/*******************************************************************************
 * SMSC Automotive Infotainment Systems Group
 *
 * (c)2002-2009 Standard Microsystems Corporation and its subsidiaries ("SMSC").
 * The copyright of the computer program(s) herein is the property of SMSC.
 * The program(s) may be used and copied only with written permission from SMSC
 * or in accordance with the terms and conditions stipulated in the
 * agreement under which the program(s) have been supplied.
******************************************************************************/

/*****************************************************

Module  : Driver for Cirrus CS42448 CODEC, header file
Version : 1.0
File    : codec.h
Date    : 09/16/2009
Author  : Gary Warren
Company : SMSC-AIS


Modifications:

09/25/2009  GW    Creation of driver

*****************************************************/
#ifndef __CS42448_CODEC__
#define __CS42448_CODEC__
/****************************************************************************
  Function definitions
****************************************************************************/
s16_t    CODEC_setDefaultValues(void);
s16_t    CODEC_setDefaultValuesFast(void);

s16_t    CODEC_mute(void);
s16_t    CODEC_demute(void);
u8_t     CODEC_isMuted(void);

void     CODEC_setVolume(u8_t ch, u8_t value);
u8_t     CODEC_getVolume(u8_t ch);

s16_t    CODEC_setAttenuation(u8_t ch, u8_t value);
s16_t    CODEC_getAttenuation(u8_t ch);

s16_t    CODEC_getStatus(void);

Type_Result codec_RegisterRead(TYPE_U8 regAddr, TYPE_U8 * regData_ptr);
Type_Result codec_RegisterWrite(TYPE_U8 regAddr, TYPE_U8 regData);


/****************************************************************************
  Register and bit definitions for CS42448
****************************************************************************/


#define CODEC_ID           0x01     // Read Only - Chip ID

#define CODEC_POWER_CON    0x02     // Power control bits
#define PDN_ADC3  7        // Power down ADC3
#define PDN_ADC2  6        // Power down ADC2
#define PDN_ADC1  5        // Power down ADC1
#define PDN_DAC4  4        // Power down DAC4
#define PDN_DAC3  3        // Power down DAC3
#define PDN_DAC2  2        // Power down DAC2
#define PDN_DAC1  1        // Power down DAC1
#define PDN       0        // Power down entire CODEC

#define CODEC_FORMATS      0x04     // Audio formats
#define FREEZE    7        // Freeze bit
#define AUX_DIF   6        // AUX In interface format
#define DAC_DIF2  5        // DAC interface format 2
#define DAC_DIF1  4        // DAC interface format 1
#define DAC_DIF0  3        // DAC interface format 0
#define ADC_DIF2  2        // ADC interface format 2
#define ADC_DIF1  1        // ADC interface format 1
#define ADC_DIF0  0        // ADC interface format 0

#define CODEC_ADC_CONT     0x05     // ADC control register
#define ADC12_FREEZE  7        // ADC 1&2 High pass filter Freeze bit
#define ADC3_FREEZE   6        // ADC 3 High pass filter Freeze bit
#define DAC_DEM       5        // De-Emphasis
#define ADC1_SINGLE   4        // ADC1 single ended mode (def is differential)
#define ADC2_SINGLE   3        // ADC2 single ended mode (def is differential)
#define ADC3_SINGLE   2        // ADC3 single ended mode (def is differential)
#define AIN5_MUX      1        // Select AIN5A or AIN5B input when in single ended mode
#define AIN6_MUX      0        // Select AIN6A or AIN6B input when in single ended mode

#define CODEC_TRANS_CONT   0x06     // Transition Control
#define DAC_SNG_VOL  7     // One channel reg controls all DAC volume
#define DAC_SZC1     6     // DAC soft transition control 1
#define DAC_SZC0     5     // DAC soft transition control 0
#define AMUTE        4     // analog in Mute
#define ADC_MUTE_SP  3     // ADC mute control
#define ADC_SNG_VOL  2     // Single gain control for adc
#define ADC_SZC1     1     // ADC soft transition control 1
#define ADC_SZC0     0     // ADC soft transition control 0

#define CODEC_DAC_MUTE     0x07     // each bit mutes respective DAC output (FF = mute all, 00 = demute)

#define CODEC_DAC1_VOL     0x08     // DAC1 volume control (if DAC_SNG_VOL set, controls all DACs
#define DAC_VOL_7    7     // msb
#define DAC_VOL_5    5     // .
#define DAC_VOL_4    4     // .
#define DAC_VOL_3    3     // .
#define DAC_VOL_2    2     // .
#define DAC_VOL_1    1     // .
#define DAC_VOL_0    0     // lsb

#define CODEC_DAC2_VOL     0x09     // DAC2 volume control
#define CODEC_DAC3_VOL     0x0A     // DAC3 volume control
#define CODEC_DAC4_VOL     0x0B     // DAC4 volume control
#define CODEC_DAC5_VOL     0x0C     // DAC5 volume control
#define CODEC_DAC6_VOL     0x0D     // DAC6 volume control
#define CODEC_DAC7_VOL     0x0E     // DAC7 volume control
#define CODEC_DAC8_VOL     0x0F     // DAC8 volume control

#define CODEC_ADC1_GAIN    0x11     // ADC1 gain control (if ADC_SNG_VOL set, controls all ADCs
#define ADC_GAIN_7   7     // msb
#define ADC_GAIN_6   6     // .
#define ADC_GAIN_5   5     // .
#define ADC_GAIN_4   4     // .
#define ADC_GAIN_3   3     // .
#define ADC_GAIN_2   2     // .
#define ADC_GAIN_1   1     // .
#define ADC_GAIN_0   0     // lsb

#define CODEC_ADC2_GAIN    0x12     // ADC2 gain control
#define CODEC_ADC3_GAIN    0x13     // ADC3 gain control
#define CODEC_ADC4_GAIN    0x14     // ADC4 gain control
#define CODEC_ADC5_GAIN    0x15     // ADC5 gain control
#define CODEC_ADC6_GAIN    0x16     // ADC6 gain control
#define CODEC_ADC7_GAIN    0x17     // ADC7 gain control

#define CODEC_STATUS_CONT  0x18     // Status control register
#define STAT_INT_1   3
#define STAT_INT_0   2

#define CODEC_STATUS_REG   0x19     // Status register
#define DAC_CLK_ERR  4
#define ADC_CLK_ERR  3
#define ADC3_OVFL    2
#define ADC2_OVFL    1
#define ADC1_OVFL    0



#endif /*CODEC_H_*/
