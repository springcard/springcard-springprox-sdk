/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  SpringProx API
  --------------

  Copyright (c) 2000-2008 SpringCard SAS, FRANCE - www.springcard.com

  sprox_dlg_asc.c
  ---------------
  Implementation of the dialog with the reader, according to ASCII protocol.

  revision :
  ----------

  JDA 07/04/2004 : created

*/
#include "sprox_api_i.h"

#ifndef SPROX_API_ONLY_BIN

#define MI_LINE_BREAK (1)

//#undef D
//#define D(x) x

SWORD SendCharASC(SPROX_CTX_ST* sprox_ctx, BYTE c)
{
	BYTE    b;

	/* Send the char */
	if (!SendByte(sprox_ctx, c))
		return MI_SER_ACCESS_ERR;

	SPROX_Trace(TRACE_DLG_HI, "-%c", c);

	/* Wait for echo */
	if (!RecvByte(sprox_ctx, &b))
		return MI_SER_TIMEOUT_ERR;

	/* Check value */
	if (b != c)
	{
		SPROX_Trace(TRACE_DLG_HI, "Ascii : wrong echo (%02X!=%02X)", b, c);
		RecvFlush(sprox_ctx);
		return MI_SER_PROTO_ERR;
	}

	/* If we're too fast, reader hasn't enough time to handle the ASCII protocol */
	Sleep(1);

	return MI_OK;
}

SWORD SendByteASC(SPROX_CTX_ST* sprox_ctx, BYTE b)
{
	BYTE    c;
	SWORD   rc;

	c = b / 0x10;
	if (c < 0x0A)
		c += '0';
	else
		c += ('A' - 0x0A);
	rc = SendCharASC(sprox_ctx, c);
	if (rc != MI_OK)
		return rc;

	c = b % 0x10;
	if (c < 0x0A)
		c += '0';
	else
		c += ('A' - 0x0A);
	rc = SendCharASC(sprox_ctx, c);
	if (rc != MI_OK)
		return rc;

	return MI_OK;
}

SWORD RecvByteASC(SPROX_CTX_ST* sprox_ctx, BYTE* b)
{
	BYTE    count = 0;
	BYTE    c, r;

	r = 0x00;
	while (count < 2)
	{
		if (!RecvByte(sprox_ctx, &c))
			return MI_SER_TIMEOUT_ERR;

		if (c == '+')
			return MI_TIME_EXTENSION;

		if ((c == '\r') || (c == '\n'))
		{
			RecvByte(sprox_ctx, &c);
			SPROX_Trace(TRACE_DLG_HI, "Ascii : break");
			return MI_LINE_BREAK;
		}

		if ((c >= '0') && (c <= '9'))
			r |= (c - '0');
		else if ((c >= 'A') && (c <= 'F'))
			r |= (c - 'A' + 0x0A);
		else if ((c >= 'a') && (c <= 'f'))
			r |= (c - 'a' + 0x0A);
		else
		{
			SPROX_Trace(TRACE_DLG_HI, "Ascii : wrong byte %02X(%c)", c, c);
			continue;
		}
		SPROX_Trace(TRACE_DLG_HI, "+%c", c);
		if (count == 0)
			r *= 0x10;
		count++;
	}

	*b = r;
	return MI_OK;
}

SWORD SendFrameASC(SPROX_CTX_ST* sprox_ctx, const BYTE* buffer, WORD buflen)
{
	SWORD   rc;
	BYTE    b;
	WORD    i;

	SerialSetTimeouts(sprox_ctx, RESPONSE_TMO, ASCII_TMO);

	SPROX_Trace(TRACE_DLG_HI, "<send>");

	/* Send the DOLLAR */
	rc = SendCharASC(sprox_ctx, '$');
	if (rc != MI_OK)
		return rc;

	/* Send the whole frame with the SEQUENCE and CHECKSUM */
	for (i = 1; i < (buflen - 1); i++)
	{
		rc = SendByteASC(sprox_ctx, buffer[i]);
		if (rc != MI_OK)
			return rc;
	}

	/* End of frame sequence : "\n" is enough, sending "\r\n" can be problematic */
	if (!SendByte(sprox_ctx, '\n'))
		return MI_SER_ACCESS_ERR;

	SPROX_Trace(TRACE_DLG_HI, "</send>");

	/* Check what we have in return */
	while (RecvByte(sprox_ctx, &b))
	{
		if (b == '\r')
			continue;
		if (b == '\n')
			continue;

		if (b == '-')
		{
			/* NAK !!! */
			SPROX_Trace(TRACE_DLG_HI, "+NAK");
			while (RecvByte(sprox_ctx, &b))
			{
				SPROX_Trace(TRACE_DLG_HI, "+%c", b);
			}
			return MI_SER_PROTO_NAK;
		}

		if (b == '+')
		{
			/* ACK */
			SPROX_Trace(TRACE_DLG_HI, "+ACK");
			return MI_OK;
		}
	}

	return MI_SER_TIMEOUT_ERR;
}

SWORD RecvFrameASC(SPROX_CTX_ST* sprox_ctx, BYTE* buffer, WORD* buflen)
{
	SWORD   rc;
	BYTE    b;
	WORD    i;

	SPROX_Trace(TRACE_DLG_HI, "<recv>");

	SerialSetTimeouts(sprox_ctx, RESPONSE_TMO, ASCII_TMO);

	/* Receive the whole frame body */
	i = 1;
	while (i < *buflen)
	{
		rc = RecvByteASC(sprox_ctx, &b);
		if (rc == MI_OK)
		{
			buffer[i] = b;
			if (i == 1)
				SerialSetTimeouts(sprox_ctx, ASCII_TMO, ASCII_TMO);
			i++;
		}
		else
		{
			if (rc == MI_LINE_BREAK)
			{
				if (i > 1)
				{
					/* End of buffer */
					*buflen = i;
					SPROX_Trace(TRACE_DLG_HI, "Ascii : break received after %d bytes", i);
					SPROX_Trace(TRACE_DLG_HI, "</recv>");
					return MI_OK;
				}
				else
					continue;
			}
			else
				if (rc != MI_TIME_EXTENSION)
				{
					SPROX_Trace(TRACE_DLG_HI, "Ascii : error %d", rc);
				}
			return rc;
		}
	}

	/* My buffer is too short ? */
	return MI_SER_LENGTH_ERR;
}

#endif
