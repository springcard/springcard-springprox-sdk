/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  SpringProx API
  --------------

  Copyright (c) 2000-2008 SpringCard SAS, FRANCE - www.springcard.com

  config_win32.c
  --------------
  Where to store configuration settings under Win32

  revision :
  ----------

	JDA 09/04/2004 : created
	JDA 26/08/2004 : added device_specific_info stuff
	JDA 22/09/2006 : added the ability to put DeviceName in registry
	JDA 29/04/2008 : forked WinCE / Win32
					 enumerating the comm devices now uses SetupAPI
	LTC 17/12/2008 : change name manufacturer to "springcard"

*/
#include "sprox_api_i.h"

#if (defined(WIN32) && !defined(UNDER_CE))

static BOOL BuildSerialDeviceList(char* buffer, WORD length);

/**f* SpringProx.API/SPROX_GetCommList
 *
 * NAME
 *   SPROX_GetCommList
 *
 * DESCRIPTION
 *   Retrieve the list of communication ports existing on the computers
 *
 * INPUTS
 *   TCHAR *buffer      : buffer to receive the list of devices
 *   WORD  length       : character-size of the buffer
 *
 * RETURNS
 *   MI_OK              : success
 *
 **/
SPROX_API_FUNC(GetCommListA) (char* buffer, WORD length)
{
	if (BuildSerialDeviceList(buffer, length))
		return MI_OK;
	return MI_LIB_INTERNAL_ERROR;
}

SPROX_API_FUNC(GetCommListW) (wchar_t* buffer, WORD length)
{
	WORD  i;
	char* t = malloc(length);
	if (t == NULL) return MI_OUT_OF_MEMORY_ERROR;

	if (BuildSerialDeviceList(t, length))
	{
		for (i = 0; i < length; i++)
		{
			buffer[i] = t[i];
			if (t[i] == '\0') break;
		}
		free(t);
		return MI_OK;
	}
	free(t);
	return MI_LIB_INTERNAL_ERROR;
}

SPROX_API_FUNC(GetCommNames) (TCHAR* buffer, WORD length)
{
#ifdef UNICODE
	return SPROX_API_CALL(GetCommListW) (buffer, length);
#else
	return SPROX_API_CALL(GetCommListA) (buffer, length);
#endif  
}



BOOL GetDefaultDevice_HW_Win32(SPROX_CTX_ST* sprox_ctx)
{
	char buffer[512];
	char* p_name, * p_value;
	int i;

	if (sprox_ctx == NULL)
		return FALSE;

	if (!BuildSerialDeviceList(buffer, sizeof(buffer)))
		return FALSE;

	p_name = buffer;
	while (*p_name != '\0')
	{
		p_value = p_name;
		/* Jump at the end of the PortName */
		while ((*p_value != '\0') && (*p_value != '=')) p_value++;
		if (*p_value == '=') { *p_value = '\0'; p_value++; }
		/* Jump at the end of the FriendlyName */
		while ((*p_value != '\0') && (*p_value != ';')) p_value++;
		if (*p_value == ';') p_value++;

		/* We are located on the Manufacturer */
		if (!_strnicmp(p_value, "springcard", 10))
		{
			/* Positive match, cleanup the name and assing it */
			*p_value = '\0';

			i = 0;
			if (!_strnicmp("COM", p_name, 3) && (strlen(p_name) > 4))
			{
				/* COM above 10 must be labelled \\.\COMxx */
				sprox_ctx->com_name[i++] = '\\';
				sprox_ctx->com_name[i++] = '\\';
				sprox_ctx->com_name[i++] = '.';
				sprox_ctx->com_name[i++] = '\\';
			}

			while ((*p_name != '\0') && (i < 31))
			{
				sprox_ctx->com_name[i++] = *p_name;
				p_name++;
			}
			sprox_ctx->com_name[i] = '\0';

			return TRUE;
		}

		/* Next line */
		p_name = p_value;
		while ((*p_name != '\0') && (*p_name != '\n')) p_name++;
		if (*p_name == '\n') p_name++;
	}

	return FALSE;
}







#include <setupapi.h>
#pragma comment (lib, "setupapi.lib")

size_t pa_strlcat(char* dst, const char* src, size_t dstsize)
{
	char* df = dst;
	size_t left = dstsize;
	size_t l1;
	size_t l2 = strlen(src);
	size_t copied;

	while (left-- != 0 && *df != '\0')
		df++;
	l1 = df - dst;
	if (dstsize == l1)
		return (l1 + l2);

	copied = l1 + l2 >= dstsize ? dstsize - l1 - 1 : l2;
	(void)memcpy(dst + l1, src, copied);
	dst[l1 + copied] = '\0';
	return (l1 + l2);
}

