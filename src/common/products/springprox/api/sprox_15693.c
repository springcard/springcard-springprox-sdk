/**h* SpringProx.API/ISO15693
 *
 * NAME
 *   SpringProx.API :: ISO 15693
 *
 * DESCRIPTION
 *   Implementation of ISO/IEC 15693 (and ICODE1)
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
 
  LTX 26/02/2008 : created
  JDA 28/02/2012 : bugfix, changed a few function and parameter names for
                   more readability and coherence, but there's still a 
                   lot of things to be done...
  JDA 22/04/2014 : added SPROX_Iso15693_Exchange and related
  JIZ 12/12/2014 : added AFI support on Iso15693_SelectAny + bugfix on 
                   Iso15693_Exchange and Iso15693_ExchangeStdCommand
  
*/
#include "sprox_api_i.h"

#include "micore_picc.h"

SWORD Save15693Snr(SPROX_PARAM  BYTE snr[8])
{
  SPROX_PARAM_TO_CTX;
  SPROX_Trace(TRACE_DEBUG, "15693Snr <- %02X%02X%02X%02X%02X%02X%02X%02X", snr[0], snr[1], snr[2], snr[3], snr[4], snr[5], snr[6], snr[7]);

  if (snr != sprox_ctx->iso15693_snr)
  {
    memset(sprox_ctx->iso15693_snr, 0, sizeof(sprox_ctx->iso15693_snr));
    if (snr != NULL)
      memcpy(sprox_ctx->iso15693_snr, snr, 8);
  }
  return MI_OK;
}

/**f* SpringProx.API/SPROX_Iso15693_SelectAny
 *
 * NAME
 *   SPROX_Iso15693_SelectAny
 *
 * DESCRIPTION
 *   Select "Any" ISO 15693 card available in the RF field
 *
 * INPUTS
 *   BYTE afi           : application family identifier. Set to 0.
 *   BYTE snr[8]        : 8-byte buffer to receive card's Unique ID
 *
 * RETURNS
 *   MI_OK              : success, card selected
 *   MI_NOTAGERR        : no card available in the RF field
 *   Other code if internal or communication error has occured. 
 *
 *
 * SEE ALSO
 *   SPROX_Iso15693_SelectAgain
 *   SPROX_Iso15693_Halt
 *
 **/
SPROX_API_FUNC(Iso15693_SelectAny) (SPROX_PARAM  BYTE afi, BYTE snr[8])
{
  SWORD rc;
  BYTE  buffer[8], afi_buf[1];
  WORD  len = sizeof(buffer);
  SPROX_PARAM_TO_CTX;  

  afi_buf[0] = afi;
  
  if (snr == NULL) return MI_LIB_CALL_ERROR;
  
  rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_15693);
  if (rc != MI_OK) return rc;

  rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_SELECT_ANY, afi_buf, sizeof(afi_buf), buffer, &len);
  if (rc == MI_OK)
  {
    Save15693Snr(SPROX_PARAM_P  &buffer[0]);

    if (snr != NULL)
      memcpy(snr, &buffer[0], 8);
  }

  if (rc == MI_OK)
    SPROX_Trace(TRACE_DEBUG, "15693_SelectAny %02X%02X%02X%02X%02X%02X%02X%02X -> %d", snr[0], snr[1], snr[2], snr[3], snr[4], snr[5], snr[6], snr[7], rc);

  return rc; 
}

/**f* SpringProx.API/SPROX_Iso15693_SelectAgain
 *
 * NAME
 *   SPROX_Iso15693_SelectAgain
 *
 * DESCRIPTION
 *   This function is deprecated
 *
 **/
SPROX_API_FUNC(Iso15693_SelectAgain) (SPROX_PARAM  BYTE snr[8])
{
  SWORD rc;
  SPROX_PARAM_TO_CTX;

  rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_15693);
  if (rc != MI_OK) return rc;
  
  if (snr == NULL)
    snr = sprox_ctx->iso15693_snr;
     
  rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_SELECT_AGAIN, snr, 8, NULL, NULL);
  
  if ((rc == MI_OK) && (snr != sprox_ctx->iso15693_snr))
    Save15693Snr(SPROX_PARAM_P  snr);

  if (rc == MI_OK)
    SPROX_Trace(TRACE_DEBUG, "15693_SelectAgain %04X%04X%04X%04X%04X%04X%04X%04X -> %d", snr[0], snr[1], snr[2], snr[3], snr[4], snr[5], snr[6], snr[7], rc);

  return rc; 
}

