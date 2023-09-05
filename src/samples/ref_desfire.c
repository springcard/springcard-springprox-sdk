/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  Copyright (c) 2003-2014 PRO ACTIVE SAS, FRANCE - www.springcard.com

  ref_desfire.c
  -------------

  This is the reference applications that validates the whole SpringProx API
  for NXP DESFire (EV0) cards.

  JDA 21/11/2003 : initial release
  JDA 19/03/2004 : cosmetic corrections
  JDA 27/07/2006 : application AAAAAA now testing a file of 550 bytes
				   instead of 100
  JDA 10/01/2007 : fixed a few warnings when compiling under GCC
  JDA 04/11/2010 : changed parsing of args to be coherent with desfire_ev1
  JDA 04/02/2013 : minor changes to adapt to release 1.7x of the SDK
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

const char* PROGRAM_NAME = "ref_desfire";
const char* szCommDevice = NULL;
BOOL fLoop = FALSE;
BOOL fAgain = FALSE;
BYTE dDSI = 0;
BYTE dDRI = 0;
BYTE dCID = 0xFF;
BOOL fIsoWrapping = FALSE;

static BOOL parse_args(int argc, char** argv);
static void usage(void);
static BOOL sprox_desfire_library_selftest(void);

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

	/* Display the informations and check the command line */
	/* --------------------------------------------------- */

	printf("SpringCard SpringProx 'Legacy' SDK\n");
	printf("\n");
	printf("%s : NXP MIFARE DESFire EV0 validation & sample\n\n", PROGRAM_NAME);
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

	/* Do the test */
	/* ----------- */

	if (!sprox_desfire_library_selftest())
	{
		goto deselect;
	}

	printf("\nSUCCESS!\n");

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
	if (rc != MI_OK)
		printf("%s (%d)\n", SPROX_GetErrorMessageA(rc), rc);

	return EXIT_SUCCESS;
}


