/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  Copyright (c) 2005-2009 PRO ACTIVE SAS - www.proactive-group.com

  Ref_Calypso.c
  -------------

  This is the reference applications that shows how to retrieve Calypso
  card's identifier.

  It performs also ISO 14443-A and ISO 14443-B polling, so that we can
  study the side-effects of switching between different configs.
  Clearly we see that going to ISO 14443-A resets the Calypso card.

  JDA 14/11/2005 : initial release
  JDA 04/09/2023 : refreshed the project to build with Visual Studio 2022
*/
#include "products/springprox/api/springprox.h"
#include "cardware/calypso/calypso_api.h"

#ifdef WIN32
#include <conio.h>              /* For kbhit and getch */
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* PROGRAM_NAME = "ref_calypso";
const char* szCommDevice = NULL;
const char* szXmlOutputFileName = NULL;
const char* szApplicationID = NULL;
BOOL fStartTransaction = FALSE;
BOOL fSilent = FALSE;
WORD wFindProtos = 0;

static BOOL parse_args(int argc, char** argv);
static void usage(void);

P_CALYPSO_CTX pCalypsoCtx = NULL;
char* szXmlOutputStream = NULL;
#define XML_OUTPUT_SIZE 64*1024   /* 64 ko, must be enough ! */

int main(int argc, char** argv)
{
	int i;
	char s_buffer[64];

	BYTE uid[12];  /* Serial number of the found card */
	BYTE uidlen;

	BYTE info[64]; /* Protocol-related information of the found card (mandatory for Innovatron protocol) */
	BYTE infolen;

	WORD proto;
	DWORD rc = MI_OK;

	printf("SpringCard SpringProx 'Legacy' SDK\n");
	printf("\n");
	printf("%s : Calypso demo software (XML dump of '1TIC.ICA' card application)\n\n", PROGRAM_NAME);
	printf("This is a free and unsupported sample provided by www.springcard.com\n");
	printf("Please read LICENSE.txt for details\n");

	if (!parse_args(argc, argv))
	{
		usage();
		return EXIT_FAILURE;
	}

	if (wFindProtos == 0)
		wFindProtos = PROTO_14443_A | PROTO_14443_B | PROTO_INNOVATRON;

	/* Allocate Calypso context and parser output */
	pCalypsoCtx = CalypsoCreateContext();
	szXmlOutputStream = malloc(XML_OUTPUT_SIZE);
	if ((pCalypsoCtx == NULL) || (szXmlOutputStream == NULL))
	{
		printf("Out of memory\n");
		goto done;
	}

	/* Check that the library is working */
	rc = SPROX_GetLibrary(s_buffer, sizeof(s_buffer));
	if (rc != MI_OK)
	{
		printf("Failed to get SpringProx API version\n");
		goto done;
	}
	printf("SpringProx API is '%s'\n", s_buffer);

	/* Open the SpringProx reader */
	rc = SPROX_ReaderOpen(szCommDevice);
	if (rc != MI_OK)
	{
		printf("Reader not found\n");
		goto done;
	}

	rc = SPROX_ReaderGetDevice(s_buffer, sizeof(s_buffer));
	if (rc == MI_OK)
		printf("SpringProx reader found on '%s'\n", s_buffer);

	rc = SPROX_ReaderGetFirmware(s_buffer, sizeof(s_buffer));
	if (rc == MI_OK)
		printf("SpringProx reader is '%s'\n", s_buffer);

	printf("\n");

	/* Wait until a smartcard arrives */
#ifdef WIN32
	printf("Waiting for a contactless smartcard... (Press any key to exit)\n\n");
	while (!_kbhit())
#else
	printf("Waiting for a contactless smartcard... (Press <Ctrl>+C to exit)\n\n");
	for (;;)
#endif
	{
		proto = 0;
		uidlen = sizeof(uid);
		infolen = sizeof(info);
		rc = SPROX_FindEx(wFindProtos, &proto, uid, &uidlen, info, &infolen);
		if (rc != MI_NOTAGERR)
			break;
}

	if (rc == MI_NOTAGERR)
	{
		printf("Cancelled...\n");
		goto close;
	}

	if (rc != MI_OK)
	{
		printf("Failed to find a card\n");
		goto close;
	}

	printf("Card found, proto=%04X, UID=", proto);
	for (i = 0; i < uidlen; i++)
		printf("%02X", uid[i]);
	printf("\n");

	/* Try to bind our CALYPSO library to the card */
	rc = CalypsoCardBindLegacyEx(pCalypsoCtx, proto, info, infolen);
	if (rc)
	{
		printf("CalypsoCardBindLegacy failed, error %04X.\n", rc);
		goto close;
	}

	/* Check this card is actually CALYPSO (ATR parsing or SELECT APPLICATION + FCI parsing) */
	if (szApplicationID == NULL)
	{
		rc = CalypsoCardActivate(pCalypsoCtx, NULL, 0);
	}
	else
	{
		rc = CalypsoCardActivate(pCalypsoCtx, szApplicationID, strlen(szApplicationID));
	}
	if (rc)
	{
		if (rc == CALYPSO_CARD_NOT_SUPPORTED)
		{
			printf("This is not a CALYPSO card !\n");
		}
		else
		{
			printf("CalypsoCardActivate failed, error %04X.\n", rc);
		}
		goto close;
	}

	if (fStartTransaction)
	{
		/* Do a dummy transaction test */
		BYTE chal[8];
		BYTE resp[128];
		CALYPSO_SZ size = sizeof(resp);
		BYTE rev;

		rc = CalypsoCardGetChallenge(pCalypsoCtx, chal);
		if (rc)
		{
			printf("CalypsoCardGetChallenge failed, error %04X.\n", rc);
			goto close;
		}

		rev = (BYTE)CalypsoCardRevision(pCalypsoCtx);

		switch (rev)
		{
		case 1: rc = CalypsoCardOpenSecureSession1(pCalypsoCtx, resp, &size, CALYPSO_KEY_DEBIT, CALYPSO_SFI_ENVIRONMENT, 1, chal, chal, NULL, NULL, NULL);
			break;

		case 2: rc = CalypsoCardOpenSecureSession2(pCalypsoCtx, resp, &size, CALYPSO_KEY_DEBIT, CALYPSO_SFI_ENVIRONMENT, 1, chal, chal, NULL, NULL, NULL, NULL);
			break;

		case 3:
		default: rc = CalypsoCardOpenSecureSession3(pCalypsoCtx, resp, &size, CALYPSO_KEY_DEBIT, CALYPSO_SFI_ENVIRONMENT, 1, chal, chal, NULL, NULL, NULL, NULL, NULL);
			break;
		}

		if (rc)
		{
			printf("CalypsoCardOpenSecureSession failed, error %04X.\n", rc);
			goto close;
		}

		rc = CalypsoCardCloseSecureSession(pCalypsoCtx, FALSE, NULL, NULL, NULL);
		if (rc)
		{
			printf("CalypsoCardCloseSecureSession failed, error %04X.\n", rc);
			goto close;
		}

		printf("CALYPSO pseudo-transaction OK...\n");
	}

	/* OK, let's explore the card */
	/* -------------------------- */
	printf("\nExploring the CALYPSO card...\n");

	/* Configure the parser output */
	CalypsoSetXmlOutputStr(pCalypsoCtx, szXmlOutputStream, XML_OUTPUT_SIZE);

	/* Initialise the timer */
	CalypsoBench(TRUE);

	/* Actual exploration */
	rc = CalypsoExploreAndParse(pCalypsoCtx);
	if (rc)
	{
		printf("CalypsoExploreAndParse failed, error %04X.\n", rc);
		goto close;
	}

	/* Done ! */
	printf("CALYPSO card explored in %ldms\n", CalypsoBench(TRUE));

	/* Stop the parser */
	CalypsoClearOutput(pCalypsoCtx);

	/* Drop card connection */
	CalypsoCardDispose(pCalypsoCtx);

	/* Now display card's data */
	/* ----------------------- */

	if (szXmlOutputFileName != NULL)
	{
		FILE* fp;

		errno_t err = _tfopen_s(&fp, szXmlOutputFileName, _T("wt"));
		if (err)
		{
			printf("Failed to open output file '%s'\n", szXmlOutputFileName);
			printf("Err. %d\n", err);
			goto done;
		}

		fprintf(fp, "<calypso_explorer>\n");

		fprintf(fp, szXmlOutputStream);

		fprintf(fp, "</calypso_explorer>\n");

		fprintf(fp, "\n");
		fclose(fp);

		printf("Output saved to '%s'\n", szXmlOutputFileName);
	}
	else if (!fSilent)
	{
		printf(szXmlOutputStream);
		printf("\n");
	}

	/* Terminated, OK */
	printf("SUCCESS!\n");

close:
	/* Close the reader */
	SPROX_ControlRF(FALSE);
	SPROX_ReaderClose();

done:
	/* Display last error */
	if (rc != MI_OK)
		printf("%s (%d)\n", SPROX_GetErrorMessage((SWORD)rc), (SWORD)rc);

	if (szXmlOutputStream != NULL)
	{
		free(szXmlOutputStream);
		szXmlOutputStream = NULL;
	}

	if (pCalypsoCtx != NULL)
	{
		CalypsoDestroyContext(pCalypsoCtx);
		pCalypsoCtx = NULL;
	}

	return 0;
}