/**f* SpringProx.API/SPROX_Iso15693_Halt
 *
 * NAME
 *   SPROX_Iso15693_Halt
 *
 * DESCRIPTION
 *   This function is deprecated
 *
 **/
SPROX_API_FUNC(Iso15693_Halt) (SPROX_PARAM_V)
{
  SWORD rc;
  SPROX_PARAM_TO_CTX;
  
  rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_15693);
  if (rc != MI_OK) return rc;
  
  rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_HALT, NULL, 0, NULL, NULL);  
  return rc;
}

/*
 *****************************************************************************
 *
 *   READ/WRITE FUNCTIONS
 *
 *****************************************************************************
 */

static SWORD Iso15693_ReadProc(SPROX_PARAM  BOOL extended, BYTE snr[8], WORD addr, WORD count, BYTE data[], WORD *datalen)
{
  BYTE buffer[8];
  WORD len = 0;
  BYTE opcode = extended ? SPROX_CSB_READ_EXT : SPROX_CSB_READ;  
  SWORD rc;

  rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_15693);
  if (rc != MI_OK) return rc;

  rc = MI_LIB_CALL_ERROR;
  if (data == NULL)
    goto exit_proc;

  if (snr != NULL)
  {
    rc = SPROX_API_CALL(Iso15693_SelectAgain) (SPROX_PARAM_P  snr);
    if (rc != MI_OK)
      goto exit_proc;
  }
    
  if (extended)
  {
    buffer[len++] = (BYTE) (addr / 0x0100);
    buffer[len++] = (BYTE) (addr % 0x0100);
    buffer[len++] = (BYTE) (count / 0x0100);
    buffer[len++] = (BYTE) (count % 0x0100);      
  } else
  {
    buffer[len++] = (BYTE) addr;
    buffer[len++] = (BYTE) count;
  }
    
  rc = SPROX_DLG_FUNC(SPROX_PARAM_P  opcode, buffer, len, data, datalen);

exit_proc:
  if (snr != NULL)
    SPROX_Trace(TRACE_DEBUG, "15693Read %d,%d %04X%04X%04X%04X%04X%04X%04X%04X -> %d", addr, count, snr[0], snr[1], snr[2], snr[3], snr[4], snr[5], snr[6], snr[7], rc);
  else
    SPROX_Trace(TRACE_DEBUG, "15693Read %d,%d -> %d", addr, count, rc);

  return rc;
}

static SWORD Iso15693_WriteProc(SPROX_PARAM  BOOL extended, BYTE snr[8], WORD addr, WORD count, const BYTE data[], WORD datalen)
{
  BYTE buffer[256];
  WORD len = 0;
  BYTE opcode = extended ? SPROX_CSB_WRITE_EXT : SPROX_CSB_WRITE;  
  SWORD rc;

  rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_15693);
  if (rc != MI_OK) return rc;

  rc = MI_LIB_CALL_ERROR;
  if (data == NULL)
    goto exit_proc;

  if (snr != NULL)
  {
    rc = SPROX_API_CALL(Iso15693_SelectAgain) (SPROX_PARAM_P  snr);
    if (rc != MI_OK)
      goto exit_proc;
  }

  if (extended)
  {
    buffer[len++] = (BYTE) (addr / 0x0100);
    buffer[len++] = (BYTE) (addr % 0x0100);
    buffer[len++] = (BYTE) (count / 0x0100);
    buffer[len++] = (BYTE) (count % 0x0100);      
  } else
  {
    buffer[len++] = (BYTE) addr;
    buffer[len++] = (BYTE) count;
  }
    
  if ((len + datalen) > sizeof(buffer))
  {
    rc = MI_LIB_CALL_ERROR;
    goto exit_proc;
  }
    
  memcpy(&buffer[len], data, datalen);
  len += datalen;
  rc = SPROX_DLG_FUNC(SPROX_PARAM_P  opcode, buffer, len, NULL, NULL);

exit_proc:
  if (snr != NULL)
    SPROX_Trace(TRACE_DEBUG, "15693Write %d,%d %04X%04X%04X%04X%04X%04X%04X%04X -> %d", addr, count, snr[0], snr[1], snr[2], snr[3], snr[4], snr[5], snr[6], snr[7], rc);
  else
    SPROX_Trace(TRACE_DEBUG, "15693Write %d,%d -> %d", addr, count, rc);

  return rc;
}

