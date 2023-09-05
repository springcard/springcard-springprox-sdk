/**h* SpringProx.API/Find
 *
 * NAME
 *   SpringProx.API :: Automatic card discovery functions
 *
 * DESCRIPTION
 *   Those functions work only for readers having version >= 1.44
 *   LPCD functions are available only in some specific readers
 *
 * PORTABILITY
 *   Win32 only for the multi-threaded and event driven part
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

   JDA 28/02/2008 : created
   JDA 30/01/2012 : forget pcd_current_rf_protocol after ControlRF
   JDA 04/12/2013 : added SPROX_FindLpcd and SPROX_FindLpcdEx

 */

#include "sprox_api_i.h"

 /**f* SpringProx.API/SPROX_FindEx
  *
  * NAME
  *   SPROX_FindEx
  *
  * DESCRIPTION
  *   Automatic discovery of any contactless card in the field, returning
  *   protocol-related information
  *
  * NOTES
  *   This function is only avalaible for readers having version >= 1.53
  *
  * INPUTS
  *   WORD want_protos   : bit-map of contactless family to look for
  *   WORD *got_proto    : on exit, found contactless family
  *                        (one bit only is set, or none)
  *   BYTE uid[10]       : Unique IDentifier of the found card
  *   BYTE *uidlen       : on input, size of UID
  *                        on output, actual length of UID
  *   BYTE info[32]      : Protocol-related information
  *   BYTE *infolen      : on input, size of info
  *                        on output, actual length of info
  *
  * RETURNS
  *   MI_OK              : success, one card selected
  *   MI_NOTAGERR        : no card available in the RF field
  *   Other code if internal or communication error has occured.
  *
  * DETAILS
  *   The 'info' buffer returns data dependant on the card's protocol
  *   - ISO 14443 A cards :
  *     byte 0 : ATQ (LSB)
  *     byte 1 : ATQ (MSB)
  *     byte 2 : SAK
  *   - ISO 14443 B cards : info is the ATQB (short ATQ on 11 bytes, or extended ATQB
  *     on 12 bytes if both the reader and the card support it)
  *   - Calypso cards running the Innovatron protocol : REPGEN
  *   - other protocols : no protocol-related data, infolen is set to 0
  *
  **/
SPROX_API_FUNC(FindEx) (SPROX_PARAM  WORD want_protos, WORD* got_proto, BYTE uid[], BYTE* uidlen, BYTE info[], BYTE* infolen)
{
	BYTE l_infolen;
	SWORD rc;
	SPROX_PARAM_TO_CTX;

	/* Forget current mode so that later SetConfig will actually reconfigure the reader */
	sprox_ctx->pcd_current_rf_protocol = 0;

	if (infolen != NULL)
	{
		l_infolen = *infolen;
		*infolen = 0;
	}

	if (sprox_ctx->sprox_version < 0x00014600)
	{
		if (want_protos & 0x0002)
		{
			/* 14443-B */
			/* ------- */
			BYTE atqb[11];

			rc = SPROX_API_CALL(B_SelectIdle)  (SPROX_PARAM_P  0x00, atqb);
			if (rc == MI_OK)
			{
				if (got_proto != NULL)
					*got_proto = 0x0002;

				if (uidlen == NULL)
				{
					if (uid != NULL)
						memcpy(uid, atqb, 4);
				}
				else
				{
					if ((*uidlen != 0) && (*uidlen < 4))
					{
						rc = MI_RESPONSE_OVERFLOW;
					}
					else
						if (uid != NULL)
						{
							memcpy(uid, atqb, 4);
						}
					*uidlen = 4;
				}

				if ((rc == MI_OK) && (info != NULL) && (l_infolen >= sizeof(atqb)))
				{
					memcpy(info, atqb, sizeof(atqb));

					if (infolen != NULL)
						*infolen = sizeof(atqb);
				}

				return rc;
			}
		}

		if (want_protos & 0x0001)
		{
			/* 14443-A */
			/* ------- */
			BYTE atq[2], sak[1];

			rc = SPROX_API_CALL(A_SelectIdle) (SPROX_PARAM_P  atq, uid, uidlen, sak);

			if (rc == MI_OK)
			{
				if (got_proto != NULL)
					*got_proto = 0x0001;

				if ((info != NULL) && (l_infolen >= 3))
				{
					memcpy(&info[0], atq, 2);
					memcpy(&info[2], sak, 1);

					if (infolen != NULL)
						*infolen = 3;
				}


				return rc;
			}
		}

		rc = MI_NOTAGERR;

	}
	else
	{
		BYTE buffer[64];
		WORD buflen;

		buffer[0] = (BYTE)(want_protos / 0x0100);
		buffer[1] = (BYTE)(want_protos % 0x0100);

		buflen = sizeof(buffer);
		rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_FIND, buffer, 2, buffer, &buflen);

		if (rc == MI_OK)
		{
			if (buflen < 2)
			{
				rc = MI_SER_LENGTH_ERR;
			}
			else
			{
				if (got_proto != NULL)
				{
					*got_proto = buffer[0];
					*got_proto *= 0x0100;
					*got_proto += buffer[1];
				}

				if (uid != NULL)
					memcpy(uid, &buffer[2], buflen - 2);

				if (uidlen != NULL)
					*uidlen = (BYTE)(buflen - 2);
			}
		}

		if ((rc == MI_OK) && (info != NULL))
		{
			buffer[0] = SPROX_FIND_INFO_PROT_BYTES;
			buflen = sizeof(buffer);

			rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_FIND_INFO, buffer, 1, buffer, &buflen);

			if (rc == MI_OK)
			{
				if (buflen <= l_infolen)
				{
					memcpy(info, buffer, buflen);
					if (infolen != NULL)
						*infolen = (BYTE)buflen;
				}
			}
			else
			{
				rc = MI_OK;
			}
		}
	}


	return rc;
}

