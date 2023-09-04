/**h* SpringProx.API/Mifare
 *
 * NAME
 *   SpringProxAPI :: Mifare
 *
 * DESCRIPTION
 *   Implementation of Philips Mifare functions
 *   (Mifare Standard and Mifare UltraLite)
 *
 **/

/*

  SpringProx API
  --------------

  Copyright (c) 2000-2008 SpringCard - www.springcard.com

  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  History
  -------
  
  JDA 15/02/2003 : created from CSB-3's OCX source code
  JDA 16/05/2003 : added SPROX_MifStUpdateAccessBlock and related stuff as helper functions
  JDA 10/07/2003 : corrected an error in SPROX_MifStDecrementCounter (didn't work with supplied key)
  JDA 13/11/2003 : corrected an error in read/write sectors functions for 4k cards -> 1.13
  JDA 25/11/2003 : still a side effect of the same bug -> 1.14
  JDA 03/02/2004 : added SPROX_MifStSelectIdle and SPROX_MifStHalt
	JDA 24/01/2011 : replace references to SPROX_RAM_KEY_COUNT and MICORE_EEPROM_KEY_COUNT by veriables as
	                 they are no longer constants
  JDA 05/02/2013 : added MifStIncrementCounter and MifStRestoreCounter
                   new implementation of MifStDecrementCounter for FW >= 1.71

*/
#include "sprox_api_i.h"

#ifndef SPROX_API_NO_MIF

#include "micore_picc.h"

BYTE sprox_ram_key_count = 4;  /*  4 A and  4 B */
BYTE sprox_e2_key_count  = 16; /* 16 A and 16 B */

SPROX_API_FUNC(MifStRequest) (SPROX_PARAM  BYTE req_code, BYTE atq[2])
{
  return SPROX_API_CALL(A_Request)  (SPROX_PARAM_P  req_code, atq);
}

SPROX_API_FUNC(MifStRequestAny) (SPROX_PARAM  BYTE atq[2])
{
  return SPROX_API_CALL(A_RequestAny)  (SPROX_PARAM_P  atq);
}

SPROX_API_FUNC(MifStRequestIdle) (SPROX_PARAM  BYTE atq[2])
{
  return SPROX_API_CALL(A_RequestIdle)  (SPROX_PARAM_P  atq);
}

/**f* SpringProx.API/SPROX_MifStSelectAny
 *
 * NAME
 *   SPROX_MifStSelectAny
 *
 * WARNING
 *   This function is deprecated. Use SPROX_A_SelectAny instead.
 *
 * INPUTS
 *   BYTE snr[4]        : 4-byte buffer to receive card's Unique ID
 *   BYTE atq[2]        : 2-byte buffer to receive card's Answer To Query
 *   BYTE sak[1]        : 1-byte buffer to receive card's Select AcKnowledge
 *
 * RETURNS
 *   MI_OK              : success, card selected
 *   MI_NOTAGERR        : no card available in the RF field
 *   Other code if internal or communication error has occured. 
 *
 * NOTES
 *   Despite the name of the function, it doesn't check the returned card type
 *   (i.e., any ISO 14443-A card will be selected, not only Mifare cards).
 *
 *   Please refer to Philips "Mifare Type Identification Procedure" documentation
 *   for atq and sak explanation.
 *
 *   The Unique ID size is to be found in atq[0] :
 *   (atq[0] & 0xC0 == 0x00) --> single size UID (snr is 4-byte long)
 *   (atq[0] & 0xC0 == 0x40) --> double size UID (snr is 7-byte long)
 *   (atq[0] & 0xC0 == 0x80) --> triple size UID (snr is 10-byte long) 
 *
 *
 * SEE ALSO
 *   SPROX_A_SelectAny
 *   SPROX_MifStSelectIdle
 *
 **/
SPROX_API_FUNC(MifStSelectAny) (SPROX_PARAM  BYTE snr[10], BYTE atq[2], BYTE sak[1])
{
  BYTE snrlen = 10;
  return SPROX_API_CALL(A_SelectAny)  (SPROX_PARAM_P  atq, snr, &snrlen, sak);
}

/**f* SpringProx.API/SPROX_MifStSelectIdle
 *
 * NAME
 *   SPROX_MifStSelectIdle
 *
 * WARNING
 *   This function is deprecated. Use SPROX_A_SelectIdle instead.
 * 
 * DESCRIPTION
 *   Same as SPROX_MifStSelectAny, but will return only a card that is in the IDLE
 *   state.
 *   This allow discovering all ISO 14443-A available in the RF field :
 *   - call SPROX_MifStSelectIdle to discover the first card
 *   - halt this card calling SPROX_MifStHalt
 *   - call SPROX_MifStSelectIdle again to discover next card, and so on...
 *
 * INPUTS
 *   BYTE snr[4]        : 4-byte buffer to receive card's Unique ID
 *   BYTE atq[2]        : 2-byte buffer to receive card's Answer To Query
 *   BYTE sak[1]        : 1-byte buffer to receive card's Select AcKnowledge
 *
 * RETURNS
 *   MI_OK              : success, card selected
 *   MI_NOTAGERR        : no IDLE card available in the RF field
 *   Other code if internal or communication error has occured. 
 *
 * NOTES
 *   Despite the name of the function, it doesn't check the returned card type
 *   (i.e., any ISO 14443-A card will be selected, not only Mifare cards).
 *
 * SEE ALSO
 *   SPROX_A_SelectIdle
 *   SPROX_MifStSelectAny
 *
 **/
SPROX_API_FUNC(MifStSelectIdle) (SPROX_PARAM  BYTE snr[10], BYTE atq[2], BYTE sak[1])
{
  BYTE snrlen = 10;
  return SPROX_API_CALL(A_SelectIdle)  (SPROX_PARAM_P  atq, snr, &snrlen, sak);
}

/**f* SpringProx.API/SPROX_MifStSelectAgain
 *
 * NAME
 *   SPROX_MifStSelectAgain
 *
 * DESCRIPTION
 *   Allow to re-select an ISO 14443-A card, provided its serial number (the card
 *   must be available in the RF field, either in the IDLE or HALTed state).
 *
 * WARNING
 *   This function is deprecated. Use SPROX_A_SelectAgain instead.
 *
 * INPUTS
 *   const BYTE snr[4]  : 4-byte Unique ID of the card to wake-up
 *
 * RETURNS
 *   MI_OK              : success, card selected
 *   MI_NOTAGERR        : the required card is not available in the RF field
 *   Other code if internal or communication error has occured. 
 *
 * SEE ALSO
 *   SPROX_A_SelectAgain
 *
 **/
SPROX_API_FUNC(MifStSelectAgain) (SPROX_PARAM  const BYTE snr[4])
{
  return SPROX_API_CALL(A_SelectAgain)  (SPROX_PARAM_P  snr, 4);
}

/**f* SpringProx.API/SPROX_MifStHalt
 *
 * NAME
 *   SPROX_MifStHalt
 *
 * WARNING
 *   This function is deprecated. Use SPROX_A_Halt instead.
 *
 * INPUTS
 *   none
 *
 * RETURNS
 *   MI_OK              : success, card halted
 *   Other code if internal or communication error has occured. 
 * 
 * SEE ALSO
 *   SPROX_A_Halt
 *
 **/
SPROX_API_FUNC(MifStHalt) (SPROX_PARAM_V)
{
  return SPROX_API_CALL(A_Halt) (SPROX_PARAM_PV);
}



SPROX_API_FUNC(MifStAuthE2) (SPROX_PARAM  BYTE auth_mode, const BYTE snr[4], BYTE key_sector, BYTE block)
{
	SWORD rc;
  SPROX_PARAM_TO_CTX;
  
  if (!sprox_ctx->sprox_version)
  {
    rc = MI_FUNCTION_NOT_AVAILABLE;
  } else
  {
    BYTE    buffer[7];
    buffer[0] = auth_mode;
    if (snr != NULL)
      memcpy(&buffer[1], snr, 4);
    else
      memcpy(&buffer[1], sprox_ctx->mif_snr, 4);
    buffer[5] = key_sector;
    buffer[6] = block;
    rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_PICCAUTHE2, buffer, sizeof(buffer), NULL, NULL);
  }
  
  SPROX_Trace(TRACE_DEBUG, "MifStAuthE2(%02X %d %d) -> %d", auth_mode, key_sector, block, rc);
  return rc;
}

SPROX_API_FUNC(MifStAuthRam) (SPROX_PARAM  BYTE auth_mode, BYTE key_sector, BYTE block)
{
	SWORD rc;
  BYTE buffer[3];  
  SPROX_PARAM_TO_CTX;

  if (auth_mode == PICC_AUTHENT1A)
    sprox_ctx->mif_auth_info = MIF_RAM_KEY | MIF_KEY_A;
  else if (auth_mode == PICC_AUTHENT1B)
    sprox_ctx->mif_auth_info = MIF_RAM_KEY | MIF_KEY_B;
  else
    return MI_LIB_CALL_ERROR;

 if (key_sector >= sprox_ram_key_count)
    return MI_LIB_CALL_ERROR;

  sprox_ctx->mif_auth_info |= key_sector;

  if (auth_mode == PICC_AUTHENT1B)
    key_sector += sprox_ram_key_count / 2;

  buffer[0] = key_sector;
  buffer[1] = 0x00;
  buffer[2] = block;
  rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_AUTHENTICATION2, buffer, sizeof(buffer), NULL, NULL);

  SPROX_Trace(TRACE_DEBUG, "MifStAuthRam(%02X %d %d) -> %d", auth_mode, key_sector, block, rc);
  return rc;
}

SPROX_API_FUNC(MifStAuthKey) (SPROX_PARAM  BYTE auth_mode, const BYTE snr[4], const BYTE key[6], BYTE block)
{
	SWORD rc;
  SPROX_PARAM_TO_CTX;
  
  if (!sprox_ctx->sprox_version)
  {
    rc = MI_FUNCTION_NOT_AVAILABLE;
  } else
  if (sprox_ctx->sprox_version >= 0x00015511)
  {
    BYTE buffer[12];   

    buffer[0] = auth_mode; 
    if (snr != NULL)
      memcpy(&buffer[1], snr, 4);
    else
      memcpy(&buffer[1], sprox_ctx->mif_snr, 4);
    memcpy(&buffer[5], key, 6);
    buffer[11] = block;
    
    rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_PICCAUTHKEY, buffer, sizeof(buffer), NULL, NULL);
      
  } else   
  {
    BYTE buffer[18];   
    BYTE coded_key[12];
    BYTE cnt = 0;
    BYTE ln = 0;
    BYTE hn = 0;
    for (cnt = 0; cnt < 6; cnt++)
    {
      ln = key[cnt] & 0x0F;
      hn = key[cnt] >> 4;
      coded_key[cnt * 2 + 1] = (~ln << 4) | ln;
      coded_key[cnt * 2] = (~hn << 4) | hn;
    }
    buffer[0] = auth_mode;
    if (snr != NULL)
      memcpy(&buffer[1], snr, 4);
    else
      memcpy(&buffer[1], sprox_ctx->mif_snr, 4);
    memcpy(&buffer[5], coded_key, 12);
    buffer[17] = block;
    
    rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_PICCAUTHKEY, buffer, sizeof(buffer), NULL, NULL);
  }

  SPROX_Trace(TRACE_DEBUG, "MifStAuthKey(%02X %d) -> %d", auth_mode, block, rc);
  return rc;
}

SPROX_API_FUNC(MifStAuthKey2) (SPROX_PARAM  const BYTE snr[4], BYTE key_idx, BYTE block)
{
  BOOL eeprom;
  BYTE key_type;
  
  eeprom     = (key_idx & MIF_E2_KEY) ? TRUE : FALSE;
  key_type   = (key_idx & MIF_KEY_B)  ? PICC_AUTHENT1B : PICC_AUTHENT1A;
  
  if (eeprom)
    return SPROX_API_CALL(MifStAuthE2) (SPROX_PARAM_P  key_type, snr, key_type, block);  
  
  if (snr != NULL)
  {
    SWORD rc = SPROX_API_CALL(MifStSelectAgain) (SPROX_PARAM_P  snr);
    if (rc != MI_OK) return rc;
  }

  return SPROX_API_CALL(MifStAuthRam) (SPROX_PARAM_P  key_type, key_type, block);
}