/**f* SpringProx.API/SPROX_Iso15693_ReadSingleBlock
 *
 * NAME
 *   SPROX_Iso15693_ReadSingleBlock
 *
 * DESCRIPTION
 *   Read one block of a ISO 15693 tag.
 *
 * INPUTS
 *   const BYTE snr[8]   : 8-byte UID of the ISO 15693 card to read
 *                         If NULL, the reader will work with currently selected tag
 *   BYTE addr           : address of the block to read
 *   BYTE data[]         : buffer to receive the data
 *   WORD *datalen       : length of the receive data
 *
 * WARNING
 *   The number of blocks available on a ISO 15693 tag depends on the tag type.
 *
 *   For example, a ISO 15693 ICODE-SLI tag stores 28 blocks (address 0 to 27), a
 *   ISO 15693 ICODE-SLI-L tag stores 8 blocks (address 0 to 7), a ISO 15693 Tag-It
 *   Plus Inlay tag stores 64 blocks (address 0 to 63).
 *
 *   For more details, please refer to specific tag documentation.
 *
 * RETURNS
 *   MI_OK              : success, data have been read
 *   MI_NOTAGERR        : the required tag is not available in the RF field
 *   Other code if internal or communication error has occured. 
 *
 * SEE ALSO
 *   SPROX_Iso15693_ReadMultipleBlocks
 *   SPROX_Iso15693_ExtendedReadSingleBlock
 *   SPROX_Iso15693_ExtendedReadMultipleBlocks
 *
 **/
SPROX_API_FUNC(Iso15693_ReadSingleBlock) (SPROX_PARAM  BYTE snr[8], BYTE addr, BYTE data[], WORD *datalen)
{
  return Iso15693_ReadProc(SPROX_PARAM_P  FALSE, snr, addr, 0, data, datalen);
}

/**f* SpringProx.API/SPROX_Iso15693_ExtendedReadSingleBlock
 *
 * NAME
 *   SPROX_Iso15693_ExtendedReadSingleBlock
 *
 * DESCRIPTION
 *   Read one block of a ISO 15693 tag, with the address on 2 bytes.
 *
 * INPUTS
 *   const BYTE snr[8]   : 8-byte UID of the ISO 15693 card to read
 *                         If NULL, the reader will work with currently selected tag
 *   WORD addr           : address of the block to read
 *   BYTE data[]         : buffer to receive the data
 *   WORD *datalen       : length of the receive data
 *
 * RETURNS
 *   MI_OK              : success, data have been read
 *   MI_NOTAGERR        : the required tag is not available in the RF field
 *   Other code if internal or communication error has occured. 
 *
 * SEE ALSO
 *   SPROX_Iso15693_ReadSingleBlock 
 *   SPROX_Iso15693_ReadMultipleBlocks
 *   SPROX_Iso15693_ExtendedReadMultipleBlocks
 *
 **/
SPROX_API_FUNC(Iso15693_ExtendedReadSingleBlock) (SPROX_PARAM  BYTE snr[8], WORD addr, BYTE data[], WORD *datalen)
{
  return Iso15693_ReadProc(SPROX_PARAM_P  TRUE, snr, addr, 0, data, datalen);
}

/**f* SpringProx.API/SPROX_Iso15693_ReadMultipleBlocks
 *
 * NAME
 *   SPROX_Iso15693_ReadMultipleBlocks
 *
 * DESCRIPTION
 *   Read multiple blocks of a ISO 15693 tag, using the "read multiple blocks" command.
 *
 * INPUTS
 *   const BYTE snr[8]   : 8-byte UID of the ISO 15693 card to read
 *                         If NULL, the reader will work with currently selected tag
 *   BYTE addr           : address of the first block to read
 *   BYTE count          : number of blocks to read
 *   BYTE data[]         : buffer to receive the data
 *   WORD *datalen       : length of the receive data
 *
 * WARNING
 *   The number of blocks available on a ISO 15693 tag depends on the tag type.
 *
 *   For example, a ISO 15693 ICODE-SLI tag stores 28 blocks (address 0 to 27), a
 *   ISO 15693 ICODE-SLI-L tag stores 8 blocks (address 0 to 7), a ISO 15693 Tag-It
 *   Plus Inlay tag stores 64 blocks (address 0 to 63).
 *
 *   For more details, please refer to specific tag documentation.
 *
 * RETURNS
 *   MI_OK              : success, data have been read
 *   MI_NOTAGERR        : the required tag is not available in the RF field
 *   Other code if internal or communication error has occured. 
 *
 * SEE ALSO
 *   SPROX_Iso15693_ReadSingleBlock
 *   SPROX_Iso15693_ExtendedReadSingleBlock
 *   SPROX_Iso15693_ExtendedReadMultipleBlocks
 *
 **/
