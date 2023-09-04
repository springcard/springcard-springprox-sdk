/**h* SpringProx.API/ISO14443-3
 *
 * NAME
 *   SpringProx.API :: ISO 14443 Layer 3 (A and B)
 *
 * DESCRIPTION
 *   Implementation of ISO/IEC 14443 layer 3
 *
 **/

/*

  SpringProx API
  --------------

  Copyright (c) 2000-2012 SpringCard SAS, FRANCE - www.springcard.com

  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  History
  -------
 
  JDA 22/06/2006 : created with ISO 14443-3 functions removed from sprox_fct.c
                   and some parts removed from sprox_tcl.c
  JDA 23/06/2006 : added type B anticollision
  JDA 30/10/2007 : removed silly parameter *sak from SelectAgain prototype
  JDA 30/01/2012 : corrected length of ATQB (11 bytes), prepared for extended ATQB (12 bytes)

*/
#include "sprox_api_i.h"

#include "micore_picc.h"

SWORD SaveASnr(SPROX_PARAM  const BYTE snr[], BYTE snrlen)
{
  SPROX_PARAM_TO_CTX;
  SPROX_Trace(TRACE_DEBUG, "MifSnr <- %02X%02X%02X%02X (%d)", snr[0], snr[1], snr[2], snr[3], snrlen);

  if (snr != sprox_ctx->mif_snr)
  {
    memset(sprox_ctx->mif_snr, 0, sizeof(sprox_ctx->mif_snr));
    if (snrlen > sizeof(sprox_ctx->mif_snr))
      snrlen = sizeof(sprox_ctx->mif_snr);
    if (snr != NULL)
      memcpy(sprox_ctx->mif_snr, snr, snrlen);
  }
  return MI_OK;
}

SPROX_API_FUNC(A_Request) (SPROX_PARAM  BYTE req_code, BYTE atq[2])
{
  WORD len = 2;
  SWORD rc;
  SPROX_PARAM_TO_CTX;
  
  rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_14443_A);
  if ((rc != MI_OK) && (rc != MI_UNKNOWN_FUNCTION)) return rc;
  
  if (sprox_ctx->sprox_version)
    rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_PICCCOMMONREQUEST, &req_code, 1, atq, &len);
  else
    rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_REQUEST, &req_code, 1, atq, &len);
  
  return rc;
}

SPROX_API_FUNC(A_RequestAny) (SPROX_PARAM  BYTE atq[2])
{
  SWORD rc;
  SPROX_PARAM_TO_CTX;  

  rc = SPROX_API_CALL(A_Request) (SPROX_PARAM_P  PICC_REQALL, atq);
  if (rc != MI_OK)
    rc = SPROX_API_CALL(A_Request) (SPROX_PARAM_P  PICC_REQALL, atq);

  /* CSB3 hack ! */
  if ((!sprox_ctx->sprox_version) && (rc == -255))
    rc = -1;

  SPROX_Trace(TRACE_DEBUG, "A_RequestAny -> %d", rc);
  return rc;
}

SPROX_API_FUNC(A_RequestIdle) (SPROX_PARAM  BYTE atq[2])
{
  SWORD rc;
  SPROX_PARAM_TO_CTX;

  rc = SPROX_API_CALL(A_Request) (SPROX_PARAM_P  PICC_REQIDL, atq);
  if (rc != MI_OK)
    rc = SPROX_API_CALL(A_Request) (SPROX_PARAM_P  PICC_REQIDL, atq);

  /* CSB3 hack ! */
  if ((!sprox_ctx->sprox_version) && (rc == -255))
    rc = -1;

  SPROX_Trace(TRACE_DEBUG, "A_RequestIdle -> %d", rc);
  return rc;
}

SPROX_API_FUNC(A_Select) (SPROX_PARAM  const BYTE *snr, BYTE *sak)
{
  SWORD rc;
  WORD  len = 1;
  SPROX_PARAM_TO_CTX;
  
  rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_14443_A);
  if ((rc != MI_OK) && (rc != MI_UNKNOWN_FUNCTION)) return rc;

  if (sprox_ctx->sprox_version)
  {
    BYTE  buffer[5];
    buffer[0] = PICC_ANTICOLL1;
    memcpy(&buffer[1], snr, 4);
    rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_PICCCASCSELECT, buffer, sizeof(buffer), sak, &len);
  } else
  {
    rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_SELECT, snr, 4, sak, &len);
  }
   
  if (rc == MI_OK)
    SaveASnr(SPROX_PARAM_P  snr, 4);
  return rc;
}

SPROX_API_FUNC(A_Anticoll) (SPROX_PARAM  BYTE snr[4])
{
  SWORD rc;
  SPROX_PARAM_TO_CTX;
  
  rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_14443_A);
  if ((rc != MI_OK) && (rc != MI_UNKNOWN_FUNCTION)) return rc;

  if (sprox_ctx->sprox_version)
  {
    BYTE    buffer[2];
    WORD    len = 4;
    buffer[0] = PICC_ANTICOLL1;
    buffer[1] = 0;
    rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_PICCCASCANTICOLL, buffer, sizeof(buffer), snr, &len);
  } else
  {
    WORD    len = 4;
    BYTE    buffer[5];
    buffer[0] = 0;
    rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_ANTICOLL, buffer, sizeof(buffer), snr, &len);
  }
  
  return rc;
}

