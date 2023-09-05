/**** SpringProx.API/ReaderUtilities
 *
 * NAME
 *   SpringProx.API :: Reader utilities
 *
 * DESCRIPTION
 *   Mixed reader related utilities
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

   JDA 15/02/2003 : created from philips rc500/mv700 files
   JDA 16/05/2003 : added support for the CSB-3 hardware
   JDA 30/09/2003 : added LED control functions
   JDA 02/12/2005 : pseudo re-entrant part moved to sprox_fct_ex.c
   JDA 22/06/2006 : moved really low level function to sprox_rc500.c
					added really re-entrant stuff
   JDA 15/02/2011 : added SPROX_ReadStorage, SPROX_WriteStorage and SPROX_StorageSize
   JDA 04/11/2011 : UserIO is deprecated, replaced by ModeIO
   JDA 21/11/2011 : added SPROX_ReaderRestart
   JDA 30/01/2012 : forget pcd_current_rf_protocol after ControlRF

 */
#include "sprox_api_i.h"

 /**f* SpringProx.API/SPROX_ReaderGetRc500Id
  *
  * NAME
  *   SPROX_ReaderGetRc500Id
  *
  * DESCRIPTION
  *   Retrieve the SpringProx reader chipset type and serial number
  *
  * INPUTS
  *   BYTE micore_type[5] : 5-byte buffer to receive the chipset type
  *   BYTE micore_snr[4]  : 4-byte buffer to receive the chipset serial number
  *
  * RETURNS
  *   MI_OK              : success
  *   Other code if internal or communication error has occured.
  *
  * NOTES
  *   Please refer to Philips MfRC5xx official documentation for explanation of the
  *   5-byte chipset type identifier.
  *
  **/
SPROX_API_FUNC(ReaderGetRc500Id) (SPROX_PARAM  BYTE micore_type[5], BYTE micore_snr[4])
{
	SPROX_PARAM_TO_CTX;

	if ((micore_type == NULL) && (micore_snr == NULL))
		return MI_LIB_CALL_ERROR;

	if (micore_type != NULL)
		memcpy(micore_type, &sprox_ctx->sprox_info.pid, 5);
	if (micore_snr != NULL)
		memcpy(micore_snr, &sprox_ctx->sprox_info.nid, 4);

	return MI_OK;
}

/**f* SpringProx.API/SPROX_ReaderReset
 *
 * NAME
 *   SPROX_ReaderReset
 *
 * DESCRIPTION
 *   Reset the device
 *
 * INPUTS
 *   none
 *
 * RETURNS
 *   MI_OK              : success
 *
 **/
SPROX_API_FUNC(ReaderReset)  (SPROX_PARAM_V)
{
	SWORD   rc;
	BYTE    buffer[4];

	// TODO  if (sprox_ctx->com_is_usb_cdc)
	//  {
		/* We CAN'T reset an USB CDC-ACM reader, has it is the comm port itself !!! */
	//    return MI_UNKNOWN_FUNCTION;
	//  }

	buffer[0] = 0xde;
	buffer[1] = 0xad;
	buffer[2] = 0xde;
	buffer[3] = 0xad;

	rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_PCDRESET, buffer, 4, NULL, NULL);
	switch (rc)
	{
	case MI_SER_LENGTH_ERR:
	case MI_SER_CHECKSUM_ERR:
	case MI_SER_PROTO_ERR:
	case MI_SER_PROTO_NAK:
	case MI_SER_ACCESS_ERR:
	case MI_SER_TIMEOUT_ERR:
	case MI_SER_NORESP_ERR:    /* Reader is resetting, a communication error is not an error ! */
		rc = MI_OK;
		break;
	}

	return rc;
}

/* JDA 21/11/2011 */
SPROX_API_FUNC(ReaderRestart)  (SPROX_PARAM_V)
{
	SWORD   rc;
	rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_PCDRESET, NULL, 0, NULL, NULL);
	return rc;
}

/**f* SpringProx.API/SPROX_ControlLed
 *
 * NAME
 *   SPROX_ControlLed
 *
 * DESCRIPTION
 *   Manages reader's LEDs
 *
 * INPUTS
 *   BYTE led_r         : value for the red LED
 *   BYTE led_g         : value for the green LED
 *
 * NOTES
 *   Allowed values for led_r and led_g are :
 *   0                  : LED is OFF
 *   1                  : LED is ON
 *   2                  : LED is BLINKING
 *   3                  : LED is managed by the reader
 *
 *   If led_r is set to 3, the red LED blinks slowly
 *   If led_g is set to 3, the green LED reflects reader activity
 *
 * RETURNS
 *   MI_OK              : success
 *   Other code if internal or communication error has occured.
 *
 * SEE ALSO
 *  SPROX_ControlLedY
 *
 **/
