/**** SpringProx.API/Win32
 *
 * NAME
 *   SpringProx.API :: Dialog - Windows code
 *
 * DESCRIPTION
 *   Provides dialog between host and reader for Windows and Windows CE,
 *   on a serial port and also using FTDI USB serial bridge.
 *
 * PARENT
 *   SpringProxAPI::Dialog
 *
 **/

 /*

   SpringProx API
   --------------

   Copyright (c) 2000-2008 SpringCard SAS, FRANCE - www.springcard.com

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
   TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   History
   -------

   JDA 15/02/2003 : created from springcard's serial_win32.c
   JDA 16/05/2003 : added power management and hard reset functions
   JDA 30/07/2003 : added support for USB trough FTD driver
   JDA 09/04/2004 : moved registry functions to config_win32.c
   JDA 26/08/2004 : added device_specific_info stuff, validated with SD device
   JDA 21/06/2005 : updated a little the structure to meet the serial_stub.c model
   JDA 05/07/2005 : added SPROX_ReaderAttachHandle
   JDA 24/05/2006 : added support of multiple USB devices
   JDA 01/08/2007 : added a call to ResetUart after CreateFile and before CloseHandle

 */
#include "sprox_api_i.h"

#ifdef WIN32

BOOL skip_vcp_check = FALSE;

#ifdef UNDER_CE
/* No FTDI support in Windows CE version */
#define SPROX_API_NO_FTDI
#endif

#ifdef _SMA1020_
typedef BOOL(*SCR_POWER) (BOOL);
typedef BOOL(*SCR_5OR3VOLT) (UINT);
SCR_POWER SCR_Power;
SCR_5OR3VOLT SCR_5or3Volt;
HINSTANCE SCR_Instance;
#endif

static void ResetUart(SPROX_CTX_ST* sprox_ctx);


#ifndef SPROX_API_NO_FTDI
static BOOL SerialOpen_FTDI(SPROX_CTX_ST* sprox_ctx, const TCHAR* device);
#endif
static BOOL SerialOpen_COM(SPROX_CTX_ST* sprox_ctx, const TCHAR* device);

/*
 *****************************************************************************
 *
 * Mandatory serial exported functions
 *
 *****************************************************************************
 */

 /*
  * SerialLookup
  * ------------
  * Lookup for the SpringProx device trying every available serial devices
  * one after the other, and calling SPROX_ReaderConnect for each device
  * If host has only one serial device, or if the SpringProx is always bound
  * to the same serial device, this function can be only an alias to
  * SerialOpen
  */
BOOL SerialLookup(SPROX_CTX_ST* sprox_ctx)
{
	BYTE    i;

#ifndef SPROX_API_NO_FTDI
	/* Try USB link */
	sprox_ctx->com_settings = DefaultSettingsForced;
	sprox_ctx->com_settings_allowed = DefaultSettingsAllowed;
	if (SerialOpen(sprox_ctx, _T("USB")))
	{
		if (SPROX_ReaderConnect(sprox_ctx) == MI_OK)
			return TRUE;
		SerialClose(sprox_ctx);
	}
#endif

	/* Loop other serial devices */
	for (i = 1; i <= 9; i++)
	{
		TCHAR tmp[16];
		_stprintf_s(tmp, sizeof(tmp) / sizeof(TCHAR), _T("COM%d"), i);
		if (SerialOpen(sprox_ctx, tmp))
		{
			sprox_ctx->com_settings = DefaultSettingsForced;
			sprox_ctx->com_settings_allowed = DefaultSettingsAllowed;
			if (SPROX_ReaderConnect(sprox_ctx) == MI_OK)
				return TRUE;
			SerialClose(sprox_ctx);
		}
	}

	/* Reader definitively not found */
	/* ----------------------------- */
	return FALSE;
}

/*
 * SerialOpen
 * ----------
 * Opens the named serial device
 * (if needed also power-up the SpringProx device)
 * If host doesn't support serial device naming, the device parameter
 * can be ignored
 */

#ifndef _SMA1020_

BOOL SerialOpen(SPROX_CTX_ST* sprox_ctx, const TCHAR* device)
{
	TCHAR* device_long;
	int l_device_long;
	BOOL  rc;

	if (sprox_ctx == NULL)
		return FALSE;

	sprox_ctx->com_options = 0;

	if (device == NULL)
	{
		/* No device name defined */
		if (!_tcslen(sprox_ctx->com_name))
			return FALSE;

		/* A device name has previously been defined, let's use this one */
		device = sprox_ctx->com_name;
	}

	/* Don't forget to close in case of it were previously open */
	SerialClose(sprox_ctx);

	/* Start-up here */
	SPROX_Trace(TRACE_ACCESS, "Opening device %s...", _ST(device));

	/* Is it the USB device ? */
	if (!_tcsncmp(device, _T("USB"), 3))
	{
#ifndef SPROX_API_NO_FTDI
		if (SerialOpen_FTDI(sprox_ctx, device))
		{
			SPROX_Trace(TRACE_ACCESS, "USB FTDI device is open, settings=%08lX", sprox_ctx->com_settings);
			return TRUE;
		}
#else
		return FALSE;
#endif
	}

	/* Regular serial device ? */
	if (SerialOpen_COM(sprox_ctx, device))
	{
		SPROX_Trace(TRACE_ACCESS, "COM device is open, settings=%08lX", sprox_ctx->com_settings);
		return TRUE;
	}

	/* Workaround for Windows silly port naming convention... */
	l_device_long = 5 + _tcslen(device);
	device_long = calloc(sizeof(TCHAR), l_device_long);
	if (device_long == NULL)
		return FALSE;
	_tcscpy_s(device_long, l_device_long, _T("\\\\.\\"));
	_tcscat_s(device_long, l_device_long, device);

	rc = SerialOpen_COM(sprox_ctx, device_long);

	free(device_long);
	return rc;
}

