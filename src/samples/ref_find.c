/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  Copyright (c) 2000-2014 PRO ACTIVE SAS, FRANCE - www.springcard.com

  ref_find.c
  ----------

  This is the reference applications that shows how to poll the reader,
  showing each incoming tag identifier

  LTX 25/08/2009 : initial release
  JDA 24/01/2012 : minor changes to adapt to release 1.6x of the SDK
  JDA 04/02/2013 : minor changes to adapt to release 1.7x of the SDK
  JDA 13/08/2014 : moved to Visual C++ Express 2010, added the 'A'
				   suffix to all text-related functions

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

const char* PROGRAM_NAME = "ref_find";
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
	BYTE uid[32];
	BYTE uid_len;
	BYTE info[32];
	BYTE info_len;

	WORD proto = 0;

	/* Display the informations and check the command line */
	/* --------------------------------------------------- */

	printf("SpringCard SpringProx 'Legacy' SDK\n");
	printf("\n");
	printf("%s : multi-protocol card polling sample\n\n", PROGRAM_NAME);
	printf("This is a free and unsupported sample provided by www.springcard.com\n");
	printf("Please read LICENSE.txt for details\n");
	printf("\n");

	if (!parse_args(argc, argv))
	{
		usage();
		return EXIT_FAILURE;
	}

	rc = SPROX_GetLibrary(s_buffer, sizeof(s_buffer));
	if (rc != MI_OK)
	{
		printf("Failed to get API version\n");
		goto done;
	}
	printf("API version : %s\n", s_buffer);
	printf("\n");

	/* Open the reader */
	/* --------------- */

	rc = SPROX_ReaderOpen(szCommDevice);
	if (rc != MI_OK)
	{
		printf("Reader not found\n");
		goto done;
	}

	/* Display reader's information */
	printf("Reader found\n");
	rc = SPROX_ReaderGetDevice(s_buffer, sizeof(s_buffer));
	if (rc == MI_OK)
		printf("Reader found on %s\n", s_buffer);

	rc = SPROX_ReaderGetFirmware(s_buffer, sizeof(s_buffer));
	if (rc == MI_OK)
		printf("Reader firmware is %s\n", s_buffer);

	/* Now perform the polling */
	/* ----------------------- */

	if (!wFindProtos)
		wFindProtos = 0xFFFF;

	printf("\nWaiting for cards... (Press <Ctrl>+C to exit)\n\n");

	for (;;)
	{
		rc = MI_OK;
		/* Not too often... */
#ifdef WIN32
		Sleep(50);
		if (_kbhit())
			break;		
#endif
#ifdef __linux
		usleep(5000);
#endif

		for (;;)
		{
			/* Invoke reader's SPROX_FIND function (with subsequent SPROX_FIND_INFO) */
			/* --------------------------------------------------------------------- */

			uid_len = sizeof(uid);
			info_len = sizeof(info);
			rc = SPROX_FindEx(wFindProtos, &proto, uid, &uid_len, info, &info_len);


			if (rc != MI_OK)
			{
				if (rc == MI_NOTAGERR)
					continue; /* No card */

				printf("\n[Failed]\n");
				printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
				if (rc < -128)
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

				/* Leave the loop to show the card's details */
				break;
			}
		}

		switch (proto)
		{
		case PROTO_14443_A:

			/* INFO returns the ATQA (2 bytes) then the SAK (1 byte) */
			/* ----------------------------------------------------- */

		{

			WORD atq;
			BYTE sak;

			atq = info[0] | info[1] << 8;
			sak = info[2];

			printf("ATQ=%04X, SAK=%02X\n", atq, sak);

			/* See NXP AN10834 and AN10833 */
			printf("Based on the SAK, this card is ");
			if (sak & 0x02)
			{
				printf("a NFC object (peer-to-peer capable)\n");
			}
			else
			{
				if (sak & 0x08)
				{
					if (sak & 0x10)
					{
						printf("a Mifare Classic 4K\n");
					}
					else if (sak & 0x01)
					{
						printf("a Mifare Mini\n");
					}
					else
					{
						printf("a Mifare Classic 1K\n");
					}
				}
				else
				{
					if (sak & 0x10)
					{
						if (sak & 0x01)
						{
							printf("a Mifare Plus 4K in SL2\n");
						}
						else
						{
							printf("a Mifare Plus 2K in SL2\n");
						}
					}
					else
					{
						if (sak & 0x20)
						{
							printf("T=CL compliant\n");
						}
						else
						{
							printf("a Mifare UltraLight or an NFC Forum Type 2 Tag\n");
						}
					}
				}
			}

			/* Halt the card */
			SPROX_A_Halt();
		}
		break;

		case PROTO_14443_B:

			/* INFO returns the ATQB (11 or maybe 12 bytes somedays) */
			/* ----------------------------------------------------- */

			printf("ATQB=");
			for (i = 0; i < info_len; i++)
				printf("%02X", info[i]);
			printf("\n");

			/* Halt the card */
			SPROX_B_Halt(uid);
			break;

		case PROTO_15693:

			/* The UID starts with the vendor name (ISO 7816-6) */
			/* ------------------------------------------------ */

			if (uid_len != 8)
			{
				printf("Strange! Length of ISO/IEC 15693 UID shall be 8\n");
			}
			if (uid[0] != 0xE0)
			{
				printf("Strange! ISO/IEC 15693 UID shall start with E0\n");
			}

			switch (uid[1])
			{
			case 0x02:
				/* ST */
				printf("Manufacturer is STMicroElectronics\n");

				switch ((uid[2] & 0xFC) >> 2)
				{
				case 0x05: /* LRI64 */
					printf("This is a LRI64\n");
					break;
				case 0x16: /* M24LR16E */
					printf("This is a M24LR16E\n");
					break;
				default: /* ST card */
					printf("Un-referenced product type: %02X\n", (uid[2] & 0xFC) >> 2);
					break;
				}

				break;

			case 0x04:
				/* NXP */
				printf("Manufacturer is NXP\n");
				switch (uid[2])
				{
				case 0x01: /* ICODE SLI */
					printf("This is a ICODE SLI\n");
					break;
				case 0x03: /* ICODE SLI-L */
					printf("This is a ICODE SLI-L\n");
					break;
				case 0x02: /* ICODE SLI-S */
					printf("This is a ICODE SLI-S\n");
					break;
				default: /* NXP card */
					printf("Un-referenced product type: %02X\n", uid[2]);
					break;
				}
				break;

			case 0x07:
				/* Texas */
				printf("Manufacturer is Texas Instruments\n");
				switch (uid[2] & 0xFE)
				{
				case 0x00: /* Tag-it HF-I Plus Inlay */
					printf("This is a Tag-it HF-I Plus Inlay\n");
					break;
				case 0x80: /* Tag-it HF-I Plus Chip */
					printf("This is a Tag-it HF-I Plus Chip\n");
					break;
				case 0xC0: /* Tag-it HF-I Standard Chip/Inlay */
					printf("This is a Tag-it HF-I Standard\n");
					break;
				case 0xC4: /* Tag-it HF-I Pro Chip/Inlay */
					printf("This is a Tag-it HF-I Pro\n");
					break;
				default: /* Tag-it card */
					printf("Un-referenced product type: %02X\n", uid[2] & 0xFE);
					break;
				}
				break;

			case 0x16:
				/* EM */
				printf("Manufacturer is EM Marin (or Legic)\n");
				break;

			default:
				printf("Un-referenced manufacturer: %02X\n", uid[1]);
			}

			/* Halt the card */
			SPROX_Iso15693_Halt();
			break;

		default:
			break;
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
	printf("usage: %s [PROTOCOLS] [OPTIONS] [-d <COMM DEVICE>]\n\n", PROGRAM_NAME);
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
	printf("OPTIONS:\n");
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
				wFindProtos |= PROTO_14443_A|PROTO_14443_B;
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
				wFindProtos |= PROTO_INSIDE_PICO_14443| PROTO_INSIDE_PICO_15693;
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
