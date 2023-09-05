/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  SpringProx API
  --------------

  Copyright (c) 2000-2008 SpringCard SAS, FRANCE - www.springcard.com

  sprox_dlg_bin.c
  ---------------
  Implementation of the dialog with the reader, according to fast binary protocol.

  revision :
  ----------

  JDA 07/04/2004 : created
  JDA 28/02/2012 : improved timeout handling

*/

#include "sprox_api_i.h"

#ifndef SPROX_API_ONLY_ASC

WORD RESPONSE_TMO_BIN = RESPONSE_TMO;

//#undef D
//#define D(x) x

SWORD SendFrameBIN(SPROX_CTX_ST* sprox_ctx, const BYTE* buffer, WORD buflen)
{
	if (buffer == NULL) return MI_LIB_INTERNAL_ERROR;

	SPROX_Trace(TRACE_DLG_HI, "<SendFrameBIN(%d)>", buflen);

	/* Send the SYN byte */
	if (!SendByte(sprox_ctx, ASCII_SYN))
	{
		SPROX_Trace(TRACE_DLG_HI, "MI_SER_ACCESS_ERR");
		return MI_SER_ACCESS_ERR;
	}

	SPROX_Trace(TRACE_DLG_HI, "_.");

	/* Send the whole frame */
	if (!SendBurst(sprox_ctx, buffer, buflen))
	{
		SPROX_Trace(TRACE_DLG_HI, "MI_SER_ACCESS_ERR");
		return MI_SER_ACCESS_ERR;
	}

	SPROX_Trace(TRACE_DLG_HI, "</SendFrameBIN>");

	return MI_OK;
}

#if 1

/* New code (> 1.41.4) with only two calls to RecvBurst and one RecvByte */
SWORD RecvFrameBIN(SPROX_CTX_ST* sprox_ctx, BYTE* buffer, WORD* buflen)
{
	WORD offset, len;
	BYTE header[5] = { 0 };

	SPROX_Trace(TRACE_DLG_HI, "<RecvFrameBIN>");

	/* Wait for the beginning of the frame                       */
	/* We want SYN+SEQ+CMD+LEN+CRC, this will be enough if LEN=0 */

	/* NOTE : we MUST split 1st request in 2 (instead of receiving 5   */
	/*        bytes at once) because there must be a bug in usbser.sys */
	/*        (Microsoft's driver for USB-CDC-ACM class) : value for   */
	/*        ReadTotalTimeoutConstant seems to be ignored.            */

	if (!SerialSetTimeouts(sprox_ctx, RESPONSE_TMO_BIN, RESPONSE_TMO_BIN))
	{
		SPROX_Trace(TRACE_DLG_HI, "SerialSetTimeouts failed");
		return MI_SER_ACCESS_ERR;
	}
	if (!RecvBurst(sprox_ctx, &header[0], 1))
	{
		/* The error here can be either a timeout (no answer) or a truncated answer (can be a NACK) */
		SPROX_Trace(TRACE_DLG_HI, "Timeout receiving STX");
		if (header[0] == 0x00)
		{
			/* No answer at all */
			SPROX_Trace(TRACE_DLG_HI, "MI_SER_NORESP_ERR");
			return MI_SER_NORESP_ERR;
		}
		/* Something arrived... */
		goto not_syn;
	}

	/* Shorten timeouts, remaining part of buffer must come immediatly */
	if (!SerialSetTimeouts(sprox_ctx, INTER_BYTE_TMO, INTER_BYTE_TMO))
	{
		SPROX_Trace(TRACE_DLG_HI, "SerialSetTimeouts failed");
		return MI_SER_ACCESS_ERR;
	}

	if (!RecvBurst(sprox_ctx, &header[1], 4))
	{
		/* The error here can be either a timeout (no answer) or a truncated answer (can be a NACK) */
		SPROX_Trace(TRACE_DLG_HI, "Timeout receiving header");
		/* Something arrived... */
		goto not_syn;
	}


	/* The first byte must be SYN */
	if (header[0] != ASCII_SYN)
		goto not_syn;

	/* Now enqueue the buffer */
	offset = 0;

	/* SEQ, CMD */
	buffer[offset++] = header[1];
	buffer[offset++] = header[2];

	/* LEN */
	buffer[offset++] = header[3];
	if (header[3] >= 0x80)
	{
		/* Another LEN byte to be provided */
		len = header[3] + header[4];
		buffer[offset++] = header[4];
		while (header[4] >= 0x80)
		{
			/* Yet another LEN byte to be provided */
			if (!RecvByte(sprox_ctx, &header[4]))
			{
				SPROX_Trace(TRACE_DLG_HI, "Timeout receiving length");
				goto rec_tmo; /* Timeout inside the frame */
			}
			len += header[4];
			buffer[offset++] = header[4];
		}
		/* We've already consumed header[4], so there's 1 byte missing */
		len += 1;
	}
	else
	{
		/* Single LEN byte */
		len = header[3];
		/* Take last byte from header ; this is first byte of data (or the CRC when LEN=0) */
		buffer[offset++] = header[4];
	}

	/* Check size before going on */
	if ((offset + len) > *buflen)
	{
		/* Provided buffer is too small */
		RecvFlush(sprox_ctx);
		SPROX_Trace(TRACE_DLG_HI, "MI_SER_LENGTH_ERR");
		return MI_SER_LENGTH_ERR;
	}

	/* Now receive remaining data + CRC */
	if (len)
	{
		if (!RecvBurst(sprox_ctx, &buffer[offset], len))
		{
			SPROX_Trace(TRACE_DLG_HI, "Timeout receiving %d bytes (data+CRC)", len);
			goto rec_tmo; /* Timeout inside the frame */
		}
	}

	/* Done ! */
	*buflen = offset + len;
	SPROX_Trace(TRACE_DLG_HI, "</RecvFrameBIN(%d)>", *buflen);
	return MI_OK;

	/* Error handling */
	/* -------------- */

not_syn:
	/* Error : first byte received is not SYN */
	if (header[0] == ASCII_NAK)
	{
		/* NACK ; assume second byte is the code */
		SPROX_Trace(TRACE_DLG_HI, ">NAK(%02X)\n", header[1]);
		RecvFlush(sprox_ctx);
		return MI_SER_PROTO_NAK;
	}
	/* Invalid code... */
	SPROX_Trace(TRACE_DLG_HI, ">SYN != %02X\n", header[0]);
	RecvFlush(sprox_ctx);
	SPROX_Trace(TRACE_DLG_HI, "MI_SER_PROTO_ERR");
	return MI_SER_PROTO_ERR;

rec_tmo:
	/* Error : timeout inside the frame (this is different from a timeout before the frame...) */
	SPROX_Trace(TRACE_DLG_HI, "MI_SER_TIMEOUT_ERR");
	return MI_SER_TIMEOUT_ERR;
}