static void usage(void)
{
	printf("Usage: %s [-x <XML FILENAME>] [-s] [-t] [-d <COMM DEVICE] [-v]\n\n", PROGRAM_NAME);
	printf("If -a <AID> is set, the software uses this Application IDentifier instead of '1TIC.ICA'.\n\n");
	printf("If -x <XML FILENAME> is set, the software writes the output into the specified file.\n\n");
	printf("If -s (silent) is set, the software does not display the XML output.\n\n");
	printf("If -t (transaction) is set, the software starts a transaction with the card.\n\n");
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
			if (!strcmp(argv[i], "-x") && i + 1 < argc)
			{
				szXmlOutputFileName = argv[i + 1];
				i++;  // Skip next item since we just processed it
			}
			if (!strcmp(argv[i], "-a") && i + 1 < argc)
			{
				szApplicationID = argv[i + 1];
				i++;  // Skip next item since we just processed it
			}
			else if (!strcmp(argv[i], "-v"))
			{
				// Ask the libraries to be verbose
				SPROX_SetVerbose(255, NULL);
				CalypsoSetTraceLevel(255);
				CalypsoSetTraceFile(NULL);
			}
			else if (!strcmp(argv[i], "-s"))
			{
				fSilent = TRUE;
			}
			else if (!strcmp(argv[i], "-t"))
			{
				fStartTransaction = TRUE;
			}
			else if (!strcmp(argv[i], "-A"))
			{
				wFindProtos |= PROTO_14443_A;
			}
			else if (!strcmp(argv[i], "-B"))
			{
				wFindProtos |= PROTO_14443_B;
			}
			else if (!strcmp(argv[i], "-I"))
			{
				wFindProtos |= PROTO_INNOVATRON;
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

