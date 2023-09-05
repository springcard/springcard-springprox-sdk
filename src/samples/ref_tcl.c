/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  Copyright (c) 2000-2013 PRO ACTIVE SAS, FRANCE - www.springcard.com

  ref_tcl.c
  ---------

  This is the reference applications that shows how to exchange APDUs
  with cards compliant with ISO 14443 layer 4.

  The test uses either Desfire "GET VERSION" APDU or ISO 7816-4
  "SELECT MASTER FILE" APDU.

  JDA 04/02/2013 : initial release
  JDA 14/02/2013 : corrected value of cid_supported returned by Test_TCL_A_Ex
  JDA 04/09/2023 : refreshed the project to build with Visual Studio 2022
*/
#include "products/springprox/api/springprox.h"

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const char* PROGRAM_NAME = "ref_tcl";
const char* szCommDevice = NULL;
WORD wFindProtos = 0;
BYTE dDSI = 0;
BYTE dDRI = 0;
int dTestCount = 5;

static BOOL parse_args(int argc, char** argv);
static void usage(void);
static SWORD Test_TCL_A(const BYTE uid[], BYTE uidlen);
static SWORD Test_TCL_B(const BYTE atqb[11]);

int main(int argc, char** argv)
{
	SWORD rc;
	int i;
	char s_buffer[64];

	BYTE uid_or_pupi[32];
	BYTE uid_or_pupi_len = sizeof(uid_or_pupi);
	BYTE info[32];
	BYTE info_len = sizeof(info);

	WORD got_proto;
	BOOL verbose = FALSE;

	/* Display the informations and check the command line */
	/* --------------------------------------------------- */

	printf("SpringCard SpringProx 'Legacy' SDK\n");
	printf("\n");
	printf("%s : ISO 14443-4 'T=CL' reference\n", PROGRAM_NAME);
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

	rc = SPROX_FindEx(wFindProtos, &got_proto, uid_or_pupi, &uid_or_pupi_len, info, &info_len);
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

	if (got_proto == PROTO_14443_A)
	{
		/* Test a ISO 14443 type A card */
		/* ---------------------------- */

		printf("Found a ISO 14443-A card, UID=");
		for (i = 0; i < uid_or_pupi_len; i++)
			printf("%02X", uid_or_pupi[i]);
		printf("\n");

		rc = Test_TCL_A(uid_or_pupi, uid_or_pupi_len);
		if (rc != MI_OK)
			goto close;
	}
	else if (got_proto == PROTO_14443_B)
	{
		/* Test a ISO 14443 type A card */
		/* ---------------------------- */

		printf("Found a ISO 14443-B card, PUPI=");
		for (i = 0; i < uid_or_pupi_len; i++)
			printf("%02X", uid_or_pupi[i]);
		printf("\n");

		printf("                          ATQB=");
		for (i = 0; i < info_len; i++)
			printf("%02X", info[i]);
		printf("\n");

		rc = Test_TCL_B(info);
		if (rc != MI_OK)
			goto close;

	}
	else
	{
		printf("Invalid protocol\n");
		goto close;
	}

	printf("\n");
	printf("SUCCESS!\n");
	printf("All tests have been successfully passed\n");

close:

	/* Close the reader */
	SPROX_ControlRF(FALSE);
	SPROX_ReaderClose();

done:
	/* Display last error */
	if (rc == MI_OK)
	{
		printf("Done\n");
	}
	else
	{
		printf("%s (%d)\n", SPROX_GetErrorMessage(rc), rc);
	}

	return 0;
}