/**f* SpringProx.API/SPROX_A_SelectAny
 *
 * NAME
 *   SPROX_A_SelectAny
 *
 * DESCRIPTION
 *   Select "Any" ISO 14443-A card available in the RF field (i.e., the first card
 *   in the IDLE or HALTed state that passes the WUPA + SELA anticollision loop)
 *
 * INPUTS
 *   BYTE atq[2]        : 2-byte buffer to receive card's Answer To Query
 *   BYTE snr[10]       : 4 to 10-byte buffer to receive card's Unique ID
 *   BYTE *snrlen       : on input, size of snr
 *                        on ouput, actual length of received Unique ID
 *   BYTE sak[1]        : 1-byte buffer to receive card's Select AcKnowledge
 *
 * RETURNS
 *   MI_OK              : success, card selected
 *   MI_NOTAGERR        : no card available in the RF field
 *   Other code if internal or communication error has occured. 
 *
 * NOTES
 *   Please refer to Philips "Mifare Type Identification Procedure" documentation
 *   for atq and sak explanation.
 *
 *   The Unique ID size is to be found in atq[0] :
 *   (atq[0] & 0xC0 == 0x00) --> single size UID (snr is 4-byte long)
 *   (atq[0] & 0xC0 == 0x40) --> double size UID (snr is 7-byte long)
 *   (atq[0] & 0xC0 == 0x80) --> triple size UID (snr is 10-byte long) 
 *
 * SEE ALSO
 *   SPROX_A_SelectIdle
 *   SPROX_A_Halt
 *   SPROX_MifStSelectAny
 *   SPROX_TclA_ActivateAny
 *
 **/
SPROX_API_FUNC(A_SelectAny) (SPROX_PARAM  BYTE atq[2], BYTE snr[10], BYTE *snrlen, BYTE sak[1])
{
  SWORD rc;
  SPROX_PARAM_TO_CTX;  

  if (snr == NULL) return MI_LIB_CALL_ERROR;
  if (atq == NULL) return MI_LIB_CALL_ERROR;
  if (sak == NULL) return MI_LIB_CALL_ERROR;
  
  rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_14443_A);
  if ((rc != MI_OK) && (rc != MI_UNKNOWN_FUNCTION)) return rc;

  if (sprox_ctx->sprox_version)
  {
    BYTE    buffer[32];
    WORD    len = sizeof(buffer);

    rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_SELECT_ANY, NULL, 0, buffer, &len);
    if (rc == MI_OK)
    {
      SaveASnr(SPROX_PARAM_P  &buffer[0], (BYTE) (len - 3));

      if (atq != NULL)
      {
        atq[0] = buffer[len - 3];
        atq[1] = buffer[len - 2];
      }
      if (sak != NULL)
      {
        sak[0] = buffer[len - 1];
      }            
      if (snrlen != NULL)
      {
        if ((*snrlen != 0) && (*snrlen < (len - 3)))
          return MI_RESPONSE_OVERFLOW;
        *snrlen = len - 3;
      }
      if (snr != NULL)
      {
        memcpy(snr, &buffer[0], len - 3);
      }        
    }
  } else
  {
    /* Request */
    rc = SPROX_API_CALL(A_RequestAny) (SPROX_PARAM_P  atq);
    if (rc == MI_OK)
    {
      /* Anticoll */
      rc = SPROX_API_CALL(A_Anticoll) (SPROX_PARAM_P  snr);
      if (rc != MI_OK)
        rc = SPROX_API_CALL(A_Anticoll) (SPROX_PARAM_P  snr);
      if (rc == MI_OK)
      {
        /* Select */
        rc = SPROX_API_CALL(A_Select) (SPROX_PARAM_P  snr, sak);
        if (rc != MI_OK)
          rc = SPROX_API_CALL(A_Select) (SPROX_PARAM_P  snr, sak);
        if (rc == MI_OK)
        {
          SaveASnr(SPROX_PARAM_P  snr, 4);
          if (snrlen != NULL)
          {
            if ((*snrlen != 0) && (*snrlen < 4))
              return MI_RESPONSE_OVERFLOW;              
            *snrlen = 4;
          }
        }
      }
    }
    /* CSB3 hack */
    if (rc == -255)
      rc = -1;
  }

  SPROX_Trace(TRACE_DEBUG, "A_SelectAny -> %d", rc);

  return rc; 
}