/**f* SpringProx.API/SPROX_Find
 *
 * NAME
 *   SPROX_Find
 *
 * DESCRIPTION
 *   Automatic discovery of any contactless card in the field
 *
 * NOTES
 *   This function is only avalaible for readers having version >= 1.44
 *
 * INPUTS
 *   WORD want_protos   : bit-map of contactless family to look for
 *   WORD *got_proto    : on exit, found contactless family
 *                        (one bit only is set, or none)
 *   BYTE uid[10]       : Unique IDentifier of the found card
 *   BYTE *uidlen       : on input, size of UID
 *                        on output, actual length of UID
 *
 * RETURNS
 *   MI_OK              : success, one card selected
 *   MI_NOTAGERR        : no card available in the RF field
 *   Other code if internal or communication error has occured.
 *
 **/
SPROX_API_FUNC(Find) (SPROX_PARAM  WORD want_protos, WORD* got_proto, BYTE uid[], BYTE* uidlen)
{
	SPROX_PARAM_TO_CTX;

	return SPROX_API_CALL(FindEx) (SPROX_PARAM_P  want_protos, got_proto, uid, uidlen, NULL, NULL);
}

SPROX_API_FUNC(FindWaitCancel) (SPROX_PARAM_V)
{
	SPROX_PARAM_TO_CTX;

	return SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_FIND_INFO, NULL, 0, NULL, NULL);
}



SPROX_API_FUNC(FindWaitEx) (SPROX_PARAM  WORD want_protos, WORD* got_proto, BYTE uid[], BYTE* uidlen, BYTE info[], BYTE* infolen, WORD timeout_s, WORD interval_ms)
{
	BYTE buffer[64];
	WORD buflen;
	BYTE l_infolen;
	SWORD rc;
	SPROX_PARAM_TO_CTX;

	/* Forget current mode so that later SetConfig will actually reconfigure the reader */
	sprox_ctx->pcd_current_rf_protocol = 0;

	if (sprox_ctx->sprox_version < 0x00015400)
		return MI_FUNCTION_NOT_AVAILABLE;

	if (infolen != NULL)
	{
		l_infolen = *infolen;
		*infolen = 0;
	}

	buffer[0] = (BYTE)(want_protos / 0x0100);
	buffer[1] = (BYTE)(want_protos % 0x0100);
	buffer[2] = (BYTE)(timeout_s / 0x0100);
	buffer[3] = (BYTE)(timeout_s % 0x0100);
	buffer[4] = 0x00;
	buffer[5] = (BYTE)(interval_ms / 0x0100);
	buffer[6] = (BYTE)(interval_ms % 0x0100);

	buflen = sizeof(buffer);
	rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_FIND, buffer, 7, buffer, &buflen);

	if ((rc != MI_POLLING) && (rc != MI_OK))
	{
		/* Oups - error */
		return rc;
	}

	if (rc == MI_POLLING)
	{
		/* Wait for the answer */
		buflen = sizeof(buffer);
		rc = SPROX_WRS_FUNC(SPROX_PARAM_P  buffer, &buflen, timeout_s);
	}

	if (rc == MI_OK)
	{
		if (buflen < 2)
		{
			rc = MI_SER_LENGTH_ERR;
		}
		else
		{
			if (got_proto != NULL)
			{
				*got_proto = buffer[0];
				*got_proto *= 0x0100;
				*got_proto += buffer[1];
			}

			if (uid != NULL)
				memcpy(uid, &buffer[2], buflen - 2);

			if (uidlen != NULL)
				*uidlen = (BYTE)(buflen - 2);
		}
	}

	if ((rc == MI_OK) && (info != NULL))
	{
		buffer[0] = SPROX_FIND_INFO_PROT_BYTES;
		buflen = sizeof(buffer);

		rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_FIND_INFO, buffer, 1, buffer, &buflen);

		if (rc == MI_OK)
		{
			if (buflen <= l_infolen)
			{
				memcpy(info, buffer, buflen);
				if (infolen != NULL)
					*infolen = (BYTE)buflen;
			}
		}
		else
		{
			rc = MI_OK;
		}
	}

	return rc;
}