void usage(void)
{
	printf("Usage: %s [PROTOCOLS] [BAUDRATES] [OPTIONS] [-d <COMM DEVICE]\n", PROGRAM_NAME);
	printf("Write PROTOCOLS as follow:\n");
	printf(" -A : for ISO 14443-A\n");
	printf(" -B : for ISO 14443-B\n");
	printf(" -I : for Innovatron (old Calypso cards)\n");
	printf(" -* : for all\n");
	printf("If PROTOCOLS is empty, all protocols are tried (same as -*)\n");
	printf("BAUDRATES:\n");
	printf(" -dr0, -dr1, -dr2 or -dr3 for PCD->PICC baudrate\n");
	printf(" -ds0, -ds1, -ds2 or -ds3 for PICC->PCD baudrate\n");
	printf("Default is -dr0 -ds0 (106kbit/s both directions)\n");
	printf("OPTIONS:\n");
	printf(" -v : verbose (trace library functions)\n");
	printf("If the name of COMM DEVICE is not specified, default is taken from Registry or from /etc/springprox.cfg\n");
}

BOOL parse_args(int argc, char** argv)
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
				wFindProtos = PROTO_14443_A | PROTO_14443_B | PROTO_INNOVATRON;
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
	}

	if (wFindProtos == 0)
		wFindProtos = PROTO_14443_A | PROTO_14443_B | PROTO_INNOVATRON;

	return TRUE;
}

SWORD Test_TCL_A_Ex(const BYTE uid[], BYTE uidlen, BYTE cid, BYTE dsi, BYTE dri, BOOL skip_pps, BOOL* cid_supported)
{
	static const BYTE send_buffer[] = { 0x90, 0x60, 0x00, 0x00, 0x00 };
	BYTE recv_buffer[256];
	WORD recv_length;
	BYTE ats[32];
	BYTE atslen = sizeof(ats);
	BYTE ta1 = 0x00;
	BYTE tc1 = 0x00;
	int i;
	SWORD rc;

	rc = SPROX_TclA_GetAts(cid, ats, &atslen);
	if (rc != MI_OK)
	{
		printf("Get ATS failed!\n");
		return rc;
	}

	/* Understand the ATS */
	if (ats[0] & 0x10)
	{
		ta1 = ats[1];
	}
	if (ats[0] & 0x40)
	{
		if ((ats[0] & 0x10) && (ats[0] & 0x20))
		{
			tc1 = ats[3];
		}
		else if ((ats[0] & 0x10) || (ats[0] & 0x20))
		{
			tc1 = ats[2];
		}
		else
		{
			tc1 = ats[1];
		}
	}

	if (cid_supported != NULL)
		*cid_supported = TRUE;

	if (!(tc1 & 0x02))
	{
		/* The card doesn't support the CID feature */
		if (cid_supported != NULL)
			*cid_supported = FALSE;
	}

	if ((dDSI == 1) && !(ta1 & 0x01))
	{
		printf("Bad option: the card doesn't support DSI=1\n");
		return MI_LIB_CALL_ERROR;
	}
	if ((dDSI == 2) && !(ta1 & 0x02))
	{
		printf("Bad option: the card doesn't support DSI=2\n");
		return MI_LIB_CALL_ERROR;
	}
	if ((dDSI == 3) && !(ta1 & 0x04))
	{
		printf("Bad option: the card doesn't support DSI=3\n");
		return MI_LIB_CALL_ERROR;
	}
	if ((dDRI == 1) && !(ta1 & 0x10))
	{
		printf("Bad option: the card doesn't support DRI=1\n");
		return MI_LIB_CALL_ERROR;
	}
	if ((dDRI == 2) && !(ta1 & 0x20))
	{
		printf("Bad option: the card doesn't support DRI=2\n");
		return MI_LIB_CALL_ERROR;
	}
	if ((dDRI == 3) && !(ta1 & 0x40))
	{
		printf("Bad option: the card doesn't support DRI=3\n");
		return MI_LIB_CALL_ERROR;
	}
	if ((dDSI != dDRI) && (ta1 & 0x80))
	{
		printf("Bad option: the card wants DSI=DRI\n");
		return MI_LIB_CALL_ERROR;
	}

	if (!skip_pps)
	{
		rc = SPROX_TclA_Pps(cid, dsi, dri);
		if (rc != MI_OK)
		{
			printf("PPS failed!\n");
			return rc;
		}
	}

	for (i = 0; i < dTestCount; i++)
	{
		recv_length = sizeof(recv_buffer);
		rc = SPROX_Tcl_Exchange(cid, send_buffer, sizeof(send_buffer), recv_buffer, &recv_length);
		if (rc != MI_OK)
		{
			printf("Exchange APDU %d/%d failed!\n", i, dTestCount);
			return rc;
		}
	}

	rc = SPROX_Tcl_Deselect(cid);
	if (rc != MI_OK)
	{
		printf("DESELECT failed!\n");
		return rc;
	}

	rc = SPROX_A_SelectAgain(uid, uidlen);
	if (rc != MI_OK)
	{
		printf("SelectAgain (WUPA+select) failed!\n");
		return rc;
	}

	return MI_OK;
}

