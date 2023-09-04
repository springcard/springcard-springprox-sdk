/**** SpringProx.API/Dialog
 *
 * NAME
 *   SpringProx.API :: Dialog
 *
 * DESCRIPTION
 *   Provides dialog between host and reader
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

   JDA 15/02/2003 : created from CSB-3's osi3964.c
   JDA 16/05/2003 : added GetFirmware and GetRc500Id
   JDA 15/09/2003 : added "burst" mode for USB devices
   JDA 07/04/2004 : moved low-layers to related sprox_dlg_xxx files
   JDA 17/06/2004 : support of multiple readers thanks to SPROX_CreateInstance, SPROX_SelectInstance,
					and SPROX_DestroyInstance
   JDA 24/06/2006 : now really reentrant under new springprox_ex.dll
   JDA 28/02/2012 : improved timeout handling
					forget current protocol every time the reader is likely to have resetted
   JDA 16/07/2014 : added support for TCP C/S protocol

 */

#include "sprox_api_i.h"

#include <assert.h>

#ifndef UNDER_CE	
#include <time.h>
#endif

extern WORD RESPONSE_TMO_BIN;

TCHAR DefaultDeviceName[32] = { 0 };
DWORD DefaultSettingsForced = 0;
DWORD DefaultSettingsAllowed = COM_BAUDRATE_38400 | COM_BAUDRATE_115200 | COM_PROTO_OSI3964 | COM_PROTO_ASCII | COM_PROTO_BIN;

//#undef D
//#define D(x) x

#define SPROX_API_CAN_REPEAT

/*
 * Low layer protocol
 * ------------------
 */
void RecvFlush(SPROX_CTX_ST* sprox_ctx)
{
	BYTE    b;
	WORD    retry = 250;

	SPROX_Trace(TRACE_DLG_HI, "<flush>");

	SerialSetTimeouts(sprox_ctx, 0, INTER_BYTE_TMO);
	retry = 250;
	while ((retry--) && (RecvByte(sprox_ctx, &b)));

	SPROX_Trace(TRACE_DLG_HI, "</flush>");
}

/*
 * High layer protocol : send a frame
 * ----------------------------------
 */
SWORD SendFrame(SPROX_CTX_ST* sprox_ctx, const BYTE* buffer, WORD buflen)
{
#ifdef SPROX_API_ONLY_BIN
	/* Binary protocol only */
	return SendFrameBIN(sprox_ctx, buffer, buflen);
#else
#ifdef SPROX_API_ONLY_ASC
	/* ASCII protocol only */
	return SendFrameASC(sprox_ctx, buffer, buflen);
#else
	/* Protocol depends on reader's settings */
	switch (sprox_ctx->com_settings & COM_PROTO_MASK)
	{
	case COM_PROTO_BIN:
		return SendFrameBIN(sprox_ctx, buffer, buflen);
	case COM_PROTO_BUS:
		return SendFrameBUS(sprox_ctx, buffer, buflen);
	case COM_PROTO_ASCII:
		return SendFrameASC(sprox_ctx, buffer, buflen);
	case COM_PROTO_OSI3964:
	default:
		return SendFrameOSI(sprox_ctx, buffer, buflen);
	}
#endif
#endif
}

/*
 * High layer protocol : receive a frame
 * -------------------------------------
 */
SWORD RecvFrame(SPROX_CTX_ST* sprox_ctx, BYTE* buffer, WORD* buflen)
{
#ifdef SPROX_API_ONLY_BIN
	/* Binary protocol only */
	return RecvFrameBIN(sprox_ctx, buffer, buflen);
#else
#ifdef SPROX_API_ONLY_ASC
	/* ASCII protocol only */
	return RecvFrameASC(sprox_ctx, buffer, buflen);
#else
	/* Protocol depends on reader's settings */
	switch (sprox_ctx->com_settings & COM_PROTO_MASK)
	{
	case COM_PROTO_BIN:
		return RecvFrameBIN(sprox_ctx, buffer, buflen);
	case COM_PROTO_BUS:
		return RecvFrameBUS(sprox_ctx, buffer, buflen);
	case COM_PROTO_ASCII:
		return RecvFrameASC(sprox_ctx, buffer, buflen);
	case COM_PROTO_OSI3964:
	default:
		return RecvFrameOSI(sprox_ctx, buffer, buflen);
	}
#endif
#endif
}

/*
 * Try to establish dialog with the reader at specified baudrate
 * -------------------------------------------------------------
 */