static void usage(void)
{
	printf("Usage: %s [BAUDRATES] [OPTIONS] [-d <COMM DEVICE] [-v]\n", PROGRAM_NAME);
	printf("BAUDRATES:\n");
	printf(" -dr0, -dr1, -dr2 or -dr3 for PCD->PICC baudrate\n");
	printf(" -ds0, -ds1, -ds2 or -ds3 for PICC->PCD baudrate\n");
	printf("Default is -dr0 -ds0 (106kbit/s both directions)\n");
	printf("OPTIONS:\n");
	printf(" -l : (loop)  the software waits for another card and continue.\n");
	printf(" -a : (again) the software runs the test on the same card again.\n");
	printf(" -s : (show)  the software shows some details.\n");
	printf(" -i : (iso)   ISO/IEC 7816-4 wrapping is used.\n");
	printf(" -c <CID>     sets the CID for T=CL\n");
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
			else if (!strcmp(argv[i], "-i"))
			{
				fIsoWrapping = TRUE;
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


/* Check DESFire library functions */
/* ------------------------------- */


void DesFire_xDES_Recv(BYTE* data, WORD blocks, const BYTE* key);

#ifdef WIN32
#ifdef LOOP_TEST
#define CHECK_RC() { if (rc!=0) { Beep(880,1000); printf("\nline %d - failed (%d) %s\n", __LINE__-1, rc, SPROX_Desfire_GetErrorMessage(rc)); err_count++; return FALSE; } printf("."); fflush(NULL); }
#else
#define CHECK_RC() { if (rc!=0) { printf("\nline %d - failed (%d) %s\n", __LINE__-1, rc, SPROX_Desfire_GetErrorMessage(rc)); return FALSE; } printf("."); fflush(NULL); }
#endif
#endif

#ifdef __linux__
#define CHECK_RC() { if (rc!=0) { printf("\nline %d - failed (%d) %s\n", __LINE__-1, rc, SPROX_Desfire_GetErrorMessage(rc)); return FALSE; } printf("."); fflush(NULL); }
#endif

#ifdef LOOP_TEST
WORD loop_count = 0;
WORD err_count = 0;
#endif

/* Les cles utilises dans ce test */
const BYTE abNullKey[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
const BYTE ab1DesKey[16] = { "ABCDEFGHABCDEFGH" };
const BYTE ab3DesKey[16] = { "Card Master Key!" };

const BYTE ab3DesTestKey[16] = { "ABCDEFGHIJKLMNOP" };

static BOOL sprox_desfire_library_selftest(void)
{

	SWORD rc;
	LONG lAmount;
	BYTE bCommMode, bKeyVersion;
	DWORD eNBytesRead, eNRecordsRead, eOffset;
	BYTE abCardMasterKey[16];
	BYTE bStdDataFileID, bFileType, bNFilesFound, bNApplicationsFound;
	BYTE temp;
	BYTE abDataBuffer[550];
	WORD wDesFireSoftwareVersion, wAccessRights;
	int i, iTransaction, iFileIndex;

	DF_VERSION_INFO stVersionInfo;
	DF_ADDITIONAL_FILE_SETTINGS unFileSettings;

next_run:

	//  After activating a DesFire card the currently selected 'application' is the card
	//  level (AID = 0). Therefore the next command is redundant.
	//  We use the command to check that the card is responding correctly.
	rc = SPROX_Desfire_SelectApplication(0);
	CHECK_RC();

	//  Get the card's version information.
	rc = SPROX_Desfire_GetVersion(&stVersionInfo);
	CHECK_RC();
	wDesFireSoftwareVersion =
		(unsigned int)((unsigned int)stVersionInfo.
			bSwMajorVersion << 8 | stVersionInfo.bSwMinorVersion);


	//  Which Card Master Key is currently in use ?
	//  Beginning with DesFire version 0.1 the GetKeyVersion command can help determining
	//  which key is currently effective.
	rc = SPROX_Desfire_GetKeyVersion(0, &bKeyVersion);
	CHECK_RC();

	//  Authenticate with the key which corresponds to the retieved key version.
	if (bKeyVersion == 0)
		rc = SPROX_Desfire_Authenticate(0, abNullKey);
	else if (bKeyVersion == 0xAA)
		rc = SPROX_Desfire_Authenticate(0, ab1DesKey);
	else if (bKeyVersion == 0xC7)
		rc = SPROX_Desfire_Authenticate(0, ab3DesKey);
	else
	{
		//  Who knows the key ?
		printf("Sorry, the master key is unknown (version = %02X)\n",
			bKeyVersion);
		return FALSE;
	}
	CHECK_RC();

	if (bKeyVersion != 0x00)
	{
		printf("Current master key version is %02X\n", bKeyVersion);
		rc = SPROX_Desfire_GetKeySettings(&abDataBuffer[0], &abDataBuffer[1]);
		CHECK_RC();
		printf("Root application key settings are %02X %02X\n",
			abDataBuffer[0], abDataBuffer[1]);

		//  Try to allow to change the key
		rc = SPROX_Desfire_ChangeKeySettings(0xF);
		CHECK_RC();

		//  Back to blank key
		rc = SPROX_Desfire_ChangeKey(0, abNullKey, NULL);
		CHECK_RC();

		//  Authenticate again
		rc = SPROX_Desfire_Authenticate(0, abNullKey);
		CHECK_RC();

		//  Startup with a blank card !!!
		printf("Formatting the tag\n");
		rc = SPROX_Desfire_FormatPICC();
		CHECK_RC();

		goto next_run;
	}
	//  Change the Card master key to a SingleDES key.
	memcpy(abCardMasterKey, ab1DesKey, 16);
	rc = SPROX_Desfire_ChangeKey(0, abCardMasterKey, NULL);
	CHECK_RC();
	rc = SPROX_Desfire_GetKeyVersion(0, &bKeyVersion);
	CHECK_RC();
	rc = SPROX_Desfire_Authenticate(0, abCardMasterKey);
	CHECK_RC();

	//  Change the Card master key to a TripleDES key.
	memcpy(abCardMasterKey, ab3DesKey, 16);
	rc = SPROX_Desfire_ChangeKey(0, abCardMasterKey, NULL);
	CHECK_RC();
	rc = SPROX_Desfire_GetKeyVersion(0, &bKeyVersion);
	CHECK_RC();
	rc = SPROX_Desfire_Authenticate(0, abCardMasterKey);
	CHECK_RC();

	//  At this point we would like to begin with a blank card.
	rc = SPROX_Desfire_FormatPICC();
	CHECK_RC();

	//  Create three applications:

	//  Authentication is not necessary in the default configuration.
	//  Clear the authentication state to show this.
	rc = SPROX_Desfire_SelectApplication(0);
	CHECK_RC();

	//  Application A is the most open one.
	//  It has no keys and its key settings are least restrictive.
	//  Note: since there are no keys, the Change Keys key settting must be assigned
	//        the value 0xE or 0xF.
	rc = SPROX_Desfire_CreateApplication(0xAAAAAA, 0xFF, 0);
	CHECK_RC();

	//  Application B's key settings can be changed at a later time.
	//  It has six keys.
	//  Authentication with a given key allows also to change that key.
	//  (the Change Keys key settting is 0xE)
	rc = SPROX_Desfire_CreateApplication(0xBBBBBB, 0xEF, 6);
	CHECK_RC();

	//  Application C keeps everything private.
	//  Even getting its file directory is not publicly allowed.
	//  The application has the maximum of 14 keys.
	//  We make key #0xC the Change Keys key.
	rc = SPROX_Desfire_CreateApplication(0xCCCCCC, 0xC0, 14);
	CHECK_RC();

	//  Verify, that the applications which have been just created exist.
	rc = SPROX_Desfire_GetApplicationIDs(10, (DWORD*)abDataBuffer, &temp);
	CHECK_RC();

	//  Create the files in application A:
	//  Since the application does not have any keys associated with it, the file access rights
	//  settings 0xE (public access granted) and 0xF (access denied) are the only permissable
	//  ones.
	rc = SPROX_Desfire_SelectApplication(0xAAAAAA);
	CHECK_RC();

	//  Create a the Standard Data file.
	bStdDataFileID = 15;
	rc =
		SPROX_Desfire_CreateStdDataFile(bStdDataFileID, 0, 0xEEEE,
			sizeof(abDataBuffer));
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
	memset(abDataBuffer, 0xCC, sizeof(abDataBuffer));
	rc =
		SPROX_Desfire_WriteData(bStdDataFileID, DF_COMM_MODE_PLAIN, 0,
			sizeof(abDataBuffer), abDataBuffer);
	CHECK_RC();
	rc =
		SPROX_Desfire_WriteData(bStdDataFileID, DF_COMM_MODE_PLAIN, 0, 30,
			(unsigned char*)
			"This is the 1st block written.");
	CHECK_RC();
	rc =
		SPROX_Desfire_WriteData(bStdDataFileID, DF_COMM_MODE_PLAIN, 34, 22,
			(unsigned char*)"This is the 2nd block.");
	CHECK_RC();

	//  Then make the file permanently read-only.
	rc =
		SPROX_Desfire_ChangeFileSettings(bStdDataFileID, DF_COMM_MODE_PLAIN,
			0xEFFF);
	CHECK_RC();

	//  Read part of the file's contents.
	rc =
		SPROX_Desfire_ReadData(bStdDataFileID, DF_COMM_MODE_PLAIN, 10, 50,
			abDataBuffer, &eNBytesRead);
	CHECK_RC();

	//  Get all data in one block. 
	//  Note: Must make sure that buffer is large enough for all data!!
	rc =
		SPROX_Desfire_ReadData(bStdDataFileID, DF_COMM_MODE_PLAIN, 0, 0,
			abDataBuffer, &eNBytesRead);
	CHECK_RC();

	//  Try to overwrite the file.
	//  Since we have made the file read-only, this should fail.
	rc =
		SPROX_Desfire_WriteData(bStdDataFileID, DF_COMM_MODE_PLAIN, 20, 5,
			(unsigned char*)"Test!");
	if (rc != (DFCARD_ERROR - DF_PERMISSION_DENIED))
		CHECK_RC();

	//  Do 15 transactions.
	for (iTransaction = 0; iTransaction < 15; iTransaction++)
	{
		//  Write to the Backup Data file.
		snprintf((char*)abDataBuffer, sizeof(abDataBuffer), "%02d,", iTransaction);
		rc =
			SPROX_Desfire_WriteData(5, DF_COMM_MODE_PLAIN, 3 * iTransaction, 3,
				abDataBuffer);
		CHECK_RC();

		//  Manipulate the Value file.
		rc = SPROX_Desfire_Credit(4, DF_COMM_MODE_PLAIN, 100);
		CHECK_RC();
		rc = SPROX_Desfire_Debit(4, DF_COMM_MODE_PLAIN, 93);
		CHECK_RC();

		//  Write to the Cyclic Record file.
		rc =
			SPROX_Desfire_WriteRecord(0, DF_COMM_MODE_PLAIN, 2, 2, abDataBuffer);
		CHECK_RC();
		//  The following Write Record will write to the same record as above
		rc =
			SPROX_Desfire_WriteRecord(0, DF_COMM_MODE_PLAIN, 0, 2,
				(unsigned char*)"r.");
		CHECK_RC();

		//  Verify that the 'official' contents of the three files has not changed
		//  before the CommitTransaction command.
		//  Must make sure that buffer is large enough for all data!
		rc =
			SPROX_Desfire_ReadData(5, DF_COMM_MODE_PLAIN, 0, 0, abDataBuffer,
				&eNBytesRead);
		CHECK_RC();
		rc = SPROX_Desfire_GetValue(4, DF_COMM_MODE_PLAIN, &lAmount);
		CHECK_RC();
		//  Note: reading from an empty record file returns an error.
		//        beginning with version 0.1 this aborts the transaction.
		if (iTransaction != 0)
		{
			rc =
				SPROX_Desfire_ReadRecords(0, DF_COMM_MODE_PLAIN, 0, 0, 4,
					abDataBuffer, &eNRecordsRead);
			CHECK_RC();
		}
		//  Declare the transaction valid.
		rc = SPROX_Desfire_CommitTransaction();
		CHECK_RC();

		//  Verify that the transaction has become effective.
		//  Note: Must make sure that buffer is large enough for all data!
		rc =
			SPROX_Desfire_ReadData(5, DF_COMM_MODE_PLAIN, 0, 0, abDataBuffer,
				&eNBytesRead);
		CHECK_RC();
		rc = SPROX_Desfire_GetValue(4, DF_COMM_MODE_PLAIN, &lAmount);
		CHECK_RC();
		rc =
			SPROX_Desfire_ReadRecords(0, DF_COMM_MODE_PLAIN, 0, 0, 4,
				abDataBuffer, &eNRecordsRead);
		CHECK_RC();
	}

	//  Limited Credit has been disabled, so the following call should fail.
	rc = SPROX_Desfire_LimitedCredit2(4, 20);
	if (rc != (DFCARD_ERROR - DF_PERMISSION_DENIED))
		CHECK_RC();

	//  Get the file IDs of the current application.
	rc = SPROX_Desfire_GetFileIDs(15, abDataBuffer, &bNFilesFound);
	CHECK_RC();

	//  Get information about the application's files.
	//  Delete each file after retrieving information about it.
	for (iFileIndex = 0; iFileIndex < bNFilesFound; iFileIndex++)
	{
		rc =
			SPROX_Desfire_GetFileSettings(abDataBuffer[iFileIndex], &bFileType,
				&bCommMode, &wAccessRights,
				&unFileSettings);
		CHECK_RC();
		rc = SPROX_Desfire_DeleteFile(abDataBuffer[iFileIndex]);
		CHECK_RC();
	}

	//  Verify that there are no files in the application.
	rc = SPROX_Desfire_GetFileIDs(15, abDataBuffer, &bNFilesFound);
	CHECK_RC();

	//  Delete application A.
	//  Since this application doesn't have an Application Master Key,
	//  this requires Card Master Key authentication.
	rc = SPROX_Desfire_SelectApplication(0);
	CHECK_RC();
	rc = SPROX_Desfire_Authenticate(0, abCardMasterKey);
	CHECK_RC();
	rc = SPROX_Desfire_DeleteApplication(0xAAAAAA);
	CHECK_RC();

	//  Verify that application A has been deleted.
	rc =
		SPROX_Desfire_GetApplicationIDs(10, (DWORD*)abDataBuffer,
			&bNApplicationsFound);
	CHECK_RC();

	//  Changing application B's keys:
	rc = SPROX_Desfire_SelectApplication(0xBBBBBB);
	CHECK_RC();

	//  Use a TripleDES key as the Application Master Key.
	rc = SPROX_Desfire_Authenticate(0, abNullKey);
	CHECK_RC();
	rc = SPROX_Desfire_ChangeKey(0, (unsigned char*)"App.B Master Key", NULL);
	CHECK_RC();

	//  Beginning with DesFire version 0.1 we can query the version number of a key.
	rc = SPROX_Desfire_GetKeyVersion(0, &bKeyVersion);
	CHECK_RC();

	//  Authenticate with the new application master key.
	//  This time use the opposite parity in all key bytes.
	rc = SPROX_Desfire_Authenticate(0, (unsigned char*)"@qq/C!L`ruds!Kdx");
	CHECK_RC();

	//  Change key #1 to a different SingleDES key (both key halves are the same).
	//  Authentication with that key is necessary for changing it.
	rc = SPROX_Desfire_Authenticate(1, abNullKey);
	CHECK_RC();
	rc = SPROX_Desfire_ChangeKey(1, (unsigned char*)"SglDES_1SglDES_1", NULL);
	CHECK_RC();

	//  Change key #5 to a TripeDES key.
	//  Authentication with that key is necessary for changing it.
	rc = SPROX_Desfire_Authenticate(5, abNullKey);
	CHECK_RC();
	rc = SPROX_Desfire_ChangeKey(5, (unsigned char*)"B's Chg Keys Key", NULL);
	CHECK_RC();

	//  Make key #5 the Change Keys Key.
	rc = SPROX_Desfire_Authenticate(0, (unsigned char*)"App.B Master Key");
	CHECK_RC();
	rc = SPROX_Desfire_ChangeKeySettings(0x5F);
	CHECK_RC();

	//  Verify the new key settings.
	rc = SPROX_Desfire_GetKeySettings(&abDataBuffer[0], &abDataBuffer[1]);
	CHECK_RC();

	//  Change keys #1 through #4 using the three key procedure.
	//  Authentication with the Change Keys Key is now necessary for changing ordinary keys.
	rc = SPROX_Desfire_Authenticate(5, (unsigned char*)"B's Chg Keys Key");
	CHECK_RC();
	rc =
		SPROX_Desfire_ChangeKey(1, (unsigned char*)"App.B Key #1.   ",
			(unsigned char*)"SglDES_1SglDES_1");
	CHECK_RC();
	rc =
		SPROX_Desfire_ChangeKey(2, (unsigned char*)"App.B Key #2..  ",
			abNullKey);
	CHECK_RC();
	rc =
		SPROX_Desfire_ChangeKey(3, (unsigned char*)"App.B Key #3... ",
			abNullKey);
	CHECK_RC();
	rc =
		SPROX_Desfire_ChangeKey(4, (unsigned char*)"App.B Key #4....",
			abNullKey);
	CHECK_RC();

	//  For demonstrating the three possible communication modes we create an instance
	//  of each basic file type:
	//  a Data file:  Read = 1, Write = 2, ReadWrite = 3, ChangeConfig = 4
	//  a Value file: Debit = 1, LimitedCredit = 3, Credit = 2, ChangeConfig = 4
	//  and a Linear Record file: Read = 1, Write = 3, ReadWrite = 2, ChangeConfig = 4
	//  Note: Must make sure that buffer is large enough for all data!
	bStdDataFileID--;
	rc = SPROX_Desfire_CreateStdDataFile(bStdDataFileID, 0, 0x1234, 100);
	CHECK_RC();
	rc =
		SPROX_Desfire_CreateValueFile(4, 0, 0x1324, -987654321, -1000,
			-1000000, 1);
	CHECK_RC();
	rc = SPROX_Desfire_CreateLinearRecordFile(1, 0, 0x1324, 25, 4);
	CHECK_RC();

	//  Do 7 transactions.
	for (iTransaction = 0; iTransaction < 7; iTransaction++)
	{
		//  Change the communication mode for all files.
		bCommMode = iTransaction % 3;
		if (bCommMode == 2)
			bCommMode++;

		//  This requires authentication with the key defined for changing the configuration.
		rc =
			SPROX_Desfire_Authenticate(4, (unsigned char*)"App.B Key #4....");
		CHECK_RC();
		rc =
			SPROX_Desfire_ChangeFileSettings(bStdDataFileID, bCommMode, 0x1234);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeFileSettings(4, bCommMode, 0x1324);
		CHECK_RC();
		rc = SPROX_Desfire_ChangeFileSettings(1, bCommMode, 0x1324);
		CHECK_RC();

		//  Authenticate with the key which allows writing to files and increasing the Value
		//  file's value.
		rc =
			SPROX_Desfire_Authenticate(2, (unsigned char*)"App.B Key #2..  ");
		CHECK_RC();

		//  Write to the Standard Data file.
		for (i = 0; i < 100; i++)
			abDataBuffer[i] = iTransaction + i;
		rc =
			SPROX_Desfire_WriteData(bStdDataFileID, bCommMode, 0, 100,
				abDataBuffer);
		CHECK_RC();
		snprintf((char*)abDataBuffer, sizeof(abDataBuffer), " Transaction #%d ", iTransaction);
		rc =
			SPROX_Desfire_WriteData(bStdDataFileID, bCommMode, 5,
				strlen((char*)abDataBuffer), abDataBuffer);
		CHECK_RC();

		//  Write to the Linear Record file.
		//  If it turns out, that the file is full, clear its contents and repeat writing.
		for (i = 0; i < 2; i++)
		{
			rc =
				SPROX_Desfire_WriteRecord(1, bCommMode, 0, 25,
					(unsigned char*)
					"0123456789012345678901234");
			if (rc == 0)
			{
				snprintf((char*)abDataBuffer, sizeof(abDataBuffer), " Transaction #%d ", iTransaction);
				rc =
					SPROX_Desfire_WriteRecord(1, bCommMode, 5,
						strlen((char*)abDataBuffer),
						abDataBuffer);
			}
			//  Was the WriteRecord successful ?
			if (rc == 0)
				break;              // yes

			//  Clear the record file.
			rc = SPROX_Desfire_ClearRecordFile(1);
			CHECK_RC();

			//  It is not allowed to write to the file before a CommitTransaction.
			//  So the following call will fail !
			rc =
				SPROX_Desfire_WriteRecord(1, bCommMode, 5,
					strlen((char*)abDataBuffer),
					abDataBuffer);
			if (rc != (DFCARD_ERROR - DF_PERMISSION_DENIED))
				CHECK_RC();

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
		rc =
			SPROX_Desfire_Authenticate(1, (unsigned char*)"App.B Key #1.   ");
		CHECK_RC();

		//  Read the first half of the Standard Data file's data specifying the exact
		//  number of bytes to read.
		rc =
			SPROX_Desfire_ReadData(bStdDataFileID, bCommMode, 0, 50,
				abDataBuffer, &eNBytesRead);
		CHECK_RC();
		//  Read the second half of the data with an open read (give me all data available).
		rc =
			SPROX_Desfire_ReadData(bStdDataFileID, bCommMode, 50, 0,
				abDataBuffer, &eNBytesRead);
		CHECK_RC();

		//  Get the Value file's current balance.
		rc = SPROX_Desfire_GetValue(4, bCommMode, &lAmount);
		CHECK_RC();

		//  Get the number of records in the Linear Record file.
		rc = SPROX_Desfire_GetFileSettings(1, 0, 0, 0, &unFileSettings);
		CHECK_RC();

		//  Read the oldest record from the file.
		rc =
			SPROX_Desfire_ReadRecords(1, bCommMode,
				unFileSettings.stRecordFileSettings.
				eCurrNRecords - 1, 1,
				unFileSettings.stRecordFileSettings.
				eRecordSize, abDataBuffer, &eNRecordsRead);
		CHECK_RC();

		//  Read all records from the file.
		rc =
			SPROX_Desfire_ReadRecords(1, bCommMode, 0, 0,
				unFileSettings.stRecordFileSettings.
				eRecordSize, abDataBuffer, &eNRecordsRead);
		CHECK_RC();
	}

	//  Get the file IDs of the current application.
	rc = SPROX_Desfire_GetFileIDs(16, abDataBuffer, &bNFilesFound);
	CHECK_RC();

	//  Get information about the application's files.
	//  Delete each file after retrieving information about it.
	for (iFileIndex = 0; iFileIndex < bNFilesFound; iFileIndex++)
	{
		rc =
			SPROX_Desfire_GetFileSettings(abDataBuffer[iFileIndex], &bFileType,
				&bCommMode, &wAccessRights,
				&unFileSettings);
		CHECK_RC();
		rc = SPROX_Desfire_DeleteFile(abDataBuffer[iFileIndex]);
		CHECK_RC();
	}

	//  Verify that there are no files in the application.
	rc = SPROX_Desfire_GetFileIDs(16, abDataBuffer, &bNFilesFound);
	CHECK_RC();

	//  Delete application B.
	//  This time we can (and do) use the Application Master Key.
	rc = SPROX_Desfire_Authenticate(0, (unsigned char*)"App.B Master Key");
	CHECK_RC();
	rc = SPROX_Desfire_DeleteApplication(0xBBBBBB);
	CHECK_RC();

	//  Verify that application B has been deleted.
	rc = SPROX_Desfire_GetApplicationIDs(10, (DWORD*)abDataBuffer, &temp);
	CHECK_RC();

	//  Working with a larger file (a Cyclic Record File) using application C:
	rc = SPROX_Desfire_SelectApplication(0xCCCCCC);
	CHECK_RC();

	//  Define keys #1 and #2 using the Change Keys key (#12).
	rc = SPROX_Desfire_Authenticate(12, abNullKey);
	CHECK_RC();

	rc =
		SPROX_Desfire_ChangeKey(1, (unsigned char*)"App.C Key #1.   ",
			abNullKey);
	CHECK_RC();

	rc =
		SPROX_Desfire_ChangeKey(2, (unsigned char*)"App.C Key #2..  ",
			abNullKey);
	CHECK_RC();

	//  Create the file.
	rc = SPROX_Desfire_Authenticate(0, abNullKey);
	CHECK_RC();

	rc = SPROX_Desfire_CreateCyclicRecordFile(6, 0, 0x12E0, 100, 22);
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
		rc = SPROX_Desfire_Authenticate(0, abNullKey);
		CHECK_RC();
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
			rc =
				SPROX_Desfire_Authenticate(2, (unsigned char*)"App.C Key #2..  ");
			CHECK_RC();
		}
		else
			bCommMode = 0;

		//  Write a new record.
		memset(abDataBuffer, '_', 100);
		abDataBuffer[0] = iTransaction;
		abDataBuffer[99] = iTransaction;
		snprintf((char*)abDataBuffer + 5, sizeof(abDataBuffer) - 5," Transaction #%d ", iTransaction);
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
	//  Note also that only 22-1 records (one less than specified in CreateCyclicRecordFile)
	//  can be accessed.
	bCommMode = 1;
	rc = SPROX_Desfire_Authenticate(1, (unsigned char*)"App.C Key #1.   ");
	CHECK_RC();
	for (eOffset = 0; eOffset < 22; eOffset++)
	{
		rc =
			SPROX_Desfire_ReadRecords(6, bCommMode, eOffset, 1, 100,
				abDataBuffer, &eNRecordsRead);
		if (rc != (DFCARD_ERROR - DF_BOUNDARY_ERROR))
			CHECK_RC();
	}

	//  Read the whole record file with a single ReadRecords call.
	//rc = SPROX_Desfire_ReadRecords( 6, bCommMode, 0, 0, 100, abDataBuffer, &eNRecordsRead );
	//CHECK_RC();

	//  Change the master key settings such that master key authentication will be required
	//  for all card operations.
	//  Leave the ChangeConfiguration bit (bit 3) set to allow undoing the change.
	rc = SPROX_Desfire_SelectApplication(0);
	CHECK_RC();
	rc = SPROX_Desfire_Authenticate(0, abCardMasterKey);
	CHECK_RC();
	rc = SPROX_Desfire_ChangeKeySettings(0x8);
	CHECK_RC();

	//  Clear the authentication state and try to get application IDs.
	//  Also try to delete application C or to create a new application.
	rc = SPROX_Desfire_SelectApplication(0);
	CHECK_RC();
	//  The following three calls will report an Authentication Error (0xAE).
	rc = SPROX_Desfire_GetApplicationIDs(10, (DWORD*)abDataBuffer, &temp);
	if (rc != (DFCARD_ERROR - DF_AUTHENTICATION_ERROR))
		CHECK_RC();
	rc = SPROX_Desfire_DeleteApplication(0xCCCCCC);
	if (rc != (DFCARD_ERROR - DF_AUTHENTICATION_ERROR))
		CHECK_RC();
	rc = SPROX_Desfire_CreateApplication(0xDDDDDD, 0xEF, 0);
	if (rc != (DFCARD_ERROR - DF_AUTHENTICATION_ERROR))
		CHECK_RC();

	//  With authentication all three should work.
	rc = SPROX_Desfire_Authenticate(0, abCardMasterKey);
	CHECK_RC();
	rc = SPROX_Desfire_GetApplicationIDs(10, (DWORD*)abDataBuffer, &temp);
	CHECK_RC();
	rc = SPROX_Desfire_DeleteApplication(0xCCCCCC);
	CHECK_RC();
	rc = SPROX_Desfire_CreateApplication(0xDDDDDD, 0xEF, 0);
	CHECK_RC();

	//  Change the master key settings back to the default which is least restrictive.
	rc = SPROX_Desfire_ChangeKeySettings(0xF);
	CHECK_RC();

	//  Change the Card master key back to the 0 key.
	rc = SPROX_Desfire_Authenticate(0, abCardMasterKey);
	CHECK_RC();
	rc = SPROX_Desfire_ChangeKey(0, abNullKey, NULL);
	CHECK_RC();

	//  Delete all demonstration data from the card.
	rc = SPROX_Desfire_Authenticate(0, abNullKey);
	CHECK_RC();
	rc = SPROX_Desfire_FormatPICC();
	CHECK_RC();

	return TRUE;
}