#else

/* Old code (< 1.41.4) with lots of RecvByte and one RecvBurst */
SWORD RecvFrameBIN(SPROX_CTX_ST* sprox_ctx, BYTE* buffer, WORD* buflen)
{
	WORD offset, length;
	BYTE b;

	if (buffer == NULL) return MI_LIB_INTERNAL_ERROR;
	if (buflen == NULL) return MI_LIB_INTERNAL_ERROR;

	SPROX_Trace(TRACE_DLG_HI, "<RecvFrameBIN (%d)>", *buflen);

	/* Wait for the beginning of the frame */
	if (!SerialSetTimeouts(sprox_ctx, RESPONSE_TMO, RESPONSE_TMO))
	{
		SPROX_Trace(TRACE_DLG_HI, "SerialSetTimeouts failed");
		return MI_SER_ACCESS_ERR;
	}

	if (!RecvByte(sprox_ctx, &b))
	{
		SPROX_Trace(TRACE_DLG_HI, "MI_SER_NORESP_ERR");
		return MI_SER_NORESP_ERR;
	}

	if (!SerialSetTimeouts(sprox_ctx, INTER_BYTE_TMO, INTER_BYTE_TMO))
	{
		SPROX_Trace(TRACE_DLG_HI, "SerialSetTimeouts failed");
		return MI_SER_ACCESS_ERR;
	}

	/* The first byte must be SYN */
	if (b != ASCII_SYN)
	{
		if (b == ASCII_NAK)
		{
			RecvByte(sprox_ctx, &b);
			SPROX_Trace(TRACE_DLG_HI, ">NAK(%02X)\n", b);
			RecvFlush(sprox_ctx);
			return MI_SER_PROTO_NAK;
		}

		SPROX_Trace(TRACE_DLG_HI, ">SYN != %02X\n", b);
		RecvFlush(sprox_ctx);
		SPROX_Trace(TRACE_DLG_HI, "MI_SER_PROTO_ERR");
		return MI_SER_PROTO_ERR;
	}

	offset = 0;

	/* I need SEQ.... */
	if (!RecvByte(sprox_ctx, &buffer[offset++]))
		goto timeout;

	/* I need CMD.... */
	if (!RecvByte(sprox_ctx, &buffer[offset++]))
		goto timeout;

	/* I need LEN.... */
	length = 0;
	for (;;)
	{
		if (!RecvByte(sprox_ctx, &b))
			goto timeout;
		buffer[offset++] = b;
		length += b;
		if (b < 0x80)
			break;
	}

	/* Check size before going further */
	if ((offset + length + 1) > *buflen)
	{
		/* Provided buffer is too small */
		SPROX_Trace(TRACE_DLG_HI, ">Expect %d bytes, room for only %d", offset + length + 1, *buflen);
		RecvFlush(sprox_ctx);
		SPROX_Trace(TRACE_DLG_HI, "MI_SER_LENGTH_ERR");
		return MI_SER_LENGTH_ERR;
	}

	SPROX_Trace(TRACE_DLG_HI, "_...(%d+1)...", length);

	/* Now I can receive the frame body + CRC */
	if (!RecvBurst(sprox_ctx, &buffer[offset], (WORD)(length + 1)))
		goto timeout;

	/* Done with this */
	*buflen = offset + length + 1;

	SPROX_Trace(TRACE_DLG_HI, "</RecvFrameBIN(%d)>", *buflen);
	return MI_OK;

timeout:
	SPROX_Trace(TRACE_DLG_HI, "MI_SER_TIMEOUT_ERR");
	return MI_SER_TIMEOUT_ERR;
}
#endif

#endif