SWORD SPROX_ReaderConnectAt(SPROX_CTX_ST* sprox_ctx, DWORD baudrate)
{
	SWORD   rc;

	/* Set the default communication settings */
	/* -------------------------------------- */

	/* OSI, ASCII or Binary ? Binary is default */
	if (sprox_ctx->com_settings_allowed & COM_PROTO_BIN)
	{
		sprox_ctx->com_settings &= ~COM_PROTO_MASK;
		sprox_ctx->com_settings |= COM_PROTO_BIN;
	}
	else
		if (sprox_ctx->com_settings_allowed & COM_PROTO_BUS)
		{
			sprox_ctx->com_settings &= ~COM_PROTO_MASK;
			sprox_ctx->com_settings |= COM_PROTO_BUS;
		}
		else
			if (sprox_ctx->com_settings_allowed & COM_PROTO_OSI3964)
			{
				sprox_ctx->com_settings &= ~COM_PROTO_MASK;
				sprox_ctx->com_settings |= COM_PROTO_OSI3964;
			}
			else
				if (sprox_ctx->com_settings_allowed & COM_PROTO_ASCII)
				{
					sprox_ctx->com_settings &= ~COM_PROTO_MASK;
					sprox_ctx->com_settings |= COM_PROTO_ASCII;
				}
				else
				{
					/* No protocol at all? At least try binary */
					sprox_ctx->com_settings &= ~COM_PROTO_MASK;
					sprox_ctx->com_settings |= COM_PROTO_BIN;
				}

	/* Maybe we have some options on our interface? */
	if (sprox_ctx->com_settings_allowed & COM_OPTION_SERIAL_RS485)
	{
		sprox_ctx->com_settings |= COM_OPTION_SERIAL_RS485;
	}
	if (sprox_ctx->com_settings_allowed & COM_OPTION_SERIAL_POWER_ON_DTR)
	{
		sprox_ctx->com_settings |= COM_OPTION_SERIAL_POWER_ON_DTR;
	}
	if (sprox_ctx->com_settings_allowed & COM_OPTION_SERIAL_RESET_ON_RTS)
	{
		sprox_ctx->com_settings |= COM_OPTION_SERIAL_RESET_ON_RTS;
	}

	/* Maybe we have some restrictions due to the defines at compile time */
#ifdef SPROX_API_ONLY_BIN
	SPROX_Trace(TRACE_DLG_HI, "Only Binary protocol compiled");
	sprox_ctx->com_settings &= ~COM_PROTO_MASK;
	sprox_ctx->com_settings |= COM_PROTO_BIN;
#endif
#ifdef SPROX_API_ONLY_ASC
	SPROX_Trace(TRACE_DLG_HI, "Only ASCII protocol compiled");
	sprox_ctx->com_settings &= ~COM_PROTO_MASK;
	sprox_ctx->com_settings |= COM_PROTO_ASCII;
#endif

	/* Set the specified baudrate */
	/* -------------------------- */
	if (baudrate)
	{
		switch (baudrate)
		{
		case 38400:
			sprox_ctx->com_settings &= ~COM_BAUDRATE_MASK;
			sprox_ctx->com_settings |= COM_BAUDRATE_38400;
			break;
		case 115200:
			sprox_ctx->com_settings &= ~COM_BAUDRATE_MASK;
			sprox_ctx->com_settings |= COM_BAUDRATE_115200;
			break;
		}

		SPROX_Trace(TRACE_DLG_HI, "SetBaudrate to %d (settings=%08lX)", baudrate, sprox_ctx->com_settings);

		if (!SerialSetBaudrate(sprox_ctx, baudrate))
			return MI_SER_ACCESS_ERR;
	}

	SPROX_Trace(TRACE_DLG_HI, "Probing the reader, settings=%08lX", sprox_ctx->com_settings);

	/* Call GetFirmware to check if the reader answers */
	/* ----------------------------------------------- */
#ifdef SPROX_API_REENTRANT
	rc = SPROXx_ReaderGetFirmware(sprox_ctx, NULL, 0);
#else
	rc = SPROX_ReaderGetFirmware(NULL, 0);
#endif

	/* New 1.54 */
#ifndef SPROX_API_NO_FIND
	if (rc == MI_POLLING)
	{
#ifdef SPROX_API_REENTRANT
		SPROXx_FindWaitCancel(sprox_ctx);
		rc = SPROXx_ReaderGetFirmware(sprox_ctx, NULL, 0);
#else
		SPROX_FindWaitCancel();
		rc = SPROX_ReaderGetFirmware(NULL, 0);
#endif
	}
#endif

	if (rc != MI_OK)
	{
		SPROX_Trace(TRACE_DLG_HI, "GetFirmware failed, settings=%08lX", sprox_ctx->com_settings);
	}
	else
	{
		/* Confirm thanks with GetFeatures */
		/* ------------------------------- */
#ifdef SPROX_API_REENTRANT
		rc = SPROXx_ReaderGetFeatures(sprox_ctx, NULL);
#else
		rc = SPROX_ReaderGetFeatures(NULL);
#endif

		if (rc != MI_OK)
		{
			SPROX_Trace(TRACE_DLG_HI, "GetFeatures failed, settings=%08lX", sprox_ctx->com_settings);
		}
	}

#if (defined(WINCE) && (defined(SPROX_API_ONLY_BIN) || defined(SPROX_API_ONLY_ASC)))
	if (rc != MI_OK)
	{
		/* Call GetFirmware */
#ifdef SPROX_API_REENTRANT
		rc = SPROXx_ReaderGetFirmware(sprox_ctx, NULL, 0);
#else
		rc = SPROX_ReaderGetFirmware(NULL, 0);
#endif
		if (rc != MI_OK)
		{
			SPROX_Trace(TRACE_DLG_HI, "GetFirmware failed (second try), settings=%08lX", sprox_ctx->com_settings);
		}
		else
		{
			/* Call GetFeatures */
#ifdef SPROX_API_REENTRANT
			rc = SPROXx_ReaderGetFeatures(sprox_ctx, NULL);
#else
			rc = SPROX_ReaderGetFeatures(NULL);
#endif
			if (rc != MI_OK)
			{
				SPROX_Trace(TRACE_DLG_HI, "GetFeatures failed (second try), settings=%08lX", sprox_ctx->com_settings);
			}

		}
	}
#endif

#ifndef SPROX_API_ONLY_BIN
#ifndef SPROX_API_ONLY_ASC
	if ((rc != MI_OK) && (sprox_ctx->com_settings_allowed & COM_PROTO_OSI3964) && !(sprox_ctx->com_settings & COM_PROTO_OSI3964) && (baudrate == 38400))
	{
		/* Activation with protocol different from OSI has failed */
		/* We revert to OSI for legacy support of CSB3 family     */
		/* ------------------------------------------------------ */

		sprox_ctx->com_settings &= ~COM_PROTO_MASK;
		sprox_ctx->com_settings |= COM_PROTO_OSI3964;

		/* Call GetFirmware */
#ifdef SPROX_API_REENTRANT
		rc = SPROXx_ReaderGetFirmware(sprox_ctx, NULL, 0);
#else
		rc = SPROX_ReaderGetFirmware(NULL, 0);
#endif
		if (rc != MI_OK)
		{
			SPROX_Trace(TRACE_DLG_HI, "GetFirmware failed (OSI), settings=%08lX", sprox_ctx->com_settings);
		}
		else
		{
			/* Call GetFeatures */
#ifdef SPROX_API_REENTRANT
			rc = SPROXx_ReaderGetFeatures(sprox_ctx, NULL);
#else
			rc = SPROX_ReaderGetFeatures(NULL);
#endif
			if (rc != MI_OK)
			{
				SPROX_Trace(TRACE_DLG_HI, "GetFeatures failed (OSI), settings=%08lX", sprox_ctx->com_settings);
			}
		}

		if (rc == MI_OK)
		{
			if (sprox_ctx->sprox_capabilities & SPROX_WITH_BIN_PROTOCOL)
			{
				/* Reader is able to dialog in Binary (faster) */
				if (sprox_ctx->com_settings_allowed & COM_PROTO_BIN)
				{
					/* Go binary ! */
					SPROX_Trace(TRACE_DLG_HI, "Reader found using OSI3964 protocol, but switching to Binary");
					sprox_ctx->com_settings &= ~COM_PROTO_MASK;
					sprox_ctx->com_settings |= COM_PROTO_BIN;
				}
			}
		}
	}
#endif
#endif

	return rc;
}

