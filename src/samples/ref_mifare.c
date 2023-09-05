/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  Copyright (c) 2000-2014 PRO ACTIVE SAS, FRANCE - www.springcard.com

  ref_mifare.c
  ------------

  This is the reference applications that validates the Reader (and its
  API) against NXP Mifare Classic cards.

  JDA 21/11/2003 : initial release
  JDA 03/11/2006 : calling SPROX_A_SelectIdle and SPROX_A_Halt instead
				   of Mf500xxxx functions
  JDA 10/01/2007 : fixed a few warnings when compiling under GCC
  JDA 23/02/2007 : added the '-b' flag for "benchmarking"
	JDA 04/11/2010 : added support of Mifare Classic with 7-byte UIDs
  JDA 04/02/2013 : minor changes to adapt to release 1.7x of the SDK
  JDA 05/02/2012 : now testing the VALUE functions
  JDA 13/08/2014 : moved to Visual C++ Express 2010, added the 'A'
				   suffix to all text-related functions
  JDA 04/09/2023 : refreshed the project to build with Visual Studio 2022
*/
#include "products/springprox/api/springprox.h"

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const char* PROGRAM_NAME = "ref_mifare";
const char* szCommDevice = NULL;
BOOL fQuickTest = FALSE;
BOOL fFullTest = FALSE;
BOOL fBenchmark = FALSE;

static BOOL parse_args(int argc, char** argv);
static void usage(void);

static SWORD reader_load_test_keys(void);
static SWORD reader_load_default_keys(void);
static SWORD reader_erase_keys(void);

static SWORD card_format_transport(BYTE sectors, BOOL persist);
static SWORD card_format_for_test(BYTE sectors, BOOL persist);

static SWORD card_read(BYTE sectors);
static SWORD card_read_failure(BYTE sectors);
static SWORD card_read_test_00(BYTE sectors, BOOL transport);
static SWORD card_write_test_xx(BYTE sectors);
static SWORD card_read_test_xx(BYTE sectors);

static SWORD card_value_test(BYTE sector);

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
	BYTE sectors = 0;

	/* Display the informations and check the command line */
	/* --------------------------------------------------- */

	printf("SpringCard SpringProx 'Legacy' SDK\n");
	printf("\n");
	printf("%s : NXP MIFARE Classic validation & sample\n\n", PROGRAM_NAME);
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
				sectors = 40;
			}
			else if (sak & 0x01)
			{
				printf("a Mifare Mini\n");
				sectors = 5;
			}
			else
			{
				printf("a Mifare Classic 1K\n");
				sectors = 16;
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
					printf("a Mifare UltraLight\n");
				}
			}
		}
	}

	if (sectors == 0)
	{
		printf("This card is not supported!\n");
		goto close;
	}

	if (fQuickTest)
		sectors = 2; /* Limit the number of tests */

	if (fFullTest)
	{
		printf("Resetting the card, restoring channel...\n");
		rc = SPROX_A_Halt();
		if (rc != MI_OK) goto close;
		rc = SPROX_A_SelectAgain(uid, uid_len);
		if (rc != MI_OK) goto close;

		printf("Preparing the card for the test...\n");
		rc = card_format_transport(sectors, TRUE);
		if (rc != MI_OK) goto close;

		rc = card_format_for_test(sectors, TRUE);
		if (rc != MI_OK) goto close;

		printf("Loading the test keys into the reader...\n");
		rc = reader_load_test_keys();
		if (rc != MI_OK) goto close;

		printf("Reading the card...\n");
		rc = card_read_test_00(sectors, FALSE);
		if (rc != MI_OK) goto close;

		printf("Writing some data into the card...\n");
		rc = card_write_test_xx(sectors);
		if (rc != MI_OK) goto close;

		printf("Reading the card again...\n");
		rc = card_read_test_xx(sectors);
		if (rc != MI_OK) goto close;

		printf("Erasing the test keys from the reader...\n");
		rc = reader_erase_keys();
		if (rc != MI_OK) goto close;

		printf("Trying to read the card again...\n");
		rc = card_read_failure(sectors);
		if (rc != MI_OK) goto close;

		printf("Erasing the card, back to transport conditions...\n");
		rc = card_format_transport(sectors, TRUE);
		if (rc != MI_OK) goto close;

		printf("Resetting the card, restoring channel...\n");
		rc = SPROX_A_Halt();
		if (rc != MI_OK) goto close;
		rc = SPROX_A_SelectAgain(uid, uid_len);
		if (rc != MI_OK) goto close;

		printf("Reading the card again...\n");
		rc = card_read_failure(sectors);
		if (rc != MI_OK) goto close;

		printf("Loading the default keys into the reader...\n");
		rc = reader_load_default_keys();
		if (rc != MI_OK) goto close;

		printf("Reading the card (last !)...\n");
		rc = card_read_test_00(sectors, TRUE);
		if (rc != MI_OK) goto close;

		printf("Testing the value functions...\n");
		rc = card_value_test(1);
		if (rc != MI_OK) goto close;

	}
	else
	{
		rc = card_read(sectors);
		if (rc != MI_OK) goto close;
	}

	/* Halt the tag */
	rc = SPROX_A_Halt();
	if (rc != MI_OK)
	{
		printf("Failed to halt the tag\n");
		goto close;
	}

	printf("SUCCESS!\n");

