/**h* SpringProx/SpringProx.API
 *
 * NAME
 *   SpringProx.API -- SpringCard unified API for SpringProx, CSB and Kxxx contactless readers
 *
 * COPYRIGHT
 *   Copyright (c) 2000-2013 SpringCard SAS, FRANCE - www.springcard.com
 *
 * NOTES
 *   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
 *   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
 *   TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 *   PARTICULAR PURPOSE.
 *
 * AUTHOR
 *   Johann.D / SpringCard
 *
 * HISTORY  
 *   JDA 15/02/2003 : [1.00]
 *                    creation from CSB3 tree
 *   JDA 16/05/2003 : [1.01]
 *                    added first set of SPROX_ functions
 *   JDA 01/07/2003 : [1.02 0]
 *                    added helpers for VB or Delphi
 *   JDA 30/09/2003 : [1.02 1]
 *                    added some utility functions
 *   JDA 13/10/2003 : [1.10]
 *                    added basic T=CL function set
 *                    added support for USB communication
 *   JDA 12/12/2003 : [1.11]
 *                    improved T=CL function set
 *   JDA 03/02/2004 : [1.12]
 *                    added SPROX_MifStSelectIdle and SPROX_MifStHalt
 *                    added SPROX_DES_... and SPROX_3DES_... (LGPL licence)
 *                    optimization for Desfire frame chaining
 *   JDA 20/11/2003 : [1.13]
 *                    bug-fix release
 *   JDA 25/11/2003 : [1.14]
 *                    bug-fix release
 *   JDA 25/01/2004 : [1.15]
 *                    added basic support for ISO 14443-B
 *   JDA 22/01/2004 : [1.16]
 *                    speed-improvement release
 *   JDA 12/02/2004 : [1.17]
 *                    reader-side changes only
 *   JDA 19/02/2004 : [1.18]
 *                    added support of 115200bps communication
 *   JDA 08/04/2004 : [1.19 0]
 *                    added support of ASCII protocol, and the GetDeviceSettings/SetDeviceSettings
 *                    functions
 *   JDA 05/05/2004 : [1.19 1]
 *                    added SPROX_MifStReadTag768 et SPROX_MifStWriteTag768
 *   JDA 17/06/2004 : [1.20]
 *                    support of multiple readers thanks to SPROX_CreateInstance, SPROX_SelectInstance,
 *                    and SPROX_DestroyInstance
 *   JDA 01/12/2004 : [1.21 0]
 *                    added the "const" modifier on most input buffers to pass strict pre-compiler checks
 *                    without a warning
 *                    fixed SPROX_TclA_ActivateAny bug
 *   JDA 14/01/2005 : [1.21 1]
 *                    added SPROX_GetFeatures and SPROX_Card_... functions
 *   JDA 25/05/2005 : [version 1.22]
 *                    added SPROX_ReaderSelectAddress
 *   JDA 21/06/2005 : [version 1.30]
 *                    added SPROX_MSO_Exchange and others MSO functions
 *   JDA 05/07/2005 : [version 1.31]
 *                    added SPROX_ReaderAttachSerial, moved multiple reader support to springprox_ex.h
 *   JDA 08/09/2005 : [version 1.32]
 *                    added SPROX_TclB_... functions
 *   JDA 08/09/2005 : [version 1.33]
 *                    reader-side changes only
 *   JDA 02/10/2005 : [version 1.34 0]
 *                    added SPROX_Card_PowerUp_Auto function
 *   JDA 14/10/2005 : [version 1.34 1]
 *                    added documentation for SPROX_Card_... functions
 *   LTX 12/04/2006 : [version 1.36 3]
 *                    added entry points for GetConsts and SetConsts
 *   JDA 24/05/2006 : [version 1.36 5]
 *                    extended USB support
 *                    every text-related function now has three version : xxx using TCHAR, xxxA and xxxW
 *                    using char (ASCII) and wchar_t (UNICODE) respectively.
 *   JDA 22/06/2006 : [version 1.40]
 *                    major rewriting :
 *                    - whole "multi-reader" stuff moved to a fully reentrant DLL (springprox_ex.dll)
 *                    - documentation moved to source code
 *                    - crypt part (DES, 3-DES and MD5) added
 *   JDA 15/01/2007 : [version 1.41]
 *                    improved Linux support
 *   JDA 10/10/2007 : [version 1.42]
 *                    added Mifare read/write functions with key_idx
 *                    SPROX_MifStAuthE2 and SPROX_MifStAuthRam or deprecated
 *                    added GetConstsEx and SetConstsEx for FEED
 *   JDA 09/01/2008 : [version 1.43]
 *                    created SPROX_WriteRCRegister
 *   JDA 19/02/2008 : [version 1.44]
 *                    added SPROX_Bi_... functions for Calypso
 *   LTC 26/02/2008 : [version 1.45]
 *                    added SPROX_15693_... functions for ISO 15693
 *                    added SPROX_I1_SelectAny for ICODE1
 *   JDA 24/10/2008 : [version 1.48]
 *                    added SPROX_ControlBuzzer
 *                    added SPROX_Bi_SamFastSpeed
 *   JDA 25/05/2009 : [version 1.51]
 *                    removed MD5 and DES/3DES stuff (moved to Desfire API)
 *   JDA 30/12/2009 : [version 1.53]
 *                    added SPROX_Find and SPROX_FindEx
 *   JDA 23/06/2010 : [version 1.54]
 *                    added SPROX_FindWait and SPROX_FindWaitEx
 *   JDA 25/01/2011 : [version 1.55]
 *                    moved defines of MI_xxx to springprox_errno.h
 *                    added SPROX_A_ExchangeRaw
 *   JDA 15/02/2011 : added SPROX_ReadStorage, SPROX_WriteStorage and SPROX_StorageSize
 *   JDA 20/04/2011 : [version 1.56]
 *                    modified ISO 15693 support
 *   JDA 20/01/2012 : [version 1.62]
 *                    re-implemented SPROX_ControlReadModeIO
 *                    separated USER I/O support in SPROX_ControlReadUserIO and SPROX_ControlWriteUserIO
 *   JDA 28/02/2012 : [version 1.63]
 *                    removed SPROX_I1_... function, added SPROX_ICode1_... instead
 *   JDA 30/09/2012 : [version 1.70]
 *                    prepared for NFC, changed a few defines
 *   JDA 04/02/2013 : [version 1.71]
 *                    max length of UIDs is now 32
 *                    added MifStIncrementCounter and MifStRestoreCounter
 *   JDA 04/12/2013 : [version 1.76]
 *                    added SPROX_FindLpcd and SPROX_FindLpcdEx
 *   JDA 22/04/2014 : [version 1.79]
 *                    added SPROX_Iso15693_Exchange and related
 *   JDA 29/09/2014 : [version 1.80]
 *                    version fully validated over K663
 *   JDA 28/04/2017 : [version 1.81]
 *                    added SPROX_Iso15693_Extended... and related 
 *                    new protocol: PicoTag 15693 (new iClass)
 *   JDA 18/05/2017 : SPROX_Iso15693_Halt was missing
 *   JDA 05/04/2018 : [version 1.90]
 *                    more flexibility introduced in protocol selection (including ability to force a protocol by a registry entry)
 *
 * PORTABILITY
 *   Win32
 *   WinCE
 *   Linux
 *
 **/