void ReadDeviceProperty(HDEVINFO devinfo, SP_DEVINFO_DATA* devinfodata, DWORD ident, char buffer[], DWORD size)
{
	memset(buffer, 0, size);
	SetupDiGetDeviceRegistryPropertyA(devinfo, devinfodata, ident, NULL, buffer, size - 1, NULL);
}

void GetSerialDeviceData(HDEVINFO devinfo, SP_DEVINFO_DATA* devinfodata, char* buffer, WORD length)
{
#define GET_PROPERTY(i) \
    ReadDeviceProperty(devinfo, devinfodata, i, szValue, sizeof(szValue));

#define GET_VALUE(n) \
    memset(szValue, 0, sizeof(szValue)); \
    dwType = REG_SZ; \
    dwSize = sizeof(szValue); \
    RegQueryValueExA(hKey, n, 0, &dwType, szValue, &dwSize);

	char szValue[512 + 1];

	HKEY hKey;
	DWORD dwIndex = 0;
	DWORD dwSize, dwType;
	char* p_vid, * p_pid;

	hKey = SetupDiOpenDevRegKey(devinfo, devinfodata, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);

	if (hKey == INVALID_HANDLE_VALUE) return;
	GET_VALUE("PortName");
	pa_strlcat(buffer, szValue, length);
	pa_strlcat(buffer, "=", length);
	RegCloseKey(hKey);

	GET_PROPERTY(SPDRP_DEVICEDESC);
	pa_strlcat(buffer, szValue, length);
	pa_strlcat(buffer, ";", length);

	GET_PROPERTY(SPDRP_MFG);
	pa_strlcat(buffer, szValue, length);
	pa_strlcat(buffer, ";", length);

	GET_PROPERTY(SPDRP_SERVICE);
	_strlwr_s(szValue, sizeof(szValue));
	pa_strlcat(buffer, szValue, length);
	pa_strlcat(buffer, ";", length);

	hKey = SetupDiOpenDevRegKey(devinfo, devinfodata, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ);
	if (hKey == INVALID_HANDLE_VALUE) return;

	GET_VALUE("MatchingDeviceID");
	_strlwr_s(szValue, sizeof(szValue));
	p_vid = szValue;
	while ((*p_vid != '\0') && strncmp(p_vid, "vid_", 4)) p_vid++;
	if (p_vid != '\0')
	{
		*p_vid = '\0';
		p_vid += 4;
	}
	p_pid = p_vid;
	while ((*p_pid != '\0') && strncmp(p_pid, "pid_", 4)) p_pid++;
	if (p_pid != '\0')
	{
		*p_pid = '\0';
		p_pid += 4;
	}
	szValue[strlen(szValue) - 1] = '\0';
	pa_strlcat(buffer, szValue, length);
	pa_strlcat(buffer, ";", length);
	p_vid[strlen(p_vid) - 1] = '\0';
	pa_strlcat(buffer, p_vid, length);
	pa_strlcat(buffer, ";", length);
	pa_strlcat(buffer, p_pid, length);
	pa_strlcat(buffer, ";", length);
	RegCloseKey(hKey);
}

static BOOL LoopSerialDeviceList(HDEVINFO devinfo, char* buffer, WORD length)
{
	DWORD dwIndex = 0;
	SP_DEVINFO_DATA devinfodata;

	devinfodata.cbSize = sizeof(SP_DEVINFO_DATA);

	for (;;)
	{
		if (!SetupDiEnumDeviceInfo(devinfo, dwIndex, &devinfodata))
		{
			// Check if we ran out of devices, or if there was an error
			if (GetLastError() == ERROR_NO_MORE_ITEMS)
				break;
			return FALSE;
		}

		GetSerialDeviceData(devinfo, &devinfodata, buffer, length);

		pa_strlcat(buffer, "\n", length);

		dwIndex++;
	}

	return TRUE;
}

static BOOL BuildSerialDeviceList(char* buffer, WORD length)
{
	GUID guid;
	HDEVINFO devinfo;
	DWORD dummy;
	BOOL rc = FALSE;

	if (buffer == NULL) return FALSE;
	if (length == 0)    return FALSE;

	buffer[0] = '\0';

	if (SetupDiClassGuidsFromName(_T("Ports"), &guid, 1, &dummy))
	{
		devinfo = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT);
		if (devinfo != INVALID_HANDLE_VALUE)
		{
			rc = LoopSerialDeviceList(devinfo, buffer, length);

			if (!SetupDiDestroyDeviceInfoList(devinfo))
				rc = FALSE;
		}
	}

	buffer[length - 1] = '\0';

	return rc;
}

#endif
