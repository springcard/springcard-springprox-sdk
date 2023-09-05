/**h* SpringProx.API/ISO14443-4
 *
 * NAME
 *   SpringProx.API :: ISO 14443 Layer 4 (T=CL)
 *
 * DESCRIPTION
 *   Implementation of ISO/IEC 14443 layer 4 ("T=CL" protocol)
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

   JDA 13/10/2003 : created
   JDA 15/07/2004 : improved support for multiple CID
   JDA 22/06/2006 : added re-entrant stuff
					layer-3 functions moved to sprox_layer3.c

 */
#include "sprox_api_i.h"

#ifndef SPROX_API_NO_TCL

 //#define RC_TCL(rc) ((rc < -60) && (rc >= -95)) ? (rc + 60 - 130) : (rc)
#define RC_TCL(rc) rc

#define TCL_FSCI_MAX  0x08
#define TCL_MAX_RETRY 3
#define MAX_RECV_LEN 256+2+10   /* 256 + 2 for SW1, SW2 + 10 guard */
#define TCL_LONG_TIMEOUT   0x14EB

static SWORD TclA_LowExchange(SPROX_PARAM  BYTE* send_data, WORD send_len, BYTE* recv_data, WORD* recv_len, DWORD timeout);
static SWORD TclB_LowExchange(SPROX_PARAM  BYTE* send_data, WORD send_len, BYTE* recv_data, WORD* recv_len, DWORD timeout);

static WORD Tcl_Make_I_Block(BYTE* frame, WORD* length, BYTE block_num, BYTE cid, BYTE nad, BOOL first_block, WORD block_size, const BYTE* data,
	WORD datalen);
static void Tcl_Make_R_Block(BYTE* frame, WORD* length, BYTE block_num, BYTE cid, BOOL ack);
static void Tcl_Make_S_Block(BYTE* frame, WORD* length, BYTE block_num, BYTE rcv0, BYTE rcv1, BYTE rcv2);
static SWORD Tcl_HalfDuplex(SPROX_PARAM  BOOL type_b, BYTE fsci, BYTE cid, BYTE nad, BYTE* block_num, const BYTE* send_buffer, WORD send_len, BYTE* recv_buffer,
	WORD* recv_len);

static BYTE glob_block_num = 0;

/*
 *****************************************************************************
 *
 *                           ISO 14443-A FUNCTIONS
 *
 *****************************************************************************
 */

 /**f* SpringProx.API/SPROX_TclA_ActivateAgain
  *
  * NAME
  *   SPROX_TclA_ActivateAgain
  *
  * WARNING
  *   This function is deprecated. Use SPROX_A_SelectAgain instead.
  *
  * SEE ALSO
  *   SPROX_A_SelectAgain
  *
  **/
SPROX_API_FUNC(TclA_ActivateAgain) (SPROX_PARAM  const BYTE snr[], BYTE snrlen)
{
	return SPROX_API_CALL(A_SelectAgain) (SPROX_PARAM_P  snr, snrlen);
}

/**f* SpringProx.API/SPROX_TclA_ActivateAny
 *
 * NAME
 *   SPROX_TclA_ActivateAny
 *
 * WARNING
 *   This function is deprecated. Use SPROX_A_SelectAny instead.
 *
 * NOTES
 *   This function is nothing more than an alias to SPROX_A_SelectAny, the
 *   only difference is in the validation of the returned SAK when a card is
 *   found :
 *   - if SAK shows that the card supports ISO/IEC 14443-4 (aka T=CL), the
 *     function returns MI_OK
 *   - if SAK shows that the card doesn't support ISO/IEC 14443-4 (for example,
 *     a Mifare memory card), the function returns MI_CARD_NOT_TCL
 *   Of course if no card is found, the function returns MI_NOTAGERR
 *
 * SEE ALSO
 *   SPROX_A_SelectAny
 *   SPROX_TclA_ActivateIdle
 *
 **/
SPROX_API_FUNC(TclA_ActivateAny) (SPROX_PARAM  BYTE atq[2], BYTE snr[], BYTE* snrlen, BYTE sak[1])
{
	SWORD rc;
	BYTE my_sak;

	if (sak == NULL)
		sak = &my_sak;

	rc = SPROX_API_CALL(A_SelectAny) (SPROX_PARAM_P  atq, snr, snrlen, sak);
	if (rc != MI_OK)
		return rc;

	/* Check that the "PICC compliant with ISO14443-4" bit is set */
	if (!(*sak & 0x20))
		rc = MI_CARD_NOT_TCL;

	return rc;
}

/**f* SpringProx.API/SPROX_TclA_ActivateIdle
 *
 * NAME
 *   SPROX_TclA_ActivateIdle
 *
 * WARNING
 *   This function is deprecated. Use SPROX_A_SelectIdle instead.
 *
 * NOTES
 *   This function is nothing more than an alias to SPROX_A_SelectIdle, the
 *   only difference is in the validation of the returned SAK when a card is
 *   found :
 *   - if SAK shows that the card supports ISO/IEC 14443-4 (aka T=CL), the
 *     function returns MI_OK
 *   - if SAK shows that the card doesn't support ISO/IEC 14443-4 (for example,
 *     a Mifare memory card), the function returns MI_CARD_NOT_TCL
 *   Of course if no card is found, the function returns MI_NOTAGERR
 *
 * SEE ALSO
 *   SPROX_A_SelectIdle
 *   SPROX_TclA_ActivateAny
 *
 **/
SPROX_API_FUNC(TclA_ActivateIdle) (SPROX_PARAM  BYTE atq[2], BYTE snr[], BYTE* snrlen, BYTE sak[1])
{
	SWORD rc;
	BYTE my_sak;

	if (sak == NULL)
		sak = &my_sak;

	rc = SPROX_API_CALL(A_SelectIdle) (SPROX_PARAM_P  atq, snr, snrlen, sak);
	if (rc != MI_OK)
		return rc;

	/* Check that the "PICC compliant with ISO14443-4" bit is set */
	if (!(*sak & 0x20))
		rc = MI_CARD_NOT_TCL;

	return rc;
}

