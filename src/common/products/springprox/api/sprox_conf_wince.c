/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  SpringProx API
  --------------

  Copyright (c) 2000-2008 SpringCard SAS, FRANCE - www.springcard.com

  config_wince.c
  --------------
  Where to store configuration settings under WinCE

  revision :
  ----------

	JDA 09/04/2004 : created
	JDA 26/08/2004 : added device_specific_info stuff
	JDA 22/09/2006 : added the ability to put DeviceName in registry
	JDA 29/04/2008 : forked WinCE / Win32

*/
#include "sprox_api_i.h"

#if (defined(WIN32) && defined(UNDER_CE))

#define PROACTIVE_VENDOR_ID              _T("PRO-ACTIVE_FR")
#define SPRINGPROX_PRODUCT_ID            _T("SPRINGPROX")

#define SOCKETMOBILE_VENDOR_ID           _T("Socket_Mobile_Inc.")
#define SOCKET_RFID_PRODUCT_ID           _T("_RFID_Card_")
#define SOCKET_COMBO_PRODUCT_ID          _T("_RFID_Combo_Card_")
#define SOCKET_COMBO_SCANNER_PORT        _T("Scanner Serial 2")

#define ARASAN_DEVICE_KEY_STR _T("\\Drivers\\SDCARD\\ClientDrivers\\Custom\\MANF-0296-CARDID-5347-FUNC-1")

#if (!defined(_SMA1020_))

BOOL LoadDefaultDevice_HW_WinCE(void)
{
  DWORD i, j = 0;

  HKEY    hKey, hSubKey;
  DWORD   size;
  TCHAR   shortBuffer[32];
  TCHAR   longBuffer[256];

  /* Open the HKLM\Drivers\Active key in registry */
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Drivers\\Active"), 0, KEY_READ|KEY_EXECUTE, &hKey) == ERROR_SUCCESS)
  {
    /* Get subkeys count */
    if (RegQueryInfoKey(hKey, NULL, NULL, NULL, &i, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
    {
      /* Browse subkeys in reverse order (pluggable cards are not at the beginning of the list !) */
      while (j < i)
      {        
        /* Select the subkey */
        size = sizeof(shortBuffer);
        if (RegEnumKeyEx(hKey, j, shortBuffer, &size, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {          
          /* Open the subkey */
          shortBuffer[sizeof(shortBuffer) - 1] = '\0';
          _stprintf(longBuffer, _T("Drivers\\Active\\%s"), shortBuffer);
          if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, longBuffer, 0, 0, &hSubKey) == ERROR_SUCCESS)
          {
            /* Read the PnPID value */
            size = sizeof(longBuffer);                
            if (RegQueryValueEx(hSubKey, _T("PnPID"), NULL, NULL, (LPBYTE) longBuffer, &size) == ERROR_SUCCESS)
            {                
              /* Compare the PnPID to requested one */
              if (((_tcsstr(longBuffer, PROACTIVE_VENDOR_ID) != NULL) && (_tcsstr(longBuffer, SPRINGPROX_PRODUCT_ID) != NULL))
               || ((_tcsstr(longBuffer, SOCKETMOBILE_VENDOR_ID) != NULL) && (_tcsstr(longBuffer, SOCKET_RFID_PRODUCT_ID) != NULL))
               || ((_tcsstr(longBuffer, SOCKETMOBILE_VENDOR_ID) != NULL) && (_tcsstr(longBuffer, SOCKET_COMBO_PRODUCT_ID) != NULL) && (_tcsstr(longBuffer, SOCKET_COMBO_SCANNER_PORT) == NULL)))
              {
                /* This is the good PnPID, now get the serial com in the "Name" value */
                size = sizeof(DefaultDeviceName);
                if (RegQueryValueEx(hSubKey, _T("Name"), NULL, NULL, (LPBYTE) DefaultDeviceName, &size) == ERROR_SUCCESS)
                {
                  /* Is it a Comm. port? */
                  if (_tcsncmp(DefaultDeviceName, _T("COM"), 3) == 0)
                  {				
                    /* Found ! */
                    RegCloseKey(hSubKey);
                    RegCloseKey(hKey);
                    /* OK, note this is a CF reader */
                    DefaultSettingsForced &= COM_OPTION_MASK;
                    DefaultSettingsForced |= COM_OPTION_POWER_AUTO;
                    return TRUE;
                  }
                }
              }
            }			

            /* New 1.20 (for SpringXXXX-SD) : lookup for Arasan device */
            /* New 1.80 (for Socket) : lookup in the Key (not only the PnpID) */
            size = sizeof(longBuffer);
            if (RegQueryValueEx(hSubKey, _T("Key"), NULL, NULL, (LPBYTE) longBuffer, &size) == ERROR_SUCCESS)
            {
              if (_tcsncmp(longBuffer, ARASAN_DEVICE_KEY_STR, _tcsclen(ARASAN_DEVICE_KEY_STR)) == 0)
              {
                /* This is the good PnPID, now get the serial com in the "Name" value */
                size = sizeof(DefaultDeviceName);
                if (RegQueryValueEx(hSubKey, _T("Name"), NULL, NULL, (LPBYTE) DefaultDeviceName, &size) == ERROR_SUCCESS)
                {
                  /* Found ! */
                  RegCloseKey(hSubKey);
                  RegCloseKey(hKey);
                  /* OK, note this is an SDIO reader */
                  DefaultSettingsForced &= COM_OPTION_MASK;
                  DefaultSettingsForced |= COM_OPTION_POWER_AUTO;
                  return TRUE;
                }
              } else
              if (((_tcsstr(longBuffer, SOCKETMOBILE_VENDOR_ID) != NULL) && (_tcsstr(longBuffer, SOCKET_RFID_PRODUCT_ID) != NULL))
               || ((_tcsstr(longBuffer, SOCKETMOBILE_VENDOR_ID) != NULL) && (_tcsstr(longBuffer, SOCKET_COMBO_PRODUCT_ID) != NULL) && (_tcsstr(longBuffer, SOCKET_COMBO_SCANNER_PORT) == NULL)))
              {
                /* This is the good Key, now get the serial com in the "Name" value */
                size = sizeof(DefaultDeviceName);
                if (RegQueryValueEx(hSubKey, _T("Name"), NULL, NULL, (LPBYTE) DefaultDeviceName, &size) == ERROR_SUCCESS)
                {
                  /* Is it a Comm. port? */
                  if (_tcsncmp(DefaultDeviceName, _T("COM"), 3) == 0)
                  {
                    /* Found ! */
                    RegCloseKey(hSubKey);
                    RegCloseKey(hKey);				  
                    /* OK, note this is a CF reader */
                    DefaultSettingsForced &= COM_OPTION_MASK;
                    DefaultSettingsForced |= COM_OPTION_POWER_AUTO;
                    return TRUE;
                  }
                }
              }
            }

            /* Close the subkey */
            RegCloseKey(hSubKey);
          }
        }
        
        j++;
      }
    }
  }
  /* Close the key */
  RegCloseKey(hKey);
  return FALSE;
}

#endif

#if (defined(_SMA1020_))
/* TRICUBES specific : device is physically bound to COM7 */
void LoadDefaultDevice_HW_WinCE(TCHAR *DefaultDeviceName)
{
  _tcscpy(DefaultDeviceName, _T("COM7:"));  
}
#endif

#endif
