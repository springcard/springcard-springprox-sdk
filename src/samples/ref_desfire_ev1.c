/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  Copyright (c) 2003-2014 PRO ACTIVE SAS, FRANCE - www.springcard.com

  Ref_DesFire_EV1.c
  -----------------

  This is the reference applications that validates the whole SpringProx API
  for NXP DESFire EV1 cards.

  JDA 13/01/2010 : initial release
  JDA 24/09/2010 : minor rewriting, list of tests improved
  JDA 13/08/2014 : moved to Visual C++ Express 2010, added the 'A'
				   suffix to all text-related functions
  JDA 04/09/2023 : refreshed the project to build with Visual Studio 2022

*/
#include "products/springprox/api/springprox.h"
#include "cardware/desfire/sprox_desfire.h"

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __linux__
#include <sys/time.h>
#endif

static BOOL parse_args(int argc, char** argv);
static void usage(void);
static BOOL sprox_desfire_library_selftest(void);

const char* PROGRAM_NAME = "ref_desfire_ev1";
const char* szCommDevice = NULL;
BOOL fLoop = FALSE;
BOOL fAgain = FALSE;
BYTE dDSI = 0;
BYTE dDRI = 0;
BYTE dCID = 0xFF;
BOOL fIsoWrapping = FALSE;
BOOL fShowDetails = FALSE;