/**f* SpringProx.API/SPROX_TclA_GetAts
 *
 * NAME
 *   SPROX_TclA_GetAts
 *
 * DESCRIPTION
 *   Send the T=CL RATS command to enter ISO 14443-A-4 dialog
 *   with currently selected card
 *
 * INPUTS
 *   BYTE cid           : CID to affect to the card
 *                        (set to 0xFF if you don't use CIDs)
 *   BYTE ats[32]       : buffer to receive the Answer To Select of the card
 *   BYTE *atslen       : input  = size of the ats buffer
 *                        output = actual ats length
 *
 * RETURNS
 *   MI_OK              : success, T=CL dialog with the card activated
 *   Other code if internal or communication error has occured.
 *
 * SEE ALSO
 *   SPROX_TclA_Deselect
 *   SPROX_TclA_Pps
 *
 **/
SPROX_API_FUNC(TclA_GetAts) (SPROX_PARAM  BYTE cid, BYTE ats[], BYTE* atslen)
{
	BYTE    buffer[32];
	WORD    buflen;
	SWORD   rc;
	BOOL first_time = TRUE;
	SPROX_PARAM_TO_CTX;

again:

	buflen = sizeof(buffer);

	if (sprox_ctx->sprox_version >= 0x00011201)
	{
		/* Version >= 1.12.1 : Function is inside the reader */
		buffer[0] = SPROX_TCL_FUNC_RATS;
		buffer[1] = cid;

		rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_TCL_FUNC, buffer, 2, buffer, &buflen);

		if ((rc == -93) && (first_time))
		{
			/* CID is active... API is nice enough to overcome this "user error" silently */

			rc = SPROX_API_CALL(TclA_Deselect) (SPROX_PARAM_P  cid);
			if (rc != MI_OK)
				return rc;

			first_time = FALSE;
			goto again;
		}

		if (rc != MI_OK)
			return RC_TCL(rc);

	}
	else
	{
		/* Function provided by software */
		BYTE    buffer[32];
		BYTE    frame[2];
		WORD    buflen;
		SWORD   rc;

		frame[0] = 0xE0;
		frame[1] = 0x50;            /* FSD set to 64 */

		buflen = sizeof(buffer);

		rc = TclA_LowExchange(SPROX_PARAM_P  frame, sizeof(frame), buffer, &buflen, TCL_LONG_TIMEOUT);
		if (rc != MI_OK)
			return rc;

	}

	if (atslen != NULL)
	{
		if (buflen > *atslen)
			return MI_RESPONSE_OVERFLOW;
		*atslen = (BYTE)buflen;
	}
	if (ats != NULL)
		memcpy(ats, buffer, buflen);

	return MI_OK;
}

/**f* SpringProx.API/SPROX_TclA_Pps
 *
 * NAME
 *   SPROX_TclA_Pps
 *
 * DESCRIPTION
 *   Send a T=CL PPS command according to ISO 14443-A-4 dialog
 *
 * INPUTS
 *   BYTE cid           : CID of the card
 *                        (set to 0xFF if you don't use CIDs)
 *   BYTE dsi           : DSI parameter according to ISO 14443-A
 *   BYTE dri           : DRI parameter according to ISO 14443-A
 *
 * RETURNS
 *   MI_OK              : success
 *   Other code if internal or communication error has occured.
 *
 * NOTES
 *   After a successfull PPS negociation, the reader is automatically configured
 *   according to DSI and DRI before returning MI_OK.
 *
 * WARNING
 *   This feature is still experimental.
 *
 * SEE ALSO
 *   SPROX_TclA_GetAts
 *
 **/
SPROX_API_FUNC(TclA_Pps) (SPROX_PARAM  BYTE cid, BYTE dsi, BYTE dri)
{
	SPROX_PARAM_TO_CTX;

	if (sprox_ctx->sprox_version >= 0x00011201)
	{
		/* Version >= 1.12.1 : Function is inside the reader */
		BYTE    buffer[4];
		SWORD   rc;

		buffer[0] = SPROX_TCL_FUNC_PPS;
		buffer[1] = cid;
		buffer[2] = dsi;
		buffer[3] = dri;

		rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_TCL_FUNC, buffer, 4, NULL, NULL);
		return RC_TCL(rc);

	}
	else
	{
		/* Function is not supported */
		return MI_UNKNOWN_FUNCTION;
	}
}

/**f* SpringProx.API/SPROX_TclA_Deselect
 *
 * NAME
 *   SPROX_TclA_Deselect
 *
 * DESCRIPTION
 *   Send the T=CL DESELECT command to an ISO 14443-A-4 card
 *
 * INPUTS
 *   BYTE cid           : CID of the card
 *                        (set to 0xFF if you don't use CIDs)
 *
 * RETURNS
 *   MI_OK              : success, card deselected
 *   Other code if internal or communication error has occured.
 *
 * SEE ALSO
 *   SPROX_TclA_GetAts
 *   SPROX_A_Halt
 *
 **/
SPROX_API_FUNC(TclA_Deselect) (SPROX_PARAM  BYTE cid)
{
	SPROX_PARAM_TO_CTX;

	if (sprox_ctx->sprox_version >= 0x00011201)
	{
		/* Version >= 1.12.1 : Function is inside the reader */
		BYTE    buffer[2];
		SWORD   rc;
		buffer[0] = SPROX_TCL_FUNC_DESELECT;
		buffer[1] = cid;

		rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_TCL_FUNC, buffer, 2, NULL, NULL);
		return RC_TCL(rc);

	}
	else
	{
		/* Function provided by software */
		BYTE    buffer[32];
		BYTE    frame[1];
		WORD    buflen;

		frame[0] = PCB_MK_S_BLOCK | PCB_S_DESELECT;
		buflen = sizeof(buffer);

		return SPROX_API_CALL(A_Exchange) (SPROX_PARAM_P  frame, sizeof(frame), buffer, &buflen, TRUE, 1000);
	}
}