/*
 * Try to establish dialog with the reader, trying both baudrates
 * --------------------------------------------------------------
 */
SWORD SPROX_ReaderConnect(SPROX_CTX_ST* sprox_ctx)
{
	BYTE    buffer[20] = { 0 };
	SWORD   rc = MI_SER_ACCESS_ERR;
	BOOL    just_reset = FALSE;
	BYTE    i;

	assert(sprox_ctx != NULL);

	SPROX_Trace(TRACE_DLG_HI, "Connect, current settings=%08lX, allowed=%08lX", sprox_ctx->com_settings, sprox_ctx->com_settings_allowed);

	sprox_ctx->com_status = COM_STATUS_CLOSED;

	/* If we actually have resetted the SpringProx, it shoul'd display is ID now */
	/* ------------------------------------------------------------------------- */
	if (sprox_ctx->com_settings & (COM_OPTION_POWER_AUTO | COM_OPTION_SERIAL_POWER_ON_DTR | COM_OPTION_SERIAL_RESET_ON_RTS))
	{
		/* Reader is supposed to be resetting, we expect a startup string */
		SerialSetBaudrate(sprox_ctx, 38400);

		/* We shall wait up to 2s for reader's reset */
		SerialSetTimeouts(sprox_ctx, 2000, 100);

		SPROX_Trace(TRACE_DLG_HI, "Waiting for reader startup string");

		memset(buffer, 0, sizeof(buffer));

		if (RecvBurst(sprox_ctx, buffer, sizeof(buffer)))
		{
			for (i = 0; i < sizeof(buffer) - 4; i++)
			{
				/* Known reader startup strings */
				if ((!strncmp((char*)&buffer[i], "SPX", 3))  /* SpringProx family */
					|| (!strncmp((char*)&buffer[i], "CSB", 3))  /* CSB family */
					|| (!strncmp((char*)&buffer[i], "531", 3))  /* K531 family */
					|| (!strncmp((char*)&buffer[i], "632", 3))  /* K632 family */
					|| (!strncmp((char*)&buffer[i], "663", 3))  /* K663/H663/E663 family */
					|| (!strncmp((char*)&buffer[i], "601", 3))) /* K601 family */
				{
					/* Reader is resetting, don't expect it to be at 38400 */
					just_reset = TRUE;
					SPROX_Trace(TRACE_DLG_HI, "Reader resetting : %c%c%c%c", buffer[i + 0], buffer[i + 1], buffer[i + 2], buffer[i + 3]);
					break;
				}
			}
		}
		else
		{
			SPROX_Trace(TRACE_DLG_HI, "(No startup string)");
		}

		if (just_reset)
		{
			/* Forget reader protocol and baudrate */
			sprox_ctx->com_settings &= ~COM_PROTO_MASK;
			sprox_ctx->com_settings &= ~COM_BAUDRATE_MASK;
		}
		else
		{
			/* Reader has not successfully resetted */
#ifdef UNDER_CE
	  /* This shoul'd be considered has a fatal error, since CF or SDIO devices MUST */
	  /* do this, but we are optimistic and continue anyway...                       */
#endif
		}
	}

#ifdef SPROX_HIGH_BAUDRATE
	/* Maybe is the reader already at 115200bps ? */
	/* ------------------------------------------ */
	if ((!just_reset) && (sprox_ctx->com_settings_allowed & COM_BAUDRATE_115200))
	{
		SPROX_Trace(TRACE_DLG_HI, "Looking for reader at 115200bps...");

		RESPONSE_TMO_BIN = 100;
		rc = SPROX_ReaderConnectAt(sprox_ctx, 115200);
		RESPONSE_TMO_BIN = RESPONSE_TMO;

		if (rc == MI_OK)
		{
			SPROX_Trace(TRACE_DLG_HI, "Reader found at 115200bps");
			goto success;
		}

		SPROX_Trace(TRACE_DLG_HI, "Connect at 115200bps failed, rc=%d", rc);
	}
#endif

	/* Try to connect to the reader, at 38400bps */
	/* ----------------------------------------- */
	if (sprox_ctx->com_settings_allowed & COM_BAUDRATE_38400)
	{
		SPROX_Trace(TRACE_DLG_HI, "Looking for reader at 38400bps...");

		SerialControl_Reset(sprox_ctx, FALSE);
		SerialControl_Reset(sprox_ctx, TRUE);

		rc = SPROX_ReaderConnectAt(sprox_ctx, 38400);
		if (rc == MI_OK)
		{
			/* Connected */
#ifdef SPROX_HIGH_BAUDRATE
			if ((sprox_ctx->sprox_capabilities & SPROX_WITH_BAUDRATE_115200) && (sprox_ctx->com_settings_allowed & COM_BAUDRATE_115200))
			{
				/* Going to 115200bps */
				BYTE    buffer[2];

				SPROX_Trace(TRACE_DLG_HI, "Reader found at 38400bps, going to 115200bps");

				buffer[0] = SPROX_CONTROL_BAUDRATE;
				buffer[1] = 115;
#ifdef SPROX_API_REENTRANT
				rc = SPROXx_Function(sprox_ctx, SPROX_CONTROL, buffer, sizeof(buffer), NULL, NULL);
#else
				rc = SPROX_Function(SPROX_CONTROL, buffer, sizeof(buffer), NULL, NULL);
#endif
				if (rc == MI_UNKNOWN_FUNCTION)
				{
					/* Reader doesn't allow 115200bps */
					goto success;
				}

				if ((rc == MI_OK) || (rc == MI_SER_CHECKSUM_ERR) || (rc == MI_SER_PROTO_ERR) || (rc == MI_SER_TIMEOUT_ERR))
				{
					/* Switch our UART to high-speed also */
					SPROX_Trace(TRACE_DLG_HI, "Reader accepted 115200");

					/* Try to establish contact again... (3 tries) */
					for (i = 0; i < 3; i++)
					{
						/* Give reader at least 25ms for UART reset */
						Sleep(25);

						rc = SPROX_ReaderConnectAt(sprox_ctx, 115200);
						if (rc == MI_OK)
						{
							SPROX_Trace(TRACE_DLG_HI, "Reader found at 115200bps");
							goto success;
						}

						Sleep(175);
					}

					SPROX_Trace(TRACE_DLG_HI, "Reader lost at 115200bps");
				}
				else
				{
					SPROX_Trace(TRACE_DLG_HI, "Reader said : rc=%d", rc);
				}
			}
			else
#endif
			{
				/* We must stay at 38400 */
				SPROX_Trace(TRACE_DLG_HI, "Reader found at 38400bps");
				goto success;
			}
		}
		else
		{
			SPROX_Trace(TRACE_DLG_HI, "Connect at 38400bps failed, rc=%d", rc);
		}
	}
#ifdef UNDER_CE
#ifdef SPROX_HIGH_BAUDRATE
	/* Maybe is the reader already at 115200bps ? */
	if (!sprox_ctx->want_settings.bdr_38400)
	{
		rc = SPROX_ReaderConnectAt(sprox_ctx, 115200);
		if (rc == MI_OK)
		{
			SPROX_Trace(TRACE_DLG_HI, "Reader found at 115200bps");
			goto success;
		}

		if (rc == MI_UNKNOWN_FUNCTION)
		{
			/* Reader doesn't allow 115200bps */
			goto success;
		}

		SPROX_Trace(TRACE_DLG_HI, "Connect at 115200bps failed, rc=%d", rc);
	}
#endif
#endif

	return rc;

success:
	sprox_ctx->com_status = COM_STATUS_OPEN_ACTIVE;
	return MI_OK;
}

