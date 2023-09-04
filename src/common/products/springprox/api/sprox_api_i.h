/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  SpringProx API
  --------------

  Copyright (c) 2000-2008 SpringCard SAS, FRANCE - www.springcard.com

  sprox_api_i.h
  --------------
  Common header included by each source file of the library.

*/
#ifndef SPROX_API_I_H
#define SPROX_API_I_H

#ifndef SPROX_API_REENTRANT
/*
 * We are building the classical SpringProx API
 * --------------------------------------------
 */

 /* Function are named SPROX_.... */
#define SPROX_API_FUNC(a)        SPRINGPROX_LIB SWORD SPRINGPROX_API SPROX_ ## a
#define SPROX_API_FUNC_T(a, b)   SPRINGPROX_LIB a     SPRINGPROX_API SPROX_ ## b

/* Redirect function calls to this name */
#define SPROX_API_CALL(a)  SPROX_ ## a  

/* The instance parameter doesn't exist */
#define SPROX_PARAM
#define SPROX_PARAM_V      void
#define SPROX_PARAM_P
#define SPROX_PARAM_PV

/* When needed the instance is emulated by a pointer to a global variable */
#define SPROX_PARAM_TO_CTX SPROX_CTX_ST *sprox_ctx = sprox_ctx_glob; (void) sprox_ctx;

/* This is the low-level dialog function */
#define SPROX_DLG_FUNC     SPROX_Function
#define SPROX_WRS_FUNC     SPROX_FunctionWaitResp

#else
/*
 * We are building the SpringProx "EX" API
 * ---------------------------------------
 */

 /* Function are renamed SPROXx_.... */
#define SPROX_API_FUNC(a)        SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ ## a
#define SPROX_API_FUNC_T(a, b)   SPRINGPROX_LIB a     SPRINGPROX_API SPROXx_ ## b

/* Redirect function calls to this name */
#define SPROX_API_CALL(a)  SPROXx_ ## a  

/* Most functions need an instance parameter */
#define SPROX_PARAM        SPROX_INSTANCE rInst, 
#define SPROX_PARAM_V      SPROX_INSTANCE rInst
#define SPROX_PARAM_P      rInst,
#define SPROX_PARAM_PV     rInst

/* This is the routine check on top of each function */
#define SPROX_PARAM_TO_CTX SPROX_CTX_ST *sprox_ctx = (SPROX_CTX_ST *) rInst; if (sprox_ctx == NULL) return MI_INVALID_READER_CONTEXT;

/* This is the low-level dialog function */
#define SPROX_DLG_FUNC     SPROXx_Function
#define SPROX_WRS_FUNC     SPROXx_FunctionWaitResp

#endif

#ifdef UNDER_CE
#ifndef WIN32
#define WIN32
#endif
#endif

#ifdef __linux__
#ifndef LINUX
#define LINUX
#endif
#endif

/* We are building the library */
/* --------------------------- */
#ifdef WIN32

  /* Windows */
#include <windows.h>
#define SPRINGPROX_LIB __declspec( dllexport )

#include "../../../lib-c/utils/strl.h"

#endif

#ifdef LINUX

#include <termios.h>
#include <unistd.h>  

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

#define _stprintf sprintf
#define _tcscat strcat
#define _tcsicmp strcmp
#define _tcsncmp strncmp
#define _tcsncpy strncpy
#define _tcscpy strcpy
#define _tcslen strlen
#define _tcsclen strlen
#define _tfopen fopen
#define _ftprintf fprintf
#define _T(x) x

#define Sleep(x) usleep(1000*x)

#endif

#define UNUSED_PARAMETER(x) (void) x

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Main library header */
/* ------------------- */

#include "springprox.h"

#ifdef SPROX_API_REENTRANT
#include "springprox_ex.h"
#endif

#define COM_PROTO_OSI3964               0x00000001
#define COM_PROTO_ASCII                 0x00000002
#define COM_PROTO_BIN                   0x00000004
#define COM_PROTO_BUS                   0x00000008
#define COM_PROTO_MASK                  0x000000FF
#define COM_PROTO_SHIFT                 0

#define COM_BAUDRATE_38400              0x00000100
#define COM_BAUDRATE_115200             0x00000200
#define COM_BAUDRATE_MASK               0x0000FF00
#define COM_BAUDRATE_SHIFT              8

#define COM_OPTION_POWER_AUTO           0x00010000
#define COM_OPTION_SERIAL_POWER_ON_DTR  0x00020000
#define COM_OPTION_SERIAL_RESET_ON_RTS  0x00040000
#define COM_OPTION_SERIAL_RS485         0x00800000
#define COM_OPTION_MASK                 0x00FF0000
#define COM_OPTION_SHIFT                16

#define COM_INTERFACE_SERIAL            0x01000000
#define COM_INTERFACE_FTDI              0x03000000
#define COM_INTERFACE_TCP               0x10000000
#define COM_INTERFACE_MASK              0xFF000000
#define COM_INTERFACE_SHIFT             24

#define COM_STATUS_CLOSED               0
#define COM_STATUS_CLOSED_BUT_SEEN      1
#define COM_STATUS_OPEN_IDLE            2
#define COM_STATUS_OPEN_ACTIVE          3

/* The internal context structure */
/* ------------------------------ */
struct _SPROX_CTX_ST
{
	DWORD   com_settings_allowed;

	DWORD   com_options;
	DWORD   com_settings;
	BYTE    com_address;
	BYTE    com_sequence;

	BYTE    com_status;

#ifdef WIN32
	HANDLE  com_handle;
#else
	int     com_handle;
#endif

#ifdef SPROX_API_WITH_TCP
	SOCKET  com_socket;
#endif