/**f* SpringProx.API/SPROX_TclA_Halt
 *
 * NAME
 *   SPROX_TclA_Halt
 *
 * WARNING
 *   This function is deprecated. Use SPROX_A_Halt instead.
 *
 * NOTES
 *   If card is already in T=CL mode, you must DESELECT the card instead of
 *   HALTing it.
 *
 * SEE ALSO
 *   SPROX_A_Halt
 *   SPROX_TclA_Deselect
 *
 **/
SPROX_API_FUNC(TclA_Halt) (SPROX_PARAM_V)
{
	return SPROX_API_CALL(A_Halt) (SPROX_PARAM_PV);
}

/**f* SpringProx.API/SPROX_TclA_Exchange
 *
 * NAME
 *   SPROX_TclA_Exchange
 *
 * DESCRIPTION
 *   Perform a T=CL block exchange according to ISO 14443-A-4
 *
 * INPUTS
 *   BYTE fsci                : FSCI parameter according to ISO 14443-A
 *                              (set to 0xFF for default card value)
 *   BYTE cid                 : CID of the card
 *                              (set to 0xFF if you don't use CIDs)
 *   BYTE nad                 : NAD of the card
 *                              (set to 0xFF if you don't use NADs)
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
 * NOTES
 *   I-Block chaining is automatically performed by the reader according to the
 *   FSCI parameter.
 *   Wait Time eXtension S-Block coming from the card are also handled directly
 *   by the reader.
 *
 * SEE ALSO
 *   SPROX_TclA_GetAts
 *   SPROX_TclA_Deselect
 *
 **/
SPROX_API_FUNC(TclA_Exchange) (SPROX_PARAM  BYTE fsci, BYTE cid, BYTE nad, const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD* recv_len)
{
	SPROX_PARAM_TO_CTX;

	if (sprox_ctx->sprox_version >= 0x00011201)
	{
		/* Version >= 1.12.1 : Function is inside the reader */
		BYTE   buffer[256 + 4];
		SWORD  rc;

		/* Check parameters */
		if ((send_buffer == NULL) || (send_len > 256) || (recv_buffer == NULL) || (recv_len == NULL))
			return MI_LIB_CALL_ERROR;

		if ((cid == TCL_UNUSED_CID) && (nad == TCL_UNUSED_NAD))
		{
			if (fsci == 0xFF)
			{
				buffer[0] = SPROX_TCL_FUNC_EXCH;
				memcpy(&buffer[1], send_buffer, send_len);
				send_len += 1;
			}
			else
			{
				buffer[0] = SPROX_TCL_FUNC_EXCH_FSC;
				buffer[1] = fsci;
				memcpy(&buffer[2], send_buffer, send_len);
				send_len += 2;
			}
		}
		else if (nad == TCL_UNUSED_NAD)
		{
			buffer[0] = SPROX_TCL_FUNC_EXCH_FSC_CID;
			buffer[1] = fsci;
			buffer[2] = cid;
			memcpy(&buffer[3], send_buffer, send_len);
			send_len += 3;
		}
		else
		{
			buffer[0] = SPROX_TCL_FUNC_EXCH_FSC_CID_NAD;
			buffer[1] = fsci;
			buffer[2] = cid;
			buffer[3] = nad;
			memcpy(&buffer[4], send_buffer, send_len);
			send_len += 4;
		}

		rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_TCL_FUNC, buffer, send_len, recv_buffer, recv_len);
		if (rc == MI_OK)
			return MI_OK;

		/* Error in exchange, PICC has been deselected */
		return RC_TCL(rc);

	}
	else
	{
		/* Function provided by software */
		return Tcl_HalfDuplex(SPROX_PARAM_P  FALSE, fsci, cid, nad, &glob_block_num, send_buffer, send_len, recv_buffer, recv_len);
	}
}

/*
 * T=CL exchange, DESFire optimized loop
 * -------------------------------------
 */
SPROX_API_FUNC(TclA_ExchangeDF) (SPROX_PARAM  BYTE cid, const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD* recv_len)
{
	SPROX_PARAM_TO_CTX;

	if (sprox_ctx->sprox_version >= 0x00011201)
	{
		/* Version >= 1.12.1 : Function is inside the reader */
		SWORD   rc;
		BYTE    buffer[256 + 2];

		/* Check parameters */
		if ((send_buffer == NULL) || (send_len > 256) || (recv_buffer == NULL) || (recv_len == NULL)) return MI_LIB_CALL_ERROR;

		/* Build command */
		buffer[0] = SPROX_TCL_FUNC_DESFIRE;
		buffer[1] = cid;
		memcpy(&buffer[2], send_buffer, send_len);
		send_len += 2;

		rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_TCL_FUNC, buffer, send_len, recv_buffer, recv_len);
		if (rc == MI_OK)
			return rc;

		return RC_TCL(rc);

	}
	else
	{
		/* Function provided by software */
		return SPROX_API_CALL(TclA_Exchange) (SPROX_PARAM_P  0x08, 0xFF, 0xFF, send_buffer, send_len, recv_buffer, recv_len);
	}
}

