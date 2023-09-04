#include "sprox_api_i.h"

#ifndef SPROX_API_NO_TCL

SPROX_API_FUNC(NfcI_Exchange) (SPROX_PARAM  BYTE did, const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD *recv_len)
{
  BYTE    buffer[256+1];
  SWORD   rc;
  
  /* Check parameters */  
  if ((send_buffer == NULL) || (send_len > 256) || (recv_buffer == NULL) || (recv_len == NULL)) return MI_LIB_CALL_ERROR;

  /* Build send buffer */
  buffer[0] = did;
  memcpy(&buffer[1], send_buffer, send_len);
  send_len += 1;

  rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_NFC_EXCHANGE, buffer, send_len, recv_buffer, recv_len);
  if (rc == MI_OK)
    return MI_OK;

  /* Error in exchange, PICC has been deselected */
  return rc;
}

SPROX_API_FUNC(NfcI_Atr) (SPROX_PARAM  BYTE did, const BYTE gi[], WORD gi_len, BYTE nfcid3t[10], BYTE gt[], WORD *gt_len)
{
  BYTE    buffer[256+1];
  WORD    send_len = 0;
  WORD    recv_len = sizeof(buffer);
  SWORD   rc;
  
  /* Check parameters */  
  if (gi_len > 256) return MI_LIB_CALL_ERROR;

  /* Build send buffer */
  buffer[send_len++] = SPROX_NFC_FUNC_INITIATOR_ATR;
  buffer[send_len++] = did;
  if (gi != NULL)
  {
    memcpy(&buffer[send_len], gi, gi_len);
    send_len += gi_len;
  }

  rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_NFC_FUNC, buffer, send_len, buffer, &recv_len);
  if (rc == MI_OK)
  {
    if (recv_len < 10)
    {
      rc = MI_RESPONSE_INVALID;
    } else
    {
      if (nfcid3t != NULL)
        memcpy(nfcid3t, &buffer[0], 10);
        
      if (gt_len != NULL)
        *gt_len = recv_len - 10;
        
      if (gt != NULL)
        memcpy(gt, &buffer[10], recv_len - 10);
        
      return MI_OK;   
    }
  }

  /* Error in exchange, PICC has been deselected */
  return rc;
}

SPROX_API_FUNC(NfcI_Dsl) (SPROX_PARAM  BYTE did)
{
  BYTE    buffer[2];
  SWORD   rc;
  
  /* Build send buffer */
  buffer[0] = SPROX_NFC_FUNC_INITIATOR_DSL;
  buffer[1] = did;

  rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_NFC_FUNC, buffer, 2, NULL, NULL);
  if (rc == MI_OK)
    return MI_OK;

  /* Error in exchange, PICC has been deselected */
  return rc;
}

SPROX_API_FUNC(NfcI_Rls) (SPROX_PARAM  BYTE did)
{
  BYTE    buffer[2];
  SWORD   rc;
  
  /* Build send buffer */
  buffer[0] = SPROX_NFC_FUNC_INITIATOR_RLS;
  buffer[1] = did;

  rc = SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_NFC_FUNC, buffer, 2, NULL, NULL);
  if (rc == MI_OK)
    return MI_OK;

  /* Error in exchange, PICC has been deselected */
  return rc;
}

#endif
