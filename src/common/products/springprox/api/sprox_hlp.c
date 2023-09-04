/*

  SpringProx API
  --------------

  Copyright (c) 2000-2008 SpringCard SAS, FRANCE - www.springcard.com

  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  History
  -------

  JDA 15/02/2003 : string list created from philips rc500/mv700 files
  JDA 16/05/2003 : added some strings
				   added helpers functions for Delphi and VB users
  LTC 26/02/2008 : added some strings for ISO 15693 and ICODE1

*/

/**** SpringProx.API/HostUtilities
 *
 * NAME
 *   SpringProx.API :: Host utilities
 *
 **/
#include "sprox_api_i.h"

#ifndef SPROX_API_NO_MSG

 /**f* SpringProx.API/SPROX_GetErrorMessage
  *
  * NAME
  *   SPROX_GetErrorMessage
  *
  * DESCRIPTION
  *   Retrieve an english string explaining a function returned code
  *
  * INPUTS
  *   SWORD status  : SpringProxAPI function returned value
  *
  * RETURNS
  *   const TCHAR * : pointer to the translation string
  *
  * WARNING
  *   The string returned is physically static in the SpringProxAPI address
  *   space. Do not try to change it nor to free the returned pointer.
  *
  * VARIANTS
  *   SPROX_GetErrorMessageA : ASCII version
  *   SPROX_GetErrorMessageW : UNICODE version
  *
  **/
