/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  Copyright (c) 2000-2014 PRO ACTIVE SAS, FRANCE - www.springcard.com

  ref_mifplus.c
  -------------

  This is the reference applications that validates the Sprox_MifPlus.dll
  with NXP Mifare Plus cards.

  JDA 23/02/2011 : first release
  JDA 24/01/2012 : minor changes to adapt to release 1.62 of the SDK
  JDA 04/02/2013 : minor changes to adapt to release 1.7x of the SDK
  JDA 13/08/2014 : moved to Visual C++ Express 2010, added the 'A'
				   suffix to all text-related functions
  JDA 05/09/2023 : refreshed the project to build with Visual Studio 2022

  IMPORTANT NOTICE:

	Due to the difficulties to get the different cards in the Mifare
	Plus family, this software has been fully tested only with some
	Mifare Plus X 4K cards.
	Please signal us any trouble that may appear with Mifare Plus S
	or with 2K cards.

	To date, only the READ / WRITE features of Level 3 are supported.
	Level 2 is not supported (as it doesn't exist in Mifare Plus S).
	VALUE features will be implemented in a later version.

*/
#include "products/springprox/api/springprox.h"
#include "cardware/mifplus/sprox_mifplus.h"

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const char* PROGRAM_NAME = "ref_mifplus";
const char* szCommDevice = NULL;
WORD wRunMode = 0;
BYTE dCID = 0xFF;
BYTE dDSI = 0;
BYTE dDRI = 0;
BOOL fConfirmation = FALSE;

#define RUN_SL0_MASK    0x000F
#define RUN_SL1_MASK    0x08F0
#define RUN_SL2_MASK    0x0F00
#define RUN_SL3_MASK    0xF000

#define RUN_PERSO       0x0001
#define RUN_SET_RID     0x0002
#define RUN_COMMIT      0x0008

#define RUN_TEST_SL1    0x0010
#define RUN_TEST_SL2    0x0100
#define RUN_TEST_SL3    0x1000

#define RUN_GO_SL2      0x0080
#define RUN_GO_SL3      0x0800

static void usage(void)
{
	printf("Usage: %s [ACTION] [BAUDRATES] [OPTIONS] [-d <COMM DEVICE]\n", PROGRAM_NAME);
	printf("ACTION could be:\n");
	printf("* If the card is in SL0:\n");
	printf("  -perso  : set the test AES keys\n");
	printf("  -rid    : configure for Random ID\n");
	printf("  -commit : go to SL1\n");
	printf("* If the card is in SL1:\n");
	printf("  -sl1    : test the functions of this level\n");
	printf("  -go2    : go to SL2\n");
	printf("  -go3    : go to SL3\n");
	printf("* If the card is in SL2:\n");
	printf("  -sl2    : test the functions of this level\n");
	printf("  -go3    : go to SL3\n");
	printf("* If the card is in SL3:\n");
	printf("  -sl3    : test the functions of this level\n");
	printf("BAUDRATES:\n");
	printf(" -dr0, -dr1, -dr2 or -dr3 for PCD->PICC baudrate\n");
	printf(" -ds0, -ds1, -ds2 or -ds3 for PICC->PCD baudrate\n");
	printf("Default is -dr0 -ds0 (106kbit/s both directions)\n");
	printf("OPTIONS:\n");
	printf(" -y : skip confirmation prompts\n");
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
			else if (!strncmp(argv[i], "-y", 2))
			{
				fConfirmation = TRUE;
			}
			else if (!strcmp(argv[i], "-rid"))
			{
				wRunMode |= RUN_SET_RID;
			}
			else if (!strcmp(argv[i], "-perso"))
			{
				wRunMode |= RUN_PERSO;
			}
			else if (!strcmp(argv[i], "-commit"))
			{
				wRunMode |= RUN_COMMIT;
			}
			else if (!strcmp(argv[i], "-sl1"))
			{
				wRunMode |= RUN_TEST_SL1;
			}
			else if (!strcmp(argv[i], "-sl2"))
			{
				wRunMode |= RUN_TEST_SL2;
			}
			else if (!strcmp(argv[i], "-sl3"))
			{
				wRunMode |= RUN_TEST_SL3;
			}
			else if (!strcmp(argv[i], "-go2"))
			{
				wRunMode |= RUN_GO_SL2;
			}
			else if (!strcmp(argv[i], "-go3"))
			{
				wRunMode |= RUN_GO_SL3;
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


BOOL MifPlus_Main(void);
BOOL MifPlus_L0_Perso(void);
BOOL MifPlus_L0_SetRid(void);
BOOL MifPlus_L0_Commit(void);
BOOL MifPlus_L1_Test(void);
BOOL MifPlus_L1_Go_L2(void);
BOOL MifPlus_L1_Go_L3(void);
BOOL MifPlus_L2_Test(void);
BOOL MifPlus_L2_Go_L3(void);
BOOL MifPlus_L3_Test(void);
BOOL MifPlus_FirstAuthentication(WORD key_address);
BOOL MifPlus_FollowingAuthentication(WORD key_address, BOOL* last_block_reached);

static BOOL confirm(void)
{
	char s[16];
	int i;

	for (;;)
	{
		printf("\nPlease enter YES to confirm or NO to exit: ");
		if (fgets(s, sizeof(s), stdin))
		{
			for (i = 0; i < sizeof(s); i++)
			{
				if (s[i] == '\r') s[i] = '\0';
				if (s[i] == '\n') s[i] = '\0';
				if (s[i] == '\0') break;
				s[i] |= 0x40;
			}
			if (!strcmp(s, "yes"))
			{
				printf("\n");
				return TRUE;
			}
			if (!strcmp(s, "no"))
			{
				printf("\n");
				return FALSE;
			}
		}
	}
}

static const BYTE mfp_ats_begin[] = MFP_ATS_BEGIN;
static const BYTE mfp_ats_sample[] = MFP_ATS_SAMPLE;

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
	BYTE ats[32];
	BYTE ats_len = 32;
	BYTE ats_offset;
	BOOL is_mifare_plus = FALSE;
	BOOL success = FALSE;
	BYTE level;

	/* Display the informations and check the command line */
	/* --------------------------------------------------- */

	printf("SpringCard SpringProx 'Legacy' SDK\n");
	printf("\n");
	printf("%s : NXP MIFARE Plus validation & sample\n\n", PROGRAM_NAME);
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
	if (rc == MI_OK)
		printf("Reader found on %s\n", s_buffer);

	rc = SPROX_ReaderGetFirmwareA(s_buffer, sizeof(s_buffer));
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

	/* Enter T=CL (even if SAK doesn't tell that the tag supports it) */
	/* -------------------------------------------------------------- */

	ats_len = sizeof(ats);
	rc = SPROX_TclA_GetAts(dCID, ats, &ats_len);
	if (rc != MI_OK)
	{
		printf("GetAts failed; is it really a Mifare Plus?\n");
		goto close;
	}

	if (dDSI || dDRI)
	{
		rc = SPROX_TclA_Pps(dCID, dDSI, dDRI);
		if (rc != MI_OK)
		{
			printf("PPS failed.\n");
			goto close;
		}
		printf("PPS OK\n");
	}

	printf("          ATS=");
	for (i = 0; i < ats_len; i++)
		printf("%02X", ats[i]);
	printf("\n");

	/* Compare the ATS to our list of known ATS */
	if ((ats_len >= sizeof(mfp_ats_begin)) && !memcmp(mfp_ats_begin, ats, sizeof(mfp_ats_begin)))
	{
		is_mifare_plus = TRUE;
	}

	if ((ats_len == sizeof(mfp_ats_sample)) && !memcmp(mfp_ats_sample, ats, sizeof(mfp_ats_sample)))
	{
		printf("This is a Mifare Plus Engineering sample!\n");
		is_mifare_plus = TRUE;
	}
	else
	{
		/* Parse the ATS */
		ats_offset = 1;
		if (ats[0] & 0x10) ats_offset++; /* TA1 present */
		if (ats[0] & 0x20) ats_offset++; /* TB1 present */
		if (ats[0] & 0x40) ats_offset++; /* TC1 present */

		printf("   Hist.Bytes=");
		for (i = 0; i < ats_offset; i++)
			printf("  ");
		for (i = ats_offset; i < ats_len; i++)
			printf("%02X", ats[i]);
		printf("\n");

		if ((ats[ats_offset] == 0xC1)
			&& (ats[ats_offset] >= 5)
			&& (ats_len > ats_offset + 5))
		{
			/* Mifare family or virtual card */
			ats_offset += 2;
			switch (ats[ats_offset] & 0xF0)
			{
			case 0x00: printf("\tMifare Virtual card(s)\n"); break;
			case 0x10: printf("\tMifare Desfire\n"); break;
			case 0x20: printf("\tMifare Plus\n"); is_mifare_plus = TRUE; break;
			default: printf("\tMifare: RFU\n"); break;
			}
			switch (ats[ats_offset] & 0x0F)
			{
			case 0x00: printf("\tMemory: <1kB\n"); break;
			case 0x01: printf("\tMemory: 1kB\n"); break;
			case 0x02: printf("\tMemory: 2kB\n"); break;
			case 0x03: printf("\tMemory: 4kB\n"); break;
			case 0x04: printf("\tMemory: 8kB\n"); break;
			case 0x0F: break;
			default: printf("\tMemory: RFU\n"); break;
			}
			ats_offset++;
			switch (ats[ats_offset] & 0xF0)
			{
			case 0x00: printf("\tEngineering sample\n"); break;
			case 0x20: printf("\tFinal silicon\n"); break;
			default: printf("\tSilicon status: RFU\n"); break;
			}
			switch (ats[ats_offset] & 0x0F)
			{
			case 0x00: printf("\tVersion 1\n"); break;
			case 0x01: printf("\tVersion 2\n"); break;
			case 0x02: printf("\tVersion 3\n"); break;
			case 0x0F: break;
			default: printf("\tVersion: RFU\n"); break;
			}
			ats_offset++;
			if ((ats[ats_offset] & 0x0E) == 0x00)
			{
				printf("\tLevel 1, Level 2 or Level 3\n");
			}
			else if ((ats[ats_offset] & 0x0E) == 0x02)
			{
				printf("\tLevel 3 only\n");
			}

			if ((ats[ats_offset] & 0x0F) == 0x0E)
			{
				printf("\tNo Virtual Card support\n");
			}
			else if ((ats[ats_offset] & 0x09) == 0x00)
			{
				printf("\tVirtualCardSupportLast only\n");
			}
			else if ((ats[ats_offset] & 0x09) == 0x01)
			{
				printf("\tVirtualCardSupport, VirtualCardSupportLast and SelectVirtualCard\n");
			}
		}
	}

	if (!is_mifare_plus)
	{
		printf("This is not the ATS of a Mifare Plus?\n");
		goto close;
	}

	/* Prepare the library for card operation */
	/* -------------------------------------- */

  /* Give the library the information it needs to find the card's Level */
	rc = SPROX_MifPlus_SelectCard(sak, ats, ats_len);
	if (rc != MI_OK)
	{
		printf("SPROX_MifPlus_SelectCard failed\n");
		goto close;
	}

	/* Retrieve the security Level of the card */
	rc = SPROX_MifPlus_GetCardLevel(&level);
	if (rc != MI_OK)
	{
		printf("SPROX_MifPlus_GetCardLevel failed\n");
		goto close;
	}

	if ((level == 1) || (level == 2))
	{
		/* The card MUST leave T=CL mode for Level 1 or 2 operation */
		rc = SPROX_MifPlus_LeaveTcl();
		if (rc != MI_OK)
		{
			printf("SPROX_MifPlus_LeaveTcl failed\n");
			goto close;
		}
	}

	/* And now... do the test(s) */
	switch (level)
	{
	case 0:

		/* Handling of a card at Level 0 */
		/* ----------------------------- */

		printf("\tThis card seems to be a Mifare Plus at Level 0 (out of factory)\n");

		if (!(wRunMode & RUN_SL0_MASK))
		{
			printf("\n");
			printf("Nothing to do here - Try running the program specifying:\n");
			printf("\t-rid to enable Random ID\n");
			printf("\t-perso write the AES keys for the test\n");
			printf("\t-commit to switch the card from Level 0 to Level 1\n");
			printf("\n");
		}

		if (wRunMode & RUN_SET_RID)
		{
			success = MifPlus_L0_SetRid();
			if (!success) break;
		}

		if (wRunMode & RUN_PERSO)
		{
			success = MifPlus_L0_Perso();
			if (!success) break;
		}

		if (wRunMode & RUN_COMMIT)
		{
			/* The user asks to enter Level 1 */
			if (!fConfirmation)
			{
				printf("We're now ready to COMMIT the personalization of this card and have it enter\n");
				printf("Level 1.\n");
				printf("Once this is done, IT IS NOT POSSIBLE to go back to Level 0, nor to change\n");
				printf("the keys neither the global parameters that have been defined.\n");
				printf("Are you really sure you want to do this ?\n");
				if (!confirm())
					exit(EXIT_FAILURE);
			}

			/* Commit the personalization. Card enters Level 1. THIS IS NOT REVERSIBLE */
			rc = SPROX_MifPlus_CommitPerso();
			if (rc)
			{
				printf("SPROX_MifPlus_CommitPerso failed, rc=%ld\n", rc);
				success = FALSE;
				break;
			}
			else
				success = TRUE;

			printf("The card has been personalized, you must now remove it from the reader.\n\n");
		}

		break;

	case 1:
		/* Handling of a card at Level 1 */
		/* ----------------------------- */

		printf("\tThis card seems to be a Mifare Plus at Level 1\n\t(Mifare Classic emulation)\n");

		if (!(wRunMode & RUN_SL1_MASK))
		{
			printf("\n");
			printf("Nothing to do here - Try running the program specifying:\n");
			printf("\t-sl1 to perform the Level 1-related tests\n\t(Mifare emulation and AES authentication)\n");
			printf("\t-go2 to switch the card from Level 1 to Level 2\n");
			printf("\t-go3 to switch the card from Level 1 to Level 3\n");
			printf("\n");
		}

		if (wRunMode & RUN_TEST_SL1)
		{
			/* Test */
			success = MifPlus_L1_Test();
			if (!success) break;
		}

		if (wRunMode & RUN_GO_SL3)
		{
			/* The user asks to enter Level 3 */
			success = MifPlus_L1_Go_L3();
			if (!success) break;
		}
		else
			if (wRunMode & RUN_GO_SL2)
			{
				/* The user asks to enter Level 2 */
				success = MifPlus_L1_Go_L2();
				if (!success) break;
			}

		break;

	case 2:

		/* Handling of a card at Level 2 */
		/* ----------------------------- */

		printf("\tThis card seems to be a Mifare Plus at Level 2\n");

		if (!(wRunMode & RUN_SL2_MASK))
		{
			printf("\n");
			printf("Nothing to do here - Try running the program specifying:\n");
			printf("\t-sl2 to perform the Level 2-related tests\n\n");
			printf("\t-go3 to switch the card from Level 2 to Level 3\n");
			printf("\n");
		}

		if (wRunMode & RUN_TEST_SL2)
		{
			/* Test */
			success = MifPlus_L2_Test();
			if (!success) break;
		}

		if (wRunMode & RUN_GO_SL3)
		{
			/* The user asks to enter Level 3 */
			success = MifPlus_L2_Go_L3();
			if (!success) break;
		}

		break;


	case 3:

		/* Handling of a card at Level 3 */
		/* ----------------------------- */

		printf("\tThis card seems to be a Mifare Plus at Level 3\n");

		if (!(wRunMode & RUN_SL3_MASK))
		{
			printf("\n");
			printf("Nothing to do here - Try running the program specifying:\n");
			printf("\t-sl3 to perform the Level 3-related tests\n\n");
			printf("\n");
		}

		if (wRunMode & RUN_TEST_SL3)
		{
			/* Test */
			success = MifPlus_L3_Test();
			if (!success) break;
		}

		break;

	default: printf("\tLevel %d is unsupported!\n", level);

	}

	if (success)
		printf("SUCCESS!\n");

close:
	SPROX_ControlRF(FALSE);
	SPROX_ReaderClose();

done:
	/* Display last error */
	if (rc != MI_OK)
		printf("%s (%ld)\n", SPROX_GetErrorMessageA(rc), rc);

	return EXIT_SUCCESS;
}



/*
 *****************************************************************************
 *
 * Initial personalisation of the Level 0 card
 *
 *****************************************************************************
 *
 * The card comes out of factory at 'Level 0'. All user-defined keys and
 * parameters shall be defined before entering one of the upper levels.
 *
 * We use an array of MIFPLUS_PERSO_ST structure to define the keys and
 * parameters to be writen when function MifPlus_L0_Perso runs.
 * Adapt the content of this array to your own values.
 *
 * DO NOT USE THOSE SAMPLE KEYS AND PARAMETERS IN A REAL-WORLD APPLICATION.
 *
 * ONCE AGAIN, REMEMBER THAT MOST OF THE KEYS AND PARAMETERS CAN'T BE
 * CHANGED AFTERWARDS. SWITCHING TO AN UPPER LEVEL IS NOT REVERSIBLE.
 * DO NOT USE THIS SOFTWARE AND YOUR CARD WITHOUT KNOWING WHAT IT WILL BE
 * DOING, AND WHY YOU ACTUALLY WANT TO HAVE IT DONE.
 *
 */

typedef struct
{
	BYTE option;
	WORD address;
	BYTE value[16];
} MIFPLUS_PERSO_ST;

/*
 * The MIFPLUS_PERSO_ST.option field tells whether the option is applicable
 * to Mifare Plus X or Mifare Plus S, or both, and whether it is applicable
 * to Mifare Plus 2K or Mifare Plus 4K.
 * The following defines shall be used:
 */

 /* Option applicable to Mifare Plus X only */
#define OPT_ONLY_X 0x02
/* Option applicable to either Mifare Plus X or S */
#define OPT_X_OR_S 0x03
/* Option applicable to Mifare Plus 2K or more */
#define OPT_GTE_2K 0x10
/* Option applicable to Mifare Plus 4K only */
#define OPT_GTE_4K 0x20

/*
 * Our array of MIFPLUS_PERSO_ST holds all the keys and parameter we
 * want to put in our card, for test and demonstration purpose.
 * Adapt the content of this array to your own values.
 *
 * DO NOT USE THOSE SAMPLE KEYS AND PARAMETERS IN A REAL-WORLD APPLICATION.
 *
 * Diversification of the keys, using the cards' serial numbers as seed, is
 * a good idea to enhance the overall security of your system.
 * Anyway, ALWAYS KEEP 'VC POLLING ENC' AND 'VC POLLING MAC' KEYS AS CONSTANT,
 * because once randon-id and VC feature are activated, you won't be able to
 * retrieve the card's serial number without a successfull VC handshaking...
 */
static const MIFPLUS_PERSO_ST mifplus_perso[] =
{
	/* Card Master Key */
	{ OPT_X_OR_S, 0x9000, { "Card master key " }},
	/* Card Configuration Key */
  { OPT_X_OR_S, 0x9001, { "Card config key " }},
  /* Level 2 switch key */
{ OPT_ONLY_X, 0x9002, { "Go to level 2!  " }},
/* Level 3 switch key */
{ OPT_X_OR_S, 0x9003, { "Go to level 3!  " }},
/* SL1 Card Authentication Key */
{ OPT_X_OR_S, 0x9004, { "Level 1 AES auth" }},
/* Select VC key */
{ OPT_ONLY_X, 0xA000, { "Select VC key   " }},
/* Proximity check key */
{ OPT_ONLY_X, 0xA001, { "Proximity check " }},

/* VC polling ENC key - must be constant among the system */
{ OPT_X_OR_S, 0xA080, { "VC polling ENC  " }},
/* VC polling MAC key - must be constant among the system */
{ OPT_X_OR_S, 0xA081, { "VC polling MAC  " }},

/* Mifare sector keys - sectors 0 to 31 */
{ OPT_GTE_2K, 3, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 7, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 11, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 15, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 19, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 23, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 27, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 31, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 35, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 39, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 43, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 47, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 51, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 55, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 59, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 63, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 67, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 71, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 75, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 79, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 83, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 87, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 91, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 95, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 99, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 103, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 107, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 111, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 115, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 119, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 123, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_2K, 127, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},

/* Mifare sector keys - sectors 32 to 39 */
{ OPT_GTE_4K, 143, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_4K, 159, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_4K, 175, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_4K, 191, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_4K, 207, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_4K, 223, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_4K, 239, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},
{ OPT_GTE_4K, 255, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff, 0x07, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }},

/* AES sector keys - sectors 0 to 31 */
{ OPT_GTE_2K, 0x4000, { "AES key A sect00" }}, { OPT_GTE_2K, 0x4001, { "AES key B sect00" }},
{ OPT_GTE_2K, 0x4002, { "AES key A sect01" }}, { OPT_GTE_2K, 0x4003, { "AES key B sect01" }},
{ OPT_GTE_2K, 0x4004, { "AES key A sect02" }}, { OPT_GTE_2K, 0x4005, { "AES key B sect02" }},
{ OPT_GTE_2K, 0x4006, { "AES key A sect03" }}, { OPT_GTE_2K, 0x4007, { "AES key B sect03" }},
{ OPT_GTE_2K, 0x4008, { "AES key A sect04" }}, { OPT_GTE_2K, 0x4009, { "AES key B sect04" }},
{ OPT_GTE_2K, 0x400A, { "AES key A sect05" }}, { OPT_GTE_2K, 0x400B, { "AES key B sect05" }},
{ OPT_GTE_2K, 0x400C, { "AES key A sect06" }}, { OPT_GTE_2K, 0x400D, { "AES key B sect06" }},
{ OPT_GTE_2K, 0x400E, { "AES key A sect07" }}, { OPT_GTE_2K, 0x400F, { "AES key B sect07" }},
{ OPT_GTE_2K, 0x4010, { "AES key A sect08" }}, { OPT_GTE_2K, 0x4011, { "AES key B sect08" }},
{ OPT_GTE_2K, 0x4012, { "AES key A sect09" }}, { OPT_GTE_2K, 0x4013, { "AES key B sect09" }},
{ OPT_GTE_2K, 0x4014, { "AES key A sect10" }}, { OPT_GTE_2K, 0x4015, { "AES key B sect10" }},
{ OPT_GTE_2K, 0x4016, { "AES key A sect11" }}, { OPT_GTE_2K, 0x4017, { "AES key B sect11" }},
{ OPT_GTE_2K, 0x4018, { "AES key A sect12" }}, { OPT_GTE_2K, 0x4019, { "AES key B sect12" }},
{ OPT_GTE_2K, 0x401A, { "AES key A sect13" }}, { OPT_GTE_2K, 0x401B, { "AES key B sect13" }},
{ OPT_GTE_2K, 0x401C, { "AES key A sect14" }}, { OPT_GTE_2K, 0x401D, { "AES key B sect14" }},
{ OPT_GTE_2K, 0x401E, { "AES key A sect15" }}, { OPT_GTE_2K, 0x401F, { "AES key B sect15" }},
{ OPT_GTE_2K, 0x4020, { "AES key A sect16" }}, { OPT_GTE_2K, 0x4021, { "AES key B sect16" }},
{ OPT_GTE_2K, 0x4022, { "AES key A sect17" }}, { OPT_GTE_2K, 0x4023, { "AES key B sect17" }},
{ OPT_GTE_2K, 0x4024, { "AES key A sect18" }}, { OPT_GTE_2K, 0x4025, { "AES key B sect18" }},
{ OPT_GTE_2K, 0x4026, { "AES key A sect19" }}, { OPT_GTE_2K, 0x4027, { "AES key B sect19" }},
{ OPT_GTE_2K, 0x4028, { "AES key A sect20" }}, { OPT_GTE_2K, 0x4029, { "AES key B sect20" }},
{ OPT_GTE_2K, 0x402A, { "AES key A sect21" }}, { OPT_GTE_2K, 0x402B, { "AES key B sect21" }},
{ OPT_GTE_2K, 0x402C, { "AES key A sect22" }}, { OPT_GTE_2K, 0x402D, { "AES key B sect22" }},
{ OPT_GTE_2K, 0x402E, { "AES key A sect23" }}, { OPT_GTE_2K, 0x402F, { "AES key B sect23" }},
{ OPT_GTE_2K, 0x4030, { "AES key A sect24" }}, { OPT_GTE_2K, 0x4031, { "AES key B sect24" }},
{ OPT_GTE_2K, 0x4032, { "AES key A sect25" }}, { OPT_GTE_2K, 0x4033, { "AES key B sect25" }},
{ OPT_GTE_2K, 0x4034, { "AES key A sect26" }}, { OPT_GTE_2K, 0x4035, { "AES key B sect26" }},
{ OPT_GTE_2K, 0x4036, { "AES key A sect27" }}, { OPT_GTE_2K, 0x4037, { "AES key B sect27" }},
{ OPT_GTE_2K, 0x4038, { "AES key A sect29" }}, { OPT_GTE_2K, 0x4039, { "AES key B sect28" }},
{ OPT_GTE_2K, 0x403A, { "AES key A sect29" }}, { OPT_GTE_2K, 0x403B, { "AES key B sect29" }},
{ OPT_GTE_2K, 0x403C, { "AES key A sect30" }}, { OPT_GTE_2K, 0x403D, { "AES key B sect30" }},
{ OPT_GTE_2K, 0x403E, { "AES key A sect31" }}, { OPT_GTE_2K, 0x403F, { "AES key B sect31" }},
/* AES sector keys - sectors 32 to 39 */
{ OPT_GTE_4K, 0x4040, { "AES key A sect32" }}, { OPT_GTE_4K, 0x4041, { "AES key B sect32" }},
{ OPT_GTE_4K, 0x4042, { "AES key A sect33" }}, { OPT_GTE_4K, 0x4043, { "AES key B sect33" }},
{ OPT_GTE_4K, 0x4044, { "AES key A sect34" }}, { OPT_GTE_4K, 0x4045, { "AES key B sect34" }},
{ OPT_GTE_4K, 0x4046, { "AES key A sect35" }}, { OPT_GTE_4K, 0x4047, { "AES key B sect35" }},
{ OPT_GTE_4K, 0x4048, { "AES key A sect36" }}, { OPT_GTE_4K, 0x4049, { "AES key B sect36" }},
{ OPT_GTE_4K, 0x404A, { "AES key A sect37" }}, { OPT_GTE_4K, 0x404B, { "AES key B sect37" }},
{ OPT_GTE_4K, 0x404C, { "AES key A sect38" }}, { OPT_GTE_4K, 0x404D, { "AES key B sect38" }},
{ OPT_GTE_4K, 0x404E, { "AES key A sect39" }}, { OPT_GTE_4K, 0x404F, { "AES key B sect39" }},

/* MFP configuration block - see MifPlux X datasheet § 10.11 */
//{ OPT_ONLY_X, 0xB000, { ...define new MFP configuration block here... }},

  /* MFP installation identifier - see MifPlux X datasheet § 9.7.7 */
  { OPT_X_OR_S, 0xB001, { "SpringCard  test" }},

  /* ATS information - here we could modify the ATS. WE'D BETTER NOT!!! */
//{ OPT_X_OR_S, 0xB002, { ...define new ATS here... }},

  /* Field Configuration Block - see MifPlux X datasheet § 10.10 */
//{ OPT_X_OR_S, 0xB003, { ...define new Field Configuration Block here... }},
};

/* Return the value associated to an address in our array */
static const BYTE* get_perso_value(WORD address)
{
	int i;

	for (i = 0; i < (sizeof(mifplus_perso) / sizeof(mifplus_perso[0])); i++)
		if (mifplus_perso[i].address == address)
			return mifplus_perso[i].value;

	return NULL;
}

BOOL MifPlus_L0_SetRid(void)
{
	SWORD rc;
	/* Enable Random ID - see MifPlux X datasheet § 10.10 */
	BYTE FCB[16] = { 0x00,
					   0xAA, /* Use Random ID */
									 0x55, /* Proximity Check is not mandatory */
									 0x00,
									 0x00, /* Picc Cap 1.2 */
									 0x00, 0x00, 0x00, 0x00,
									 0x00, /* Picc Cap 2.5 */
									 0x00, /* Picc Cap 2.6 */
									 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	printf("Activating Random ID...\n");

	rc = SPROX_MifPlus_WritePerso(0xB003, FCB);
	if (rc)
	{
		printf("SPROX_MifPlus_WritePerso(%04X) failed, rc=%ld\n", 0xB003, rc);
		return FALSE;
	}

	return TRUE;
}

BOOL MifPlus_L0_Perso(void)
{
	BOOL is_s = FALSE;
	BOOL is_x = FALSE;
	BOOL is_2k = FALSE;
	BOOL is_4k = FALSE;
	int i;
	SWORD rc;

	printf("Doing Level 0 personalization...\n");

	for (i = 0; i < (sizeof(mifplus_perso) / sizeof(mifplus_perso[0])); i++)
	{
		/* Skip options not applicable to this card. */
		switch (mifplus_perso[i].option)
		{
		case OPT_ONLY_X: if (is_s)  continue;
		case OPT_GTE_4K: if (is_2k) continue;
		default: break;
		}

		rc = SPROX_MifPlus_WritePerso(mifplus_perso[i].address, mifplus_perso[i].value);
		if (rc)
		{
			if (rc == (MFP_ERROR - MFP_ERR_INVALID_BLOCK_NUMBER))
			{
				/* Invalid block number */
				switch (mifplus_perso[i].option)
				{
				case OPT_ONLY_X: if (is_x) break;        /* Card is supposed to be X. This failure is an error. */
					is_s = TRUE; continue;  /* Card is not X, so it is S. Not yet an error */

				case OPT_GTE_4K: if (is_4k) break;       /* Card is supposed to be 4K. This failure is an error. */
					is_2k = TRUE; continue; /* Card is not 4K, so it is 2K. Not yet an error */

				default: break;
				}
			}

			printf("SPROX_MifPlus_WritePerso(%04X) failed, rc=%ld\n", mifplus_perso[i].address, rc);
			return FALSE;
		}

		/* No error - This helps knowing what the card is */
		switch (mifplus_perso[i].option)
		{
		case OPT_ONLY_X: is_x = TRUE; break;        /* Only X cards could be OK for this command */
		case OPT_GTE_4K: is_4k = TRUE; break;        /* Only 4K cards could be OK for this command */

		default: break;
		}
	}

	if (is_s && is_x)
	{
		printf("Incoherent card behaviour - seem to be both Mifare Plus S and X\n");
		return FALSE;
	}

	if (is_2k && is_4k)
	{
		printf("Incoherent card behaviour - seem to be both Mifare Plus 2K and 4K\n");
		return FALSE;
	}


	return TRUE;
}

/*
 *****************************************************************************
 *
 * Test suite for the card running at Level 1
 *
 *****************************************************************************
 *
 * At Level 1, the Mifare Plus emulates a plain-old Mifare Classic. Therefore,
 * the basis is to pass the same tests as ref_mifare_pcsc.c.
 *
 * The Mifare Plus also features a 'SL1 Card Authentication Key' that may be
 * used to check whether the card is genuine or not (yet it doesn't protect
 * the data stored on it).
 * We start and end by performing such an authentication.
 *
 */

BOOL MifPlus_L1_Test(void)
{
	BYTE sectors = 40;

	/* Mifare Plus Level 1 AES Authentication */
	/* -------------------------------------- */

  /* Verify that the card is genuine */
	printf("Performing AES authentication with SL1 Card Authentication key\n");
	if (!MifPlus_FollowingAuthentication(0x9004, NULL))
		return FALSE;

	/* Not much more to do */
	printf("\nNow try ref_mifare to confirm that the Mifare Plus in SL1 is actually able to emulate a Mifare Classic...\n\n");

	return TRUE;
}

/* AES authentication to switch from Level 1 to Level 2 */
/* ---------------------------------------------------- */
BOOL MifPlus_L1_Go_L2(void)
{
	SWORD rc;

	if (!fConfirmation)
	{
		printf("We're now ready to put the card in Level 2.\n");
		printf("Once this is done, IT IS NOT POSSIBLE to go back to Level 1.\n");
		printf("Are you really sure you want to do this ?\n");
		if (!confirm())
			exit(EXIT_FAILURE);
	}

	/* In Level 1, the Level-switch authentication must be performed in T=CL */
	rc = SPROX_MifPlus_EnterTcl();
	if (rc)
	{
		printf("SPROX_MifPlus_EnterTcl failed, rc=%ld\n", rc);
		return FALSE;
	}

	/* The Level-switch authentication is a Following Authentication */
	printf("Performing AES authentication with Level 2 Switch Key\n");
	if (!MifPlus_FollowingAuthentication(0x9002, NULL))
		return FALSE;

	printf("The security level has been changed, you must now remove the card from the reader.\n\n");
	return TRUE;
}

/* AES authentication to switch from Level 1 to Level 3 */
/* ---------------------------------------------------- */
BOOL MifPlus_L1_Go_L3(void)
{
	SWORD rc;

	if (!fConfirmation)
	{
		printf("We're now ready to put the card in Level 3.\n");
		printf("Once this is done, IT IS NOT POSSIBLE to go back to Level 1.\n");
		printf("Are you really sure you want to do this ?\n");
		if (!confirm())
			exit(EXIT_FAILURE);
	}

	/* In Level 1, the Level-switch authentication must be performed in T=CL */
	rc = SPROX_MifPlus_EnterTcl();
	if (rc)
	{
		printf("SPROX_MifPlus_EnterTcl failed, rc=%ld\n", rc);
		return FALSE;
	}

	/* The Level-switch authentication is a Following Authentication */
	printf("Performing AES authentication with Level 3 Switch Key\n");
	if (!MifPlus_FollowingAuthentication(0x9003, NULL))
		return FALSE;

	printf("The security level has been changed, you must now remove the card from the reader.\n\n");
	return TRUE;
}

/*
 *****************************************************************************
 *
 * Test suite for the card running at Level 2
 *
 *****************************************************************************
 *
 */

BOOL MifPlus_L2_Test(void)
{
	printf("\n\n**** Level 2 is not implemented! ****\n\n");
	return FALSE;
}

BOOL MifPlus_L2_Go_L3(void)
{
	SWORD rc;

	if (!fConfirmation)
	{
		printf("We're now ready to put the card in Level 3.\n");
		printf("Once this is done, IT IS NOT POSSIBLE to go back to Level 2.\n");
		printf("Are you really sure you want to do this ?\n");
		if (!confirm())
			exit(EXIT_FAILURE);
	}

	/* In Level 2, the Level-switch authentication must be performed in T=CL */
	rc = SPROX_MifPlus_EnterTcl();
	if (rc)
	{
		printf("SPROX_MifPlus_EnterTcl failed, rc=%ld\n", rc);
		return FALSE;
	}

	/* The Level-switch authentication is a Following Authentication */
	printf("Performing AES authentication with Level 3 Switch Key\n");
	if (!MifPlus_FollowingAuthentication(0x9003, NULL))
		return FALSE;

	printf("The security level has been changed, you must now remove the card from the reader.\n\n");
	return TRUE;
}

/*
 *****************************************************************************
 *
 * Test suite for the card running at Level 3
 *
 *****************************************************************************
 *
 */

 /*
  * Test of the Virtual Card feature
  * --------------------------------
  */
BOOL MifPlus_L3_Test_VirtualCard()
{
	BYTE i;
	BYTE picc_info;
	BYTE picc_cap[2];
	BYTE picc_uid[10];
	BYTE picc_uid_len;
	const BYTE pcd_cap[3] = { 0x00, 0x00, 0x00 };
	const BYTE dummy_id[16] = { 0 };
	const BYTE* install_id = get_perso_value(0xB001); /* Installation Identifier */
	const BYTE* polling_enc_key = get_perso_value(0xA080); /* VC Polling Enc Key */
	const BYTE* polling_mac_key = get_perso_value(0xA081); /* VC Polling Mac Key */

	SWORD rc;

	/* Check that we can run a Virtual Card selection with the card */
	/* ------------------------------------------------------------ */

	printf("Doing VC select with valid keys... must succeed!\n");
	rc = SPROX_MifPlus_VirtualCard(install_id,
		polling_enc_key,
		polling_mac_key,
		pcd_cap,
		sizeof(pcd_cap),
		&picc_info,
		picc_cap,
		picc_uid,
		&picc_uid_len);
	if (rc)
	{
		printf("SPROX_MifPlus_VirtualCard failed, rc=%ld\n", rc);
		return FALSE;
	}

	printf("\tVC OK, Info=%02X, UID=", picc_info);
	for (i = 0; i < picc_uid_len; i++)
		printf("%02X", picc_uid[i]);
	printf(" Cap1=%02X%02X\n", picc_cap[0], picc_cap[1]);

	rc = SPROX_MifPlus_DeselectVirtualCard();
	if (rc)
	{
		printf("SPROX_MifPlus_DeselectVirtualCard failed, rc=%ld\n", rc);
		return FALSE;
	}

	/* Check that Virtual Card selection fails when Installation ID or keys are unknown */
	/* -------------------------------------------------------------------------------- */

	printf("Doing VC select with invalid MAC key... must fail!\n");
	rc = SPROX_MifPlus_VirtualCard(install_id,
		polling_enc_key,
		polling_enc_key,
		pcd_cap,
		sizeof(pcd_cap),
		&picc_info,
		picc_cap,
		picc_uid,
		&picc_uid_len);
	if (!rc)
	{
		printf("SPROX_MifPlus_VirtualCard : oups... line %d\n", __LINE__);
		return FALSE;
	}
	rc = SPROX_MifPlus_DeselectVirtualCard();
	if (rc)
	{
		printf("SPROX_MifPlus_DeselectVirtualCard failed, rc=%ld\n", rc);
		return FALSE;
	}

	printf("Doing VC select with invalid Encryption key but with valid MAC key...\n");
	rc = SPROX_MifPlus_VirtualCard(install_id,
		polling_mac_key,
		polling_mac_key,
		pcd_cap,
		sizeof(pcd_cap),
		&picc_info,
		picc_cap,
		picc_uid,
		&picc_uid_len);
	if (rc)
	{
		printf("SPROX_MifPlus_VirtualCard failed, rc=%ld\n", rc);
		return FALSE;
	}
	rc = SPROX_MifPlus_DeselectVirtualCard();
	if (rc)
	{
		printf("SPROX_MifPlus_DeselectVirtualCard failed, rc=%ld\n", rc);
		return FALSE;
	}

	/* Volontary oups ;-) */
	printf("\tVC ??, Info=%02X, UID=", picc_info);
	for (i = 0; i < picc_uid_len; i++)
		printf("%02X", picc_uid[i]);
	printf(" Cap1=%02X%02X\n", picc_cap[0], picc_cap[1]);

	printf("Doing VC select with valid keys but with invalid identifier...\n");
	rc = SPROX_MifPlus_VirtualCard(dummy_id,
		polling_enc_key,
		polling_mac_key,
		pcd_cap,
		sizeof(pcd_cap),
		&picc_info,
		picc_cap,
		picc_uid,
		&picc_uid_len);
	if (!rc)
	{
		printf("SPROX_MifPlus_VirtualCard : oups... line %d\n", __LINE__);
		return FALSE;
	}
	rc = SPROX_MifPlus_DeselectVirtualCard();
	if (rc)
	{
		printf("SPROX_MifPlus_DeselectVirtualCard failed, rc=%ld\n", rc);
		return FALSE;
	}


	/* Run a Virtual Card selection again, so we leave the card in an 'available' state */
	/* -------------------------------------------------------------------------------- */

	rc = SPROX_MifPlus_VirtualCard(install_id,
		polling_enc_key,
		polling_mac_key,
		pcd_cap,
		sizeof(pcd_cap),
		&picc_info,
		picc_cap,
		picc_uid,
		&picc_uid_len);
	if (rc)
	{
		printf("SPROX_MifPlus_VirtualCard failed, rc=%ld\n", rc);
		return FALSE;
	}

	return TRUE;
}

BOOL MifPlus_L3_Read(WORD address, BYTE data[])
{
	SWORD rc;

	rc = SPROX_MifPlus_Read(address, data);
	if (rc)
	{
		printf("SPROX_MifPlus_Read(%04X) failed, rc=%ld\n", address, rc);
		return FALSE;
	}

	return TRUE;
}

BOOL MifPlus_L3_Write(WORD address, BYTE const data[])
{
	SWORD rc;

	rc = SPROX_MifPlus_Write(address, data);
	if (rc)
	{
		printf("SPROX_MifPlus_Write(%04X) failed, rc=%ld\n", address, rc);
		return FALSE;
	}

	return TRUE;
}

BOOL MifPlus_L3_Test(void)
{
	WORD sector;
	BOOL last_block_reached;

	printf("Testing the VirtualCard feature\n");
	if (!MifPlus_L3_Test_VirtualCard())
		return FALSE;

	printf("Performing AES authentication with Card Configuration Key\n");
	if (!MifPlus_FirstAuthentication(0x9001))
		return FALSE;

	printf("Performing AES authentication with Card Master Key\n");
	if (!MifPlus_FirstAuthentication(0x9000))
		return FALSE;

	printf("Reading and writing with A keys\n");
	last_block_reached = FALSE;
	for (sector = 0; sector < 40; sector++)
	{
		BYTE data[16];
		BYTE block, count;
		WORD auth_address = 0x4000 + 2 * sector;

		if (sector == 0)
		{
			if (!MifPlus_FirstAuthentication(auth_address))
				return FALSE;
		}
		else
		{
			if (!MifPlus_FollowingAuthentication(auth_address, &last_block_reached))
			{
				if (last_block_reached) break;
				return FALSE;
			}
		}

		if (sector < 32)
		{
			block = 4 * sector;
			count = 3;
		}
		else
		{
			block = 128 + 16 * (sector - 32);
			count = 15;
		}

		do
		{
			if (!MifPlus_L3_Read(block, data))
				return FALSE;

			if (block > 0)
			{
				if (!MifPlus_L3_Write(block, data))
					return FALSE;
			}

			block++;
			count--;
		} while (count);
	}

	printf("%d sectors OK\n", sector);

	printf("Reading and writing with B keys\n");
	last_block_reached = FALSE;
	for (sector = 0; sector < 40; sector++)
	{
		BYTE data[16];
		BYTE block, count;
		WORD auth_address = 0x4001 + 2 * sector;

		if (sector == 0)
		{
			if (!MifPlus_FirstAuthentication(auth_address))
				return FALSE;
		}
		else
		{
			if (!MifPlus_FollowingAuthentication(auth_address, &last_block_reached))
			{
				if (last_block_reached) break;
				return FALSE;
			}
		}

		if (sector < 32)
		{
			block = 4 * sector;
			count = 3;
		}
		else
		{
			block = 128 + 16 * (sector - 32);
			count = 15;
		}

		do
		{
			if (!MifPlus_L3_Read(block, data))
				return FALSE;

			if (block > 0)
			{
				if (!MifPlus_L3_Write(block, data))
					return FALSE;
			}

			block++;
			count--;
		} while (count);
	}

	printf("%d sectors OK\n", sector);

	printf("Performing AES authentication over sector 0\n");
	if (!MifPlus_FirstAuthentication(0x4000))
		return FALSE;

	printf("Performing AES authentication with Card Configuration key\n");
	if (!MifPlus_FollowingAuthentication(0x9001, NULL))
		return FALSE;

	printf("Performing AES authentication with Card Master key\n");
	if (!MifPlus_FollowingAuthentication(0x9000, NULL))
		return FALSE;

	return TRUE;
}


BOOL MifPlus_FirstAuthentication(WORD key_address)
{
	BYTE pcd_cap[6] = { 0xB0,0xB1,0xB2,0xB3,0xB4,0xB5 };
	const BYTE* key_value = get_perso_value(key_address);
	SWORD rc;

	if (key_value == NULL)
	{
		printf("Internal error, key %04X is unknown\n", key_address);
		exit(EXIT_FAILURE);
	}

	rc = SPROX_MifPlus_FirstAuthenticate(key_address, key_value, pcd_cap, 6, NULL);
	if (rc)
	{
		printf("SPROX_MifPlus_FirstAuthenticate(%04X) failed, rc=%ld\n", key_address, rc);
		return FALSE;
	}

	return TRUE;
}

BOOL MifPlus_FollowingAuthentication(WORD key_address, BOOL* last_block_reached)
{
	const BYTE* key_value = get_perso_value(key_address);
	SWORD rc;

	if (key_value == NULL)
	{
		printf("Internal error, key %04X is unknown\n", key_address);
		exit(EXIT_FAILURE);
	}

	rc = SPROX_MifPlus_FollowingAuthenticate(key_address, key_value);

	if ((rc == (MFP_ERROR - MFP_ERR_INVALID_BLOCK_NUMBER))
		&& (last_block_reached != NULL)
		&& ((key_address == 0x4040) || (key_address == 0x4041)))
	{
		/* End of 2K card */
		*last_block_reached = TRUE;
		return FALSE;
	}

	if (rc)
	{
		printf("SPROX_MifPlus_FollowingAuthenticate(%04X) failed, rc=%ld\n", key_address, rc);
		return FALSE;
	}

	return TRUE;
}