SPROX_API_FUNC(Iso15693_ReadMultipleBlocks) (SPROX_PARAM  BYTE snr[8], BYTE addr, BYTE count, BYTE data[], WORD *datalen)
{
  if (count == 0) return MI_LIB_CALL_ERROR;
  return Iso15693_ReadProc(SPROX_PARAM_P  FALSE, snr, addr, count, data, datalen);
}

/**f* SpringProx.API/SPROX_Iso15693_ExtendedReadMultipleBlocks
 *
 * NAME
 *   SPROX_Iso15693_ExtendedReadMultipleBlocks
 *
 * DESCRIPTION
 *   Read multiple blocks of a ISO 15693 tag, using the "read multiple blocks" command,
 *   with the address and count and 2 bytes.
 *
 * INPUTS
 *   const BYTE snr[8]   : 8-byte UID of the ISO 15693 card to read
 *                         If NULL, the reader will work with currently selected tag
 *   WORD addr           : address of the first block to read
 *   WORD count          : number of blocks to read
 *   BYTE data[]         : buffer to receive the data
 *   WORD *datalen       : length of the receive data
 *
 * RETURNS
 *   MI_OK              : success, data have been read
 *   MI_NOTAGERR        : the required tag is not available in the RF field
 *   Other code if internal or communication error has occured. 
 *
 * SEE ALSO
 *   SPROX_Iso15693_ExtendedReadSingleBlock
 *   SPROX_Iso15693_ReadSingleBlock 
 *   SPROX_Iso15693_ReadMultipleBlocks
 *
 **/
SPROX_API_FUNC(Iso15693_ExtendedReadMultipleBlocks) (SPROX_PARAM  BYTE snr[8], WORD addr, WORD count, BYTE data[], WORD *datalen)
{
  if (count == 0) return MI_LIB_CALL_ERROR;
  return Iso15693_ReadProc(SPROX_PARAM_P  TRUE, snr, addr, count, data, datalen);
}

/**f* SpringProx.API/SPROX_Iso15693_WriteSingleBlock
 *
 * NAME
 *   SPROX_Iso15693_WriteSingleBlock
 *
 * DESCRIPTION
 *   Write bytes in a block of a ISO 15693 tag.
 *
 * INPUTS
 *   const BYTE snr[8]  : 8-byte UID of the ISO 15693 card to read
 *                        If NULL, the reader will work with currently selected tag
 *   BYTE addr          : address of the block to write
 *   BYTE data[]        : buffer of data
 *   WORD datalen       : length of data
 *
 * RETURNS
 *   MI_OK              : success, data have been written
 *   MI_NOTAGERR        : the required tag is not available in the RF field
 *   Other code if internal or communication error has occured. 
 *
 * SEE ALSO
 *   SPROX_Iso15693_WriteMultipleBlocks
 *   SPROX_Iso15693_ExtendedWriteSingleBlock
 *   SPROX_Iso15693_ExtendedWriteMultipleBlocks
 *
 **/
SPROX_API_FUNC(Iso15693_WriteSingleBlock) (SPROX_PARAM  BYTE snr[8], BYTE addr, const BYTE data[], WORD datalen)
{
  return Iso15693_WriteProc(SPROX_PARAM_P  FALSE, snr, addr, 0, data, datalen);
}

/**f* SpringProx.API/SPROX_Iso15693_WriteMultipleBlocks
 *
 * NAME
 *   SPROX_Iso15693_WriteMultipleBlocks
 *
 * DESCRIPTION
 *   Write bytes in block(s) of a ISO 15693 tag.
 *
 * INPUTS
 *   const BYTE snr[8]  : 8-byte UID of the ISO 15693 card to read
 *                        If NULL, the reader will work with currently selected tag
 *   BYTE addr          : address of the 1st block to write
 *   BYTE count         : number of block(s) to write
 *   BYTE data[]        : buffer of data
 *   WORD datalen       : length of data
 *
 * RETURNS
 *   MI_OK              : success, data have been written
 *   MI_NOTAGERR        : the required tag is not available in the RF field
 *   Other code if internal or communication error has occured. 
 *
 * SEE ALSO
 *   SPROX_Iso15693_WriteSingleBlocks
 *   SPROX_Iso15693_ExtendedWriteSingleBlock
 *   SPROX_Iso15693_ExtendedWriteMultipleBlocks
 *
 **/
SPROX_API_FUNC(Iso15693_WriteMultipleBlocks) (SPROX_PARAM  BYTE snr[8], BYTE addr, BYTE count, const BYTE data[], WORD datalen)
{
  if (count == 0) return MI_LIB_CALL_ERROR;
  return Iso15693_WriteProc(SPROX_PARAM_P  FALSE, snr, addr, count, data, datalen);
}