/*
 *****************************************************************************
 *
 *                           ISO 14443-B FUNCTIONS
 *
 *****************************************************************************
 */

 /**f* SpringProx.API/SPROX_TclB_ActivateAny
  *
  * NAME
  *   SPROX_TclB_ActivateAny
  *
  * WARNING
  *   This function is deprecated. Use SPROX_B_SelectAny instead.
  *
  * NOTES
  *   This function is nothing more than an alias to SPROX_B_SelectAny, the
  *   only difference is in the validation of the returned ATQ when a card is
  *   found :
  *   - if ATQ shows that the card supports ISO/IEC 14443-4 (aka T=CL), the
  *     function returns MI_OK
  *   - if ATQ shows that the card doesn't support ISO/IEC 14443-4 (for example,
  *     a memory card), the function returns MI_CARD_NOT_TCL
  *   Of course if no card is found, the function returns MI_NOTAGERR
  *
  * SEE ALSO
  *   SPROX_B_SelectAny
  *   SPROX_TclB_ActivateIdle
  *
  **/
SPROX_API_FUNC(TclB_ActivateAny) (SPROX_PARAM  BYTE afi, BYTE atq[11])
{
	SWORD rc;
	BYTE my_atq[11];

	if (atq == NULL)
		atq = my_atq;

	rc = SPROX_API_CALL(B_SelectAny) (SPROX_PARAM_P  afi, atq);
	if (rc != MI_OK)
		return rc;

	/* Check that the "PICC compliant with ISO14443-4" bit is set */
	if (!(atq[9] & 0x01))
		rc = MI_CARD_NOT_TCL;

	return rc;
}

/**f* SpringProx.API/SPROX_TclB_ActivateIdle
 *
 * NAME
 *   SPROX_TclB_ActivateIdle
 *
 * WARNING
 *   This function is deprecated. Use SPROX_B_SelectIdle instead.
 *
 * NOTES
 *   This function is nothing more than an alias to SPROX_B_SelectIdle, the
 *   only difference is in the validation of the returned ATQ when a card is
 *   found :
 *   - if ATQ shows that the card supports ISO/IEC 14443-4 (aka T=CL), the
 *     function returns MI_OK
 *   - if ATQ shows that the card doesn't support ISO/IEC 14443-4 (for example,
 *     a memory card), the function returns MI_CARD_NOT_TCL
 *   Of course if no card is found, the function returns MI_NOTAGERR
 *
 * SEE ALSO
 *   SPROX_B_SelectIdle
 *   SPROX_TclB_ActivateAny
 *
 **/
SPROX_API_FUNC(TclB_ActivateIdle) (SPROX_PARAM  BYTE afi, BYTE atq[11])
{
	SWORD rc;
	BYTE my_atq[11];

	if (atq == NULL)
		atq = my_atq;

	rc = SPROX_API_CALL(B_SelectIdle) (SPROX_PARAM_P  afi, atq);
	if (rc != MI_OK)
		return rc;

	/* Check that the "PICC compliant with ISO14443-4" bit is set */
	if (!(atq[9] & 0x01))
		rc = MI_CARD_NOT_TCL;

	return rc;
}

/**f* SpringProx.API/SPROX_TclB_Attrib
 *
 * NAME
 *   SPROX_TclB_Attrib
 *
 * DESCRIPTION
 *   Send the T=CL ATTRIB command to enter ISO 14443-B-4 dialog specified card
 *
 * INPUTS
 *   const BYTE pupi[4] : first bytes of card's ATQ as returned by SPROX_B_SelectIdle or SPROX_B_SelectAny
 *   BYTE cid           : CID to send to the card
 *                        (set to 0xFF if you don't use CIDs)
 *
 * RETURNS
 *   MI_OK              : success, T=CL dialog with the card activated
 *   Other code if internal or communication error has occured.
 *
 * SEE ALSO
 *   SPROX_TclB_Deselect
 *   SPROX_TclB_AttribEx
 *
 **/
SPROX_API_FUNC(TclB_Attrib) (SPROX_PARAM  const BYTE pupi[4], BYTE cid)
{
	return SPROX_API_CALL(TclB_AttribEx) (SPROX_PARAM_P  pupi, cid, 0, 0);
}

/**f* SpringProx.API/SPROX_TclB_AttribEx
 *
 * NAME
 *   SPROX_TclB_AttribEx
 *
 * DESCRIPTION
 *   Send the T=CL ATTRIB command to enter ISO 14443-B-4 dialog with specified card
 *
 * INPUTS
 *   const BYTE pupi[4] : first bytes of card's ATQ as returned by SPROX_B_SelectIdle or SPROX_B_SelectAny
 *   BYTE cid           : CID to send to the card
 *                        (set to 0xFF if you don't use CIDs)
 *   BYTE dsi           : DSI parameter according to ISO 14443-B
 *   BYTE dri           : DRI parameter according to ISO 14443-B
 *
 * RETURNS
 *   MI_OK              : success, T=CL dialog with the card activated
 *   Other code if internal or communication error has occured.
 *
 * SEE ALSO
 *   SPROX_TclB_Deselect
 *   SPROX_TclB_Attrib
 *
 **/