SPROX_API_FUNC(MifStValue)  (SPROX_PARAM  BYTE dd_mode, BYTE addr, BYTE *value, BYTE trans_addr)
{
  BYTE    buffer[7];
  buffer[0] = dd_mode;
  buffer[1] = addr;
  memcpy(&buffer[2], value, 4);
  buffer[6] = trans_addr;
  return SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_PICCVALUE, buffer, sizeof(buffer), NULL, NULL);
}

/**f* SpringProx.API/SPROX_MifRead
 *
 * NAME
 *   SPROX_MifRead
 *
 * DESCRIPTION
 *   Read one 16-byte block of a Mifare card.
 *   No implicit authentication is performed ; this is typically the appropriate
 *   function to read from a Mifare UltraLight card.
 *
 * WARNING
 *   On a Mifare Standard card, authentication is mandatory, use SPROX_MifStReadBlock.
 *
 * INPUTS
 *   const BYTE snr[4]  : 4-byte UID of the Mifare card to read
 *                        If NULL, the reader will work with currently selected card
 *   BYTE address       : address to read
 *   BYTE data[16]      : 16-byte buffer to receive the data
 *
 * RETURNS
 *   MI_OK              : success, data have been read
 *   MI_NOTAGERR        : the required card is not available in the RF field
 *   Other code if internal or communication error has occured. 
 *
 * SEE ALSO
 *   SPROX_MifWrite
 *   SPROX_MifStReadBlock
 *
 **/
SPROX_API_FUNC(MifRead) (SPROX_PARAM  const BYTE snr[4], BYTE addr, BYTE data[16])
{
  SWORD rc;
  SPROX_PARAM_TO_CTX;
  
  if (snr != NULL)
  {
    rc = SPROX_API_CALL(MifStSelectAgain) (SPROX_PARAM_P  snr);
    if (rc != MI_OK) return rc;
  }

  if (sprox_ctx->sprox_version)
  {
    BYTE    buffer[3];
    SWORD   rc;
    WORD    len = 16;
    buffer[0] = PICC_READ16;
    buffer[1] = addr;
    buffer[2] = 16;
    rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_PICCCOMMONREAD, buffer, sizeof(buffer), data, &len);
    if (rc != MI_OK)
      return rc;
    if (len != 16)
      return MI_WRONG_LENGTH;
    return MI_OK;
  } else
  {
    WORD    len = 16;
    return SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_READ, &addr, 1, data, &len);
  }
}

/**f* SpringProx.API/SPROX_MifWrite
 *
 * NAME
 *   SPROX_MifWrite
 *
 * DESCRIPTION
 *   Write one 16-byte block of a Mifare card.
 *   No implicit authentication is performed ; this is typically the appropriate
 *   function to read from a Mifare UltraLight card.
 *
 * WARNING
 *   Mifare UltraLight cards accept this command, but only the 4 first bytes are
 *   actually written (same behaviour as SPROX_MifWrite4).
 *   On a Mifare Standard card, authentication is mandatory, use SPROX_MifStWriteBlock.
 *
 * INPUTS
 *   const BYTE snr[4]   : 4-byte UID of the Mifare card to read
 *                         If NULL, the reader will work with currently selected card
 *   BYTE address        : address to write
 *   const BYTE data[16] : data to be written
 *
 * RETURNS
 *   MI_OK              : success, data have been written
 *   MI_NOTAGERR        : the required card is not available in the RF field
 *   Other code if internal or communication error has occured. 
 *
 * SEE ALSO
 *   SPROX_MifWrite4
 *   SPROX_MifRead
 *   SPROX_MifStWriteBlock
 *
 **/
SPROX_API_FUNC(MifWrite) (SPROX_PARAM  const BYTE snr[4], BYTE addr, const BYTE data[16])
{
  SWORD rc;
  SPROX_PARAM_TO_CTX;

  if (snr != NULL)
  {
    rc = SPROX_API_CALL(MifStSelectAgain) (SPROX_PARAM_P  snr);
    if (rc != MI_OK) return rc;
  }
  
  if (sprox_ctx->sprox_version)
  {
    BYTE    buffer[16+3];
    buffer[0] = PICC_WRITE16;
    buffer[1] = addr;
    buffer[2] = 16;
    memcpy(&buffer[3], data, 16);
    return SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_PICCCOMMONWRITE, buffer, sizeof(buffer), NULL, NULL);
  } else  
  {
    BYTE    buffer[17];
    buffer[0] = addr;
    memcpy(&buffer[1], data, 16);
    return SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_WRITE, buffer, sizeof(buffer), NULL, NULL);
  }
}

/**f* SpringProx.API/SPROX_MifWrite4
 *
 * NAME
 *   SPROX_MifWrite4
 *
 * DESCRIPTION
 *   Write one 4-byte block of a Mifare card.
 *   No implicit authentication is performed ; this is typically the appropriate
 *   function to read from a Mifare UltraLight card.
 *
 * WARNING
 *   On a Mifare Standard card, authentication is mandatory and only 16-byte blocks can
 *   be written ; use SPROX_MifStWriteBlock.
 *
 * INPUTS
 *   const BYTE snr[4]  : 4-byte UID of the Mifare card to read
 *                        If NULL, the reader will work with currently selected card
 *   BYTE address       : address to write
 *   const BYTE data[4] : data to be written
 *
 * RETURNS
 *   MI_OK              : success, data have been written
 *   MI_NOTAGERR        : the required card is not available in the RF field
 *   Other code if internal or communication error has occured. 
 *
 * SEE ALSO
 *   SPROX_MifWrite
 *   SPROX_MifStWriteBlock
 *
 **/
SPROX_API_FUNC(MifWrite4) (SPROX_PARAM  const BYTE snr[4], BYTE addr, const BYTE data[4])
{
  SWORD rc;
  SPROX_PARAM_TO_CTX;
  
  if (!sprox_ctx->sprox_version)
    return MI_FUNCTION_NOT_AVAILABLE;

  if (snr != NULL)
  {
    rc = SPROX_API_CALL(MifStSelectAgain) (SPROX_PARAM_P  snr);
    if (rc != MI_OK) return rc;
  }

  {
    BYTE    buffer[4+3];
    buffer[0] = PICC_WRITE4;
    buffer[1] = addr;
    buffer[2] = 4;
    memcpy(&buffer[3], data, 4);
    return SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_PICCCOMMONWRITE, buffer, sizeof(buffer), NULL, NULL);
  }
}

/*
 * Internal function to read or write a block, using keys in EEPROM or RAM
 * -----------------------------------------------------------------------
 */
static SWORD MifStRWBlock(SPROX_PARAM  BOOL w, const BYTE *snr, BYTE bloc, BYTE *data)
{
  SWORD   rc;
  SPROX_PARAM_TO_CTX;
  
  rc = MI_LIB_CALL_ERROR;
  if (data == NULL)
    goto exit_proc;

  if (sprox_ctx->sprox_version)
  {
    if (snr != NULL)
    {
      rc = SPROX_API_CALL(MifStSelectAgain) (SPROX_PARAM_P  snr);
      if (rc != MI_OK)
        goto exit_proc;
    }

    if (w)
    {
      BYTE    buffer[17];
      buffer[0] = bloc;
      memcpy(&buffer[1], data, 16);
      rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_WRITE_BLOC, buffer, sizeof(buffer), NULL, NULL);
    } else
    {
      WORD    len = 16;
      rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_READ_BLOC, &bloc, 1, data, &len);
      if ((rc == MI_OK) && (len != 16))
        rc = MI_WRONG_LENGTH;
    }

  } else
  {
    BYTE    key_id;
    BYTE    key_type_id;
    BYTE    key_type;

    if (sprox_ctx->mif_auth_ok)
    {
      /* Skip authentication and try to Read or Write directly */
      if (w)
        rc = SPROX_API_CALL(MifWrite) (SPROX_PARAM_P  NULL, bloc, data);
      else
        rc = SPROX_API_CALL(MifRead) (SPROX_PARAM_P  NULL, bloc, data);
      if (rc == MI_OK)
        goto exit_proc;
    }
    sprox_ctx->mif_auth_ok = FALSE;

    /* Try all available keys in CSB-3 RAM */
    /* ----------------------------------- */

    for (key_type_id = 0; key_type_id < 2; key_type_id++)
    {
      if (w)
        key_type = (BYTE) (PICC_AUTHENT1B - key_type_id); /* B, then A */
      else
        key_type = (BYTE) (PICC_AUTHENT1A + key_type_id); /* A, then B */

      for (key_id = 0; key_id < sprox_ram_key_count; key_id++)
      {
        /* Request + Select */
        rc = SPROX_API_CALL(MifStSelectAgain) (SPROX_PARAM_P  snr);
        if (rc != MI_OK)
          goto exit_proc;

        rc = SPROX_API_CALL(MifStAuthRam) (SPROX_PARAM_P  key_type, key_id, bloc);
        if (rc == MI_OK)
        {
          /* Authentication OK ? Let's try to Read or to Write the block ! */
          if (w)
            rc = SPROX_API_CALL(MifWrite) (SPROX_PARAM_P  NULL, bloc, data);
          else
            rc = SPROX_API_CALL(MifRead) (SPROX_PARAM_P  NULL, bloc, data);

          /* If successfull, remember it ! */
          if (rc == MI_OK)
          {
            sprox_ctx->mif_auth_ok = TRUE;
            goto exit_proc;
          }
        }
      }
    }
  }

exit_proc:
  if (snr != NULL)
  {
    SPROX_Trace(TRACE_DEBUG, "MifSt%sBlock %d %04X%04X%04X%04X -> %d", w ? "Write" : "Read", bloc, snr[0], snr[1], snr[2], snr[3], rc);
  } else
  {
    SPROX_Trace(TRACE_DEBUG, "MifSt%sBlock %d -> %d", w ? "Write" : "Read", bloc, rc);
  }

  return rc;
}

/*
 * Internal function to read or write a block, using supplied key
 * --------------------------------------------------------------
 */
static SWORD MifStRWBlockKey(SPROX_PARAM  BOOL w, const BYTE *snr, BYTE bloc, BYTE *data, const BYTE key_val[6])
{
  SWORD rc;
  SPROX_PARAM_TO_CTX;

  if (!sprox_ctx->sprox_version)
    return MI_FUNCTION_NOT_AVAILABLE;

  rc = MI_LIB_CALL_ERROR;
  if (data == NULL)
    goto exit_proc;
  if (key_val == NULL)
    goto exit_proc;

  if ((snr != NULL) || (!sprox_ctx->sprox_version))
  {
    rc = SPROX_API_CALL(MifStSelectAgain) (SPROX_PARAM_P  snr);
    if (rc != MI_OK)
      goto exit_proc;
  }

  if (w)
  {
    BYTE    buffer[23];
    buffer[0] = bloc;
    memcpy(&buffer[1], data, 16);
    memcpy(&buffer[17], key_val, 6);
    rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_WRITE_BLOC, buffer, sizeof(buffer), NULL, NULL);
  } else
  {
    WORD    len = 16;
    BYTE    buffer[7];
    buffer[0] = bloc;
    memcpy(&buffer[1], key_val, 6);
    rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_READ_BLOC, buffer, sizeof(buffer), data, &len);
    if ((rc == MI_OK) && (len != 16))
      rc = MI_WRONG_LENGTH;
  }