int main(int argc, char** argv)
{
	SWORD rc;
	int i;
	char s_buffer[64];
	WORD atq;
	BYTE sak;
	BYTE ats[32];
	BYTE ats_len;
	BYTE uid[32];
	BYTE uid_len;
	BYTE info[32];
	BYTE info_len;

	printf("SpringCard SpringProx 'Legacy' SDK\n");
	printf("\n");
	printf("%s : NXP MIFARE DESFire EV1 validation & sample\n\n", PROGRAM_NAME);
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

	SPROX_Desfire_SelectCid(dCID);

	if (fIsoWrapping)
		SPROX_Desfire_IsoWrapping(DF_ISO_WRAPPING_CARD);
	else
		SPROX_Desfire_IsoWrapping(DF_ISO_WRAPPING_OFF);

	for (;;)
	{
		/* Open reader */
		/* ----------- */

		rc = SPROX_ReaderOpenA(szCommDevice);
		if (rc != MI_OK)
		{
			printf("Reader not found\n");
			goto done;
		}

		rc = SPROX_ReaderGetDeviceA(s_buffer, sizeof(s_buffer));
		if (rc == MI_OK)
			printf("Reader found on %s\n", s_buffer);

		rc = SPROX_ReaderGetFirmwareA(s_buffer, sizeof(s_buffer));
		if (rc == MI_OK)
			printf("Reader firmware is %s\n", s_buffer);

		/* Now wait for a card */
		/* ------------------- */

	loop:

		uid_len = sizeof(uid);
		info_len = sizeof(info);
		rc = SPROX_FindEx(PROTO_14443_A, NULL, uid, &uid_len, info, &info_len);
		if (rc == MI_NOTAGERR)
		{
			if (fLoop)
				goto loop;

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

		/* Open a T=CL session on the tag */
		/* ------------------------------ */

		ats_len = sizeof(ats);
		rc = SPROX_TclA_GetAts(dCID, ats, &ats_len);
		if (rc != MI_OK)
		{
			/* Erreur */
			printf("Failed to activate the tag (%d)\n", rc);
			goto halt;
		}

		printf("T=CL tag activated, ATS=");
		for (i = 0; i < ats_len; i++)
			printf("%02X", ats[i]);
		printf("\n");

		/* Perform PPS  */
		rc = SPROX_TclA_Pps(dCID, dDSI, dDRI);
		if (rc == MI_OK)
		{
			printf("PPS OK, DSI=%d, DRI=%d\n", dDSI, dDRI);
		}
		else if ((rc == MI_UNKNOWN_FUNCTION) || (rc == MI_WRONG_PARAMETER))
		{
			printf("PPS not supported by this reader\n");
		}
		else
		{
			/* Erreur */
			printf("Failed to negociate PPS (DSI=%d, DRI=%d)\n", dDSI, dDRI);
			goto deselect;
		}

		/* Check DESFire library functions */
		/* ------------------------------- */

		sprox_desfire_library_selftest();

		goto deselect;

	deselect:

		/* T=CL (ISO 14443-4) tag stop */
		/* --------------------------- */

		rc = SPROX_TclA_Deselect(dCID);
		if (rc != MI_OK)
		{
			printf("T=CL DESELECT failed (%d)\n", rc);
			goto halt;
		}

	halt:

		/* ISO 14443-3 tag stop */
		/* -------------------- */

		rc = SPROX_TclA_Halt();
		if (rc != MI_OK)
		{
			printf("HALT failed\n");
			goto close;
		}

		if (fAgain)
			goto loop;

		if (fLoop)
		{
			/* Wait until the card has been removed */
			printf("Remove the card from the reader...\n\n");
			while (SPROX_A_SelectAgain(NULL, 0) == MI_OK);

			/* Reset the field to reset the card */
		//    SPROX_ControlRF(0);
			printf("\nNow waiting for the next card...\n\n");
			goto loop;
		}

		printf("SUCCESS!\n");

	close:
		SPROX_ControlRF(FALSE);
		SPROX_ReaderClose();

	done:
		/* Display last error */
		if (rc == MI_OK)
		{
			printf("SUCCESS!\n");
		}
		else
		{
			printf("%s (%d)\n", SPROX_GetErrorMessageA(rc), rc);
		}

#ifdef LOOP_TEST
		gwLoopCount++;
		printf("--> %d error(s) after %d loop(s)\n", gwErrCount, gwLoopCount);
#else
		break;
#endif
	}

	return 0;
}

static void usage(void)
{
	printf("Usage: %s [BAUDRATE] [-l] [-a] [-s] [-i] [-c <CID>] [-d <COMM DEVICE] [-v]\n\n", PROGRAM_NAME);
	printf("Choose the baudrates:\n");
	printf(" -dr0, -dr1, -dr2 or -dr3 for PCD->PICC baudrate\n");
	printf(" -ds0, -ds1, -ds2 or -ds3 for PICC->PCD baudrate\n");
	printf("Default is -dr0 -ds0 (106kbit/s both directions)\n\n");
	printf("If -l (loop) is set, the software waits for another card and continue.\n\n");
	printf("If -a (again) is set, the software runs the test on the same card again.\n\n");
	printf("If -s (show) is set, the software shows some details.\n\n");
	printf("If -i (iso) is set, ISO/IEC 7816-4 wrapping is used.\n\n");
	printf("If -c (cid) is set, the corresponding CID is used.\n\n");
	printf("If the comm. name is not specified, the default device is taken from Registry or from /etc/springprox.cfg\n\n");
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
			else if (!strcmp(argv[i], "-i"))
			{
				fIsoWrapping = TRUE;
			}
			else if (!strcmp(argv[i], "-s"))
			{
				fShowDetails = TRUE;
			}
			else if (!strcmp(argv[i], "-c") && i + 1 < argc)
			{
				dCID = atoi(argv[i + 1]);
				i++;  // Skip next item since we just processed it
			}
			else if (!strcmp(argv[i], "-l") || !strcmp(argv[i], "-loop"))
			{
				fLoop = TRUE;
			}
			else if (!strcmp(argv[i], "-a") || !strcmp(argv[i], "-again"))
			{
				fAgain = TRUE;
			}
			else if (!strcmp(argv[i], "-ds0"))
			{
				dDSI = 0;
			}
			else if (!strcmp(argv[i], "-ds1"))
			{
				dDSI = 1;
			}
			else if (!strcmp(argv[i], "-ds2"))
			{
				dDSI = 2;
			}
			else if (!strcmp(argv[i], "-ds3"))
			{
				dDSI = 3;
			}
			else if (!strcmp(argv[i], "-dr0"))
			{
				dDRI = 0;
			}
			else if (!strcmp(argv[i], "-dr1"))
			{
				dDRI = 1;
			}
			else if (!strcmp(argv[i], "-dr2"))
			{
				dDRI = 2;
			}
			else if (!strcmp(argv[i], "-dr3"))
			{
				dDRI = 3;
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


//#define LOOP_TEST

#ifdef WIN32
#ifdef LOOP_TEST
#define CHECK_RC() { if (rc!=0) { Beep(880,1000); printf("\nline %d, mode %d - failed (%d) %s\n", __LINE__-1, iTestMode, rc, SPROX_Desfire_GetErrorMessage(rc)); gwErrCount++; return FALSE; } printf("."); fflush(NULL); }
#else
#define CHECK_RC() { if (rc!=0) { printf("\nline %d, mode %d - failed (%d) %s\n", __LINE__-1, iTestMode, rc, SPROX_Desfire_GetErrorMessage(rc)); return FALSE; } printf("."); fflush(NULL); }
#endif
#endif

#ifdef __linux__
#define CHECK_RC() { if (rc!=0) { printf("\nline %d, mode %d - failed (%d) %s\n", __LINE__-1, iTestMode, rc, SPROX_Desfire_GetErrorMessage(rc)); return FALSE; } printf("."); fflush(NULL); }
#endif

#ifdef LOOP_TEST
WORD gwLoopCount = 0;
WORD gwErrCount = 0;
#endif

typedef enum
{
	TM_UNSET,
	TM_LEGACY,
	TM_ISO_3DES2K,
	TM_ISO_3DES3K,
	TM_ISO_AES

} TEST_MODE;


static BOOL sprox_desfire_library_selftest_ex(TEST_MODE iTestMode, BOOL fLegacyCard);

const BYTE abNullKey[24] = { 0 };

const BYTE abRootKey1K[24] = { "ABCDEFGHABCDEFGHABCDEFGH" };
const BYTE abRootKey2K[24] = { "Card Master Key!Card Mas" };
const BYTE abRootKey3K[24] = { "Card Master Key!        " };

const BYTE abTestKeyDes1K[24] = { "ABCDEFGHABCDEFGHABCDEFGH" };
const BYTE abTestKeyDes2K[24] = { "ABCDEFGHIJKLMNOPABCDEFGH" };
const BYTE abTestKeyDes3K[24] = { "ABCDEFGHIJKLMNOPQRSTUVWX" };

const BYTE appBKeyMaster0_16[16] = { "App.B Master Key" };
const BYTE appBKeyMaster1_16[16] = { "@qq/C!L`ruds!Kdx" };
const BYTE appBChangeKey_16[16] = { "B's Chg Keys Key" };

const BYTE appBKeyMaster0_24[24] = { "App.B Master KeyApp. Key" };
const BYTE appBKeyMaster1_24[24] = { "@qq/C!L`ruds!Kdx@qq/!Kdx" };
const BYTE appBChangeKey_24[24] = { "B's Chg Keys KeyB's  Key" };

const BYTE appBKey1_16[16] = { "App.B Key #1.   " };
const BYTE appBKey2_16[16] = { "App.B Key #2..  " };
const BYTE appBKey3_16[16] = { "App.B Key #3... " };
const BYTE appBKey4_16[16] = { "App.B Key #4...." };

const BYTE appBKey1_24[24] = { "App.B Key #1.   .   .   " };
const BYTE appBKey2_24[24] = { "App.B Key #2..  ..  ..  " };
const BYTE appBKey3_24[24] = { "App.B Key #3... ... ... " };
const BYTE appBKey4_24[24] = { "App.B Key #4............" };

const BYTE appCKey1_16[16] = { "App.C Key #1.   " };
const BYTE appCKey2_16[16] = { "App.C Key #2..  " };
const BYTE appCKey3_16[16] = { "App.C Key #3... " };
const BYTE appCKey4_16[16] = { "App.C Key #4...." };

const BYTE appCKey1_24[24] = { "App.C Key #1.   .   .   " };
const BYTE appCKey2_24[24] = { "App.C Key #2..  ..  ..  " };
const BYTE appCKey3_24[24] = { "App.C Key #3... ... ... " };
const BYTE appCKey4_24[24] = { "App.C Key #4............" };


static BOOL sprox_desfire_library_selftest(void)
{
	DF_VERSION_INFO stVersionInfo;
	WORD wDesFireSoftwareVersion;
	TEST_MODE iTestMode = TM_UNSET;
	SWORD rc;

	//  Get the card's version information.
	rc = SPROX_Desfire_GetVersion(&stVersionInfo);
	CHECK_RC();
	wDesFireSoftwareVersion = (unsigned int)((unsigned int)stVersionInfo.bSwMajorVersion << 8 | stVersionInfo.bSwMinorVersion);
	printf("\n");

	if (wDesFireSoftwareVersion < 0x0100)
	{
		printf("This is a Desfire EV0!\n");

		if (!sprox_desfire_library_selftest_ex(TM_LEGACY, TRUE))
			return FALSE;

	}
	else
	{
		printf("Desfire EV0 compatibility test (DES/3DES2K)\n");

		if (!sprox_desfire_library_selftest_ex(TM_LEGACY, FALSE))
			return FALSE;

		printf("Desfire EV1, ISO mode with 3DES2K\n");

		if (!sprox_desfire_library_selftest_ex(TM_ISO_3DES2K, FALSE))
			return FALSE;

		printf("Desfire EV1, ISO mode with 3DES3K\n");

		if (!sprox_desfire_library_selftest_ex(TM_ISO_3DES3K, FALSE))
			return FALSE;

		printf("Desfire EV1, ISO mode with AES\n");

		if (!sprox_desfire_library_selftest_ex(TM_ISO_AES, FALSE))
			return FALSE;

	}

	printf("SUCCESS!\n");
	printf("All tests have been successfully passed\n");

	return TRUE;
}

static BOOL sprox_desfire_library_selftest_ex(TEST_MODE iTestMode, BOOL fLegacyCard)
{
	SWORD rc;
	LONG lAmount;
	BYTE bCommMode, bKeyVersion;
	DWORD eNRecordsRead, eOffset, eLength, dwFreeBytes = 4096;
	BYTE bStdDataFileID, bFileType, bNFilesFound, bNApplicationsFound;
	DWORD dwNbBytesRead;
	DWORD adwApplications[32];
	BYTE  abFiles[32];
	DF_ISO_APPLICATION_ST astIsoApplications[32];
	WORD  awIsoFiles[32];
	BYTE abDataBuffer[8192];
	WORD wAccessRights;
	BOOL trace = FALSE;
	BOOL beep = TRUE;
	int i, j, iTransaction, iFileIndex;
	DF_VERSION_INFO stVersionInfo;
	DF_ADDITIONAL_FILE_SETTINGS unFileSettings;

	BOOL f = FALSE;

	//  After activating a DesFire card the currently selected 'application' is the card
	//  iTestMode (AID = 0). Therefore the next command is redundant.
	//  We use the command to check that the card is responding correctly.
	rc = SPROX_Desfire_SelectApplication(0);
	CHECK_RC();

	if (!fLegacyCard)
	{
		rc = SPROX_Desfire_GetFreeMemory(&dwFreeBytes);
		CHECK_RC();
	}

	//  Which Card Master Key is currently in use ?
	//  Beginning with DesFire version 0.1 the GetKeyVersion command can help determining
	//  which key is currently effective.
	rc = SPROX_Desfire_GetKeyVersion(0, &bKeyVersion);
	CHECK_RC();

	//  Authenticate with the key which corresponds to the retieved key version.
	if (bKeyVersion == 0)
	{
		rc = SPROX_Desfire_Authenticate(0, abNullKey);
		if (rc == (DFCARD_ERROR - DF_AUTHENTICATION_ERROR))
		{
			rc = SPROX_Desfire_AuthenticateIso(0, abNullKey);
			CHECK_RC();

			//  The key is configured for 3DES3K
				  //  for obvious reasons we cannot revert the config to 3DES2K, but a hook by AES permits it
			rc = SPROX_Desfire_ChangeKeyAes(DF_APPLSETTING2_AES, 0xAE, abRootKey2K, NULL);
			CHECK_RC();
			rc = SPROX_Desfire_AuthenticateAes(0, abRootKey2K);
		}
		CHECK_RC();


	}
	else if (bKeyVersion == 0xAA)
	{
		rc = SPROX_Desfire_Authenticate(0, abRootKey1K);
		if (rc == (DFCARD_ERROR - DF_AUTHENTICATION_ERROR))
		{
			rc = SPROX_Desfire_AuthenticateIso(0, abRootKey1K);

			//  The key is configured for 3DES3K
					//  for obvious reasons we cannot revert the config to 3DES2K, but a hook by AES permits it
			rc = SPROX_Desfire_ChangeKeyAes(DF_APPLSETTING2_AES, 0xAE, abRootKey2K, NULL);
			CHECK_RC();
			rc = SPROX_Desfire_AuthenticateAes(0, abRootKey2K);
		}
		CHECK_RC();

	}
	else if (bKeyVersion == 0xC7)
	{
		rc = SPROX_Desfire_Authenticate(0, abRootKey2K);
		if (rc == (DFCARD_ERROR - DF_AUTHENTICATION_ERROR))
		{
			rc = SPROX_Desfire_AuthenticateIso(0, abRootKey2K);

			//  The key is configured for 3DES3K
					//  for obviously we cannot revert the config to 3DES2K, but a hook by AES permits it
			rc = SPROX_Desfire_ChangeKeyAes(DF_APPLSETTING2_AES, 0xAE, abRootKey2K, NULL);
			CHECK_RC();
			rc = SPROX_Desfire_AuthenticateAes(0, abRootKey2K);
		}
		CHECK_RC();

	}
	else if (bKeyVersion == 0xAE)
	{
		rc = SPROX_Desfire_AuthenticateAes(0, abRootKey2K);
		CHECK_RC();

	}
	else
	{
		//  Who knows the key ?
		printf("Sorry, the master key is unknown (version = %02X)\n", bKeyVersion);
		return FALSE;
	}

	//  Back to default (null) key
	rc = SPROX_Desfire_ChangeKey(0, abNullKey, NULL);
	CHECK_RC();

	//  Authenticate again
	rc = SPROX_Desfire_Authenticate(0, abNullKey);
	CHECK_RC();

	//  Try to allow to change the key
	rc = SPROX_Desfire_ChangeKeySettings(0xF);
	CHECK_RC();

	//  List the existing applications
	rc = SPROX_Desfire_GetApplicationIDs(0, NULL, &bNApplicationsFound);
	CHECK_RC();

	if (bNApplicationsFound)
	{
		//  Startup with a blank card !!!
		rc = SPROX_Desfire_FormatPICC();
		CHECK_RC();
	}

	//  Card is ready...
	printf("\n");

	//  Authenticate again
	rc = SPROX_Desfire_Authenticate(0, abNullKey);
	CHECK_RC();

	if (!fLegacyCard)
	{
		// Get the UID of the card
		rc = SPROX_Desfire_GetCardUID(abDataBuffer);
		CHECK_RC();
	}

	//  Change the Card master key to a SingleDES key.
	rc = SPROX_Desfire_ChangeKey(0, abRootKey1K, NULL);
	CHECK_RC();

	rc = SPROX_Desfire_GetKeyVersion(0, &bKeyVersion);
	CHECK_RC();

	if (iTestMode == TM_LEGACY)
	{
		rc = SPROX_Desfire_Authenticate(0, abRootKey1K);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKey(0, abRootKey2K, NULL);
		CHECK_RC();

	}
	else if (iTestMode == TM_ISO_3DES2K)
	{
		rc = SPROX_Desfire_AuthenticateIso(0, abRootKey1K);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKey(0, abRootKey2K, NULL);
		CHECK_RC();

	}
	else if (iTestMode == TM_ISO_3DES3K)
	{
		rc = SPROX_Desfire_AuthenticateIso24(0, abRootKey1K);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKey24(DF_APPLSETTING2_3DES3K, abRootKey2K, NULL);
		CHECK_RC();

	}
	else if (iTestMode == TM_ISO_AES)
	{
		rc = SPROX_Desfire_AuthenticateIso(0, abRootKey1K);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKeyAes(DF_APPLSETTING2_AES, 0xAE, abRootKey2K, NULL);
		CHECK_RC();

	}

	/*
	// Reconfigure the card
	{
		BYTE b = 0x02;
		//BYTE ats[] = { 0x06, 0x75, 0x77, 0x81, 0x02, 0xFF };
		//rc = SPROX_Desfire_SetConfiguration(0x02, ats, sizeof(ats));
		rc = SPROX_Desfire_SetConfiguration(0x00, &b, 1);
		CHECK_RC();
	}
	*/

	if (!fLegacyCard)
	{
		rc = SPROX_Desfire_GetFreeMemory(&dwFreeBytes);
		CHECK_RC();
	}

	rc = SPROX_Desfire_GetKeyVersion(0, &bKeyVersion);
	CHECK_RC();
	rc = SPROX_Desfire_GetKeyVersion(0, &bKeyVersion);
	CHECK_RC();

	if (iTestMode == TM_LEGACY)
	{
		rc = SPROX_Desfire_Authenticate(0, abRootKey2K);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES2K)
	{
		rc = SPROX_Desfire_AuthenticateIso(0, abRootKey2K);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES3K)
	{
		rc = SPROX_Desfire_AuthenticateIso24(0, abRootKey2K);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_AES)
	{
		rc = SPROX_Desfire_AuthenticateAes(0, abRootKey2K);
		CHECK_RC();
	}

	rc = SPROX_Desfire_GetKeySettings(&abDataBuffer[0], &abDataBuffer[1]);
	CHECK_RC();

	if (!fLegacyCard)
	{
		rc = SPROX_Desfire_GetFreeMemory(&dwFreeBytes);
		CHECK_RC();
	}

	//  At this point we would like to begin with a blank card.
	rc = SPROX_Desfire_FormatPICC();
	CHECK_RC();

	if (!fLegacyCard)
	{
		rc = SPROX_Desfire_GetFreeMemory(&dwFreeBytes);
		CHECK_RC();
	}

	//  Create three applications:

	//  Application A is the most open one.
	//  It has no keys and its key settings are least restrictive.
	//  Note: since there are no keys, the Change Keys key settting must be assigned
	//        the value 0xE or 0xF.
	if (iTestMode == TM_LEGACY)
	{
		rc = SPROX_Desfire_CreateApplication(0xAAAAAA, 0xFF, 0);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES2K)
	{
		rc = SPROX_Desfire_CreateApplication(0xAAAAAA, 0xFF, 0);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES3K)
	{
		rc = SPROX_Desfire_CreateApplication(0xAAAAAA, 0xFF, DF_APPLSETTING2_3DES3K | 0);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_AES)
	{
		rc = SPROX_Desfire_CreateApplication(0xAAAAAA, 0xFF, DF_APPLSETTING2_AES | 0);
		CHECK_RC();
	}

	//  Application B's key settings can be changed at a later time.
	//  It has six keys.
	//  Authentication with a given key allows also to change that key.
	//  (the Change Keys key settting is 0xE)
	if (iTestMode == TM_LEGACY)
	{
		rc = SPROX_Desfire_CreateApplication(0xBBBBBB, 0xEF, 6);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES2K)
	{
		rc = SPROX_Desfire_CreateApplication(0xBBBBBB, 0xEF, 6);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES3K)
	{
		rc = SPROX_Desfire_CreateApplication(0xBBBBBB, 0xEF, DF_APPLSETTING2_3DES3K | 6);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_AES)
	{
		rc = SPROX_Desfire_CreateApplication(0xBBBBBB, 0xEF, DF_APPLSETTING2_AES | 6);
		CHECK_RC();
	}

	//  Application C keeps everything private.
	//  Even getting its file directory is not publicly allowed.
	//  The application has the maximum of 14 keys.
	//  We make key #0xC the Change Keys key.
	if (iTestMode == TM_LEGACY)
	{
		rc = SPROX_Desfire_CreateApplication(0xCCCCCC, 0xC0, 14);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES2K)
	{
		rc = SPROX_Desfire_CreateApplication(0xCCCCCC, 0xC0, 14);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES3K)
	{
		rc = SPROX_Desfire_CreateApplication(0xCCCCCC, 0xC0, DF_APPLSETTING2_3DES3K | 14);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_AES)
	{
		rc = SPROX_Desfire_CreateApplication(0xCCCCCC, 0xC0, DF_APPLSETTING2_AES | 14);
		CHECK_RC();
	}

	//  Verify that the applications have been created.
	rc = SPROX_Desfire_GetApplicationIDs(32, adwApplications, &bNApplicationsFound);
	CHECK_RC();

	// New applications for ISO testing 
	if (!fLegacyCard)
	{
		// The ISO DF=2000 application will allow ISO EFs
		rc = SPROX_Desfire_CreateIsoApplication(0xFF2000, 0xFF, DF_APPLSETTING2_ISO_EF_IDS, 0x2000, "1TIC.ICA", 8);
		CHECK_RC();
		rc = SPROX_Desfire_CreateIsoApplication(0xFF3000, 0xFF, DF_APPLSETTING2_ISO_EF_IDS, 0x3000, "2MPP.EPP", 8);
		CHECK_RC();
		rc = SPROX_Desfire_CreateIsoApplication(0xFF4000, 0xFF, DF_APPLSETTING2_ISO_EF_IDS, 0x4000, "3JDA.JDA", 8);
		CHECK_RC();
		rc = SPROX_Desfire_CreateIsoApplication(0xFF3F00, 0xFF, DF_APPLSETTING2_ISO_EF_IDS, 0x3F00, "Test Master File", 16);
		CHECK_RC();
	}

	// More applications
	if (dwFreeBytes > 4096)
	{
		rc = SPROX_Desfire_CreateApplication(0xFFFF00, 0xFF, 0);
		CHECK_RC();
		rc = SPROX_Desfire_CreateApplication(0xFFFF01, 0xFF, 0);
		CHECK_RC();
		rc = SPROX_Desfire_CreateApplication(0xFFFF02, 0xFF, 0);
		CHECK_RC();
		rc = SPROX_Desfire_CreateApplication(0xFFFF03, 0xFF, 0);
		CHECK_RC();
		rc = SPROX_Desfire_CreateApplication(0xFFFF04, 0xFF, 0);
		CHECK_RC();
		rc = SPROX_Desfire_CreateApplication(0xFFFF05, 0xFF, 0);
		CHECK_RC();
		rc = SPROX_Desfire_CreateApplication(0xFFFF06, 0xFF, 0);
		CHECK_RC();
		rc = SPROX_Desfire_CreateApplication(0xFFFF07, 0xFF, 0);
		CHECK_RC();
		rc = SPROX_Desfire_CreateApplication(0xFFFF08, 0xFF, 0);
		CHECK_RC();
		rc = SPROX_Desfire_CreateApplication(0xFFFF0A, 0xFF, 0);
		CHECK_RC();
		rc = SPROX_Desfire_CreateApplication(0xFFFF0B, 0xFF, 0);
		CHECK_RC();
		rc = SPROX_Desfire_CreateApplication(0xFFFF0C, 0xFF, 0);
		CHECK_RC();
		rc = SPROX_Desfire_CreateApplication(0xFFFF0D, 0xFF, 0);
		CHECK_RC();
		rc = SPROX_Desfire_CreateApplication(0xFFFF0E, 0xFF, 0);
		CHECK_RC();
		rc = SPROX_Desfire_CreateApplication(0xFFFF0F, 0xFF, 0);
		CHECK_RC();

		rc = SPROX_Desfire_CreateApplication(0xFFFF10, 0xFF, 0);
		CHECK_RC();
		rc = SPROX_Desfire_CreateApplication(0xFFFF20, 0xFF, 0);
		CHECK_RC();
		rc = SPROX_Desfire_CreateApplication(0xFFFF30, 0xFF, 0);
		CHECK_RC();
		rc = SPROX_Desfire_CreateApplication(0xFFFF40, 0xFF, 0);
		CHECK_RC();
		rc = SPROX_Desfire_CreateApplication(0xFFFF50, 0xFF, 0);
		CHECK_RC();
		rc = SPROX_Desfire_CreateApplication(0xFFFF60, 0xFF, 0);
		CHECK_RC();
	}

	//  Verify that the applications have been created.
	rc = SPROX_Desfire_GetApplicationIDs(32, adwApplications, &bNApplicationsFound);
	CHECK_RC();

	// Cleanup authentication state
	rc = SPROX_Desfire_SelectApplication(0);
	CHECK_RC();

	// Retrieve the list of applications
	rc = SPROX_Desfire_GetApplicationIDs(32, adwApplications, &bNApplicationsFound);
	CHECK_RC();

	if (fShowDetails)
	{
		printf("\n");
		printf("%d application(s) found:", bNApplicationsFound);
		for (i = 0; i < bNApplicationsFound; i++)
		{
			if ((i % 8) == 0)
				printf("\n- ");
			printf("%06lX   ", adwApplications[i]);
		}
		printf("\n");
	}

	if (!fLegacyCard)
	{
		// Retrieve the list of ISO DF
		rc = SPROX_Desfire_GetIsoApplications(32, astIsoApplications, &bNApplicationsFound);
		CHECK_RC();

		if (fShowDetails)
		{
			printf("\n");
			printf("%d ISO application(s) found:\n", bNApplicationsFound);
			for (i = 0; i < bNApplicationsFound; i++)
			{
				printf("- %06lX = ISO %04X  ", astIsoApplications[i].dwAid, astIsoApplications[i].wIsoId);
				for (j = 0; j < astIsoApplications[i].bIsoNameLen; j++)
					printf("%02X", astIsoApplications[i].abIsoName[j]);
				for (; j < 16; j++)
					printf("  ");
				printf("  ");
				for (j = 0; j < astIsoApplications[i].bIsoNameLen; j++)
					printf("%c", (astIsoApplications[i].abIsoName[j] >= ' ') ? astIsoApplications[i].abIsoName[j] : '.');
				printf("\n");
			}
		}
	}

	if (!fLegacyCard)
	{
		// Create ISO files in ISO application 2000
		rc = SPROX_Desfire_SelectApplication(0xFF2000);
		CHECK_RC();

		rc = SPROX_Desfire_CreateIsoStdDataFile(1, 0x2001, 0, 0xEEEE, 32);
		CHECK_RC();

		rc = SPROX_Desfire_CreateIsoBackupDataFile(2, 0x2002, 0, 0xEEEE, 32);
		CHECK_RC();

		rc = SPROX_Desfire_CreateIsoLinearRecordFile(3, 0x2003, 0, 0xEEEE, 16, 4);
		CHECK_RC();

		rc = SPROX_Desfire_CreateIsoCyclicRecordFile(4, 0x2004, 0, 0xEEEE, 16, 4);
		CHECK_RC();

		//  Get the file IDs
		rc = SPROX_Desfire_GetFileIDs(32, abFiles, &bNFilesFound);
		CHECK_RC();

		if (fShowDetails)
		{
			printf("\n");
			printf("%d file(s) found:", bNFilesFound);
			for (i = 0; i < bNFilesFound; i++)
			{
				if ((i % 16) == 0)
					printf("\n- ");
				printf("%02X   ", abFiles[i]);
			}
			printf("\n");
		}

		//  Get the ISO file IDs
		rc = SPROX_Desfire_GetIsoFileIDs(32, awIsoFiles, &bNFilesFound);
		CHECK_RC();

		if (fShowDetails)
		{
			printf("\n");
			printf("%d ISO file(s) found:", bNFilesFound);
			for (i = 0; i < bNFilesFound; i++)
			{
				if ((i % 12) == 0)
					printf("\n- ");
				printf("%04X   ", awIsoFiles[i]);
			}
			printf("\n");
		}

	}

	bStdDataFileID = 15;

	//  Create the files in application A:
	//  Since the application does not have any keys associated with it, the file access rights
	//  settings 0xE (public access granted) and 0xF (access denied) are the only permissable
	//  ones.
	rc = SPROX_Desfire_SelectApplication(0xAAAAAA);
	CHECK_RC();

	//  Create a the Standard Data file.
	rc = SPROX_Desfire_CreateStdDataFile(bStdDataFileID, 0, 0xEEEE, 640);
	CHECK_RC();

	//  Create a 64 byte Backup file.
	rc = SPROX_Desfire_CreateBackupDataFile(5, 0, 0xEEEE, 64);
	CHECK_RC();

	//  Create a Value file allowing values from 0 to 1000.
	//  The initial value is 0.
	//  The Limited Credit feature is disabled.
	rc = SPROX_Desfire_CreateValueFile(4, 0, 0xEEEE, 0, 1000, 0, 0);
	CHECK_RC();

	//  And finally create a Cyclic Record file capable of holding 10 records of 4 bytes each.
	rc = SPROX_Desfire_CreateCyclicRecordFile(0, 0, 0xEEEE, 4, 10);
	CHECK_RC();

	//  Fill the Standard Data file with some data.
	memset(abDataBuffer, 0xDA, sizeof(abDataBuffer));

	rc = SPROX_Desfire_WriteData(bStdDataFileID, DF_COMM_MODE_PLAIN, 0, 640, abDataBuffer);
	CHECK_RC();
	rc = SPROX_Desfire_WriteData(bStdDataFileID, DF_COMM_MODE_PLAIN, 0, 30, (unsigned char*)"This is the 1st block written.");
	CHECK_RC();
	rc = SPROX_Desfire_WriteData(bStdDataFileID, DF_COMM_MODE_PLAIN, 34, 22, (unsigned char*)"This is the 2nd block.");
	CHECK_RC();

	//  Then make the file permanently read-only.
	rc = SPROX_Desfire_ChangeFileSettings(bStdDataFileID, DF_COMM_MODE_PLAIN, 0xEFFF);
	CHECK_RC();

	//  Read part of the file's contents.
	rc = SPROX_Desfire_ReadData(bStdDataFileID, DF_COMM_MODE_PLAIN, 10, 50, abDataBuffer, &dwNbBytesRead);
	CHECK_RC();

	//  Get all data in one block. 
	//  Note: Must make sure that buffer is large enough for all data!!
	rc = SPROX_Desfire_ReadData(bStdDataFileID, DF_COMM_MODE_PLAIN, 0, 0, abDataBuffer, &dwNbBytesRead);
	CHECK_RC();

	//  Test different lengths
	for (eLength = 45; eLength < 65; eLength++)
	{
		rc = SPROX_Desfire_ReadData(bStdDataFileID, DF_COMM_MODE_PLAIN, 0, eLength, abDataBuffer, &dwNbBytesRead);
		CHECK_RC();
	}

	//  Try to overwrite the file.
	//  Since we have made the file read-only, this should fail.

	rc = SPROX_Desfire_WriteData(bStdDataFileID, DF_COMM_MODE_PLAIN, 20, 5, "Essai");
	if (rc != (DFCARD_ERROR - DF_PERMISSION_DENIED))
		CHECK_RC();

	//  Do 15 transactions.
	for (iTransaction = 0; iTransaction < 15; iTransaction++)
	{
		//  Write to the Backup Data file.
		snprintf((char*)abDataBuffer, sizeof(abDataBuffer), "%02d,", iTransaction);
		rc = SPROX_Desfire_WriteData(5, DF_COMM_MODE_PLAIN, 3 * iTransaction, 3,
			abDataBuffer);
		CHECK_RC();

		//  Manipulate the Value file.
		rc = SPROX_Desfire_Credit(4, DF_COMM_MODE_PLAIN, 100);
		CHECK_RC();
		rc = SPROX_Desfire_Debit(4, DF_COMM_MODE_PLAIN, 93);
		CHECK_RC();

		//  Write to the Cyclic Record file.
		rc = SPROX_Desfire_WriteRecord(0, DF_COMM_MODE_PLAIN, 2, 2, abDataBuffer);
		CHECK_RC();
		//  The following Write Record will write to the same record as above
		rc = SPROX_Desfire_WriteRecord(0, DF_COMM_MODE_PLAIN, 0, 2,
			(unsigned char*)"r.");
		CHECK_RC();

		//  Verify that the 'official' contents of the three files has not changed
		//  before the CommitTransaction command.
		//  Must make sure that buffer is large enough for all data!
		rc = SPROX_Desfire_ReadData(5, DF_COMM_MODE_PLAIN, 0, 0, abDataBuffer, &dwNbBytesRead);
		CHECK_RC();
		rc = SPROX_Desfire_GetValue(4, DF_COMM_MODE_PLAIN, &lAmount);
		CHECK_RC();

		//  Note: reading from an empty record file returns an error.
		//        beginning with version 0.1 this aborts the transaction.
		if (iTransaction != 0)
		{
			rc = SPROX_Desfire_ReadRecords(0, DF_COMM_MODE_PLAIN, 0, 0, 4, abDataBuffer, &eNRecordsRead);
			CHECK_RC();
		}
		//  Declare the transaction valid.
		rc = SPROX_Desfire_CommitTransaction();
		CHECK_RC();

		//  Verify that the transaction has become effective.
		//  Note: Must make sure that buffer is large enough for all data!
		rc = SPROX_Desfire_ReadData(5, DF_COMM_MODE_PLAIN, 0, 0, abDataBuffer, &dwNbBytesRead);
		CHECK_RC();
		rc = SPROX_Desfire_GetValue(4, DF_COMM_MODE_PLAIN, &lAmount);
		CHECK_RC();
		rc = SPROX_Desfire_ReadRecords(0, DF_COMM_MODE_PLAIN, 0, 0, 4, abDataBuffer, &eNRecordsRead);
		CHECK_RC();
	}

	//  Limited Credit has been disabled, so the following call should fail.
	rc = SPROX_Desfire_LimitedCredit2(4, 20);
	if (rc != (DFCARD_ERROR - DF_PERMISSION_DENIED))
		CHECK_RC();

	//  Get the file IDs of the current application.
	rc = SPROX_Desfire_GetFileIDs(32, abFiles, &bNFilesFound);
	CHECK_RC();

	if (fShowDetails)
	{
		printf("\n");
		printf("%d file(s) found:", bNFilesFound);
		for (i = 0; i < bNFilesFound; i++)
		{
			if ((i % 16) == 0)
				printf("\n- ");
			printf("%02X   ", abFiles[i]);
		}
		printf("\n");
	}

	//  Get information about the application's files.
	//  Delete each file after retrieving information about it.
	for (iFileIndex = 0; iFileIndex < bNFilesFound; iFileIndex++)
	{
		rc = SPROX_Desfire_GetFileSettings(abFiles[iFileIndex], &bFileType,
			&bCommMode, &wAccessRights,
			&unFileSettings);
		CHECK_RC();
		rc = SPROX_Desfire_DeleteFile(abFiles[iFileIndex]);
		CHECK_RC();
	}

	//  Verify that there are no files in the application.
	rc = SPROX_Desfire_GetFileIDs(32, abFiles, &bNFilesFound);
	CHECK_RC();

	//  Delete application A.
	//  Since this application doesn't have an Application Master Key,
	//  this requires Card Master Key authentication.
	rc = SPROX_Desfire_SelectApplication(0);
	CHECK_RC();

	if (iTestMode == TM_LEGACY)
	{
		rc = SPROX_Desfire_Authenticate(0, abRootKey2K);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES2K)
	{
		rc = SPROX_Desfire_AuthenticateIso(0, abRootKey2K);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES3K)
	{
		rc = SPROX_Desfire_AuthenticateIso24(0, abRootKey2K);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_AES)
	{
		rc = SPROX_Desfire_AuthenticateAes(0, abRootKey2K);
		CHECK_RC();
	}

	rc = SPROX_Desfire_DeleteApplication(0xAAAAAA);
	CHECK_RC();

	//  Verify that application A has been deleted.
	rc = SPROX_Desfire_GetApplicationIDs(32, adwApplications, &bNApplicationsFound);
	CHECK_RC();

	//  Changing application B's keys:
	rc = SPROX_Desfire_SelectApplication(0xBBBBBB);
	CHECK_RC();

	//  Use a TripleDES key as the Application Master Key.
	if (iTestMode == TM_LEGACY)
	{
		rc = SPROX_Desfire_Authenticate(0, abNullKey);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES2K)
	{
		rc = SPROX_Desfire_AuthenticateIso(0, abNullKey);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES3K)
	{
		rc = SPROX_Desfire_AuthenticateIso24(0, abNullKey);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_AES)
	{
		rc = SPROX_Desfire_AuthenticateAes(0, abNullKey);
		CHECK_RC();
	}

	rc = SPROX_Desfire_GetVersion(&stVersionInfo);
	CHECK_RC();

	if (fShowDetails)
	{
		printf("\n");
		printf("bHwVendorID=%02X\n", stVersionInfo.bHwVendorID);
		printf("bHwType=%02X\n", stVersionInfo.bHwType);
		printf("bHwSubType=%02X\n", stVersionInfo.bHwSubType);
		printf("bHwMajorVersion=%02X\n", stVersionInfo.bHwMajorVersion);
		printf("bHwMinorVersion=%02X\n", stVersionInfo.bHwMinorVersion);
		printf("bHwStorageSize=%02X\n", stVersionInfo.bHwStorageSize);
		printf("bHwProtocol=%02X\n", stVersionInfo.bHwProtocol);
		printf("bSwVendorID=%02X\n", stVersionInfo.bSwVendorID);
		printf("bSwType=%02X\n", stVersionInfo.bSwType);
		printf("bSwSubType=%02X\n", stVersionInfo.bSwSubType);
		printf("bSwMajorVersion=%02X\n", stVersionInfo.bSwMajorVersion);
		printf("bSwMinorVersion=%02X\n", stVersionInfo.bSwMinorVersion);
		printf("bSwStorageSize=%02X\n", stVersionInfo.bSwStorageSize);
		printf("bSwProtocol=%02X\n", stVersionInfo.bSwProtocol);
		printf("abUid=%02X:%02X:%02X:%02X:%02X:%02X:%02X\n", stVersionInfo.abUid[0], stVersionInfo.abUid[1], stVersionInfo.abUid[2], stVersionInfo.abUid[3], stVersionInfo.abUid[4], stVersionInfo.abUid[5], stVersionInfo.abUid[6]);
		printf("abBatchNo=%02X:%02X:%02X:%02X:%02X\n", stVersionInfo.abBatchNo[0], stVersionInfo.abBatchNo[1], stVersionInfo.abBatchNo[2], stVersionInfo.abBatchNo[3], stVersionInfo.abBatchNo[4]);
		printf("bProductionCW=%02X\n", stVersionInfo.bProductionCW);
		printf("bProductionYear=%02X\n", stVersionInfo.bProductionYear);
	}

	//  Set the new application master key and get authenticated with it

	if (iTestMode == TM_LEGACY)
	{
		rc = SPROX_Desfire_ChangeKey(0, appBKeyMaster0_16, NULL);
		CHECK_RC();
		rc = SPROX_Desfire_Authenticate(0, appBKeyMaster0_16);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES2K)
	{
		rc = SPROX_Desfire_ChangeKey24(0, appBKeyMaster0_16, NULL); // JDA
		CHECK_RC();
		rc = SPROX_Desfire_AuthenticateIso(0, appBKeyMaster0_16);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES3K)
	{
		rc = SPROX_Desfire_ChangeKey24(0, appBKeyMaster0_24, NULL); // JDA
		CHECK_RC();
		rc = SPROX_Desfire_AuthenticateIso24(0, appBKeyMaster0_24);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_AES)
	{
		rc = SPROX_Desfire_ChangeKeyAes(0, 0, appBKeyMaster0_16, NULL); // JDA
		CHECK_RC();
		rc = SPROX_Desfire_AuthenticateAes(0, appBKeyMaster0_16);
		CHECK_RC();
	}

	if (!fLegacyCard)
	{
		rc = SPROX_Desfire_GetFreeMemory(&dwFreeBytes);
		CHECK_RC();
		rc = SPROX_Desfire_GetFreeMemory(&dwFreeBytes);
		CHECK_RC();
	}

	//  Beginning with DesFire version 0.1 we can query the version number of a key.
	rc = SPROX_Desfire_GetKeyVersion(0, &bKeyVersion);
	CHECK_RC();

	//  Authenticate with the new application master key.
	//  This time use the opposite parity in all key bytes (will not work for AES!).
	if (iTestMode == TM_LEGACY)
	{
		rc = SPROX_Desfire_Authenticate(0, appBKeyMaster1_16);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES2K)
	{
		rc = SPROX_Desfire_AuthenticateIso(0, appBKeyMaster1_16);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES3K)
	{
		rc = SPROX_Desfire_AuthenticateIso24(0, appBKeyMaster1_24);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_AES)
	{


	}

	//  Change key #1 to a different SingleDES key (both key halves are the same).
	//  Authentication with that key is necessary for changing it.
	if (iTestMode == TM_LEGACY)
	{
		rc = SPROX_Desfire_Authenticate(1, abNullKey);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKey(1, abTestKeyDes1K, NULL);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES2K)
	{
		rc = SPROX_Desfire_AuthenticateIso(1, abNullKey);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKey24(1, abTestKeyDes1K, NULL);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES3K)
	{
		rc = SPROX_Desfire_AuthenticateIso24(1, abNullKey);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKey24(1, abTestKeyDes1K, NULL);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_AES)
	{
		rc = SPROX_Desfire_AuthenticateAes(1, abNullKey);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKeyAes(1, 0x33, abTestKeyDes1K, NULL);
		CHECK_RC();
	}

	//  Change key #5 to a TripeDES key.
	//  Authentication with that key is necessary for changing it.
	if (iTestMode == TM_LEGACY)
	{
		rc = SPROX_Desfire_Authenticate(5, abNullKey);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKey(5, appBChangeKey_16, NULL);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES2K)
	{
		rc = SPROX_Desfire_AuthenticateIso24(5, abNullKey);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKey(5, appBChangeKey_16, NULL);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES3K)
	{
		rc = SPROX_Desfire_AuthenticateIso24(5, abNullKey);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKey24(5, appBChangeKey_24, NULL);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_AES)
	{
		rc = SPROX_Desfire_AuthenticateAes(5, abNullKey);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKeyAes(5, 0x77, appBChangeKey_16, NULL);
		CHECK_RC();
	}

	//  Get authenticated again with appl's Master key
	if (iTestMode == TM_LEGACY)
	{
		rc = SPROX_Desfire_Authenticate(0, appBKeyMaster0_16);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES2K)
	{
		rc = SPROX_Desfire_AuthenticateIso(0, appBKeyMaster0_16);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES3K)
	{
		rc = SPROX_Desfire_AuthenticateIso24(0, appBKeyMaster0_24);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_AES)
	{
		rc = SPROX_Desfire_AuthenticateAes(0, appBKeyMaster0_16);
		CHECK_RC();
	}

	if (!fLegacyCard)
	{
		rc = SPROX_Desfire_GetFreeMemory(&dwFreeBytes);
		CHECK_RC();
		rc = SPROX_Desfire_GetFreeMemory(&dwFreeBytes);
		CHECK_RC();
	}

	//  Make key #5 the Change Keys Key.
	rc = SPROX_Desfire_ChangeKeySettings(0x5F);
	CHECK_RC();

	//  Verify the new key settings.
	rc = SPROX_Desfire_GetKeySettings(&abDataBuffer[0], &abDataBuffer[1]);
	CHECK_RC();

	//  Change keys #1 through #4 using the three key procedure.
	//  Authentication with the Change Keys Key is now necessary for changing ordinary keys.
	if (iTestMode == TM_LEGACY)
	{
		rc = SPROX_Desfire_Authenticate(5, appBChangeKey_16);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKey(1, appBKey1_16, abTestKeyDes1K);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKey(2, appBKey2_16, abNullKey);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKey(3, appBKey3_16, abNullKey);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKey(4, appBKey4_16, abNullKey);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES2K)
	{
		rc = SPROX_Desfire_AuthenticateIso(5, appBChangeKey_16);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKey(1, appBKey1_16, abTestKeyDes1K);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKey(2, appBKey2_16, abNullKey);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKey(3, appBKey3_16, abNullKey);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKey(4, appBKey4_16, abNullKey);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES3K)
	{
		rc = SPROX_Desfire_AuthenticateIso24(5, appBChangeKey_24);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKey24(1, appBKey1_24, abTestKeyDes1K);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKey24(2, appBKey2_24, abNullKey);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKey24(3, appBKey3_24, abNullKey);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKey24(4, appBKey4_24, abNullKey);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_AES)
	{
		rc = SPROX_Desfire_AuthenticateAes(5, appBChangeKey_16);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKeyAes(1, 0x33, appBKey1_16, abTestKeyDes1K);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKeyAes(2, 0x55, appBKey2_16, abNullKey);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKeyAes(3, 0x33, appBKey3_16, abNullKey);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKeyAes(4, 0x55, appBKey4_16, abNullKey);
		CHECK_RC();
	}

	//  For demonstrating the three possible communication modes we create an instance
	//  of each basic file type:
	//  a Data file:  Read = 1, Write = 2, ReadWrite = 3, ChangeConfig = 4
	//  a Value file: Debit = 1, LimitedCredit = 3, Credit = 2, ChangeConfig = 4
	//  and a Linear Record file: Read = 1, Write = 3, ReadWrite = 2, ChangeConfig = 4
	//  Note: Must make sure that buffer is large enough for all data!
	bStdDataFileID--;
	rc = SPROX_Desfire_CreateStdDataFile(bStdDataFileID, 0, 0x1234, 100);
	CHECK_RC();
	rc = SPROX_Desfire_CreateValueFile(4, 0, 0x1324, -987654321, -1000, -1000000, 1);
	CHECK_RC();
	rc = SPROX_Desfire_CreateLinearRecordFile(1, 0, 0x1324, 25, 4);
	CHECK_RC();

	if (!fLegacyCard)
	{
		rc = SPROX_Desfire_GetFreeMemory(&dwFreeBytes);
		CHECK_RC();
	}

	//  Do 7 transactions.
	for (iTransaction = 0; iTransaction < 7; iTransaction++)
	{
		//  Change the communication mode for all files.
		bCommMode = iTransaction % 3;
		if (bCommMode == 2) bCommMode = 3;

		//  This requires authentication with the key defined for changing the configuration.
		if (iTestMode == TM_LEGACY)
		{
			rc = SPROX_Desfire_Authenticate(4, appBKey4_16);
			CHECK_RC();
		}
		else if (iTestMode == TM_ISO_3DES2K)
		{
			rc = SPROX_Desfire_AuthenticateIso(4, appBKey4_16);
			CHECK_RC();
		}
		else if (iTestMode == TM_ISO_3DES3K)
		{
			rc = SPROX_Desfire_AuthenticateIso24(4, appBKey4_24);
			CHECK_RC();
		}
		else if (iTestMode == TM_ISO_AES)
		{
			rc = SPROX_Desfire_AuthenticateAes(4, appBKey4_16);
			CHECK_RC();
		}

		rc = SPROX_Desfire_ChangeFileSettings(bStdDataFileID, bCommMode, 0x1234);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeFileSettings(4, bCommMode, 0x1324);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeFileSettings(1, bCommMode, 0x1324);
		CHECK_RC();

		//  Authenticate with the key which allows reading the files
		if (iTestMode == TM_LEGACY)
		{
			rc = SPROX_Desfire_Authenticate(1, appBKey1_16);
			CHECK_RC();
		}
		else if (iTestMode == TM_ISO_3DES2K)
		{
			rc = SPROX_Desfire_AuthenticateIso(1, appBKey1_16);
			CHECK_RC();
		}
		else if (iTestMode == TM_ISO_3DES3K)
		{
			rc = SPROX_Desfire_AuthenticateIso24(1, appBKey1_24);
			CHECK_RC();
		}
		else if (iTestMode == TM_ISO_AES)
		{
			rc = SPROX_Desfire_AuthenticateAes(1, appBKey1_16);
			CHECK_RC();
		}

		if (iTransaction < 3)
		{
			//  Test different lengths
			for (eLength = 45; eLength < 65; eLength++)
			{
				rc = SPROX_Desfire_ReadData(bStdDataFileID, bCommMode, 100 - eLength, eLength, abDataBuffer, &dwNbBytesRead);
				CHECK_RC();
			}
			for (eLength = 10; eLength <= 100; eLength += 10)
			{
				rc = SPROX_Desfire_ReadData(bStdDataFileID, bCommMode, 100 - eLength, eLength, abDataBuffer, &dwNbBytesRead);
				CHECK_RC();
			}
		}


		rc = SPROX_Desfire_ReadData(bStdDataFileID, bCommMode, 0, 0, abDataBuffer, &dwNbBytesRead);
		CHECK_RC();

		//  Authenticate with the key which allows writing to files and increasing the Value
		//  file's value.
		if (iTestMode == TM_LEGACY)
		{
			rc = SPROX_Desfire_Authenticate(2, appBKey2_16);
			CHECK_RC();
		}
		else if (iTestMode == TM_ISO_3DES2K)
		{
			rc = SPROX_Desfire_AuthenticateIso(2, appBKey2_16);
			CHECK_RC();
		}
		else if (iTestMode == TM_ISO_3DES3K)
		{
			rc = SPROX_Desfire_AuthenticateIso24(2, appBKey2_24);
			CHECK_RC();
		}
		else if (iTestMode == TM_ISO_AES)
		{
			rc = SPROX_Desfire_AuthenticateAes(2, appBKey2_16);
			CHECK_RC();
		}

		//  Write to the Standard Data file.
		for (i = 0; i < 100; i++)
			abDataBuffer[i] = ((iTransaction << 6) + i + 0);

		if (iTransaction < 3)
		{
			rc = SPROX_Desfire_WriteData(bStdDataFileID, bCommMode, 0, 8, abDataBuffer);
			CHECK_RC();
			rc = SPROX_Desfire_WriteData(bStdDataFileID, bCommMode, 0, 16, abDataBuffer);
			CHECK_RC();

			rc = SPROX_Desfire_WriteData(bStdDataFileID, bCommMode, 0, 1, abDataBuffer);
			CHECK_RC();
			rc = SPROX_Desfire_WriteData(bStdDataFileID, bCommMode, 0, 2, abDataBuffer);
			CHECK_RC();


			rc = SPROX_Desfire_WriteData(bStdDataFileID, bCommMode, 0, 10, abDataBuffer);
			CHECK_RC();

			rc = SPROX_Desfire_WriteData(bStdDataFileID, bCommMode, 10, 10, abDataBuffer);
			CHECK_RC();

			rc = SPROX_Desfire_WriteData(bStdDataFileID, bCommMode, 20, 10, abDataBuffer);
			CHECK_RC();

			rc = SPROX_Desfire_WriteData(bStdDataFileID, bCommMode, 30, 70, abDataBuffer);
			CHECK_RC();
		}

		rc = SPROX_Desfire_WriteData(bStdDataFileID, bCommMode, 0, 100, abDataBuffer);
		CHECK_RC();

		snprintf((char*)abDataBuffer, sizeof(abDataBuffer), " Transaction #%d ", iTransaction);
		rc = SPROX_Desfire_WriteData(bStdDataFileID, bCommMode, 5, strlen((char*)abDataBuffer), abDataBuffer);
		CHECK_RC();

		//  Write to the Linear Record file.
		//  If it turns out, that the file is full, clear its contents and repeat writing.
		for (i = 0; i < 2; i++)
		{
			rc = SPROX_Desfire_WriteRecord(1, bCommMode, 0, 25,
				(unsigned char*)
				"0123456789012345678901234");
			if (rc == 0)
			{
				snprintf((char*)abDataBuffer, sizeof(abDataBuffer), " Transaction #%d ", iTransaction);
				rc = SPROX_Desfire_WriteRecord(1, bCommMode, 5,
					strlen((char*)abDataBuffer),
					abDataBuffer);
			}
			//  Was the WriteRecord successful ?
			if (rc == 0)
				break;              // yes

			// The current authentication status has been lost. We must get authenticated again
			if (iTestMode == TM_LEGACY)
			{
				rc = SPROX_Desfire_Authenticate(2, appBKey2_16);
				CHECK_RC();
			}
			else if (iTestMode == TM_ISO_3DES2K)
			{
				rc = SPROX_Desfire_AuthenticateIso(2, appBKey2_16);
				CHECK_RC();
			}
			else if (iTestMode == TM_ISO_3DES3K)
			{
				rc = SPROX_Desfire_AuthenticateIso24(2, appBKey2_24);
				CHECK_RC();
			}
			else if (iTestMode == TM_ISO_AES)
			{
				rc = SPROX_Desfire_AuthenticateAes(2, appBKey2_16);
				CHECK_RC();
			}

			//  Clear the record file.
			rc = SPROX_Desfire_ClearRecordFile(1);
			CHECK_RC();

			//  It is not allowed to write to the file before a CommitTransaction.
			//  So the following call will fail !
			rc = SPROX_Desfire_WriteRecord(1, bCommMode, 5,
				strlen((char*)abDataBuffer),
				abDataBuffer);
			if (rc != (DFCARD_ERROR - DF_PERMISSION_DENIED))
				CHECK_RC();

			// The current authentication status has been lost. We must get authenticated again
			if (iTestMode == TM_LEGACY)
			{
				rc = SPROX_Desfire_Authenticate(2, appBKey2_16);
				CHECK_RC();
			}
			else if (iTestMode == TM_ISO_3DES2K)
			{
				rc = SPROX_Desfire_AuthenticateIso(2, appBKey2_16);
				CHECK_RC();
			}
			else if (iTestMode == TM_ISO_3DES3K)
			{
				rc = SPROX_Desfire_AuthenticateIso24(2, appBKey2_24);
				CHECK_RC();
			}
			else if (iTestMode == TM_ISO_AES)
			{
				rc = SPROX_Desfire_AuthenticateAes(2, appBKey2_16);
				CHECK_RC();
			}

			//  Version 0.1 and above execute an implicit AbortTransaction after any
			//  error. Therefore the ClearRecordFile has been cancelled. We have to repeat it.
			rc = SPROX_Desfire_ClearRecordFile(1);
			CHECK_RC();

			//  After the following the Record file is again ready for data.
			rc = SPROX_Desfire_CommitTransaction();
			CHECK_RC();
		}

		//  Modify the Value file.
		rc = SPROX_Desfire_Debit(4, bCommMode, 1300);
		CHECK_RC();
		rc = SPROX_Desfire_Credit(4, bCommMode, 20);
		CHECK_RC();
		rc = SPROX_Desfire_Debit(4, bCommMode, 1700);
		CHECK_RC();

		//  Make all changes current.
		rc = SPROX_Desfire_CommitTransaction();
		CHECK_RC();

		//  Return the whole debited amount to the Value File.
		rc = SPROX_Desfire_LimitedCredit(4, bCommMode, 3000);
		CHECK_RC();

		//  Make the change current.
		rc = SPROX_Desfire_CommitTransaction();
		CHECK_RC();


		//  Authenticate with the key which allows reading files and retrieving the Value
		//  file's value.
		if (iTestMode == TM_LEGACY)
		{
			rc = SPROX_Desfire_Authenticate(1, appBKey1_16);
			CHECK_RC();
		}
		else if (iTestMode == TM_ISO_3DES2K)
		{
			rc = SPROX_Desfire_AuthenticateIso(1, appBKey1_16);
			CHECK_RC();
		}
		else if (iTestMode == TM_ISO_3DES3K)
		{
			rc = SPROX_Desfire_AuthenticateIso24(1, appBKey1_24);
			CHECK_RC();
		}
		else if (iTestMode == TM_ISO_AES)
		{
			rc = SPROX_Desfire_AuthenticateAes(1, appBKey1_16);
			CHECK_RC();
		}

		//  Read the first half of the Standard Data file's data specifying the exact
		//  number of bytes to read.
		rc = SPROX_Desfire_ReadData(bStdDataFileID, bCommMode, 0, 50, abDataBuffer, &dwNbBytesRead);
		CHECK_RC();
		//  Read the second half of the data with an open read (give me all data available).
		rc = SPROX_Desfire_ReadData(bStdDataFileID, bCommMode, 50, 0, abDataBuffer, &dwNbBytesRead);
		CHECK_RC();

		//  Get the Value file's current balance.
		rc = SPROX_Desfire_GetValue(4, bCommMode, &lAmount);
		CHECK_RC();

		//  Get the number of records in the Linear Record file.
		rc = SPROX_Desfire_GetFileSettings(1, 0, 0, 0, &unFileSettings);
		CHECK_RC();

		//  Read the oldest record from the file.
		rc = SPROX_Desfire_ReadRecords(1, bCommMode,
			unFileSettings.stRecordFileSettings.
			eCurrNRecords - 1, 1,
			unFileSettings.stRecordFileSettings.
			eRecordSize, abDataBuffer, &eNRecordsRead);
		CHECK_RC();

		//  Read all records from the file.
		rc = SPROX_Desfire_ReadRecords(1, bCommMode, 0, 0,
			unFileSettings.stRecordFileSettings.
			eRecordSize, abDataBuffer, &eNRecordsRead);
		CHECK_RC();
	}

	//  Get the file IDs of the current application.
	rc = SPROX_Desfire_GetFileIDs(32, abFiles, &bNFilesFound);
	CHECK_RC();

	//  Get information about the application's files.
	//  Delete each file after retrieving information about it.
	for (iFileIndex = 0; iFileIndex < bNFilesFound; iFileIndex++)
	{
		rc =
			SPROX_Desfire_GetFileSettings(abFiles[iFileIndex], &bFileType,
				&bCommMode, &wAccessRights,
				&unFileSettings);
		CHECK_RC();
		rc = SPROX_Desfire_DeleteFile(abFiles[iFileIndex]);
		CHECK_RC();
	}

	//  Verify that there are no files in the application.
	rc = SPROX_Desfire_GetFileIDs(32, abFiles, &bNFilesFound);
	CHECK_RC();

	//  Delete application B.
	//  This time we can (and do) use the Application Master Key.
	if (iTestMode == TM_LEGACY)
	{
		rc = SPROX_Desfire_Authenticate(0, appBKeyMaster0_16);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES2K)
	{
		rc = SPROX_Desfire_AuthenticateIso(0, appBKeyMaster0_16);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES3K)
	{
		rc = SPROX_Desfire_AuthenticateIso24(0, appBKeyMaster0_24);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_AES)
	{
		rc = SPROX_Desfire_AuthenticateAes(0, appBKeyMaster0_16);
		CHECK_RC();
	}

	rc = SPROX_Desfire_DeleteApplication(0xBBBBBB);
	CHECK_RC();

	//  Verify that application B has been deleted.
	rc = SPROX_Desfire_GetApplicationIDs(32, adwApplications, &bNApplicationsFound);
	CHECK_RC();

	if (dwFreeBytes < 2000)
	{
		rc = SPROX_Desfire_SelectApplication(0);
		CHECK_RC();

		if (iTestMode == TM_LEGACY)
		{
			rc = SPROX_Desfire_Authenticate(0, abRootKey2K);
			CHECK_RC();
		}
		else if (iTestMode == TM_ISO_3DES2K)
		{
			rc = SPROX_Desfire_AuthenticateIso(0, abRootKey2K);
			CHECK_RC();
		}
		else if (iTestMode == TM_ISO_3DES3K)
		{
			rc = SPROX_Desfire_AuthenticateIso24(0, abRootKey2K);
			CHECK_RC();
		}
		else if (iTestMode == TM_ISO_AES)
		{
			rc = SPROX_Desfire_AuthenticateAes(0, abRootKey2K);
			CHECK_RC();
		}

		rc = SPROX_Desfire_FormatPICC();
		CHECK_RC();

		if (iTestMode == TM_LEGACY)
		{
			rc = SPROX_Desfire_CreateApplication(0xCCCCCC, 0xC0, 14);
			CHECK_RC();
		}
		else if (iTestMode == TM_ISO_3DES2K)
		{
			rc = SPROX_Desfire_CreateApplication(0xCCCCCC, 0xC0, 14);
			CHECK_RC();
		}
		else if (iTestMode == TM_ISO_3DES3K)
		{
			rc = SPROX_Desfire_CreateApplication(0xCCCCCC, 0xC0, DF_APPLSETTING2_3DES3K | 14);
			CHECK_RC();
		}
		else if (iTestMode == TM_ISO_AES)
		{
			rc = SPROX_Desfire_CreateApplication(0xCCCCCC, 0xC0, DF_APPLSETTING2_AES | 14);
			CHECK_RC();
		}
	}

	//  Working with a larger file (a Cyclic Record File) using application C:
	rc = SPROX_Desfire_SelectApplication(0xCCCCCC);
	CHECK_RC();

	//  Define keys #1 and #2 using the Change Keys key (#12).
	if (iTestMode == TM_LEGACY)
	{
		rc = SPROX_Desfire_Authenticate(12, abNullKey);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKey(1, appCKey1_16, abNullKey);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKey(2, appCKey2_16, abNullKey);
		CHECK_RC();
		// Back to master key
		rc = SPROX_Desfire_Authenticate(0, abNullKey);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES2K)
	{
		rc = SPROX_Desfire_AuthenticateIso(12, abNullKey);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKey(1, appCKey1_16, abNullKey);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKey(2, appCKey2_16, abNullKey);
		CHECK_RC();
		// Back to master key
		rc = SPROX_Desfire_AuthenticateIso(0, abNullKey);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES3K)
	{
		rc = SPROX_Desfire_AuthenticateIso24(12, abNullKey);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKey24(1, appCKey1_24, abNullKey);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKey24(2, appCKey2_24, abNullKey);
		CHECK_RC();
		// Back to master key
		rc = SPROX_Desfire_AuthenticateIso24(0, abNullKey);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_AES)
	{
		rc = SPROX_Desfire_AuthenticateAes(12, abNullKey);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKeyAes(1, 0, appCKey1_16, abNullKey);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeKeyAes(2, 0, appCKey2_16, abNullKey);
		CHECK_RC();
		// Back to master key
		rc = SPROX_Desfire_AuthenticateAes(0, abNullKey);
		CHECK_RC();
	}

	//  Create the file - it has 16 records of 100 bytes
	rc = SPROX_Desfire_CreateCyclicRecordFile(6, 0, 0x12E0, 100, 16);
	CHECK_RC();

	//  Do 50 transactions.
	for (iTransaction = 0; iTransaction < 50; iTransaction++)
	{
		//  Change the file's communication mode.
		bCommMode = (iTransaction % 3);
		if (bCommMode == 2)
			bCommMode++;

		//  This requires authentication with the key defined for changing the configuration.
		//  For this file it is the Application Master Key.
		if (iTestMode == TM_LEGACY)
		{
			rc = SPROX_Desfire_Authenticate(0, abNullKey);
			CHECK_RC();
		}
		else if (iTestMode == TM_ISO_3DES2K)
		{
			rc = SPROX_Desfire_AuthenticateIso(0, abNullKey);
			CHECK_RC();
		}
		else if (iTestMode == TM_ISO_3DES3K)
		{
			rc = SPROX_Desfire_AuthenticateIso24(0, abNullKey);
			CHECK_RC();
		}
		else if (iTestMode == TM_ISO_AES)
		{
			rc = SPROX_Desfire_AuthenticateAes(0, abNullKey);
			CHECK_RC();
		}

		rc = SPROX_Desfire_ChangeFileSettings(6, bCommMode, 0x12E0);
		CHECK_RC();

		//  The file can be written using either the Write access key (0x.2..) or the
		//  Read/Write key (0x..E.).
		//  A valid authentication with key #2 causes the communication to take place in
		//  the mode defined with ChangeFileSettings above.
		//  Not being authenticated with key #2 causes the Read/Write access right to be used
		//  forcing the communication to plain mode.
		if (iTransaction & 4)
		{
			if (iTestMode == TM_LEGACY)
			{
				rc = SPROX_Desfire_Authenticate(2, appCKey2_16);
				CHECK_RC();
			}
			else if (iTestMode == TM_ISO_3DES2K)
			{
				rc = SPROX_Desfire_AuthenticateIso(2, appCKey2_16);
				CHECK_RC();
			}
			else if (iTestMode == TM_ISO_3DES3K)
			{
				rc = SPROX_Desfire_AuthenticateIso24(2, appCKey2_24);
				CHECK_RC();
			}
			else if (iTestMode == TM_ISO_AES)
			{
				rc = SPROX_Desfire_AuthenticateAes(2, appCKey2_16);
				CHECK_RC();
			}
		}
		else
			bCommMode = 0;

		//  Write a new record.
		memset(abDataBuffer, 0, 100);
		snprintf((char*)abDataBuffer, sizeof(abDataBuffer), " Transaction #%d !TEST!?!TEST! ", iTransaction);
		rc = SPROX_Desfire_WriteRecord(6, bCommMode, 0, 100, abDataBuffer);

		//  Cancel every 7th WriteRecord operation.
		if (iTransaction % 7 == 0)
			rc = SPROX_Desfire_AbortTransaction();
		else
			rc = SPROX_Desfire_CommitTransaction();

		if (rc != (DFCARD_ERROR - DF_NO_CHANGES))
			if (rc != (DFCARD_ERROR - DF_COMMAND_ABORTED))
				CHECK_RC();
	}

	//  Read each individual record using the designated read key.
	//  Note that increasing the offset lets us go back in the record history.
	//  Note also that only 16-1 records (one less than specified in CreateCyclicRecordFile)
	//  can be accessed.
	bCommMode = 1;

	if (iTestMode == TM_LEGACY)
	{
		rc = SPROX_Desfire_Authenticate(1, appCKey1_16);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES2K)
	{
		rc = SPROX_Desfire_AuthenticateIso(1, appCKey1_16);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES3K)
	{
		rc = SPROX_Desfire_AuthenticateIso24(1, appCKey1_24);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_AES)
	{
		rc = SPROX_Desfire_AuthenticateAes(1, appCKey1_16);
		CHECK_RC();
	}

	for (eOffset = 0; eOffset < 16; eOffset++)
	{
		rc = SPROX_Desfire_ReadRecords(6, bCommMode, eOffset, 1, 100, abDataBuffer, &eNRecordsRead);
		if (rc != (DFCARD_ERROR - DF_BOUNDARY_ERROR))
			CHECK_RC();
	}

	//  Following the Boundary Error we lose the authentication - so we must get authenticated again
	if (iTestMode == TM_LEGACY)
	{
		rc = SPROX_Desfire_Authenticate(1, appCKey1_16);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES2K)
	{
		rc = SPROX_Desfire_AuthenticateIso(1, appCKey1_16);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES3K)
	{
		rc = SPROX_Desfire_AuthenticateIso24(1, appCKey1_24);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_AES)
	{
		rc = SPROX_Desfire_AuthenticateAes(1, appCKey1_16);
		CHECK_RC();
	}


	rc = SPROX_Desfire_ReadRecords(6, bCommMode, 0, 1, 100, abDataBuffer, &eNRecordsRead);
	CHECK_RC();

	//  Read the whole record file with a single ReadRecords call.
	rc = SPROX_Desfire_ReadRecords(6, bCommMode, 0, 15, 100, abDataBuffer, &eNRecordsRead);
	CHECK_RC();

	rc = SPROX_Desfire_ReadRecords(6, bCommMode, 0, 0, 100, abDataBuffer, &eNRecordsRead);
	CHECK_RC();

	//  Change the master key settings such that master key authentication will be required
	//  for all card operations.
	//  Leave the ChangeConfiguration bit (bit 3) set to allow undoing the change.
	rc = SPROX_Desfire_SelectApplication(0);
	CHECK_RC();

	if (iTestMode == TM_LEGACY)
	{
		rc = SPROX_Desfire_Authenticate(0, abRootKey2K);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES2K)
	{
		rc = SPROX_Desfire_AuthenticateIso(0, abRootKey2K);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES3K)
	{
		rc = SPROX_Desfire_AuthenticateIso24(0, abRootKey2K);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_AES)
	{
		rc = SPROX_Desfire_AuthenticateAes(0, abRootKey2K);
		CHECK_RC();
	}

	rc = SPROX_Desfire_ChangeKeySettings(0x8);
	CHECK_RC();

	//  Clear the authentication state and try to get application IDs.
	//  Also try to delete application C or to create a new application.
	rc = SPROX_Desfire_SelectApplication(0);
	CHECK_RC();
	//  The following three calls will report an Authentication Error (0xAE).
	rc = SPROX_Desfire_GetApplicationIDs(32, adwApplications, &bNApplicationsFound);
	if (rc != (DFCARD_ERROR - DF_AUTHENTICATION_ERROR))
		CHECK_RC();
	rc = SPROX_Desfire_DeleteApplication(0xCCCCCC);
	if (rc != (DFCARD_ERROR - DF_AUTHENTICATION_ERROR))
		CHECK_RC();

	// This one must fail
	rc = SPROX_Desfire_CreateApplication(0xDDDDDD, 0xEF, 0);
	if (rc != (DFCARD_ERROR - DF_AUTHENTICATION_ERROR))
		CHECK_RC();

	//  With authentication all three should work.
	if (iTestMode == TM_LEGACY)
	{
		rc = SPROX_Desfire_Authenticate(0, abRootKey2K);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES2K)
	{
		rc = SPROX_Desfire_AuthenticateIso(0, abRootKey2K);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES3K)
	{
		rc = SPROX_Desfire_AuthenticateIso24(0, abRootKey2K);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_AES)
	{
		rc = SPROX_Desfire_AuthenticateAes(0, abRootKey2K);
		CHECK_RC();
	}

	rc = SPROX_Desfire_GetApplicationIDs(32, adwApplications, &bNApplicationsFound);
	CHECK_RC();
	rc = SPROX_Desfire_DeleteApplication(0xCCCCCC);
	CHECK_RC();
	rc = SPROX_Desfire_CreateApplication(0xDDDDDD, 0xEF, 0);
	CHECK_RC();

	//  Change the master key settings back to the default which is least restrictive.
	rc = SPROX_Desfire_ChangeKeySettings(0xF);
	CHECK_RC();

	//  Change the Card master key back to the 0 key.
	if (iTestMode == TM_LEGACY)
	{
		rc = SPROX_Desfire_Authenticate(0, abRootKey2K);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES2K)
	{
		rc = SPROX_Desfire_AuthenticateIso(0, abRootKey2K);
		CHECK_RC();
	}
	else if (iTestMode == TM_ISO_3DES3K)
	{
		rc = SPROX_Desfire_AuthenticateIso24(0, abRootKey2K);
		CHECK_RC();

		//  for obvious reasons we cannot revert the config to 3DES2K, but a hook by AES permits it
		rc = SPROX_Desfire_ChangeKeyAes(DF_APPLSETTING2_AES | 0, 0xAE, abRootKey2K, NULL);
		CHECK_RC();
		rc = SPROX_Desfire_AuthenticateAes(0, abRootKey2K);
		CHECK_RC();

	}
	else if (iTestMode == TM_ISO_AES)
	{
		rc = SPROX_Desfire_AuthenticateAes(0, abRootKey2K);
		CHECK_RC();
	}

	rc = SPROX_Desfire_ChangeKey(0, abNullKey, NULL);
	CHECK_RC();

	//  Delete all demonstration data from the card.
	rc = SPROX_Desfire_Authenticate(0, abNullKey);
	CHECK_RC();

	if (!fLegacyCard)
	{
		rc = SPROX_Desfire_GetFreeMemory(&dwFreeBytes);
		CHECK_RC();
	}

	rc = SPROX_Desfire_FormatPICC();
	CHECK_RC();

	printf("\n");
	return TRUE;
}