SPROX_API_FUNC(TclB_AttribEx) (SPROX_PARAM  const BYTE pupi[4], BYTE cid, BYTE dsi, BYTE dri)
{
	SWORD   rc;
	BOOL first_time = TRUE;
	SPROX_PARAM_TO_CTX;

again:

	if (sprox_ctx->sprox_version >= 0x00014100)
	{
		/* Version >= 1.41 : Function is fully implemented inside the reader */
		BYTE    buffer[64];
		WORD    buflen = sizeof(buffer);

		buffer[0] = SPROX_TCL_FUNC_ATTRIB;
		buffer[1] = cid;
		buffer[2] = dsi;
		buffer[3] = dri; /* Corrected 2.02 (was dsi) */
		if (pupi != NULL)
		{
			memcpy(&buffer[4], pupi, 4);
			rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_TCL_FUNC, buffer, 8, buffer, &buflen);
		}
		else
		{
			rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_TCL_FUNC, buffer, 4, buffer, &buflen);
		}

		if ((rc == -93) && (first_time))
		{
			/* CID is active... API is nice enough to overcome this "user error" silently */
			rc = SPROX_API_CALL(TclB_Deselect) (SPROX_PARAM_P  cid);
			if (rc != MI_OK)
				return rc;

			first_time = FALSE;
			goto again;
		}

		if (rc != MI_OK)
			return RC_TCL(rc);

	}
	else
		if (sprox_ctx->sprox_version >= 0x00013700)
		{
			/* Version >= 1.37 : Function is inside the reader but with limitations */
			BYTE    buffer[64];
			WORD    buflen = sizeof(buffer);

			buffer[0] = SPROX_TCL_FUNC_ATTRIB;
			buffer[1] = cid;
			rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_TCL_FUNC, buffer, 2, buffer, &buflen);

			if ((rc == -93) && (first_time))
			{
				/* CID is active... API is nice enough to overcome this "user error" silently */
				rc = SPROX_API_CALL(TclB_Deselect) (SPROX_PARAM_P  cid);
				if (rc != MI_OK)
					return rc;

				first_time = FALSE;
				goto again;
			}

			if (rc != MI_OK)
				return RC_TCL(rc);

		}
		else
		{
			/* Function is provided by software */
			WORD    send_len, recv_len;
			BYTE    recv_data[64];
			BYTE    send_data[256];

			send_data[0] = 0x1D;
			memcpy(&send_data[1], pupi, 4);
			send_data[5] = 0x00;        /* default ISO 14443-4 parameters */
			send_data[6] = 0x05;        /* 106kbit/s, max frame size set to 64 */
			send_data[7] = 0x01;        /* going to ISO 14443-4 (T=CL) layer */
			send_data[8] = 0x00;        /* CID here */
			if (cid < TCL_CID_COUNT)
				send_data[8] |= (cid & 0x0F);
			send_len = 9;

			recv_len = sizeof(recv_data);
			rc = TclB_LowExchange(SPROX_PARAM_P  send_data, send_len, recv_data, &recv_len, 0xFFFF);
		}

	return rc;
}
/**f* SpringProx.API/SPROX_TclB_Deselect
 *
 * NAME
 *   SPROX_TclB_Deselect
 *
 * DESCRIPTION
 *   Send the T=CL DESELECT command to an ISO 14443-B-4 card
 *
 * INPUTS
 *   BYTE cid           : CID of the card to DESELECT
 *                        (set to 0xFF if you don't use CIDs)
 *
 * RETURNS
 *   MI_OK              : success, card deselected
 *   Other code if internal or communication error has occured.
 *
 * SEE ALSO
 *   SPROX_TclB_Attrib
 *   SPROX_B_Halt
 *
 **/
SPROX_API_FUNC(TclB_Deselect) (SPROX_PARAM  BYTE cid)
{
	SWORD   rc;
	SPROX_PARAM_TO_CTX;

	if (sprox_ctx->sprox_version >= 0x00013700)
	{
		/* Version >= 1.37 : Function is inside the reader */
		BYTE    buffer[2];
		SWORD   rc;

		buffer[0] = SPROX_TCL_FUNC_DESELECT;
		buffer[1] = cid;

		rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_TCL_FUNC, buffer, 2, NULL, NULL);
		rc = RC_TCL(rc);

	}
	else
	{
		BYTE    recv_data[32];
		BYTE    send_data[1];
		WORD    recv_len;

		send_data[0] = PCB_MK_S_BLOCK | PCB_S_DESELECT;

		recv_len = sizeof(recv_data);
		rc = TclB_LowExchange(SPROX_PARAM_P  send_data, sizeof(send_data), recv_data, &recv_len, 1000);
		if (rc)
			return rc;
	}

	return rc;
}

/**f* SpringProx.API/SPROX_TclB_Halt
 *
 * NAME
 *   SPROX_TclB_Halt
 *
 * WARNING
 *   This function is deprecated. Use SPROX_B_Halt instead.
 *
 * NOTES
 *   If card is already in T=CL mode, you must DESELECT the card instead of
 *   HALTing it.
 *
 * SEE ALSO
 *   SPROX_B_Halt
 *   SPROX_TclB_Deselect
 *
 **/
SPROX_API_FUNC(TclB_Halt) (SPROX_PARAM  const BYTE pupi[4])
{
	return SPROX_API_CALL(B_Halt) (SPROX_PARAM_P  pupi);
}

/**f* SpringProx.API/SPROX_TclB_Exchange
 *
 * NAME
 *   SPROX_TclB_Exchange
 *
 * DESCRIPTION
 *   Perform a T=CL block exchange according to ISO 14443-B-4
 *
 * INPUTS
 *   BYTE fsci                : FSCI parameter according to ISO 14443-B
 *                              (set to 0xFF for default card value)
 *   BYTE cid                 : CID of the card
 *                              (set to 0xFF if you don't use CIDs)
 *   BYTE nad                 : NAD of the card
 *                              (set to 0xFF if you don't use NADs)
 *   const BYTE send_buffer[] : buffer to sens to the card
 *   WORD send_len            : length of send_buffer (max 256)
 *   BYTE recv_buffer[]       : buffer for card's answer
 *   WORD *recv_len           : input  : size of recv_buffer
 *                              output : actual length of reply
 *
 * RETURNS
 *   MI_OK                    : success
 *   Other code if internal or communication error has occured.
 *
 * NOTES
 *   I-Block chaining is automatically performed by the reader according to the
 *   FSCI parameter.
 *   Wait Time eXtension S-Block coming from the card are also handled directly
 *   by the reader.
 *
 * SEE ALSO
 *   SPROX_TclB_Attrib
 *   SPROX_TclB_Deselect
 *
 **/
