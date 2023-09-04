/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  SpringProx API
  --------------

  Copyright (c) 2000-2008 SpringCard SAS, FRANCE - www.springcard.com

  serial_ftdi_win32.c
  -------------------
  serial communication on top of FTDI library, under Windows

  revision :
  ----------

  JDA 30/07/2003 : creation

*/
#include "sprox_api_i.h"

#if (defined(WIN32) && !defined(UNDER_CE) && !defined(SPROX_API_NO_FTDI))

static HINSTANCE hModule = NULL;
static BOOL FTDI_ENABLED = FALSE;

typedef BOOL(WINAPI T_FT_W32_ReadFile) (FT_HANDLE ftHandle, LPVOID lpBuffer, DWORD nBufferSize, LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped);
typedef BOOL(WINAPI T_FT_W32_WriteFile) (FT_HANDLE ftHandle, LPVOID lpBuffer, DWORD nBufferSize, LPDWORD lpBytesWritten, LPOVERLAPPED lpOverlapped);
typedef FT_HANDLE(WINAPI T_FT_W32_CreateFile) (LPCSTR lpszName, DWORD dwAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreate,
                                               DWORD dwAttrsAndFlags, HANDLE hTemplate);
typedef BOOL(WINAPI T_FT_W32_CloseHandle) (FT_HANDLE ftHandle);
typedef BOOL(WINAPI T_FT_W32_GetCommState) (FT_HANDLE ftHandle, LPFTDCB lpftDcb);
typedef BOOL(WINAPI T_FT_W32_SetCommState) (FT_HANDLE ftHandle, LPFTDCB lpftDcb);
typedef BOOL(WINAPI T_FT_W32_GetCommTimeouts) (FT_HANDLE ftHandle, FTTIMEOUTS * pTimeouts);
typedef BOOL(WINAPI T_FT_W32_SetCommTimeouts) (FT_HANDLE ftHandle, FTTIMEOUTS * pTimeouts);
typedef BOOL(WINAPI T_FT_W32_ClearCommError) (FT_HANDLE ftHandle, LPDWORD lpdwErrors, LPFTCOMSTAT lpftComstat);
typedef BOOL(WINAPI T_FT_W32_PurgeComm) (FT_HANDLE ftHandle, DWORD dwMask);

typedef FT_STATUS(WINAPI T_FT_ResetDevice) (FT_HANDLE ftHandle);
typedef FT_STATUS(WINAPI T_FT_SetUSBParameters) (FT_HANDLE ftHandle, ULONG ulInTransferSize, ULONG ulOutTransferSize);
typedef FT_STATUS(WINAPI T_FT_SetLatencyTimer) (FT_HANDLE ftHandle, UCHAR ucLatency);
typedef FT_STATUS(WINAPI T_FT_ListDevices) (PVOID pArg1, PVOID pArg2, DWORD Flags);
typedef DWORD(WINAPI T_FT_W32_GetLastError) (FT_HANDLE ftHandle);

T_FT_W32_ReadFile *p_FT_W32_ReadFile;
T_FT_W32_WriteFile *p_FT_W32_WriteFile;
T_FT_W32_CreateFile *p_FT_W32_CreateFile;
T_FT_W32_CloseHandle *p_FT_W32_CloseHandle;
T_FT_W32_GetCommState *p_FT_W32_GetCommState;
T_FT_W32_SetCommState *p_FT_W32_SetCommState;
T_FT_W32_GetCommTimeouts *p_FT_W32_GetCommTimeouts;
T_FT_W32_SetCommTimeouts *p_FT_W32_SetCommTimeouts;
T_FT_W32_ClearCommError *p_FT_W32_ClearCommError;
T_FT_W32_PurgeComm *p_FT_W32_PurgeComm;
T_FT_W32_GetLastError *p_FT_W32_GetLastError;

T_FT_ResetDevice *p_FT_ResetDevice;
T_FT_SetUSBParameters *p_FT_SetUSBParameters;
T_FT_SetLatencyTimer *p_FT_SetLatencyTimer;
T_FT_ListDevices *p_FT_ListDevices;