/**f* SpringProx.API/SPROX_A_SelectIdle
 *
 * NAME
 *   SPROX_A_SelectIdle
 *
 * DESCRIPTION
 *   Same as SPROX_A_SelectAny, but will return only a card that is in the IDLE
 *   state.
 *   This allow discovering all ISO 14443-A available in the RF field :
 *   - call SPROX_A_SelectIdle to discover the first card
 *   - halt this card calling SPROX_A_Halt
 *   - call SPROX_A_SelectIdle again to discover next card, and so on...
 *
 * INPUTS
 *   BYTE atq[2]        : 2-byte buffer to receive card's Answer To Query
 *   BYTE snr[10]       : 10-byte buffer to receive card's Unique ID
 *   BYTE *snrlen       : on input, size of snr
 *                        on ouput, actual length of received Unique ID
 *   BYTE sak[1]        : 1-byte buffer to receive card's Select AcKnowledge
 *
 * RETURNS
 *   MI_OK              : success, card selected
 *   MI_NOTAGERR        : no IDLE card available in the RF field
 *   Other code if internal or communication error has occured. 
 *
 * NOTES
 *   Despite the name of the function, it doesn't check the returned card type
 *   (i.e., any ISO 14443-A card will be selected, not only Mifare tags).
 *
 * SEE ALSO
 *   SPROX_A_SelectAny
 *   SPROX_A_Halt
 *   SPROX_MifStSelectIdle
 *   SPROX_TclA_ActivateIdle
 *
 **/
SPROX_API_FUNC(A_SelectIdle) (SPROX_PARAM  BYTE atq[2], BYTE snr[10], BYTE *snrlen, BYTE sak[1])
{
  SWORD   rc;
  BYTE    buffer[24];
  WORD    len = 24;

  rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_14443_A);
  if ((rc != MI_OK) && (rc != MI_UNKNOWN_FUNCTION)) return rc;

  buffer[0] = 0;
  rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_PICCACTIVATEIDLE, buffer, 1, buffer, &len);
  if (rc == MI_OK)
  {
    SaveASnr(SPROX_PARAM_P  &buffer[4], buffer[3]);
    
    if (atq != NULL)
    {
      atq[0] = buffer[0];
      atq[1] = buffer[1];
    }
    if (sak != NULL)
    {
      sak[0] = buffer[2];
    }
    if (snrlen != NULL)
    {
      if ((*snrlen != 0) && (*snrlen < buffer[3]))
        return MI_RESPONSE_OVERFLOW;
      *snrlen = buffer[3];
    }
    if (snr != NULL)
    {
      memcpy(snr, &buffer[4], buffer[3]);
    }
  }

  SPROX_Trace(TRACE_DEBUG, "A_SelectIdle -> %d", rc);

  return rc;
}

/**f* SpringProx.API/SPROX_A_SelectAgain
 *
 * NAME
 *   SPROX_A_SelectAgain
 *
 * DESCRIPTION
 *   Allow to re-select an ISO 14443-A card, provided its serial number (the card
 *   must be available in the RF field, either in the IDLE or HALTed state).
 *
 * INPUTS
 *   const BYTE snr[10] : 4 to 10-byte Unique ID of the card to wake-up
 *   BYTE snrlen        : actual length of card's Unique ID
 *
 * RETURNS
 *   MI_OK              : success, card selected
 *   MI_NOTAGERR        : the required card is not available in the RF field
 *   Other code if internal or communication error has occured. 
 *
 * SEE ALSO
 *   SPROX_A_SelectAny
 *   SPROX_A_SelectIdle
 *
 **/
SPROX_API_FUNC(A_SelectAgain) (SPROX_PARAM  const BYTE snr[10], BYTE snrlen)
{
  SWORD rc;
  SPROX_PARAM_TO_CTX;

  rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_14443_A);
  if ((rc != MI_OK) && (rc != MI_UNKNOWN_FUNCTION)) return rc;

  if (sprox_ctx->sprox_version)
  {
    if (snr == NULL)
    {
      /* Changed DLL v.>=1.52 - instead of supplying current DLL's SNR, we send nothing (and let the reader manage it) */
      rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_SELECT_AGAIN, NULL, 0, NULL, NULL);
    } else
    {
      rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_SELECT_AGAIN, snr, snrlen, NULL, NULL);    
    }

  } else
  {
    BYTE    ats[2];
    BYTE    sak;
    sprox_ctx->mif_auth_ok = FALSE;

    if (snr == NULL)
      snr = sprox_ctx->mif_snr;

    rc = SPROX_API_CALL(A_RequestAny) (SPROX_PARAM_P  ats);
    if (rc == MI_OK)
    {
      rc = SPROX_API_CALL(A_Select) (SPROX_PARAM_P  snr, &sak);
      if (rc != MI_OK)
        rc = SPROX_API_CALL(A_Select) (SPROX_PARAM_P  snr, &sak);
    }

    /* CSB3 hack */
    if (rc == -255)
      rc = -1;
  }

  if ((rc == MI_OK) && (snr != NULL) && (snr != sprox_ctx->mif_snr))
    SaveASnr(SPROX_PARAM_P  snr, 4);

  if (snr != NULL)
    SPROX_Trace(TRACE_DEBUG, "A_SelectAgain -> %d", rc);

  return rc; 
}