SPROX_API_FUNC_T(const TCHAR*, GetErrorMessage) (SWORD status)
{
	static TCHAR msg[256];
	switch (status)
	{
		case MI_OK:
			return _T("Success");
		case MI_NOTAGERR:
			return _T("No answer (no card / card is mute)");
		case MI_CRCERR:
			return _T("Invalid CRC in card's response");
		case MI_EMPTY:
			return _T("No frame received (NFC mode)");
		case MI_AUTHERR:
			return _T("Card: Authentication failed or access denied");
		case MI_PARITYERR:
			return _T("Invalid parity bit(s) in card's response");
		case MI_CODEERR:
			return _T("NACK or status indicating error");
		case MI_SERNRERR:
			return _T("Wrong LRC in card's serial number");
		case MI_LOCKED:
			return _T("Card or block locked");
		case MI_NOTAUTHERR:
			return _T("Card: Authentication must be performed first");
		case MI_BITCOUNTERR:
			return _T("Wrong number of bits in card's answer");
		case MI_BYTECOUNTERR:
			return _T("Wrong number of bytes in card's answer");
		case MI_VALUEERR:
			return _T("Card: Counter is invalid");
		case MI_TRANSERR:
			return _T("Card: Transaction error");
		case MI_WRITEERR:
			return _T("Card: Write failed");
		case MI_INCRERR:
			return _T("Card: Counter increase failed");
		case MI_DECRERR:
			return _T("Card: Counter decrease failed");
		case MI_READERR:
			return _T("Card: Read failed");
		case MI_OVFLERR:
			return _T("RC: FIFO overflow");
		case MI_POLLING:
			return _T("Polling mode pending");
		case MI_FRAMINGERR:
			return _T("Invalid framing in card's response");
		case MI_ACCESSERR:
			return _T("Card: Access error (bad address or denied)");
		case MI_UNKNOWN_COMMAND:
			return _T("RC: Unknown command");
		case MI_COLLERR:
			return _T("A collision has occurred");
		case MI_COMMAND_FAILED:
			return _T("Command execution failed");
		case MI_INTERFACEERR:
			return _T("Hardware error");
		case MI_ACCESSTIMEOUT:
			return _T("RC: timeout");
		case MI_NOBITWISEANTICOLL:
			return _T("More than one card found, but at least one does not support anticollision");
		case MI_EXTERNAL_FIELD:
			return _T("An external RF field has been detected");
		case MI_QUIT:
			return _T("Polling terminated (timeout or break)");
		case MI_CODINGERR:
			return _T("Bogus status in card's response");
		case MI_CUSTERR:
			return _T("Card: Vendor specific error");
		case MI_CMDSUPERR:
			return _T("Card: Command not supported");
		case MI_CMDFMTERR:
			return _T("Card: Format of command invalid");
		case MI_CMDOPTERR:
			return _T("Card: Option(s) of command invalid");
		case MI_OTHERERR:
			return _T("Card: other error");
		case MI_WRONG_MODE:
			return _T("Command not available in this mode");
		case MI_WRONG_PARAMETER:
			return _T("Wrong parameter for the command");
		case MI_CID_NOT_ACTIVE:
			return _T("No active card with this CID");
		case MI_BAD_ATS_LENGTH:
			return _T("Length error in card's ATS");
		case MI_ATTRIB_ERROR:
			return _T("Error in card's response to ATTRIB");
		case MI_BAD_ATS_FORMAT:
			return _T("Format error in card's ATS");
		case MI_TCL_PROTOCOL:
			return _T("Fatal protocol error in card's response");
		case MI_TCL_TOO_MANY_RETRIES:
			return _T("Too many errors while communicating with the card");
		case MI_BAD_PPS_FORMAT:
			return _T("Format error in card's PPS response");
		case MI_PPS_ERROR:
			return _T("Other error in card's PPS response");
		case MI_CID_NOT_SUPPORTED:
			return _T("The card doesn't support CID");
		case MI_CID_ALREADY_ACTIVE:
			return _T("A card is already active with this CID");
		case MI_UNKNOWN_FUNCTION:
			return _T("Command not supported by the coupler");
		case MI_INTERNAL_ERROR:
			return _T("Internal error in the coupler");
		case MI_BUFFER_OVERFLOW:
			return _T("Internal buffer overflow");
		case MI_WRONG_LENGTH:
			return _T("Wrong data length for the command");
		case MI_TIME_EXTENSION:
			return _T("More time needed to process the command");
		case MI_CARD_NOT_TCL:
			return _T("Library: The found card doesn't support ISO 14443-4");
		case MI_RESPONSE_OVERFLOW:
			return _T("Library: Coupler's response is longer than application's buffer");
		case MI_RESPONSE_INVALID:
			return _T("Library: Coupler's response is not formated as required");
		case MI_COMMAND_OVERFLOW:
			return _T("Library: Coupler's command is longer than allowed");
		case MI_FUNCTION_NOT_AVAILABLE:
			return _T("Library: The coupler doesn't support this function");
		case MI_SER_LENGTH_ERR:
			return _T("Library: Wrong length in Coupler's response");
		case MI_SER_CHECKSUM_ERR:
			return _T("Library: Wrong checksum in Coupler's response");
		case MI_SER_PROTO_ERR:
			return _T("Library: Protocol error in Coupler's response");
		case MI_SER_PROTO_NAK:
			return _T("Library: The coupler has sent a NACK");
		case MI_SER_ACCESS_ERR:
			return _T("Library: Access to the communication device failed");
		case MI_SER_TIMEOUT_ERR:
			return _T("Library: Coupler communication timeout");
		case MI_SER_NORESP_ERR:
			return _T("Library: No response from coupler");
		case MI_LIB_CALL_ERROR:
			return _T("Library: Invalid function call");
		case MI_OUT_OF_MEMORY_ERROR:
			return _T("Library: Memory allocation failed");
		case MI_LIB_INTERNAL_ERROR:
			return _T("Library: An internal error has occured");
		case MI_INVALID_READER_CONTEXT:
			return _T("Library: the sprox_ctx parameter is invalid");
	}

	_stprintf_s(msg, sizeof(msg)/sizeof(TCHAR), _T("Error %d"), status);
	return msg;
}

SPROX_API_FUNC_T(const wchar_t*, GetErrorMessageW) (SWORD status)
{
#ifdef UNICODE
	return SPROX_API_CALL(GetErrorMessage) (status);
#else
	static wchar_t buffer[256 + 1];
	const char* p;
	int i;
	p = SPROX_API_CALL(GetErrorMessage) (status);
	for (i = 0; i < 256; i++)
	{
		buffer[i] = p[i];
		if (buffer[i] == '\0') break;
	}
	buffer[256] = '\0';
	return buffer;
#endif  
}

