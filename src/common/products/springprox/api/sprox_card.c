/**h* SpringProx.API/Smartcards
 *
 * NAME
 *   SpringProx.API :: Access to smartcard readers
 *
 * DESCRIPTION
 *   Access to SmartCard slot for CSB-5 and related products
 *   Those functions work only on readers featuring a GemPlus GemCore smartcard reader
 *
 * NOTES
 *   Most parameters of the SPROX_Card_xxx functions are directly sent to reader's internal
 *   GemCore chipset.
 *   Please refer to GemPlus GemCore documentation for details regarding those functions.
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

   JDA 22/06/2004 : created
   JDA 22/06/2006 : added re-entrant stuff

 */

#include "sprox_api_i.h"

#ifndef SPROX_API_NO_CARD

 /**f* SpringProx.API/SPROX_Card_Exchange
  *
  * NAME
  *   SPROX_Card_Exchange
  *
  * DESCRIPTION
  *   Perform a T=0 or T=1 exchange according to ISO 7816-3
  *
  * INPUTS
  *   BYTE slot                : smarcard slot number
  *   const BYTE send_buffer[] : buffer to sens to the card
  *   WORD send_len            : length of send_buffer
  *   BYTE recv_buffer[]       : buffer for card's answer
  *   WORD *recv_len           : input  : size of recv_buffer
  *                              output : actual length of reply
  *
  * RETURNS
  *   MI_OK              : success
  *   GemCore specific code if an error has occured ; please refer to
  *   relevant GemPlus documentation.
  *
  * NOTES
  *   This functions accepts only short APDUs (Lc <= 256, Le <= 254)
  *   No interpretation of the SW is performed. In particular, a 61xx status word
  *   is returned to the caller "as it".
  *
  * SEE ALSO
  *   SPROX_Card_Status
  *   SPROX_Card_PowerUp
  *
  **/
SPROX_API_FUNC(Card_Exchange) (SPROX_PARAM  BYTE slot, const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD* recv_len)
{
	BYTE buffer[400];
	buffer[0] = slot;
	buffer[1] = 0x00;
	memcpy(&buffer[2], send_buffer, send_len);

	return SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CARD_ISO_EXCHANGE, buffer, (WORD)(2 + send_len), recv_buffer, recv_len);
}

/**f* SpringProx.API/SPROX_Card_Status
 *
 * NAME
 *   SPROX_Card_Status
 *
 * DESCRIPTION
 *   Retrieve status of a smartcard reader slot, or of the smartcard itself
 *
 * INPUTS
 *   BYTE slot          : smarcard slot number
 *   BYTE *stat         : on correct execution, this byte provides the
 *                        status of the slot :
 *                          xxxxx0xx : card not inserted
 *                          xxxxx1xx : card inserted but not powered
 *                                     (note : this bit is always set on SIM/SAM
 *                                      slots - do not trust it !)
 *                          xxxxx110 : card inserted and powered (5V)
 *                          xxxxx111 : card inserted and powered (3V)
 *                          xxxx011x : T=0 protocol
 *                          xxxx111x : T=1 protocol
 *   BYTE *type         : on correct execution, this byte provides the
 *                        activated card type (see SPROX_Card_SetConfig)
 *   BYTE config[4]     : on correct execution, this buffer provides card's
 *                        communication parameters :
 *                        - for a T=0 card
 *                          config[0] : TA1 (FI & DI)
 *                          config[1] : TC1 (EGT)
 *                          config[2] : WI
 *                          config[3] : 0x00
 *                        - for a T=1 card
 *                          config[0] : TA1 (FI & DI)
 *                          config[1] : TC1 (EGT)
 *                          config[2] : IFSC
 *                          config[3] : TB3 (BWI & CWI)
 *
 * RETURNS
 *   MI_OK              : success
 *   -214               : smartcard slot is not wired
 *   -216               : smartcard slot does not exists
 *   GemCore specific code if an error has occured ; please refer to
 *   relevant GemPlus documentation.
 *
 * SEE ALSO
 *   SPROX_Card_SetConfig
 *   SPROX_Card_GetConfig
 *
 **/
SPROX_API_FUNC(Card_Status) (SPROX_PARAM  BYTE slot, BYTE* status, BYTE* type, BYTE config[4])
{
	BYTE    buffer[7];
	WORD    buflen = sizeof(buffer);
	SWORD   rc;

	buffer[0] = slot;
	buffer[1] = SPROX_CARD_FUNC_CARD_STATUS;

	rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CARD_FUNCTION, buffer, 2, buffer, &buflen);

	if (buflen == sizeof(buffer))
	{
		if (status != NULL)
			*status = buffer[0];
		if (type != NULL)
			*type = buffer[1];
		if (config != NULL)
			memcpy(config, &buffer[2], 4);
	}

	return rc;
}

