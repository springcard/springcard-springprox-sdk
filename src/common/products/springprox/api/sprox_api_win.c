/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  SpringProx API
  --------------

  Copyright (c) 2000-2008 SpringCard SAS, FRANCE - www.springcard.com

  sprox_win_dll.c
  ---------------
  Specific stuff to build a Windows DLL

  revision :
  ----------

	JDA 29/04/2008 : created with stuff moved from config_win32.c
	LTC 17/12/2008 : change HKEY_SPRINGPROX to _T("Software\\SpringCard\\SpringProx")

*/
#include "sprox_api_i.h"

#if (defined(WIN32))


#define HKEY_SPRINGPROX       _T("Software\\SpringCard\\SpringProx")

BOOL LoadDefaultDevice(void)
{
#if (defined(UNDER_CE))
	return LoadDefaultDevice_HW_WinCE(sprox_ctx);
#endif    
	return FALSE;
}

/*
 * Retrieve (and apply) debug level
 * --------------------------------
 */
void LoadSettings(void)
{
	HKEY    hKey;
	DWORD   size;
	TCHAR   buffer[256];
	BYTE    loop;
	BYTE    level = 0;

	/*
	 * Load system and user specified settings
	 * ---------------------------------------
	 */
	for (loop = 0; loop <= 1; loop++)
	{
		if (loop == 0)
		{
			/* First step : try to locate debug config under HKCU\Sofware\SpringCard\SpringProx */
			if (!RegOpenKeyEx(HKEY_CURRENT_USER, HKEY_SPRINGPROX, 0, KEY_READ | KEY_EXECUTE, &hKey) == ERROR_SUCCESS)
				continue;
		}
		else
		{
			/* Second step : do the same under HKLM\Sofware\SpringCard\SpringProx */
			if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE, HKEY_SPRINGPROX, 0, KEY_READ | KEY_EXECUTE, &hKey) == ERROR_SUCCESS)
				continue;
		}

		size = sizeof(buffer);
		if (RegQueryValueEx(hKey, _T("DebugLevel"), NULL, NULL, (LPBYTE)buffer, &size) == ERROR_SUCCESS)
			level = (BYTE)_ttoi(buffer);

		size = sizeof(buffer);
		if (RegQueryValueEx(hKey, _T("DebugFile"), NULL, NULL, (LPBYTE)buffer, &size) == ERROR_SUCCESS)
			SPROX_TraceSetFile(buffer);

		size = sizeof(buffer);
		if (RegQueryValueEx(hKey, _T("DeviceName"), NULL, NULL, (LPBYTE)buffer, &size) == ERROR_SUCCESS)
		{
			_tcsncpy_s(DefaultDeviceName, sizeof(DefaultDeviceName) / sizeof(TCHAR), buffer, size / sizeof(TCHAR));
			DefaultDeviceName[(sizeof(DefaultDeviceName) / sizeof(TCHAR)) - 1] = '\0';
		}

		size = sizeof(buffer);
		if (RegQueryValueEx(hKey, _T("Protocols"), NULL, NULL, (LPBYTE)buffer, &size) == ERROR_SUCCESS)
		{
			DWORD m = _ttol(buffer) & 0x0FF;
			DefaultSettingsAllowed &= ~COM_PROTO_MASK;
			DefaultSettingsAllowed |= m << COM_PROTO_SHIFT;
		}

		size = sizeof(buffer);
		if (RegQueryValueEx(hKey, _T("Baudrates"), NULL, NULL, (LPBYTE)buffer, &size) == ERROR_SUCCESS)
		{
			DWORD m = _ttol(buffer) & 0x0FF;
			DefaultSettingsAllowed &= ~COM_BAUDRATE_MASK;
			DefaultSettingsAllowed |= m << COM_BAUDRATE_SHIFT;
		}

		size = sizeof(buffer);
		if (RegQueryValueEx(hKey, _T("Options"), NULL, NULL, (LPBYTE)buffer, &size) == ERROR_SUCCESS)
		{
			DWORD m = _ttol(buffer) & 0x0FF;
			DefaultSettingsAllowed &= ~COM_OPTION_MASK;
			DefaultSettingsAllowed |= m << COM_OPTION_SHIFT;
		}

		size = sizeof(buffer);
		if (RegQueryValueEx(hKey, _T("ForceProtocol"), NULL, NULL, (LPBYTE)buffer, &size) == ERROR_SUCCESS)
		{
			DWORD m = _ttol(buffer) & 0x0FF;
			DefaultSettingsForced &= ~COM_PROTO_MASK;
			DefaultSettingsForced |= m << COM_PROTO_SHIFT;
		}

		size = sizeof(buffer);
		if (RegQueryValueEx(hKey, _T("ForceBaudrate"), NULL, NULL, (LPBYTE)buffer, &size) == ERROR_SUCCESS)
		{
			DWORD m = _ttol(buffer) & 0x0FF;
			DefaultSettingsForced &= ~COM_BAUDRATE_MASK;
			DefaultSettingsForced |= m << COM_BAUDRATE_SHIFT;
		}

		size = sizeof(buffer);
		if (RegQueryValueEx(hKey, _T("SetOptions"), NULL, NULL, (LPBYTE)buffer, &size) == ERROR_SUCCESS)
		{
			DWORD m = _ttol(buffer) & 0x0FF;
			DefaultSettingsForced &= ~COM_OPTION_MASK;
			DefaultSettingsForced |= m << COM_OPTION_SHIFT;
		}

		RegCloseKey(hKey);
	}

	if (level)
	{
		SPROX_TraceSetLevel(level);
		if (level >= 6)
		{
			MessageBox(0, _T("The DLL is currently running with highest debug level. Be careful that debug file will become rapidly very very big !"), _T("SpringProx API"), 0);
		}
	}
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	__try
	{
		switch (ul_reason_for_call)
		{
		case DLL_PROCESS_ATTACH:
			LoadSettings();
#ifndef SPROX_API_REENTRANT
			memset(sprox_ctx_glob, 0, sizeof(SPROX_CTX_ST));
			sprox_ctx_glob->com_handle = INVALID_HANDLE_VALUE;
#endif
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
#ifndef SPROX_API_REENTRANT
			SPROX_ReaderClose();
#endif
			break;
		}
		return TRUE;
	}
	__except (1)
	{
		return FALSE;
	}
}

#endif
