#ifndef _DEFINES_H_
#define _DEFINES_H_

#include <avr32/io.h>

typedef char                s8_t, TYPE_S8;
typedef unsigned char       u8_t, TYPE_U8, TYPE_BOOL;
typedef short               s16_t, TYPE_S16;
typedef unsigned short      u16_t, TYPE_U16;
typedef int                 s32_t, TYPE_S32;
typedef unsigned int        u32_t, TYPE_U32;
typedef long long           s64_t, TYPE_S64;
typedef unsigned long long  u64_t, TYPE_U64;

typedef TYPE_S16            Type_Result;

//#ifndef U8
//typedef TYPE_U8 U8;
//#endif

#ifndef NULL
#define NULL 0
#endif

#define TRUE 1
#define FALSE 0


//#define DEV_USE_BUB85650

#define DEV_ENABLE

////////////////////////////////////////////////////////////////////////////////
#define ERR_NONE                                (0)

#define ERR_DRVI2C_BASE                         (-100)


////////////////////////////////////////////////////////////////////////////////
#define __AVR32_GCC__

////////////////////////////////////////////////////////////////////////////////
#ifdef __AVR32_GCC__
#define SMSC_ATTRIB_MEM_FLASH const
#define SMSC_ATTRIB_CPU_ID
#else // #ifdef __AVR32_GCC__
#ifdef __IAR_SYSTEMS_ICC__
#define SMSC_ATTRIB_MEM_FLASH __flash
#define SMSC_ATTRIB_CPU_ID __CPU__
#else // #ifdef __IAR_SYSTEMS_ICC__
#define SMSC_ATTRIB_MEM_FLASH const
#define SMSC_ATTRIB_CPU_ID
#endif // #ifdef __IAR_SYSTEMS_ICC__
#endif // #ifdef __AVR32_GCC__


////////////////////////////////////////////////////////////////////////////////

#define MOST50

#define NUM_DACS 1
#define DRVI2C_SPEED_DEFAULT (2*400000)
//#define DRVI2C_SPEED_CODEC (2*98000)
#define DRVI2C_SPEED_CODEC 100000  //100kHz

////////////////////////////////////////////////////////////////////////////////
// INIC's Control Port I2C Address
#define I2C_INIC_ADDR       (0x40>>1)        // write address
// Power Management Chip I2C Addr
#define PM_ADDR       (0x10>>1)
// DAC address 
#define DAC_ADDR (0x20>>1)
// CODEC_ADDR
#define CODEC_ADDR (0x90>>1)


////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
#define CR         (uint8_t) 0x0D      // carriage return
#define LF         (uint8_t) 0x0A       // line feed
#define SP         (uint8_t) 0x20       // space
#define ESC        (uint8_t) 0x1B       // escape character
#define XON        (uint8_t) 0x11       // start / OK to send
#define XOFF       (uint8_t) 0x13       // stop sending
#define RX_CHAT_THRESH          44                              // 44 bytes is max an I2C control port message can be
#define ISO8859              0x01;  // Format Identifier

//#define HB(value) ( (uint8_t) (value >> 8) )
//#define LB(value) ( (uint8_t) (value) )


////////////////////////////////////////////////////////////////////////////////
// GPIO Defs

// DAC_Int
#define DAC_INT_PENDING() 0



////////////////////////////////////////////////////////////////////////////////
#define MAX_RS232_RX_BUFFER_SIZE (512)



#ifdef DEV_ENABLE
extern TYPE_BOOL system_halt;
#define SYSTEM_HALT() LOG_DBG("HALT\n"); while(system_halt);
//#define SYSTEM_HALT() asm("BREAKPOINT")
#else
#define SYSTEM_HALT()
#endif // #ifdef DEV_ENABLE

#define strcpy_P strcpy

extern SMSC_ATTRIB_MEM_FLASH char CompanyName[];
extern SMSC_ATTRIB_MEM_FLASH char ProductName[];
extern SMSC_ATTRIB_MEM_FLASH char ProductVersion[];

//#define TEST_TIMER_ENABLE

#define AVN_DEFAULT_VOLUME	30
#define AVN_VOLUME_MAX		40
#define AVN_VOLUME_MIN		0

#define LOG_NOR		printf
#define LOG_ERR		printf
#define LOG_DBG		printf

/*  Data Structures */
typedef struct fw_state_st {
	uint8_t fwVerStored;
	uint8_t fwVerCur;
	uint8_t fVolSet;
	uint8_t fVolCur;
	uint8_t rVolSet;
	uint8_t rVolCur;
} fw_state_t;
#endif // #ifndef _DEFINES_H_