SPROX_API_FUNC(ControlLed) (SPROX_PARAM  BYTE led_r, BYTE led_g)
{
	BYTE    buffer[3];
	buffer[0] = SPROX_CONTROL_LEDS;
	buffer[1] = led_r;
	buffer[2] = led_g;
	return SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CONTROL, buffer, sizeof(buffer), NULL, NULL);
}

SPROX_API_FUNC(ControlLeds) (SPROX_PARAM  WORD leds)
{
	BYTE    buffer[5];
	buffer[0] = SPROX_CONTROL_LEDS;
	buffer[1] = (BYTE)(leds & 0x000F);
	buffer[2] = (BYTE)((leds >> 4) & 0x000F);
	buffer[3] = (BYTE)((leds >> 8) & 0x000F);
	buffer[4] = (BYTE)((leds >> 12) & 0x000F);
	return SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CONTROL, buffer, sizeof(buffer), NULL, NULL);
}

SPROX_API_FUNC(ControlLedsT) (SPROX_PARAM  WORD leds, WORD ms)
{
	BYTE    buffer[7];
	buffer[0] = SPROX_CONTROL_LEDS;
	buffer[1] = (BYTE)(leds & 0x000F);
	buffer[2] = (BYTE)((leds >> 4) & 0x000F);
	buffer[3] = (BYTE)((leds >> 8) & 0x000F);
	buffer[4] = (BYTE)((leds >> 12) & 0x000F);
	buffer[5] = (BYTE)((ms >> 8) & 0x00FF);
	buffer[6] = (BYTE)(ms & 0x00FF);
	return SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CONTROL, buffer, sizeof(buffer), NULL, NULL);
}

/**f* SpringProx.API/SPROX_ControlLedY
 *
 * NAME
 *   SPROX_ControlLedY
 *
 * DESCRIPTION
 *   Manages reader's LEDs
 *
 * INPUTS
 *   BYTE led_r         : value for the red LED
 *   BYTE led_g         : value for the green LED
 *   BYTE led_y         : value for the yellow LED
 *
 * NOTES
 *   Allowed values for led_r and led_g are :
 *   0                  : LED is OFF
 *   1                  : LED is ON
 *   2                  : LED is BLINKING
 *   3                  : LED is managed by the reader
 *
 *   If led_r is set to 3, the red LED blinks slowly
 *   If led_g is set to 3, the green LED reflects reader activity
 *   If led_y is set to 3, the yellow LED reflects antenna status
 *
 * RETURNS
 *   MI_OK              : success
 *   Other code if internal or communication error has occured.
 *
 * SEE ALSO
 *  SPROX_ControlLed
 *
 **/
SPROX_API_FUNC(ControlLedY) (SPROX_PARAM  BYTE led_r, BYTE led_g, BYTE led_y)
{
	BYTE    buffer[4];
	buffer[0] = SPROX_CONTROL_LEDS;
	buffer[1] = led_r;
	buffer[2] = led_g;
	buffer[3] = led_y;
	return SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CONTROL, buffer, sizeof(buffer), NULL, NULL);
}

/**f* SpringProx.API/SPROX_ControlBuzzer
 *
 * NAME
 *   SPROX_ControlBuzzer
 *
 * DESCRIPTION
 *   Manages reader's LEDs
 *
 * INPUTS
 *   WORD time_ms       : buzzer duration in ms
 *
 * RETURNS
 *   MI_OK              : success
 *   Other code if internal or communication error has occured.
 *
 **/
SPROX_API_FUNC(ControlBuzzer) (SPROX_PARAM  WORD time_ms)
{
	BYTE    buffer[3];
	buffer[0] = SPROX_CONTROL_BUZZER;
	buffer[1] = (BYTE)(time_ms / 0x0100);
	buffer[2] = (BYTE)(time_ms % 0x0100);
	return SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CONTROL, buffer, sizeof(buffer), NULL, NULL);
}

/**f* SpringProx.API/SPROX_ControlReadModeIO
 *
 * NAME
 *   SPROX_ControlReadModeIO
 *
 * DESCRIPTION
 *   Read value of reader's MODE pin
 *
 * INPUTS
 *   BOOL *in_value     : returns the current level of the input pin.
 *
 * RETURNS
 *   MI_OK              : success
 *   Other code if internal or communication error has occured.
 *
 * SEE ALSO
 *   SPROX_ControlReadUserIO
 *
 **/
SPROX_API_FUNC(ControlReadModeIO) (SPROX_PARAM  BOOL* in_value)
{
	BYTE    buffer[1];
	WORD    l = 1;
	if (in_value == NULL)
		return MI_LIB_CALL_ERROR;
	buffer[0] = SPROX_CONTROL_MODE_I;
	return SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CONTROL, buffer, sizeof(buffer), (BYTE*)in_value, &l);
}