/**f* SpringProx.API/SPROX_Iso15693_ExtendedWriteSingleBlock
 *
 * NAME
 *   SPROX_Iso15693_ExtendedWriteSingleBlock
 *
 * DESCRIPTION
 *   Write bytes in a block of a ISO 15693 tag. The address is on 2B.
 *
 * INPUTS
 *   const BYTE snr[8]  : 8-byte UID of the ISO 15693 card to read
 *                        If NULL, the reader will work with currently selected tag
 *   WORD addr          : address of the block to write
 *   BYTE data[]        : buffer of data
 *   WORD datalen       : length of data
 *
 * RETURNS
 *   MI_OK              : success, data have been written
 *   MI_NOTAGERR        : the required tag is not available in the RF field
 *   Other code if internal or communication error has occured. 
 *
 * SEE ALSO
 *   SPROX_Iso15693_WriteSingleBlock 
 *   SPROX_Iso15693_WriteMultipleBlocks
 *   SPROX_Iso15693_ExtendedWriteMultipleBlocks
 *
 **/
SPROX_API_FUNC(Iso15693_ExtendedWriteSingleBlock) (SPROX_PARAM  BYTE snr[8], WORD addr, const BYTE data[], WORD datalen)
{
  return Iso15693_WriteProc(SPROX_PARAM_P  TRUE, snr, addr, 0, data, datalen);
}

/**f* SpringProx.API/SPROX_Iso15693_WriteMultipleBlocks
 *
 * NAME
 *   SPROX_Iso15693_WriteMultipleBlocks
 *
 * DESCRIPTION
 *   Write bytes in block(s) of a ISO 15693 tag. Address and count are on 2B.
 *
 * INPUTS
 *   const BYTE snr[8]  : 8-byte UID of the ISO 15693 card to read
 *                        If NULL, the reader will work with currently selected tag
 *   WORD addr          : address of the 1st block to write
 *   WORD count         : number of block(s) to write
 *   BYTE data[]        : buffer of data
 *   WORD datalen       : length of data
 *
 * RETURNS
 *   MI_OK              : success, data have been written
 *   MI_NOTAGERR        : the required tag is not available in the RF field
 *   Other code if internal or communication error has occured. 
 *
 * SEE ALSO
 *   SPROX_Iso15693_WriteSingleBlock
 *   SPROX_Iso15693_WriteMultipleBlocks
 *   SPROX_Iso15693_ExtendedWriteMultipleBlocks
 *
 **/
SPROX_API_FUNC(Iso15693_ExtendedWriteMultipleBlocks) (SPROX_PARAM  BYTE snr[8], WORD addr, WORD count, const BYTE data[], WORD datalen)
{
  if (count == 0) return MI_LIB_CALL_ERROR;
  return Iso15693_WriteProc(SPROX_PARAM_P  TRUE, snr, addr, count, data, datalen);
}

/*
 *****************************************************************************
 *
 *   LOCK FUNCTIONS
 *
 *****************************************************************************
 */

static SWORD Iso15693_LockProc(SPROX_PARAM  BOOL extended, BYTE snr[8], WORD addr)
{
  SWORD rc;
  BYTE buffer[8];
  WORD len = 0;

  rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_15693);
  if (rc != MI_OK) return rc;

  if (snr != NULL)
  {
    rc = SPROX_API_CALL(Iso15693_SelectAgain) (SPROX_PARAM_P  snr);
    if (rc != MI_OK)
      goto exit_proc;
  }

  if (extended)
  {
    buffer[len++] = (BYTE) (addr / 0x0100);
    buffer[len++] = (BYTE) (addr % 0x0100);
  } else
  {
    buffer[len++] = (BYTE) addr;
  }

  rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_LOCK, buffer, len, NULL, NULL);

exit_proc:
  if (snr != NULL)
    SPROX_Trace(TRACE_DEBUG, "15693_LockBlock %d %04X%04X%04X%04X%04X%04X%04X%04X -> %d", addr, snr[0], snr[1], snr[2], snr[3], snr[4], snr[5], snr[6], snr[7], rc);
  else
    SPROX_Trace(TRACE_DEBUG, "15693_LockBlock %d -> %d", addr, rc);

  return rc;
}