BOOL FT_W32_Initialize(void)
{
  if (FTDI_ENABLED)
    return TRUE;

  FTDI_ENABLED = FALSE;
#ifdef UNICODE
  hModule = LoadLibraryW(L"ftd2xx.dll");
#else
  hModule = LoadLibraryA("ftd2xx.dll");
#endif

  if (hModule == NULL)
    return FALSE;
  FTDI_ENABLED = TRUE;

  p_FT_W32_CreateFile = (T_FT_W32_CreateFile *) GetProcAddress(hModule, "FT_W32_CreateFile");
  if (p_FT_W32_CreateFile == NULL)
    FTDI_ENABLED = FALSE;

  p_FT_W32_CloseHandle = (T_FT_W32_CloseHandle *) GetProcAddress(hModule, "FT_W32_CloseHandle");
  if (p_FT_W32_CloseHandle == NULL)
    FTDI_ENABLED = FALSE;

  p_FT_W32_ReadFile = (T_FT_W32_ReadFile *) GetProcAddress(hModule, "FT_W32_ReadFile");
  if (p_FT_W32_ReadFile == NULL)
    FTDI_ENABLED = FALSE;

  p_FT_W32_WriteFile = (T_FT_W32_WriteFile *) GetProcAddress(hModule, "FT_W32_WriteFile");
  if (p_FT_W32_WriteFile == NULL)
    FTDI_ENABLED = FALSE;

  p_FT_W32_GetCommState = (T_FT_W32_GetCommState *) GetProcAddress(hModule, "FT_W32_GetCommState");
  if (p_FT_W32_GetCommState == NULL)
    FTDI_ENABLED = FALSE;

  p_FT_W32_SetCommState = (T_FT_W32_SetCommState *) GetProcAddress(hModule, "FT_W32_SetCommState");
  if (p_FT_W32_SetCommState == NULL)
    FTDI_ENABLED = FALSE;

  p_FT_W32_GetCommTimeouts = (T_FT_W32_GetCommTimeouts *) GetProcAddress(hModule, "FT_W32_GetCommTimeouts");
  if (p_FT_W32_GetCommTimeouts == NULL)
    FTDI_ENABLED = FALSE;

  p_FT_W32_SetCommTimeouts = (T_FT_W32_SetCommTimeouts *) GetProcAddress(hModule, "FT_W32_SetCommTimeouts");
  if (p_FT_W32_SetCommTimeouts == NULL)
    FTDI_ENABLED = FALSE;

  p_FT_W32_ClearCommError = (T_FT_W32_ClearCommError *) GetProcAddress(hModule, "FT_W32_ClearCommError");
  if (p_FT_W32_ClearCommError == NULL)
    FTDI_ENABLED = FALSE;

  p_FT_W32_PurgeComm = (T_FT_W32_PurgeComm *) GetProcAddress(hModule, "FT_W32_PurgeComm");
  if (p_FT_W32_PurgeComm == NULL)
    FTDI_ENABLED = FALSE;

  p_FT_W32_GetLastError = (T_FT_W32_GetLastError *) GetProcAddress(hModule, "FT_W32_GetLastError");
  if (p_FT_W32_GetLastError == NULL)
    FTDI_ENABLED = FALSE;

  p_FT_ResetDevice = (T_FT_ResetDevice *) GetProcAddress(hModule, "FT_ResetDevice");
  if (p_FT_ResetDevice == NULL)
    FTDI_ENABLED = FALSE;

  p_FT_SetUSBParameters = (T_FT_SetUSBParameters *) GetProcAddress(hModule, "FT_SetUSBParameters");
  if (p_FT_SetUSBParameters == NULL)
    FTDI_ENABLED = FALSE;

  p_FT_SetLatencyTimer = (T_FT_SetLatencyTimer *) GetProcAddress(hModule, "FT_SetLatencyTimer");
  if (p_FT_SetLatencyTimer == NULL)
    FTDI_ENABLED = FALSE;

  p_FT_ListDevices = (T_FT_ListDevices *) GetProcAddress(hModule, "FT_ListDevices");
  if (p_FT_ListDevices == NULL)
    FTDI_ENABLED = FALSE;

  if (!FTDI_ENABLED)
  {
    FreeLibrary(hModule);
    hModule = NULL;
  } else
  {

  }
  return FTDI_ENABLED;
}

void FT_W32_Finalize(void)
{
  if (hModule != NULL)
  {
    FreeLibrary(hModule);
    hModule = NULL;
  }
  FTDI_ENABLED = FALSE;
}

FTD2XX_API FT_HANDLE WINAPI FT_W32_CreateFile(LPCSTR lpszName, DWORD dwAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreate,
                                              DWORD dwAttrsAndFlags, HANDLE hTemplate)
{
  __try
  {
    if (!FT_W32_Initialize())
      return NULL;
    return p_FT_W32_CreateFile(lpszName, dwAccess, dwShareMode, lpSecurityAttributes, dwCreate, dwAttrsAndFlags, hTemplate);
  }
  __except(1)
  {
    return NULL;
  }
}

