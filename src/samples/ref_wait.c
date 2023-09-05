/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  Copyright (c) 2009-2014 PRO ACTIVE SAS, FRANCE - www.springcard.com

  ref_wait.c
  ----------

  This is the reference applications that shows how to ask the reader
  to wake us when a card arrives.

  LTX 25/08/2009 : initial release
  JDA 24/01/2012 : minor changes to adapt to release 1.62 of the SDK
  JDA 13/08/2014 : moved to Visual C++ Express 2010, added the 'A'
				   suffix to all text-related functions
  JDA 04/09/2023 : refreshed the project to build with Visual Studio 2022
*/
#include "products/springprox/api/springprox.h"

#ifdef WIN32
#include <conio.h>              /* For kbhit and getch */
#endif
#ifdef __linux
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


const char* PROGRAM_NAME = "ref_wait";
const char* szCommDevice = NULL;
WORD wFindProtos = 0;
BOOL fFindOnce = FALSE;

static BOOL parse_args(int argc, char** argv);
static void usage(void);

int main(int argc, char** argv)
{
	SWORD rc;
	int i;
	char s_buffer[64];
	BYTE uid[12];
	BYTE uid_len = 12;
	WORD proto = 0;

	/* Display the informations and check the command line */
	/* --------------------------------------------------- */

	printf("SpringCard SpringProx 'Legacy' SDK\n");
	printf("\n");
	printf("%s : multi-protocol card polling sample, using the WAIT (not FIND) function\n\n", PROGRAM_NAME);
	printf("This is a free and unsupported sample provided by www.springcard.com\n");
	printf("Please read LICENSE.txt for details\n");	
	printf("\n");

	if (!parse_args(argc, argv))
	{
		usage();
		return EXIT_FAILURE;
	}

	rc = SPROX_GetLibraryA(s_buffer, sizeof(s_buffer));
	if (rc != MI_OK)
	{
		printf("Failed to get API version\n");
		goto done;
	}
	printf("API version : %s\n", s_buffer);
	printf("\n");
	
	/* Open reader */
	/* ----------- */

	rc = SPROX_ReaderOpenA(szCommDevice);
	if (rc != MI_OK)
	{
		printf("Reader not found\n");
		goto done;
	}

	printf("Reader found\n");
	rc = SPROX_ReaderGetDeviceA(s_buffer, sizeof(s_buffer));
	if (rc == MI_OK)
		printf("Reader found on %s\n", s_buffer);

	rc = SPROX_ReaderGetFirmwareA(s_buffer, sizeof(s_buffer));
	if (rc == MI_OK)
		printf("Reader firmware is %s\n", s_buffer);

	/* Now perform the polling */
	/* ----------------------- */

	if (!wFindProtos)
		wFindProtos = 0xFFFF;

	printf("\nWaiting for cards... (Press <Ctrl>+C to exit)\n\n");

	for (;;)
	{
		uid_len = sizeof(uid);
		rc = SPROX_FindWait(PROTO_ANY, &proto, uid, &uid_len, 10, 250);
		if (rc == MI_QUIT)
		{
			printf("\n[Timeout, restarting]\n");
		}
		else if (rc != MI_OK)
		{
			printf("\n[Failed]\n");
			printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
			goto close; /* Fatal error */
		}
		else
		{
			/* A card has been found */
			/* --------------------- */

			printf("\nTag found [OK]\n");
			printf("Family=");

			switch (proto)
			{
			case PROTO_14443_A:
				printf("ISO/IEC 14443-A (or NXP Mifare)\n");
				break;
			case PROTO_14443_B:
				printf("ISO/IEC 14443-B\n");
				break;
			case PROTO_15693:
				printf("ISO/IEC 15693\n");
				break;
			case PROTO_ICODE1:
				printf("NXP ICODE-1\n");
				break;
			case PROTO_INSIDE_PICO_14443:
				printf("Inside Secure PicoTag (14443)\n");
				break;
			case PROTO_INSIDE_PICO_15693:
				printf("Inside Secure PicoTag (15693)\n");
				break;
			case PROTO_ST_SR:
				printf("STMicroElectronics SR\n");
				break;
			case PROTO_ASK_CTS:
				printf("ASK CTS256B/CTS512B\n");
				break;
			case PROTO_INNOVATRON:
				printf("Calypso (Innovatron protocol)\n");
				break;
			case PROTO_INNOVISION_JEWEL:
				printf("Innovision Jewel\n");
				break;
			case PROTO_KOVIO_BARCODE:
				printf("Kovio RF barcode\n");
				break;
			case PROTO_FELICA:
				printf("Felica\n");
				break;
			default:
				printf("%04X\n", proto);
			}

			printf("UID=");
			for (i = 0; i < uid_len; i++)
				printf("%02X", uid[i]);
			printf("\n");
		}

		if (fFindOnce)
			break;
	}

close:

	/* Close the reader */
	SPROX_ControlRF(FALSE);
	SPROX_ReaderClose();

done:
	/* Display last error */
	if (rc != MI_OK)
		printf("%s (%d)\n", SPROX_GetErrorMessageA(rc), rc);

	return EXIT_SUCCESS;
}