/**f* SpringProx.API/SPROX_A_Halt
 *
 * NAME
 *   SPROX_A_Halt
 *
 * DESCRIPTION
 *   Send the ISO 14443-A HALT command to the currently selected card.
 *
 * WARNING
 *   If card is already in T=CL mode, you must DESELECT the card instead of
 *   HALTing it.
 *
 * INPUTS
 *   none
 *
 * RETURNS
 *   MI_OK              : success, card halted
 *   Other code if internal or communication error has occured. 
 * 
 * SEE ALSO
 *   SPROX_TclA_Deselect
 *
 **/
SPROX_API_FUNC(A_Halt) (SPROX_PARAM_V)
{
  SWORD rc;
  SPROX_PARAM_TO_CTX;
  
  rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_14443_A);
  if ((rc != MI_OK) && (rc != MI_UNKNOWN_FUNCTION)) return rc;
  
  rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_HALT, NULL, 0, NULL, NULL);  
  
  return rc;
}

/**f* SpringProx.API/SPROX_A_Exchange
 *
 * NAME
 *   SPROX_A_Exchange
 *
 * DESCRIPTION
 *   Low-level frame exchange with currently activated ISO 14443-A card.
 *
 * WARNING
 *   Once card has entered T=CL mode, the high-level frame exchange must
 *   be performed through SPROX_Tcl_Exchange
 *
 * INPUTS
 *   const BYTE send_data[]  : command (buffer PCD->PICC)
 *   WORD       send_bytelen : length of send_data
 *   BYTE       recv_data[]  : answer (buffer PICC->PCD)
 *   WORD      *recv_bytelen : IN  : size of recv_data (max length)
 *                             OUT : actual length of recv_data
 *   BOOL       calc_crc     : if TRUE, the last 2 bytes of send_data
 *                             will be replaced by a valid 14443 CRC_A
 *                             if FALSE, the CRC must have been computed
 *                             by caller
 *   WORD       timeout      : timeout of the exchange, in ETU
 *                             (at 106kbps, 1 ETU = 9,44s)
 *
 * RETURNS
 *   MI_OK              : success, card answer to be found in recv_data
 *   Other code if any error has occured. 
 * 
 * NOTES
 *   Remember that send_bytelen (and recv_bytelen) are the actual length
 *   of the frame, INCLUDING THE CRC (on the 2-last bytes).
 *   Even if calc_crc is TRUE, the caller must provide room in send_data[]
 *   (and in recv_data[]) to store the CRC.
 *
 **/
SPROX_API_FUNC(A_Exchange) (SPROX_PARAM  const BYTE *send_data, WORD send_bytelen, BYTE *recv_data, WORD *recv_bytelen, BYTE append_crc, WORD timeout)
{
  BYTE    send_buffer[SPROX_FRAME_CONTENT_SIZE];
  BYTE    recv_buffer[SPROX_FRAME_CONTENT_SIZE];
  WORD    len;
  SWORD   rc;
  
  rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_14443_A);
  if ((rc != MI_OK) && (rc != MI_UNKNOWN_FUNCTION)) return rc;

  if (send_data == NULL) return MI_LIB_CALL_ERROR;
  if (recv_data == NULL) return MI_LIB_CALL_ERROR;
  if (recv_bytelen == NULL) return MI_LIB_CALL_ERROR;

  if ((send_bytelen + 9) >= SPROX_FRAME_CONTENT_SIZE) return MI_COMMAND_OVERFLOW;

  len = *recv_bytelen + 2;
  if (len >= SPROX_FRAME_CONTENT_SIZE) len = SPROX_FRAME_CONTENT_SIZE;
  
  send_buffer[0] = (BYTE) (send_bytelen / 0x0100);
  send_buffer[1] = (BYTE) (send_bytelen % 0x0100);
  send_buffer[2] = (BYTE) (*recv_bytelen / 0x0100);
  send_buffer[3] = (BYTE) (*recv_bytelen % 0x0100);
  send_buffer[4] = append_crc;
  send_buffer[5] = (BYTE) (timeout / 0x0100);
  send_buffer[6] = (BYTE) (timeout % 0x0100);

  memcpy(&send_buffer[7], send_data, send_bytelen);

  rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_PICCEXCHANGEBLOCK_A, send_buffer, (WORD) (send_bytelen + 7), recv_buffer, &len);

  if (rc == MI_OK)
  {
    *recv_bytelen = recv_buffer[0] * 0x0100 + recv_buffer[1];
    if (*recv_bytelen > (len - 2)) return MI_RESPONSE_INVALID;
    memcpy(recv_data, &recv_buffer[2], *recv_bytelen);
  }

  return rc;
}