SWORD Test_TCL_A(const BYTE uid[], BYTE uidlen)
{
	SWORD rc;
	BOOL cid_supported;

	if ((dDSI == 0) && (dDRI == 0))
	{
		/* The PPS is not mandatory */
		/* ------------------------ */

		printf("Test T=CL A, no CID, no PPS\n");
		rc = Test_TCL_A_Ex(uid, uidlen, 0xFF, 0, 0, TRUE, &cid_supported);
		if (rc != MI_OK) return rc;

		printf("Test T=CL A, CID=0, no PPS\n");
		rc = Test_TCL_A_Ex(uid, uidlen, 0, 0, 0, TRUE, NULL);
		if (rc != MI_OK) return rc;

		if (cid_supported)
		{
			printf("Test T=CL A, CID=1, no PPS\n");
			rc = Test_TCL_A_Ex(uid, uidlen, 1, 0, 0, TRUE, NULL);
			if (rc != MI_OK) return rc;

			printf("Test T=CL A, CID=13, no PPS\n");
			rc = Test_TCL_A_Ex(uid, uidlen, 13, 0, 0, TRUE, NULL);
			if (rc != MI_OK) return rc;

			printf("Test T=CL A, CID=14, no PPS\n");
			rc = Test_TCL_A_Ex(uid, uidlen, 14, 0, 0, TRUE, NULL);
			if (rc != MI_OK) return rc;
		}
	}

	/* We must use the PPS */
	/* ------------------- */

	printf("Test T=CL A, no CID, PPS dsi=%d, dri=%d\n", dDSI, dDRI);
	rc = Test_TCL_A_Ex(uid, uidlen, 0xFF, dDSI, dDRI, FALSE, &cid_supported);
	if (rc != MI_OK) return rc;

	printf("Test T=CL A, CID=0, PPS dsi=%d, dri=%d\n", dDSI, dDRI);
	rc = Test_TCL_A_Ex(uid, uidlen, 0, dDSI, dDRI, FALSE, NULL);
	if (rc != MI_OK) return rc;

	if (cid_supported)
	{
		printf("Test T=CL A, CID=1, PPS dsi=%d, dri=%d\n", dDSI, dDRI);
		rc = Test_TCL_A_Ex(uid, uidlen, 1, dDSI, dDRI, FALSE, NULL);
		if (rc != MI_OK) return rc;

		printf("Test T=CL A, CID=13, PPS dsi=%d, dri=%d\n", dDSI, dDRI);
		rc = Test_TCL_A_Ex(uid, uidlen, 13, dDSI, dDRI, FALSE, NULL);
		if (rc != MI_OK) return rc;

		printf("Test T=CL A, CID=14, PPS dsi=%d, dri=%d\n", dDSI, dDRI);
		rc = Test_TCL_A_Ex(uid, uidlen, 14, dDSI, dDRI, FALSE, NULL);
		if (rc != MI_OK) return rc;
	}

	return MI_OK;
}