SPROX_API_FUNC_T(const char*, GetErrorMessageA) (SWORD status)
{
#ifndef UNICODE
	return SPROX_API_CALL(GetErrorMessage) (status);
#else
	static char buffer[256 + 1];
	const wchar_t* p;
	int i;
	p = SPROX_API_CALL(GetErrorMessage) (status);
	for (i = 0; i < 256; i++)
	{
		buffer[i] = (char)p[i];
		if (buffer[i] == '\0') break;
	}
	buffer[256] = '\0';
	return buffer;
#endif  
}

/**f* SpringProx.API/SPROX_ArrayToString
 *
 * NAME
 *   SPROX_ArrayToString
 *
 * DESCRIPTION
 *   Translate an array of bytes into a string of hexadecimal digits
 *
 * INPUTS
 *   TCHAR *string      : buffer to hold the hexadecimal string
 *   const BYTE *buffer : byte array
 *   WORD size          : size of byte array
 *
 * RETURNS
 *   MI_OK              : success
 *
 * NOTES
 *   You must ensure that size of string buffer is enough to store
 *   the whole hexadecimal string (i.e. at least 2 * size characters)
 *
 * VARIANTS
 *   SPROX_ArrayToStringA : ASCII version
 *   SPROX_ArrayToStringW : UNICODE version
 *
 **/
SPROX_API_FUNC(ArrayToString) (TCHAR* string, const BYTE* buffer, WORD size)
{
#ifdef UNICODE
	return SPROX_API_CALL(ArrayToStringW) (string, buffer, size);
#else
	return SPROX_API_CALL(ArrayToStringA) (string, buffer, size);
#endif
}

#ifdef WIN32
SPROX_API_FUNC(ArrayToStringW) (wchar_t* string, const BYTE* buffer, WORD size)
{
	WORD    i;
	wchar_t tmp[3];
	if (string == NULL) return MI_LIB_CALL_ERROR;
	if (buffer == NULL) return MI_LIB_CALL_ERROR;
	string[0] = '\0';
	for (i = 0; i < size; i++)
	{
		swprintf(tmp, 3, L"%02X", buffer[i]);
		wcscat_s(string, 2*size+1, tmp); /* Assume the target buffer is large enough */
	}
	return MI_OK;
}
#endif
SPROX_API_FUNC(ArrayToStringA) (char* string, const BYTE* buffer, WORD size)
{
	WORD i;
	char tmp[3];
	if (string == NULL) return MI_LIB_CALL_ERROR;
	if (buffer == NULL) return MI_LIB_CALL_ERROR;
	string[0] = '\0';
	for (i = 0; i < size; i++)
	{
		snprintf(tmp, 3, "%02X", buffer[i]);
		strlcat(string, tmp, 2*size+1); /* Assume the target buffer is large enough */
	}
	return MI_OK;
}

/**f* SpringProx.API/SPROX_StringToArray
 *
 * NAME
 *   SPROX_StringToArray
 *
 * DESCRIPTION
 *   Translate a string of hexadecimal digits into an array of bytes
 *
 * INPUTS
 *   BYTE *buffer        : byte array to receive the result
 *   const TCHAR *string : hexadecimal string
 *   WORD size           : size of the byte array
 *
 * RETURNS
 *   MI_OK              : success
 *
 * NOTES
 *   The length of the string must be equals to 2 * size
 *
 * VARIANTS
 *   SPROX_StringToArrayA : ASCII version
 *   SPROX_StringToArrayW : UNICODE version
 *
 **/
static BYTE h2q(char h)
{
	if ((h >= '0') && (h <= '9')) return h - '0';
	if ((h >= 'A') && (h <= 'F')) return h - 'A' + 10;
	if ((h >= 'a') && (h <= 'f')) return h - 'a' + 10;
	return 0xFF;
}

SPROX_API_FUNC(StringToArray) (BYTE* buffer, const TCHAR* string, WORD size)
{
#ifdef UNICODE
	return SPROX_API_CALL(StringToArrayW) (buffer, string, size);
#else
	return SPROX_API_CALL(StringToArrayA) (buffer, string, size);
#endif
}