/*
 * High layer protocol : half-duplex frame exchange
 * -------------------------------------------------
 */
static SWORD SPROX_Function_Send(SPROX_CTX_ST* sprox_ctx, BYTE command, const BYTE* send_data, WORD send_len)
{
	BYTE buffer[SPROX_FRAME_CONTENT_SIZE + 20];
	WORD pos, len, i;
	BYTE crc;
	SWORD rc;

	assert(sprox_ctx != NULL);

	/* Check parameters... */
	if (send_len >= SPROX_FRAME_CONTENT_SIZE)
	{
		SPROX_Trace(TRACE_DLG_HI, "MI_COMMAND_OVERFLOW (%d>=%d)", send_len, SPROX_FRAME_CONTENT_SIZE);
		return MI_COMMAND_OVERFLOW;
	}

	if (send_data == NULL) send_len = 0;

	/* Create frame header */
	pos = 0;
	buffer[pos++] = sprox_ctx->com_sequence;
	buffer[pos++] = command;
	len = send_len;
	while (len >= 0x80)
	{
		buffer[pos++] = 0x80;
		len -= 0x80;
	}
	buffer[pos++] = (BYTE)len;

	/* Add frame data */
	if (send_data != NULL)
	{
		for (i = 0; i < send_len; i++)
			buffer[pos++] = send_data[i];
	}

	/* Append CRC at the end of the frame */
	crc = 0;
	for (i = 0; i < pos; i++)
		crc ^= buffer[i];
	buffer[pos++] = crc;

	/* Send the frame */
	rc = SendFrame(sprox_ctx, buffer, pos);
	if (rc != MI_OK)
	{
		SPROX_Trace(TRACE_DLG_HI, "Send %02X failed : %d", command, rc);
		return rc;
	}

	return MI_OK;
}