/**f* SpringProx.API/SPROX_Card_PowerUp_Ex
 *
 * NAME
 *   SPROX_Card_PowerUp_Ex
 *
 * DESCRIPTION
 *   Power up a T=0 or T=1 smartcard with user-supplied options
 *
 * INPUTS
 *   BYTE slot            : smartcard slot number
 *   BYTE options         : power-up options
 *   BYTE atr[]           : buffer to retrieve card's ATR
 *   WORD *atr_len        : input  : max size of ATR
 *                          output : actual size of ATR
 *
 * RETURNS
 *   MI_OK              : success
 *   GemCore specific code if an error has occured ; please refer to
 *   relevant GemPlus documentation.
 *
 * SEE ALSO
 *   SPROX_Card_Status
 *   SPROX_Card_SetConfig
 *   SPROX_Card_PowerDown
 *
 **/
SPROX_API_FUNC(Card_PowerUp_Ex)  (SPROX_PARAM  BYTE slot, BYTE options, BYTE atr[], WORD* atr_len)
{
	BYTE    buffer[3];

	buffer[0] = slot;
	buffer[1] = SPROX_CARD_FUNC_POWER_UP;
	buffer[2] = options;

	/* Small hack for lazy callers... */
	if ((atr != NULL) && (atr_len != NULL) && (*atr_len == 0)) *atr_len = 32;

	return SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CARD_FUNCTION, buffer, 3, atr, atr_len);
}

/**f* SpringProx.API/SPROX_Card_PowerUp
 *
 * NAME
 *   SPROX_Card_PowerUp
 *
 * DESCRIPTION
 *   Power up a T=0 or T=1 smartcard with default options
 *
 * INPUTS
 *   BYTE slot          : smartcard slot number
 *   BYTE unused        : shall be set to 0x00
 *   BYTE *atr          : buffer to retrieve card's ATR
 *   WORD *atr_len      : input  : size of atr
 *                        output : actual length of atr
 *
 * RETURNS
 *   MI_OK              : success
 *   GemCore specific code if an error has occured ; please refer to
 *   relevant GemPlus documentation.
 *
 * SEE ALSO
 *   SPROX_Card_Status
 *   SPROX_Card_SetConfig
 *   SPROX_Card_PowerDown
 *
 **/
SPROX_API_FUNC(Card_PowerUp)  (SPROX_PARAM  BYTE slot, BYTE unused, BYTE atr[], WORD* atr_len)
{
	BYTE    buffer[2];
	(void)unused;

	buffer[0] = slot;
	buffer[1] = SPROX_CARD_FUNC_POWER_UP;

	/* Small hack for lazy callers... */
	if ((atr != NULL) && (atr_len != NULL) && (*atr_len == 0)) *atr_len = 32;

	return SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CARD_FUNCTION, buffer, 2, atr, atr_len);
}

/**f* SpringProx.API/SPROX_Card_PowerUp_Auto
 *
 * NAME
 *   SPROX_Card_PowerUp_Auto
 *
 * DESCRIPTION
 *   This function is deprecated and shall not be used
 *
 * SEE ALSO
 *   SPROX_Card_PowerUp
 *
 **/
SPROX_API_FUNC(Card_PowerUp_Auto)  (SPROX_PARAM  BYTE slot, BYTE atr[], WORD* atr_len)
{
	return SPROX_API_CALL(Card_PowerUp) (SPROX_PARAM_P  slot, 0, atr, atr_len);
}

/**f* SpringProx.API/SPROX_Card_SetConfig
 *
 * NAME
 *   SPROX_Card_SetConfig
 *
 * DESCRIPTION
 *   This function is deprecated and shall not be used
 *
 **/
SPROX_API_FUNC(Card_SetConfig)  (SPROX_PARAM  BYTE slot, BYTE mode, BYTE type)
{
	(void)slot;
	(void)mode;
	(void)type;
	return MI_UNKNOWN_FUNCTION;
}

/**f* SpringProx.API/SPROX_Card_GetConfig
 *
 * NAME
 *   SPROX_Card_GetConfig
 *
 * DESCRIPTION
 *   This function is deprecated and shall not be used
 *
 **/
SPROX_API_FUNC(Card_GetConfig)  (SPROX_PARAM  BYTE slot, BYTE* mode, BYTE* type)
{
	(void)slot;
	(void)mode;
	(void)type;
	return MI_UNKNOWN_FUNCTION;
}

