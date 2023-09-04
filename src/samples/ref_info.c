/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  Copyright (c) 2000-2023 PRO ACTIVE SAS, FRANCE - www.springcard.com

  ref_info.c
  ----------

  This is the reference applications that shows how to dump reader info.
  JDA 04/09/2023 : creation

*/
#include "products/springprox/api/springprox.h"
#include "products/springprox/api/sprox_capas.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* PROGRAM_NAME = "ref_info";
const char* szCommDevice = NULL;

static BOOL parse_args(int argc, char** argv);
static void usage(void);

int main(int argc, char** argv)
{
	SWORD rc;
	char s_buffer[64];
	DWORD features;
	const char* yes = "YES";
	const char* no = "NO";

	/* Display the informations and check the command line */
	/* --------------------------------------------------- */

	printf("SpringCard SpringProx 'Legacy' SDK\n");
	printf("\n");
	printf("%s : dump characteristics of SpringProx reader\n\n", PROGRAM_NAME);
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

	/* Open the reader */
	/* --------------- */

	rc = SPROX_ReaderOpenA(szCommDevice);
	if (rc != MI_OK)
	{
		printf("Reader not found\n");
		goto done;
	}

	/* Display reader's information */
	printf("Reader found\n");
	rc = SPROX_ReaderGetDeviceA(s_buffer, sizeof(s_buffer));
	if (rc != MI_OK)
	{
		printf("SPROX_ReaderGetDevice err. %d\n", rc);
		goto close;
	}
	printf("Reader found on %s\n", s_buffer);

	rc = SPROX_ReaderGetFirmwareA(s_buffer, sizeof(s_buffer));
	if (rc != MI_OK)
	{
		printf("SPROX_ReaderGetFirmware err. %d\n", rc);
		goto close;
	}
	printf("Reader firmware is %s\n", s_buffer);

	rc = SPROX_ReaderGetFeatures(&features);
	if (rc != MI_OK)
	{
		printf("SPROX_ReaderGetFeatures err. %d\n", rc);
		goto close;
	}

	printf("Reader features are %08lX\n", features);
	printf("\tISO/IEC 14443 supported                 : %s\n", (features & SPROX_WITH_ISO_14443) ? yes : no);
	printf("\tISO/IEC 15693 supported                 : %s\n", (features & SPROX_WITH_ISO_15693) ? yes : no);
	printf("\tISO/IEC 18092 supported                 : %s\n", (features & SPROX_WITH_ISO_18092) ? yes : no);
	printf("\tFeliCa supported                        : %s\n", (features & SPROX_WITH_FELICA) ? yes : no);
	printf("\tNFC Target mode supported               : %s\n", (features & SPROX_WITH_TARGET) ? yes : no);
	printf("\tNFC Card emulation supported            : %s\n", (features & SPROX_WITH_EMUL) ? yes : no);
	printf("\tMultiple contactless antennas           : %s\n", (features & SPROX_WITH_MANY_ANTENNAS) ? yes : no);
	printf("\tISO/IEC 7816 (contact) interface        : %s\n", (features & SPROX_WITH_ISO_7816) ? yes : no);
	printf("\tI2C interface                           : %s\n", (features & SPROX_WITH_I2C_BUS) ? yes : no);
	printf("\t125kHz (LF) interface                   : %s\n", (features & SPROX_WITH_125_KHZ) ? yes : no);
	printf("\tASCII protocol available                : %s\n", (features & SPROX_WITH_ASCII_PROTOCOL) ? yes : no);
	printf("\tBinary protocol available               : %s\n", (features & SPROX_WITH_BIN_PROTOCOL) ? yes : no);
	printf("\tBus protocol available                  : %s\n", (features & SPROX_WITH_BIN_BUS_PROTOCOL) ? yes : no);
	printf("\tRS-485 logic implemented                : %s\n", (features & SPROX_WITH_RS485_SERIAL) ? yes : no);
	printf("\tRS-485 driver wired                     : %s\n", (features & SPROX_WITH_RS485_DRIVER) ? yes : no);
	printf("\t115200bps supported                     : %s\n", (features & SPROX_WITH_BAUDRATE_115200) ? yes : no);
	printf("\tSeparated RX/TX buffers                 : %s\n", (features & SPROX_WITH_DUAL_BUFFERS) ? yes : no);
	printf("\tJumbo RX / TX buffers                   : % s\n", (features & SPROX_WITH_XXL_BUFFERS) ? yes : no);
	printf("\tUSB CDC-ACM (virtul serial) profile     : %s\n", (features & SPROX_WITH_USB_VCP) ? yes : no);
	printf("\tUSB CCID (PC/SC) profile                : %s\n", (features & SPROX_WITH_USB_CCID) ? yes : no);
	printf("\tUSB HID Keyboard (RFID Scanner) profile : %s\n", (features & SPROX_WITH_USB_HID) ? yes : no);
	printf("\tConfiguration memory (\"FEED\")           : % s\n", (features & SPROX_WITH_FEED) ? yes : no);
	printf("\tStorage memory                          : % s\n", (features & SPROX_WITH_STORAGE) ? yes : no);
	printf("\tShell (\"human console\")                 : % s\n", (features & SPROX_WITH_HUMAN_CONSOLE) ? yes : no);
	printf("\n");

close:

	/* Close the reader */
	SPROX_ReaderClose();

done:
	/* Display last error */
	if (rc != MI_OK)
		printf("%s (%d)\n", SPROX_GetErrorMessageA(rc), rc);

	return EXIT_SUCCESS;
}


void usage(void)
{
	printf("usage: %s [-d <COMM. DEVICE>] [-v]\n\n", PROGRAM_NAME);
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
