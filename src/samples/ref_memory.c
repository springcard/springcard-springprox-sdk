/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  Copyright (c) 2009-2014 PRO ACTIVE SAS, FRANCE - www.springcard.com

  Ref_memory.c
  ------------

  This is the reference applications that shows how to work with
  memory cards.

  LTX 25/08/2009 : initial release for ISO/IEC 15693 only (ref_15693.c)
  JDA 15/12/2010 : fixed everything...
  JDA 13/08/2014 : moved to Visual C++ Express 2010, added the 'A'
				   suffix to all text-related functions
  JDA 05/09/2023 : merged ref_15693.c, ref_stsr176.c and ref_mif_ul.c to dump any memory content

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

const char* PROGRAM_NAME = "ref_memory";
const char* szCommDevice = NULL;
WORD wFindProtos = 0;
BOOL fFindOnce = FALSE;

static BOOL parse_args(int argc, char** argv);
static void usage(void);

static void dump_iso15693(const BYTE uid[8]);
static void dump_mif_ul(void);

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
	printf("%s : dump content of memory cards\n\n", PROGRAM_NAME);
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
		wFindProtos = PROTO_14443_A | PROTO_15693;

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
				case PROTO_15693:
					printf("ISO/IEC 15693\n");
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
							dump_mif_ul();
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

			/* Dump the content of the card */
			dump_iso15693(uid);

			/* Halt the card */
			SPROX_Iso15693_Halt();
			break;

		default:
			break;
		}

		if (fFindOnce)
			break;

		/* Wait 2 seconds for card removal */
#ifdef WIN32
		Sleep(2000);
#endif
#ifdef __linux
		sleep(2);