SPROX_API_FUNC(TclB_Exchange) (SPROX_PARAM  BYTE fsci, BYTE cid, BYTE nad, const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD* recv_len)
{
	SPROX_PARAM_TO_CTX;

	if (sprox_ctx->sprox_version >= 0x00013700)
	{
		/* Version >= 1.37 : Function is inside the reader */
		BYTE   buffer[256 + 4];
		SWORD  rc;

		/* Check parameters */
		if ((send_buffer == NULL) || (send_len > 256) || (recv_buffer == NULL) || (recv_len == NULL)) return MI_LIB_CALL_ERROR;

		if ((cid == TCL_UNUSED_CID) && (nad == TCL_UNUSED_NAD))
		{
			if (fsci == 0xFF)
			{
				buffer[0] = SPROX_TCL_FUNC_EXCH;
				memcpy(&buffer[1], send_buffer, send_len);
				send_len += 1;
			}
			else
			{
				buffer[0] = SPROX_TCL_FUNC_EXCH_FSC;
				buffer[1] = fsci;
				memcpy(&buffer[2], send_buffer, send_len);
				send_len += 2;
			}
		}
		else if (nad == TCL_UNUSED_NAD)
		{
			buffer[0] = SPROX_TCL_FUNC_EXCH_FSC_CID;
			buffer[1] = fsci;
			buffer[2] = cid;
			memcpy(&buffer[3], send_buffer, send_len);
			send_len += 3;
		}
		else
		{
			buffer[0] = SPROX_TCL_FUNC_EXCH_FSC_CID_NAD;
			buffer[1] = fsci;
			buffer[2] = cid;
			buffer[3] = nad;
			memcpy(&buffer[4], send_buffer, send_len);
			send_len += 4;
		}

		rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_TCL_FUNC, buffer, send_len, recv_buffer, recv_len);
		if (rc == MI_OK)
			return MI_OK;

		/* Error in exchange, PICC has been deselected */
		return RC_TCL(rc);

	}
	else
	{
		return Tcl_HalfDuplex(SPROX_PARAM_P  TRUE, fsci, cid, nad, &glob_block_num, send_buffer, send_len, recv_buffer, recv_len);
	}
}


/*
 *****************************************************************************
 *
 *                             COMMON PART
 *
 *****************************************************************************
 */

 /**f* SpringProx.API/SPROX_Tcl_Exchange
  *
  * NAME
  *   SPROX_Tcl_Exchange
  *
  * DESCRIPTION
  *   Perform a T=CL block exchange according to ISO 14443-4 (without NAD, and
  *   with card's default FSCI)
  *
  * INPUTS
  *   BYTE cid                 : CID of the card
  *                              (set to 0xFF if you don't use CIDs)
  *   const BYTE send_buffer[] : buffer to sens to the card
  *   WORD send_len            : length of send_buffer (max 256)
  *   BYTE recv_buffer[]       : buffer for card's answer
  *   WORD *recv_len           : input  : size of recv_buffer
  *                              output : actual length of reply
  *
  * RETURNS
  *   MI_OK                    : success
  *   Other code if internal or communication error has occured.
  *
  * NOTES
  *   This function calls either SPROX_TclA_Exchange or SPROX_TclB_Exchange
  *   with appropriate parameters depending on CID related informations
  *
  * SEE ALSO
  *   SPROX_TclA_Exchange
  *   SPROX_TclB_Exchange
  *
  **/
SPROX_API_FUNC(Tcl_Exchange) (SPROX_PARAM  BYTE cid, const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD* recv_len)
{
	BYTE    buffer[261 + 1]; /* New 1.71 - used to be 256+1 */
	SWORD   rc;

	/* Check parameters */
	if (((send_buffer == NULL) && (send_len > 0)) || (send_len > (sizeof(buffer) - 1)) || (recv_buffer == NULL) || (recv_len == NULL)) return MI_LIB_CALL_ERROR;

	/* Build send buffer */
	buffer[0] = cid;
	memcpy(&buffer[1], send_buffer, send_len);
	send_len += 1;

	rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_TCL_EXCHANGE, buffer, send_len, recv_buffer, recv_len);
	if (rc == MI_OK)
		return MI_OK;

	/* Error in exchange, PICC has been deselected */
	return RC_TCL(rc);
}

/**f* SpringProx.API/SPROX_Tcl_Deselect
 *
 * NAME
 *   SPROX_Tcl_Deselect
 *
 * DESCRIPTION
 *   Send the T=CL DESELECT command to an ISO 14443-4 card
 *
 * INPUTS
 *   BYTE cid                 : CID of the card
 *                              (set to 0xFF if you don't use CIDs)
 *
 * RETURNS
 *   MI_OK                    : success
 *   Other code if internal or communication error has occured.
 *
 * SEE ALSO
 *   SPROX_TclA_Deselect
 *   SPROX_TclB_Deselect
 *
 **/
SPROX_API_FUNC(Tcl_Deselect) (SPROX_PARAM  BYTE cid)
{
	BYTE    buffer[2];
	SWORD   rc;

	SPROX_PARAM_TO_CTX;
	UNUSED_PARAMETER(sprox_ctx);

	buffer[0] = SPROX_TCL_FUNC_DESELECT;
	buffer[1] = cid;

	rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_TCL_FUNC, buffer, 2, NULL, NULL);
	return RC_TCL(rc);
}

/*
 *****************************************************************************
 *
 *                      LOW LEVEL T=CL COMMON FUNCTIONS
 *
 *****************************************************************************
 */

static WORD Tcl_Make_I_Block(BYTE* frame, WORD* length, BYTE block_num, BYTE cid, BYTE nad, BOOL first_block, WORD block_size, const BYTE* data,
	WORD datalen)
{
	WORD    i;

	/* Build block header */
	frame[0] = PCB_MK_I_BLOCK;
	*length = 1;

	/* Append CID if required */
	if (cid < TCL_CID_COUNT)
	{
		frame[0] |= PCB_CID_FOLLOWING;
		frame[*length] = cid;
		*length = 1 + *length;
	}
	/* Append NAD on the first frame if required */
	if ((first_block) && (nad != TCL_UNUSED_NAD))
	{
		frame[0] |= PCB_NAD_FOLLOWING;
		frame[*length] = nad;
		*length = 1 + *length;
	}

	/* Enqueue data... */
	for (i = 0; i < datalen; i++)
	{
		frame[*length] = data[i];
		*length = 1 + *length;
		if (*length >= block_size)
			break;
	}

	/* Add chaining bit if we didn't sent everything */
	if (i < datalen)
		frame[0] |= PCB_CHAINING;

	/* Append block number */
	if (block_num)
		frame[0] |= 0x01;

	return i;
}