/**f* SpringProx.API/SPROX_Card_PowerDown
 *
 * NAME
 *   SPROX_Card_PowerDown
 *
 * DESCRIPTION
 *   Power down a smarcard
 *
 * INPUTS
 *   BYTE slot          : smartcard slot number
 *
 * RETURNS
 *   MI_OK              : success
 *   GemCore specific code if an error has occured ; please refer to
 *   relevant GemPlus documentation.
 *
 * SEE ALSO
 *   SPROX_Card_Status
 *   SPROX_Card_PowerUp
 *
 **/
SPROX_API_FUNC(Card_PowerDown)  (SPROX_PARAM  BYTE slot)
{
	BYTE    buffer[2];

	buffer[0] = slot;
	buffer[1] = SPROX_CARD_FUNC_POWER_DOWN;

	return SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CARD_FUNCTION, buffer, 2, NULL, NULL);
}

/**f* SpringProx.API/SPROX_Card_GetFirmware
 *
 * NAME
 *   SPROX_Card_GetFirmware
 *
 * DESCRIPTION
 *   Retrieve the SpringProx' internal GemCore version info
 *
 * INPUTS
 *   TCHAR firmware[]   : buffer to receive the firmware info
 *   WORD  len          : character-size of the buffer
 *
 * RETURNS
 *   MI_OK              : success
 *   GemCore specific code if an error has occured ; please refer to
 *   relevant GemPlus documentation.
 *
 **/
SPROX_API_FUNC(Card_GetFirmware)  (SPROX_PARAM  TCHAR firmware[], WORD len)
{
	BYTE    buffer[32];
	WORD    sz = sizeof(buffer);
	WORD    i;
	SWORD   rc;

	buffer[0] = 0xFF; /* New 1.70: we don't care for the slot, but it is cleaner to set a value anyway */
	buffer[1] = SPROX_CARD_FUNC_GET_FIRMWARE;

	rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CARD_FUNCTION, buffer, 2, buffer, &sz);
	if (!rc)
	{
		if (firmware != NULL)
		{
			for (i = 0; i < sz; i++)
			{
				if (i >= len) break;
				firmware[i] = buffer[i];
			}
			if (i >= len) i--;
			firmware[i] = '\0';
		}
	}

	return rc;
}

#ifdef WIN32
SPROX_API_FUNC(Card_GetFirmwareW)  (SPROX_PARAM  wchar_t* firmware, WORD len)
{
#ifdef UNICODE
	return SPROX_API_CALL(Card_GetFirmware) (SPROX_PARAM_P  firmware, len);
#else
	char buffer[32];
	int i;
	SWORD rc;
	rc = SPROX_API_CALL(Card_GetFirmware) (SPROX_PARAM_P  buffer, len);
	if (rc != MI_OK) return rc;
	if (len > 32) len = 32;
	for (i = 0; i < 32; i++)
	{
		firmware[i] = buffer[i];
		if (firmware[i] == '\0') break;
	}
	firmware[len - 1] = '\0';
	return MI_OK;
#endif
}
#ifndef UNDER_CE
SPROX_API_FUNC(Card_GetFirmwareA)  (SPROX_PARAM  char* firmware, WORD len)
{
#ifndef UNICODE
	return SPROX_API_CALL(Card_GetFirmware) (SPROX_PARAM_P  firmware, len);
#else
	wchar_t buffer[32];
	int i;
	SWORD rc;
	rc = SPROX_API_CALL(Card_GetFirmware) (SPROX_PARAM_P  buffer, len);
	if (rc != MI_OK) return rc;
	if (len > 32) len = 32;
	for (i = 0; i < 32; i++)
	{
		firmware[i] = (char)buffer[i];
		if (firmware[i] == '\0') break;
	}
	firmware[len - 1] = '\0';
	return MI_OK;
#endif
}
#endif
#endif

SPROX_API_FUNC(Card_Control)  (SPROX_PARAM  const BYTE* send_buffer, WORD send_len, BYTE* recv_buffer, WORD* recv_len)
{
	return SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CARD_DIRECT, send_buffer, send_len, recv_buffer, recv_len);
}


SPROX_API_FUNC(Bi_SamFastSpeed)  (SPROX_PARAM  BYTE card_slot)
{
	BYTE    buffer[3];
	buffer[0] = SPROX_CONTROL_CALYPSO;
	buffer[1] = card_slot;
	buffer[2] = 0x04;
	return SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CONTROL, buffer, sizeof(buffer), NULL, NULL);
}


#endif
