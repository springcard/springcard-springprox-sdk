/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  SpringProx API
  --------------

  Copyright (c) 2000-2008 SpringCard SAS, FRANCE - www.springcard.com

  sprox_dlg_bus.c
  ---------------
  Implementation of the dialog with the reader, according to bus binary protocol.

  revision :
  ----------

  JDA 07/04/2004 : created

*/

#include "sprox_api_i.h"

#ifndef SPROX_API_ONLY_BIN
#ifndef SPROX_API_ONLY_ASC

SWORD SendFrameBUS(SPROX_CTX_ST *sprox_ctx, const BYTE *buffer, WORD buflen)
{
  BYTE frame[SPROX_FRAME_CONTENT_SIZE + 3];
  BYTE b;
  BYTE retry = 3;

  if (buflen > SPROX_FRAME_CONTENT_SIZE)
    return MI_SER_LENGTH_ERR;

  frame[0] = ASCII_SOH;
  frame[1] = sprox_ctx->com_address;
  frame[2] = ASCII_ACK;

  if (frame[1] == 0x00)
    frame[1] = 0xFF;

  /* We are to wait shortly */
  SerialSetTimeouts(sprox_ctx, INTER_BYTE_TMO, INTER_BYTE_TMO);

again:
  /* Send the whole frame with the header */
  memcpy(&frame[3], buffer, buflen);
  if (!SendBurst(sprox_ctx, frame, (WORD) (3 + buflen)))
    return MI_SER_ACCESS_ERR;

  /* Wait for an ACK */
  if (!RecvByte(sprox_ctx, &b))
  {
    SPROX_Trace(TRACE_DLG_HI, "ACK not received\n");

    if (retry--)
    {
      Sleep(100);
      goto again;
    }

    return MI_SER_NORESP_ERR;
  }

  if (b != ASCII_ACK)
  {
    if (b == ASCII_NAK)
    {
      RecvByte(sprox_ctx, &b);
      SPROX_Trace(TRACE_DLG_HI, ">NAK(%02X)\n", b);
      RecvFlush(sprox_ctx);
      return MI_SER_PROTO_NAK;
    }
    
    SPROX_Trace(TRACE_DLG_HI, ">ACK != %02X\n", b);
    RecvFlush(sprox_ctx);

    if (retry--)
    {
      Sleep(100);
      goto again;
    }

    return MI_SER_PROTO_ERR;
  }

  return MI_OK;
}

SWORD RecvFrameBUS(SPROX_CTX_ST * sprox_ctx, BYTE * buffer, WORD * buflen)
{
  BYTE    b;
  WORD    p, l;
  BYTE    retry = 100;

  /* Wait for the beginning of the frame */
  SerialSetTimeouts(sprox_ctx, RESPONSE_TMO, INTER_BYTE_TMO);

  while (retry--)
  {
    if (!RecvByte(sprox_ctx, &b))
    {
      SPROX_Trace(TRACE_DLG_HI, "SOH not received\n");
      return MI_SER_NORESP_ERR;
    }

    /* The first byte must be SOH */
    if (b == ASCII_SOH)
      break;

    if (b == ASCII_NAK)
    {
      RecvByte(sprox_ctx, &b);
      SPROX_Trace(TRACE_DLG_HI, ">!SOH>NAK(%02X)\n", b);
      RecvFlush(sprox_ctx);
      return MI_SER_PROTO_NAK;
    }
  }

  if (b != ASCII_SOH)
  {
    SPROX_Trace(TRACE_DLG_HI, ">SOH != %02X\n", b);
    return MI_SER_PROTO_ERR;
  }

  if (!RecvByte(sprox_ctx, &b))
  {
    SPROX_Trace(TRACE_DLG_HI, "Address not received\n");
    return MI_SER_NORESP_ERR;
  }

  if (b != 0x00)
  {
    SPROX_Trace(TRACE_DLG_HI, "Wrong address %02X\n", b);
    RecvFlush(sprox_ctx);
    return MI_SER_PROTO_ERR;
  }

  if (!RecvByte(sprox_ctx, &b))
  {
    SPROX_Trace(TRACE_DLG_HI, "ACK not received\n");
    return MI_SER_NORESP_ERR;
  }

  /* The next byte must be ACK */
  if (b != ASCII_ACK)
  {
    if (b == ASCII_NAK)
    {
      RecvByte(sprox_ctx, &b);
      SPROX_Trace(TRACE_DLG_HI, ">NAK(%02X)\n", b);
      RecvFlush(sprox_ctx);
      return MI_SER_PROTO_NAK;     
    }

    SPROX_Trace(TRACE_DLG_HI, ">ACK != %02X\n", b);
    RecvFlush(sprox_ctx);
    return MI_SER_PROTO_ERR;
  }

  SerialSetTimeouts(sprox_ctx, OSI_PROTOCOL_TMO, INTER_BYTE_TMO);

  p = 0;

  /* I need SEQ.... */
  if (!RecvByte(sprox_ctx, &buffer[p++]))
    goto timeout;

  /* I need CMD.... */
  if (!RecvByte(sprox_ctx, &buffer[p++]))
    goto timeout;

  /* I need LEN.... */
  l = 0;
  for (;;)
  {
    if (!RecvByte(sprox_ctx, &b))
      goto timeout;
    buffer[p++] = b;
    l += b;
    if (b < 0x80)
      break;
  }

  /* Now I can receive the frame body + CRC */
  if (!RecvBurst(sprox_ctx, &buffer[p], (WORD) (l + 1)))
    goto timeout;

  p += l + 1;

  if (p > *buflen)
  {
    return MI_SER_LENGTH_ERR;   /* Oups, too late... */
  }

  /* Done with this */
  *buflen = p;

  return MI_OK;

timeout:
  return MI_SER_TIMEOUT_ERR;
}

/**f* SpringProx.API/SPROX_ReaderSelectAddress
 *
 * NAME
 *   SPROX_ReaderSelectAddress
 *
 * DESCRIPTION
 *   Select the adress of the reader you want to work with
 *
 * INPUTS
 *   BYTE address       : address of the reader
 *
 * RETURNS
 *   MI_OK              : success
 *
 * NOTES
 *   This function let the caller application switch from one reader to
 *   another when working in RS485 binary bus only.
 *
 **/
SPROX_API_FUNC(ReaderSelectAddress) (SPROX_PARAM  BYTE address)
{
  SPROX_PARAM_TO_CTX;
  
  if (address == 0x00)
    address = 0xFF;

  sprox_ctx->com_address = address;
  return MI_OK;
}

#endif
#endif