static void Tcl_Make_R_Block(BYTE* frame, WORD* length, BYTE block_num, BYTE cid, BOOL ack)
{
	/* Build block header */
	frame[0] = PCB_MK_R_BLOCK;
	*length = 1;
	/* Append CID if required */
	if (cid < TCL_CID_COUNT)
	{
		frame[0] |= PCB_CID_FOLLOWING;
		frame[*length] = cid;
		*length = 1 + *length;
	}

	if (ack)
	{
		frame[0] |= PCB_R_ACK;
	}
	else
	{
		frame[0] |= PCB_R_NAK;
	}

	/* Append block number */
	if (block_num)
		frame[0] |= 0x01;

}

static void Tcl_Make_S_Block(BYTE* frame, WORD* length, BYTE block_num, BYTE rcv0, BYTE rcv1, BYTE rcv2)
{
	(void)block_num;
	/* Build block header */
	frame[0] = rcv0;
	if (rcv0 & PCB_CID_FOLLOWING)
	{
		frame[1] = rcv1;
		frame[2] = rcv2 & 0x3F;
		*length = 3;
	}
	else
	{
		frame[1] = rcv1 & 0x3F;
		*length = 2;
	}
}

static SWORD Tcl_HalfDuplex(SPROX_PARAM BOOL type_b, BYTE fsci, BYTE cid, BYTE nad, BYTE* block_num, const BYTE* send_buffer, WORD send_len, BYTE* recv_buffer,
	WORD* recv_len)
{
	SWORD   rc;
	DWORD   fwt_card, fwt_current;

	WORD    i;
	WORD    block_size;

	BYTE    retry;

	BYTE    type;                 /* Block type */
	BOOL    flag;                 /* Temporary boolean value */

	WORD    recv_offset;
	WORD    send_offset, sent_length;

	BOOL    pcd_say_ack;
	BOOL    picc_was_chaining;

	BYTE    x_buffer[260];
	WORD    x_s_length;
	WORD    x_r_length;

	/* Warning, CID must be shortened */
	if (cid > TCL_CID_COUNT)
		cid = TCL_CID_COUNT;

	/* Retrieve actual FSCI */
	if (fsci > TCL_FSCI_MAX)
		fsci = TCL_FSCI_MAX;

	/* Compute block_size against FSCI                  */
	/* !! 2 bytes are reserved for CRC in each frame !! */
	if (fsci <= 4)
	{
		/* 0->16 , 1->24 , 2->32 , 3->40 , 4->48 */
		block_size = 14 + 8 * fsci;
	}
	else if (fsci <= 7)
	{
		/* 5->64 , 6->96 , 7->128 */
		block_size = 62 + 32 * (fsci - 5);
	}
	else
	{
		/* 8->256 , others = RFU */
		block_size = 254;
	}

#if 0
	/* TODO                                                    */
	/* Retrieve also FWT (number of ETU before card's timeout) */
	/* (We accept a 125% delay to ease the process)            */
#else
	fwt_card = 10 * TCL_LONG_TIMEOUT;
#endif

	/* Prepare send buffer */
	send_offset = 0;

	/* Prepare receive buffer */
	recv_offset = 0;
	*recv_len = 0;

	picc_was_chaining = FALSE;
	pcd_say_ack = FALSE;

	retry = TCL_MAX_RETRY;
	fwt_current = fwt_card;
	for (;;)
	{
		if (send_offset < send_len)
		{
			/* PCD sends an I-Block */
			/* -------------------- */

			flag = !send_offset;
			sent_length = Tcl_Make_I_Block(x_buffer, &x_s_length, *block_num, cid, nad, flag, block_size, &send_buffer[send_offset], (WORD)(send_len - send_offset));

		}
		else
		{
			/* PCD sends an R-Block */
			/* -------------------- */

			Tcl_Make_R_Block(x_buffer, &x_s_length, *block_num, cid, pcd_say_ack);
			sent_length = 0;

		}

		for (;;)
		{
			/* Perform the exchange */
			/* -------------------- */

			type = 0;
			x_r_length = sizeof(x_buffer);
			if (type_b)
			{
				rc = TclB_LowExchange(SPROX_PARAM_P  x_buffer, x_s_length, x_buffer, &x_r_length, fwt_current);
			}
			else
			{
				rc = TclA_LowExchange(SPROX_PARAM_P  x_buffer, x_s_length, x_buffer, &x_r_length, fwt_current);
			}
			fwt_current = fwt_card;

			if (rc != MI_OK)
			{
				if (rc == MI_ACCESSTIMEOUT)
				{
					/* No response from PICC */
					return MI_NOTAGERR;
				}

				if (!retry)
				{
					/* No more retry allowed */
					return rc;
				}
				retry--;
				if (picc_was_chaining)
				{
					/* 7.5.4.2 5 : in the case of PICC chaining, when an invalid block is received or a FWT time-out */
					/*             occurs, an R(ACK) block shall be sent                                             */
					pcd_say_ack = TRUE;
				}
				else
				{
					/* 7.5.4.2 4 : when an invalid block is received or a FWT time-out occurs, an R(NAK) block shall */
					/*             sent (except in the case of PICC chaining or S(DESELECT))                         */
					pcd_say_ack = FALSE;
				}

				break;
			}

			/* OK -> retry counter up to the max */
			retry = TCL_MAX_RETRY;

			/* Block type ? */
			type = x_buffer[0] & PCB_BLOCK_MASK;

			if (type == PCB_IS_S_BLOCK)
			{
				/* This is an S-BLOCK */
				/* ------------------ */

				if ((x_buffer[0] & PCB_S_WTX) == PCB_S_WTX)
				{
					/* S(WTX) : More time needed */
					/* ------------------------- */

					/* Compute next FWT according to ISO */
					fwt_current = fwt_card;
					if (x_buffer[0] & PCB_CID_FOLLOWING)
						fwt_current *= (x_buffer[2] & 0x3F);
					else
						fwt_current *= (x_buffer[1] & 0x3F);

					/* Tell host we need some more time */

					Tcl_Make_S_Block(x_buffer, &x_s_length, *block_num, x_buffer[0], x_buffer[1], x_buffer[2]);
					continue;
				}

				/* Invalid S-BLOCK */
				return MI_TCL_PROTOCOL;
			}

			/* Else : R-BLOCK or I-BLOCK, done with this loop */
			break;
		}

		/* Error condition */
		if (rc != MI_OK)
			continue;

		/* Same block number ? */
		flag = !(*block_num ^ (x_buffer[0] & 0x01));

		if (flag)
		{
			/* 7.5.3.1 B : when an I-block or an R(ACK) block with a block number equal to the current   */
			/*             block number is received, the PCD shall toggle the current block number for   */
			/*             that PICC                                                                     */

			/* Note : an R(NAK) block is never sent by the PICC */

			*block_num = *block_num ^ 0x01;

			/* 7.5.4.2 7 : When an R(ACK) block is received, if its block number is equal to the PCD's   */
			/*             current block number, shaining shall be continued                             */

			/* 7.5.4.2 6 : When an R(ACK) block is received, if its block number is not equal to the     */
			/*             PCD's current block number, the last I-Block shall be retransmitted           */

			send_offset += sent_length;
			sent_length = 0;
		}

		if (type == PCB_IS_R_BLOCK)
		{
			/* This is an R-BLOCK */
			/* ------------------ */

			if (send_offset >= send_len)
			{
				/* We are not in PCD chaining, why did the PICC send an R-block here ??? */
				return MI_TCL_PROTOCOL;
			}
			continue;
		}

		if (type == PCB_IS_I_BLOCK)
		{
			/* This is an I-BLOCK */
			/* ------------------ */

			if (send_offset < send_len)
			{
				/* We were still in PCD chaining, why did the PICC send an I-block here ??? */
				return MI_TCL_PROTOCOL;
			}

			picc_was_chaining = (x_buffer[0] & PCB_CHAINING);

			if (!flag)
			{
				/* 7.5.4.2 4 : when an invalid block is received or a FWT time-out occurs, an R(NAK) block shall */
				/*             sent (except in the case of PICC chaining or S(DESELECT))                         */
				pcd_say_ack = picc_was_chaining;
				continue;
			}

			/* Drop header before copying incoming data */
			i = 1;
			if (x_buffer[0] & PCB_CID_FOLLOWING)
				i++;
			if (x_buffer[0] & PCB_NAD_FOLLOWING)
				i++;

			for (; i < x_r_length; i++)
			{
				if (recv_offset >= MAX_RECV_LEN)
					rc = MI_TCL_PROTOCOL;
				else
					recv_buffer[recv_offset++] = x_buffer[i];
			}

			if (picc_was_chaining)
			{
				/* PICC is still chaining */
				/* ---------------------- */

				/* 7.5.4.1 2 : When an I-Block indicating chaining is received, the block shall be acknow- */
				/*             ledged by an R(ACK) block                                                   */
				pcd_say_ack = TRUE;
				continue;
			}

			/* No more data */
			/* ------------ */
			*recv_len = recv_offset;
			return rc;
		}

		/* Invalid block received ??? */
		return MI_TCL_PROTOCOL;
	}
}