#ifndef SPRINGPROX_H
#define SPRINGPROX_H

#if (defined(UNDER_CE) || defined(_WIN32_WCE))
  /* WinCE is only a subset of Win32 */
  #ifndef WIN32
    #define WIN32
  #endif
  #ifndef WINCE
    #define WINCE
  #endif
#endif

#if (defined (_WIN32))
  #ifndef WIN32
    #define WIN32
  #endif
#endif

#ifdef WIN32

  /* Win32 code */
  /* ---------- */

  #include <windows.h>
  #include <tchar.h>

  typedef signed char  SBYTE;
  typedef signed short SWORD;
  typedef signed long  SDWORD;

  #ifndef SPRINGPROX_LIB
    /* We are to link to the DLL */
    #define SPRINGPROX_LIB __declspec( dllimport )
  #endif

  #if (defined(WINCE))
    /* Under Windows CE we use the stdcall calling convention */
    #define SPRINGPROX_API __stdcall
  #else  
    #if (defined(FORCE_STDCALL))
      /* stdcall is forced */
      #define SPRINGPROX_API __stdcall
    #else
      /* cdecl is the default calling convention */
      #define SPRINGPROX_API __cdecl
    #endif
  #endif

#else

  /* Not Win32 */
  /* --------- */
  
  /* Linkage directive : not specified, use compiler default */
  #define SPRINGPROX_API
  
  /* Calling convention : not specified, use compiler default */
  #define SPRINGPROX_LIB
  
  #include <stdint.h>
  #include <stddef.h>
  
  #ifndef HAS_LIB_C_TYPES

    typedef uint8_t  BOOL;
    typedef uint8_t  BYTE;
    typedef int8_t   SBYTE;
    typedef uint16_t WORD;
    typedef int16_t  SWORD;
    typedef uint32_t DWORD;
    typedef int32_t  SDWORD;
    typedef signed long  LONG;
    
    #ifndef TRUE
      #define TRUE 1
    #endif
    #ifndef FALSE
      #define FALSE 0
    #endif
    
    #ifdef HAVE_TCHAR_H
      #include <tchar.h>
    #else
      #ifdef UNICODE
        typedef wchar_t TCHAR;
      #else
        typedef char TCHAR;
      #endif
    #endif

  #endif
  
#endif