close:
	/* Close the reader */
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
	printf("Usage: %s [-OPTIONS] [-d <COMM DEVICE]\n", PROGRAM_NAME);
	printf("OPTIONS:\n");
	printf(" -q : (quick) the software tests only the sectors 0 and 1, not all sectors on the card.\n");
	printf(" -f : (full)  the software formats the card and passes in all functions.\n");
	printf(" -b : (bench) , the software measures the timings.\n");
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
			else if (!strcmp(argv[i], "-q"))
			{
				fQuickTest = TRUE;
			}
			else if (!strcmp(argv[i], "-f"))
			{
				fFullTest = TRUE;
			}
			else if (!strcmp(argv[i], "-b"))
			{
				fBenchmark = TRUE;
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

BYTE key_aa[6] = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA };
BYTE key_bb[6] = { 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB };
BYTE key_ff[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
BYTE key_cc[6] = { 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC };

SWORD card_read(BYTE sectors)
{
	BYTE data[240];
	clock_t t0, t1;
	BYTE blocks, block, sector;
	BYTE offset;
	SWORD rc;

	if (fBenchmark)
	{
		printf("Reading %d sectors...\n", sectors);
		t0 = clock();
	}

	block = 0;
	for (sector = 0; sector < sectors; sector++)
	{
		if (!fBenchmark)
			printf("Reading sector %02d : ", sector);

		/* The SPROX_MifStReadSector functions assumes that the data buffer is big enough... */
		rc = SPROX_MifStReadSector(NULL, sector, data, NULL);
		if (rc != MI_OK)
		{
			if (fBenchmark)
				printf("Read sector %02d ", sector);
			printf("[Failed]\n");
			printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
			return rc;
		}

		if (!fBenchmark)
		{
			printf("[OK]\n");
			/* Display sector's data */
			if (sector < 32)
				blocks = 3;
			else
				blocks = 15;
			for (block = 0; block < blocks; block++)
			{
				printf("%02d : ", block);
				/* Each blocks is 16-bytes wide */
				for (offset = 0; offset < 16; offset++)
				{
					printf("%02X ", data[16 * block + offset]);
				}
				for (offset = 0; offset < 16; offset++)
				{
					if (data[16 * block + offset] >= ' ')
					{
						printf("%c", data[16 * block + offset]);
					}
					else
					{
						printf(".");
					}
				}
				printf("\n");
			}
		}
	}

	if (fBenchmark)
	{
		t1 = clock();
		printf("Time elapsed: %ldms\n", (t1 - t0) / (CLOCKS_PER_SEC / 1000));
	}

	return MI_OK;
}

void compute_test_keys(BYTE sector, BYTE key_a[6], BYTE key_b[6])
{
	char str[12];

	if (key_a != NULL)
	{
		snprintf(str, sizeof(str), "SC_A%02d", sector);
		memcpy(key_a, str, 6);
	}
	if (key_b != NULL)
	{
		snprintf(str, sizeof(str), "SC_B%02d", sector);
		memcpy(key_b, str, 6);
	}
}

SWORD card_format_transport(BYTE sectors, BOOL persist)
{
	BYTE sector;
	BYTE key_a[6], key_b[6];
	BYTE data[240];
	SWORD rc;

	memset(data, 0, sizeof(data));

	for (sector = 0; sector < sectors; sector++)
	{
		compute_test_keys(sector, key_a, key_b);

		rc = SPROX_MifStUpdateAccessBlock(NULL, sector, key_b, key_ff, key_ff, ACC_BLOCK_TRANSPORT, ACC_BLOCK_TRANSPORT, ACC_BLOCK_TRANSPORT, ACC_AUTH_TRANSPORT);
		if (rc != MI_OK)
			rc = SPROX_MifStUpdateAccessBlock(NULL, sector, key_a, key_ff, key_ff, ACC_BLOCK_TRANSPORT, ACC_BLOCK_TRANSPORT, ACC_BLOCK_TRANSPORT, ACC_AUTH_TRANSPORT);
		if (rc != MI_OK)
			rc = SPROX_MifStUpdateAccessBlock(NULL, sector, key_ff, key_ff, key_ff, ACC_BLOCK_TRANSPORT, ACC_BLOCK_TRANSPORT, ACC_BLOCK_TRANSPORT, ACC_AUTH_TRANSPORT);
		if (rc != MI_OK)
			rc = SPROX_MifStUpdateAccessBlock(NULL, sector, key_bb, key_ff, key_ff, ACC_BLOCK_TRANSPORT, ACC_BLOCK_TRANSPORT, ACC_BLOCK_TRANSPORT, ACC_AUTH_TRANSPORT);
		if (rc != MI_OK)
			rc = SPROX_MifStUpdateAccessBlock(NULL, sector, key_aa, key_ff, key_ff, ACC_BLOCK_TRANSPORT, ACC_BLOCK_TRANSPORT, ACC_BLOCK_TRANSPORT, ACC_AUTH_TRANSPORT);

		if (rc != MI_OK)
		{
			printf("Format (transport) sector %02d [Failed]\n", sector);
			printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
			if (!persist)
				return rc;
		}
	}

	for (sector = 0; sector < sectors; sector++)
	{
		rc = SPROX_MifStWriteSector(NULL, sector, data, key_ff);
		if (rc != MI_OK)
		{
			printf("Write (transport) sector %02d [Failed]\n", sector);
			printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
			if (!persist)
				return rc;
		}
	}

	rc = SPROX_MifStReadBlock(NULL, 1, data, key_cc);
	if (rc != MI_AUTHERR)
	{
		printf("Read sector %02d -> %d (transport)\n", 1, rc);
		return 1;
	}

	return MI_OK;
}

SWORD card_format_for_test(BYTE sectors, BOOL persist)
{
	BYTE sector;
	BYTE key_a[6], key_b[6];
	BYTE data[240];
	SWORD rc;

	for (sector = 0; sector < sectors; sector++)
	{
		compute_test_keys(sector, key_a, key_b);

		rc = SPROX_MifStUpdateAccessBlock(NULL, sector, key_ff, key_a, key_b, ACC_BLOCK_DATA, ACC_BLOCK_DATA, ACC_BLOCK_DATA, ACC_AUTH_NORMAL);
		if (rc != MI_OK)
			rc = SPROX_MifStUpdateAccessBlock(NULL, sector, key_b, key_a, key_b, ACC_BLOCK_DATA, ACC_BLOCK_DATA, ACC_BLOCK_DATA, ACC_AUTH_NORMAL);
		if (rc != MI_OK)
		{
			printf("Format (for test) sector %02d [Failed]\n", sector);
			printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
			if (!persist)
				return rc;
		}

		rc = SPROX_MifStReadSector(NULL, sector, data, key_a);
		if (rc != MI_OK)
		{
			printf("Read (for test) sector %02d [Failed]\n", sector);
			printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
			if (!persist)
				return rc;
		}

		memset(data, 0, sizeof(data));

		rc = SPROX_MifStWriteSector(NULL, sector, data, key_b);
		if (!persist && (rc != MI_OK))
		{
			printf("Write (for test) sector %02d [Failed]\n", sector);
			printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
			if (!persist)
				return rc;
		}
	}

	rc = SPROX_MifStReadBlock(NULL, 1, data, key_cc);
	if (rc != MI_AUTHERR)
	{
		printf("Read sector %02d -> %d (format)\n", 1, rc);
		return 1;
	}

	return MI_OK;
}

SWORD reader_load_test_keys(void)
{
	BYTE idx;
	BYTE key_a[6], key_b[6];
	SWORD rc;

	for (idx = 0; idx < 16; idx++)
	{
		compute_test_keys(idx, key_a, key_b);

		rc = SPROX_MifLoadKey(TRUE, FALSE, idx, key_a);
		if (rc != MI_OK)
		{
			printf("Load key A%02d to EEPROM [Failed]\n", idx);
			printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
			return rc;
		}

		rc = SPROX_MifLoadKey(TRUE, TRUE, idx, key_b);
		if (rc != MI_OK)
		{
			printf("Load key B%02d to EEPROM [Failed]\n", idx);
			printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
			return rc;
		}
	}

	return MI_OK;
}

SWORD reader_load_default_keys(void)
{
	static const BYTE KEYS[3][6] = { { 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5 },
									 { 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5 },
									 { 0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7 } };

	BYTE idx;
	BYTE key_v[6];
	SWORD rc;

	/* Base key A */
	memcpy(key_v, KEYS[0], 6);
	rc = SPROX_MifLoadKey(TRUE, FALSE, 0, key_v);
	if (rc != MI_OK) goto failed;

	/* Base key B */
	memcpy(key_v, KEYS[1], 6);
	rc = SPROX_MifLoadKey(TRUE, TRUE, 0, key_v);
	if (rc != MI_OK) goto failed;

	/* Transport key FF */
	memset(key_v, 0xFF, 6);
	rc = SPROX_MifLoadKey(TRUE, FALSE, 1, key_v);
	if (rc != MI_OK) goto failed;
	rc = SPROX_MifLoadKey(TRUE, TRUE, 1, key_v);
	if (rc != MI_OK) goto failed;

	/* NFC key */
	memcpy(key_v, KEYS[2], 6);
	rc = SPROX_MifLoadKey(TRUE, FALSE, 2, key_v);
	if (rc != MI_OK) goto failed;
	rc = SPROX_MifLoadKey(TRUE, TRUE, 2, key_v);
	if (rc != MI_OK) goto failed;

	/* Blank key */
	memset(key_v, 0x00, 6);
	rc = SPROX_MifLoadKey(TRUE, FALSE, 3, key_v);
	if (rc != MI_OK) goto failed;
	rc = SPROX_MifLoadKey(TRUE, TRUE, 3, key_v);
	if (rc != MI_OK) goto failed;

	/* Dummy keys */
	memset(key_v, 0xCC, 6);
	for (idx = 4; idx < 16; idx++)
	{
		rc = SPROX_MifLoadKey(TRUE, FALSE, idx, key_v);
		if (rc != MI_OK) goto failed;
		rc = SPROX_MifLoadKey(TRUE, TRUE, idx, key_v);
		if (rc != MI_OK) goto failed;
	}

	return MI_OK;

failed:
	printf("Load key to EEPROM [Failed]\n");
	printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
	return rc;
}


SWORD reader_erase_keys(void)
{
	BYTE idx;
	BYTE key_cc[6];
	SWORD rc;

	memset(key_cc, 0xCC, 6);

	for (idx = 0; idx < 16; idx++)
	{
		rc = SPROX_MifLoadKey(TRUE, FALSE, idx, key_cc);
		if (rc != MI_OK)
		{
			printf("Load key A%02d to EEPROM [Failed]\n", idx);
			printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
			return rc;
		}

		rc = SPROX_MifLoadKey(TRUE, TRUE, idx, key_cc);
		if (rc != MI_OK)
		{
			printf("Load key B%02d to EEPROM [Failed]\n", idx);
			printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
			return rc;
		}
	}

	return MI_OK;
}

SWORD card_read_failure(BYTE sectors)
{
	BYTE data[240];
	BYTE sector;
	SWORD rc;

	for (sector = 0; sector < sectors; sector++)
	{
		rc = SPROX_MifStReadSector(NULL, sector, data, NULL);
		if (rc == MI_OK)
		{
			printf("Read sector %02d is OK, but it shouldn't!!!", sector);
			return 1;
		}
		if (rc != MI_AUTHERR)
		{
			printf("Read sector %02d ", sector);
			printf("[Failed]\n");
			printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
			return rc;
		}
	}

	return MI_OK;
}

SWORD card_read_test_00(BYTE sectors, BOOL transport)
{
	BYTE data[240];
	BYTE key_a[6];
	BYTE sector;
	BYTE offset, length;
	SWORD rc;

	for (sector = 0; sector < sectors; sector++)
	{
		memset(data, 0xCC, sizeof(data));

		if (!transport && (sector >= 16))
		{
			compute_test_keys(sector, key_a, NULL);
			rc = SPROX_MifStReadSector(NULL, sector, data, key_a);
		}
		else
		{
			rc = SPROX_MifStReadSector(NULL, sector, data, NULL);
		}
		if (rc != MI_OK)
		{
			printf("Read sector %02d ", sector);
			printf("[Failed]\n");
			printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
			return rc;
		}

		if (sector < 32) length = 48; else length = 240;
		if (sector == 0) offset = 16; else offset = 0;

		for (; offset < length; offset++)
		{
			if (data[offset] != 0x00)
			{
				printf("Sector %02d, expecting only 00, found %02X at offset %d\n", sector, data[offset], offset);
				return 1;
			}
		}
	}

	rc = SPROX_MifStReadBlock(NULL, 0, data, key_cc);
	if (rc != MI_AUTHERR)
	{
		printf("Read sector %02d -> %d\n", sector, rc);
		return 1;
	}

	return MI_OK;
}

SWORD card_read_test_xx(BYTE sectors)
{
	BYTE data[240];
	BYTE key_a[6];
	BYTE sector;
	BYTE offset, length;
	SWORD rc;

	for (sector = 0; sector < sectors; sector++)
	{
		memset(data, 0xCC, sizeof(data));

		if (sector >= 16)
		{
			compute_test_keys(sector, key_a, NULL);
			rc = SPROX_MifStReadSector(NULL, sector, data, key_a);
		}
		else
		{
			rc = SPROX_MifStReadSector(NULL, sector, data, NULL);
		}
		if (rc != MI_OK)
		{
			printf("Read sector %02d ", sector);
			printf("[Failed]\n");
			printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
			return rc;
		}

		if (sector < 32) length = 48; else length = 240;
		if (sector == 0) offset = 16; else offset = 0;

		for (; offset < length; offset++)
		{
			BYTE v = 0x80 ^ sector ^ offset;
			if (data[offset] != v)
			{
				printf("Sector %02d, found %02X instead of %02X at offset %d\n", sector, data[offset], v, offset);
				return 1;
			}
		}
	}

	rc = SPROX_MifStReadBlock(NULL, 0, data, key_cc);
	if (rc != MI_AUTHERR)
	{
		printf("Read sector %02d -> %d\n", sector, rc);
		return 1;
	}

	return MI_OK;
}

SWORD card_write_test_xx(BYTE sectors)
{
	BYTE data[240];
	BYTE key_b[6];
	BYTE sector;
	BYTE offset;
	SWORD rc;

	for (sector = 0; sector < sectors; sector++)
	{
		for (offset = 0; offset < 240; offset++)
			data[offset] = 0x80 ^ sector ^ offset;

		if (sector >= 16)
		{
			compute_test_keys(sector, NULL, key_b);
			rc = SPROX_MifStWriteSector(NULL, sector, data, key_b);
		}
		else
		{
			rc = SPROX_MifStWriteSector(NULL, sector, data, NULL);
		}
		if (rc != MI_OK)
		{
			printf("Write sector %02d ", sector);
			printf("[Failed]\n");
			printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
			return rc;
		}
	}

	rc = SPROX_MifStReadBlock(NULL, 0, data, key_cc);
	if (rc != MI_AUTHERR)
	{
		printf("Read sector %02d -> %d\n", sector, rc);
		return 1;
	}

	return MI_OK;
}

SWORD card_value_test_ex(BYTE block, SDWORD initial_value)
{
	SDWORD value;
	SWORD rc;

	/* Write the value into the block */
	/* ------------------------------ */

	rc = SPROX_MifStWriteCounter(NULL, block, initial_value, key_bb);
	if (rc != MI_OK)
	{
		printf("Write value on block %02d [Failed]\n", block);
		printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
		return rc;
	}

	/* Verify the value (using key A) */
	/* ------------------------------ */

	rc = SPROX_MifStReadCounter(NULL, block, &value, key_aa);
	if (rc != MI_OK)
	{
		printf("Read value from block %02d with key A [Failed]\n", block);
		printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
		return rc;
	}

	if (value != initial_value)
	{
		printf("Bad initial value (%ld instead of %ld) on block %02d\n", value, initial_value, block);
		return 1;
	}

	/* Decrement the block using key A, then key B */
	/* ------------------------------------------- */

	rc = SPROX_MifStDecrementCounter(NULL, block, 1260, key_aa);
	if (rc != MI_OK)
	{
		printf("Decrement block %02d with key A [Failed] (1260)\n", block);
		printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
		return rc;
	}

	rc = SPROX_MifStDecrementCounter(NULL, block, 256, key_aa);
	if (rc != MI_OK)
	{
		printf("Decrement block %02d with key A [Failed] (256)\n", block);
		printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
		return rc;
	}

	rc = SPROX_MifStDecrementCounter(NULL, block, 512, key_bb);
	if (rc != MI_OK)
	{
		printf("Decrement block %02d with key A [Failed] (512)\n", block);
		printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
		return rc;
	}

	/* Increment the block using key B */
	/* ------------------------------- */

	rc = SPROX_MifStIncrementCounter(NULL, block, 256, key_bb);
	if (rc != MI_OK)
	{
		printf("Increment block %02d with key B [Failed] (256)\n", block);
		printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
		return rc;
	}

	rc = SPROX_MifStIncrementCounter(NULL, block, 512, key_bb);
	if (rc != MI_OK)
	{
		printf("Increment block %02d with key B [Failed] (512)\n", block);
		printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
		return rc;
	}

	rc = SPROX_MifStIncrementCounter(NULL, block, 1260, key_bb);
	if (rc != MI_OK)
	{
		printf("Increment block %02d with key B [Failed] (1260)\n", block);
		printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
		return rc;
	}

	/* Try to increment the block using key A. This MUST fail */
	/* ------------------------------------------------------ */

	rc = SPROX_MifStIncrementCounter(NULL, block, 10, key_aa);
	if (rc == MI_OK)
	{
		printf("Increment block %02d with key A succeeded, this is an error!\n", block);
		printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
		return 1;
	}
	printf("(%d)\n", rc);

	/* Decrement the block using key A */
	/* ------------------------------- */

	rc = SPROX_MifStDecrementCounter(NULL, block, 10, key_aa);
	if (rc != MI_OK)
	{
		printf("Decrement block %02d with key A [Failed] (10)\n", block);
		printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
		return rc;
	}

	/* Increment the block using key B */
	/* ------------------------------- */

	rc = SPROX_MifStIncrementCounter(NULL, block, 10, key_bb);
	if (rc != MI_OK)
	{
		printf("Increment block %02d with key B [Failed] (10)\n", block);
		printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
		return rc;
	}

	/* Verify the value (using key B) */
	/* ------------------------------ */

	rc = SPROX_MifStReadCounter(NULL, block, &value, key_bb);
	if (rc != MI_OK)
	{
		printf("Read value from block %02d with key B [Failed]\n", block);
		printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
		return rc;
	}

	if (value != initial_value)
	{
		printf("Bad final value (%ld instead of %ld) on block %02d\n", value, initial_value, block);
		return 1;
	}

	return MI_OK;
}

SWORD card_value_test(BYTE sector)
{
	const SDWORD initial_value = 1234567;
	BYTE data[48];
	SWORD rc;
	BYTE block = 4 * sector;
	SDWORD value;

	/* Format the blocks 4 and 5 to hold a counter. Block 6 remains a standard data block */
	/* ---------------------------------------------------------------------------------- */

	rc = SPROX_MifStUpdateAccessBlock(NULL, sector, key_ff, key_aa, key_bb, ACC_BLOCK_COUNTER, ACC_BLOCK_COUNTER, ACC_BLOCK_DATA, ACC_AUTH_NORMAL);
	if (rc != MI_OK)
	{
		printf("Format (for value test) sector %02d [Failed] A\n", sector);
		printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
		return rc;
	}

	rc = SPROX_MifStUpdateAccessBlock(NULL, sector, key_bb, key_ff, key_ff, ACC_BLOCK_COUNTER, ACC_BLOCK_COUNTER, ACC_BLOCK_DATA, ACC_AUTH_NORMAL);
	if (rc != MI_OK)
	{
		printf("Format (for value test) sector %02d [Failed] B\n", sector);
		printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
		return rc;
	}

	rc = SPROX_MifStUpdateAccessBlock(NULL, sector, key_ff, key_aa, key_bb, ACC_BLOCK_COUNTER, ACC_BLOCK_COUNTER, ACC_BLOCK_DATA, ACC_AUTH_NORMAL);
	if (rc != MI_OK)
	{
		printf("Format (for value test) sector %02d [Failed] C\n", sector);
		printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
		return rc;
	}

	/* Test with a positive value on block 4 */
	/* ------------------------------------- */

	rc = card_value_test_ex(block, initial_value);
	if (rc != MI_OK)
		return rc;

	/* Test with a negative value on block 5 */
	/* ------------------------------------- */

	rc = card_value_test_ex((BYTE)(block + 1), 0 - initial_value);
	if (rc != MI_OK)
		return rc;

	/* Try to use block 6 as counter */
	/* ----------------------------- */

	rc = SPROX_MifStWriteCounter(NULL, (BYTE)(block + 2), initial_value, key_bb);
	if (rc != MI_OK)
	{
		printf("Write value on block %02d [Failed]\n", block + 2);
		printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
		return rc;
	}

	rc = SPROX_MifStDecrementCounter(NULL, (BYTE)(block + 2), 1, key_aa);
	if (rc == MI_OK)
	{
		printf("Decrement block %02d with key A succeeded, this is an error!\n", block + 2);
		printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
		return 1;
	}
	printf("(%d)\n", rc);

	/* Copy block 4 to block 5 */
	/* ----------------------- */

	rc = SPROX_MifStRestoreCounter(NULL, block, (BYTE)(block + 1), key_bb);
	if (rc != MI_OK)
	{
		printf("Restore block %02d on block %02d with key B [Failed]\n", block, block + 1);
		printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
		return rc;
	}

	rc = SPROX_MifStRestoreCounter(NULL, block, (BYTE)(block + 1), key_aa);
	if (rc != MI_OK)
	{
		printf("Restore block %02d on block %02d with key A [Failed]\n", block, block + 1);
		printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
		return rc;
	}

	rc = SPROX_MifStReadCounter(NULL, (BYTE)(block + 1), &value, key_aa);
	if (rc != MI_OK)
	{
		printf("Read value from block %02d with key A [Failed]\n", block + 1);
		printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
		return rc;
	}

	if (value != initial_value)
	{
		printf("Bad restored value (%ld instead of %ld) on block %02d\n", value, initial_value, block + 1);
		return 1;
	}

	/* Restore the sector */
	/* ------------------ */

	memset(data, 0, 48);
	rc = SPROX_MifStWriteSector(NULL, sector, data, key_bb);
	if (rc != MI_OK)
	{
		printf("Write sector %02d [Failed]\n", sector);
		printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
		return rc;
	}


	rc = SPROX_MifStUpdateAccessBlock(NULL, sector, key_bb, key_ff, key_ff, ACC_BLOCK_DATA, ACC_BLOCK_DATA, ACC_BLOCK_DATA, ACC_AUTH_TRANSPORT);
	if (rc != MI_OK)
	{
		printf("Format (back to normal) sector %02d [Failed]\n", sector);
		printf("  %s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
		return rc;
	}

	return MI_OK;
}