static SWORD SPROX_Function_Recv(SPROX_CTX_ST* sprox_ctx, BYTE* sequence, BYTE* recv_data, WORD* recv_len)
{
	BYTE buffer[SPROX_FRAME_CONTENT_SIZE + 20];
	WORD pos, len, i;
	BYTE crc;
	SWORD rc;

	assert(sprox_ctx != NULL);

time_extension:

	pos = sizeof(buffer);
	rc = RecvFrame(sprox_ctx, buffer, &pos);
	if (rc != MI_OK)
	{
		if (rc == MI_TIME_EXTENSION)
		{
			SPROX_Trace(TRACE_DLG_HI, "<TIME_EXTENSION>");
			goto time_extension;
		}
		SPROX_Trace(TRACE_DLG_HI, "Recv failed : %d", rc);
		return rc;
	}

	/* Check the CRC in the answer */
	if (!(sprox_ctx->com_settings & COM_PROTO_ASCII))
	{
		/* No CRC in ASCII protocol */
		crc = 0;
		for (i = 0; i < pos; i++)
			crc ^= buffer[i];

		if (crc)
		{
			/* CRC is KO */
			SPROX_Trace(TRACE_DLG_HI, "MI_SER_CHECKSUM_ERR");
			return MI_SER_CHECKSUM_ERR;
		}
	}

	/* Decode received header */
	pos = 0;
	*sequence = buffer[pos++];
	rc = 0 - buffer[pos++];
	len = 0;
	while (buffer[pos] >= 0x80)
		len += buffer[pos++];
	len += buffer[pos++];

	/* Time extension ? (new 1.20) */
	if (buffer[1] == (0 - MI_TIME_EXTENSION))
	{
		SPROX_Trace(TRACE_DLG_HI, "<TIME_EXTENSION>");
		goto time_extension;
	}

	/* Decode received data */
	if ((recv_data != NULL) && (recv_len != NULL))
	{
		if (len > *recv_len)
		{
			SPROX_Trace(TRACE_DLG_HI, "MI_RESPONSE_OVERFLOW (%d>%d)", len, *recv_len);
			return MI_RESPONSE_OVERFLOW;
		}

		for (i = 0; i < len; i++)
			recv_data[i] = buffer[pos++];

		*recv_len = len;
	}

	return rc;
}


