/**h* SpringProx.API/MSO
 *
 * NAME
 *   MSO
 *
 * DESCRIPTION
 *   SpringProxAPI :: Access to Sagem fingerprint module
 *   Those functions work only on readers featuring a Sagem MorphoSmart CBM module
 *
 * NOTES
 *   Parameters of the SPROX_MSO_Exchange function are directly sent to reader's
 *   embedded MSO.
 *   Please refer to Sagem MorphoSmart documentation for for details regarding this
 *   function.
 *
 **/

 /*

   SpringProx API
   --------------

   Copyright (c) 2000-2008 SpringCard - www.springcard.com

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
   TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   History
   -------

   JDA 11/03/2005 : created
   JDA 22/06/2006 : added re-entrant stuff

 */
#include "sprox_api_i.h"

#ifndef SPROX_API_NO_MSO

 /* Note : the sagem_mso.h is part of SpringBio SDK, not SpringProx */
#include "sagem_mso/sagem_mso.h"

static BOOL MSO_GetNextILV(BYTE* buffer, WORD* offset, BYTE* tag, WORD* length, BYTE** value)
{
	WORD    l;

	if ((buffer == NULL) || (offset == NULL))
		return FALSE;

	if (tag != NULL)
		*tag = buffer[*offset];
	l = buffer[*offset + 2];
	l *= 0x0100;
	l += buffer[*offset + 1];
	if (length != NULL)
		*length = l;
	if (value != NULL)
		*value = &buffer[*offset + 3];

	*offset = *offset + 3 + l;

	if (l > 1024)
		return FALSE;
	return TRUE;
}

static BOOL MSO_IsAsyncMessage(BYTE* buffer)
{
	BYTE    tag;
	WORD    offset = 0;

	if (!MSO_GetNextILV(buffer, &offset, &tag, NULL, NULL))
		return FALSE;

	if (tag == ILV_ASYNC_MESSAGE)
		return TRUE;

	return FALSE;
}

/**f* SpringProx.API/SPROX_MSO_Exchange
 *
 * NAME
 *   SPROX_MSO_Exchange
 *
 * DESCRIPTION
 *   Send a command to the MSO fingerprint reader, and retrieve its answer
 *
 * INPUTS
 *   const BYTE send_buffer[] : buffer to sens to the card
 *   WORD send_len            : length of send_buffer
 *   BYTE recv_buffer[]       : buffer for card's answer
 *   WORD *recv_len           : input  : size of recv_buffer
 *                              output : actual length of reply
 *   DWORD timeout            : timeout (in ms) allowed for fingerprint operation
 *   BYTE *async              : will be TRUE if answer is an asynchronous message
 *
 * RETURNS
 *   MI_OK                    : success
 *   MSO specific status code if an error has occured ; please refer to
 *   relevant Sagem documentation.
 *
 * SEE ALSO
 *   SPROX_MSO_Open
 *   SPROX_MSO_Close
 *
 **/
SPROX_API_FUNC(MSO_Exchange) (SPROX_PARAM  const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD* recv_len, DWORD timeout, BYTE* async)
{
	BYTE    buffer[SPROX_FRAME_CONTENT_SIZE];
	WORD    size;
	SWORD   rc;

again:

	buffer[0] = (BYTE)((timeout / 0x01000000) % 0x00000100);
	buffer[1] = (BYTE)((timeout / 0x00010000) % 0x00000100);
	buffer[2] = (BYTE)((timeout / 0x00000100) % 0x00000100);
	buffer[3] = (BYTE)(timeout % 0x01000000);
	if ((send_buffer != NULL) && (send_len != 0))
		memcpy(&buffer[4], send_buffer, send_len);
	else
		send_len = 0;

	size = sizeof(buffer);

	rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_MSO_EXCHANGE, buffer, (WORD)(4 + send_len), buffer, &size);
	if (rc != ILV_OK)
	{
		SPROX_Trace(TRACE_ERRORS, "MSO_Exchange : error %d", rc);
		return rc;
	}

	if (MSO_IsAsyncMessage(buffer))
	{
		/* Asynchronous message */
		if (async == NULL)
		{
			/* Process locally, do not forward to the application */
			send_len = 0;
			goto again;
		}
		else
		{
			/* Processing by the application (hope it will be fast enough...) */
			*async = TRUE;
		}
	}
	else
	{
		if (async != NULL)
			*async = FALSE;
	}

	if (recv_len != NULL)
	{
		if (*recv_len < size)
		{
			SPROX_Trace(TRACE_ERRORS, "MSO_Exchange : overflow");
			rc = ILVERR_OVERFLOW;
			size = *recv_len;
		}
		else
		{
			*recv_len = size;
		}
	}

	if (recv_buffer != NULL)
	{
		memcpy(recv_buffer, buffer, size);
	}

	return rc;
}

/**f* SpringProx.API/SPROX_MSO_Open
 *
 * NAME
 *   SPROX_MSO_Open
 *
 * DESCRIPTION
 *   Power up the MSO fingerprint reader, and retrieve its information
 *
 * INPUTS
 *   TCHAR *mso_product  : MSO product information
 *                         (must be at least 256 characters long)
 *   TCHAR *mso_firmware : MSO embedded firmware information
 *                         (must be at least 256 characters long)
 *   TCHAR *mso_sensor   : MSO sensor information
 *                         (must be at least 256 characters long)
 *
 * RETURNS
 *   MI_OK              : success
 *   MSO specific status code if an error has occured ; please refer to
 *   relevant Sagem documentation.
 *
 **/