/**f* SpringProx.API/SPROX_ControlReadUserIO
 *
 * NAME
 *   SPROX_ControlReadUserIO
 *
 * DESCRIPTION
 *   Configure the USER pin as input and read its value
 *
 * INPUTS
 *   BOOL *in_value     : returns the current level of the USER pin.
 *
 * RETURNS
 *   MI_OK              : success
 *   Other code if internal or communication error has occured.
 *
 * SEE ALSO
 *   SPROX_ControlWriteUserIO
 *   SPROX_ControlReadModeIO
 *
 **/
SPROX_API_FUNC(ControlReadUserIO) (SPROX_PARAM  BOOL* in_value)
{
	BYTE    buffer[1];
	WORD    l = 1;
	if (in_value == NULL)
		return MI_LIB_CALL_ERROR;
	buffer[0] = SPROX_CONTROL_GPIOS;
	return SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CONTROL, buffer, sizeof(buffer), (BYTE*)in_value, &l);
}

/**f* SpringProx.API/SPROX_ControlWriteUserIO
 *
 * NAME
 *   SPROX_ControlWriteUserIO
 *
 * DESCRIPTION
 *   Configure the USER pin as output and write its value
 *
 * WARNING
 *   Do not call this function is the USER pin is tied to an external
 *   hardware that assume it is an input. This may damage the product.
 *
 * INPUTS
 *   BOOL outvalue      : level to be assigned to the USER pin.
 *
 * RETURNS
 *   MI_OK              : success
 *   Other code if internal or communication error has occured.
 *
 * SEE ALSO
 *   SPROX_ControlReadUserIO
 *
 **/
SPROX_API_FUNC(ControlWriteUserIO) (SPROX_PARAM  BOOL out_value)
{
	BYTE    buffer[2];
	buffer[0] = SPROX_CONTROL_GPIOS;
	buffer[1] = out_value;
	return SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CONTROL, buffer, sizeof(buffer), NULL, NULL);
}

/**f* SpringProx.API/SPROX_ControlRF
 *
 * NAME
 *   SPROX_ControlRF
 *
 * DESCRIPTION
 *   Turn ON or OFF reader's RF field
 *
 * INPUTS
 *   BOOL mode          : TRUE starts the RF field, FALSE shuts it down
 *
 * RETURNS
 *   MI_OK              : success
 *   Other code if internal or communication error has occured.
 *
 * NOTES
 *   Shutting down the RF field means that every tags are stopped.
 *   When the RF field will be turned on again, they all will be in
 *   the IDLE state.
 *
 **/
SPROX_API_FUNC(ControlRF) (SPROX_PARAM  BYTE mode)
{
	SWORD   rc;
	BYTE    buffer[2];
	SPROX_PARAM_TO_CTX;

	/* Forget current mode so that later SetConfig will actually reconfigure the reader */
	sprox_ctx->pcd_current_rf_protocol = 0;

	buffer[0] = SPROX_CONTROL_RF;
	buffer[1] = mode;
	rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CONTROL, buffer, sizeof(buffer), NULL, NULL);
	return rc;
}

/**f* SpringProx.API/SPROX_SetConfig
 *
 * NAME
 *   SPROX_SetConfig
 *
 * DESCRIPTION
 *   Configure the reader in any available ISO mode
 *
 * INPUTS
 *   BYTE mode          : must be either
 *                        - CFG_MODE_ISO_14443_A       : ISO/IEC 14443 type A
 *                        - CFG_MODE_ISO_14443_B       : ISO/IEC 14443 type B
 *                        - CFG_MODE_ISO_14443_Bi      : Innovatron (Legacy Calypso cards)
 *                        - CFG_MODE_ISO_15693         : ISO/IEC 15693
 *                        - CFG_MODE_ICODE1            : NXP ICODE1
 *                        - CFG_MODE_INSIDE_PICO_14443 : Inside PicoTag (also HID iClass)
 *                        - CFG_MODE_INSIDE_PICO_15693 : Inside PicoTag (also HID iClass)
 *
 * RETURNS
 *   MI_OK              : success
 *   Other code if internal or communication error has occured.
 *
 **/
SPROX_API_FUNC(SetConfig) (SPROX_PARAM  BYTE mode)
{
	BYTE    buffer[2];
	SWORD   rc;
	SPROX_PARAM_TO_CTX;

	if (sprox_ctx->pcd_current_rf_protocol == mode) return MI_OK;

	buffer[0] = SPROX_CONTROL_MODE;
	buffer[1] = mode;

	rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CONTROL, buffer, sizeof(buffer), NULL, NULL);

	if (rc == MI_OK) sprox_ctx->pcd_current_rf_protocol = mode;

	return rc;
}