exit_proc:
  if (snr != NULL)
  {
    SPROX_Trace(TRACE_DEBUG, "MifSt%sBlockKey %d %04X%04X%04X%04X %02X%02X%02X%02X%02X%02X -> %d", w ? "Write" : "Read", bloc, snr[0], snr[1], snr[2], snr[3], key_val[0], key_val[1], key_val[2], key_val[3], key_val[4], key_val[5], rc);
  } else
  {
    SPROX_Trace(TRACE_DEBUG, "MifSt%sBlockKey %d %02X%02X%02X%02X%02X%02X -> %d", w ? "Write" : "Read", bloc, key_val[0], key_val[1], key_val[2], key_val[3], key_val[4], key_val[5], rc);
  }

  return rc;
}

/*
 * Internal function to read or write a block, using specified key
 * ---------------------------------------------------------------
 */
static SWORD MifStRWBlockKey2(SPROX_PARAM  BOOL w, const BYTE *snr, BYTE bloc, BYTE *data, BYTE key_idx)
{
  SWORD rc;
  SPROX_PARAM_TO_CTX;

  if (sprox_ctx->sprox_version < 0x00014200)
    return MI_FUNCTION_NOT_AVAILABLE;

  if (snr != NULL)
  {
    rc = SPROX_API_CALL(MifStSelectAgain) (SPROX_PARAM_P  snr);
    if (rc != MI_OK)
      goto exit_proc;
  }

  if (w)
  {
    BYTE    buffer[18];
    buffer[0] = bloc;
    memcpy(&buffer[1], data, 16);
    buffer[17] = key_idx;
    rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_WRITE_BLOC, buffer, sizeof(buffer), NULL, NULL);
  } else
  {
    WORD    len = 16;
    BYTE    buffer[2];
    buffer[0] = bloc;
    buffer[1] = key_idx;
    rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_READ_BLOC, buffer, sizeof(buffer), data, &len);
    if ((rc == MI_OK) && (len != 16))
      rc = MI_WRONG_LENGTH;
  }

exit_proc:
  if (snr != NULL)
  {
    SPROX_Trace(TRACE_DEBUG, "MifSt%sBlockKey2 %d %04X%04X%04X%04X %02X -> %d", w ? "Write" : "Read", bloc, snr[0], snr[1], snr[2], snr[3], key_idx, rc);
  } else
  {
    SPROX_Trace(TRACE_DEBUG, "MifSt%sBlockKey2 %d %02X -> %d", w ? "Write" : "Read", bloc, key_idx, rc);
  }

  return rc;
}

/*
 * Internal function to read or write a sector, using keys in EEPROM or RAM
 * ------------------------------------------------------------------------
 */
static SWORD MifStRWSector(SPROX_PARAM  BOOL w, const BYTE *snr, BYTE sect, BYTE *data)
{
  SWORD rc;
  SPROX_PARAM_TO_CTX;

  rc = MI_LIB_CALL_ERROR;
  if (data == NULL)
    goto exit_proc;

  if (((sprox_ctx->sprox_version > 0x00000000) && (sect < 16))
      || ((sprox_ctx->sprox_version >= 0x00011300) && (sect >= 32)) || ((sprox_ctx->sprox_version >= 0x00011600)))
  {
    if (snr != NULL)
    {
      rc = SPROX_API_CALL(MifStSelectAgain) (SPROX_PARAM_P  snr);
      if (rc != MI_OK)
        goto exit_proc;
    }

    if (sect < 32)
    {
      /* 1 sector = 3 blocks */
      if (w)
      {
        BYTE    buffer[49];
        buffer[0] = sect;
        memcpy(&buffer[1], data, 48);
        rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_WRITE_SECT, buffer, sizeof(buffer), NULL, NULL);
      } else
      {
        WORD    len = 48;
        rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_READ_SECT, &sect, 1, data, &len);
        if ((rc == MI_OK) && (len != 48))
          rc = MI_WRONG_LENGTH;
      }
    } else
    {
      /* 1 sector = 15 blocks */
      if (w)
      {
        BYTE    buffer[241];
        buffer[0] = sect;
        memcpy(&buffer[1], data, 240);
        rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_WRITE_SECT, buffer, sizeof(buffer), NULL, NULL);
      } else
      {
        WORD    len = 240;
        rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_READ_SECT, &sect, 1, data, &len);
        if ((rc == MI_OK) && (len != 240))
          rc = MI_WRONG_LENGTH;
      }
    }
  } else
  {
    BYTE    bloc;
    BYTE    fbloc;

    if (sect < 32)
    {
      /* 1 sector = 3 blocks */
      if (w && (sect == 0))
        fbloc = 1;
      else
        fbloc = 0;

      for (bloc = fbloc; bloc < 3; bloc++)
      {
        rc = MifStRWBlock(SPROX_PARAM_P  w, snr, (BYTE) (4 * sect + bloc), &data[16 * bloc]);
        if (rc != MI_OK)
          break;
      }
    } else
    {
      /* 1 sector = 15 blocks */
      sect -= 32;
      for (bloc = 0; bloc < 15; bloc++)
      {
        rc = MifStRWBlock(SPROX_PARAM_P  w, snr, (BYTE) (128 + 16 * sect + bloc), &data[16 * bloc]);
        if (rc != MI_OK)
          break;
      }
    }
  }

exit_proc:
  if (snr != NULL)
  {
    SPROX_Trace(TRACE_DEBUG, "MifSt%sSector %d %04X%04X%04X%04X -> %d", w ? "Write" : "Read", sect, snr[0], snr[1], snr[2], snr[3], rc);
  } else
  {
    SPROX_Trace(TRACE_DEBUG, "MifSt%sSector %d -> %d", w ? "Write" : "Read", sect, rc);
  }

  return rc;
}

/*
 * Internal function to read or write a sector, using supplied key
 * ---------------------------------------------------------------
 */
static SWORD MifStRWSectorKey(SPROX_PARAM  BOOL w, const BYTE *snr, BYTE sect, BYTE *data, const BYTE key_val[6])
{
  SWORD rc;
  SPROX_PARAM_TO_CTX;

  if (!sprox_ctx->sprox_version)
    return MI_FUNCTION_NOT_AVAILABLE;

  rc = MI_LIB_CALL_ERROR;
  if (data == NULL)
    goto exit_proc;
  if (key_val == NULL)
    goto exit_proc;

  if (snr != NULL)
  {
    rc = SPROX_API_CALL(MifStSelectAgain) (SPROX_PARAM_P  snr);
    if (rc != MI_OK)
      goto exit_proc;
  }

  if ((sect < 16) || (sprox_ctx->sprox_version >= 0x00011300))
  {
    if (sect < 32)
    {
      /* 1 sector = 3 blocks */
      if (w)
      {
        BYTE    buffer[55];
        buffer[0] = sect;
        memcpy(&buffer[1], data, 48);
        memcpy(&buffer[49], key_val, 6);
        rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_WRITE_SECT, buffer, sizeof(buffer), NULL, NULL);
      } else
      {
        WORD    len = 48;
        BYTE    buffer[7];
        buffer[0] = sect;
        memcpy(&buffer[1], key_val, 6);
        rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_READ_SECT, buffer, sizeof(buffer), data, &len);
        if ((rc == MI_OK) && (len != 48))
          rc = MI_WRONG_LENGTH;
      }
    } else
    {
      /* 1 sector = 15 blocks */
      if (w)
      {
        BYTE    buffer[247];
        buffer[0] = sect;
        memcpy(&buffer[1], data, 240);
        memcpy(&buffer[241], key_val, 6);
        rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_WRITE_SECT, buffer, sizeof(buffer), NULL, NULL);
      } else
      {
        WORD    len = 240;
        BYTE    buffer[7];
        buffer[0] = sect;
        memcpy(&buffer[1], key_val, 6);
        rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_READ_SECT, buffer, sizeof(buffer), data, &len);
        if ((rc == MI_OK) && (len != 240))
          rc = MI_WRONG_LENGTH;
      }
    }
  } else
  {
    BYTE    bloc;

    /* Bug fix for version prior than 1.13 */
    if (sect < 16)
    {
      /* 1 sector = 3 blocks */
      if (w)
      {
        BYTE    buffer[55];
        buffer[0] = sect;
        memcpy(&buffer[1], data, 48);
        memcpy(&buffer[49], key_val, 6);
        rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_WRITE_SECT, buffer, sizeof(buffer), NULL, NULL);
      } else
      {
        WORD    len = 48;
        BYTE    buffer[7];
        buffer[0] = sect;
        memcpy(&buffer[1], key_val, 6);
        rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_READ_SECT, buffer, sizeof(buffer), data, &len);
        if ((rc == MI_OK) && (len != 48))
          rc = MI_WRONG_LENGTH;
      }
    } else if (sect < 32)
    {
      /* 1 sector = 3 blocks */
      for (bloc = 0; bloc < 3; bloc++)
      {
        rc = MifStRWBlockKey(SPROX_PARAM_P  w, snr, (BYTE) (4 * sect + bloc), &data[16 * bloc], key_val);
        if (rc != MI_OK)
          break;
      }
    } else
    {
      /* 1 sector = 15 blocks */
      sect -= 32;
      for (bloc = 0; bloc < 16; bloc++)
      {
        rc = MifStRWBlockKey(SPROX_PARAM_P  w, snr, (BYTE) (128 + 16 * sect + bloc), &data[16 * bloc], key_val);
        if (rc != MI_OK)
          break;
      }
    }
  }

exit_proc:
  if (snr != NULL)
  {
    SPROX_Trace(TRACE_DEBUG, "MifSt%sSectorKey %d %04X%04X%04X%04X %02X%02X%02X%02X%02X%02X -> %d", w ? "Write" : "Read", sect, snr[0], snr[1], snr[2], snr[3], key_val[0], key_val[1], key_val[2], key_val[3], key_val[4], key_val[5], rc);
  } else
  {
    SPROX_Trace(TRACE_DEBUG, "MifSt%sSectorKey %d %02X%02X%02X%02X%02X%02X -> %d", w ? "Write" : "Read", sect, key_val[0], key_val[1], key_val[2], key_val[3], key_val[4], key_val[5], rc);
  }

  return rc;
}

/*
 * Internal function to read or write a sector, using specified key
 * ----------------------------------------------------------------
 */
static SWORD MifStRWSectorKey2(SPROX_PARAM  BOOL w, const BYTE *snr, BYTE sect, BYTE *data, BYTE key_idx)
{
  SWORD rc;
  SPROX_PARAM_TO_CTX;

  if (sprox_ctx->sprox_version < 0x00014200)
    return MI_FUNCTION_NOT_AVAILABLE;

  if (snr != NULL)
  {
    rc = SPROX_API_CALL(MifStSelectAgain) (SPROX_PARAM_P  snr);
    if (rc != MI_OK)
      goto exit_proc;
  }

  if (sect < 32)
  {
    /* 1 sector = 3 blocks */
    if (w)
    {
      BYTE    buffer[50];
      buffer[0] = sect;
      memcpy(&buffer[1], data, 48);
      buffer[49] = key_idx;
      rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_WRITE_SECT, buffer, sizeof(buffer), NULL, NULL);
    } else
    {
      WORD    len = 48;
      BYTE    buffer[2];
      buffer[0] = sect;
      buffer[1] = key_idx;
      rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_READ_SECT, buffer, sizeof(buffer), data, &len);
      if ((rc == MI_OK) && (len != 48))
        rc = MI_WRONG_LENGTH;
    }
  } else
  {
    /* 1 sector = 15 blocks */
    if (w)
    {
      BYTE    buffer[242];
      buffer[0] = sect;
      memcpy(&buffer[1], data, 240);
      buffer[241] = key_idx;
      rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_WRITE_SECT, buffer, sizeof(buffer), NULL, NULL);
    } else
    {
      WORD    len = 240;
      BYTE    buffer[2];
      buffer[0] = sect;
      buffer[1] = key_idx;
      rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_READ_SECT, buffer, sizeof(buffer), data, &len);
      if ((rc == MI_OK) && (len != 240))
        rc = MI_WRONG_LENGTH;
    }
  }
  