SPROX_API_FUNC(MSO_Open) (SPROX_PARAM  TCHAR* mso_product, TCHAR* mso_firmware, TCHAR* mso_sensor)
{
	BYTE    buffer[SPROX_FRAME_CONTENT_SIZE];
	WORD    size, offset, length, i;
	BYTE    tag;
	BYTE* value;
	SWORD   rc;

	/* Open the MSO in the CSB/SpringProx */
	buffer[0] = SPROX_MSO_FUNC_OPEN;
	size = sizeof(buffer);
	rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_MSO_FUNCTION, buffer, 1, buffer, &size);
	if (rc != ILV_OK)
		return rc;

	/* Retrieve MSO Descriptor */
	buffer[0] = 0x05;
	buffer[1] = 0x01;
	buffer[2] = 0x00;
	buffer[3] = 0x2F;
	size = sizeof(buffer);
	rc = SPROX_API_CALL(MSO_Exchange) (SPROX_PARAM_P  buffer, 4, buffer, &size, 1000, NULL);
	if (rc != ILV_OK)
		return rc;

	offset = 4;
	while ((offset < size) && (MSO_GetNextILV(buffer, &offset, &tag, &length, &value)))
	{
		switch (tag)
		{
		case ID_DESC_PRODUCT:
			if (mso_product != NULL)
			{
				for (i = 0; i < length; i++)
					mso_product[i] = value[i];
				mso_product[length] = '\0';
				SPROX_Trace(TRACE_INFOS, "MSO product : %s", mso_product);
			}
			break;
		case ID_DESC_SOFTWARE:
			if (mso_firmware != NULL)
			{
				for (i = 0; i < length; i++)
					mso_firmware[i] = value[i];
				mso_firmware[length] = '\0';
				SPROX_Trace(TRACE_INFOS, "MSO firmware : %s", mso_firmware);
			}
			break;
		case ID_DESC_SENSOR:
			if (mso_sensor != NULL)
			{
				for (i = 0; i < length; i++)
					mso_sensor[i] = value[i];
				mso_sensor[length] = '\0';
				SPROX_Trace(TRACE_INFOS, "MSO sensor : %s", mso_sensor);
			}
			break;
		default:
			break;
		}
	}

	return rc;
}

#ifdef WIN32
SPROX_API_FUNC(MSO_OpenW) (SPROX_PARAM  wchar_t* mso_product, wchar_t* mso_firmware, wchar_t* mso_sensor)
{
#ifdef UNICODE
	return SPROX_API_CALL(MSO_Open) (SPROX_PARAM_P  mso_product, mso_firmware, mso_sensor);
#else
	char buffer[3][256];
	int i;
	SWORD rc;
	rc = SPROX_API_CALL(MSO_Open) (SPROX_PARAM_P  buffer[0], buffer[1], buffer[2]);
	if (rc != MI_OK) return rc;
	if (mso_product != NULL)
		for (i = 0; i < 256; i++)
		{
			mso_product[i] = buffer[0][i];
			if (mso_product[i] == '\0') break;
		}
	if (mso_firmware != NULL)
		for (i = 0; i < 256; i++)
		{
			mso_firmware[i] = buffer[1][i];
			if (mso_firmware[i] == '\0') break;
		}
	if (mso_sensor != NULL)
		for (i = 0; i < 256; i++)
		{
			mso_sensor[i] = buffer[2][i];
			if (mso_sensor[i] == '\0') break;
		}
	return MI_OK;
#endif
}
#ifndef UNDER_CE
SPROX_API_FUNC(MSO_OpenA)  (SPROX_PARAM  char* mso_product, char* mso_firmware, char* mso_sensor)
{
#ifndef UNICODE
	return SPROX_API_CALL(MSO_Open) (SPROX_PARAM_P  mso_product, mso_firmware, mso_sensor);
#else
	wchar_t buffer[3][256];
	int i;
	SWORD rc;
	rc = SPROX_API_CALL(MSO_Open) (SPROX_PARAM_P  buffer[0], buffer[1], buffer[2]);
	if (rc != MI_OK) return rc;
	if (mso_product != NULL)
		for (i = 0; i < 256; i++)
		{
			mso_product[i] = (char)buffer[0][i];
			if (mso_product[i] == '\0') break;
		}
	if (mso_firmware != NULL)
		for (i = 0; i < 256; i++)
		{
			mso_firmware[i] = (char)buffer[1][i];
			if (mso_firmware[i] == '\0') break;
		}
	if (mso_sensor != NULL)
		for (i = 0; i < 256; i++)
		{
			mso_sensor[i] = (char)buffer[2][i];
			if (mso_sensor[i] == '\0') break;
		}
	return MI_OK;
#endif
}
#endif
#endif

/**f* SpringProx.API/SPROX_MSO_Close
 *
 * NAME
 *   SPROX_MSO_Close
 *
 * DESCRIPTION
 *   Power down the MSO fingerprint reader
 *
 **/
SPROX_API_FUNC(MSO_Close) (SPROX_PARAM_V)
{
	BYTE    buffer[SPROX_FRAME_CONTENT_SIZE];
	WORD    size;

	buffer[0] = SPROX_MSO_FUNC_CLOSE;
	size = sizeof(buffer);
	return SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_MSO_FUNCTION, buffer, 1, buffer, &size);
}


#endif