SPROX_API_FUNC(Debug_SetApiConfig) (SPROX_PARAM  BYTE mode)
{
	SPROX_PARAM_TO_CTX;
	sprox_ctx->pcd_current_rf_protocol = mode;
	return MI_OK;
}

SPROX_API_FUNC(ReadStorage) (SPROX_PARAM  DWORD address, BYTE buffer[], WORD length)
{
	BYTE cb[5];
	WORD rl = length;
	SWORD  rc;
	SPROX_PARAM_TO_CTX;

	if ((length == 0) || (length > 256) || (buffer == NULL))
		return MI_WRONG_PARAMETER;

	cb[3] = (BYTE)address; address >>= 8;
	cb[2] = (BYTE)address; address >>= 8;
	cb[1] = (BYTE)address; address >>= 8;
	cb[0] = (BYTE)address; address >>= 8;

	if (length == 256)
		cb[4] = 0;
	else
		cb[4] = (BYTE)length;

	rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_STORAGE, cb, 5, buffer, &rl);
	if ((rc == MI_OK) && (rl != length))
		rc = MI_RESPONSE_INVALID;

	return rc;
}

SPROX_API_FUNC(WriteStorage) (SPROX_PARAM  DWORD address, const BYTE buffer[], WORD length)
{
	BYTE cb[256 + 5];
	SWORD  rc;
	SPROX_PARAM_TO_CTX;

	if ((length == 0) || (length > 256) || (buffer == NULL))
		return MI_WRONG_PARAMETER;

	cb[3] = (BYTE)address; address >>= 8;
	cb[2] = (BYTE)address; address >>= 8;
	cb[1] = (BYTE)address; address >>= 8;
	cb[0] = (BYTE)address; address >>= 8;

	if (length == 256)
		cb[4] = 0;
	else
		cb[4] = (BYTE)length;

	memcpy(&cb[5], buffer, length);

	rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_STORAGE, cb, (WORD)(5 + length), NULL, NULL);

	return rc;
}

SPROX_API_FUNC(StorageSize) (SPROX_PARAM  DWORD* size)
{
	BYTE b[4];
	WORD rl = sizeof(b);
	SWORD  rc;
	SPROX_PARAM_TO_CTX;

	rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_STORAGE, NULL, 0, b, &rl);
	if ((rc == MI_OK) && (rl != sizeof(b)))
		rc = MI_RESPONSE_INVALID;

	if (size != NULL)
	{
		*size = (b[0] << 24)
			| (b[1] << 16)
			| (b[2] << 8)
			| b[3];
	}

	return rc;
}



SPROX_API_FUNC(ReaderGetConsts) (SPROX_PARAM  DWORD* consts)
{
	SWORD   rc;
	BYTE    buffer[4];
	WORD    l = 4;

	buffer[0] = SPROX_CONTROL_CONST;
	rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CONTROL, buffer, 1, buffer, &l);
	if ((rc == MI_OK) && (consts != NULL))
	{
		DWORD   value;

		value = buffer[0];
		value *= 0x00000100;
		value |= buffer[1];
		value *= 0x00000100;
		value |= buffer[2];
		value *= 0x00000100;
		value |= buffer[3];

		*consts = value;
	}
	return rc;
}

SPROX_API_FUNC(ReaderSetConsts) (SPROX_PARAM  DWORD consts)
{
	SWORD   rc;
	BYTE    buffer[5];

	buffer[0] = SPROX_CONTROL_CONST;
	buffer[4] = (BYTE)(consts % 0x00000100);
	consts /= 0x00000100;
	buffer[3] = (BYTE)(consts % 0x00000100);
	consts /= 0x00000100;
	buffer[2] = (BYTE)(consts % 0x00000100);
	consts /= 0x00000100;
	buffer[1] = (BYTE)(consts % 0x00000100);

	rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CONTROL, buffer, 5, NULL, NULL);
	return rc;
}

SPROX_API_FUNC(ReaderGetConstsEx) (SPROX_PARAM  BYTE ident, BYTE consts[], WORD* length)
{
	BYTE    buffer[2];

	buffer[0] = SPROX_CONTROL_CONFIG_READ;
	buffer[1] = ident;

	return SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CONTROL, buffer, 2, consts, length);
}

SPROX_API_FUNC(ReaderSetConstsEx) (SPROX_PARAM  BYTE ident, const BYTE consts[], WORD length)
{
	SWORD   rc;
	BYTE    buffer[2 + 128];
	SPROX_PARAM_TO_CTX;

	if (length > 128)
		return MI_LIB_CALL_ERROR;
	if ((consts == NULL) && (length != 0))
		return MI_LIB_CALL_ERROR;

	if (sprox_ctx->sprox_version < 0x00014500)
		buffer[0] = SPROX_CONTROL_CONFIG_READ;
	else
		buffer[0] = SPROX_CONTROL_CONFIG_WRITE;
	buffer[1] = ident;

	memcpy(&buffer[2], consts, length);

	rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CONTROL, buffer, (WORD)(2 + length), NULL, NULL);
	return rc;
}