/**f* SpringProx.API/SPROX_Iso15693_LockBlock
 *
 * NAME
 *   SPROX_Iso15693_LockBlock
 *
 * DESCRIPTION
 *   Lock permanently one 4-byte block of a ISO 15693 tag.
 *
 * INPUTS
 *   const BYTE snr[8]  : 8-byte UID of the ISO 15693 card to read
 *                        If NULL, the reader will work with currently selected tag
 *   BYTE addr          : address of the block to write
 *
 * WARNING
 *   When a block is locked, it will be imposssible to change the value of block again.
 *
 *   For more details, please refer to specific tag documentation.
 *
 * RETURNS
 *   MI_OK              : success, data have been written
 *   MI_NOTAGERR        : the required tag is not available in the RF field
 *   Other code if internal or communication error has occured. 
 *
 * SEE ALSO
 *   SPROX_15693_IsoWriteBlock
 *   SPROX_15693_IsoReadBlock
 *
 **/
SPROX_API_FUNC(Iso15693_LockBlock) (SPROX_PARAM  BYTE snr[8], BYTE addr)
{
  return Iso15693_LockProc(SPROX_PARAM_P  FALSE, snr, addr);
}

SPROX_API_FUNC(Iso15693_ExtendedLockBlock) (SPROX_PARAM  BYTE snr[8], WORD addr)
{
  return Iso15693_LockProc(SPROX_PARAM_P  TRUE, snr, addr);
}

/*
 *****************************************************************************
 *
 *   GET SYSTEM INFORMATION FUNCTION
 *
 *****************************************************************************
 */

/**f* SpringProxINS/Iso15693_GetSystemInformation
 *
 * SYNOPSIS
 *   SBYTE Iso15693_GetSystemInformation(BYTE *data, WORD *datalen)
 *
 * DESCRIPTION
 *   Get system information in a ISO 15693 tag.
 *   The target card is the currently selected one (which params are loaded
 *   into vicc_tag).
 *
 * INPUTS
 *   const BYTE snr[8]  : 8-byte UID of the ISO 15693 card to read
 *                         If NULL, the reader will work with currently selected tag
 *   BYTE *data         : data buffer (out)
 *   WORD *datalen      : length of data buffer (in or out)
 *
 * WARNING
 *   For more details, please refer to specific tag documentation.
 *
 * RETURNS
 *   MI_OK              : success, data have been read
 *   MI_NOTAGERR        : the required tag is not available in the RF field
 *   Other code if internal or communication error has occured. 
 *
 * SEE ALSO
 *   SPROX_Iso15693_ReadMultipleBlocks
 *   SPROX_Iso15693_ReadMultipleBytes
 *   SPROX_Iso15693_WriteBlock
 *   SPROX_Iso15693_LockBlock
 *
 **/
SPROX_API_FUNC(Iso15693_GetSystemInformation) (SPROX_PARAM  BYTE snr[8], BYTE data[], WORD *datalen)
{
  BYTE    recv_data[SPROX_FRAME_CONTENT_SIZE];
	WORD    recv_len = sizeof(recv_data);
  SWORD   rc;
  
  rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_15693);
  if (rc != MI_OK) return rc;
  
  rc = MI_LIB_CALL_ERROR;

  if (snr != NULL)
  {
    rc = SPROX_API_CALL(Iso15693_SelectAgain) (SPROX_PARAM_P  snr);
    if (rc != MI_OK)
      goto exit_proc;
  }
  
  rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_GET_SYSTEM_INFO, NULL, 0, recv_data, &recv_len);

exit_proc:
  if (snr != NULL)
    SPROX_Trace(TRACE_DEBUG, "Iso15693_GetSystemInformation %04X%04X%04X%04X%04X%04X%04X%04X -> %d", snr[0], snr[1], snr[2], snr[3], snr[4], snr[5], snr[6], snr[7], rc);
  else
    SPROX_Trace(TRACE_DEBUG, "Iso15693_GetSystemInformation -> %d", rc);
    
  if (rc == MI_OK)
  {
    if (datalen != NULL)
    {
      if ((*datalen != 0) && (*datalen < recv_len))
        return MI_RESPONSE_OVERFLOW;        
      *datalen = recv_len;      
    }
    
    if (data != NULL)
      memcpy(data, recv_data, recv_len);
  }
    
  return rc;
}
																								 
