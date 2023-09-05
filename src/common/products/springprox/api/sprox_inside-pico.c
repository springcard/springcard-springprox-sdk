/**h* SpringProx.API/InsidePico
 *
 * NAME
 *   SpringProx.API :: Inside Pico
 *
 * DESCRIPTION
 *   Implementation of Inside Pico
 *
 **/

 /*

   SpringProx API
   --------------

   Copyright (c) 2000-2017 SpringCard SAS, FRANCE - www.springcard.com

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
   TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   History
   -------

   JDA 03/05/2017 : created with part of sprox_14443-3.c

 */
#include "sprox_api_i.h"

SPROX_API_FUNC(Pico_ExchangeEx) (SPROX_PARAM  BYTE opcode, const BYTE* send_data, WORD send_bytelen, BYTE* recv_data, WORD* recv_bytelen, BYTE append_crc, WORD timeout)
{
	BYTE    buffer[SPROX_FRAME_CONTENT_SIZE];
	WORD    rlen = sizeof(buffer);
	SWORD   rc;

	if (send_data == NULL) return MI_LIB_CALL_ERROR;
	if ((send_bytelen + 7) >= SPROX_FRAME_CONTENT_SIZE) return MI_COMMAND_OVERFLOW;

	buffer[0] = (BYTE)(send_bytelen / 0x0100);
	buffer[1] = (BYTE)(send_bytelen % 0x0100);
	if (recv_bytelen != NULL)
	{
		buffer[2] = (BYTE)(*recv_bytelen / 0x0100);
		buffer[3] = (BYTE)(*recv_bytelen % 0x0100);
	}
	else
	{
		buffer[2] = (BYTE)(rlen / 0x0100);
		buffer[3] = (BYTE)(rlen % 0x0100);
	}
	buffer[4] = append_crc;
	buffer[5] = (BYTE)(timeout / 0x0100);
	buffer[6] = (BYTE)(timeout % 0x0100);

	memcpy(&buffer[7], send_data, send_bytelen);

	rc = SPROX_DLG_FUNC(SPROX_PARAM_P  opcode, buffer, (WORD)(send_bytelen + 7), buffer, &rlen);
	if (rc != MI_OK)
		return rc;

	if ((rlen <= 2) || ((rlen - 2) != (buffer[0] * 0x0100 + buffer[1])))
		return MI_RESPONSE_INVALID;

	rlen -= 2;
	if (recv_bytelen != NULL)
	{
		if ((*recv_bytelen != 0) && (*recv_bytelen < rlen))
			return MI_RESPONSE_OVERFLOW;
		*recv_bytelen = rlen;
	}

	if (recv_data != NULL)
	{
		memcpy(recv_data, &buffer[2], rlen);
	}

	return rc;
}

/**f* SpringProx.API/SPROX_Pico_Exchange14443
 *
 * NAME
 *   SPROX_Pico_Exchange14443
 *
 * DESCRIPTION
 *   Low-level frame exchange with currently activated Inside Picotag (or HID iClass) card.
 *
 * INPUTS
 *   const BYTE send_data[]  : command (buffer PCD->PICC)
 *   WORD       send_bytelen : length of send_data
 *   BYTE       recv_data[]  : answer (buffer PICC->PCD)
 *   WORD      *recv_bytelen : IN  : size of recv_data (max length)
 *                             OUT : actual length of recv_data
 *   BOOL       calc_crc     : if TRUE, the last 2 bytes of send_data
 *                             will be replaced by a valid 14443 CRC_B
 *                             if FALSE, the CRC must have been computed
 *                             by caller
 *   WORD       timeout      : timeout of the exchange, in ETU
 *                             (at 106kbps, 1 ETU = 9,44s)
 *
 * RETURNS
 *   MI_OK              : success, card answer to be found in recv_data
 *   Other code if any error has occured.
 *
 * NOTES
 *   Remember that send_bytelen (and recv_bytelen) are the actual length
 *   of the frame, INCLUDING THE CRC (on the 2-last bytes).
 *   Even if calc_crc is TRUE, the caller must provide room in send_data[]
 *   (and in recv_data[]) to store the CRC.
 *
 **/
SPROX_API_FUNC(Pico_Exchange14443) (SPROX_PARAM  const BYTE* send_data, WORD send_bytelen, BYTE* recv_data, WORD* recv_bytelen, BYTE append_crc, WORD timeout)
{
	SWORD rc;

	rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_INSIDE_PICO_14443);
	if (rc != MI_OK) return rc;

	rc = SPROX_API_CALL(Pico_ExchangeEx) (SPROX_PARAM_P  SPROX_PICCEXCHANGEBLOCK_B, send_data, send_bytelen, recv_data, recv_bytelen, append_crc, timeout);
	return rc;
}

/**f* SpringProx.API/SPROX_Pico_Exchange15693
 *
 * NAME
 *   SPROX_Pico_Exchange15693
 *
 * DESCRIPTION
 *   Low-level frame exchange with currently activated Inside Picotag (or HID iClass) card.
 *
 * INPUTS
 *   const BYTE send_data[]  : command (buffer PCD->PICC)
 *   WORD       send_bytelen : length of send_data
 *   BYTE       recv_data[]  : answer (buffer PICC->PCD)
 *   WORD      *recv_bytelen : IN  : size of recv_data (max length)
 *                             OUT : actual length of recv_data
 *   BOOL       calc_crc     : if TRUE, the last 2 bytes of send_data
 *                             will be replaced by a valid 14443 CRC_B
 *                             if FALSE, the CRC must have been computed
 *                             by caller
 *   WORD       timeout      : timeout of the exchange, in ETU
 *                             (at 106kbps, 1 ETU = 9,44s)
 *
 * RETURNS
 *   MI_OK              : success, card answer to be found in recv_data
 *   Other code if any error has occured.
 *
 * NOTES
 *   Remember that send_bytelen (and recv_bytelen) are the actual length
 *   of the frame, INCLUDING THE CRC (on the 2-last bytes).
 *   Even if calc_crc is TRUE, the caller must provide room in send_data[]
 *   (and in recv_data[]) to store the CRC.
 *
 **/
SPROX_API_FUNC(Pico_Exchange15693) (SPROX_PARAM  const BYTE* send_data, WORD send_bytelen, BYTE* recv_data, WORD* recv_bytelen, BYTE append_crc, WORD timeout)
{
	SWORD rc;

	rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_INSIDE_PICO_15693);
	if (rc != MI_OK) return rc;

	rc = SPROX_API_CALL(Pico_ExchangeEx) (SPROX_PARAM_P  SPROX_PICCEXCHANGEBLOCK_V, send_data, send_bytelen, recv_data, recv_bytelen, append_crc, timeout);
	return rc;
}