SPROX_API_FUNC(ControlEx) (SPROX_PARAM  const BYTE* send, WORD sendlen, BYTE* recv, WORD* recvlen)
{
	SWORD   rc;
	rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CONTROL, send, sendlen, recv, recvlen);
	return rc;
}

SPROX_API_FUNC(Echo) (SPROX_PARAM  WORD len)
{
	WORD    buflen;
	BYTE    send_buffer[SPROX_FRAME_CONTENT_SIZE];
	BYTE    recv_buffer[SPROX_FRAME_CONTENT_SIZE];
	WORD    i;
	SWORD   rc;


	memset(send_buffer, len, sizeof(send_buffer));

	for (i = 0; i < len; i++)
		send_buffer[i] ^= (BYTE)(i + 1);

	memset(recv_buffer, 0xCA, sizeof(recv_buffer));

	buflen = len;
	rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_ECHO, send_buffer, len, recv_buffer, &buflen);

	if (rc == MI_OK)
	{
		if (buflen != len)
		{
			//		  printf("%d != %d\n", buflen, len);
			rc = MI_RESPONSE_INVALID;
		}
		else
			for (i = 0; i < len; i++)
			{
				if (recv_buffer[i] != send_buffer[i])
				{
					//			  printf("Pos %d/%d : %02X!=%02X\n", i, len, recv_buffer[i], send_buffer[i]);
					rc = MI_RESPONSE_INVALID;
					break;
				}
			}
	}


	if (rc == MI_RESPONSE_INVALID)
	{
		printf("<");
		for (i = 0; i < len; i++)
			printf("%02X", send_buffer[i]);
		printf("\n");

		printf(">");
		for (i = 0; i < len; i++)
			printf("%02X", recv_buffer[i]);
		printf("\n");
	}

	return rc;
}

/**f* SpringProx.API/SPROX_ReaderGetFirmware
 *
 * NAME
 *   SPROX_ReaderGetFirmware
 *
 * DESCRIPTION
 *   Retrieve the SpringProx reader version info
 *
 * INPUTS
 *   TCHAR firmware[]   : buffer to receive the firmware info
 *   WORD  len          : character-size of the buffer
 *
 * RETURNS
 *   MI_OK              : success
 *   Other code if internal or communication error has occured.
 *
 * NOTES
 *   The reader returns
 *   - "SPRINGCARD SPRINGPROX-CF <level> <version>" for the SpringProx-CF family
 *   - "SPRINGCARD CSB-IV <level> <version>" for the CryptoSignBox family
 *   - "SPRINGCARD K531 <level> <version>" for the the MOD-K531 OEM module
 *
 *   <level> shows firmware capabilities :
 *   - 1        basic firmware
 *
 *   <version> is the version field, formatted "M.mm" as follow :
 *   - M        is the major release number (1 to 2 digits)
 *   - m        is the minor release number (1 to 3 digits)
 *
 **/
SPROX_API_FUNC(ReaderGetFirmware) (SPROX_PARAM  TCHAR firmware[], WORD len)
{
	char    buffer[64];
	char    temp[20];
	int     i;
	WORD    l;
	SWORD   rc;
	SPROX_PARAM_TO_CTX;

	l = sizeof(sprox_ctx->sprox_info);
	rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_GET_INFOS, NULL, 0, (BYTE*)&sprox_ctx->sprox_info, &l);

	if (rc == MI_UNKNOWN_FUNCTION)
	{
		l = 0;
		rc = MI_OK;
	}
	if (rc != MI_OK)
		return rc;

	if (rc == MI_OK)
	{
		if (l == 0)
		{
			/* CSB 3 or prior */
			sprox_ctx->sprox_version = 0x00000000;
			memset(&sprox_ctx->sprox_info, 0x00, sizeof(sprox_ctx->sprox_info));
			SPROX_Trace(TRACE_ALL, "Sprox version : old");
		}
		else
		{
			/* CSB 4 or SpringProx or... */
			sprox_ctx->sprox_version = sprox_ctx->sprox_info.ver[0];  /* Major */
			sprox_ctx->sprox_version *= 0x00000100;
			sprox_ctx->sprox_version += sprox_ctx->sprox_info.ver[1]; /* Minor */
			sprox_ctx->sprox_version *= 0x00000100;
			sprox_ctx->sprox_version += sprox_ctx->sprox_info.ver[2];
			SPROX_Trace(TRACE_ALL, "Sprox version : %08lX", sprox_ctx->sprox_version);
		}
	}

	if ((firmware == NULL) || (len == 0))
		return rc;

	if (!sprox_ctx->sprox_version)
	{
		snprintf(buffer, sizeof(buffer), "SPRINGCARD CSB3");
	}
	else
	{
		snprintf(buffer, sizeof(buffer),
			"SPRINGCARD %c%c%c%c", sprox_ctx->sprox_info.prd[0], sprox_ctx->sprox_info.prd[1], sprox_ctx->sprox_info.prd[2], sprox_ctx->sprox_info.prd[3]);
		snprintf(temp, sizeof(temp),
			" %X.%02X [%d]", sprox_ctx->sprox_info.ver[0], sprox_ctx->sprox_info.ver[1], sprox_ctx->sprox_info.ver[2]);
		strlcat(buffer, temp, sizeof(buffer));
	}
	l = (WORD)strlen(buffer);

	if (l < len) l = len - 1;
	for (i = 0; i <= l; i++)
		firmware[i] = buffer[i];
	firmware[len - 1] = '\0';

	return MI_OK;
}