SPROX_API_FUNC(Iso15693_Exchange) (SPROX_PARAM  const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD *recv_len, WORD timeout)
{
  BYTE    send_buffer_header[SPROX_FRAME_CONTENT_SIZE];
  BYTE    recv_data[SPROX_FRAME_CONTENT_SIZE];
	WORD    len;
  SWORD   rc;
  
  rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_15693);
	
	if ((rc != MI_OK) && (rc != MI_UNKNOWN_FUNCTION)) return rc;
	
  if (send_buffer == NULL) return MI_LIB_CALL_ERROR;
	
  if (recv_buffer == NULL) return MI_LIB_CALL_ERROR;
  
	if (recv_len == NULL) return MI_LIB_CALL_ERROR;
	
  if ((send_len + 9) >= SPROX_FRAME_CONTENT_SIZE) return MI_COMMAND_OVERFLOW;
	
  len = 2 + *recv_len;
	
  if (len >= SPROX_FRAME_CONTENT_SIZE) len = SPROX_FRAME_CONTENT_SIZE;
  
  send_buffer_header[0] = (BYTE) ((send_len + 2) / 0x0100);
  send_buffer_header[1] = (BYTE) ((send_len + 2) % 0x0100);
  send_buffer_header[2] = (BYTE) (*recv_len / 0x0100);
  send_buffer_header[3] = (BYTE) (*recv_len % 0x0100);
  send_buffer_header[4] = TRUE;
  send_buffer_header[5] = (BYTE) (timeout / 0x0100);
  send_buffer_header[6] = (BYTE) (timeout % 0x0100);
	
  memcpy(&send_buffer_header[7], send_buffer, send_len);
	
	send_buffer_header[send_len+7] = 0xCC;
	send_buffer_header[send_len+8] = 0xCC;
	  
  rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_PICCEXCHANGEBLOCK_V, send_buffer_header, (WORD) (send_len + 7 + 2), recv_data, &len);

  if (rc == MI_OK)
  {
    *recv_len = recv_data[0] * 0x0100 + recv_data[1];
		
    if (*recv_len > (len - 2))
			return MI_RESPONSE_INVALID;
    	
		memcpy(recv_buffer, &recv_data[2], *recv_len);
  } 
	
  return rc;
}

SPROX_API_FUNC(Iso15693_ExchangeStdCommand) (SPROX_PARAM  BOOL opt_flag, BYTE snr[8], BYTE cmd_opcode, const BYTE cmd_params[], WORD cmd_params_len, BYTE recv_buffer[], WORD *recv_len, WORD timeout)
{
  BYTE send_buffer[SPROX_FRAME_CONTENT_SIZE-8];
  WORD send_len = 0;
  WORD i;
  SPROX_PARAM_TO_CTX;
  
  if (snr == NULL)
    snr = sprox_ctx->iso15693_snr;
  
  if ((cmd_params == NULL) && (cmd_params_len != 0))
    return MI_LIB_CALL_ERROR;
    
  if ((cmd_params_len + 8 + 1) > sizeof(send_buffer))
    return MI_COMMAND_OVERFLOW;
  
  send_buffer[send_len++] = opt_flag ? 0x62 : 0x22;
  send_buffer[send_len++] = cmd_opcode;
  
  for (i=0; i<8; i++)
    send_buffer[send_len++] = snr[7-i]; 

  if (cmd_params != NULL)
    for (i=0; i<cmd_params_len; i++)
      send_buffer[send_len++] = cmd_params[i];
  
  return SPROX_API_CALL(Iso15693_Exchange) (SPROX_PARAM_P  send_buffer, send_len, recv_buffer, recv_len, timeout);
}

SPROX_API_FUNC(Iso15693_ExchangeCustomCommand) (SPROX_PARAM  BOOL opt_flag, BYTE mfg_id, BYTE snr[8], BYTE cmd_opcode, const BYTE cmd_params[], WORD cmd_params_len, BYTE recv_buffer[], WORD *recv_len, WORD timeout)
{
  BYTE send_buffer[SPROX_FRAME_CONTENT_SIZE-8];
  WORD send_len = 0;
  WORD i;
  SPROX_PARAM_TO_CTX;
  
  if (snr == NULL)
    snr = sprox_ctx->iso15693_snr;
  
  if ((cmd_params_len + 8 + 1 + 1) > sizeof(send_buffer))
    return MI_COMMAND_OVERFLOW;
  
  send_buffer[send_len++] = opt_flag ? 0x62 : 0x22;
	send_buffer[send_len++] = cmd_opcode;
  send_buffer[send_len++] = mfg_id;
	
  for (i=0; i<8; i++)
    send_buffer[send_len++] = snr[7-i];
		
  if (cmd_params != NULL)
	{
    for (i=0; i<cmd_params_len; i++)
      send_buffer[send_len++] = cmd_params[i];
  }
	
  return SPROX_API_CALL(Iso15693_Exchange) (SPROX_PARAM_P  send_buffer, send_len, recv_buffer, recv_len, timeout);
}

/*
 *
 * ICODE1
 *
 */