FTD2XX_API BOOL WINAPI FT_W32_ReadFile(FT_HANDLE ftHandle, LPVOID lpBuffer, DWORD nBufferSize, LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped)
{
  if (!FTDI_ENABLED)
    return FALSE;

  __try
  {
    BOOL    rc;
    rc = p_FT_W32_ReadFile(ftHandle, lpBuffer, nBufferSize, lpBytesReturned, lpOverlapped);
    if (!rc)
    {
      if (FT_W32_GetLastError(ftHandle) == FT_IO_ERROR)
        FT_W32_Finalize();
    }
    return rc;
  }
  __except(1)
  {
    return FALSE;
  }
}

FTD2XX_API BOOL WINAPI FT_W32_WriteFile(FT_HANDLE ftHandle, LPVOID lpBuffer, DWORD nBufferSize, LPDWORD lpBytesWritten, LPOVERLAPPED lpOverlapped)
{
  if (!FTDI_ENABLED)
    return FALSE;

  __try
  {
    BOOL    rc;
    rc = p_FT_W32_WriteFile(ftHandle, lpBuffer, nBufferSize, lpBytesWritten, lpOverlapped);
    if (!rc)
    {
      if (FT_W32_GetLastError(ftHandle) == FT_IO_ERROR)
        FT_W32_Finalize();
    }
    return rc;
  }
  __except(1)
  {
    return FALSE;
  }
}

FTD2XX_API BOOL WINAPI FT_W32_CloseHandle(FT_HANDLE ftHandle)
{
  __try
  {
    BOOL    rc = TRUE;
    if (FTDI_ENABLED)
    {
      rc = p_FT_W32_CloseHandle(ftHandle);
      FT_W32_Finalize();
    }
    return rc;
  }
  __except(1)
  {
    return FALSE;
  }
}

FTD2XX_API BOOL WINAPI FT_W32_GetCommState(FT_HANDLE ftHandle, LPFTDCB lpftDcb)
{
  if (!FTDI_ENABLED)
    return FALSE;
  __try
  {
    return p_FT_W32_GetCommState(ftHandle, lpftDcb);
  }
  __except(1)
  {
    return FALSE;
  }
}

FTD2XX_API BOOL WINAPI FT_W32_SetCommState(FT_HANDLE ftHandle, LPFTDCB lpftDcb)
{
  if (!FTDI_ENABLED)
    return FALSE;
  __try
  {
    return p_FT_W32_SetCommState(ftHandle, lpftDcb);
  }
  __except(1)
  {
    return FALSE;
  }
}

FTD2XX_API BOOL WINAPI FT_W32_GetCommTimeouts(FT_HANDLE ftHandle, FTTIMEOUTS * pTimeouts)
{
  if (!FTDI_ENABLED)
    return FALSE;
  __try
  {
    return p_FT_W32_GetCommTimeouts(ftHandle, pTimeouts);
  }
  __except(1)
  {
    return FALSE;
  }
}

FTD2XX_API BOOL WINAPI FT_W32_SetCommTimeouts(FT_HANDLE ftHandle, FTTIMEOUTS * pTimeouts)
{
  if (!FTDI_ENABLED)
    return FALSE;
  __try
  {
    return p_FT_W32_SetCommTimeouts(ftHandle, pTimeouts);
  }
  __except(1)
  {
    return FALSE;
  }
}

FTD2XX_API BOOL WINAPI FT_W32_ClearCommError(FT_HANDLE ftHandle, PDWORD lpdwErrors, LPFTCOMSTAT lpftComstat)
{
  __try
  {
    DWORD   arg1;
    FTCOMSTAT arg2;
    if (!FTDI_ENABLED)
      return FALSE;
    if (lpdwErrors == NULL)
      lpdwErrors = &arg1;
    if (lpftComstat == NULL)
      lpftComstat = &arg2;
    return p_FT_W32_ClearCommError(ftHandle, lpdwErrors, lpftComstat);
  }
  __except(1)
  {
    return FALSE;
  }
}

FTD2XX_API BOOL WINAPI FT_W32_PurgeComm(FT_HANDLE ftHandle, DWORD dwMask)
{
  if (!FTDI_ENABLED)
    return FALSE;
  __try
  {
    return p_FT_W32_PurgeComm(ftHandle, dwMask);
  }
  __except(1)
  {
    return FALSE;
  }
}

FTD2XX_API DWORD WINAPI FT_W32_GetLastError(FT_HANDLE ftHandle)
{
  if (!FTDI_ENABLED)
    return 0;
  __try
  {
    return p_FT_W32_GetLastError(ftHandle);
  }
  __except(1)
  {
    return 0;
  }
}

