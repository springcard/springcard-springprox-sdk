/**h* SpringProx.API/CRC
 *
 * NAME
 *   SpringProx.API :: CRC computation
 *
 * DESCRIPTION
 *   Provides CRC computation according to ISO 14443
 *
 **/

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
  
  JDA 30/10/2003 : added to the SpringProx library
  
*/

#ifndef _BUILD_SPRINGPROX_EX_DLL

#include "sprox_api_i.h"

static WORD UpdateCrc(BYTE ch, WORD * lpwCrc)
{
  ch = (ch ^ (BYTE) ((*lpwCrc) & 0x00FF));
  ch = (ch ^ (ch << 4));
  *lpwCrc = (*lpwCrc >> 8) ^ ((WORD) ch << 8) ^ ((WORD) ch << 3) ^ ((WORD) ch >> 4);
  return (*lpwCrc);
}

/**f* SpringProx.API/SPROX_ComputeIso14443ACrc
 *
 * NAME
 *   SPROX_ComputeIso14443ACrc
 *
 * DESCRIPTION
 *   Compute a CRC according to ISO/IEC 14443-A (layer 3)
 *
 * INPUTS
 *   BYTE crc[2]         : 2-byte buffers to receive the CRC
 *                         (can be set to NULL since the CRC is also returned as
 *                         function result)
 *   const BYTE buffer[] : data to be checksumed
 *   WORD size           : length of the data
 *
 * RETURNS
 *   The computed CRC.
 *
 **/
SPRINGPROX_LIB WORD SPRINGPROX_API SPROX_ComputeIso14443ACrc(BYTE crc[2], const BYTE buffer[], WORD size)
{
  BYTE  chBlock;
  WORD  wCrc;
  BYTE *p = (BYTE *) buffer;
  
  wCrc = 0x6363;                /* ITU-V.41 */

  do
  {
    chBlock = *p++;
    UpdateCrc(chBlock, &wCrc);
  } while (--size);

  if (crc != NULL)
  {
    crc[0] = (BYTE) (wCrc & 0xFF);
    crc[1] = (BYTE) ((wCrc >> 8) & 0xFF);
  }

  return wCrc;
}

/**f* SpringProx.API/SPROX_ComputeIso14443BCrc
 *
 * NAME
 *   SPROX_ComputeIso14443BCrc
 *
 * DESCRIPTION
 *   Compute a CRC according to ISO/IEC 14443-B (layer 3)
 *
 * INPUTS
 *   BYTE crc[2]         : 2-byte buffers to receive the CRC
 *                         (can be set to NULL since the CRC is also returned as
 *                         function result)
 *   const BYTE buffer[] : data to be checksumed
 *   WORD size           : length of the data
 *
 * RETURNS
 *   The computed CRC.
 *
 **/
SPRINGPROX_LIB WORD SPRINGPROX_API SPROX_ComputeIso14443BCrc(BYTE crc[2], const BYTE buffer[], WORD size)
{
  BYTE chBlock;
  WORD wCrc;
  BYTE *p = (BYTE *) buffer;  

  wCrc = 0xFFFF;                /* ISO/IEC 13239 (formerly ISO/IEC 3309) */

  do
  {
    chBlock = *p++;
    UpdateCrc(chBlock, &wCrc);
  } while (--size);

  wCrc = ~wCrc;                 /* ISO/IEC 13239 (formerly ISO/IEC 3309) */

  if (crc != NULL)
  {
    crc[0] = (BYTE) (wCrc & 0xFF);
    crc[1] = (BYTE) ((wCrc >> 8) & 0xFF);
  }

  return wCrc;
}

#endif
