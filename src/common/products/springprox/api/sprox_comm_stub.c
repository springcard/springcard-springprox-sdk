/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  SpringProx API
  --------------

  Copyright (c) 2000-2008 SpringCard SAS, FRANCE - www.springcard.com

  serial_stub.c
  -------------
  Serial communication entry points

*/

#include "sprox_api_i.h"

#if (!defined(WIN32) && !defined(LINUX))

/*
 * SerialLookup
 * ------------
 * Lookup for the SpringProx device trying every available serial devices
 * one after the other.
 * If host has only one serial device, or if the SpringProx is always bound
 * to the same serial device, this function can be only an alias to
 * SerialOpen + ReaderConnect
 */
BOOL SerialLookup(SPROX_CTX_ST* sprox_ctx)
{
	/* Sample sequence can be something like that : */
	if (SerialOpen(sprox_ctx, "DEVICE0"))
	{
		if (ReaderConnect(sprox_ctx) == MI_OK)
			return TRUE;
		SerialClose(sprox_ctx);
	}
	if (SerialOpen(sprox_ctx, "DEVICE1"))
	{
		if (ReaderConnect(sprox_ctx) == MI_OK)
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
	/* TODO : open the device and perform the 1st-time initialization */
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
	/* TODO : close the device */
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

}

BOOL SerialPowerUp(SPROX_CTX_ST* sprox_ctx)
{

}

BOOL SerialPowerDown(SPROX_CTX_ST* sprox_ctx)
{

}

BOOL SerialReset(SPROX_CTX_ST* sprox_ctx)
{

}

#endif