#endif


#ifdef _SMA1020_

/*
 * TriCubes SMA1020 specific
 * -------------------------
 * serial device has a fixed name and power-management must be controlled manually
 */
static BOOL SerialOpen(SPROX_CTX_ST* sprox_ctx)
{
	/* Power management */
	if (SCR_Instance == NULL)
	{
		SCR_Instance = LoadLibrary(_T("PowerMgmt.dll"));
		if (SCR_Instance == NULL)
		{
			MessageBox(0, _T("Unable to load PowerMgmt.dll"), _T("SpringProx for TriCubes"), 0);
			return FALSE;
		}
		SCR_Power = (SCR_POWER)GetProcAddress(SCR_Instance, _T("SCR_Power"));
		SCR_5or3Volt = (SCR_5OR3VOLT)GetProcAddress(SCR_Instance, _T("SCR_5or3Volt"));
		if ((SCR_Power == NULL) || (SCR_5or3Volt == NULL))
		{
			MessageBox(0, _T("Unable to work with PowerMgmt.dll"), _T("SpringProx for TriCubes"), 0);
			return FALSE;
		}
	}

	/* Power ON the device */
	if (!SCR_5or3Volt(2))
	{
		MessageBox(0, _T("Cannot power up the reader"), _T("SpringProx for TriCubes"), 0);
		return FALSE;
	}
	Sleep(100);

	/* Device name is fixed to COM5: */
	return SerialOpen_COM(sprox_ctx, _T("COM5:"));
}

#endif

static BOOL SerialOpen_COM(SPROX_CTX_ST* sprox_ctx, const TCHAR* device)
{
	sprox_ctx->com_settings &= ~COM_INTERFACE_MASK;
	sprox_ctx->com_settings |= COM_INTERFACE_SERIAL;

	/* Open the serial port */
	sprox_ctx->com_handle = CreateFile((LPCTSTR)device, GENERIC_READ | GENERIC_WRITE, 0, // comm devices must be opened w/exclusive- 
		NULL,  // no security attributes
		OPEN_EXISTING, // comm devices must use OPEN_EXISTING
		0, // not overlapped I/O
		NULL // hTemplate must be NULL for comm devices
	);

	if (sprox_ctx->com_handle == INVALID_HANDLE_VALUE)
		return FALSE;

	/* Clear any UART error, flush the buffers */
	ResetUart(sprox_ctx);

	/* Configure the port with a descent queue */
	if (!SetupComm(sprox_ctx->com_handle, 512, 512))
	{
		SPROX_Trace(TRACE_ACCESS, "SetupComm failed");
		return FALSE;
	}

	/* Remembere current device name */
	if (sprox_ctx->com_name != device)
	{
		_tcscpy_s(sprox_ctx->com_name, sizeof(sprox_ctx->com_name) / sizeof(TCHAR), device);
	}

#ifndef UNDER_CE
	// JDA: TODO
	/*
	if (!skip_vcp_check)
	{
	  sprox_ctx->com_is_usb_cdc  = IsCommUSB_CDC(device);
	  if (sprox_ctx->com_is_usb_cdc)
		SPROX_Trace(TRACE_DLG_HI, "Comm %s seems to be an USB VCP (CDC-ACM)", device);

	  sprox_ctx->com_is_usb_ftdi = IsCommUSB_FTDI(device);
	  if (sprox_ctx->com_is_usb_ftdi)
		SPROX_Trace(TRACE_DLG_HI, "Comm %s seems to be an USB VCP (FTDI)", device);
	}
	*/
#endif

	return TRUE;
}

#ifndef SPROX_API_NO_FTDI