#endif
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
	printf(" -* : all protocols supported by this sample\n");
	printf(" -A : for ISO 14443-A\n");
	printf(" -V : for ISO/IEC 15693\n");
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
				wFindProtos |= PROTO_14443_A | PROTO_15693;
			}
			else if (!strcmp(argv[i], "-A"))
			{
				wFindProtos |= PROTO_14443_A;
			}
			else if (!strcmp(argv[i], "-V"))
			{
				wFindProtos |= PROTO_15693;
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

static void iso15693_parse_uid(const BYTE uid[8], BOOL *has_read_multiple)
{
	*has_read_multiple = TRUE;

	switch (uid[1])
	{
	case 0x02:
		printf("Manufacturer:   ST MicroElectronics\n");
		switch ((uid[2] & 0xFC) >> 2)
		{
		case 0x05:	/* LRI64 : 15 blocks, 1 byte in each block (block 0 et 7 -> UID, block 8 -> AFI, block 9 -> DSFID)*/
			printf("Chip:           LRI64\n");
			*has_read_multiple = FALSE;
			break;
		default:	/* Other ST card */
			printf("Unknown chip!\n");
			break;
		}
		break;

	case 0x04:
		printf("Manufacturer:   NXP\n");
		switch (uid[2])
		{
		case 0x01: /* ICODE SLI : 28 blocks, 4 bytes in each block */
			printf("Chip:           ICODE-SLI\n");
			break;
		case 0x03: /* ICODE SLI-L : 8 blocks, 4 bytes in each block */
			printf("Chip:           ICODE-SLI-L\n");
			*has_read_multiple = FALSE;
			break;
		case 0x02: /* ICODE SLI-S : 40 blocks, 4 bytes in each block */
			printf("Chip:           ICODE-SLI-S\n");
			*has_read_multiple = FALSE;
			break;
		default: /* Other NXP card */
			printf("Unknown chip!\n");
			break;
		}
		break;

	case 0x07:
		printf("Manufacturer:   Texas Instrument\n");
		switch (uid[2] & 0xFE)
		{
		case 0x00:	/* Tag-it HF-I Plus Inlay : 64 blocks, 4 bytes in each block */
			printf("Chip:           TagIT HF-I Plus (inlay)\n");
			break;
		case 0x80:	/* Tag-it HF-I Plus Chip : 64 blocks, 4 bytes in each block */
			printf("Chip:           TagIT HF-I Plus (die)\n");
			break;
		case 0xC0:	/* Tag-it HF-I Standard Chip/Inlay : 11 blocks, 4 bytes in each block (block 8 & 9 -> UID, block 10 -> AFI)*/
			printf("Chip:           TagIT HF-I Standard\n");
			*has_read_multiple = FALSE;
			break;
		case 0xC4:	/* Tag-it HF-I Pro Chip/Inlay : 12 blocks, 4 bytes in each block (block 8 & 9 -> UID, block 10 -> AFI, block 11 -> password)*/
			printf("Chip:           TagIT HF-I Pro\n");
			*has_read_multiple = FALSE;
			break;
		default:	/* Other Tag-it card */
			printf("Unknown chip!\n");
			break;
		}
		break;


	case 0x16:   /* EMMarin */
		printf("Manufacturer:   ElectronicMarin (or Legic)\n");
		break;

	default:
		printf("Unknown manufacturer/unknown chip\n");
		break;
	}
}


static void dump_iso15693(const BYTE uid[8])
{
	SWORD rc;
	int i = 0;
	BYTE buffer[256];
	WORD length;

	BOOL has_read_multiple = FALSE;
	WORD block_size = 0;
	WORD block_count = 0;
	WORD block, blocks;

	iso15693_parse_uid(uid, &has_read_multiple);

	printf("GetSystemInformation ");
	length = sizeof(buffer);
	rc = SPROX_Iso15693_GetSystemInformation(NULL, buffer, &length);
	if (rc == MI_OK)
	{
		printf("[OK]\n");

		if (length > 9)
		{
			printf("UID=");
			for (i = 1; i < 9; i++)
				printf("%02X", buffer[9 - i]);
			printf("\n");

			if (buffer[0] & 0x01)
			{
				printf("DSFID=%02X\n", buffer[i++]);
			}

			if (buffer[0] & 0x02)
			{
				printf("AFI=%02X\n", buffer[i++]);
			}

			if (buffer[0] & 0x04)
			{
				block_count = buffer[i++] + 1;
				printf("Number of Blocks=%d\n", block_count);
				block_size = (buffer[i++] & 0x1F) + 1;
				printf("Block Size=%d\n", block_size);
			}

			if (buffer[0] & 0x08)
			{
				printf("IC-Ref=%02X\n", buffer[i++]);
			}
		}
	}
	else
	{
		printf("[Failed]\n");
		printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
	}

	if (block_count || block_size)
	{
		printf("Reading %d blocks of %dB...\n", block_count, block_size);
	}
	else
	{
		printf("Trying to read up to 256 blocks\n");
		block_count = 256;
		block_size = 256;
	}

	for (block = 0; block < block_count; block++)
	{
		printf("%03d : ", block);		
		length = sizeof(buffer);
		rc = SPROX_Iso15693_ReadSingleBlock(NULL, (BYTE)block, buffer, &length);
		if (rc == MI_OK)
		{
			for (i = 0; i < length; i++)
			{
				printf("%02X", buffer[i]);
			}
			printf("   ");
			for (i = 0; i < length; i++)
			{
				printf("%c", (buffer[i] >= ' ') ? buffer[i] : '.');
			}
			printf("\n");

			if (block_size != length)
			{
				block_size = length;
				printf("Actual block size = %d\n", block_size);
			}
		}
		else
		{
			printf("%s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
			if ((rc == MI_ACCESSERR) || (rc == MI_OTHERERR))
			{
				/* End of tag reached! */
				block_count = block;
				printf("Actual number of blocks = %d\n", block_count);
				break;
			}
			else if (rc == MI_NOTAGERR)
			{
				printf("Card removed?\n");
				return;
			}
		}
	}

	if (has_read_multiple)
	{
		printf("Trying the ReadMultiple function\n");

		for (blocks = 1; blocks < block_count; blocks++)
		{
			if ((blocks * block_size) > 256) break;

			printf("%03d block(s) (%03dB)... ", blocks, blocks * block_size);
			length = sizeof(buffer);
			rc = SPROX_Iso15693_ReadMultipleBlocks(NULL, 0, (BYTE)blocks, buffer, &length);
			if (rc == MI_OK)
			{
				if (length == blocks * block_size)
					printf("OK\n");
				else
					printf("Got %03dB ?\n", length);
			}
			else
			{
				printf("%s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
				if ((rc == MI_ACCESSERR) || (rc == MI_OTHERERR))
				{
					/* End of tag reached! */
					block_count = block;
					printf("Actual number of blocks = %d\n", block_count);
					break;
				}
				else if (rc == MI_NOTAGERR)
				{
					printf("Card removed?\n");
					return;
				}
			}
		}
	}
}

static void dump_mif_ul(void)
{
	SWORD rc;
	int i, j;
	WORD page;
	BYTE data[16];

	/* Read pages 0 to 15 */
	printf("Reading page 0 to 15...\n");

	/* We read 4 pages at once */
	for (page = 0; page < 16; page += 4)
	{
		rc = SPROX_MifRead(NULL, (BYTE) page, data);
		if (rc != MI_OK)
		{
			printf("Failed to read at address %d\n", page);
			return;
		}

		for (i = 0; i < 4; i++)
		{
			printf("Page %03d : ", page + i);

			/* Hexadecimal display */
			for (j = 4 * i; j < 4 * (i + 1); j++)
				printf("%02X", data[j]);

			printf("   ");

			/* RAW display of ASCII printable data */
			for (j = 4 * i; j < 4 * (i + 1); j++)
			{
				if (data[i] >= ' ')
					printf("%c", data[j]);
				else
					printf(".");
			}

			printf("\n");
		}
	}

	/* Reading after the first 16 pages */
	printf("Reading page 16 to 255...\n");

	/* We read 4 pages at once */
	for (page = 16; page < 255; page += 4)
	{
		rc = SPROX_MifRead(NULL, (BYTE)page, data);
		if (rc != MI_OK)
		{
			printf("Failed to read at address %d\n", page);
			return;
		}

		for (i = 0; i < 4; i++)
		{
			printf("Page %03d : ", page + i);

			/* Hexadecimal display */
			for (j = 4 * i; j < 4 * (i + 1); j++)
				printf("%02X", data[j]);

			printf("   ");

			/* RAW display of ASCII printable data */
			for (j = 4 * i; j < 4 * (i + 1); j++)
			{
				if (data[i] >= ' ')
					printf("%c", data[j]);
				else
					printf(".");
			}

			printf("\n");
		}
	}

}