exit_proc:
  if (snr != NULL)
  {
    SPROX_Trace(TRACE_DEBUG, "MifSt%sSectorKey2 %d %04X%04X%04X%04X %02X -> %d", w ? "Write" : "Read", sect, snr[0], snr[1], snr[2], snr[3], key_idx, rc);
  } else
  {
    SPROX_Trace(TRACE_DEBUG, "MifSt%sSectorKey2 %d %02X -> %d", w ? "Write" : "Read", sect, key_idx, rc);
  }

  return rc;
}

/**f* SpringProx.API/SPROX_MifStReadBlock
 *
 * NAME
 *   SPROX_MifStReadBlock
 *
 * DESCRIPTION
 *   Read one 16-byte block of a Mifare tag, using given key or internally available
 *   keys for authentication.
 *
 * INPUTS
 *   const BYTE snr[4]  : 4-byte UID of the Mifare card to read
 *                        If NULL, the reader will work with currently selected tag
 *   BYTE bloc          : address of the block to read
 *   BYTE data[16]      : 16-byte buffer to receive the data
 *   const BYTE key[6]  : The Mifare access key of the sector (either A or B)
 *                        If NULL, the reader will try all the preloaded keys
 *
 * WARNING
 *   The number of blocks available on a Mifare tag depends on the tag type.
 *
 *   For example, a Mifare classic 1k tag stores 64 blocks (address 0 to 63), a
 *   Mifare classic 4k tag stores 256 blocks (address 0 to 255).
 *   The last block of each Mifare classic sector is known as security block (or
 *   sector trailer) and holds the access keys to the sector. This functions allows
 *   reading of those specific blocks, but the keys are masked by Mifare integrated
 *   security features.
 *
 *   For more details, please refer to specific tag documentation.
 *
 * RETURNS
 *   MI_OK              : success, data have been read
 *   MI_NOTAGERR        : the required tag is not available in the RF field,
 *                        or the supplied key has been denied
 *   MI_AUTHERR         : access to the block has been denied
 *   Other code if internal or communication error has occured. 
 *
 * NOTES
 *   If key is not NULL, it is tried first as an 'A' key, then on failure as a 'B' key.
 *
 *   If not Mifare key is supplied, the reader automatically tries every keys
 *   available in its memory.
 *   If key is NULL, the key trying sequence is performed in this order :
 *   - 4  'A' keys stored in reader's RAM (if no key has been loaded into
 *     RAM since reader's last reset, this is skipped),
 *   - 4  'B' keys stored in reader's RAM (if no key has been loaded into
 *     RAM since reader's last reset, this is skipped),
 *   - 16 'A' keys stored in the RC chipset's EEPROM
 *   - 16 'B' keys stored in the RC chipset's EEPROM
 *
 *   Highest performance will be achieved when the right authentication key for
 *   this block is available at the beginning of this sequence !
 *
 *   After a succesfull read action, you can retrieve the key that has been used
 *   with SPROX_MifLastAuthKey.
 *
 * SEE ALSO
 *   SPROX_MifLoadKey
 *   SPROX_MifStReadSector
 *
 **/
SPROX_API_FUNC(MifStReadBlock) (SPROX_PARAM  const BYTE snr[4], BYTE bloc, BYTE data[16], const BYTE key_val[6])
{
  if (key_val == NULL)
    return MifStRWBlock(SPROX_PARAM_P  FALSE, snr, bloc, data);
  else
    return MifStRWBlockKey(SPROX_PARAM_P  FALSE, snr, bloc, data, key_val);
}

SPROX_API_FUNC(MifStReadBlock2) (SPROX_PARAM  const BYTE snr[4], BYTE bloc, BYTE data[16], BYTE key_idx)
{
  return MifStRWBlockKey2(SPROX_PARAM_P  FALSE, snr, bloc, data, key_idx);
}

/**f* SpringProx.API/SPROX_MifStReadSector
 *
 * NAME
 *   SPROX_MifStReadSector
 *
 * DESCRIPTION
 *   Read one sector of a Mifare tag, using given key or internally available
 *   keys for authentication.
 *
 * INPUTS
 *   const BYTE snr[4]  : 4-byte UID of the Mifare card to read
 *                        If NULL, the reader will work with currently selected tag
 *   BYTE sect          : address of the sector to read
 *   BYTE data[48/240]  : 48-byte or 240-byte buffer to receive the data
 *   const BYTE key[6]  : The Mifare access key of the sector (either A or B)
 *                        If NULL, the reader will try all the preloaded keys
 *
 * WARNING
 *   The number of sectors available on a Mifare tag depends on the tag type.
 *   The size of each sector depends both on the tag type and on the sector number.
 *
 *   For example, a Mifare classic 1k tag has 16 sectors (address 0 to 15). Each
 *   sector has a size of 3+1 blocks (48 bytes of data). A Mifare classic 4k tag 
 *   has 40 sectors (address 0 to 39). Sectors 0 to 31 have a size of 3+1 blocks
 *   (48 bytes of data), while sectors 32 to 39 have a size of 15+1 blocks (240
 *   bytes of data).
 *
 *   For more details, please refer to specific tag documentation.
 *
 *   This function never returns the last block of each sector (security block or
 *   sector trailer). Explicitly use SPROX_MifStReadBlock to access those blocks.
 *
 * RETURNS
 *   MI_OK              : success, data have been read
 *   MI_NOTAGERR        : the required tag is not available in the RF field,
 *                        or the supplied key has been denied
 *   MI_AUTHERR         : access to the sector has been denied
 *   Other code if internal or communication error has occured. 
 *
 * NOTES
 *   The automatic key trying sequence is the same as in SPROX_MifStReadBlock.
 *
 * SEE ALSO
 *   SPROX_MifStReadBlock
 *
 **/
SPROX_API_FUNC(MifStReadSector) (SPROX_PARAM  const BYTE snr[4], BYTE sect, BYTE data[240], const BYTE key_val[6])
{
  if (key_val == NULL)
    return MifStRWSector(SPROX_PARAM_P  FALSE, snr, sect, data);
  else
    return MifStRWSectorKey(SPROX_PARAM_P  FALSE, snr, sect, data, key_val);
}

SPROX_API_FUNC(MifStReadSector2) (SPROX_PARAM  const BYTE snr[4], BYTE sect, BYTE data[240], BYTE key_idx)
{
  return MifStRWSectorKey2(SPROX_PARAM_P  FALSE, snr, sect, data, key_idx);
}

/**f* SpringProx.API/SPROX_MifStWriteBlock
 *
 * NAME
 *   SPROX_MifStWriteBlock
 *
 * DESCRIPTION
 *   Write one 16-byte block of a Mifare tag, using given key or internally available
 *   keys for authentication.
 *
 * INPUTS
 *   const BYTE snr[4]   : 4-byte UID of the Mifare card to read
 *                         If NULL, the reader will work with currently selected tag
 *   BYTE bloc           : address of the block to write
 *   const BYTE data[16] : 16-byte buffer of data
 *   const BYTE key[6]   : The Mifare access key of the sector (either B or A)
 *                         If NULL, the reader will try all the preloaded keys
 *
 * WARNING
 *   The last block of each Mifare classic sector is known as security block (or
 *   sector trailer) and holds the access keys to the sector. This functions allows
 *   writing of those specific blocks, but a malformed block will irremediatly
 *   lock the sector. We strongly recommend to use SPROX_MifStUpdateAccessBlock
 *   instead.
 *
 *   For more details, please refer to specific tag documentation.
 *
 * RETURNS
 *   MI_OK              : success, data have been written
 *   MI_NOTAGERR        : the required tag is not available in the RF field,
 *                        or the supplied key has been denied
 *   MI_AUTHERR         : access to the block has been denied
 *   Other code if internal or communication error has occured. 
 *
 * NOTES
 *   If key is not NULL, it is tried first as an 'B' key, then on failure as a 'A' key.
 *
 *   If not Mifare key is supplied, the reader automatically tries every keys
 *   available in its memory.
 *   If key is NULL, the key trying sequence is performed in this order :
 *   - 4  'B' keys stored in reader's RAM (if no key has been loaded into
 *     RAM since reader's last reset, this is skipped),
 *   - 4  'A' keys stored in reader's RAM (if no key has been loaded into
 *     RAM since reader's last reset, this is skipped),
 *   - 16 'B' keys stored in the RC chipset's EEPROM
 *   - 16 'A' keys stored in the RC chipset's EEPROM
 *
 *   Highest performance will be achieved when the right authentication key for
 *   this block is available at the beginning of this sequence !
 *
 *   After a succesfull read action, you can retrieve the key that has been used
 *   with SPROX_MifLastAuthKey.
 *
 * SEE ALSO
 *   SPROX_MifLoadKey
 *   SPROX_MifStWriteSector
 *
 **/
SPROX_API_FUNC(MifStWriteBlock) (SPROX_PARAM  const BYTE snr[4], BYTE bloc, const BYTE data[16], const BYTE key_val[6])
{
  if (key_val == NULL)
    return MifStRWBlock(SPROX_PARAM_P  TRUE, snr, bloc, (BYTE *) data);
  else
    return MifStRWBlockKey(SPROX_PARAM_P  TRUE, snr, bloc, (BYTE *) data, key_val);
}

SPROX_API_FUNC(MifStWriteBlock2) (SPROX_PARAM  const BYTE snr[4], BYTE bloc, const BYTE data[16], BYTE key_idx)
{
  return MifStRWBlockKey2(SPROX_PARAM_P  TRUE, snr, bloc, (BYTE *) data, key_idx);
}

/**f* SpringProx.API/SPROX_MifStWriteSector
 *
 * NAME
 *   SPROX_MifStWriteSector
 *
 * DESCRIPTION
 *   Write one sector of a Mifare tag, using given key or internally available
 *   keys for authentication.
 *
 * INPUTS
 *   const BYTE snr[4]       : 4-byte UID of the Mifare card to read
 *                             If NULL, the reader will work with currently selected tag
 *   BYTE bloc               : address of the sector to write
 *   const BYTE data[48/240] : 48-byte or 240-byte buffer of data
 *   const BYTE key[6]       : The Mifare access key of the sector (either B or A)
 *                             If NULL, the reader will try all the preloaded keys 
 *
 * WARNING
 *   The number of sectors available on a Mifare tag depends on the tag type.
 *   The size of each sector depends both on the tag type and on the sector number.
 *
 *   For more details, please see SPROX_MifStReadSector or refer to specific tag
 *   documentation.
 *
 *   This function never writes the last block of each sector (security block or
 *   sector trailer). Explicitly use SPROX_MifStUpdateAccessBlock to write those
 *   blocks.
 *
 * RETURNS
 *   MI_OK              : success, data have been written
 *   MI_NOTAGERR        : the required tag is not available in the RF field,
 *                        or the supplied key has been denied
 *   MI_AUTHERR         : access to the sector has been denied
 *   Other code if internal or communication error has occured. 
 *
 * NOTES
 *   The automatic key trying sequence is the same as in SPROX_MifStWriteBlock.
 *
 * SEE ALSO
 *   SPROX_MifStWriteBlock
 *
 **/
SPROX_API_FUNC(MifStWriteSector) (SPROX_PARAM  const BYTE snr[4], BYTE sect, const BYTE data[240], const BYTE key_val[6])
{
  if (key_val == NULL)
    return MifStRWSector(SPROX_PARAM_P  TRUE, snr, sect, (BYTE *) data);
  else
    return MifStRWSectorKey(SPROX_PARAM_P  TRUE, snr, sect, (BYTE *) data, key_val);
}

SPROX_API_FUNC(MifStWriteSector2) (SPROX_PARAM  const BYTE snr[4], BYTE sect, const BYTE data[240], BYTE key_idx)
{
  return MifStRWSectorKey2(SPROX_PARAM_P  TRUE, snr, sect, (BYTE *) data, key_idx);
}