static BOOL SerialOpen_FTDI(SPROX_CTX_ST* sprox_ctx, const TCHAR* device)
{
	FT_STATUS    ftStatus;
	FTCOMSTAT    stat;
	DWORD        errors;
	DWORD        count;
	DWORD        idx = 0;
	char         buffer[64];
	unsigned int i;

	sprox_ctx->com_settings &= ~COM_INTERFACE_MASK;
	sprox_ctx->com_settings |= COM_INTERFACE_FTDI;

	/* How many USB devices do we have ? */
	ftStatus = FT_ListDevices(&count, NULL, FT_LIST_NUMBER_ONLY);
	if ((ftStatus != FT_OK) || (count == 0))
	{
		SPROX_Trace(TRACE_ACCESS, "No USB device found");
		return FALSE;
	}

	if ((device == NULL) || (device[3] == '\0'))
	{
		/* No device name, or device name is "USB" -> we want the first one */
		idx = 1;
	}
	else
		if (device[3] != ':')
		{
			/* Assume the device name is USBx where x is a number */
			idx = _ttoi(&device[3]);
			if (idx <= 0)
			{
				SPROX_Trace(TRACE_ACCESS, "Wrong USB device identifier %s", _ST(device));
				return FALSE;
			}
		}
		else
		{
			/* We have the form USB:xxxxx, assume x is the serial number of the device we need */
			i = 0;
			while (i < sizeof(buffer))
			{
				buffer[i] = (char)device[4 + i];
				if (buffer[i] == '\0') break;
				i++;
			}
			buffer[sizeof(buffer) - 1] = '\0';
		}

	if (idx > 0)
	{
		/* Translate index into serial number */
		idx--;
		ftStatus = FT_ListDevices((PVOID)idx, buffer, FT_LIST_BY_INDEX | FT_OPEN_BY_SERIAL_NUMBER);
		if (ftStatus != FT_OK)
		{
			SPROX_Trace(TRACE_ACCESS, "USB device %d not found", idx + 1);
			return FALSE;               /* No USB device */
		}
	}

	_tcscpy(sprox_ctx->com_name, _T("USB:"));
	for (i = 0; i < strlen(buffer); i++)
	{
		if (i >= sizeof(sprox_ctx->com_name) / sizeof(TCHAR)) break;
		sprox_ctx->com_name[i + 4] = buffer[i];
	}
	sprox_ctx->com_name[sizeof(sprox_ctx->com_name) / sizeof(TCHAR) - 1] = '\0';

	SPROX_Trace(TRACE_ACCESS, "Selected USB device %s", _ST(buffer));

	sprox_ctx->com_handle = FT_W32_CreateFile(buffer, GENERIC_READ | GENERIC_WRITE, 0,  // comm devices must be opened w/exclusive- 
		NULL, // no security attributes
		OPEN_EXISTING,  // comm devices must use OPEN_EXISTING
		FT_OPEN_BY_SERIAL_NUMBER, // not overlapped I/O
		NULL  // hTemplate must be NULL for comm devices
	);

	if (sprox_ctx->com_handle == INVALID_HANDLE_VALUE)
		return FALSE;

	/* Configure the device */
	FT_SetLatencyTimer(sprox_ctx->com_handle, FTDI_BURST_TIME);
	FT_SetUSBParameters(sprox_ctx->com_handle, FTDI_BURST_SIZE, FTDI_BURST_SIZE);

	/* Clear UART */
	FT_W32_PurgeComm(sprox_ctx->com_handle, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	FT_W32_ClearCommError(sprox_ctx->com_handle, &errors, &stat);

	return TRUE;
}

#endif

/*
 * SerialClose
 * -----------
 * Close the serial device
 * (if possible also power-down the SpringProx device)
 */
void SerialClose(SPROX_CTX_ST* sprox_ctx)
{
	if (sprox_ctx == NULL)
		return;

	SPROX_Trace(TRACE_ACCESS, "Closing device with settings=%08lX", sprox_ctx->com_settings);

	if (sprox_ctx->com_handle != INVALID_HANDLE_VALUE)
	{
		if ((sprox_ctx->com_settings & COM_INTERFACE_MASK) == COM_INTERFACE_FTDI)
		{
#ifndef SPROX_API_NO_FTDI
			SPROX_Trace(TRACE_ACCESS, "Selected USB device %s", _ST(sprox_ctx->com_name));
			FT_W32_CloseHandle(sprox_ctx->com_handle);
#endif
		}
		else
		{
			SPROX_Trace(TRACE_ACCESS, "Selected serial device %s", _ST(sprox_ctx->com_name));
			ResetUart(sprox_ctx);
			CloseHandle(sprox_ctx->com_handle);
		}
	}
	sprox_ctx->com_handle = INVALID_HANDLE_VALUE;

#ifdef _SMA1020_
	/* TRICUBES specific : power OFF the device */
	/* ---------------------------------------- */
	if (SCR_5or3Volt != NULL)
		SCR_5or3Volt(3);
#endif
}

/*
 * SerialSetBaudrate
 * -----------------
 * Configure the baudrate for the serial device
 * (UART shoul'd accept 38400bps or 115200bps, other baudrates not used)
 * Params :
 * - baudrate (38400bps or 115200bps or ...)
 * Returns :
 * - TRUE  if the UART has accepted the specified baudrate
 * - FALSE if the specified baudrate is unavailable
 * NOTES :
 * - whatever the baudrate, the communication settings are 8 data bits, 1 stop bit,
 *   no parity, no flow control/
 * - if the UART doesn't support 115200bps, the SPROX_HIGH_BAUDRATE must be #undef
 */
BOOL SerialSetBaudrate(SPROX_CTX_ST* sprox_ctx, DWORD baudrate)
{
	if (sprox_ctx == NULL)
		return FALSE;
	if (sprox_ctx->com_handle == INVALID_HANDLE_VALUE)
		return FALSE;

	if ((sprox_ctx->com_settings & COM_INTERFACE_MASK) == COM_INTERFACE_FTDI)
	{
#ifndef SPROX_API_NO_FTDI
		FTDCB   dcb;

		if (!FT_W32_GetCommState(sprox_ctx->com_handle, &dcb))
		{
			SPROX_Trace(TRACE_ACCESS, "FT_W32_GetCommState failed (%d)", FT_W32_GetLastError(sprox_ctx->com_handle));
			return FALSE;
		}

		dcb.BaudRate = baudrate;
		dcb.fBinary = TRUE;
		dcb.fParity = FALSE;
		dcb.fOutxCtsFlow = FALSE;
		dcb.fOutxDsrFlow = FALSE;
		dcb.fDsrSensitivity = FALSE;
		dcb.fOutX = FALSE;
		dcb.fInX = FALSE;
		dcb.fNull = FALSE;
		dcb.fAbortOnError = FALSE;
		dcb.ByteSize = 8;
		dcb.Parity = NOPARITY;
		dcb.StopBits = ONESTOPBIT;
		dcb.fRtsControl = RTS_CONTROL_ENABLE; /* New 1.73 : for compatibility with RL78 flash board */
		dcb.fDtrControl = DTR_CONTROL_ENABLE; /* New 1.73 : for compatibility with RL78 flash board */

		if (!FT_W32_SetCommState(sprox_ctx->com_handle, &dcb))
		{
			SPROX_Trace(TRACE_ACCESS, "FT_W32_SetCommState failed (%d)", FT_W32_GetLastError(sprox_ctx->com_handle));
			return FALSE;
		}
#endif
	}
	else
	{
		DCB     dcb;

		Sleep(10);
		ResetUart(sprox_ctx);

		if (!GetCommState(sprox_ctx->com_handle, &dcb))
		{
			SPROX_Trace(TRACE_ACCESS, "GetCommState failed (%d)", GetLastError());
			return FALSE;
		}

		dcb.BaudRate = baudrate;

		dcb.fBinary = TRUE;
		dcb.fParity = FALSE;
		dcb.fOutxCtsFlow = FALSE;
		dcb.fOutxDsrFlow = FALSE;
		dcb.fDsrSensitivity = FALSE;
		dcb.fOutX = FALSE;
		dcb.fInX = FALSE;
		dcb.fNull = FALSE;
		dcb.ByteSize = 8;
		dcb.Parity = NOPARITY;
		dcb.StopBits = ONESTOPBIT;
		dcb.fRtsControl = RTS_CONTROL_ENABLE; /* New 1.73 : for compatibility with RL78 flash board */
		dcb.fDtrControl = DTR_CONTROL_ENABLE; /* New 1.73 : for compatibility with RL78 flash board */

		dcb.fAbortOnError = TRUE;
		dcb.fTXContinueOnXoff = TRUE;

		if (!SetCommState(sprox_ctx->com_handle, &dcb))
		{
			SPROX_Trace(TRACE_ACCESS, "SetCommState failed (%d)", GetLastError());
			return FALSE;
		}

		Sleep(10);
		ResetUart(sprox_ctx);
	}

	SPROX_Trace(TRACE_ACCESS, "Serial device configured at %ldbps", baudrate);
	return TRUE;
}

/*
 * SerialSetTimeouts
 * -----------------
 * Configure timeouts for the serial device
 * Params :
 * - resp_tmo : time to wait for SpringProx answer (timeout between last byte sent, and first answer byte)
 * - byte_tmo : inter-byte timeout (timeout between two consecutive bytes of the answer)
 */
BOOL SerialSetTimeouts(SPROX_CTX_ST* sprox_ctx, DWORD resp_tmo, DWORD byte_tmo)
{
	if (sprox_ctx == NULL)
		return FALSE;
	if (sprox_ctx->com_handle == INVALID_HANDLE_VALUE)
		return FALSE;

	if ((sprox_ctx->com_settings & COM_INTERFACE_MASK) == COM_INTERFACE_FTDI)
	{
#ifndef SPROX_API_NO_FTDI
		static FTTIMEOUTS stTimeCur;
		FTTIMEOUTS        stTimeNew;

		stTimeNew.ReadIntervalTimeout = byte_tmo;
		stTimeNew.ReadTotalTimeoutConstant = resp_tmo;
		stTimeNew.ReadTotalTimeoutMultiplier = byte_tmo;
		stTimeNew.WriteTotalTimeoutConstant = 100;
		stTimeNew.WriteTotalTimeoutMultiplier = 10;

		if (memcmp(&stTimeCur, &stTimeNew, sizeof(FTTIMEOUTS)))
		{
			if (!FT_W32_SetCommTimeouts(sprox_ctx->com_handle, &stTimeNew))
			{
				SPROX_Trace(TRACE_ACCESS, "FT_W32_SetCommTimeouts failed (%d)", FT_W32_GetLastError(sprox_ctx->com_handle));
				return FALSE;
			}
			memcpy(&stTimeCur, &stTimeNew, sizeof(FTTIMEOUTS));
		}
#endif
	}
	else
	{
		static COMMTIMEOUTS stTimeCur = { 0 };
		COMMTIMEOUTS        stTimeNew;

		stTimeNew.ReadIntervalTimeout = byte_tmo;
		stTimeNew.ReadTotalTimeoutConstant = resp_tmo;
		stTimeNew.ReadTotalTimeoutMultiplier = byte_tmo;
		stTimeNew.WriteTotalTimeoutConstant = 250;
		stTimeNew.WriteTotalTimeoutMultiplier = 100;

		if (memcmp(&stTimeCur, &stTimeNew, sizeof(COMMTIMEOUTS)))
		{
			if (!SetCommTimeouts(sprox_ctx->com_handle, &stTimeNew))
			{
				SPROX_Trace(TRACE_ACCESS, "SetCommTimeouts failed (%d)", GetLastError());
				return FALSE;
			}
			memcpy(&stTimeCur, &stTimeNew, sizeof(COMMTIMEOUTS));
		}
	}

	return TRUE;
}

/*
 * SendByte
 * --------
 * Send one byte to the SpringProx device
 * Returns :
 * - TRUE  if byte has been sent
 * - FALSE if byte has not been sent (whatever the reason is)
 */
BOOL SendByte(SPROX_CTX_ST* sprox_ctx, BYTE b)
{
	DWORD   dwWritten = 0;

	if (sprox_ctx == NULL)
		return FALSE;
	if (sprox_ctx->com_handle == INVALID_HANDLE_VALUE)
		return FALSE;

	if ((sprox_ctx->com_settings & COM_INTERFACE_MASK) == COM_INTERFACE_FTDI)
	{
#ifndef SPROX_API_NO_FTDI
		if (!FT_W32_WriteFile(sprox_ctx->com_handle, &b, 1, &dwWritten, 0))
		{
			SPROX_Trace(TRACE_ACCESS, "FT_W32_WriteFile error (%d)", FT_W32_GetLastError(sprox_ctx->com_handle));
			return FALSE;
		}

		SPROX_Trace(TRACE_DLG_LO, "-%02X", b);
#endif
	}
	else
		if ((sprox_ctx->com_settings & COM_INTERFACE_MASK) == COM_INTERFACE_SERIAL)
		{
			if (sprox_ctx->com_settings & COM_OPTION_SERIAL_RS485)
				SerialControl_Rs485(sprox_ctx, TRUE);

			if (!WriteFile(sprox_ctx->com_handle, &b, 1, &dwWritten, 0))
			{
				SPROX_Trace(TRACE_ACCESS, "WriteFile(%d) error (%d)", 1, GetLastError());
				goto failed;
			}

			if (dwWritten < 1)
			{
				SPROX_Trace(TRACE_ACCESS, "WriteFile(%d/%d) failed (%d)", dwWritten, 1, GetLastError());
				goto failed;
			}

			if (sprox_ctx->com_settings & COM_OPTION_SERIAL_RS485)
				SerialControl_Rs485(sprox_ctx, FALSE);

			SPROX_Trace(TRACE_DLG_LO, "-%02X", b);
		}
		else
		{
			SPROX_Trace(TRACE_ACCESS, "SendByte:Interface not set (settings=%08lX)", sprox_ctx->com_settings);
			return FALSE;
		}

	return TRUE;

failed:
	ResetUart(sprox_ctx);
	if (sprox_ctx->com_settings & COM_OPTION_SERIAL_RS485)
		SerialControl_Rs485(sprox_ctx, FALSE);
	return FALSE;
}

/*
 * RecvByte
 * --------
 * Recv one byte from the SpringProx device
 * Returns :
 * - TRUE  if one byte has been received
 * - FALSE if no byte has been received (whatever the reason is)
 */
BOOL RecvByte(SPROX_CTX_ST* sprox_ctx, BYTE* b)
{
	DWORD   dwRead = 0;

	if (sprox_ctx == NULL)
		return FALSE;
	if (sprox_ctx->com_handle == INVALID_HANDLE_VALUE)
		return FALSE;

	if ((sprox_ctx->com_settings & COM_INTERFACE_MASK) == COM_INTERFACE_FTDI)
	{
#ifndef SPROX_API_NO_FTDI
		if (!FT_W32_ReadFile(sprox_ctx->com_handle, b, 1, &dwRead, 0))
		{
			SPROX_Trace(TRACE_ACCESS, "FT_W32_ReadFile failed (%d)", FT_W32_GetLastError(sprox_ctx->com_handle));
			return FALSE;
		}
#endif
	}
	else
		if ((sprox_ctx->com_settings & COM_INTERFACE_MASK) == COM_INTERFACE_SERIAL)
		{
			if (!ReadFile(sprox_ctx->com_handle, b, 1, &dwRead, 0))
			{
				SPROX_Trace(TRACE_ACCESS, "ReadFile failed (%d)", GetLastError());
				return FALSE;
			}
		}
		else
		{
			SPROX_Trace(TRACE_ACCESS, "RecvByte:Interface not set (settings=%08lX)", sprox_ctx->com_settings);
			return FALSE;
		}


	if (dwRead == 0)
		return FALSE;

	SPROX_Trace(TRACE_DLG_LO, "+%02X", *b);

	return TRUE;
}

/*
 * SendBurst
 * ---------
 * Send multiple bytes to the SpringProx device
 * Returns :
 * - TRUE  if ALL bytes has been sent
 * - FALSE if AT LEAST ONE byte has not been sent (whatever the reason is)
 */
BOOL SendBurst(SPROX_CTX_ST* sprox_ctx, const BYTE* b, WORD len)
{
	const BYTE* pSendBuffer;
	DWORD dwWriteLen;
	DWORD dwTotalLen;
	DWORD dwWritten = 0;
	DWORD i;

	if (sprox_ctx == NULL)
		return FALSE;
	if (sprox_ctx->com_handle == INVALID_HANDLE_VALUE)
		return FALSE;

	pSendBuffer = b;
	dwTotalLen = len;

	if ((sprox_ctx->com_settings & COM_INTERFACE_MASK) == COM_INTERFACE_FTDI)
	{
#ifndef SPROX_API_NO_FTDI
		while (dwTotalLen)
		{
			if (dwTotalLen < 256)
				dwWriteLen = dwTotalLen;
			else
				dwWriteLen = 256;

			if (!FT_W32_WriteFile(sprox_ctx->com_handle, (void*)pSendBuffer, dwWriteLen, &dwWritten, 0))
			{
				SPROX_Trace(TRACE_ACCESS, "FT_W32_WriteFile failed (%d)", FT_W32_GetLastError(sprox_ctx->com_handle));
				return FALSE;
			}

			dwTotalLen -= dwWritten;
			pSendBuffer += dwWritten;
		}

#endif
	}
	else
		if ((sprox_ctx->com_settings & COM_INTERFACE_MASK) == COM_INTERFACE_SERIAL)
		{
			if (sprox_ctx->com_settings & COM_OPTION_SERIAL_RS485)  /* Enter transmit mode */
				SerialControl_Rs485(sprox_ctx, TRUE);

			while (dwTotalLen)
			{

				if (sprox_ctx->com_settings & COM_OPTION_SERIAL_RS485)
				{
					/* We write bytes one after one, and add an extra delay because of echoes on the bus */
					dwWriteLen = 1;
				}
				else
				{
#ifdef UNDER_CE
					/* We limit us to 32 bytes paquets because some SDIO implementations can't deal with bigger frames */
					if (dwTotalLen < 32)
						dwWriteLen = dwTotalLen;
					else
						dwWriteLen = 32;
#else
					if (dwTotalLen < 260)
						dwWriteLen = dwTotalLen;
					else
						dwWriteLen = 256;
#endif
				}

				if (!WriteFile(sprox_ctx->com_handle, pSendBuffer, dwWriteLen, &dwWritten, 0))
				{
					SPROX_Trace(TRACE_ACCESS, "WriteFile(%d) error (%d)", dwWriteLen, GetLastError());
					goto failed;
				}

				if (dwWritten < dwWriteLen)
				{
					SPROX_Trace(TRACE_ACCESS, "WriteFile(%d/%d) failed (%d)", dwWritten, dwWriteLen, GetLastError());
					goto failed;
				}

				dwTotalLen -= dwWritten;
				pSendBuffer += dwWritten;
			}

			if (sprox_ctx->com_settings & COM_OPTION_SERIAL_RS485)
			{
				BYTE    dummy = 0xFF;
				WriteFile(sprox_ctx->com_handle, &dummy, 1, &dwWritten, 0);
				SerialControl_Rs485(sprox_ctx, FALSE);
				WriteFile(sprox_ctx->com_handle, &dummy, 1, &dwWritten, 0);
			}
		}
		else
		{
			SPROX_Trace(TRACE_ACCESS, "SendBurst:Interface not set (settings=%08lX)", sprox_ctx->com_settings);
			return FALSE;
		}


	if (SPROX_TraceGetLevel() & TRACE_DLG_LO)
		for (i = 0; i < len; i++)
			SPROX_Trace(TRACE_DLG_LO, "-%02X", b[i]);

	return TRUE;

failed:
	ResetUart(sprox_ctx);
	if (sprox_ctx->com_settings & COM_OPTION_SERIAL_RS485)
		SerialControl_Rs485(sprox_ctx, FALSE);
	return FALSE;
}

/*
 * RecvBurst
 * --------
 * Recv mutiple bytes from the SpringProx device
 * Returns :
 * - TRUE  if ALL expected bytes have been received (received length == len parameter)
 * - FALSE if AT LEAST ONE byte has not been received (received length < len parameter)
 */
BOOL RecvBurst(SPROX_CTX_ST* sprox_ctx, BYTE* b, WORD len)
{
	BYTE* pRecvBuffer;
	DWORD dwRemainingLen = len;
	DWORD dwReceivedLen = 0;
	DWORD i;

	if (sprox_ctx == NULL)
		return FALSE;
	if (sprox_ctx->com_handle == INVALID_HANDLE_VALUE)
		return FALSE;

	pRecvBuffer = b;

	if ((sprox_ctx->com_settings & COM_INTERFACE_MASK) == COM_INTERFACE_FTDI)
	{
#ifndef SPROX_API_NO_FTDI
		while (dwRemainingLen)
		{
			DWORD dwWantLen, dwGotLen;

			if (dwRemainingLen < 256)
				dwWantLen = dwRemainingLen;
			else
				dwWantLen = 256;

			if (!FT_W32_ReadFile(sprox_ctx->com_handle, pRecvBuffer, dwWantLen, &dwGotLen, 0))
			{
				SPROX_Trace(TRACE_ACCESS, "FT_W32_ReadFile failed (%d)", FT_W32_GetLastError(sprox_ctx->com_handle));
				return FALSE;
			}

			dwRemainingLen -= dwGotLen;
			dwReceivedLen += dwGotLen;
			pRecvBuffer += dwGotLen;

			if (dwGotLen < dwWantLen)
			{
				SPROX_Trace(TRACE_ACCESS, "FT_W32_ReadFile timeout (%d<%d, total %d/%d)", dwGotLen, dwWantLen, dwReceivedLen, len);
				break;
			}
		}
#endif
	}
	else
		if ((sprox_ctx->com_settings & COM_INTERFACE_MASK) == COM_INTERFACE_SERIAL)
		{
			while (dwRemainingLen)
			{
				DWORD dwWantLen, dwGotLen;

				if (dwRemainingLen < 32)
					dwWantLen = dwRemainingLen;
				else
					dwWantLen = 32;

				if (!ReadFile(sprox_ctx->com_handle, pRecvBuffer, dwWantLen, &dwGotLen, 0))
				{
					SPROX_Trace(TRACE_ACCESS, "ReadFile failed (%d)", GetLastError());
					return FALSE;
				}

				dwRemainingLen -= dwGotLen;
				dwReceivedLen += dwGotLen;
				pRecvBuffer += dwGotLen;

				if (dwGotLen < dwWantLen)
				{
					SPROX_Trace(TRACE_ACCESS, "ReadFile timeout (%d<%d, total %d/%d, code %d)", dwGotLen, dwWantLen, dwReceivedLen, len, GetLastError());
					break;
				}
			}
		}
		else
		{
			SPROX_Trace(TRACE_ACCESS, "RecvBurst:Interface not set (settings=%08lX)", sprox_ctx->com_settings);
			return FALSE;
		}

	if (SPROX_TraceGetLevel() & TRACE_DLG_LO)
		for (i = 0; i < dwReceivedLen; i++)
			SPROX_Trace(TRACE_DLG_LO, "+%02X", b[i]);

	if (dwRemainingLen) /* A timeout has occured */
		return FALSE;

	return TRUE;
}

BOOL SerialPowerUp(SPROX_CTX_ST* sprox_ctx)
{
	if (sprox_ctx == NULL)
		return FALSE;
	if (!(sprox_ctx->com_settings & COM_OPTION_SERIAL_POWER_ON_DTR))
		return TRUE;                /* No hardware POWER CONTROL */
	return SerialControl_Power(sprox_ctx, TRUE);
}

BOOL SerialPowerDown(SPROX_CTX_ST* sprox_ctx)
{
	if (sprox_ctx == NULL)
		return FALSE;
	if (!(sprox_ctx->com_settings & COM_OPTION_SERIAL_POWER_ON_DTR))
		return TRUE;                /* No hardware POWER CONTROL */
	return SerialControl_Power(sprox_ctx, FALSE);
}

BOOL SerialReset(SPROX_CTX_ST* sprox_ctx)
{
	if (sprox_ctx == NULL)
		return FALSE;
	if (!(sprox_ctx->com_settings & COM_OPTION_SERIAL_RESET_ON_RTS))
		return TRUE;                /* No hardware RESET available */
	if (!SerialControl_Reset(sprox_ctx, FALSE))
		return FALSE;
	if (!SerialControl_Reset(sprox_ctx, TRUE))
		return FALSE;
	return TRUE;
}

/*
 *****************************************************************************
 *
 * Low level internal functions
 *
 *****************************************************************************
 */

 /* Flush the UART, clear errors */
 /* ---------------------------- */
static void ResetUart(SPROX_CTX_ST* sprox_ctx)
{
	DWORD   comerrors;
	COMSTAT comstat;

	if (!ClearCommError(sprox_ctx->com_handle, &comerrors, &comstat))
	{
		SPROX_Trace(TRACE_ACCESS, "Comm : fatal error (%ld)", GetLastError());
	}
	else
	{
		if (comerrors & CE_BREAK)    SPROX_Trace(TRACE_ACCESS, "Comm: Break");
		if (comerrors & CE_FRAME)    SPROX_Trace(TRACE_ACCESS, "Comm: Frame error");
		if (comerrors & CE_IOE)      SPROX_Trace(TRACE_ACCESS, "Comm: I/O error");
		if (comerrors & CE_MODE)     SPROX_Trace(TRACE_ACCESS, "Comm: Unsupported mode");
		if (comerrors & CE_OVERRUN)  SPROX_Trace(TRACE_ACCESS, "Comm: Overrun");
		if (comerrors & CE_RXOVER)   SPROX_Trace(TRACE_ACCESS, "Comm: RX overflow");
		if (comerrors & CE_RXPARITY) SPROX_Trace(TRACE_ACCESS, "Comm: RX parity error");
		if (comerrors & CE_TXFULL)   SPROX_Trace(TRACE_ACCESS, "Comm: TX overflow");
		SPROX_Trace(TRACE_ACCESS, "Comm errors %ld : %08lX", GetLastError(), comerrors);
	}

	SPROX_Trace(TRACE_DLG_HI, "Comm : purge & flush");

	PurgeComm(sprox_ctx->com_handle, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	FlushFileBuffers(sprox_ctx->com_handle);
}

/* RS485 : The reader RS485 shoul'd be linked to the RTS pin */
/* --------------------------------------------------------- */
BOOL SerialControl_Rs485(SPROX_CTX_ST* sprox_ctx, BOOL rs485_output)
{
	DCB     dcb;

	if (sprox_ctx == NULL)
		return FALSE;
	if (sprox_ctx->com_handle == INVALID_HANDLE_VALUE)
		return FALSE;

	if (!(sprox_ctx->com_settings & COM_OPTION_SERIAL_RS485))
		return TRUE;
	if (sprox_ctx->com_settings & COM_OPTION_SERIAL_RESET_ON_RTS)
		return TRUE;                /* The pin is used by RESET */

	if (!rs485_output)
	{
		/* Wait for pending outgoing communication to finish */
		FlushFileBuffers(sprox_ctx->com_handle);
	}

	if (!GetCommState(sprox_ctx->com_handle, &dcb))
		return FALSE;

	if (rs485_output)
		dcb.fRtsControl = RTS_CONTROL_DISABLE;
	else
		dcb.fRtsControl = RTS_CONTROL_ENABLE;

	if (!SetCommState(sprox_ctx->com_handle, &dcb))
		return FALSE;

	if (rs485_output)
	{
		/* Switching to output can take a few ms (time for the MAX485 driver to be ready) */
		SPROX_Trace(TRACE_DLG_HI, "<rs485_out>");
	}
	else
	{
		/* No need to wait, but we must clear the queue because we have received echoes of what we've sent */
		SPROX_Trace(TRACE_DLG_HI, "</rs485_out>");
	}

	return TRUE;
}

/* The reader RESET shoul'd be linked to the RTS pin */
/* ------------------------------------------------- */
BOOL SerialControl_Reset(SPROX_CTX_ST* sprox_ctx, BOOL can_run)
{
	if (sprox_ctx == NULL)
		return FALSE;
	if (sprox_ctx->com_handle == INVALID_HANDLE_VALUE)
		return FALSE;

	if (can_run)
		SPROX_Trace(TRACE_DLG_HI, "</reset>");
	else
		SPROX_Trace(TRACE_DLG_HI, "<reset>");

	if ((sprox_ctx->com_settings & COM_INTERFACE_MASK) == COM_INTERFACE_FTDI)
	{
#ifndef SPROX_API_NO_FTDI
		FTDCB   dcb;

		if (!FT_W32_GetCommState(sprox_ctx->com_handle, &dcb))
			return FALSE;

		if (!can_run)
			dcb.fRtsControl = RTS_CONTROL_ENABLE;
		else
			dcb.fRtsControl = RTS_CONTROL_DISABLE;

		/* Reset mode */
		dcb.fRtsControl = RTS_CONTROL_ENABLE;

		if (!FT_W32_SetCommState(sprox_ctx->com_handle, &dcb))
			return FALSE;
#endif
	}
	else
	{
		DCB     dcb;

		if (!GetCommState(sprox_ctx->com_handle, &dcb))
			return FALSE;

		if (!can_run)
			dcb.fRtsControl = RTS_CONTROL_ENABLE;
		else
			dcb.fRtsControl = RTS_CONTROL_DISABLE;

		if (!SetCommState(sprox_ctx->com_handle, &dcb))
			return FALSE;
	}

	return TRUE;
}

/* The reader POWER shoul'd be linked to the DTR pin */
/* ------------------------------------------------- */
BOOL SerialControl_Power(SPROX_CTX_ST* sprox_ctx, BOOL power_up)
{
	if (sprox_ctx == NULL)
		return FALSE;
	if (sprox_ctx->com_handle == INVALID_HANDLE_VALUE)
		return FALSE;

	if (power_up)
		SPROX_Trace(TRACE_DLG_HI, "<power>");
	else
		SPROX_Trace(TRACE_DLG_HI, "</power>");

	if ((sprox_ctx->com_settings & COM_INTERFACE_MASK) == COM_INTERFACE_FTDI)
	{
#ifndef SPROX_API_NO_FTDI
		FTDCB   dcb;

		if (!FT_W32_GetCommState(sprox_ctx->com_handle, &dcb))
			return FALSE;

		if (power_up)
			dcb.fDtrControl = DTR_CONTROL_DISABLE;
		else
			dcb.fDtrControl = DTR_CONTROL_ENABLE;
		/* Reset mode */
		dcb.fRtsControl = RTS_CONTROL_ENABLE;

		if (!FT_W32_SetCommState(sprox_ctx->com_handle, &dcb))
			return FALSE;
#endif
	}
	else
	{
		DCB     dcb;

		if (!GetCommState(sprox_ctx->com_handle, &dcb))
			return FALSE;

		if (power_up)
			dcb.fDtrControl = DTR_CONTROL_DISABLE;
		else
			dcb.fDtrControl = DTR_CONTROL_ENABLE;
		/* Reset mode */
		dcb.fRtsControl = RTS_CONTROL_ENABLE;

		if (!SetCommState(sprox_ctx->com_handle, &dcb))
			return FALSE;
	}

	return TRUE;
}

/*
 * Attach the SpringProx library to a previously opened comm
 * ---------------------------------------------------------
 */
#ifndef SPROX_API_NO_ATTACH
SPROX_API_FUNC(ReaderAttachSerial) (SPROX_PARAM  HANDLE hComm)
{
	DCB dcb;
	SPROX_PARAM_TO_CTX;

	if (hComm == INVALID_HANDLE_VALUE)
		return MI_LIB_CALL_ERROR;

	memset(sprox_ctx, 0, sizeof(SPROX_CTX_ST));
	sprox_ctx->com_handle = hComm;
	sprox_ctx->com_settings &= ~COM_INTERFACE_MASK;
	sprox_ctx->com_settings |= COM_INTERFACE_SERIAL;

	/* Retrieve UART config */
	if (!GetCommState(sprox_ctx->com_handle, &dcb))
		return MI_LIB_CALL_ERROR;

	sprox_ctx->com_settings &= COM_BAUDRATE_MASK;
#ifdef SPROX_HIGH_BAUDRATE
	if (dcb.BaudRate == 115200)
	{
		sprox_ctx->com_settings |= COM_BAUDRATE_115200;
		return SPROX_ReaderConnectAt(sprox_ctx, 0);
	}
	else
#endif
		if (dcb.BaudRate == 38400)
		{
			sprox_ctx->com_settings |= COM_BAUDRATE_38400;
			return SPROX_ReaderConnectAt(sprox_ctx, 0);
		}

	return MI_SER_ACCESS_ERR;
}

#ifndef SPROX_API_NO_FTDI

SPROX_API_FUNC(ReaderAttachUSB) (SPROX_PARAM  HANDLE hComm)
{
	FTDCB dcb;
	SPROX_PARAM_TO_CTX;

	if (hComm == INVALID_HANDLE_VALUE)
		return MI_LIB_CALL_ERROR;

	memset(sprox_ctx, 0, sizeof(SPROX_CTX_ST));
	sprox_ctx->com_handle = hComm;
	sprox_ctx->com_settings &= ~COM_INTERFACE_MASK;
	sprox_ctx->com_settings |= COM_INTERFACE_FTDI;

	/* Retrieve UART config */
	if (!FT_W32_GetCommState(sprox_ctx->com_handle, &dcb))
		return MI_LIB_CALL_ERROR;

	sprox_ctx->com_settings &= COM_BAUDRATE_MASK;
#ifdef SPROX_HIGH_BAUDRATE
	if (dcb.BaudRate == 115200)
	{
		sprox_ctx->com_settings |= COM_BAUDRATE_115200;
		return SPROX_ReaderConnectAt(sprox_ctx, 0);
	}
	else
#endif
		if (dcb.BaudRate == 38400)
		{
			sprox_ctx->com_settings |= COM_BAUDRATE_38400;
			return SPROX_ReaderConnectAt(sprox_ctx, 0);
		}

	return MI_SER_ACCESS_ERR;
}

#endif
#endif

#endif