#ifdef WIN32
SPROX_API_FUNC(ReaderGetFirmwareW) (SPROX_PARAM  wchar_t* firmware, WORD len)
{
#ifdef UNICODE
	return SPROX_API_CALL(ReaderGetFirmware) (SPROX_PARAM_P  firmware, len);
#else
	char buffer[64];
	int i;
	SWORD rc;
	if (len > 64) len = 64;
	rc = SPROX_API_CALL(ReaderGetFirmwareA) (SPROX_PARAM_P  buffer, len);
	if (rc != MI_OK) return rc;
	for (i = 0; i < len; i++) firmware[i] = buffer[i];
	return MI_OK;
#endif
}
#ifndef UNDER_CE
SPROX_API_FUNC(ReaderGetFirmwareA) (SPROX_PARAM  char* firmware, WORD len)
{
#ifndef UNICODE
	return SPROX_API_CALL(ReaderGetFirmware) (SPROX_PARAM_P  firmware, len);
#else
	wchar_t buffer[64];
	int i;
	SWORD rc;
	if (len > 64) len = 64;
	rc = SPROX_API_CALL(ReaderGetFirmwareW) (SPROX_PARAM_P  buffer, len);
	if (rc != MI_OK) return rc;
	for (i = 0; i < len; i++) firmware[i] = (char)buffer[i];
	return MI_OK;
#endif
}
#endif
#endif

/**f* SpringProx.API/SPROX_ReaderGetFeatures
 *
 * NAME
 *   SPROX_ReaderGetFeatures
 *
 * DESCRIPTION
 *   Used to retrieve the features supported by the selected device
 *
 * INPUTS
 *   DWORD  *features   : feature set return buffer
 *
 * RETURNS
 *   MI_OK              : success
 *
 * NOTES
 *   On a successfull return, the feature set tells which functions
 *   are actually implemented inside the reader.
 *
 *   b31  b24 b23  b16 b15   b8 b7    b0
 *   ........ ........ ........ ........
 *                                     +-- ISO 14443 stack
 *                                    +--- ISO 15698 stack
 *                                  ++---- RFU
 *                                 +------ GemCore based smartcard reader
 *                                +------- Sagem MSO CMB fingerprint reader
 *                              ++-------- RFU
 *
 *                            +----------- ASCII communication protocol
 *                           +------------ Fast Binary communication protocol
 *                          +------------- RFU
 *                         +-------------- Adressing enabled (for bus operation)
 *                        +--------------- Physical link is RS485, not RS232/422
 *                       +---------------- RFU
 *                      +----------------- Reader supports 115200bps
 *                     +------------------ Reader can receive frames up to 1024 bytes
 *
 *            ++++++++-------------------- RFU
 *
 *          +----------------------------- Reader has an internal command processor ("console")
 *    ++++++------------------------------ RFU
 *   +------------------------------------ Reader is not compliant to the SpringProx library
 *                                         (the reader may implement proprietary functions only)
 *
 *   (See products datasheet for a detailed explanation of the feature set)
 *
 **/
SPROX_API_FUNC(ReaderGetFeatures) (SPROX_PARAM  DWORD* features)
{
	BYTE    buffer[4];
	WORD    len;
	SWORD   rc;
	SPROX_PARAM_TO_CTX;

	sprox_ctx->sprox_capabilities = 0x00000000;

	/* Retrieve capabilities */
	len = sizeof(buffer);
	rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_GET_CAPABILITIES, NULL, 0, buffer, &len);
	if ((rc == MI_OK) && (len == sizeof(buffer)))
	{
		sprox_ctx->sprox_capabilities |= buffer[0];
		sprox_ctx->sprox_capabilities *= 0x00000100;
		sprox_ctx->sprox_capabilities |= buffer[1];
		sprox_ctx->sprox_capabilities *= 0x00000100;
		sprox_ctx->sprox_capabilities |= buffer[2];
		sprox_ctx->sprox_capabilities *= 0x00000100;
		sprox_ctx->sprox_capabilities |= buffer[3];
	}
	else if (rc == MI_UNKNOWN_FUNCTION)
	{
		rc = MI_OK;
	}

	SPROX_Trace(TRACE_ALL, "Sprox features : %08lX", sprox_ctx->sprox_capabilities);

	if (features != NULL)
		*features = sprox_ctx->sprox_capabilities;

	return rc;
}