/**f* SpringProx.API/SPROX_MifStReadTag768
 *
 * NAME
 *   SPROX_MifStReadTag768
 *
 * DESCRIPTION
 *   Read a whole Mifare 1k tag (768 bytes), using internally available keys for
 *   authentication.
 *
 * INPUTS
 *   const BYTE snr[4]  : 4-byte UID of the Mifare card to read
 *                        If NULL, the reader will work with currently selected tag
 *   WORD *sectors      : pointer to a WORD used as bit-array
 *                        - in input, bit b(i) set means that sector i must be read
 *                        - in output, bit b(i) set means that sector i has been
 *                          successfully read
 *   BYTE data[768]     : 768-byte buffer to receive the data
 *
 * RETURNS
 *   MI_OK              : success, some data have been read. Check the *sectors bit-
 *                        array to see which sectors have been read
 *   MI_NOTAGERR        : the required tag is not available in the RF field,
 *                        or the supplied key has been denied
 *   Other code if internal or communication error has occured. 
 *
 * NOTES
 *   The automatic key trying sequence is the same as in SPROX_MifStWriteBlock.
 *
 * SEE ALSO
 *   SPROX_MifStReadBlock
 *   SPROX_MifStReadSector
 *
 **/
SPROX_API_FUNC(MifStReadTag768) (SPROX_PARAM  const BYTE snr[4], WORD *sectors, BYTE data[768])
{
  SWORD   rc;
  WORD    sectors_ok;
  BYTE    sect;
  SPROX_PARAM_TO_CTX;

  if (!sprox_ctx->sprox_version)
    return MI_FUNCTION_NOT_AVAILABLE;
  if (sectors == NULL)
    return MI_LIB_CALL_ERROR;
  if (data == NULL)
    return MI_LIB_CALL_ERROR;

  rc = SPROX_API_CALL(MifStSelectAgain) (SPROX_PARAM_P  snr);
  if (rc != MI_OK)
    return rc;

  if (sprox_ctx->sprox_capabilities & SPROX_WITH_XXL_BUFFERS)
  {
    /* Function supplied by the CSB itself */
    BYTE    buffer[770];
    WORD    length;

    /* Prepare the buffer */
    buffer[0] = 0xFF;
    buffer[1] = (BYTE) (*sectors / 0x0100);
    buffer[2] = (BYTE) (*sectors % 0x0100);
    length = sizeof(buffer);

    /* Execute the function */
    rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_READ_SECT, buffer, 3, buffer, &length);
    if ((rc == MI_OK) && (length == 770))
    {
      sectors_ok = buffer[0];
      sectors_ok *= 0x0100;
      sectors_ok |= buffer[1];
      *sectors = sectors_ok;
      memcpy(data, &buffer[2], 768);
    }

    return rc;

  } else
  {
    /* Function performed manually */
    BYTE   *ptr = data;
    SWORD   rc = MI_OK;

    sectors_ok = 0x0000;

    for (sect = 0; sect < 16; sect++)
    {
      if ((*sectors) & (1 << sect))
      {
        /* Read/Write this sector */
        rc = MifStRWSector(SPROX_PARAM_P  FALSE, snr, sect, ptr);
        if (rc == MI_OK)
        {
          /* Success with this sector */
          sectors_ok |= (1 << sect);
        } else
        if ((rc != MI_AUTHERR) && (rc != MI_NOTAUTHERR))
        {
          /* Fatal (?) error */
          break;
        } else
        {
          /* Dummy OK for this one */
          rc = MI_OK;
        }
      }
      /* Next sector */
      ptr += 48;
    }

    *sectors = sectors_ok;
    return rc;
  }
}

/**f* SpringProx.API/SPROX_MifStWriteTag768
 *
 * NAME
 *   SPROX_MifStWriteTag768
 *
 * DESCRIPTION
 *   Write a whole Mifare 1k tag (768 bytes), using internally available keys for
 *   authentication.
 *
 * INPUTS
 *   const BYTE snr[4]    : 4-byte UID of the Mifare card to write
 *                          If NULL, the reader will work with currently selected tag
 *   WORD *sectors        : pointer to a WORD used as bit-array
 *                          - in input, bit b(i) set means that sector i must be written
 *                          - in output, bit b(i) set means that sector i has been
 *                            successfully writen
 *   const BYTE data[768] : 768-byte buffer of data
 *
 * RETURNS
 *   MI_OK              : success, some data have been written. Check the *sectors bit-
 *                        array to see which sectors have been written
 *   MI_NOTAGERR        : the required tag is not available in the RF field,
 *                        or the supplied key has been denied
 *   Other code if internal or communication error has occured. 
 *
 * NOTES
 *   The automatic key trying sequence is the same as in SPROX_MifStWriteBlock.
 *
 * SEE ALSO
 *   SPROX_MifStWriteBlock
 *   SPROX_MifStWriteSector
 *
 **/
SPROX_API_FUNC(MifStWriteTag768) (SPROX_PARAM  const BYTE snr[4], WORD *sectors, const BYTE data[768])
{
  SWORD   rc;
  WORD    sectors_ok;
  BYTE    sect;
  SPROX_PARAM_TO_CTX;

  if (!sprox_ctx->sprox_version)
    return MI_FUNCTION_NOT_AVAILABLE;
  if (sectors == NULL)
    return MI_LIB_CALL_ERROR;
  if (data == NULL)
    return MI_LIB_CALL_ERROR;

  rc = SPROX_API_CALL(MifStSelectAgain) (SPROX_PARAM_P  snr);
  if (rc != MI_OK)
    return rc;

  if (sprox_ctx->sprox_capabilities & SPROX_WITH_XXL_BUFFERS)
  {
    /* Function supplied by the CSB itself */
    BYTE    buffer[770];
    WORD    length;

    /* Prepare the buffer */
    buffer[0] = (BYTE) (*sectors / 0x0100);
    buffer[1] = (BYTE) (*sectors % 0x0100);
    memcpy(&buffer[2], data, 768);
    length = 2;

    /* Execute the function */
    rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_WRITE_SECT, buffer, sizeof(buffer), buffer, &length);
    if ((rc == MI_OK) && (length == 2))
    {
      sectors_ok = buffer[0];
      sectors_ok *= 0x0100;
      sectors_ok |= buffer[1];
      *sectors = sectors_ok;
    }

    return rc;

  } else
  {
    /* Function performed manually */
    const BYTE *ptr = data;
    SWORD   rc = MI_OK;

    sectors_ok = 0x0000;

    for (sect = 0; sect < 16; sect++)
    {
      if ((*sectors) & (1 << sect))
      {
        /* Read/Write this sector */
        rc = MifStRWSector(SPROX_PARAM_P  TRUE, snr, sect, (BYTE *) ptr);
        if (rc == MI_OK)
        {
          /* Success with this sector */
          sectors_ok |= (1 << sect);
        } else if ((rc != MI_AUTHERR) && (rc != MI_NOTAUTHERR))
        {
          /* Fatal (?) error */
          break;
        } else
        {
          /* Dummy OK for this one */
          rc = MI_OK;
        }
      }
      /* Next sector */
      ptr += 48;
    }

    *sectors = sectors_ok;
    return rc;
  }

}


/*
 * Internal functions to encode / decode a counter
 * -----------------------------------------------
 */
static SWORD SPROX_MifStEncodeCounter(BYTE *buffer, SDWORD value, BYTE addr)
{
  DWORD dw = (DWORD) value;

  if (buffer == NULL)
    return MI_LIB_CALL_ERROR;

  buffer[0]  = (BYTE) dw; dw >>= 8;
  buffer[1]  = (BYTE) dw; dw >>= 8;
  buffer[2]  = (BYTE) dw; dw >>= 8;
  buffer[3]  = (BYTE) dw;

  buffer[4]  = ~buffer[0];
  buffer[5]  = ~buffer[1];
  buffer[6]  = ~buffer[2];
  buffer[7]  = ~buffer[3];

  buffer[8]  = buffer[0];
  buffer[9]  = buffer[1];
  buffer[10] = buffer[2];
  buffer[11] = buffer[3];

  buffer[12] = addr;
  buffer[13] = ~addr;
  buffer[14] = addr;
  buffer[15] = ~addr;

  return MI_OK;
}

static SWORD SPROX_MifStDecodeCounter(BYTE * buffer, SDWORD * value, BYTE * addr)
{
  DWORD   dw;

  if (buffer == NULL)
    return MI_LIB_CALL_ERROR;

  /* Check format */
  if ((buffer[4] != (BYTE) ~ buffer[0]) || (buffer[5] != (BYTE) ~ buffer[1]) || (buffer[6] != (BYTE) ~ buffer[2]) || (buffer[7] != (BYTE) ~ buffer[3]))
    return MI_VALUEERR;

  if ((buffer[8] != buffer[0]) || (buffer[9] != buffer[1]) || (buffer[10] != buffer[2]) || (buffer[11] != buffer[3]))
    return MI_VALUEERR;

  if ((buffer[13] != (BYTE) ~ buffer[12]) || (buffer[14] != buffer[12]) || (buffer[15] != (BYTE) ~ buffer[12]))
    return MI_VALUEERR;

  /* Retrieve value */
  dw = buffer[3];
  dw *= 0x00000100;
  dw += buffer[2];
  dw *= 0x00000100;
  dw += buffer[1];
  dw *= 0x00000100;
  dw += buffer[0];
  if (value != NULL)
    *value = (DWORD) dw;

  /* Retrieve addr */
  if (addr != NULL)
    *addr = buffer[12];

  return MI_OK;
}

/**f* SpringProx.API/SPROX_MifStReadCounter
 *
 * NAME
 *   SPROX_MifStReadCounter
 *
 * DESCRIPTION
 *   Read a Mifare tag counter, using given key or internally available
 *   keys for authentication.
 *
 * INPUTS
 *   const BYTE snr[4]  : 4-byte UID of the Mifare card
 *                        If NULL, the reader will work with currently selected tag
 *   BYTE   bloc        : address of the block holding the counter
 *   SDWORD *value      : pointer to receive current counter value
 *   const BYTE key[6]  : The Mifare access key of the sector (either A or B)
 *                        If NULL, the reader will try all the preloaded keys
 *
 * WARNING
 *   This function works only on Mifare blocks that have been explicitly defined
 *   as counter (access condition ACC_BLOCK_COUNT) and that have been previously
 *   correctly initialized through SPROX_MifStWriteCounter.
 *
 *   For more details, please refer to specific tag documentation.
 *
 * RETURNS
 *   MI_OK              : success, data have been read
 *   MI_NOTAGERR        : the required tag is not available in the RF field,
 *                        or the supplied key has been denied
 *   MI_AUTHERR         : access to the block has been denied
 *   Other code if internal or communication error has occured. 
 *
 * NOTES
 *   The automatic key trying sequence is the same as in SPROX_MifStReadBlock.
 *
 * SEE ALSO
 *   SPROX_MifStWriteCounter
 *   SPROX_MifStDecrementCounter
 *
 **/
SPROX_API_FUNC(MifStReadCounter) (SPROX_PARAM  const BYTE snr[4], BYTE bloc, SDWORD * value, const BYTE key_val[6])
{
  BYTE    data[16];
  SWORD   rc;

  /* Read the block */
  rc = SPROX_API_CALL(MifStReadBlock) (SPROX_PARAM_P  snr, bloc, data, key_val);
  if (rc != MI_OK)
    return rc;

  /* Check if the block is a valid counter */
  rc = SPROX_MifStDecodeCounter(data, value, NULL);
  if (rc != MI_OK)
    return rc;

  /* OK ! */
  return MI_OK;
}