SPROX_API_FUNC(A_ExchangeRawEx) (SPROX_PARAM  BYTE param1, BYTE param2, const BYTE send_data[], WORD send_bitslen, BYTE recv_data[], WORD *recv_bitslen, WORD timeout)
{
  BYTE    send_buffer[SPROX_FRAME_CONTENT_SIZE];
  BYTE    recv_buffer[SPROX_FRAME_CONTENT_SIZE];
  WORD    slen, rlen, t;
  SWORD   rc;
  SPROX_PARAM_TO_CTX;  
  
  rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_14443_A);
  if ((rc != MI_OK) && (rc != MI_UNKNOWN_FUNCTION)) return rc;

  if (send_data == NULL) return MI_LIB_CALL_ERROR;
  if (recv_data == NULL) return MI_LIB_CALL_ERROR;
  if (recv_bitslen == NULL) return MI_LIB_CALL_ERROR;

  slen = (send_bitslen + 7) / 8;  
  if ((slen + 6) >= SPROX_FRAME_CONTENT_SIZE) return MI_COMMAND_OVERFLOW;

  send_buffer[0] = param1;
  send_buffer[1] = param2; 
  send_buffer[2] = (BYTE) (send_bitslen / 0x0100);
  send_buffer[3] = (BYTE) (send_bitslen % 0x0100);
  send_buffer[4] = (BYTE) (timeout / 0x0100);
  send_buffer[5] = (BYTE) (timeout % 0x0100);
  
  memcpy(&send_buffer[6], send_data, slen);

  rlen = sizeof(recv_buffer);
  rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_PICCEXCHANGEBLOCK_A_RAW, send_buffer, (WORD) (slen + 6), recv_buffer, &rlen);

  if (rc == MI_OK)
  {
  	t  = recv_buffer[0];
  	t *= 0x0100;
  	t += recv_buffer[1];

  	if (((t + 7) / 8) != (rlen - 2))
  	{
  		rc = MI_RESPONSE_INVALID;
  	} else  	
  	if (t > *recv_bitslen)
  	{
  	  rc = MI_RESPONSE_OVERFLOW;
  	} else
  	{
  	  *recv_bitslen = t;
      memcpy(recv_data, &recv_buffer[2], t);  		
  	}  	
  }

  return rc;
}

SPROX_API_FUNC(A_ExchangeRaw) (SPROX_PARAM  const BYTE send_data[], WORD send_bitslen, BYTE recv_data[], WORD *recv_bitslen, WORD timeout)
{
  SPROX_PARAM_TO_CTX;  

	return SPROX_API_CALL(A_ExchangeRawEx) (SPROX_PARAM_P  0x00, 0x00, send_data, send_bitslen, recv_data, recv_bitslen, timeout);
}

/**f* SpringProx.API/SPROX_B_SelectAny
 *
 * NAME
 *   SPROX_B_SelectAny
 *
 * DESCRIPTION
 *   Select "Any" ISO 14443-B card available in the RF field (i.e., the first
 *   card in the IDLE or HALTed state that answers the REQB command).
 *   There is no anticollision.
 *
 * INPUTS
 *   BYTE afi           : Application Family Identifier. Set to 0x00 for all
 *                        families and all sub-families.
 *   BYTE atq[11]       : 11-byte buffer to receive card's Answer To Query
 *
 * RETURNS
 *   MI_OK              : success, card selected
 *   MI_NOTAGERR        : no card available in the RF field
 *   Other code if internal or communication error has occured. 
 *
 * NOTES
 *   The 4 first bytes of the ATQ are card's Pseudo-Unique Identifier (PUPI) .
 *   You can use the atq buffer directly as paramater for SPROX_TclB_Attrib or 
 *   SPROX_B_Halt
 *
 * SEE ALSO
 *   SPROX_B_SelectIdle
 *   SPROX_B_Halt 
 *   SPROX_TclB_ActivateAny
 *
 **/
SPROX_API_FUNC(B_SelectAny) (SPROX_PARAM  BYTE afi, BYTE atq[11])
{
  SWORD rc;
  SPROX_PARAM_TO_CTX;  

  rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_14443_B);
  if (rc != MI_OK) return rc;

  if (sprox_ctx->sprox_version >= 0x00013500)
  {
    /* Function is inside the reader */
    BYTE buffer[12]; // JDA : bugfix in 1.62.8
    WORD len = 12;    
    rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_SELECT_ANY, &afi, 1, buffer, &len);
    if (rc == MI_OK)
    {
      if (atq != NULL)
        memcpy(atq, &buffer[1], 11);
    }
  } else
  {
    WORD    recv_len;
    BYTE    recv_data[14];
    BYTE    send_data[] = { 0x05, 0x00, 0x08, 0xCC, 0xCC };

    send_data[1] = afi;

    recv_len = sizeof(recv_data);
    rc = SPROX_API_CALL(B_Exchange) (SPROX_PARAM_P  send_data, sizeof(send_data), recv_data, &recv_len, 1, 4096);
    if (rc == MI_OK)
    {
      if (recv_len != 14)
      {
        rc = MI_BYTECOUNTERR;
      } else
      if (recv_data[0] != 0x50)
      {
        rc = MI_CODEERR;
      } else  
      {
        /* OK */
        if (atq != NULL)        
          memcpy(atq, &recv_data[1], 11);   
      }
    }
  }
  
  SPROX_Trace(TRACE_DEBUG, "B_SelectAny -> %d", rc);  
  return rc;
}

