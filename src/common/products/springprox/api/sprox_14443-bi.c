/**h* SpringProx.API/Innovatron
 *
 * NAME
 *   SpringProx.API :: Innovatron radio protocol
 *
 * DESCRIPTION
 *   Implementation of Innovatron radio protocol (Calypso card)
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

   JDA 19/02/2008 : created

 */
#include "sprox_api_i.h"

#include "micore_picc.h"

 /**f* SpringProx.API/SPROX_Bi_Apgen
  *
  * NAME
  *   SPROX_Bi_Apgen
  *
  * DESCRIPTION
  *   Configure the reader for B' operation and perform APGEN
  *
  * NOTES
  *   This function is only avalaible for Calypso-enabled readers
  *   (-C suffix in the part number)
  *
  *   Please refer to Calypso documentation for details regarding
  *   the command itself.
  *
  * INPUTS
  *   BYTE uid[4]        : 4-byte Unique ID of the card
  *   BYTE atr[32]       : ATR returned by the card (if some)
  *   BYTE *atrlen       : on input, size of ATR
  *                        on output, actual length of ATR
  *
  * RETURNS
  *   MI_OK              : success, card selected
  *   MI_NOTAGERR        : no card available in the RF field
  *   Other code if internal or communication error has occured.
  *
  * SEE ALSO
  *   SPROX_Bi_Attrib
  *
  **/
SPROX_API_FUNC(Bi_Apgen) (SPROX_PARAM  BYTE uid[4], BYTE atr[], BYTE* atrlen)
{
	BYTE    buffer[64];
	WORD    rlen;
	SWORD   rc;

	SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_14443_Bi);

	rlen = sizeof(buffer);
	rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_SELECT_ANY, NULL, 0, buffer, &rlen);
	if (rc != MI_OK)
		return rc;

	if (uid != NULL)
		memcpy(uid, &buffer[0], 4);

	rlen -= 6;

	if (atrlen != NULL)
	{
		if ((*atrlen != 0) && (*atrlen < rlen))
			return MI_RESPONSE_OVERFLOW;
		*atrlen = (BYTE)rlen;
	}
	if (atr != NULL)
		memcpy(atr, &buffer[6], rlen);

	return rc;
}

/**f* SpringProx.API/SPROX_Bi_Attrib
 *
 * NAME
 *   SPROX_Bi_Attrib
 *
 * DESCRIPTION
 *   Perform type B' ATTRIB
 *
 * NOTES
 *   This function is only avalaible for Calypso-enabled readers
 *   (-C suffix in the part number)
 *
 *   Please refer to Calypso documentation for details regarding
 *   the command itself.
 *
 * INPUTS
 *   BYTE uid[4]        : 4-byte Unique ID of the card as returned
 *                        by SPROX_Bi_Apgen
 *
 * RETURNS
 *   MI_OK              : success, card activated
 *   Other code if internal or communication error has occured.
 *
 * SEE ALSO
 *   SPROX_Bi_Apgen
 *   SPROX_Bi_Exchange
 *
 **/
SPROX_API_FUNC(Bi_Attrib) (SPROX_PARAM  BYTE uid[4])
{
	BYTE    buffer[32];
	WORD    slen, rlen;
	SWORD   rc;

	SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_14443_Bi);

	buffer[0] = SPROX_TCL_FUNC_ATTRIB;
	buffer[1] = 0xFC;
	slen = 2;

	if (uid != NULL)
	{
		memcpy(&buffer[2], uid, 4);
		slen += 4;
	}

	rlen = sizeof(buffer);
	rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_TCL_FUNC, buffer, slen, buffer, &rlen);

	return rc;
}

/**f* SpringProx.API/SPROX_Bi_Exchange
 *
 * NAME
 *   SPROX_Bi_Exchange
 *
 * DESCRIPTION
 *   Perform type B' COM_R/RR exchange
 *
 * NOTES
 *   This function is only avalaible for Calypso-enabled readers
 *   (-C suffix in the part number)
 *
 *   Please refer to Calypso documentation for details regarding
 *   the command itself.
 *
 * INPUTS
 *   const BYTE send_buffer[] : buffer to send to the card
 *   WORD send_len            : length of send_buffer (max 256)
 *   BYTE recv_buffer[]       : buffer for card's answer
 *   WORD *recv_len           : input  : size of recv_buffer
 *                              output : actual length of reply
 *
 * RETURNS
 *   MI_OK                    : success
 *   Other code if internal or communication error has occured.
 *
 * SEE ALSO
 *   SPROX_Bi_Attrib
 *
 **/
SPROX_API_FUNC(Bi_Exchange) (SPROX_PARAM  const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD* recv_len)
{
	SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_14443_Bi);

	return SPROX_API_CALL(Tcl_Exchange) (SPROX_PARAM_P  0xFC, send_buffer, send_len, recv_buffer, recv_len);
}

/**f* SpringProx.API/SPROX_Bi_Disc
 *
 * NAME
 *   SPROX_Bi_Disc
 *
 * DESCRIPTION
 *   Perform type B' DISC
 *
 * NOTES
 *   This function is only avalaible for Calypso-enabled readers
 *   (-C suffix in the part number)
 *
 *   Please refer to Calypso documentation for details regarding
 *   the command itself.
 *
 * INPUTS
 *   None
 *
 * RETURNS
 *   MI_OK              : success, card de-activated
 *   Other code if internal or communication error has occured.
 *
 * SEE ALSO
 *   SPROX_Bi_Apgen
 *   SPROX_Bi_Attrib
 *
 **/
SPROX_API_FUNC(Bi_Disc) (SPROX_PARAM_V)
{
	SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_14443_Bi);

	return SPROX_API_CALL(Tcl_Deselect) (SPROX_PARAM_P  0xFC);
}