SPROX_API_FUNC(MifStReadCounter2) (SPROX_PARAM  const BYTE snr[4], BYTE bloc, SDWORD * value, BYTE key_idx)
{
  BYTE    data[16];
  SWORD   rc;

  /* Read the block */
  rc = SPROX_API_CALL(MifStReadBlock2) (SPROX_PARAM_P  snr, bloc, data, key_idx);
  if (rc != MI_OK)
    return rc;

  /* Check if the block is a valid counter */
  rc = SPROX_MifStDecodeCounter(data, value, NULL);
  if (rc != MI_OK)
    return rc;

  /* OK ! */
  return MI_OK;
}

/**f* SpringProx.API/SPROX_MifStWriteCounter
 *
 * NAME
 *   SPROX_MifStWriteCounter
 *
 * DESCRIPTION
 *   Write a Mifare tag counter, using given key or internally available
 *   keys for authentication.
 *
 * INPUTS
 *   const BYTE snr[4]  : 4-byte UID of the Mifare card
 *                        If NULL, the reader will work with currently selected tag
 *   BYTE   bloc        : address of the block holding the counter
 *   SDWORD value       : new counter value
 *   const BYTE key[6]  : The Mifare access key of the sector (either B or A)
 *                        If NULL, the reader will try all the preloaded keys
 *
 * WARNING
 *   This function works only on Mifare blocks that have been explicitly defined
 *   as counter (access condition ACC_BLOCK_COUNT).
 *
 *   For more details, please refer to specific tag documentation.
 *
 * RETURNS
 *   MI_OK              : success, data have been read
 *   MI_NOTAGERR        : the required tag is not available in the RF field,
 *                        or the supplied key has been denied
 *   MI_AUTHERR         : access to the block has been denied
 *   Other code if internal or communication error has occured. 
 *
 * NOTES
 *   The automatic key trying sequence is the same as in SPROX_MifStWriteBlock.
 *
 * SEE ALSO
 *   SPROX_MifStReadCounter
 *   SPROX_MifStDecrementCounter
 *
 **/
SPROX_API_FUNC(MifStWriteCounter) (SPROX_PARAM  const BYTE snr[4], BYTE bloc, SDWORD value, const BYTE key_val[6])
{
  BYTE    data[16];
  SWORD   rc;

  /* Format the block as a valid counter */
  rc = SPROX_MifStEncodeCounter(data, value, bloc);
  if (rc != MI_OK)
    return rc;

  /* Write the block */
  rc = SPROX_API_CALL(MifStWriteBlock) (SPROX_PARAM_P  snr, bloc, data, key_val);
  if (rc != MI_OK)
    return rc;

  /* OK ! */
  return MI_OK;
}

SPROX_API_FUNC(MifStWriteCounter2) (SPROX_PARAM  const BYTE snr[4], BYTE bloc, SDWORD value, BYTE key_idx)
{
  BYTE    data[16];
  SWORD   rc;

  /* Format the block as a valid counter */
  rc = SPROX_MifStEncodeCounter(data, value, bloc);
  if (rc != MI_OK)
    return rc;

  /* Write the block */
  rc = SPROX_API_CALL(MifStWriteBlock2) (SPROX_PARAM_P  snr, bloc, data, key_idx);
  if (rc != MI_OK)
    return rc;

  /* OK ! */
  return MI_OK;
}

/**f* SpringProx.API/SPROX_MifStDecrementCounter
 *
 * NAME
 *   SPROX_MifStDecrementCounter
 *
 * DESCRIPTION
 *   Decrement a Mifare tag counter, using given key or internally available
 *   keys for authentication.
 *
 * INPUTS
 *   const BYTE snr[4]  : 4-byte UID of the Mifare card
 *                        If NULL, the reader will work with currently selected tag
 *   BYTE  bloc         : address of the block holding the counter
 *   DWORD value        : number of units the counter is to be decremented
 *   const BYTE key[6]  : The Mifare access key of the sector (either A or B)
 *                        If NULL, the reader will try all the preloaded keys
 *
 * WARNING
 *   This function works only on Mifare blocks that have been explicitly defined
 *   as counter (access condition ACC_BLOCK_COUNT) and that have been previously
 *   correctly initialized through SPROX_MifStWriteCounter.
 *
 *   For more details, please refer to specific tag documentation.
 *
 * RETURNS
 *   MI_OK              : success, data have been read
 *   MI_NOTAGERR        : the required tag is not available in the RF field,
 *                        or the supplied key has been denied
 *   MI_AUTHERR         : access to the block has been denied
 *   Other code if internal or communication error has occured. 
 *
 * NOTES
 *   The automatic key trying sequence is the same as in SPROX_MifStReadBlock.
 *
 * SEE ALSO
 *   SPROX_MifStReadCounter
 *   SPROX_MifStWriteCounter
 *
 **/
SPROX_API_FUNC(MifStCounterOperation) (SPROX_PARAM  const BYTE snr[4], BYTE opcode, BYTE src_bloc, DWORD value, BYTE dst_bloc, const BYTE key[6])
{
  SWORD rc;
  SPROX_PARAM_TO_CTX;

  if (sprox_ctx->sprox_version >= 0x00017100)
  {
    BYTE buffer[13];

    /* Request + Select */
    if (snr != NULL)
    {
      rc = SPROX_API_CALL(MifStSelectAgain) (SPROX_PARAM_P  snr);
      if (rc != MI_OK)
        return rc;
    }

    buffer[0] = opcode;
    buffer[1] = src_bloc;
    buffer[5] = (BYTE) value; value >>= 8;
    buffer[4] = (BYTE) value; value >>= 8;
    buffer[3] = (BYTE) value; value >>= 8;
    buffer[2] = (BYTE) value;
    buffer[6] = dst_bloc;
    
    if (key != NULL)
    {
      memcpy(&buffer[7], key, 6);
      rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_VALUE, buffer, 13, NULL, NULL);
    } else
    {
      rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_VALUE, buffer, 7, NULL, NULL);
    }

  } else
  if (sprox_ctx->sprox_version)
  {  
    BYTE data[4];
    
    data[0] = (BYTE) value; value >>= 8;
    data[1] = (BYTE) value; value >>= 8;
    data[2] = (BYTE) value; value >>= 8;
    data[3] = (BYTE) value;

    if (key == NULL)
    {
      /* Try all available keys */
      /* ---------------------- */

      BYTE    key_id;
      BYTE    key_type;

      for (key_type = 0; key_type < 2; key_type++)
      {
        for (key_id = 0; key_id < sprox_e2_key_count; key_id++)
        {
          /* Request + Select */
          rc = SPROX_API_CALL(MifStSelectAgain) (SPROX_PARAM_P  snr);
          if (rc != MI_OK)
            return rc;

          /* First loop, try as key A, second loop, as key B */
          if (key_type)
            rc = SPROX_API_CALL(MifStAuthE2) (SPROX_PARAM_P  PICC_AUTHENT1B, snr, key_id, src_bloc);
          else
            rc = SPROX_API_CALL(MifStAuthE2) (SPROX_PARAM_P  PICC_AUTHENT1A, snr, key_id, src_bloc);
          if (rc == MI_OK)
          {
            /* Authentication OK ? Let's try to decrement the counter ! */
            rc = SPROX_API_CALL(MifStValue) (SPROX_PARAM_P  PICC_DECREMENT, src_bloc, data, dst_bloc);
            if (rc == MI_OK)
              return MI_OK;     /* Perfect ! */
          }
        }
      }

      for (key_id = 0; key_id < sprox_ram_key_count; key_id++)
      {
			  for (key_type = 0; key_type < 2; key_type++)
				{
					/* Request + Select */
					rc = SPROX_API_CALL(MifStSelectAgain) (SPROX_PARAM_P  snr);
					if (rc != MI_OK)
						return rc;

					/* First step, try A keys, second step, B keys */
					if (key_type)
					  rc = SPROX_API_CALL(MifStAuthRam) (SPROX_PARAM_P  PICC_AUTHENT1B, key_id, src_bloc);
					else
						rc = SPROX_API_CALL(MifStAuthRam) (SPROX_PARAM_P  PICC_AUTHENT1A, key_id, src_bloc);
						
					if (rc == MI_OK)
					{
						/* Authentication OK ? Let's try to decrement the counter ! */
						rc = SPROX_API_CALL(MifStValue) (SPROX_PARAM_P  PICC_DECREMENT, src_bloc, data, dst_bloc);
						return MI_OK;         /* Perfect ! */
					}
				}
      }

    } else
    {
      /* Try supplied key */
      /* ---------------- */
      BYTE    key_type;

      for (key_type = 0; key_type < 2; key_type++)
      {
        /* Request + Select */
        rc = SPROX_API_CALL(MifStSelectAgain) (SPROX_PARAM_P  snr);
        if (rc != MI_OK)
          return rc;

        /* First step, try the key as type A, second step, as type B */
        if (key_type)
          rc = SPROX_API_CALL(MifStAuthKey) (SPROX_PARAM_P  PICC_AUTHENT1B, snr, key, src_bloc);
        else
          rc = SPROX_API_CALL(MifStAuthKey) (SPROX_PARAM_P  PICC_AUTHENT1A, snr, key, src_bloc);

        if (rc == MI_OK)
        {
          /* Authentication OK ? Let's try to change the counter ! */
          rc = SPROX_API_CALL(MifStValue) (SPROX_PARAM_P  opcode, src_bloc, data, dst_bloc);
          if (rc == MI_OK)
            return MI_OK;       /* Perfect ! */
        }
      }
    }  
  } else
  {
    /* The CSB-3 doesn't have this feature */
    /* ----------------------------------- */
    SDWORD  counter;

    /* Read current counter value */
    rc = SPROX_API_CALL(MifStReadCounter) (SPROX_PARAM_P  snr, src_bloc, &counter, key);
    if (rc != MI_OK)
      return rc;

    /* Update counter */
    switch (opcode)
    {
      case PICC_DECREMENT :
        counter -= value;
        break;
      case PICC_INCREMENT :
        counter += value;
        break;
      default :
        break;
    }

    /* Write new counter value */
    rc = SPROX_API_CALL(MifStWriteCounter) (SPROX_PARAM_P  snr, dst_bloc, counter, key);
  }
  
  return rc;
}

SPROX_API_FUNC(MifStCounterOperation2) (SPROX_PARAM  const BYTE snr[4], BYTE opcode, BYTE src_bloc, DWORD value, BYTE dst_bloc, BYTE key_idx)
{
  SWORD rc;
  BYTE  data[4];
  SPROX_PARAM_TO_CTX;

  if (sprox_ctx->sprox_version >= 0x00017100)
  {
    BYTE buffer[8];

    /* Request + Select */
    if (snr != NULL)
    {
      rc = SPROX_API_CALL(MifStSelectAgain) (SPROX_PARAM_P  snr);
      if (rc != MI_OK)
        return rc;
    }

    buffer[0] = opcode;
    buffer[1] = src_bloc;
    buffer[5] = (BYTE) value; value >>= 8;
    buffer[4] = (BYTE) value; value >>= 8;
    buffer[3] = (BYTE) value; value >>= 8;
    buffer[2] = (BYTE) value;
    buffer[6] = dst_bloc;
    buffer[7] = key_idx;
    
    rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_VALUE, buffer, 8, NULL, NULL);

  } else
  if (sprox_ctx->sprox_version >= 0x00014200)
  {
    data[0] = (BYTE) value; value >>= 8;
    data[1] = (BYTE) value; value >>= 8;
    data[2] = (BYTE) value; value >>= 8;
    data[3] = (BYTE) value;
    
    rc = SPROX_API_CALL(MifStAuthKey2) (SPROX_PARAM_P  snr, key_idx, src_bloc);

    if (rc == MI_OK)
    {
      /* Authentication OK ? Let's try to decrement the counter ! */
      rc = SPROX_API_CALL(MifStValue) (SPROX_PARAM_P  opcode, src_bloc, data, dst_bloc);
    }
  } else
    rc = MI_UNKNOWN_FUNCTION;  
  
  
  return rc;
}