/**f* SpringProx.API/SPROX_B_SelectIdle
 *
 * NAME
 *   SPROX_B_SelectIdle
 *
 * DESCRIPTION
 *   Same as SPROX_TclB_ActivateAny, but will return only a card that is in the
 *   IDLE state (sends REQB instead of WUPB).
 *   There is no anticollision.
 *
 * INPUTS
 *   BYTE afi           : Application Family Identifier. Set to 0x00 for all
 *                        families and all sub-families.
 *   BYTE atq[11]       : 11-byte buffer to receive card's Answer To Query
 *
 * RETURNS
 *   MI_OK              : success, tag selected
 *   MI_NOTAGERR        : no tag available in the RF field
 *   Other code if internal or communication error has occured. 
 *
 * NOTES
 *   The 4 first bytes of the ATQ are card's Pseudo-Unique Identifier (PUPI) .
 *   You can use the atq buffer directly as paramater for SPROX_TclB_Attrib or 
 *   SPROX_B_Halt
 *
 * SEE ALSO
 *   SPROX_B_SelectAny
 *   SPROX_B_Halt 
 *   SPROX_TclB_ActivateIdle
 *
 **/
SPROX_API_FUNC(B_SelectIdle) (SPROX_PARAM  BYTE afi, BYTE atq[11])
{
  SWORD rc;
  SPROX_PARAM_TO_CTX;
  
  rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_14443_B);
  if (rc != MI_OK) return rc;

  if (sprox_ctx->sprox_version >= 0x00013500)
  {
    /* Function is inside the reader */
    BYTE buffer[12]; // JDA : bugfix in 1.62.8
    WORD len = 12;    
    rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_SELECT_IDLE, &afi, 1, buffer, &len);
    if (rc == MI_OK)
    {
      if (atq != NULL)
        memcpy(atq, &buffer[1], 11);
    }
  } else
  {
    WORD    recv_len;
    BYTE    recv_data[14];
    BYTE    send_data[] = { 0x05, 0x00, 0x00, 0xCC, 0xCC };

    send_data[1] = afi;

    recv_len = sizeof(recv_data);
    rc = SPROX_API_CALL(B_Exchange) (SPROX_PARAM_P  send_data, sizeof(send_data), recv_data, &recv_len, 1, 4096);
    if (rc == MI_OK)
    {
      if (recv_len != 14)
      {
        rc = MI_BYTECOUNTERR;
      } else
      if (recv_data[0] != 0x50)
      {
        rc = MI_CODEERR;
      } else
      {
        /* OK ! */
        if (atq != NULL)
          memcpy(atq, &recv_data[1], 11);
      }
    }
  }
  
  SPROX_Trace(TRACE_DEBUG, "B_SelectIdle -> %d", rc);  
  return rc;
}

/**f* SpringProx.API/SPROX_B_AnticollAny
 *
 * NAME
 *   SPROX_B_AnticollAny
 *
 * DESCRIPTION
 *   Init the ISO 14443-B anticollision loop, looking for "any" card available.
 *   Subsequent slots must be addressed with SPROX_B_AnticollSlot
 *
 * INPUTS
 *   BYTE slots         : Number of anticollision slots.
 *                        Must be set to 16.
 *   BYTE afi           : Application Family Identifier. Set to 0x00 for all
 *                        families and all sub-families.
 *   BYTE atq[11]       : 11-byte buffer to receive card's Answer To Query,
 *                        if a card is active in slot 0
 *
 * RETURNS
 *   MI_OK              : success, card selected
 *   MI_NOTAGERR        : no card active in slot 0
 *   Other code if internal or communication error has occured. 
 *
 * SEE ALSO
 *   SPROX_B_SelectAny
 *   SPROX_B_AnticollIdle
 *
 **/
SPROX_API_FUNC(B_AnticollAny) (SPROX_PARAM  BYTE slots, BYTE afi, BYTE atq[11])
{
  SWORD rc;
  SPROX_PARAM_TO_CTX;  

  rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_14443_B);
  if (rc != MI_OK) return rc;

  if (slots != 16)
    return MI_UNKNOWN_FUNCTION;

  if (sprox_ctx->sprox_version >= 0x00014102)
  {
    /* Function is inside the reader */
    BYTE buffer[12]; // JDA : bugfix in 1.62.8
    WORD len = 12;    
    
    buffer[0] = afi;
    buffer[1] = 0;
    
    rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_SELECT_ANY, buffer, 2, buffer, &len);
    if (rc == MI_OK)
    {
      if (atq != NULL)
        memcpy(atq, &buffer[1], 11);
    }
  } else
  {
    rc = SPROX_API_CALL(B_SelectAny) (SPROX_PARAM_P  afi, atq);
  }
  
  SPROX_Trace(TRACE_DEBUG, "B_AnticollAny -> %d", rc);  
  return rc;
}