SPROX_API_FUNC(Function) (SPROX_PARAM  BYTE command, const BYTE* send_data, WORD send_len, BYTE* recv_data, WORD* recv_len)
{
	SWORD rc, first_rc = MI_OK;
	BYTE  recv_sequence;
	BYTE  retry = 3;
	WORD  first_recv_len = 0;

	SPROX_PARAM_TO_CTX;

#ifdef SPROX_API_WITH_TCP
	if ((sprox_ctx->com_settings & COM_INTERFACE_MASK) == COM_INTERFACE_TCP)
	{
		return SPROX_TCP_Function(sprox_ctx, command, send_data, send_len, recv_data, recv_len);
	}
#endif

#ifdef SPROX_API_WITH_BRCD
	if (sprox_ctx->settings.brcd)
	{
		return SPROX_Brcd_Function(command, send_data, send_len, recv_data, recv_len);
	}
#endif

	if (recv_len != NULL) first_recv_len = *recv_len;

	/* Send the command */
send_command:
	rc = SPROX_Function_Send(sprox_ctx, command, send_data, send_len);
	if (rc != MI_OK) return rc;

	/* Retrieve the answer */
recv_answer:
	rc = SPROX_Function_Recv(sprox_ctx, &recv_sequence, recv_data, recv_len);

	if (rc != MI_OK)
	{
		/* Error in receive function */
		if (first_rc == MI_OK)
			first_rc = rc; /* Remember first error code */

		if ((sprox_ctx->com_status >= COM_STATUS_OPEN_ACTIVE) && retry)
		{
			retry--;

			if ((rc == MI_SER_CHECKSUM_ERR)
				|| (rc == MI_SER_PROTO_ERR)
				|| (rc == MI_SER_TIMEOUT_ERR)
				|| (rc == MI_SER_NORESP_ERR))
			{
				/* The reader may have resetted ? */
				sprox_ctx->pcd_current_rf_protocol = 0;

				/* This is an error due to communication layer, we ask the reader to repeat its last frame  */
				/* REPEAT_PLEASE command is only implemented in readers >= 1.40 with separate RX/TX buffer, */
				/* so most of the time we will receive a MI_UNKNOWN_FUNCTION error                          */
				if (sprox_ctx->sprox_capabilities & SPROX_WITH_DUAL_BUFFERS)
				{
					rc = SPROX_Function_Send(sprox_ctx, SPROX_REPEAT_PLEASE, NULL, 0);
					if (rc != MI_OK) return first_rc;
					if (recv_len != NULL) *recv_len = first_recv_len;
					goto recv_answer;
				}
			}
			if (rc == MI_SER_PROTO_NAK)
			{
				/* Reader says NACK, we send our last frame once again with a lot of hope... */
				if (recv_len != NULL) *recv_len = first_recv_len;
				goto send_command;
			}
		}

		/* Restore first error code */
		rc = first_rc;
	}

	/* No sequence in ASCII protocol */
	if (!(sprox_ctx->com_settings & COM_PROTO_ASCII))
	{
		if (recv_sequence != sprox_ctx->com_sequence)
		{
			/* Wrong sequence number */
			SPROX_Trace(TRACE_DLG_HI, "SEQ %02X != %02X", sprox_ctx->com_sequence, recv_sequence);
			if ((sprox_ctx->com_status >= COM_STATUS_OPEN_ACTIVE) && retry)
			{
				/* If we just sent the REPEAT_PLEASE command and we receive a wrong sequence number, it is    */
				/* likely that reader has totally lost our last frame. We send it again with a lot of hope... */
				retry--;
				if (recv_len != NULL) *recv_len = first_recv_len;
				goto send_command;
			}
			return MI_SER_PROTO_ERR;
		}
	}

	/* Next sequence */
	sprox_ctx->com_sequence++;

	/* Return the status code */
	return rc;
}

/* New 1.54 */
SPROX_API_FUNC(FunctionWaitResp) (SPROX_PARAM  BYTE* recv_data, WORD* recv_len, WORD timeout_s)
{
	SWORD rc;
	BYTE recv_sequence;
	time_t wait_until = 0;
	SPROX_PARAM_TO_CTX;

	if (timeout_s != 0xFFFF)
	{
#ifndef UNDER_CE	
		wait_until = time(NULL);
		wait_until += timeout_s + 1;
#else
		return MI_LIB_CALL_ERROR;
#endif
	}

	for (;;)
	{
		rc = SPROX_Function_Recv(sprox_ctx, &recv_sequence, recv_data, recv_len);

		if (rc != MI_SER_NORESP_ERR)
			break;

#ifndef UNDER_CE	
		if ((wait_until != 0) && (wait_until < time(NULL)))
			break;
#endif
	}

	return rc;
}

SPROX_API_FUNC(ReaderOpenAuto) (SPROX_PARAM_V)
{
	SPROX_PARAM_TO_CTX;

	SPROX_Trace(TRACE_ACCESS, "ReaderOpenAuto()");

	/* Cleanup */
	if (sprox_ctx->com_status >= COM_STATUS_OPEN_ACTIVE)
		return MI_OK;
	if (sprox_ctx->com_status > COM_STATUS_CLOSED_BUT_SEEN)
		SPROX_API_CALL(ReaderClose) (SPROX_PARAM_PV);

	LoadDefaultDevice();
	if (_tcslen(DefaultDeviceName) && _tcsicmp(DefaultDeviceName, _T("auto")))
	{
		/* A device name is specified in configuration area */
		return SPROX_API_CALL(ReaderOpen) (SPROX_PARAM_P  DefaultDeviceName);
	}

	/* Device name is "auto", let's force a lookup */
	if (!SerialLookup(sprox_ctx))
		return MI_SER_ACCESS_ERR;

	return MI_OK;
}

/**f* SpringProx.API/SPROX_ReaderOpen
 *
 * NAME
 *   SPROX_ReaderOpen
 *
 * DESCRIPTION
 *   Open the SpringProx reader
 *
 * INPUTS
 *   TCHAR device[]     : name of the device where is SpringProx reader is to be found
 *
 * RETURNS
 *   MI_OK              : success
 *   MI_SER_ACCESS_ERR  : reader not found
 *   Other code if internal or communication error has occured.
 *
 * NOTES
 *   For Desktop PC, device can be "COM1" to "COM9" or "USB". If device is set to NULL,
 *   an automatic detection of the reader is performed. On success, the selected device
 *   can be retrieved using SPROX_GetReaderDevice.
 *   New version 1.40 : support of multiple USB devices is now possible, using names
 *   "USB1" to "USB9". Earlier name "USB" remains as an alias of "USB1". Access to
 *   USB device through serial number is also possible with "USB:xxxxx" where xxxxx is
 *   the serial number. Use SPROX_EnumUSBDevices to build the list of available
 *   devices.
 *
 *   For Pocket PC, device must remain NULL. The CF module is implicitly powered up.
 *
 **/