#ifdef WIN32
SPROX_API_FUNC(StringToArrayW) (BYTE* buffer, const wchar_t* string, WORD size)
{
	WORD i;
	BYTE q;

	if (string == NULL) return MI_LIB_CALL_ERROR;
	if (buffer == NULL) return MI_LIB_CALL_ERROR;
	if (wcslen(string) < (size_t)(2 * size)) return MI_LIB_CALL_ERROR;

	for (i = 0; i < size; i++)
	{
		q = h2q((BYTE)string[2 * i]);
		if (q > 0x0F) return MI_LIB_CALL_ERROR;
		buffer[i] = q;
		buffer[i] <<= 4;
		q = h2q((BYTE)string[2 * i + 1]);
		if (q > 0x0F) return MI_LIB_CALL_ERROR;
		buffer[i] += q;
	}

	return MI_OK;
}
#endif

SPROX_API_FUNC(StringToArrayA) (BYTE* buffer, const char* string, WORD size)
{
	WORD i;
	BYTE q;

	if (string == NULL) return MI_LIB_CALL_ERROR;
	if (buffer == NULL) return MI_LIB_CALL_ERROR;
	if (strlen(string) < (size_t)(2 * size)) return MI_LIB_CALL_ERROR;

	for (i = 0; i < size; i++)
	{
		q = h2q((BYTE)string[2 * i]);
		if (q > 0x0F) return MI_LIB_CALL_ERROR;
		buffer[i] = q;
		buffer[i] <<= 4;
		q = h2q((BYTE)string[2 * i + 1]);
		if (q > 0x0F) return MI_LIB_CALL_ERROR;
		buffer[i] += q;
	}

	return MI_OK;
}

/**f* SpringProx.API/SPROX_StrLen
 *
 * NAME
 *   SPROX_StrLen
 *
 * DESCRIPTION
 *   Determines the length of a string
 *
 * INPUTS
 *   const TCHAR *string : input string
 *
 * RETURNS
 *   WORD : actual length (= number of characters) of the string
 *
 * VARIANTS
 *   SPROX_StrLenA : ASCII version
 *   SPROX_StrLenW : UNICODE version
 *
 **/
SPROX_API_FUNC_T(WORD, StrLen) (const TCHAR* string)
{
	if (string == NULL) return 0;
	return (WORD)_tcsclen(string);
}

#ifdef WIN32
SPROX_API_FUNC_T(WORD, StrLenW) (const wchar_t* string)
{
	if (string == NULL)  return 0;
	return (WORD)wcslen(string);
}
#endif
SPROX_API_FUNC_T(WORD, StrLenA) (const char* string)
{
	if (string == NULL)  return 0;
	return (WORD)strlen(string);
}

/**f* SpringProx.API/SPROX_Malloc
 *
 * NAME
 *   SPROX_Malloc
 *
 * DESCRIPTION
 *   Dynamic allocation of a buffer
 *
 * INPUTS
 *   BYTE **buffer      : pointer to receive the address of the allocated buffer
 *   WORD size          : size of the buffer
 *
 * RETURNS
 *   MI_OK              : success
 *
 * NOTES
 *   The buffer must be freed using SPROX_Free
 *
 **/
SPROX_API_FUNC(Malloc) (BYTE** buffer, WORD size)
{
	if (buffer == NULL)
		return MI_LIB_CALL_ERROR;
	*buffer = (BYTE*)malloc(size);
	if (*buffer == NULL)
		return MI_OUT_OF_MEMORY_ERROR;
	return MI_OK;
}

/**f* SpringProx.API/SPROX_Free
 *
 * NAME
 *   SPROX_Free
 *
 * DESCRIPTION
 *   Desallocation of a dynamic buffer
 *
 * INPUTS
 *   BYTE *buffer       : address of the allocated buffer
 *
 * RETURNS
 *   MI_OK              : success
 *
 * SEE ALSO
 *   SPROX_Malloc
 *
 **/
SPROX_API_FUNC(Free) (BYTE* buffer)
{
	if (buffer == NULL)
		return MI_LIB_CALL_ERROR;
	free(buffer);
	return MI_OK;
}

#endif