/**f* SpringProx.API/SPROX_B_AnticollIdle
 *
 * NAME
 *   SPROX_B_AnticollIdle
 *
 * DESCRIPTION
 *   Init the ISO 14443-B anticollision loop, looking for idle cards only.
 *   Subsequent slots must be addressed with SPROX_B_AnticollSlot
 *
 * INPUTS
 *   BYTE slots         : Number of anticollision slots.
 *                        Must be set to 16.
 *   BYTE afi           : Application Family Identifier. Set to 0x00 for all
 *                        families and all sub-families.
 *   BYTE atq[11]       : 11-byte buffer to receive card's Answer To Query,
 *                        if a card is active in slot 0
 *
 * RETURNS
 *   MI_OK              : success, card selected
 *   MI_NOTAGERR        : no card active in slot 0
 *   Other code if internal or communication error has occured. 
 *
 * SEE ALSO
 *   SPROX_B_SelectIdle
 *   SPROX_B_AnticollAny
 *
 **/
SPROX_API_FUNC(B_AnticollIdle) (SPROX_PARAM  BYTE slots, BYTE afi, BYTE atq[11])
{
  SWORD rc;
  SPROX_PARAM_TO_CTX;  

  rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_14443_B);
  if (rc != MI_OK) return rc;

  if (slots != 16)
    return MI_UNKNOWN_FUNCTION;

  if (sprox_ctx->sprox_version >= 0x00014102)
  {
    /* Function is inside the reader */
    BYTE buffer[12]; // JDA : bugfix in 1.62.8
    WORD len = 12;    
    
    buffer[0] = afi;
    buffer[1] = 0;
    
    rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_SELECT_IDLE, buffer, 2, buffer, &len);
    if (rc == MI_OK)
    {
      if (atq != NULL)
        memcpy(atq, &buffer[1], 11);
    }
  } else
  {
    return SPROX_API_CALL(B_SelectIdle) (SPROX_PARAM_P  afi, atq);
  }
  
  SPROX_Trace(TRACE_DEBUG, "B_AnticollIdle -> %d", rc);  
  return rc;
}

/**f* SpringProx.API/SPROX_B_AnticollSlot
 *
 * NAME
 *   SPROX_B_AnticollSlot
 *
 * DESCRIPTION
 *   Send the next 14443-B SLOT-MARKER to continue the anticollision loop
 *
 * INPUTS
 *   BYTE slot          : Index of anticollision slot, 1 <= slot <= 15
 *   BYTE atq[11]       : 11-byte buffer to receive card's Answer To Query,
 *                        if a card is active in the selected slot
 *
 * RETURNS
 *   MI_OK              : success, card selected
 *   MI_NOTAGERR        : no card active in selected slot
 *   Other code if internal or communication error has occured. 
 *
 * SEE ALSO
 *   SPROX_B_AnticollAny
 *   SPROX_B_AnticollIdle
 *
 **/
SPROX_API_FUNC(B_AnticollSlot) (SPROX_PARAM  BYTE slot, BYTE atq[11])
{
  SWORD rc;
  SPROX_PARAM_TO_CTX;  

  rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_14443_B);
  if (rc != MI_OK) return rc;

  if ((slot == 0) || (slot >= 16))
    return MI_LIB_CALL_ERROR;

  if (sprox_ctx->sprox_version >= 0x00014102)
  {
    /* Function is inside the reader */
    BYTE buffer[12]; // JDA : bugfix in 1.62.8
    WORD len = 12;    
    
    buffer[0] = 0; /* Don't care */
    buffer[1] = slot;
    
    /* No difference between SPROX_CSB_SELECT_ANY and SPROX_CSB_SELECT_IDLE here */
    rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_SELECT_ANY, buffer, 2, buffer, &len);
    if (rc == MI_OK)
    {
      if (atq != NULL)
        memcpy(atq, &buffer[1], 11);
    }
  } else
  {
    rc = MI_UNKNOWN_FUNCTION;
  }
  
  SPROX_Trace(TRACE_DEBUG, "B_AnticollSlot -> %d", rc);  
  return rc;
}

/**f* SpringProx.API/SPROX_B_Halt
 *
 * NAME
 *   SPROX_B_Halt
 *
 * DESCRIPTION
 *   Send the ISO 14443-B HALT command to a card.
 *
 * WARNING
 *   If card is already in T=CL mode, you must DESELECT the card instead of
 *   HALTing it.
 *
 * INPUTS
 *   const BYTE pupi[4] : 4-byte buffer of card's PUPI
 *                        (= 4 first bytes of card's ATQ)
 *
 * RETURNS
 *   MI_OK              : success, card halted
 *   Other code if internal or communication error has occured. 
 * 
 * SEE ALSO
 *   SPROX_TclB_Deselect
 *
 **/