void usage(void)
{
	printf("usage: %s [PROTOCOLS] [OPTIONS] [-d <COMM. DEVICE>]\n", PROGRAM_NAME);
	printf("Write PROTOCOLS as follow:\n");
	printf(" -* : all protocols supported by the device\n");
	printf(" -P : for ISO/IEC 14443 A and B\n");
	printf(" -A : for ISO 14443-A only\n");
	printf(" -B : for ISO 14443-B only\n");
	printf(" -V : for ISO/IEC 15693\n");
	printf(" -I : for Innovatron (old Calypso cards)\n");
	printf(" -F : for Felica\n");
	printf(" -P : for Inside PicoTag (also HID iClass)\n");
	printf(" -M : for other memory cards (ST SR..., ASK CTS)\n");
	printf("If PROTOCOLS is empty, all protocols are tried (same as -*)\n");
	printf("OPTIONS:");
	printf(" -1 : run only once (exit when first card is found)\n");
	printf(" -v : verbose (trace library functions)\n");
	printf("If the name of COMM DEVICE is not specified, default is taken from Registry or from /etc/springprox.cfg\n");
}

static BOOL parse_args(int argc, char** argv)
{
	for (int i = 1; i < argc; i++)
	{ // Start at 1 because argv[0] is the program name
		if (argv[i][0] == '-')
		{ // This is an option
			if (!strcmp(argv[i], "-d") && i + 1 < argc)
			{
				szCommDevice = argv[i + 1];
				i++;  // Skip next item since we just processed it
			}
			else if (!strcmp(argv[i], "-v"))
			{
				// Ask the library to be verbose
				SPROX_SetVerbose(255, NULL);
			}
			else if (!strcmp(argv[i], "-*"))
			{
				wFindProtos |= 0xFFFF;
			}
			else if (!strcmp(argv[i], "-P"))
			{
				wFindProtos |= PROTO_14443_A | PROTO_14443_B;
			}
			else if (!strcmp(argv[i], "-A"))
			{
				wFindProtos |= PROTO_14443_A;
			}
			else if (!strcmp(argv[i], "-B"))
			{
				wFindProtos |= PROTO_14443_B;
			}
			else if (!strcmp(argv[i], "-V"))
			{
				wFindProtos |= PROTO_15693;
			}
			else if (!strcmp(argv[i], "-F"))
			{
				wFindProtos |= PROTO_FELICA;
			}
			else if (!strcmp(argv[i], "-I"))
			{
				wFindProtos |= PROTO_INNOVATRON;
			}
			else if (!strcmp(argv[i], "-P"))
			{
				wFindProtos |= PROTO_INSIDE_PICO_14443 | PROTO_INSIDE_PICO_15693;
			}
			else if (!strcmp(argv[i], "-M"))
			{
				wFindProtos |= PROTO_ST_SR | PROTO_ASK_CTS;
			}
			else if (!strcmp(argv[i], "-1"))
			{
				fFindOnce = TRUE;
			}
			else if (!strcmp(argv[i], "-h"))
			{
				/* Return FALSE to display the usage message */
				return FALSE;
			}
			else
			{
				printf("Unknown option: %s\n", argv[i]);
				return FALSE;
			}
		}
		else
		{
			// It's not an option, maybe a standalone argument
			printf("Unsupported argument: %s\n", argv[i]);
			return FALSE;
		}
	}

	return TRUE;
}
