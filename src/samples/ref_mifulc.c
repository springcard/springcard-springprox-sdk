/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  Copyright (c) 2011-2014 PRO ACTIVE SAS, FRANCE - www.springcard.com

  ref_mif_ulc.c
  ------------

  This is the reference applications that shows how to authenticate
  on a MIFARE UltraLight C tag.

  JDA 05/09/2023 : refreshed the project to build with Visual Studio 2022

*/
#include "products/springprox/api/springprox.h"
#include "cardware/mifulc/sprox_mifulc.h"
#include <stdio.h>

const char* PROGRAM_NAME = "ref_mifulc";
const char* szCommDevice = NULL;

const BYTE default_key[16] = { 0x49, 0x45, 0x4D, 0x4B, 0x41, 0x45, 0x52, 0x42, 0x21, 0x4E, 0x41, 0x43, 0x55, 0x4F, 0x59, 0x46 };
const BYTE blank_key[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static BOOL parse_args(int argc, char** argv);
static void usage(void);

int main(int argc, char** argv)
{
	SWORD rc;
	int i;
	char s_buffer[64];
	WORD atq;
	BYTE sak;
	BYTE uid[32];
	BYTE uid_len = 32;
	BYTE info[32];
	BYTE info_len = 32;
	BYTE addr;
	BYTE data[16];

	printf("SpringCard SpringProx 'Legacy' SDK\n");
	printf("\n");
	printf("%s : NXP MIFARE UltraLight C validation & sample\n\n", PROGRAM_NAME);
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

	/* Open reader */
	/* ----------- */

	rc = SPROX_ReaderOpen(szCommDevice);
	if (rc != MI_OK)
	{
		printf("Reader not found\n");
		goto done;
	}

	rc = SPROX_ReaderGetDevice(s_buffer, sizeof(s_buffer));
	if (rc == MI_OK)
		printf("Reader found on %s\n", s_buffer);

	rc = SPROX_ReaderGetFirmware(s_buffer, sizeof(s_buffer));
	if (rc == MI_OK)
		printf("Reader firwmare is %s\n", s_buffer);

	/* Now wait for a card */
	/* ------------------- */

	rc = SPROX_FindEx(PROTO_14443_A, NULL, uid, &uid_len, info, &info_len);
	if (rc == MI_NOTAGERR)
	{
		printf("No card on the reader!\n");
		goto close;
	}
	if (rc != MI_OK)
	{
		printf("Failed to select the card!\n");
		goto close;
	}

	/* Display the data */
	/* ---------------- */

	printf("UID=");
	for (i = 0; i < uid_len; i++)
		printf("%02X", uid[i]);
	printf("\n");

	atq = info[0] | info[1] << 8;
	sak = info[2];

	printf("ATQ=%04X, SAK=%02X\n", atq, sak);

	/* Is the tag a Mifare UltraLight C ? */
	/* ---------------------------------- */
	if ((uid[0] != 0x04) || (atq != 0x0044) || (sak != 0x00))
	{
		printf("This does not look like a Mifare UltraLight C tag\n");
		goto halt;
	}

	if (TRUE)
	{
		printf("Authentication with blank key... ");
		rc = SPROX_MifUlC_Authenticate(blank_key);
		if (rc == MI_OK) printf("OK!\n"); else printf("Failed (%d)\n", rc);
	}

	if (rc != MI_OK)
	{
		printf("Authentication with default key... ");
		rc = SPROX_MifUlC_Authenticate(default_key);
		if (rc == MI_OK) printf("OK!\n"); else printf("Failed (%d)\n", rc);
	}

	if (rc != MI_OK)
	{
		printf("Authentication failed (%d)\n", rc);
		goto halt;
	}

	rc = SPROX_MifUlC_ChangeKey(default_key);
	if (rc != MI_OK)
	{
		printf("Change key failed (%d)\n", rc);
		goto halt;
	}

	for (addr = 0; addr < 44; addr += 4)
	{
		rc = SPROX_MifUlC_Read(addr, data);
		if (rc != MI_OK)
		{
			printf("Read at %d failed (%d)\n", addr, rc);
			goto halt;
		}
		printf("%03d:", addr);
		for (i = 0; i < 16; i++)
		{
			if ((i % 4) == 0)
				printf(" ");
			printf("%02X", data[i]);
		}
		printf("\n");
	}

	data[0] = 0x12;
	data[1] = 0x34;
	data[2] = 0x56;
	data[3] = 0x78;

	rc = SPROX_MifUlC_Write4(4, data);
	if (rc != MI_OK)
	{
		printf("Write at %d failed (%d)\n", 4, rc);
		goto halt;
	}

	printf("SUCCESS!\n");

halt:

	/* ISO 14443-3 tag stop */
	/* -------------------- */

	SPROX_A_Halt();

close:
	SPROX_ControlRF(FALSE);
	SPROX_ReaderClose();

done:
	/* Display last error */
	if (rc != MI_OK)
		printf("%s (%d)\n", SPROX_GetErrorMessageA(rc), rc);

	return 0;
}

static void usage(void)
{
	printf("Usage: %s [OPTIONS] [-d <COMM DEVICE]\n", PROGRAM_NAME);
	printf("OPTIONS:\n");
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