FTD2XX_API FT_STATUS WINAPI FT_ListDevices(PVOID pArg1, PVOID pArg2, DWORD Flags)
{
  __try
  {
    if (!FT_W32_Initialize())
      return FT_INVALID_HANDLE;
    return p_FT_ListDevices(pArg1, pArg2, Flags);
  }
  __except(1)
  {
    return FT_OTHER_ERROR;
  }
}

FTD2XX_API FT_STATUS WINAPI FT_SetLatencyTimer(FT_HANDLE ftHandle, UCHAR ucLatency)
{
  if (!FTDI_ENABLED)
    return FT_INVALID_HANDLE;
  __try
  {
    return p_FT_SetLatencyTimer(ftHandle, ucLatency);
  }
  __except(1)
  {
    return FT_OTHER_ERROR;
  }
}

FTD2XX_API FT_STATUS WINAPI FT_SetUSBParameters(FT_HANDLE ftHandle, ULONG ulInTransferSize, ULONG ulOutTransferSize)
{
  if (!FTDI_ENABLED)
    return FT_INVALID_HANDLE;
  __try
  {
    return p_FT_SetUSBParameters(ftHandle, ulInTransferSize, ulOutTransferSize);
  }
  __except(1)
  {
    return FT_OTHER_ERROR;
  }
}

FTD2XX_API FT_STATUS WINAPI FT_ResetDevice(FT_HANDLE ftHandle)
{
  if (!FTDI_ENABLED)
    return FT_INVALID_HANDLE;
  __try
  {
    return p_FT_ResetDevice(ftHandle);
  }
  __except(1)
  {
    return FT_OTHER_ERROR;
  }
}


/**f* SpringProx.API/SPROX_EnumUSBDevices
 *
 * NAME
 *   SPROX_EnumUSBDevices
 *
 * DESCRIPTION
 *   Used to enumerate available USB devices that are supported by the SpringProx library
 *   (WIN32 with USB support only)
 *
 * INPUTS
 *   DWORD idx             : index of the device to query
 *   TCHAR device[64]      : buffer to receive the device identifier (mandatory)
 *   TCHAR description[64] : buffer to receive the device descriptor (optional)
 *
 * RETURNS
 *   MI_OK                 : success, device (and description) have been provided
 *   MI_LIB_CALL_ERROR     : invalid value for idx, end of the loop
 *   MI_SER_ACCESS_ERR     : error during query
 *
 **/
SPROX_API_FUNC(EnumUSBDevices) (DWORD idx, TCHAR device[64], TCHAR description[64])
{
  FT_STATUS    ftStatus;
  DWORD        count;
  char         buffer[64];
  unsigned int i;
  
  if (device == NULL)
    return MI_LIB_CALL_ERROR;

  ftStatus = FT_ListDevices(&count, NULL, FT_LIST_NUMBER_ONLY);
  if (ftStatus == FT_OK)
  {
    /* FT_ListDevices OK, number of devices connected is in numDevs */
    if (idx >= count)
      return MI_LIB_CALL_ERROR;
  } else
  {
    /* FT_ListDevices failed */
    return MI_SER_ACCESS_ERR;
  }

  ftStatus = FT_ListDevices((PVOID) idx, buffer, FT_LIST_BY_INDEX|FT_OPEN_BY_SERIAL_NUMBER);
  if (ftStatus == FT_OK)
  {
    /* FT_ListDevices OK, serial number is in Buffer */
    device[0] = 'U'; device[1] = 'S'; device[2] = 'B'; device[3] = ':';
    for (i=0; i<=strlen(buffer); i++)
    {
      if (i>=60) break;
      device[4+i] = buffer[i];
    }
    device[63] = '\0';
  } else
  {
    /* FT_ListDevices failed */
    return MI_SER_ACCESS_ERR;
  }
  
  if (description != NULL)
  {
    ftStatus = FT_ListDevices((PVOID) idx, buffer, FT_LIST_BY_INDEX|FT_OPEN_BY_DESCRIPTION);
    if (ftStatus == FT_OK)
    {
      /* FT_ListDevices OK, description is in Buffer */
      for (i=0; i<=strlen(buffer); i++)
      {
        if (i>=64) break;
        description[i] = buffer[i];
      }
      description[63] = '\0';
    } else
    {
      /* FT_ListDevices failed */
      return MI_SER_ACCESS_ERR;
    }
  }

  return MI_OK;
}

#endif