/*
 *****************************************************************************
 *
 *                             HARDWARE LEVEL PART
 *
 *****************************************************************************
 */

static SWORD TclA_LowExchange(SPROX_PARAM  BYTE* send_data, WORD send_len, BYTE* recv_data, WORD* recv_len, DWORD timeout)
{
	BYTE    buffer[254];
	WORD    length = sizeof(buffer);
	SWORD   rc;

	memcpy(buffer, send_data, send_len);

	rc = SPROX_API_CALL(A_Exchange) (SPROX_PARAM_P  buffer, (WORD)(2 + send_len), buffer, &length, TRUE, (WORD)timeout);

	if (rc)
	{
		return rc;
	}

	if (length >= 2)
		length = length - 2;
	else
		length = 0;

	if (recv_len != NULL)
	{
		if ((*recv_len != 0) && (*recv_len < length))
			return MI_RESPONSE_OVERFLOW;
		*recv_len = length;
	}
	if (recv_data != NULL)
		memcpy(recv_data, buffer, length);

	return MI_OK;
}

static SWORD TclB_LowExchange(SPROX_PARAM  BYTE* send_data, WORD send_len, BYTE* recv_data, WORD* recv_len, DWORD timeout)
{
	BYTE    buffer[254];
	WORD    length = sizeof(buffer);
	SWORD   rc;

	memcpy(buffer, send_data, send_len);
	rc = SPROX_API_CALL(B_Exchange) (SPROX_PARAM_P  buffer, (WORD)(2 + send_len), buffer, &length, TRUE, (WORD)timeout);
	if (rc)
	{
		return rc;
	}

	if (length >= 2)
		length = length - 2;
	else
		length = 0;

	if (recv_len != NULL)
	{
		if ((*recv_len != 0) && (*recv_len < length))
			return MI_RESPONSE_OVERFLOW;
		*recv_len = length;
	}
	if (recv_data != NULL)
		memcpy(recv_data, buffer, length);

	return MI_OK;
}

#endif
