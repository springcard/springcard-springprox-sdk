/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  SpringProx API
  --------------

  Copyright (c) 2000-2008 SpringCard SAS, FRANCE - www.springcard.com

  sprox_serial.h
  --------------
  Serial communication prototypes

*/
#ifndef SPROX_SERIAL_H
#define SPROX_SERIAL_H

BOOL    SendByte(SPROX_CTX_ST *sprox_ctx, BYTE b);
BOOL    SendBurst(SPROX_CTX_ST *sprox_ctx, const BYTE *b, WORD len);
BOOL    RecvByte(SPROX_CTX_ST *sprox_ctx, BYTE *b);
BOOL    RecvBurst(SPROX_CTX_ST *sprox_ctx, BYTE *b, WORD len);

BOOL    SerialSetTimeouts(SPROX_CTX_ST *sprox_ctx, DWORD resp_tmo, DWORD byte_tmo);
BOOL    SerialSetBaudrate(SPROX_CTX_ST *sprox_ctx, DWORD baudrate);
BOOL    SerialLookup(SPROX_CTX_ST *sprox_ctx);

BOOL    SerialOpen(SPROX_CTX_ST *sprox_ctx, const TCHAR *device);
void    SerialClose(SPROX_CTX_ST *sprox_ctx);
BOOL    SerialPowerUp(SPROX_CTX_ST *sprox_ctx);
BOOL    SerialPowerDown(SPROX_CTX_ST *sprox_ctx);
BOOL    SerialReset(SPROX_CTX_ST *sprox_ctx);

/* FTDI support */
/* ------------ */
#ifndef SPROX_API_NO_FTDI

  #ifdef WIN32
    /* FDTI support using FDT2XX library */
    #define FTD2XX_API
    #include "ftd2xx.h"
    BOOL    FT_W32_Initialize(void);
    void    FT_W32_Finalize(void);
    #define FTDI_BURST_SIZE 64
    #define FTDI_BURST_TIME 8    
  #endif
  
  #ifdef LINUX
    /* FTDI support using libftdi */
    #include <ftdi.h>    
    BOOL FTDI_Open(SPROX_CTX_ST *sprox_ctx, const TCHAR *device);
    void FTDI_Close(SPROX_CTX_ST *sprox_ctx);
    BOOL FTDI_SetBaudrate(SPROX_CTX_ST *sprox_ctx, DWORD baudrate);
    BOOL FTDI_SendBurst(SPROX_CTX_ST *sprox_ctx, const BYTE *b, WORD len);
    BOOL FTDI_RecvBurst(SPROX_CTX_ST *sprox_ctx, const BYTE *b, WORD len);    
  #endif
#endif

#define INTER_BYTE_TMO     200  /* Timeout between each byte        (ms) */
#define OSI_PROTOCOL_TMO   500  /* Timeout for the OSI flow control (ms) */
#define ASCII_TMO         5000  /* Timeout for the ASCII protocol   (ms) */
#define RESPONSE_TMO      1200  /* Timeout between send and receive (ms) */

#endif