SWORD Test_TCL_B_Ex(const BYTE pupi[4], BYTE cid, BYTE dsi, BYTE dri)
{
	static const BYTE send_buffer[] = { 0x00, 0xA4, 0x00, 0x00, 0x02, 0x3F, 0x00, 0x00 };
	BYTE recv_buffer[256];
	WORD recv_length;
	int i;
	SWORD rc;

	rc = SPROX_TclB_AttribEx(pupi, cid, dsi, dri);
	if (rc != MI_OK)
	{
		printf("ATTRIB failed!\n");
		return rc;
	}

	for (i = 0; i < dTestCount; i++)
	{
		recv_length = sizeof(recv_buffer);
		rc = SPROX_Tcl_Exchange(cid, send_buffer, sizeof(send_buffer), recv_buffer, &recv_length);
		if (rc != MI_OK)
		{
			printf("Exchange APDU %d/%d failed!\n", i, dTestCount);
			return rc;
		}
	}

	rc = SPROX_Tcl_Deselect(cid);
	if (rc != MI_OK)
	{
		printf("DESELECT failed!\n");
		return rc;
	}

	rc = SPROX_B_SelectAny(0, NULL);
	if (rc != MI_OK)
	{
		printf("SelectAny (WUPB) failed!\n");
		return rc;
	}

	return MI_OK;
}

SWORD Test_TCL_B(const BYTE atqb[11])
{
	SWORD rc;

	if ((dDSI == 1) && !(atqb[8] & 0x01))
	{
		printf("Bad option: the card doesn't support DSI=1\n");
		return MI_LIB_CALL_ERROR;
	}
	if ((dDSI == 2) && !(atqb[8] & 0x02))
	{
		printf("Bad option: the card doesn't support DSI=2\n");
		return MI_LIB_CALL_ERROR;
	}
	if ((dDSI == 3) && !(atqb[8] & 0x04))
	{
		printf("Bad option: the card doesn't support DSI=3\n");
		return MI_LIB_CALL_ERROR;
	}
	if ((dDRI == 1) && !(atqb[8] & 0x10))
	{
		printf("Bad option: the card doesn't support DRI=1\n");
		return MI_LIB_CALL_ERROR;
	}
	if ((dDRI == 2) && !(atqb[8] & 0x20))
	{
		printf("Bad option: the card doesn't support DRI=2\n");
		return MI_LIB_CALL_ERROR;
	}
	if ((dDRI == 3) && !(atqb[8] & 0x40))
	{
		printf("Bad option: the card doesn't support DRI=3\n");
		return MI_LIB_CALL_ERROR;
	}
	if ((dDSI != dDRI) && (atqb[8] & 0x80))
	{
		printf("Bad option: the card wants DSI=DRI\n");
		return MI_LIB_CALL_ERROR;
	}

	printf("Test T=CL B, no CID, dsi=%d, dri=%d\n", dDSI, dDRI);
	rc = Test_TCL_B_Ex(atqb, 0xFF, dDSI, dDRI);
	if (rc != MI_OK) return rc;

	printf("Test T=CL B, CID=0, dsi=%d, dri=%d\n", dDSI, dDRI);
	rc = Test_TCL_B_Ex(atqb, 0, dDSI, dDRI);
	if (rc != MI_OK) return rc;

	if (atqb[10] & 0x01)
	{
		/* Only possible if the card supports the CID */
		printf("Test T=CL B, CID=1, dsi=%d, dri=%d\n", dDSI, dDRI);
		rc = Test_TCL_B_Ex(atqb, 1, dDSI, dDRI);
		if (rc != MI_OK) return rc;

		printf("Test T=CL B, CID=13, dsi=%d, dri=%d\n", dDSI, dDRI);
		rc = Test_TCL_B_Ex(atqb, 13, dDSI, dDRI);
		if (rc != MI_OK) return rc;

		printf("Test T=CL B, CID=14, dsi=%d, dri=%d\n", dDSI, dDRI);
		rc = Test_TCL_B_Ex(atqb, 14, dDSI, dDRI);
		if (rc != MI_OK) return rc;
	}

	return MI_OK;
}