SPROX_API_FUNC(MifStDecrementCounter) (SPROX_PARAM  const BYTE snr[4], BYTE bloc, DWORD value, const BYTE key[6])
{
  SPROX_PARAM_TO_CTX;
  return SPROX_API_CALL(MifStCounterOperation) (SPROX_PARAM_P  snr, PICC_DECREMENT, bloc, value, bloc, key);
}

SPROX_API_FUNC(MifStDecrementCounter2) (SPROX_PARAM  const BYTE snr[4], BYTE bloc, DWORD value, BYTE key_idx)
{
  SPROX_PARAM_TO_CTX;
  return SPROX_API_CALL(MifStCounterOperation2) (SPROX_PARAM_P  snr, PICC_DECREMENT, bloc, value, bloc, key_idx);
}

SPROX_API_FUNC(MifStIncrementCounter) (SPROX_PARAM  const BYTE snr[4], BYTE bloc, DWORD value, const BYTE key[6])
{
  SPROX_PARAM_TO_CTX;
  return SPROX_API_CALL(MifStCounterOperation) (SPROX_PARAM_P  snr, PICC_INCREMENT, bloc, value, bloc, key);
}

SPROX_API_FUNC(MifStIncrementCounter2) (SPROX_PARAM  const BYTE snr[4], BYTE bloc, DWORD value, BYTE key_idx)
{
  SPROX_PARAM_TO_CTX;
  return SPROX_API_CALL(MifStCounterOperation2) (SPROX_PARAM_P  snr, PICC_INCREMENT, bloc, value, bloc, key_idx);
}

SPROX_API_FUNC(MifStRestoreCounter) (SPROX_PARAM  const BYTE snr[4], BYTE src_bloc, BYTE dst_bloc, const BYTE key[6])
{
  SPROX_PARAM_TO_CTX;
  return SPROX_API_CALL(MifStCounterOperation) (SPROX_PARAM_P  snr, PICC_RESTORE, src_bloc, 0, dst_bloc, key);
}

SPROX_API_FUNC(MifStRestoreCounter2) (SPROX_PARAM  const BYTE snr[4], BYTE src_bloc, BYTE dst_bloc, BYTE key_idx)
{
  SPROX_PARAM_TO_CTX;
  return SPROX_API_CALL(MifStCounterOperation2) (SPROX_PARAM_P  snr, PICC_RESTORE, src_bloc, 0, dst_bloc, key_idx);
}


/**f* SpringProx.API/SPROX_MifStUpdateAccessBlock
 *
 * NAME
 *   SPROX_MifStUpdateAccessBlock
 *
 * DESCRIPTION
 *   Write one a "correctly formated" sector trailer in a Mifare tag.
 *
 * INPUTS
 *   const BYTE snr[4]       : 4-byte UID of the Mifare card
 *                             If NULL, the reader will work with currently selected tag
 *   BYTE sect               : address of the sector (not of the block !!!)
 *   const BYTE old_key[6]   : current Mifare access key of the sector (either B or A)
 *                             If NULL, the reader will try all the preloaded keys 
 *   const BYTE new_key_A[6] : new A access key of the sector
 *   const BYTE new_key_A[B] : new B access key of the sector
 *   BYTE ac0                : access condition for block 0
 *   BYTE ac1                : access condition for block 1
 *   BYTE ac2                : access condition for block 2
 *   BYTE ac3                : access condition for block 3 (sector trailer itself)
 *
 * WARNING
 *   Please refer to specific tag documentation for explanation of ac0, ac1, ac2
 *   and ac3, or use only the values enumerated below :
 *   - ACC_BLOCK_VALUE or ACC_BLOCK_COUNT for ac0, ac1 and ac2
 *   - ACC_AUTH_TRANSPORT or ACC_AUTH_NORMAL for ac3
 *
 *   You can also read ClassicAccessConditions for more details.
 *
 *   Setting bogus access conditions can irremediably lock the sector !
 *
 * RETURNS
 *   MI_OK              : success, access condition have been written
 *   MI_NOTAGERR        : the required tag is not available in the RF field,
 *                        or the supplied key has been denied
 *   MI_AUTHERR         : access to the sector has been denied
 *   Other code if internal or communication error has occured. 
 *
 * NOTES
 *   The automatic key trying sequence is the same as in SPROX_MifStWriteBlock.  
 *
 *   If new_key_A is NULL, the reader will write Mifare default 'A' base key
 *   ( 0xA0,0xA1,0xA2,0xA3,0xA4,0xA5 ).
 *   If new_key_B is NULL, the reader will write Mifare default 'B' base key
 *   ( 0xB0,0xB1,0xB2,0xB3,0xB4,0xB5 ).
 *
 * SEE ALSO
 *   SPROX_MifLoadKey
 *
 **/
 
/**d* SpringProx.API/ACC_BLOCK_VALUE
 *
 * NAME
 *   ACC_BLOCK_VALUE
 *
 * DESCRIPTION
 *   Recommended access condition for a Mifare block used as data storage :
 *   - key 'A' offers read only access
 *   - key 'B' offers read and write access
 *
 * NOTES
 *   This constant is to be used as parameter ac0, ac1 and ac2 of 
 *   SPROX_MifStUpdateAccessBlock.
 *
 **/

/**d* SpringProx.API/ACC_BLOCK_COUNT
 *
 * NAME
 *   ACC_BLOCK_COUNT
 *
 * DESCRIPTION
 *   Recommended access condition for a Mifare block used as a counter :
 *   - key 'A' offers read and decrement only access
 *   - key 'B' offers read, write, decrement and increment access
 *
 * NOTES
 *   This constant is to be used as parameter ac0, ac1 and ac2 of 
 *   SPROX_MifStUpdateAccessBlock.
 *
 **/

/**d* SpringProx.API/ACC_AUTH_NORMAL
 *
 * NAME
 *   ACC_AUTH_NORMAL
 *
 * DESCRIPTION
 *   Recommended access condition for a Mifare sector trailer :
 *   - key 'A' doesn't give access to sector trailer
 *   - key 'B' gives full access to sector trailer (sectors' master key)
 *
 * NOTES
 *   This constant is to be used as parameter ac3 of SPROX_MifStUpdateAccessBlock.
 *
 **/

/**d* SpringProx.API/ACC_AUTH_TRANSPORT
 *
 * NAME
 *   ACC_AUTH_TRANSPORT
 *
 * DESCRIPTION
 *   "Transport" access condition of a Mifare sector trailer :
 *   - key 'A' gives full access to sector trailer (sectors' master key)
 *   - key 'B' is not used for authentication (and thus readable with key 'A')
 *
 * NOTES
 *   This constant is to be used as parameter ac3 of SPROX_MifStUpdateAccessBlock.
 *
 **/

/**** SpringProx.API/ClassicAccessConditions
 *
 * NAME
 *   ClassicAccessConditions
 *
 * DESCRIPTION
 *   Mifare classic 1k access conditions according to Philips' reference documentation.
 *
 * NOTES
 *
 *  For blocks 0 to 2, the access condition bits-field accept following values :
 *  -----------------
 *  
 *     Read       Write      Incr       Decrement
 *                                      Transfert
 *                                      Restore
 *  ---------------------------------------------
 *  00 KeyA|KeyB  KeyA|KeyB  KeyA|KeyB  KeyA|KeyB
 *  01 KeyA|KeyB  never      never      KeyA|KeyB
 *  02 KeyA|KeyB  never      never      never
 *  03 KeyB       KeyB       never      never
 *  04 KeyA|KeyB  KeyB       never      never
 *  05 KeyB       never      never      never
 *  06 KeyA|KeyB  KeyB       KeyB       KeyA|KeyB
 *  07 never      never      never      never
 *
 *  For block 3 (sector trailer), the access condition bits-field accept following values :
 *  ----------------------------
 *  
 *     KeyA       KeyA       Acc.cond.  Acc.cond.  KeyB       KeyB 
 *     Read       Write      Read       Write      Read       Write 
 *  -------------------------------------------------------------------
 *  00 never      KeyA       KeyA       never      KeyA       KeyA
 *  01 never      KeyA       KeyA       KeyA       KeyA       KeyA
 *  02 never      never      KeyA       never      KeyA       never
 *  03 never      KeyB       KeyA|KeyB  KeyB       never      KeyB
 *  04 never      KeyB       KeyA|KeyB  never      never      KeyB 
 *  05 never      never      KeyA|KeyB  KeyB       never      never
 *  06 never      never      KeyA|KeyB  never      never      never
 *  07 never      never      KeyA|KeyB  never      never      never 
 *
 *  The access condition 01 allows KeyB to be used for data storage.
 *  Use this feature carefully !
 *
 *  Setting access condition 00, 02, 04, 06 or 07 may be a bad idea !
 *
 **/

#define Bit2(a) ((a&0x04)?1:0)
#define Bit1(a) ((a&0x02)?1:0)
#define Bit0(a) ((a&0x01)?1:0)

static void MifStBuildAccessBlock(BYTE data[16], const BYTE new_key_A[6], const BYTE new_key_B[6], BYTE ac0, BYTE ac1, BYTE ac2, BYTE ac3)
{  
  /* First step, write the keys into the access condition buffer */
  if (new_key_A != NULL)
  {
    memcpy(&data[0], new_key_A, 6);
  } else
  {
    BYTE    default_key_A[6] = { 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5 };
    memcpy(&data[0], default_key_A, 6);
  }
  if (new_key_B != NULL)
  {
    memcpy(&data[10], new_key_B, 6);
  } else
  {
    BYTE    default_key_B[6] = { 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5 };
    memcpy(&data[10], default_key_B, 6);
  }

  /* Second step, build the access conditions dword */
  {
    BYTE    C3, C2, C1;

    C3 = Bit0(ac3);
    C3 <<= 1;
    C3 |= Bit0(ac2);
    C3 <<= 1;
    C3 |= Bit0(ac1);
    C3 <<= 1;
    C3 |= Bit0(ac0);

    C2 = Bit1(ac3);
    C2 <<= 1;
    C2 |= Bit1(ac2);
    C2 <<= 1;
    C2 |= Bit1(ac1);
    C2 <<= 1;
    C2 |= Bit1(ac0);

    C1 = Bit2(ac3);
    C1 <<= 1;
    C1 |= Bit2(ac2);
    C1 <<= 1;
    C1 |= Bit2(ac1);
    C1 <<= 1;
    C1 |= Bit2(ac0);

    data[6] = (~(C2 << 4) & 0xF0) | (~(C1) & 0x0F);
    data[7] = ((C1 << 4) & 0xF0) | (~(C3) & 0x0F);
    data[8] = ((C3 << 4) & 0xF0) | (C2 & 0x0F);
    data[9] = 0x00;
  }
}