/**f* SpringProx.API/SPROX_ReaderGetDeviceSettings
 *
 * NAME
 *   SPROX_ReaderGetDeviceSettings
 *
 * DESCRIPTION
 *   Retrieve current reader's operating mode
 *
 * INPUTS
 *   DWORD  *features   : feature set return buffer
 *
 * RETURNS
 *   MI_OK              : success
 *
 * NOTES
 *   On a successfull return, the current status is retrieved as follow :
 *
 *   b31  b24 b23  b16 b15   b8 b7    b0
 *   ........ ........ ........ ........
 *                                    ++-- Current protocol
 *                                         00 = OSI
 *                                         01 = ASCII
 *                                         10 = Fast Binary
 *                                         11 = Binary bus
 *                                   +---- Hardware control enabled
 *                               ++------- Channel
 *                                         00 = RS232/RS422
 *                                         01 = RS485
 *                                         10 = USB
 *                                         11 = RFU
 *                              +--------- Baudrate
 *                                         0 = 38400bps
 *                                         1 = 115200bps
 *
 *
 *   ++++++++-++++++++-++++++++----------- RFU
 *
 **/
SPROX_API_FUNC(ReaderGetDeviceSettings) (SPROX_PARAM  DWORD* settings)
{
	DWORD value = 0;
	SPROX_PARAM_TO_CTX;

	switch (sprox_ctx->com_settings & COM_PROTO_MASK)
	{
	case COM_PROTO_BUS:
		value |= SPROX_SETTINGS_PROTOCOL_BUS;
		break;
	case COM_PROTO_BIN:
		value |= SPROX_SETTINGS_PROTOCOL_BIN;
		break;
	case COM_PROTO_ASCII:
		value |= SPROX_SETTINGS_PROTOCOL_ASCII;
		break;
	case COM_PROTO_OSI3964:
	default:
		break;
	}

	switch (sprox_ctx->com_settings & COM_BAUDRATE_MASK)
	{
	case COM_BAUDRATE_115200:
		value |= SPROX_SETTINGS_BAUDRATE_115200;
		break;
	case COM_BAUDRATE_38400:
	default:
		break;
	}

	if (sprox_ctx->com_settings & COM_OPTION_SERIAL_RS485)
		value |= SPROX_SETTINGS_CHANNEL_RS485;

	if (settings != NULL)
		*settings = value;

	return MI_OK;
}

/**f* SpringProx.API/SPROX_ReaderSetDeviceSettings
 *
 * NAME
 *   SPROX_ReaderSetDeviceSettings
 *
 * NOTES
 *   This function is RFU and intentionaly left undocumented. Do not call it.
 *
 **/
SPROX_API_FUNC(ReaderSetDeviceSettings) (SPROX_PARAM  DWORD settings)
{
	SPROX_PARAM_TO_CTX;

	if (settings & SPROX_SETTINGS_FORCE_PROTOCOL)
	{
		switch (settings & SPROX_SETTINGS_PROTOCOL_MASK)
		{
		case SPROX_SETTINGS_PROTOCOL_OSI:
			DefaultSettingsAllowed &= ~COM_PROTO_MASK;
			DefaultSettingsAllowed |= COM_PROTO_OSI3964;
			break;
		case SPROX_SETTINGS_PROTOCOL_ASCII:
			DefaultSettingsAllowed &= ~COM_PROTO_MASK;
			DefaultSettingsAllowed |= COM_PROTO_ASCII;
			break;
		case SPROX_SETTINGS_PROTOCOL_BUS:
			DefaultSettingsAllowed &= ~COM_PROTO_MASK;
			DefaultSettingsAllowed |= COM_PROTO_BUS;
			break;
		case SPROX_SETTINGS_PROTOCOL_BIN:
			DefaultSettingsAllowed &= ~COM_PROTO_MASK;
			DefaultSettingsAllowed |= COM_PROTO_BIN;
			break;
		}
	}

	if (settings & SPROX_SETTINGS_FORCE_BAUDRATE)
	{
		if (settings & SPROX_SETTINGS_BAUDRATE_115200)
		{
			DefaultSettingsAllowed &= ~COM_BAUDRATE_MASK;
			DefaultSettingsAllowed |= COM_BAUDRATE_115200;
		}
		else
		{
			DefaultSettingsAllowed &= ~COM_BAUDRATE_MASK;
			DefaultSettingsAllowed |= COM_BAUDRATE_38400;
		}
	}

	if (settings & SPROX_SETTINGS_FORCE_CHANNEL_RS485)
	{
		DefaultSettingsAllowed |= COM_OPTION_SERIAL_RS485;
	}

	return MI_OK;
}

