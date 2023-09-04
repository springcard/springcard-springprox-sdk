#ifndef __SPROX_CAPAS_H__
#define __SPROX_CAPAS_H__

/*
 * Definition of product capabilities
 * ----------------------------------
 * This is a 4-byte value
 *
 * Byte 3 : proprietary extensions
 * Byte 2 : supported function set
 * Byte 1 : supported communication protocols
 * Byte 0 : supported ISO RF protocols
 */

//#define SPROX_NOT_A_SPRINGPROX       0x80000000

#define SPROX_WITH_NORALSY_SPECIFIC  0x02000000
#define SPROX_WITH_HUMAN_CONSOLE     0x01000000

#define SPROX_WITH_STORAGE           0x00800000
#define SPROX_WITH_FEED              0x00400000
#define SPROX_WITH_I2C_BUS           0x00200000    /* New 1.72 */
#define SPROX_WITH_MANY_ANTENNAS     0x00100000    /* New 1.72 */

#define SPROX_WITH_USB_HID           0x00040000
#define SPROX_WITH_USB_CCID          0x00020000
#define SPROX_WITH_USB_VCP           0x00010000

#define SPROX_WITH_XXL_BUFFERS       0x00008000
#define SPROX_WITH_BAUDRATE_115200   0x00004000
#define SPROX_WITH_DUAL_BUFFERS      0x00002000
#define SPROX_WITH_RS485_DRIVER      0x00001000
#define SPROX_WITH_BIN_BUS_PROTOCOL  0x00000800
#define SPROX_WITH_RS485_SERIAL      0x00000400
#define SPROX_WITH_BIN_PROTOCOL      0x00000200
#define SPROX_WITH_ASCII_PROTOCOL    0x00000100

#define SPROX_WITH_EMUL              0x00000080
#define SPROX_WITH_125_KHZ           0x00000040    /* New 1.72 - before: was Sagem MSO */
#define SPROX_WITH_ISO_7816          0x00000020
#define SPROX_WITH_TARGET            0x00000010

#define SPROX_WITH_ISO_18092         0x00000008
#define SPROX_WITH_FELICA            0x00000004
#define SPROX_WITH_ISO_15693         0x00000002
#define SPROX_WITH_ISO_14443         0x00000001


#endif