SPROX_API_FUNC(B_Halt) (SPROX_PARAM  const BYTE pupi[4])
{
  SWORD rc;
  SPROX_PARAM_TO_CTX;
  
  rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_14443_B);
  if (rc != MI_OK) return rc;
   
  if (sprox_ctx->sprox_version >= 0x00013500)
  {
    /* Function is inside the reader */
    if (pupi == NULL)
      rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_HALT, NULL, 0, NULL, NULL);
    else
      rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_CSB_HALT, pupi, 4, NULL, NULL);

  } else
  {
    WORD    recv_len;
    BYTE    recv_data[64];
    BYTE    send_data[7];

    if (pupi == NULL)
      return MI_UNKNOWN_FUNCTION;

    send_data[0] = 0x50;
    memcpy(&send_data[1], pupi, 4);

    recv_len = sizeof(recv_data);
    rc = SPROX_API_CALL(B_Exchange) (SPROX_PARAM_P  send_data, sizeof(send_data), recv_data, &recv_len, 1, 4096);
    if (rc != MI_OK)
      return rc;

    if (recv_len != 1)
    {
      rc = MI_BYTECOUNTERR;
    } else
    if ((recv_data[0] != 0x00) && (recv_data[0] != 0x50))
    {
      rc = MI_CODEERR;
    }
  }
  return rc;
}

/**f* SpringProx.API/SPROX_B_Exchange
 *
 * NAME
 *   SPROX_B_Exchange
 *
 * DESCRIPTION
 *   Low-level frame exchange with currently activated ISO 14443-B card.
 *
 * WARNING
 *   Once card has entered T=CL mode, the high-level frame exchange must
 *   be performed through SPROX_Tcl_Exchange
 *
 * INPUTS
 *   const BYTE send_data[]  : command (buffer PCD->PICC)
 *   WORD       send_bytelen : length of send_data
 *   BYTE       recv_data[]  : answer (buffer PICC->PCD)
 *   WORD      *recv_bytelen : IN  : size of recv_data (max length)
 *                             OUT : actual length of recv_data
 *   BOOL       calc_crc     : if TRUE, the last 2 bytes of send_data
 *                             will be replaced by a valid 14443 CRC_B
 *                             if FALSE, the CRC must have been computed
 *                             by caller
 *   WORD       timeout      : timeout of the exchange, in ETU
 *                             (at 106kbps, 1 ETU = 9,44s)
 *
 * RETURNS
 *   MI_OK              : success, card answer to be found in recv_data
 *   Other code if any error has occured. 
 * 
 * NOTES
 *   Remember that send_bytelen (and recv_bytelen) are the actual length
 *   of the frame, INCLUDING THE CRC (on the 2-last bytes).
 *   Even if calc_crc is TRUE, the caller must provide room in send_data[]
 *   (and in recv_data[]) to store the CRC.
 *
 **/
SPROX_API_FUNC(B_Exchange) (SPROX_PARAM  const BYTE *send_data, WORD send_bytelen, BYTE *recv_data, WORD *recv_bytelen, BYTE append_crc, WORD timeout)
{
  BYTE    buffer[SPROX_FRAME_CONTENT_SIZE];
  WORD    rlen = sizeof(buffer);
  SWORD   rc;
  
  rc = SPROX_API_CALL(SetConfig) (SPROX_PARAM_P  CFG_MODE_ISO_14443_B);
  if (rc != MI_OK) return rc;

  if (send_data == NULL) return MI_LIB_CALL_ERROR;
  if ((send_bytelen + 7) >= SPROX_FRAME_CONTENT_SIZE) return MI_COMMAND_OVERFLOW;

  buffer[0] = (BYTE) (send_bytelen / 0x0100);
  buffer[1] = (BYTE) (send_bytelen % 0x0100);
  if (recv_bytelen != NULL)
  {
    buffer[2] = (BYTE) (*recv_bytelen / 0x0100);
    buffer[3] = (BYTE) (*recv_bytelen % 0x0100);
  } else
  {
    buffer[2] = (BYTE) (rlen / 0x0100);
    buffer[3] = (BYTE) (rlen % 0x0100);
  }
  buffer[4] = append_crc;
  buffer[5] = (BYTE) (timeout / 0x0100);
  buffer[6] = (BYTE) (timeout % 0x0100);

  memcpy(&buffer[7], send_data, send_bytelen);

  rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_PICCEXCHANGEBLOCK_B, buffer, (WORD) (send_bytelen + 7), buffer, &rlen);
  if (rc != MI_OK)
    return rc;

  if ((rlen <= 2) || ((rlen-2) != (buffer[0]*0x0100 + buffer[1])))
    return MI_RESPONSE_INVALID;
  
  rlen -= 2;
  if (recv_bytelen != NULL)
  {
    if ((*recv_bytelen != 0) && (*recv_bytelen < rlen))
      return MI_RESPONSE_OVERFLOW;
    *recv_bytelen = rlen;
  }

  if (recv_data != NULL)
  {            
    memcpy(recv_data, &buffer[2], rlen);
  }

  return rc;
}