SPROX_API_FUNC(FindWait) (SPROX_PARAM  WORD want_protos, WORD* got_proto, BYTE uid[], BYTE* uidlen, WORD timeout_s, WORD interval_ms)
{
	SPROX_PARAM_TO_CTX;

	return SPROX_API_CALL(FindWaitEx) (SPROX_PARAM_P  want_protos, got_proto, uid, uidlen, NULL, NULL, timeout_s, interval_ms);
}



SPROX_API_FUNC(FindLpcdEx) (SPROX_PARAM  WORD want_protos, WORD* got_proto, BYTE uid[], BYTE* uidlen, BYTE info[], BYTE* infolen, WORD timeout_s, BOOL forced_pulses)
{
	BYTE buffer[64];
	WORD buflen;
	BYTE l_infolen;
	SWORD rc;
	SPROX_PARAM_TO_CTX;

	/* Forget current mode so that later SetConfig will actually reconfigure the reader */
	sprox_ctx->pcd_current_rf_protocol = 0;

	if (sprox_ctx->sprox_version < 0x00015400)
		return MI_FUNCTION_NOT_AVAILABLE;

	if (infolen != NULL)
	{
		l_infolen = *infolen;
		*infolen = 0;
	}

	buffer[0] = (BYTE)(want_protos / 0x0100);
	buffer[1] = (BYTE)(want_protos % 0x0100);
	buffer[2] = (BYTE)(timeout_s / 0x0100);
	buffer[3] = (BYTE)(timeout_s % 0x0100);
	buffer[4] = 0x01;
	if (forced_pulses)
		buffer[4] |= 0x02;

	buflen = sizeof(buffer);
	rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_FIND, buffer, 5, buffer, &buflen);

	if ((rc != MI_POLLING) && (rc != MI_OK))
	{
		/* Oups - error */
		return rc;
	}

	if (rc == MI_POLLING)
	{
		/* Wait for the answer */
		buflen = sizeof(buffer);
		rc = SPROX_WRS_FUNC(SPROX_PARAM_P  buffer, &buflen, timeout_s);
	}

	if (rc == MI_OK)
	{
		if (buflen < 2)
		{
			rc = MI_SER_LENGTH_ERR;
		}
		else
		{
			if (got_proto != NULL)
			{
				*got_proto = buffer[0];
				*got_proto *= 0x0100;
				*got_proto += buffer[1];
			}

			if (uid != NULL)
				memcpy(uid, &buffer[2], buflen - 2);

			if (uidlen != NULL)
				*uidlen = (BYTE)(buflen - 2);
		}
	}

	if ((rc == MI_OK) && (info != NULL))
	{
		buffer[0] = SPROX_FIND_INFO_PROT_BYTES;
		buflen = sizeof(buffer);

		rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_FIND_INFO, buffer, 1, buffer, &buflen);

		if (rc == MI_OK)
		{
			if (buflen <= l_infolen)
			{
				memcpy(info, buffer, buflen);
				if (infolen != NULL)
					*infolen = (BYTE)buflen;
			}
		}
		else
		{
			rc = MI_OK;
		}
	}

	return rc;
}

SPROX_API_FUNC(FindLpcd) (SPROX_PARAM  WORD want_protos, WORD* got_proto, BYTE uid[], BYTE* uidlen, WORD timeout_s)
{
	SPROX_PARAM_TO_CTX;

	return SPROX_API_CALL(FindLpcdEx) (SPROX_PARAM_P  want_protos, got_proto, uid, uidlen, NULL, NULL, timeout_s, FALSE);
}