SPROX_API_FUNC(ReaderOpen) (SPROX_PARAM  const TCHAR device[])
{
	SPROX_PARAM_TO_CTX;

	sprox_ctx->pcd_current_rf_protocol = 0;

	if ((device == NULL) || (!_tcslen(device)) || (!_tcsicmp(device, _T("auto"))))
	{
		return SPROX_API_CALL(ReaderOpenAuto) (SPROX_PARAM_PV);
	}

	SPROX_Trace(TRACE_ACCESS, "ReaderOpen(%s)", _ST(device));

	/* Same device as earlier? */
	if (_tcsicmp(device, sprox_ctx->com_name))
	{
		/* Not the same device as earlier, forget everything */
		if (sprox_ctx->com_status > COM_STATUS_CLOSED)
			SPROX_API_CALL(ReaderClose) (SPROX_PARAM_PV);
		sprox_ctx->com_status = COM_STATUS_CLOSED;
	}
	else
	{
		/* Yes, and the device is already open */
		if (sprox_ctx->com_status >= COM_STATUS_OPEN_ACTIVE)
			return MI_OK;
		/* Cleanup */
		if (sprox_ctx->com_status > COM_STATUS_CLOSED_BUT_SEEN)
			SPROX_API_CALL(ReaderClose) (SPROX_PARAM_PV);
	}

	/* Cleanup context */
	sprox_ctx->com_settings = DefaultSettingsForced;
	sprox_ctx->com_settings_allowed = DefaultSettingsAllowed;
	SPROX_Trace(TRACE_DLG_HI, "Set settings to %08lX, allowed=%08lX", sprox_ctx->com_settings, sprox_ctx->com_settings_allowed);

	/* Non-serial devices */
#ifdef SPROX_API_WITH_TCP
	if (!_tcsncicmp(device, _T("TCP:"), 4))
		return SPROX_TCP_ReaderOpen(sprox_ctx, &device[4]);
#endif

#ifdef SPROX_API_WITH_BRCD
	if (!_tcsncicmp(device, _T("BARACODA"), 8))
	{
		if (!_tcsncicmp(&device[8], _T("_DIRECT:"), 8))
			return SPROX_Brcd_ReaderOpen(&device[16], TRUE);

		return SPROX_Brcd_ReaderOpen(&device[9], FALSE);
	}
#endif

	/* Serial devices */
	if (!SerialOpen(sprox_ctx, device))
	{
		SPROX_Trace(TRACE_ACCESS, "ReaderOpen(%s) ERROR", _ST(device));
		return MI_SER_ACCESS_ERR;
	}

	/* Try to connect to the specified device */
	if (SPROX_ReaderConnect(sprox_ctx) != MI_OK)
	{
		SerialClose(sprox_ctx);
		SPROX_Trace(TRACE_ACCESS, "ReaderConnect(%s) ERROR", _ST(device));
		return MI_SER_NORESP_ERR;
	}

	SPROX_Trace(TRACE_ACCESS, "ReaderOpen OK");
	return MI_OK;
}

#ifdef WIN32
SPROX_API_FUNC(ReaderOpenW) (SPROX_PARAM  const wchar_t* device)
{
#ifdef UNICODE
	return SPROX_API_CALL(ReaderOpen) (SPROX_PARAM_P  device);
#else
	char buffer[64 + 1];
	int i;
	if (device == NULL) return SPROX_API_CALL(ReaderOpen) (SPROX_PARAM_P  NULL);
	for (i = 0; i < 64; i++)
	{
		buffer[i] = (char)device[i];
		if (buffer[i] == '\0') break;
	}
	buffer[64] = '\0';
	return SPROX_API_CALL(ReaderOpen) (SPROX_PARAM_P  buffer);
#endif
}
#ifndef UNDER_CE
SPROX_API_FUNC(ReaderOpenA) (SPROX_PARAM  const char* device)
{
#ifndef UNICODE
	return SPROX_API_CALL(ReaderOpen) (SPROX_PARAM_P  device);
#else
	wchar_t buffer[64 + 1];
	int i;
	if (device == NULL) return SPROX_API_CALL(ReaderOpen) (SPROX_PARAM_P  NULL);
	for (i = 0; i < 64; i++)
	{
		buffer[i] = device[i];
		if (buffer[i] == '\0') break;
	}
	buffer[64] = '\0';
	return SPROX_API_CALL(ReaderOpen) (SPROX_PARAM_P  buffer);
#endif
}
#endif
#endif

/**f* SpringProx.API/SPROX_ReaderClose
 *
 * NAME
 *   SPROX_ReaderClose
 *
 * DESCRIPTION
 *   Close the SpringProx reader
 *
 * RETURNS
 *   MI_OK              : success 7
 *
 * NOTES
 *   The RF field is halted before.
 *   On Pocket PC, the CF module is implicitly powered down.
 **/

SPROX_API_FUNC(ReaderClose) (SPROX_PARAM_V)
{
	SPROX_PARAM_TO_CTX;

	sprox_ctx->pcd_current_rf_protocol = 0;

	SPROX_Trace(TRACE_ACCESS, "ReaderClose");

	if (sprox_ctx->com_status > COM_STATUS_CLOSED_BUT_SEEN)
	{
#ifdef SPROX_API_WITH_TCP
		if ((sprox_ctx->com_settings & COM_INTERFACE_MASK) == COM_INTERFACE_TCP)
		{
			SPROX_TCP_ReaderClose(sprox_ctx);
		}
		else
#endif  
		{
			SerialClose(sprox_ctx);
		}
	}

	sprox_ctx->com_status = COM_STATUS_CLOSED_BUT_SEEN;
	return MI_OK;
}

