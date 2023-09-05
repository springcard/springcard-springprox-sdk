/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  Copyright (c) 2003-2014 PRO ACTIVE SAS, FRANCE - www.springcard.com

  ref_desfire_benchmark.c
  -----------------------

  This reference application test the overall communication speed
  with a NXP DESFire EV1 cards.

  JDA 30/10/2012 : initial release
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

static BOOL parse_args(int argc, char** argv);
static void usage(void);
static BOOL sprox_desfire_benchmark(void);

const char* PROGRAM_NAME = "ref_desfire_benchmark";
const char* szCommDevice = NULL;
BYTE dDSI = 0;
BYTE dDRI = 0;
BYTE dCID = 0xFF;
BOOL fIsoWrapping = FALSE;

int main(int argc, char** argv)
{
	SWORD rc;
	int i;
	char s_buffer[64];
	BYTE atq[2];
	BYTE sak[1];
	BYTE snr[12];
	BYTE snrlen = 12;
	BYTE ats[32];
	BYTE atslen = 32;

	printf("SpringCard SpringProx 'Legacy' SDK\n");
	printf("\n");
	printf("%s : NXP MIFARE DESFire EV0/EV1 benchmarking\n\n", PROGRAM_NAME);
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

	rc = SPROX_ReaderGetFirmwareA(s_buffer, sizeof(s_buffer));
	if (rc == MI_OK)
		printf("Reader firmware is %s\n", s_buffer);

	/* Configure reader in ISO 14443-A mode */
	/* ------------------------------------ */
	rc = SPROX_SetConfig(CFG_MODE_ISO_14443_A);
	if (rc != MI_OK)
	{
		printf("Failed to configure the reader\n");
		goto close;
	}

	/* RF field ON */
	SPROX_ControlRF(TRUE);

	/* Select any tag in the ISO 14443-A RF field */
	/* ------------------------------------------ */

	printf("Looking for a tag...\n");

	rc = SPROX_TclA_ActivateIdle(atq, snr, &snrlen, sak);
	if (rc == MI_CARD_NOT_TCL)
	{
		/* On a bien trouve un badge, mais il s'arrete au type 3 */
		printf("The selected tag is not ISO 14443-4 (T=CL)\n");
		goto halt;
	}
	if (rc == MI_NOTAGERR)
	{
		/* Pas de badge dans le champ */
		printf("No available tag in RF field\n");
		goto close;
	}
	if (rc != MI_OK)
	{
		/* Erreur */
		printf("Failed to select tag\n");
		goto close;
	}

	printf("T=CL tag found, SNR=");
	for (i = 0; i < snrlen; i++)
		printf("%02X", snr[i]);
	printf(" ATQ=%02X%02X SAK=%02X\n", atq[1], atq[0], sak[0]);

	/* Is the tag a DESFire ? */
	/* ---------------------- */
	if ((atq[1] != 0x03) || ((atq[0] != 0x44) && (atq[0] != 0x04)))
	{
		printf("This is not a DESFire tag\n");
		goto halt;
	}

	/* Open a T=CL session on the tag */
	/* ------------------------------ */
	rc = SPROX_TclA_GetAts(dCID, ats, &atslen);
	if (rc != MI_OK)
	{
		/* Erreur */
		printf("Failed to activate the tag (%d)\n", rc);
		goto halt;
	}

	printf("T=CL tag activated, ATS=");
	for (i = 0; i < atslen; i++)
		printf("%02X", ats[i]);
	printf(" (TA1=%02X TA2=%02X TA3=%02X)\n", ats[2], ats[3], ats[4]);

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

	sprox_desfire_benchmark();

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

	printf("SUCCESS!\n");

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
	printf("Usage: %s [BAUDRATES] [OPTIONS] [-d <COMM DEVICE] [-v]\n", PROGRAM_NAME);
	printf("BAUDRATES:\n");
	printf(" -dr0, -dr1, -dr2 or -dr3 for PCD->PICC baudrate\n");
	printf(" -ds0, -ds1, -ds2 or -ds3 for PICC->PCD baudrate\n");
	printf("Default is -dr0 -ds0 (106kbit/s both directions)\n");
	printf("OPTIONS:\n");
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

#ifdef WIN32
int gettimeofday(struct timeval* p, void* tz)
{
	ULARGE_INTEGER ul; // As specified on MSDN.
	FILETIME ft;

	// Returns a 64-bit value representing the number of
	// 100-nanosecond intervals since January 1, 1601 (UTC).
	GetSystemTimeAsFileTime(&ft);

	// Fill ULARGE_INTEGER low and high parts.
	ul.LowPart = ft.dwLowDateTime;
	ul.HighPart = ft.dwHighDateTime;
	// Convert to microseconds.
	ul.QuadPart /= 10UL;
	// Remove Windows to UNIX Epoch delta.
	ul.QuadPart -= 11644473600000000UL;
	// Modulo to retrieve the microseconds.
	p->tv_usec = (long)(ul.QuadPart % 1000000UL);
	// Divide to retrieve the seconds.
	p->tv_sec = (long)(ul.QuadPart / 1000000UL);

	return 0;
}
#endif

#ifdef __linux__
#include <inttypes.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif

#ifdef WIN32
#define CHECK_RC() { if (rc!=0) { printf("\nline %d - failed (%d) %s\n", __LINE__-1, rc, SPROX_Desfire_GetErrorMessage(rc)); return FALSE; } }
#endif

#ifdef __linux__
#define CHECK_RC() { if (rc!=0) { printf("\nline %d - failed (%d) %s\n", __LINE__-1, rc, SPROX_Desfire_GetErrorMessage(rc)); return FALSE; } }
#endif

const BYTE abNullKey[24] = { 0 };

const BYTE abRootKey1K[24] = { "ABCDEFGHABCDEFGHABCDEFGH" };
const BYTE abRootKey2K[24] = { "Card Master Key!Card Mas" };
const BYTE abRootKey3K[24] = { "Card Master Key!        " };

static void clock_increment_diff(struct timeval* to, const struct timeval* inc, const struct timeval* dec)
{
	to->tv_sec += inc->tv_sec;
	to->tv_sec -= dec->tv_sec;

	to->tv_usec += inc->tv_usec;
	to->tv_usec -= dec->tv_usec;

	while (to->tv_usec > 1000000)
	{
		to->tv_usec -= 1000000;
		to->tv_sec += 1;
	}
}

static unsigned long clock_to_ms(const struct timeval* c)
{
	unsigned long r;

	r = c->tv_sec * 1000;
	r += c->tv_usec / 1000;

	return r;
}

static float clock_bitrate(const struct timeval* c, unsigned long nBytes)
{
	float r, t;

	t = (float)c->tv_sec;
	t += (float)c->tv_usec / (float)1000000.0;

	r = (float)nBytes / t;
	r *= 8.0; /* bytes -> bits */
	r /= 1024.0;

	return r;
}


static BOOL sprox_desfire_benchmark(void)
{
	DF_VERSION_INFO stVersionInfo;
	WORD wDesFireSoftwareVersion;
	SWORD rc;
	BYTE bKeyVersion;
	BYTE bNApplicationsFound;
	BYTE bStdDataFileID = 1;
	DWORD dwNbBytesRead;
	BYTE abDataBuffer[2048];

	int i;
	DWORD nBytes = 0;
	struct timeval t_read;
	struct timeval t_write;
	struct timeval t0, t1;

	t_read.tv_sec = t_read.tv_usec = 0;
	t_write.tv_sec = t_write.tv_usec = 0;


	//  Get the card's version information.
	rc = SPROX_Desfire_GetVersion(&stVersionInfo);
	CHECK_RC();
	wDesFireSoftwareVersion = (unsigned int)((unsigned int)stVersionInfo.bSwMajorVersion << 8 | stVersionInfo.bSwMinorVersion);
	printf("\n");

	if (wDesFireSoftwareVersion < 0x0100)
	{
		printf("This is a Desfire EV0!\n");

	}
	else
	{
		printf("This is a Desfire EV1!\n");
	}

	//  After activating a DesFire card the currently selected 'application' is the card
	//  iTestMode (AID = 0). Therefore the next command is redundant.
	//  We use the command to check that the card is responding correctly.
	rc = SPROX_Desfire_SelectApplication(0);
	CHECK_RC();

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
	else
		if (bKeyVersion == 0xAA)
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
		else
			if (bKeyVersion == 0xC7)
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
			else
				if (bKeyVersion == 0xAE)
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

	//  Create one application
	//  It has no keys and its key settings are least restrictive.
	//  Note: since there are no keys, the Change Keys key settting must be assigned
	//        the value 0xE or 0xF.
	rc = SPROX_Desfire_CreateApplication(0xAAAAAA, 0xFF, 0);
	CHECK_RC();

	//  Create one file in application A:
	//  Since the application does not have any keys associated with it, the file access rights
	//  settings 0xE (public access granted) and 0xF (access denied) are the only permissable
	//  ones.
	rc = SPROX_Desfire_SelectApplication(0xAAAAAA);
	CHECK_RC();

	//  Create a the Standard Data file.
	rc = SPROX_Desfire_CreateStdDataFile(bStdDataFileID, 0, 0xEEEE, 2048);
	CHECK_RC();

	//  Fill the Standard Data file with some data.
	memset(abDataBuffer, 0xDA, sizeof(abDataBuffer));

	printf("Performing benchmark, please wait...\n");

	for (i = 0; i < 16; i++)
	{
		//  Write the file content at once
		gettimeofday(&t0, NULL);
		rc = SPROX_Desfire_WriteData(bStdDataFileID, DF_COMM_MODE_PLAIN, 0, 2048, abDataBuffer);
		gettimeofday(&t1, NULL);
		CHECK_RC();
		clock_increment_diff(&t_write, &t1, &t0);
		printf("."); fflush(NULL);

		//  Read the file content at once
		gettimeofday(&t0, NULL);
		rc = SPROX_Desfire_ReadData(bStdDataFileID, DF_COMM_MODE_PLAIN, 0, 2048, abDataBuffer, &dwNbBytesRead);
		gettimeofday(&t1, NULL);
		CHECK_RC();
		clock_increment_diff(&t_read, &t1, &t0);
		printf("."); fflush(NULL);

		nBytes += 2048;
	}

	printf("\nCleanup...\n");

	//  Back to the root application
	rc = SPROX_Desfire_SelectApplication(0);
	CHECK_RC();

	//  Get authenticated
	rc = SPROX_Desfire_Authenticate(0, abNullKey);
	CHECK_RC();

	//  Delete all demonstration data from the card.
	rc = SPROX_Desfire_FormatPICC();
	CHECK_RC();

	printf("Read : %dB received in %ldms (%fkbit/s)\n", nBytes, clock_to_ms(&t_read), clock_bitrate(&t_read, nBytes));
	printf("Write: %dB sent in %ldms (%fkbit/s)\n", nBytes, clock_to_ms(&t_write), clock_bitrate(&t_write, nBytes));

#ifdef __linux__
	{
		struct rusage usage;
		getrusage(RUSAGE_SELF, &usage);
		printf("user CPU time:                %ld.%06ld\n", usage.ru_utime.tv_sec, usage.ru_utime.tv_usec);
		printf("system CPU time:              %ld.%06ld\n", usage.ru_stime.tv_sec, usage.ru_stime.tv_usec);
		printf("voluntary context switches:   %ld\n", usage.ru_nvcsw);
		printf("involuntary context switches: %ld\n", usage.ru_nivcsw);
	}
#endif

	printf("\n");
	return TRUE;
}

