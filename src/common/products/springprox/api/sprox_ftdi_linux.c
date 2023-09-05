/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  SpringProx API
  --------------

  Copyright (c) 2000-2008 SpringCard SAS, FRANCE - www.springcard.com

  serial_ftdi_linux.c
  ------------------
  serial communication on top of USB, under Linux

  revision :
  ----------

	JDA 17/12/2006 : creation
	JDA 19/02/2007 : major improvements, first really working release


	note :
	------

	serial_usb_linux.c uses libftdi and libusb libraries, released under
	the L-GPL license (see COPYING.lig in libftdi and libusb directories).

	If you do not want to (or do not need to) link against libftdi and libusb,
	define SPROX_API_NO_FTDI to disable this module.

*/

#include "sprox_api_i.h"

#if (defined (LINUX) && !defined(SPROX_API_NO_FTDI))

#include <sys/time.h>
#include <signal.h>

//#undef D
//#define D(x) x

static struct ftdi_context ftdi_ctx;

/*
 * Try to open the FTDI USB port as specified in device.
 * Format for device string is "FTDI:XXXX:YYYY" where
 * - XXXX is the Vendor ID
 * - YYYY is the Product ID
 */
BOOL FTDI_Open(SPROX_CTX_ST* sprox_ctx, const TCHAR* device)
{
	int ret;
	int vid, pid;

	if (sprox_ctx == NULL) return FALSE;
	if ((device == NULL) || (!strlen(device))) return FALSE;
	if (sscanf(device, "FTDI:%04X:%04X", &vid, &pid) != 2) return FALSE;

	D(printf("Opening FTDI:%04X:%04X\n", vid, pid));

	if (ftdi_init(&ftdi_ctx) < 0)
	{
		fprintf(stderr, "ftdi_init: %s\n", ftdi_get_error_string(&ftdi_ctx));
		return FALSE;
	}

	ret = ftdi_usb_open(&ftdi_ctx, vid, pid);
	if (ret < 0)
	{
		D(fprintf(stderr, "ftdi_usb_open: %s\n", ftdi_get_error_string(&ftdi_ctx)));
		ftdi_deinit(&ftdi_ctx);
		return FALSE;
	}

	sprox_ctx->com_handle = ret; /* Don't really know what to do with this */
	return TRUE;
}


void FTDI_Close(SPROX_CTX_ST* sprox_ctx)
{
	if (sprox_ctx == NULL) return;
	if (sprox_ctx->com_handle != -1)
	{
		if (ftdi_usb_close(&ftdi_ctx) < 0)
			fprintf(stderr, "ftdi_usb_close: %s\n", ftdi_get_error_string(&ftdi_ctx));
		ftdi_deinit(&ftdi_ctx);
		sprox_ctx->com_handle = -1;
	}
}

BOOL FTDI_SetBaudrate(SPROX_CTX_ST* sprox_ctx, DWORD baudrate)
{
	if (ftdi_set_baudrate(&ftdi_ctx, (int)baudrate) < 0)
	{
		fprintf(stderr, "ftdi_set_baudrate: %s\n", ftdi_get_error_string(&ftdi_ctx));
		return FALSE;
	}

	if (ftdi_set_line_property(&ftdi_ctx, BITS_8, STOP_BIT_1, NONE) < 0)
	{
		fprintf(stderr, "ftdi_set_line_property: %s\n", ftdi_get_error_string(&ftdi_ctx));
		return FALSE;
	}

	if (ftdi_setflowctrl(&ftdi_ctx, SIO_DISABLE_FLOW_CTRL) < 0)
	{
		fprintf(stderr, "ftdi_set_flowctrl: %s\n", ftdi_get_error_string(&ftdi_ctx));
		return FALSE;
	}

	return TRUE;
}

BOOL FTDI_SendBurst(SPROX_CTX_ST* sprox_ctx, BYTE* b, WORD len)
{
	D(DWORD i);
	int done;
	int tosend;
	int offset;

	/* Set timeouts */
	ftdi_ctx.usb_write_timeout = 1000;

	tosend = len;
	offset = 0;

	/* Loop until we've send the expected amount of data */
	while (tosend)
	{
		done = ftdi_write_data(&ftdi_ctx, &b[offset], tosend);
		if (done <= 0)
		{
			fprintf(stderr, "ftdi_write_data: %s\n", ftdi_get_error_string(&ftdi_ctx));
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
	D(printf("\n"));
	return TRUE;
}

static volatile BOOL timer_overflow;

static void timer_handler(int unused)
{
	timer_overflow = TRUE;
}

static void timer_start(DWORD ms_timeout)
{
	struct itimerval val;

	val.it_interval.tv_sec = 0;
	val.it_interval.tv_usec = 0;
	val.it_value.tv_sec = ms_timeout / 1000;
	val.it_value.tv_usec = (ms_timeout % 1000) * 1000;

	timer_overflow = FALSE;
	signal(SIGALRM, timer_handler);
	if (setitimer(ITIMER_REAL, &val, NULL))
	{
		perror("setitimer");
	}
}


BOOL FTDI_RecvBurst(SPROX_CTX_ST* sprox_ctx, BYTE* b, WORD len)
{
	D(DWORD i);
	int done;
	int torecv;
	int offset;

	/* Set timeouts */
	ftdi_ctx.usb_read_timeout = sprox_ctx->sprox_timeout.resp_tmo + len * sprox_ctx->sprox_timeout.byte_tmo;

	torecv = len;
	offset = 0;

	/* Loop until we've received the expected amount of data */
	while (torecv)
	{
		/*
		 * TRICKY TRICKY TRICKY TRICKY // TODO TODO TODO TODO
		 *
		 * It seems that the usb_read_timeout has no effect at all (or really really little effect)
		 * on actual timeouts in our test environment (Linux Kernel 2.4.27-2-386, Debian 3.3.6-8).
		 *
		 * To overcome this we use an infinite loop + an interval timer
		 *
		 */
		timer_start(ftdi_ctx.usb_read_timeout);
		while (!timer_overflow)
		{
			done = ftdi_read_data(&ftdi_ctx, &b[offset], torecv);
			if (done != 0)
				break;
		}

		if (done == 0)
		{
			D(fprintf(stderr, "ftdi_read_data: timeout\n"));
			D(for (i = 0; i < offset; i++) printf("+%02X", b[i]););
			D(printf("\n"));
			return FALSE;
		}

		if (done < 0)
		{
			fprintf(stderr, "ftdi_read_data: %s\n", ftdi_get_error_string(&ftdi_ctx));
			return FALSE;
		}
		torecv -= done;
		offset += done;

		ftdi_ctx.usb_read_timeout = torecv * sprox_ctx->sprox_timeout.byte_tmo;
	}

	D(for (i = 0; i < len; i++) printf("+%02X", b[i]););
	D(printf("\n"));
	return TRUE;
}

#endif
