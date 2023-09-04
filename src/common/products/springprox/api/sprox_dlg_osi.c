/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  SpringProx API
  --------------

  Copyright (c) 2000-2008 SpringCard SAS, FRANCE - www.springcard.com

  sprox_dlg_osi.c
  ---------------
  Implementation of the dialog with the reader, according to OSI3964 protocol

  revision :
  ----------

  JDA 07/04/2004 : created

*/
#include "sprox_api_i.h"

#ifndef SPROX_API_ONLY_BIN
#ifndef SPROX_API_ONLY_ASC

//#undef D
//#define D(x) x

SWORD SendFrameOSI(SPROX_CTX_ST * sprox_ctx, const BYTE *buffer, WORD buflen)
{
  BYTE    b;
  WORD    i;

  /* Send the START OF TEXT */
  if (!SendByte(sprox_ctx, ASCII_STX))
    return MI_SER_ACCESS_ERR;

  SPROX_Trace(TRACE_DLG_HI, "-STX");

retry:
  /* Wait for the ACKNOWLEDGE (DLE) */
  SerialSetTimeouts(sprox_ctx, OSI_PROTOCOL_TMO, INTER_BYTE_TMO);
  if (!RecvByte(sprox_ctx, &b))
  {
    SPROX_Trace(TRACE_DLG_HI, ">First DLE not received !\n");
    return MI_SER_TIMEOUT_ERR;
  }
  if (b != ASCII_DLE)
  {
    if (b == 0x00)
      goto retry;

    SPROX_Trace(TRACE_DLG_HI, ">First DLE != %02X\n", b);
    RecvFlush(sprox_ctx);
    return MI_SER_PROTO_ERR;
  }
	SPROX_Trace(TRACE_DLG_HI, "+DLE");

  /* Send the whole frame */
  for (i = 0; i < buflen; i++)
  {
    if (!SendByte(sprox_ctx, buffer[i]))
      return MI_SER_ACCESS_ERR;

    /* DLE is a specific value, must be repeated ! */
    if (buffer[i] == ASCII_DLE)
    {
      if (!SendByte(sprox_ctx, ASCII_DLE))
        return MI_SER_ACCESS_ERR;
    }
    SPROX_Trace(TRACE_DLG_HI, "-%02X", buffer[i]);
  }

  /* This is the end of frame sequence */
  if (!SendByte(sprox_ctx, ASCII_DLE))
    return MI_SER_ACCESS_ERR;
  SPROX_Trace(TRACE_DLG_HI, "-DLE");

  if (!SendByte(sprox_ctx, ASCII_ETX))
    return MI_SER_ACCESS_ERR;
  SPROX_Trace(TRACE_DLG_HI, "-ETX");

  /* Wait for the ACKNOWLEDGE (DLE) */
  if (RecvByte(sprox_ctx, &b))
  {
    if (b == ASCII_NAK)
    {
      RecvByte(sprox_ctx, &b);
      SPROX_Trace(TRACE_DLG_HI, ">NAK(%02X)\n", b);
      RecvFlush(sprox_ctx);
      return MI_SER_PROTO_NAK;
    }
    if (b == ASCII_DLE)
    {
      SPROX_Trace(TRACE_DLG_HI, ">DLE");
      return MI_OK;
    }
    SPROX_Trace(TRACE_DLG_HI, ">Last DLE != %02X", b);
    return MI_SER_PROTO_ERR;
  }
  SPROX_Trace(TRACE_DLG_HI, ">Last DLE not received !\n");
  return MI_SER_TIMEOUT_ERR;
}

SWORD RecvFrameOSI(SPROX_CTX_ST * sprox_ctx, BYTE * buffer, WORD * buflen)
{
  BOOL    remind_dle = FALSE;
  BYTE    b;
  WORD    i;

  /* Wait for the beginning of the frame */
  SerialSetTimeouts(sprox_ctx, RESPONSE_TMO, RESPONSE_TMO);
again:
  if (!RecvByte(sprox_ctx, &b))
  {
    SPROX_Trace(TRACE_DLG_HI, "STX not received");
    return MI_SER_NORESP_ERR;
  }
  if (b == ASCII_DLE) goto again;

  /* The first byte must be START OF TEXT */
  if (b != ASCII_STX)
  {
    SPROX_Trace(TRACE_DLG_HI, ">STX != %02X", b);
    RecvFlush(sprox_ctx);
    return MI_SER_PROTO_ERR;
  }

  SPROX_Trace(TRACE_DLG_HI, "+STX");

  /* I must acknowledge it by DLE */
  if (!SendByte(sprox_ctx, ASCII_DLE))
    return MI_SER_ACCESS_ERR;

  SPROX_Trace(TRACE_DLG_HI, "-DLE");

  SerialSetTimeouts(sprox_ctx, OSI_PROTOCOL_TMO, INTER_BYTE_TMO);

  /* Now I can receive the frame body */
  i = 0;
  while (i < *buflen)
  {
    if (!RecvByte(sprox_ctx, &b))
      return MI_SER_TIMEOUT_ERR;

    if (b == ASCII_DLE)
    {
      /* DLE is a specific value, must be repeated ! */
      if (remind_dle)
      {
        buffer[i] = ASCII_DLE;
        remind_dle = FALSE;
      } else
      {
        remind_dle = TRUE;
        continue;
      }
    } else
    if (b == ASCII_ETX)
    {
      /* DLE + ETX -> end of frame */
      if (remind_dle)
      {
        SPROX_Trace(TRACE_DLG_HI, "+DLE");
        SPROX_Trace(TRACE_DLG_HI, "+ETX");
        
        if (!SendByte(sprox_ctx, ASCII_DLE))
          return MI_SER_ACCESS_ERR;

        SPROX_Trace(TRACE_DLG_HI, "-DLE");

        goto end_of_frame;
      }
      /* "Regular" DLE */
      buffer[i] = ASCII_ETX;
    } else
    {
      /* New byte... */
      buffer[i] = b;
    }
    /* Next i */
    SPROX_Trace(TRACE_DLG_HI, "+%02X", buffer[i]);
    i++;
  }

  /* My buffer is too short ? */
  return MI_SER_LENGTH_ERR;

end_of_frame:

  /* I must acknowledge it by DLE */

  SPROX_Trace(TRACE_DLG_HI, NULL);

  /* Done with this */
  *buflen = i;
  return MI_OK;
}

#endif
#endif