/**f* SpringProx.API/SPROX_ReaderActivate
 *
 * NAME
 *   SPROX_ReaderActivate
 *
 * DESCRIPTION
 *   Powers up the SpringProx reader again after a successfull call to SPROX_ReaderDeactivate
 *
 * RETURNS
 *   MI_OK              : success
 *   MI_SER_ACCESS_ERR  : reader not found
 *   Other code if internal or communication error has occured.
 *
 * NOTES
 *   If this fails, call immediatly SPROX_ReaderOpen(NULL) to try again
 *
 **/

SPROX_API_FUNC(ReaderActivate) (SPROX_PARAM_V)
{
	SWORD rc;
	SPROX_PARAM_TO_CTX;

	sprox_ctx->pcd_current_rf_protocol = 0;

#ifdef SPROX_API_WITH_BRCD
	if (sprox_ctx->settings.brcd)
		sprox_ctx->sprox_status = 2;
#endif

	if (sprox_ctx->com_status >= COM_STATUS_OPEN_ACTIVE)
		return MI_OK;

	if (sprox_ctx->com_status < COM_STATUS_CLOSED_BUT_SEEN)
		return MI_SER_ACCESS_ERR;

	if (sprox_ctx->com_settings & COM_OPTION_POWER_AUTO)
	{
		/* Power up and connect again... */
		if (!SerialOpen(sprox_ctx, sprox_ctx->com_name))
		{
			sprox_ctx->com_status = COM_STATUS_CLOSED;
			return MI_SER_ACCESS_ERR;
		}
	}
	else
		if (sprox_ctx->com_settings & COM_OPTION_SERIAL_POWER_ON_DTR)
		{
			/* Power up and connect again... */
			if (!SerialPowerUp(sprox_ctx))
			{
				SerialClose(sprox_ctx);
				sprox_ctx->com_status = COM_STATUS_CLOSED;
				return MI_SER_ACCESS_ERR;
			}
		}

	/* After power up, reconnect the reader */
	if ((sprox_ctx->com_settings & COM_OPTION_POWER_AUTO) || (sprox_ctx->com_settings & COM_OPTION_SERIAL_POWER_ON_DTR))
	{
		if (SPROX_ReaderConnect(sprox_ctx) != MI_OK)
		{
			SPROX_Trace(TRACE_ACCESS, "ReaderConnect ERROR");
			SerialClose(sprox_ctx);
			sprox_ctx->com_status = COM_STATUS_CLOSED;
			return MI_SER_ACCESS_ERR;
		}
		goto ready;
	}

	/* Only enable RF field again */
	rc = SPROX_API_CALL(ControlRF) (SPROX_PARAM_P  TRUE);
	if ((rc != MI_OK) && (rc != MI_UNKNOWN_FUNCTION))
	{
		SPROX_Trace(TRACE_ACCESS, "ReaderControlRF ERROR");
		SerialClose(sprox_ctx);
		sprox_ctx->com_status = COM_STATUS_CLOSED;
		return MI_SER_ACCESS_ERR;
	}

ready:
	SPROX_Trace(TRACE_ACCESS, "ReaderActivate OK");
	sprox_ctx->com_status = COM_STATUS_OPEN_ACTIVE;
	return MI_OK;
}

/**f* SpringProx.API/SPROX_ReaderDeactivate
 *
 * NAME
 *   SPROX_ReaderDeactivate
 *
 * DESCRIPTION
 *   Powers down the SpringProx reader without closing the serial comm device
 *
 * RETURNS
 *   MI_OK              : success
 *   Other code if internal or communication error has occured.
 *
 * NOTES
 *   This function is highly dependend of the hardware platform. On Pocket PC
 *   devices it actually powers down the reader in order to reduce power consumption.
 *   On Desktop PC it does nothing.
 **/

SPROX_API_FUNC(ReaderDeactivate) (SPROX_PARAM_V)
{
	SPROX_PARAM_TO_CTX;

	sprox_ctx->pcd_current_rf_protocol = 0;

	SPROX_Trace(TRACE_ACCESS, "ReaderDeactivate");

	if (sprox_ctx->com_status <= COM_STATUS_CLOSED_BUT_SEEN)
		return MI_SER_ACCESS_ERR;

	if (sprox_ctx->com_status < COM_STATUS_OPEN_ACTIVE)
		return MI_OK;


#ifdef SPROX_API_WITH_BRCD
	if (sprox_ctx->settings.brcd)
		return MI_OK;
#endif

	if (sprox_ctx->com_settings & COM_OPTION_POWER_AUTO)
	{
		/* Closing is enough... */
		SerialClose(sprox_ctx);
		sprox_ctx->com_status = COM_STATUS_CLOSED_BUT_SEEN;
	}
	else
		if (sprox_ctx->com_settings & COM_OPTION_SERIAL_POWER_ON_DTR)
		{
			/* Powering down is enough... */
			SerialPowerDown(sprox_ctx);
			sprox_ctx->com_status = COM_STATUS_OPEN_IDLE;
		}
		else
		{
			/* Turn OFF RF field only */
			SPROX_API_CALL(ControlRF) (SPROX_PARAM_P  FALSE);
			sprox_ctx->com_status = COM_STATUS_OPEN_IDLE;
		}

	return MI_OK;
}