SWORD SaveI1Snr(SPROX_PARAM  BYTE snr[8])
{
  SPROX_PARAM_TO_CTX;
  SPROX_Trace(TRACE_DEBUG, "15693Snr <- %02X%02X%02X%02X%02X%02X%02X%02X", snr[0], snr[1], snr[2], snr[3], snr[4], snr[5], snr[6], snr[7]);

  if (snr != sprox_ctx->i1_snr)
  {
    memset(sprox_ctx->i1_snr, 0, sizeof(sprox_ctx->i1_snr));
    if (snr != NULL)
      memcpy(sprox_ctx->i1_snr, snr, 8);
  }
  return MI_OK;
}

/**f* SpringProx.API/SPROX_ICode1_SelectAny
 *
 * NAME
 *   SPROX_ICode1_SelectAny
 *
 * DESCRIPTION
 *   Select "Any" ICODE1 card available in the RF field
 *
 * INPUTS
 *   BYTE afi           : application family identifier. Set to 0.
 *   BYTE snr[8]        : 8-byte buffer to receive card's Unique ID
 *
 * RETURNS
 *   MI_OK              : success, card selected
 *   MI_NOTAGERR        : no card available in the RF field
 *   Other code if internal or communication error has occured. 
 *
 * SEE ALSO
 *   SPROX_ICode1_UnselectedRead
 *
 **/
SPROX_API_FUNC(ICode1_SelectAny) (SPROX_PARAM  BYTE afi, BYTE snr[8])
{
  SWORD rc;
  BYTE  buffer[8];
  WORD  len = sizeof(buffer);
  SPROX_PARAM_TO_CTX;  

  if (snr == NULL) return MI_LIB_CALL_ERROR;
  
  rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ICODE1);
  if (rc != MI_OK) return rc;

  rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_SELECT_ANY, NULL, 0, buffer, &len);
  if (rc == MI_OK)
  {
    SaveI1Snr(SPROX_PARAM_P  &buffer[0]);

    if (snr != NULL)
      memcpy(snr, &buffer[0], 8);
  }

  if (rc == MI_OK)
    SPROX_Trace(TRACE_DEBUG, "I1_SelectAny %02X%02X%02X%02X%02X%02X%02X%02X -> %d", snr[0], snr[1], snr[2], snr[3], snr[4], snr[5], snr[6], snr[7], rc);

  return rc; 
}

SPROX_API_FUNC(I1_SelectAny) (SPROX_PARAM  BYTE afi, BYTE snr[8])
{
  return SPROX_API_CALL(ICode1_SelectAny) (SPROX_PARAM_P  afi, snr);
}

SPROX_API_FUNC(I1_Halt) (SPROX_PARAM_V)
{
  SWORD rc;
  SPROX_PARAM_TO_CTX;
  
  rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ICODE1);
  if (rc != MI_OK) return rc;
  
  rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_HALT, NULL, 0, NULL, NULL);  
  return rc;
}

/**f* SpringProx.API/SPROX_ICode1_UnselectedRead
 *
 * NAME
 *   SPROX_ICode1_UnselectedRead
 *
 * DESCRIPTION
 *   Read multiple blocks of a ICODE1 tag.
 *
 * INPUTS
 *   BYTE addr           : address of the first block to read
 *   BYTE count          : number of blocks to read
 *   const BYTE *data    : buffer to receive the data
 *   const WORD *datalen : length of the receive data
 *
 * RETURNS
 *   MI_OK              : success, data have been read
 *   MI_NOTAGERR        : the required tag is not available in the RF field
 *   Other code if internal or communication error has occured. 
 *
 * SEE ALSO
 *   SPROX_ICode1_SelectAny
 *
 **/
SPROX_API_FUNC(ICode1_UnselectedRead) (SPROX_PARAM  BYTE addr, BYTE count, BYTE data[], WORD *datalen)
{
  SWORD rc;
  BYTE buffer[2];
  
  rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ICODE1);
  if (rc != MI_OK) return rc;

  buffer[0] = addr;
  buffer[1] = count;      
  
  rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_READ, buffer, 2, data, datalen);
  return rc;
}

 
SPROX_API_FUNC(I1_Read) (SPROX_PARAM  BYTE addr, BYTE count, BYTE data[], WORD *datalen)
{
  return SPROX_API_CALL(ICode1_UnselectedRead) (SPROX_PARAM_P  addr, count, data, datalen);
}

SPROX_API_FUNC(I1_Write) (SPROX_PARAM BYTE addr, BYTE data[4])
{
  return MI_UNKNOWN_FUNCTION;
}