/**f* SpringProx.API/SPROX_ReaderGetDevice
 *
 * NAME
 *   SPROX_ReaderGetDevice
 *
 * DESCRIPTION
 *   Retrieve the name of the device where the SpringProx has been found by SPROX_ReaderOpen
 *
 * INPUTS
 *   TCHAR device[]     : buffer to receive the device name
 *   WORD  len          : size of the buffer
 *
 * RETURNS
 *   MI_OK              : success
 *
 **/
SPROX_API_FUNC(ReaderGetDevice) (SPROX_PARAM  TCHAR device[], WORD len)
{
	SPROX_PARAM_TO_CTX;
	UNUSED_PARAMETER(sprox_ctx);

#ifdef UNICODE
	return SPROX_API_CALL(ReaderGetDeviceW) (SPROX_PARAM_P  device, len);
#else
	return SPROX_API_CALL(ReaderGetDeviceA) (SPROX_PARAM_P  device, len);
#endif
}

SPROX_API_FUNC(ReaderGetDeviceW) (SPROX_PARAM  wchar_t* device, WORD len)
{
	int i;
	SPROX_PARAM_TO_CTX;

	if (device == NULL)
		return MI_LIB_CALL_ERROR;
	if (len == 0)
		return MI_LIB_CALL_ERROR;
	for (i = 0; i < len; i++)
	{
		device[i] = sprox_ctx->com_name[i];
		if (device[i] == '\0') break;
	}
	device[len - 1] = '\0';
	return MI_OK;
}
SPROX_API_FUNC(ReaderGetDeviceA) (SPROX_PARAM  char* device, WORD len)
{
	int i;
	SPROX_PARAM_TO_CTX;

	if (device == NULL)
		return MI_LIB_CALL_ERROR;
	if (len == 0)
		return MI_LIB_CALL_ERROR;
	for (i = 0; i < len; i++)
	{
		device[i] = (char)sprox_ctx->com_name[i];
		if (device[i] == '\0') break;
	}
	device[len - 1] = '\0';
	return MI_OK;
}

SPROX_API_FUNC(ReaderSetFirmwareVersion) (SPROX_PARAM  DWORD version)
{
	SPROX_PARAM_TO_CTX;
	sprox_ctx->sprox_version = version;
	return MI_OK;
}

SPROX_API_FUNC(ReaderSetFirmwareFeatures) (SPROX_PARAM  DWORD features)
{
	SPROX_PARAM_TO_CTX;
	sprox_ctx->sprox_capabilities = features;
	return MI_OK;
}

SPROX_API_FUNC(ReaderLpcd) (SPROX_PARAM  BYTE action, BYTE* buffer, int buffer_size)
{
	WORD    buflen = SPROX_FRAME_CONTENT_SIZE;
	BYTE    send_buffer[SPROX_FRAME_CONTENT_SIZE];
	BYTE    recv_buffer[SPROX_FRAME_CONTENT_SIZE];
	SWORD   rc = MI_RESPONSE_INVALID;

	SPROX_PARAM_TO_CTX;

	if (action == 0x01)
	{
		rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_LPCD, NULL, 0, recv_buffer, &buflen);
		return rc;
	}

	send_buffer[0] = action;

	if ((buffer != NULL) &&
		(buffer_size > 0))
	{
		memcpy(&send_buffer[1], buffer, buffer_size);
	}

	rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_LPCD, send_buffer, (buffer_size + 1), recv_buffer, &buflen);

	return rc;
}


SPROX_API_FUNC(ReaderFdt) (SPROX_PARAM  DWORD* Fdt)
{
	WORD    buflen = SPROX_FRAME_CONTENT_SIZE;
	BYTE    recv_buffer[SPROX_FRAME_CONTENT_SIZE];
	SWORD   rc = MI_RESPONSE_INVALID;

	SPROX_PARAM_TO_CTX;

	rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_PCDREADEFDT, NULL, 0, recv_buffer, &buflen);

	if (Fdt != NULL)
	{
		*Fdt = (DWORD)(recv_buffer[3] << 24);
		*Fdt += (DWORD)(recv_buffer[2] << 16);
		*Fdt += (DWORD)(recv_buffer[1] << 8);
		*Fdt += (DWORD)(recv_buffer[0]);
	}

	return rc;
}