	TCHAR   com_name[48 + 1];

	//  BOOL    com_reset_ctrl :1;
	//  BOOL    com_power_ctrl :1;
	//  BOOL    com_power_auto :1;
	//  BOOL    com_is_usb_vcp :1;
	//  BOOL    com_is_usb_cdc :1;
	//  BOOL    com_is_usb_ftdi:1;


	struct
	{
		DWORD   resp_tmo;
		DWORD   byte_tmo;
	}
	sprox_timeout;

	struct
	{
		BYTE    prd[4];
		BYTE    ver[3];
		BYTE    pid[5];
		BYTE    nid[4];
	} sprox_info;

	DWORD   sprox_version;
	DWORD   sprox_capabilities;

	/* Current RF operating mode */
	BYTE    pcd_current_rf_protocol;

	/* For Mifare functions */
	BYTE    mif_auth_ok;
	BYTE    mif_auth_info;
	BYTE    mif_snr[10];
	BYTE    mif_snr_len;

	/* For ISO 15693 functions */
	BYTE    iso15693_snr[8];

	/* For ICODE1 functions */
	BYTE    i1_snr[8];
};

typedef struct _SPROX_CTX_ST SPROX_CTX_ST;



#ifndef SPROX_API_REENTRANT
extern SPROX_CTX_ST* sprox_ctx_glob;
#endif

/* Project includes */
/* ---------------- */
#ifndef SPROX_API_NO_SERIAL
#include "sprox_serial.h"
#endif

#include "sprox_dialog.h"
#include "sprox_14443-4.h"
#include "sprox_capas.h"

#define SPROX_HIGH_BAUDRATE 1

/* Low layer protocol */
/* ------------------ */
void  RecvFlush(SPROX_CTX_ST* sprox_ctx);

SWORD SendFrameBIN(SPROX_CTX_ST* sprox_ctx, const BYTE* buffer, WORD buflen);
SWORD RecvFrameBIN(SPROX_CTX_ST* sprox_ctx, BYTE* buffer, WORD* buflen);
SWORD SendFrameBUS(SPROX_CTX_ST* sprox_ctx, const BYTE* buffer, WORD buflen);
SWORD RecvFrameBUS(SPROX_CTX_ST* sprox_ctx, BYTE* buffer, WORD* buflen);
SWORD SendFrameOSI(SPROX_CTX_ST* sprox_ctx, const BYTE* buffer, WORD buflen);
SWORD RecvFrameOSI(SPROX_CTX_ST* sprox_ctx, BYTE* buffer, WORD* buflen);
SWORD SendFrameASC(SPROX_CTX_ST* sprox_ctx, const BYTE* buffer, WORD buflen);
SWORD RecvFrameASC(SPROX_CTX_ST* sprox_ctx, BYTE* buffer, WORD* buflen);

BOOL SerialControl_Rs485(SPROX_CTX_ST* sprox_ctx, BOOL rs485_output);
BOOL SerialControl_Reset(SPROX_CTX_ST* sprox_ctx, BOOL can_run);
BOOL SerialControl_Power(SPROX_CTX_ST* sprox_ctx, BOOL power_up);

//BOOL IsCommUSB_FTDI(const TCHAR *device);
//BOOL IsCommUSB_CDC(const TCHAR *device);


/* Internal library functions */
/* -------------------------- */
SWORD SPROX_ReaderConnect(SPROX_CTX_ST* sprox_ctx);
SWORD SPROX_ReaderConnectAt(SPROX_CTX_ST* sprox_ctx, DWORD baudrate);
SWORD SPROX_ReaderConnectTCP(SPROX_CTX_ST* sprox_ctx, const TCHAR* conn_string);
void LoadSettings(void);
BOOL LoadDefaultDevice(void);
BOOL LoadDefaultDevice_HW_WinCE(void);

/* Debug functions */
/* --------------- */

void SPROX_Trace(BYTE level, const char* fmt, ...);
BYTE SPROX_TraceGetLevel(void);
void SPROX_TraceSetLevel(BYTE level);
void SPROX_TraceSetFile(const TCHAR* filename);


SWORD SPROX_Brcd_Function(BYTE cmd, const BYTE send_buffer[], WORD send_bytelen, BYTE recv_buffer[], WORD* recv_bytelen);
SWORD SPROX_Brcd_ReaderOpen(const TCHAR device[], BOOL direct);

SWORD SPROX_TCP_Function(SPROX_CTX_ST* sprox_ctx, BYTE cmd, const BYTE send_buffer[], WORD send_bytelen, BYTE recv_buffer[], WORD* recv_bytelen);
SWORD SPROX_TCP_ReaderOpen(SPROX_CTX_ST* sprox_ctx, const TCHAR device[]);
SWORD SPROX_TCP_ReaderClose(SPROX_CTX_ST* sprox_ctx);

#ifdef UNICODE
const char* _ST(const TCHAR* s);
#else
#define _ST(s) s
#endif

#define TRACE_DLG_LO    0x01
#define TRACE_DLG_HI    0x02
#define TRACE_ACCESS    0x04

#define TRACE_DEBUG     0x10
#define TRACE_INFOS     0x20
#define TRACE_WARNINGS  0x40
#define TRACE_ERRORS    0x80

#define TRACE_ALL       0xFF

#ifdef _DEBUG
#define D(x) x
#else
#define D(x)
#endif

extern const char sprox_library_revision[];

extern DWORD DefaultSettingsForced;
extern DWORD DefaultSettingsAllowed;
extern TCHAR DefaultDeviceName[32];

#endif