#ifdef __cplusplus
/* SpringProx API is pure C */
extern  "C"
{
#endif

/* Library identification */
/* ---------------------- */

SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_GetLibrary(TCHAR library[], WORD len);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_GetLibraryW(wchar_t library[], WORD len);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_GetLibraryA(char library[], WORD len);

/* Return code to error message translation */
/* ---------------------------------------- */
SPRINGPROX_LIB const TCHAR   *SPRINGPROX_API SPROX_GetErrorMessage(SWORD status);
SPRINGPROX_LIB const wchar_t *SPRINGPROX_API SPROX_GetErrorMessageW(SWORD status);
SPRINGPROX_LIB const char    *SPRINGPROX_API SPROX_GetErrorMessageA(SWORD status);

SPRINGPROX_LIB void  SPRINGPROX_API SPROX_SetVerbose(BYTE level, const TCHAR *filename);

/* Reader access functions */
/* ----------------------- */

/* Open the reader (this library is not reentrant, we work with one reader at a time   */
/* Set device to the communication port name ("COM1", "USB", "/dev/ttyS2" ...) or NULL */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderOpen(const TCHAR device[]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderOpenW(const wchar_t device[]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderOpenA(const char device[]);

/* Close the reader */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderClose(void);

/* Suspend the reader, without destroying the handle to access it */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderDeactivate(void);

/* Resume the reader */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderActivate(void);

/* Discover the reader on a previously opened communication port */
#ifdef WIN32
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderAttachSerial(HANDLE hComm);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderAttachUSB(HANDLE hComm);
#endif

/* Enumerate the compliant devices found on USB */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_EnumUSBDevices(DWORD idx, TCHAR device[64], TCHAR description[64]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_EnumUSBDevicesW(DWORD idx, wchar_t device[64], wchar_t description[64]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_EnumUSBDevicesA(DWORD idx, char device[64], char description[64]);

/* Select the address (RS-485 bus mode only) */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderSelectAddress(BYTE address);

/* Reader information functions */
/* ---------------------------- */

/* Retrieve name of the communication port */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderGetDevice(TCHAR device[], WORD len);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderGetDeviceW(wchar_t device[], WORD len);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderGetDeviceA(char device[], WORD len);

/* Retrieve reader's firmware (type - version) */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderGetFirmware(TCHAR firmware[], WORD len);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderGetFirmwareW(wchar_t firmware[], WORD len);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderGetFirmwareA(char firmware[], WORD len);

/* Retrieve actual features of reader's firmware */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderGetFeatures(DWORD *features);

/* Retrieve communication settings between host and reader */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderGetDeviceSettings(DWORD *settings);
/* Select communication settings between host and reader */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderSetDeviceSettings(DWORD settings);

/* Bitmasks for SPROX_ReaderGetDeviceSettings and SPROX_ReaderSetDeviceSettings */
#define SPROX_SETTINGS_PROTOCOL_MASK       0x00000003
#define SPROX_SETTINGS_PROTOCOL_OSI        0x00000000
#define SPROX_SETTINGS_PROTOCOL_ASCII      0x00000001
#define SPROX_SETTINGS_PROTOCOL_BIN        0x00000002
#define SPROX_SETTINGS_PROTOCOL_BUS        0x00000003
#define SPROX_SETTINGS_HARDWARE_CTRL       0x00000004
#define SPROX_SETTINGS_BAUDRATE_MASK       0x00000008
#define SPROX_SETTINGS_BAUDRATE_38400      0x00000000
#define SPROX_SETTINGS_BAUDRATE_115200     0x00000008
#define SPROX_SETTINGS_CHANNEL_MASK        0x00000060
#define SPROX_SETTINGS_CHANNEL_RS232       0x00000000
#define SPROX_SETTINGS_CHANNEL_RS485       0x00000020
#define SPROX_SETTINGS_CHANNEL_USB         0x00000040
#define SPROX_SETTINGS_CHANNEL_TCP         0x00000060
#define SPROX_SETTINGS_FORCE_CHANNEL_RS485 0x00000020
#define SPROX_SETTINGS_FORCE_BAUDRATE      0x00000010
#define SPROX_SETTINGS_FORCE_PROTOCOL      0x00000004

/* Read configuration constants of the reader */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderGetConsts(DWORD *consts);
/* Write configuration constants of the reader */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderSetConsts(DWORD consts);
/* Read advanced configuration constants of the reader */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderGetConstsEx(BYTE ident, BYTE consts[], WORD *length);
/* Write advanced configuration constants of the reader */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderSetConstsEx(BYTE ident, const BYTE consts[], WORD length);

/* Miscellaneous reader functions */
/* ------------------------------ */

/* Restart the reader */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderRestart(void);

/* Reset the reader */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderReset(void);

/* Drive the LEDs */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ControlLed(BYTE led_r, BYTE led_g);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ControlLedY(BYTE led_r, BYTE led_g, BYTE led_y);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ControlLeds(WORD leds);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ControlLedsT(WORD leds, WORD ms);

#define LED_OFF             0x00
#define LED_ON              0x01
#define LED_BLINK_SLOW      0x02
#define LED_AUTO            0x03
#define LED_BLINK_FAST      0x04
#define LED_HEART_BEAT      0x05
#define LED_BLINK_SLOW_INV  0x06
#define LED_BLINK_FAST_INV  0x07
#define LED_BLINK_HEART_INV 0x08

/* Drive the buzzer */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ControlBuzzer(WORD time_ms);

/* Read the MODE pin */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ControlReadModeIO(BOOL *in_value);

/* Read or write the USER pin */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ControlReadUserIO(BOOL *in_value);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ControlWriteUserIO(BOOL out_value);

/* Drive the RF field */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ControlRF(BYTE param);

/* Configure the reader for ISO/IEC 14443-A or 14443-B or 15693 or ICODE1 operation */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_SetConfig(BYTE mode);
#define CFG_MODE_ISO_14443_A          0x01
#define CFG_MODE_ISO_14443_B          0x02
#define CFG_MODE_ISO_14443_Bi         0x03
#define CFG_MODE_ISO_15693            0x04
#define CFG_MODE_ICODE1               0x05
#define CFG_MODE_INSIDE_PICO_14443    0x10
#define CFG_MODE_INSIDE_PICO_15693    0x13
#define CFG_MODE_INSIDE_PICO CFG_MODE_INSIDE_PICO_14443

/* Retrieve RF chipset's information (NXP RC type and serial number) */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReaderGetRc500Id(BYTE micore_type[5], BYTE micore_snr[4]);

/* Send a raw control command to the reader */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ControlEx(const BYTE send_buffer[], WORD send_bytelen, BYTE recv_buffer[], WORD *recv_bytelen);

/* Send a raw command to the reader, and receive its response */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Function(BYTE cmd, const BYTE send_buffer[], WORD send_bytelen, BYTE recv_buffer[], WORD *recv_bytelen);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_FunctionWaitResp(BYTE recv_buffer[], WORD *recv_bytelen, WORD timeout_s);

/* Test communication with the reader */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Echo(WORD len);

/* Access to reader's non-volatile memory */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ReadStorage(DWORD address, BYTE buffer[], WORD length);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_WriteStorage(DWORD address, const BYTE buffer[], WORD length);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_StorageSize(DWORD *size);

/* Low level registry access */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_WriteRCRegister(BYTE reg, BYTE value);

/* ISO/IEC 14443-A functions */
/* ------------------------- */


/* Layer 3 */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_A_Request(BYTE req_code, BYTE atq[2]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_A_RequestAny(BYTE atq[2]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_A_RequestIdle(BYTE atq[2]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_A_SelectAny(BYTE atq[2], BYTE snr[10], BYTE *snrlen, BYTE sak[1]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_A_SelectIdle(BYTE atq[2], BYTE snr[10], BYTE *snrlen, BYTE sak[1]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_A_SelectAgain(const BYTE snr[10], BYTE snrlen);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_A_Halt(void);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_A_Exchange(const BYTE send_buffer[], WORD send_bytelen, BYTE recv_buffer[], WORD *recv_bytelen, BYTE append_crc, WORD timeout);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_A_ExchangeRaw(const BYTE send_buffer[], WORD send_bitslen, BYTE recv_buffer[], WORD *recv_bitslen, WORD timeout);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_A_ExchangeRawEx(BYTE param1, BYTE param2, const BYTE send_data[], WORD send_bitslen, BYTE recv_data[], WORD *recv_bitslen, WORD timeout);
SPRINGPROX_LIB WORD SPRINGPROX_API  SPROX_ComputeIso14443ACrc(BYTE crc[2], const BYTE buffer[], WORD size);


/* Layer 4 (T=CL) */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclA_ActivateAny(BYTE atq[2], BYTE snr[10], BYTE *snrlen, BYTE sak[1]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclA_ActivateIdle(BYTE atq[2], BYTE snr[10], BYTE *snrlen, BYTE sak[1]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclA_ActivateAgain(const BYTE snr[10], BYTE snrlen);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclA_Halt(void);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclA_Deselect(BYTE cid);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclA_GetAts(BYTE cid, BYTE ats[32], BYTE *atslen);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclA_Pps(BYTE cid, BYTE dsi, BYTE dri);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclA_Exchange(BYTE fsci, BYTE cid, BYTE nad, const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD *recv_len);

/* ISO/IEC 14443-B functions */
/* ------------------------- */

/* Layer 3 */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_B_SelectAny(BYTE afi, BYTE atq[11]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_B_SelectIdle(BYTE afi, BYTE atq[11]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_B_AnticollAny(BYTE slots, BYTE afi, BYTE atq[11]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_B_AnticollIdle(BYTE slots, BYTE afi, BYTE atq[11]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_B_AnticollSlot(BYTE slot, BYTE atq[11]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_B_Exchange(const BYTE send_buffer[], WORD send_bytelen, BYTE recv_buffer[], WORD *recv_bytelen, BYTE append_crc, WORD timeout);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_B_Halt(const BYTE pupi[4]);
SPRINGPROX_LIB WORD SPRINGPROX_API  SPROX_ComputeIso14443BCrc(BYTE crc[2], const BYTE buffer[], WORD size);

/* Layer 4 (T=CL) */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclB_ActivateAny(BYTE afi, BYTE atq[11]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclB_ActivateIdle(BYTE afi, BYTE atq[11]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclB_Attrib(const BYTE atq[11], BYTE cid);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclB_AttribEx(const BYTE atq[11], BYTE cid, BYTE dsi, BYTE dri);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclB_Deselect(BYTE cid);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclB_Halt(const BYTE pupi[4]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_TclB_Exchange(BYTE fsci, BYTE cid, BYTE nad, const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD *recv_len);

/* ISO/IEC 14443 type independant functions */
/* ------------------------------------------ */

SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Tcl_Exchange(BYTE cid, const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD *recv_len);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Tcl_Deselect(BYTE cid);

/* B' (Innovatron) */
/* --------------- */

/* Those function are only avalaible for Calypso-enabled readers */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Bi_Apgen(BYTE uid[4], BYTE atr[32], BYTE *atrlen);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Bi_Attrib(BYTE uid[4]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Bi_Exchange(const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD *recv_len);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Bi_Disc(void);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Bi_SamFastSpeed(BYTE card_slot);

/* ISO/IEC 15693 functions */
/* ----------------------- */

SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Iso15693_SelectAny(BYTE afi, BYTE snr[8]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Iso15693_SelectAgain(BYTE snr[8]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Iso15693_Halt(void);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Iso15693_ReadSingleBlock(BYTE snr[8], BYTE addr, BYTE data[], WORD *datalen);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Iso15693_WriteSingleBlock(BYTE snr[8], BYTE addr, const BYTE data[], WORD datalen);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Iso15693_LockBlock(BYTE snr[8], BYTE addr);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Iso15693_ReadMultipleBlocks(BYTE snr[8], BYTE addr, BYTE count, BYTE data[], WORD *datalen);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Iso15693_WriteMultipleBlocks(BYTE snr[8], BYTE addr, BYTE count, const BYTE data[], WORD datalen);
// SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Iso15693_WriteAFI(BYTE snr[8], BYTE afi);
// SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Iso15693_LockAFI(BYTE snr[8]);
// SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Iso15693_WriteDSFID(BYTE snr[8], BYTE afi);
// SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Iso15693_LockDSFID(BYTE snr[8]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Iso15693_GetSystemInformation(BYTE snr[8], BYTE data[], WORD *datalen);
// SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Iso15693_GetMultipleBlockSecurityStatus(BYTE snr[8], BYTE addr, BYTE count, BYTE data[], WORD *datalen);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Iso15693_ExtendedReadSingleBlock(BYTE snr[8], WORD addr, BYTE data[], WORD *datalen);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Iso15693_ExtendedWriteSingleBlock(BYTE snr[8], WORD addr, const BYTE data[], WORD datalen);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Iso15693_ExtendedLockBlock(BYTE snr[8], WORD addr);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Iso15693_ExtendedReadMultipleBlocks(BYTE snr[8], WORD addr, WORD count, BYTE data[], WORD *datalen);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Iso15693_ExtendedWriteMultipleBlocks(BYTE snr[8], WORD addr, WORD count, const BYTE data[], WORD datalen);
// SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Iso15693_ExtendedGetMultipleBlockSecurityStatus(BYTE snr[8], WORD addr, WORD count, BYTE data[], WORD *datalen);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Iso15693_Exchange(const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD *recv_len, WORD timeout);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Iso15693_ExchangeStdCommand(BOOL opt_flag, BYTE snr[8], BYTE cmd_opcode, const BYTE cmd_params[], WORD cmd_params_len, BYTE recv_buffer[], WORD *recv_len, WORD timeout);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Iso15693_ExchangeCustomCommand(BOOL opt_flag, BYTE mfg_id, BYTE snr[8], BYTE cmd_opcode, const BYTE cmd_params[], WORD cmd_params_len, BYTE recv_buffer[], WORD *recv_len, WORD timeout);

/* ICODE1 functions */
/* ---------------- */

SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ICode1_SelectAny(BYTE afi, BYTE snr[8]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ICode1_UnselectedRead(BYTE addr, BYTE count, BYTE data[], WORD *datalen);

/* Inside Pico family/HID iClass */
/* ----------------------------- */

SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Pico_Exchange14443(const BYTE *send_data, WORD send_bytelen, BYTE *recv_data, WORD *recv_bytelen, BYTE append_crc, WORD timeout);
#define SPROX_Pico_Exchange SPROX_Pico_Exchange14443
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Pico_Exchange15693(const BYTE *send_data, WORD send_bytelen, BYTE *recv_data, WORD *recv_bytelen, BYTE append_crc, WORD timeout);

/* Find functions */
/* -------------- */

SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Find(WORD want_protos, WORD *got_proto, BYTE uid[32], BYTE *uidlen);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_FindEx(WORD want_protos, WORD *got_proto, BYTE uid[32], BYTE *uidlen, BYTE info[32], BYTE *infolen);

SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_FindWait(WORD want_protos, WORD *got_proto, BYTE uid[32], BYTE *uidlen, WORD timeout_s, WORD interval_ms);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_FindWaitEx(WORD want_protos, WORD *got_proto, BYTE uid[32], BYTE *uidlen, BYTE info[32], BYTE *infolen, WORD timeout_s, WORD interval_ms);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_FindWaitCancel(void);

SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_FindLpcd(WORD want_protos, WORD *got_proto, BYTE uid[32], BYTE *uidlen, WORD timeout_s);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_FindLpcdEx(WORD want_protos, WORD *got_proto, BYTE uid[32], BYTE *uidlen, BYTE info[32], BYTE *infolen, WORD timeout_s, BOOL forced_pulses);

SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_NfcI_Exchange(BYTE did, const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD *recv_len);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_NfcI_Atr(BYTE did, const BYTE gi[], WORD gi_len, BYTE nfcid3t[10], BYTE gt[], WORD *gt_len);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_NfcI_Dsl(BYTE did);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_NfcI_Rls(BYTE did);


#define PROTO_14443_A              0x0001
#define PROTO_14443_B              0x0002
#define PROTO_15693                0x0004
#define PROTO_ICODE1               0x0008
#define PROTO_INSIDE_PICO_14443    0x0010
#define PROTO_INSIDE_PICO PROTO_INSIDE_PICO_14443
#define PROTO_ST_SR                0x0020
#define PROTO_ASK_CTS              0x0040
#define PROTO_INNOVATRON           0x0080
#define PROTO_14443_Bi             PROTO_INNOVATRON
#define PROTO_CALYPSO              PROTO_INNOVATRON
#define PROTO_INNOVISION_JEWEL     0x0400
#define PROTO_KOVIO_BARCODE        0x0800
#define PROTO_FELICA               0x1000
#define PROTO_INSIDE_PICO_15693    0x2000

#define PROTO_ANY                  0xFFFF


/* Legacy Mifare functions */
/* ----------------------- */

/* Mifare is a registered trademark of Philips.                            */
/* Please refer to Philips documentation for any explanation on this part. */

SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStSelectAny(BYTE snr[10], BYTE atq[2], BYTE sak[1]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStSelectIdle(BYTE snr[10], BYTE atq[2], BYTE sak[1]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStSelectAgain(const BYTE snr[4]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStHalt(void);

/* Mifare read, without authentication (Mifare UltraLight cards) */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifRead(const BYTE snr[4], BYTE addr, BYTE data[16]);

/* Mifare write, without authentication (Mifare UltraLight cards) */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifWrite(const BYTE snr[4], BYTE addr, const BYTE data[16]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifWrite4(const BYTE snr[4], BYTE addr, const BYTE data[4]);

/* Mifare standard authenticate and read */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStReadBlock(const BYTE snr[4], BYTE block, BYTE data[16], const BYTE key_val[6]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStReadBlock2(const BYTE snr[4], BYTE block, BYTE data[16], BYTE key_idx);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStReadSector(const BYTE snr[4], BYTE sector, BYTE data[240], const BYTE key_val[6]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStReadSector2(const BYTE snr[4], BYTE sector, BYTE data[240], BYTE key_idx);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStReadTag768(const BYTE snr[4], WORD *sectors, BYTE data[768]);

/* Mifare standard authenticate and write */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStWriteBlock(const BYTE snr[4], BYTE block, const BYTE data[16], const BYTE key_val[6]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStWriteBlock2(const BYTE snr[4], BYTE block, const BYTE data[16], BYTE key_idx);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStWriteSector(const BYTE snr[4], BYTE sector, const BYTE data[240], const BYTE key_val[6]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStWriteSector2(const BYTE snr[4], BYTE sector, const BYTE data[240], BYTE key_idx);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStWriteTag768(const BYTE snr[4], WORD *sectors, const BYTE data[768]);

/* Mifare standard counter manipulation */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStReadCounter(const BYTE snr[4], BYTE block, SDWORD *value, const BYTE key_val[6]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStReadCounter2(const BYTE snr[4], BYTE block, SDWORD *value, BYTE key_idx);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStWriteCounter(const BYTE snr[4], BYTE block, SDWORD value, const BYTE key_val[6]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStWriteCounter2(const BYTE snr[4], BYTE block, SDWORD value, BYTE key_idx);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStDecrementCounter(const BYTE snr[4], BYTE block, DWORD step, const BYTE key_val[6]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStDecrementCounter2(const BYTE snr[4], BYTE block, DWORD step, BYTE key_idx);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStIncrementCounter(const BYTE snr[4], BYTE block, DWORD step, const BYTE key_val[6]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStIncrementCounter2(const BYTE snr[4], BYTE block, DWORD step, BYTE key_idx);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStRestoreCounter(const BYTE snr[4], BYTE src_block, BYTE dst_block, const BYTE key_val[6]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStRestoreCounter2(const BYTE snr[4], BYTE src_block, BYTE dst_block, BYTE key_idx);

/* Mifare standard sector trailers formatting */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStUpdateAccessBlock(const BYTE snr[4], BYTE sect, const BYTE old_key_val[6], const BYTE new_key_A[6], const BYTE new_key_B[6], BYTE ac0, BYTE ac1, BYTE ac2, BYTE ac3);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStUpdateAccessBlock2(const BYTE snr[4], BYTE sect, BYTE old_key_idx, const BYTE new_key_A[6], const BYTE new_key_B[6], BYTE ac0, BYTE ac1, BYTE ac2, BYTE ac3);
/* Valid access conditions for ac0, ac1 and ac2 */
#define ACC_BLOCK_TRANSPORT 0x00
#define ACC_BLOCK_DATA      0x04
#define ACC_BLOCK_VALUE ACC_BLOCK_DATA
#define ACC_BLOCK_COUNTER   0x06
/* Valid access conditions for ac3 */
#define ACC_AUTH_NORMAL     0x03
#define ACC_AUTH_TRANSPORT  0x01

/* Perform Mifare standard authentication */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStAuthKey(BYTE auth_mode, const BYTE snr[4], const BYTE key_val[6], BYTE block);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStAuthKey2(const BYTE snr[4], BYTE key_idx, BYTE block);

SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStAuthE2(BYTE auth_mode, const BYTE snr[4], BYTE key_sector, BYTE block);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifStAuthRam(BYTE auth_mode, BYTE key_sector, BYTE block);

/* Load a Mifare standard key into reader's memory (RAM or EEPROM) */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifLoadKey(BOOL eeprom, BYTE key_type, BYTE key_offset, const BYTE key_val[6]);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifLoadKeyEx(BYTE key_idx, const BYTE key_val[6]);

/* Who is the last authentication key successfully used ? */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MifLastAuthKey(BYTE * info);
/* Related defines */
#define MIF_RAM_KEY     0x80
#define MIF_E2_KEY      0x40
#define MIF_CODED_KEY   0xC0
#define MIF_KEY_A       0x00
#define MIF_KEY_B       0x20

/* Smartcard related functions (readers with embedded GemPlus GemCore smartcard reader) */
/* ------------------------------------------------------------------------------------ */

/* Power up the card in a slot */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Card_PowerUp(BYTE slot, BYTE config, BYTE atr[32], WORD *atr_len);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Card_PowerUp_Auto(BYTE slot, BYTE atr[32], WORD *atr_len);

/* Power down the card in a slot */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Card_PowerDown(BYTE slot);

/* Exchange with the card in a slot */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Card_Exchange(BYTE slot, const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD *recv_len);

/* Get status of a slot */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Card_Status(BYTE slot, BYTE *stat, BYTE *type, BYTE config[4]);

/* Configure / retrieve actual configuration of a slot */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Card_SetConfig(BYTE slot, BYTE mode, BYTE type);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Card_GetConfig(BYTE slot, BYTE *mode, BYTE *type);

/* Retrieve smartcard coupler's firmware (type - version) */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Card_GetFirmware(TCHAR firmware[], WORD len);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Card_GetFirmwareW(wchar_t firmware[], WORD len);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Card_GetFirmwareA(char firmware[], WORD len);

/* Transparent exchange between host and smartcard coupler */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Card_Control(const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD *recv_len);

/* Fingerprint related functions (reader with embedded Sagem MorphoSmart CBM module) */
/* --------------------------------------------------------------------------------- */

/* Power up the MorphoSmart */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MSO_Open(TCHAR *mso_product, TCHAR *mso_firmware, TCHAR *mso_sensor);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MSO_OpenW(wchar_t *mso_product, wchar_t *mso_firmware, wchar_t *mso_sensor);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MSO_OpenA(char *mso_product, char *mso_firmware, char *mso_sensor);

/* Power down the MorphoSmart */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MSO_Close(void);

/* Transparent exchange between host and MorphoSmart */
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_MSO_Exchange(const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD *recv_len, DWORD timeout, BYTE *async);

/* Miscelleanous utilities, helpers for VB or Delphi users */
/* ------------------------------------------------------- */

SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Malloc(BYTE **buffer, WORD size);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_Free(BYTE * buffer);
SPRINGPROX_LIB WORD  SPRINGPROX_API SPROX_StrLen(const TCHAR *buffer);
SPRINGPROX_LIB WORD  SPRINGPROX_API SPROX_StrLenW(const wchar_t *buffer);
SPRINGPROX_LIB WORD  SPRINGPROX_API SPROX_StrLenA(const char *buffer);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ArrayToString(TCHAR *string, const BYTE *buffer, WORD size);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ArrayToStringW(wchar_t *string, const BYTE *buffer, WORD size);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ArrayToStringA(char *string, const BYTE *buffer, WORD size);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_StringToArray(BYTE *buffer, const TCHAR *string, WORD size);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_StringToArrayW(BYTE *buffer, const wchar_t *string, WORD size);
SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_StringToArrayA(BYTE *buffer, const char *string, WORD size);

/* Philips basic function set */
/* -------------------------- */

/* Those functions are copied from Philips "Pegoda" low-level API */
/* For better performance, DO NOT USE those low level calls.      */ 

/* Modes for 'Mf500InterfaceOpen' */
#define USB         0x00000031
#define RS232       0x00000040
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500InterfaceOpen(DWORD mode, DWORD unused);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500InterfaceClose(void);

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PcdConfig(void);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PcdSetDefaultAttrib(void);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PcdSetAttrib(BYTE DSI, BYTE DRI);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PcdGetAttrib(BYTE *FSCImax, BYTE *FSDImax, BYTE *DSsupp, BYTE *DRsupp, BYTE *DREQDS);

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccRequest(BYTE req_code, BYTE *atq);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccCommonRequest(BYTE req_code, BYTE *atq);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccAnticoll(BYTE bcnt, BYTE *snr);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccCascAnticoll(BYTE select_code, BYTE bcnt, BYTE *snr);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccSelect(const BYTE *snr, BYTE *sak);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccCascSelect(BYTE sel_code, const BYTE *snr, BYTE *sak);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccActivateWakeup(BYTE br, BYTE *atq, BYTE *sak, const BYTE *uid, BYTE uid_len);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccActivateIdle(BYTE br, BYTE *atq, BYTE *sak, BYTE *uid, BYTE *uid_len);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccAuth(BYTE auth_mode, BYTE key_sector, BYTE block);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccAuthRam(BYTE auth_mode, BYTE key_sector, BYTE block);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccAuthE2(BYTE auth_mode, BYTE *snr, BYTE key_sector, BYTE block);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccAuthKey(BYTE auth_mode, const BYTE *snr, BYTE *keys, BYTE block);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccRead(BYTE addr, BYTE *data);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccCommonRead(BYTE cmd, BYTE addr, BYTE datalen, BYTE *data);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccWrite(BYTE addr, BYTE *data);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccWrite4(BYTE addr, BYTE *data);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccCommonWrite(BYTE cmd, BYTE addr, BYTE datalen, BYTE *data);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccValue(BYTE dd_mode, BYTE addr, BYTE *value, BYTE trans_addr);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccValueDebit(BYTE dd_mode, BYTE addr, BYTE *value);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccHalt(void);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccExchangeBlock(BYTE *send_data, WORD send_bytelen, BYTE *rec_data, WORD *rec_bytelen, BYTE append_crc, DWORD timeout);

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500HostCodeKey(BYTE *uncoded, BYTE *coded);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PcdLoadKeyE2(BYTE key_type, BYTE sector, BYTE *uncoded_keys);
SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PcdLoadKeyRam(BYTE key_type, BYTE sector, BYTE *uncoded_keys);

SPRINGPROX_LIB SWORD SPRINGPROX_API PcdSetTmo(DWORD tmoLength);
SPRINGPROX_LIB SWORD SPRINGPROX_API PcdGetSnr(BYTE *snr);
SPRINGPROX_LIB SWORD SPRINGPROX_API PcdReadE2(WORD startaddr, BYTE length, BYTE *data);
SPRINGPROX_LIB SWORD SPRINGPROX_API PcdWriteE2(WORD startaddr, BYTE length, BYTE *data);
SPRINGPROX_LIB SWORD SPRINGPROX_API PcdReset(void);

SPRINGPROX_LIB SWORD SPRINGPROX_API PcdSetIdleMode(void);
SPRINGPROX_LIB SWORD SPRINGPROX_API PcdClearIdleMode(void);

SPRINGPROX_LIB SWORD SPRINGPROX_API ExchangeByteStream(BYTE cmd, BYTE *send_data, WORD send_bytelen, BYTE *rec_data, WORD *rec_bytelen);
SPRINGPROX_LIB SWORD SPRINGPROX_API ReadRIC(BYTE reg, BYTE *value);
SPRINGPROX_LIB SWORD SPRINGPROX_API WriteRIC(BYTE reg, BYTE value);
SPRINGPROX_LIB SWORD SPRINGPROX_API SetRCBitMask(BYTE reg, BYTE mask);
SPRINGPROX_LIB SWORD SPRINGPROX_API ClearRCBitMask(BYTE reg, BYTE mask);

#ifdef __cplusplus
}
#endif

/* Error codes */
/* ----------- */

#include "springprox_errno.h"

#endif
