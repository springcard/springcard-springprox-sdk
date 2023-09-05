/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  SpringProx API
  --------------

  Copyright (c) 2000-2012 SpringCard SAS, FRANCE - www.springcard.com

  serial_linux.c
  --------------
  Serial communication under Linux

  revision :
  ----------

	JDA 03/02/2004 : created from SpringCard's serial_linux.c
  JDA 27/01/2012 : better handling of timeout in RecvBurst

*/

#include "sprox_api_i.h"

#ifdef LINUX

//#undef D
//#define D(x) x

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static BOOL SerialOpen_COM(SPROX_CTX_ST* sprox_ctx, const TCHAR* device);
static BOOL SerialSetRs485Mode(SPROX_CTX_ST* sprox_ctx, BOOL rs485_output);

/*
 * SerialLookup
 * ------------
 * Lookup for the SpringProx device trying every available serial devices
 * one after the other.
 * If host has only one serial device, or if the SpringProx is always bound
 * to the same serial device, this function can be only an alias to
 * SerialOpen + SPROX_ReaderConnect
 */
BOOL SerialLookup(SPROX_CTX_ST* sprox_ctx)
{
#ifndef SPROX_API_NO_FTDI
	/* Try FTDI devices on USB */
	if (SerialOpen(sprox_ctx, _T("FTDI:0403:D969"))) /* CSB */
	{
		if (SPROX_ReaderConnect(sprox_ctx) == MI_OK)
			return TRUE;
		SerialClose(sprox_ctx);
	}
	if (SerialOpen(sprox_ctx, _T("FTDI:0403:D968"))) /* Generic SpringCard */
	{
		if (SPROX_ReaderConnect(sprox_ctx) == MI_OK)
			return TRUE;
		SerialClose(sprox_ctx);
	}
	memset(&sprox_ctx->settings, 0, sizeof(sprox_ctx->settings));
	if (SerialOpen(sprox_ctx, _T("FTDI:0403:6001"))) /* FTDI default ID */
	{
		if (SPROX_ReaderConnect(sprox_ctx) == MI_OK)
			return TRUE;
		SerialClose(sprox_ctx);
	}
#endif

	/* Sample sequence can be something like that : */
	if (SerialOpen(sprox_ctx, "/dev/ttyUSB0"))
	{
		if (SPROX_ReaderConnect(sprox_ctx) == MI_OK)
			return TRUE;
		SerialClose(sprox_ctx);
	}
	/* ... */
	if (SerialOpen(sprox_ctx, "/dev/ttyS0"))
	{
		if (SPROX_ReaderConnect(sprox_ctx) == MI_OK)
			return TRUE;
		SerialClose(sprox_ctx);
	}
	if (SerialOpen(sprox_ctx, "/dev/ttyS1"))
	{
		if (SPROX_ReaderConnect(sprox_ctx) == MI_OK)
			return TRUE;
		SerialClose(sprox_ctx);
	}
	/* ... */
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
BOOL SerialOpen(SPROX_CTX_ST* sprox_ctx, const TCHAR* device)
{
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

#ifndef SPROX_API_NO_FTDI
	if (!_tcsncmp(device, "FTDI:", 5))
	{
		if (FTDI_Open(sprox_ctx, device))
		{
			sprox_ctx->com_settings = COM_INTERFACE_FTDI;
			goto ok;
		}
		return FALSE;
	}
#endif

	/* Regular serial device ? */
	if (SerialOpen_COM(sprox_ctx, device))
		goto ok;
	return FALSE;

ok:
	/* Startup sequence */
	SerialControl_Reset(sprox_ctx, FALSE);
	SerialPowerUp(sprox_ctx);
	SerialControl_Reset(sprox_ctx, TRUE);

	if (sprox_ctx->com_name != device)
	{
		_tcsncpy(sprox_ctx->com_name, device, sizeof(sprox_ctx->com_name) / sizeof(TCHAR) - 1);
		sprox_ctx->com_name[sizeof(sprox_ctx->com_name) / sizeof(TCHAR) - 1] = '\0';
	}

	return TRUE;
}

static BOOL SerialOpen_COM(SPROX_CTX_ST* sprox_ctx, const TCHAR* device)
{
	sprox_ctx->com_handle = open(device, O_RDWR | O_NOCTTY);

	if (sprox_ctx->com_handle < 0)
	{
		D(perror("open"));
		return FALSE;
	}

	/* Configure the device */
	if (sprox_ctx->com_settings & COM_OPTION_SERIAL_RS485)
		SerialSetRs485Mode(sprox_ctx, FALSE);

	/* Clear UART */
	tcflush(sprox_ctx->com_handle, TCIFLUSH);

	return TRUE;
}

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

	if (sprox_ctx->com_handle >= 0)
	{
#ifndef SPROX_API_NO_FTDI
		if (sprox_ctx->com_type == DEVICE_IS_USB)
		{
			FTDI_Close(sprox_ctx);
		}
		else
#endif
		{
			close(sprox_ctx->com_handle);
		}
		sprox_ctx->com_handle = -1;
	}
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
	struct termios newtio;

	if (sprox_ctx == NULL)
		return FALSE;

#ifndef SPROX_API_NO_FTDI
	if (sprox_ctx->com_type == DEVICE_IS_USB)
		return FTDI_SetBaudrate(sprox_ctx, baudrate);
#endif

	bzero(&newtio, sizeof(newtio));
	// CS8  = 8n1 (8bit,no parity,1 stopbit
	// CLOCAL= local connection, no modem control
	// CREAD  = enable receiving characters
	newtio.c_cflag = CS8 | CLOCAL | CREAD;
	switch (baudrate)
	{
	case 115200: newtio.c_cflag |= B115200; break;
	case  38400: newtio.c_cflag |= B38400; break;
	case  19200: newtio.c_cflag |= B19200; break;
	case   9600: newtio.c_cflag |= B9600; break;
	case   4800: newtio.c_cflag |= B4800; break;
	case   2400: newtio.c_cflag |= B2400; break;
	case   1200: newtio.c_cflag |= B1200; break;
	default: return FALSE;
	}
	newtio.c_iflag = IGNPAR | IGNBRK;
	newtio.c_oflag = 0;

	// set input mode (non-canonical, no echo,...) 
	newtio.c_lflag = 0;

	newtio.c_cc[VTIME] = 0;       // inter-character timer unused
	newtio.c_cc[VMIN] = 1;        // blocking read until 1 chars received

	tcflush(sprox_ctx->com_handle, TCIFLUSH);

	if (tcsetattr(sprox_ctx->com_handle, TCSANOW, &newtio))
	{
		perror("tcsetattr");
		SerialClose(sprox_ctx);
		return FALSE;
	}

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

	SPROX_Trace(TRACE_DLG_HI, "SetTimeouts: %d, %d", resp_tmo, byte_tmo);

	sprox_ctx->sprox_timeout.resp_tmo = resp_tmo;
	sprox_ctx->sprox_timeout.byte_tmo = byte_tmo;

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
	int done;

#ifndef SPROX_API_NO_FTDI
	if (sprox_ctx->com_type == DEVICE_IS_USB)
		return FTDI_SendBurst(sprox_ctx, &b, 1);
#endif

	done = write(sprox_ctx->com_handle, &b, 1);
	if (done <= 0)
	{
		perror("write");
		return FALSE;
	}
	D(printf("-%02X", b));
	D(fflush(stdout));
	return TRUE;
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
	fd_set  fdset;
	struct timeval timeout;
	int     res, done;

#ifndef SPROX_API_NO_FTDI
	if (sprox_ctx->com_type == DEVICE_IS_USB)
		return FTDI_RecvBurst(sprox_ctx, b, 1);
#endif

	timeout.tv_sec = sprox_ctx->sprox_timeout.resp_tmo / 1000;
	timeout.tv_usec = (sprox_ctx->sprox_timeout.resp_tmo % 1000) * 1000;

	FD_ZERO(&fdset);
	FD_SET(sprox_ctx->com_handle, &fdset);

	res = select(sprox_ctx->com_handle + 1, &fdset, NULL, NULL, &timeout);

	if (res > 0)
	{
		done = read(sprox_ctx->com_handle, b, 1);
		if (done < 1)
		{
			perror("read");
			return FALSE;
		}
		D(printf("+%02X", *b);
		);
		D(fflush(stdout));
		return TRUE;
	}
	else if (res == 0)
	{
		return FALSE;
	}
	else
	{
		perror("select");
		return FALSE;
	}
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
	D(DWORD i);
	int done = 0;
	int tosend;
	int offset;

#ifndef SPROX_API_NO_FTDI
	if (sprox_ctx->com_type == DEVICE_IS_USB)
		return FTDI_SendBurst(sprox_ctx, b, len);
#endif

	tosend = len;
	offset = 0;
	while (tosend)
	{
		done = write(sprox_ctx->com_handle, &b[offset], tosend);
		if (done <= 0)
		{
			perror("write");
			return FALSE;
		}
		tosend -= done;
		offset += done;
	}
	if (done < len)
	{
		return FALSE;
	}

	D(for (i = 0; i < len; i++) printf("-%02X", b[i]););

	return TRUE;
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
	D(DWORD i);
	fd_set  fdset;
	struct timeval timeout;
	int     res;
	int done;
	int torecv;
	int offset;

#ifndef SPROX_API_NO_FTDI
	if (sprox_ctx->com_type == DEVICE_IS_USB)
		return FTDI_RecvBurst(sprox_ctx, b, len);
#endif

	torecv = len;
	offset = 0;
	while (torecv)
	{
		/* Init timeout */
		FD_ZERO(&fdset);
		FD_SET(sprox_ctx->com_handle, &fdset);

		if (offset == 0)
		{
			/* Beginning -> initial timeout */
			timeout.tv_sec = sprox_ctx->sprox_timeout.resp_tmo / 1000;
			timeout.tv_usec = (sprox_ctx->sprox_timeout.resp_tmo % 1000) * 1000;
		}
		else
		{
			/* Chaining -> inter-byte timeout */
			timeout.tv_sec = sprox_ctx->sprox_timeout.byte_tmo / 1000;
			timeout.tv_usec = (sprox_ctx->sprox_timeout.byte_tmo % 1000) * 1000;
		}

		/* Wait */
		res = select(sprox_ctx->com_handle + 1, &fdset, NULL, NULL, &timeout);

		if (res > 0)
		{
			/* Data available */
			done = read(sprox_ctx->com_handle, &b[offset], torecv);
			if (done <= 0)
			{
				perror("read");
				return FALSE;
			}
			torecv -= done;
			offset += done;
		}
		else if (res == 0)
		{
			/* Timeout */
			return FALSE;
		}
		else
		{
			/* Select error */
			perror("select");
			return FALSE;
		}
	}

	D(for (i = 0; i < len; i++) printf("+%02X", b[i]););

	return TRUE;
}

BOOL SerialPowerUp(SPROX_CTX_ST* sprox_ctx)
{
	/* TODO : Add your code here to set POWER to ON (reader running) */
	(void)sprox_ctx;
	return TRUE;
}

BOOL SerialPowerDown(SPROX_CTX_ST* sprox_ctx)
{
	/* TODO : Add your code here to set POWER to OFF (reader halted) */
	(void)sprox_ctx;
	return TRUE;
}

static BOOL SerialSetRs485Mode(SPROX_CTX_ST* sprox_ctx, BOOL rs485_output)
{
	/* TODO : add so code here when using a RS-485 line, to drive the output buffer direction */
	(void)sprox_ctx;
	(void)rs485_output;
	return TRUE;
}

BOOL SerialControl_Reset(SPROX_CTX_ST* sprox_ctx, BOOL can_run)
{
	(void)sprox_ctx;
	(void)can_run;
	if (can_run)
	{
		/* TODO : Add your code here to set RESET to HIGH (reader running) */
	}
	else
	{
		/* TODO : Add your code here to set RESET to LOW (reader resetting) */
	}
	return TRUE;
}

#endif