SPROX_API_FUNC(MifStUpdateAccessBlock) (SPROX_PARAM  const BYTE snr[4], BYTE sect, const BYTE old_key[6], const BYTE new_key_A[6], const BYTE new_key_B[6], BYTE ac0, BYTE ac1, BYTE ac2, BYTE ac3)
{
  BYTE    data[16];
  BYTE    bloc;
  SWORD   rc;
  SPROX_PARAM_TO_CTX;

  /* Choose the right block */
  if (sect < 32)
  {
    bloc = sect;
    bloc *= 4;
    bloc += 3;
  } else
  {
    bloc = sect;
    bloc -= 32;
    bloc *= 16;
    bloc += 128;
    bloc += 15;
  }

  MifStBuildAccessBlock(data, new_key_A, new_key_B, ac0, ac1, ac2, ac3);

  /* Try with embedded functions */
  /* --------------------------- */
  if (old_key == NULL)
    rc = MifStRWBlock(SPROX_PARAM_P  TRUE, snr, bloc, data);
  else
    rc = MifStRWBlockKey(SPROX_PARAM_P  TRUE, snr, bloc, data, old_key);

  if (rc == MI_OK)
    return MI_OK;               /* Nice ! */

  /* Since key A must be tried first for transport condition, we can't rely only on embedded functions */
  /* ------------------------------------------------------------------------------------------------- */

  if (old_key == NULL)
  {
    BYTE    key_id;
    BYTE    key_type;

    if (sprox_ctx->sprox_version)
    {
      /* Try all EEPROM keys */
      for (key_type = 0; key_type < 2; key_type++)
      {
        for (key_id = 0; key_id < sprox_e2_key_count; key_id++)
        {
          /* Request + Select */
          rc = SPROX_API_CALL(MifStSelectAgain) (SPROX_PARAM_P  snr);
          if (rc != MI_OK)
            return rc;

          /* First loop, try as key A, second loop, as key B */
          if (key_type)
            rc = SPROX_API_CALL(MifStAuthE2) (SPROX_PARAM_P  PICC_AUTHENT1B, snr, key_id, bloc);
          else
            rc = SPROX_API_CALL(MifStAuthE2) (SPROX_PARAM_P  PICC_AUTHENT1A, snr, key_id, bloc);
          if (rc == MI_OK)
          {
            /* Authentication OK ? Let's try to Write the block ! */
            rc = SPROX_API_CALL(MifWrite) (SPROX_PARAM_P  NULL, bloc, data);
            if (rc == MI_OK)
              return MI_OK;     /* Perfect ! */
          }
        }
      }
    }

    /* Try all RAM keys */
    for (key_id = 0; key_id < sprox_ram_key_count; key_id++)
    {
      for (key_type = 0; key_type < 2; key_type++)
      {
				/* Request + Select */
				rc = SPROX_API_CALL(MifStSelectAgain) (SPROX_PARAM_P  snr);
				if (rc != MI_OK)
					return rc;

				/* First step, try A keys, second step, B keys */
				if (key_type)
					rc = SPROX_API_CALL(MifStAuthRam) (SPROX_PARAM_P  PICC_AUTHENT1B, key_id, bloc);
				else
					rc = SPROX_API_CALL(MifStAuthRam) (SPROX_PARAM_P  PICC_AUTHENT1A, key_id, bloc);
				if (rc == MI_OK)
				{
					/* Authentication OK ? Let's try to Write the block ! */
					rc = SPROX_API_CALL(MifWrite) (SPROX_PARAM_P  NULL, bloc, data);
					if (rc == MI_OK)
						return MI_OK;
				}
			}
    }
  } else
  {
    /* Try supplied key */
    BYTE key_type;

    if (!sprox_ctx->sprox_version)
      return MI_FUNCTION_NOT_AVAILABLE; /* CSB-3 doesn't allow this... */

    for (key_type = 0; key_type < 2; key_type++)
    {
      /* Request + Select */
      rc = SPROX_API_CALL(MifStSelectAgain) (SPROX_PARAM_P  snr);
      if (rc != MI_OK)
        return rc;

      /* First step, try the key as type A, second step, as type B */
      if (key_type)
        rc = SPROX_API_CALL(MifStAuthKey) (SPROX_PARAM_P  PICC_AUTHENT1B, snr, old_key, bloc);
      else
        rc = SPROX_API_CALL(MifStAuthKey) (SPROX_PARAM_P  PICC_AUTHENT1A, snr, old_key, bloc);

      if (rc == MI_OK)
      {
        /* Authentication OK ? Let's try to Write the block ! */
        rc = SPROX_API_CALL(MifWrite) (SPROX_PARAM_P  NULL, bloc, data);
        if (rc == MI_OK)
          return MI_OK;
      }
    }
  }

  return rc;
}

SPROX_API_FUNC(MifStUpdateAccessBlock2) (SPROX_PARAM  const BYTE snr[4], BYTE sect, BYTE old_key_idx, const BYTE new_key_A[6], const BYTE new_key_B[6], BYTE ac0, BYTE ac1, BYTE ac2, BYTE ac3)
{
  BYTE    data[16];
  BYTE    bloc;

  /* Choose the right block */
  if (sect < 32)
  {
    bloc = sect;
    bloc *= 4;
    bloc += 3;
  } else
  {
    bloc = sect;
    bloc -= 32;
    bloc *= 16;
    bloc += 128;
    bloc += 15;
  }

  MifStBuildAccessBlock(data, new_key_A, new_key_B, ac0, ac1, ac2, ac3);

  return MifStRWBlockKey2(SPROX_PARAM_P  TRUE, snr, bloc, data, old_key_idx);
}



/*
 * Load keys to EEPROM (CSB-4, SpringProx) or to RAM (CSB-3)
 * ---------------------------------------------------------
 */ 
SPROX_API_FUNC(MifLoadKeyE2) (SPROX_PARAM  BYTE key_type, BYTE key_offset, const BYTE key_data[6])
{
  BYTE buffer[8];
  SPROX_PARAM_TO_CTX;
  	
  if (!sprox_ctx->sprox_version)
    return MI_FUNCTION_NOT_AVAILABLE;
    
  if (key_data == NULL)
  	return MI_LIB_CALL_ERROR;

  buffer[0] = key_type;
  buffer[1] = key_offset;
  memcpy(&buffer[2], key_data, 6);

  return SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_PCDLOADKEYE2, buffer, sizeof(buffer), NULL, NULL);
}

SPROX_API_FUNC(MifLoadKeyRam) (SPROX_PARAM  BYTE key_type, BYTE key_offset, const BYTE key_data[6])
{
  BYTE buffer[14];
//  SPROX_PARAM_TO_CTX;  

  if ((key_type != PICC_AUTHENT1A) && (key_type != PICC_AUTHENT1B))
    return MI_LIB_CALL_ERROR;

  if (key_data == NULL)
  	return MI_LIB_CALL_ERROR;

  if (key_type == PICC_AUTHENT1B)
    key_offset += sprox_ram_key_count;

  if (key_offset >= 2*sprox_ram_key_count)
    return MI_LIB_CALL_ERROR;

  buffer[0]  = key_offset;
  buffer[1]  = 0x00;
  buffer[2]  = 0xBD; buffer[3] = 0xDE; buffer[4] = 0x6F; buffer[5] = 0x37; buffer[6] = 0x83; buffer[7] = 0x83;
  memcpy(&buffer[8], key_data, 6);
 
  return SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_LOAD_KEY, buffer, sizeof(buffer), NULL, NULL);
}
 
/**f* SpringProx.API/SPROX_MifLoadKey
 *
 * NAME
 *   SPROX_MifLoadKey
 *
 * DESCRIPTION
 *   Load Mifare key into the reader. It will be used by every function
 *   performing automatic key trying.
 *
 * INPUTS
 *   BOOL eeprom            : set to TRUE for EEPROM storage
 *                            set to FALSE for temporary RAM storage
 *   BYTE key_type          : set to 'A' for a Mifare A key
 *                            set to 'B' for a Mifare B key
 *   BYTE key_offset        : index of the key into the memory
 *                            - 0 to 15 for EEPROM storage
 *                            - 0 to 3 for RAM storage
 *   const BYTE key_data[6] : value of the key
 *
 * RETURNS
 *   MI_OK              : success, key has been written
 *   Other code if internal or communication error has occured. 
 *
 * WARNING
 *   EEPROM keys are stored in the RC chipset EEPROM. Do not write the keys
 *   "too often" since an EEPROM has a limited lifetime depending on the
 *   write operations count.
 *
 *   Typically, load your application keys in EEPROM once at product install,
 *   then forget it...
 *
 * NOTES
 *   There's not mean to read back the loaded keys.
 *
 *   Highest performance will be achieved when the right authentication key for
 *   a block or a sector is available at the beginning of the RAM or of
 *   the EEPROM !
 *
 * SEE ALSO
 *   SPROX_MifLastAuthKey
 * 
 **/
SPROX_API_FUNC(MifLoadKey) (SPROX_PARAM  BOOL eeprom, BYTE key_type, BYTE key_offset, const BYTE key_data[6])
{
  SPROX_PARAM_TO_CTX;
  
  if (eeprom)
    if (!sprox_ctx->sprox_version)
      return MI_FUNCTION_NOT_AVAILABLE;

  if (key_data == NULL)
    return MI_LIB_CALL_ERROR;

  if (eeprom)
  {
    if (key_offset >= sprox_e2_key_count)
      return MI_LIB_CALL_ERROR;

    if ((key_type == 0) || (key_type == 'a') || (key_type == 'A') || (key_type == PICC_AUTHENT1A))
      return SPROX_API_CALL(MifLoadKeyE2) (SPROX_PARAM_P  PICC_AUTHENT1A, key_offset, (BYTE *) key_data);

    if ((key_type == 1) || (key_type == 'b') || (key_type == 'B') || (key_type == PICC_AUTHENT1B))
      return SPROX_API_CALL(MifLoadKeyE2) (SPROX_PARAM_P  PICC_AUTHENT1B, key_offset, (BYTE *) key_data);

  } else
  {
    if (key_offset >= sprox_ram_key_count)
      return MI_LIB_CALL_ERROR;

    if ((key_type == 0) || (key_type == 'a') || (key_type == 'A') || (key_type == PICC_AUTHENT1A))
      return SPROX_API_CALL(MifLoadKeyRam) (SPROX_PARAM_P  PICC_AUTHENT1A, key_offset, (BYTE *) key_data);

    if ((key_type == 1) || (key_type == 'b') || (key_type == 'B') || (key_type == PICC_AUTHENT1B))
      return SPROX_API_CALL(MifLoadKeyRam) (SPROX_PARAM_P  PICC_AUTHENT1B, key_offset, (BYTE *) key_data);
  }

  return MI_LIB_CALL_ERROR;
}

SPROX_API_FUNC(MifLoadKeyEx) (SPROX_PARAM  BYTE key_idx, const BYTE key_data[6])
{
  BOOL eeprom;
  BYTE key_type;
  BYTE key_offset;
  
  eeprom     = (key_idx & MIF_E2_KEY) ? TRUE : FALSE;
  key_type   = (key_idx & MIF_KEY_B)  ? PICC_AUTHENT1B : PICC_AUTHENT1A;
  key_offset = (key_idx & 0x1F);
  
  return SPROX_API_CALL(MifLoadKey) (SPROX_PARAM_P  eeprom, key_type, key_offset, key_data);
}

/**f* SpringProx.API/SPROX_MifLastAuthKey
 *
 * NAME
 *   SPROX_MifLastAuthKey
 *
 * DESCRIPTION
 *   Returns the last Mifare key that has allowed a successfull read or
 *   write operation.
 *
 * INPUTS
 *   BYTE *info         : pointer to a byte to return the expected information 
 *
 * RETURNS
 *   MI_OK              : success, data have been read
 *   Other code if internal or communication error has occured. 
 *
 * NOTES
 *   Returned value for info is a bit-field coded as follow :
 *
 *   xxxxxxxx
 *      +++++-- key number   : offset of the key in RAM or EEPROM
 *     +------- key type     : 1 for a 'B' key, 0 for an 'A' key
 *   ++-------- key provider : 10 for a key found in reader's RAM
 *                             01 for a key found in RC chipset's EEPROM
 *                             11 if the key has been passed directly to
 *                                the function (no automatic discovery). 
 *                             00 is RFU
 *
 **/
SPROX_API_FUNC(MifLastAuthKey) (SPROX_PARAM  BYTE *info)
{
  SWORD rc;
  SPROX_PARAM_TO_CTX;

  if (sprox_ctx->sprox_version)
  {
    BYTE    buffer[1];
    WORD    len = sizeof(buffer);
    buffer[0] = SPROX_CONTROL_MIF_LASTKEY;
    rc = SPROX_API_CALL(ControlEx) (SPROX_PARAM_P  buffer, len, buffer, &len);
    if (rc != MI_OK)
      return rc;

    sprox_ctx->mif_auth_info = buffer[0];
  }

  if (info != NULL)
    *info = sprox_ctx->mif_auth_info;

  return MI_OK;
}


#endif
