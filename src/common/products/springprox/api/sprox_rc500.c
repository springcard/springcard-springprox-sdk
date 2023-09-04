/**h* SpringProx.API/RC500
 *
 * NAME
 *   SpringProxAPI :: RC500
 *
 * DESCRIPTION
 *   MICORE I low level library (RC500/Pegoda basic function set)
 *
 * NOTES
 *   The function described here comes from Philips RC500 sample source code
 *   available at
 *   www.semiconductors.philips.com/files/markets/identification/download/MC081380.zip .
 *
 *   Please refer to Philips documentation for further info regarding this function.
 *
 * WARNING
 *   The RC500 functions are not available with the reentrant library.
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
  
  JDA 22/06/2006 : created with low level functions removed from sprox_fct.c

*/

#include "sprox_api_i.h"
#include "micore_picc.h"

extern BYTE sprox_ram_key_count;
extern BYTE sprox_e2_key_count;


SPROX_API_FUNC(WriteRCRegister) (SPROX_PARAM  BYTE reg, BYTE value)
{
  BYTE    buffer[2];
  buffer[0] = reg;
  buffer[1] = value;

  return SPROX_DLG_FUNC(SPROX_PARAM_P  SPROX_PCDWRITERCREG, buffer, sizeof(buffer), NULL, NULL);
}

#ifndef SPROX_API_REENTRANT

SWORD SaveASnr(SPROX_PARAM  const BYTE snr[], BYTE snrlen);


SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PcdConfig(void)
{
  return SPROX_Function(SPROX_PCDCONFIG, NULL, 0, NULL, NULL);
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PcdSetDefaultAttrib(void)
{
  return SPROX_Function(SPROX_PCDSETDEFAULTATTRIB, NULL, 0, NULL, NULL);
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PcdSetAttrib(BYTE DSI, BYTE DRI)
{
  BYTE    buffer[2];
  buffer[0] = DSI;
  buffer[1] = DRI;
  return SPROX_Function(SPROX_PCDSETATTRIB, buffer, sizeof(buffer), 0, NULL);
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PcdGetAttrib(BYTE * FSCImax, BYTE * FSDImax, BYTE * DSsupp, BYTE * DRsupp, BYTE * DREQDS)
{
  BYTE    buffer[5];
  WORD    len = sizeof(buffer);
  SWORD   res;

  res = SPROX_Function(SPROX_PCDGETATTRIB, NULL, 0, buffer, &len);

  if (res == MI_OK)
  {
    if (FSCImax != NULL)
      *FSCImax = buffer[0];
    if (FSDImax != NULL)
      *FSDImax = buffer[1];
    if (DSsupp != NULL)
      *DSsupp = buffer[2];
    if (DRsupp != NULL)
      *DRsupp = buffer[3];
    if (DREQDS != NULL)
      *DREQDS = buffer[4];
  }

  return res;
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccRequest(BYTE req_code, BYTE * atq)
{
  if (sprox_ctx_glob->sprox_version)
  {
    return Mf500PiccCommonRequest(req_code, atq);
  }

  {
    WORD    len = 2;
    BYTE    buffer[1];
    buffer[0] = req_code;
    return SPROX_Function(SPROX_CSB_REQUEST, buffer, sizeof(buffer), atq, &len);
  }
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccCommonRequest(BYTE req_code, BYTE * atq)
{
  WORD    len = 2;

  return SPROX_Function(SPROX_PICCCOMMONREQUEST, &req_code, 1, atq, &len);
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccAnticoll(BYTE bcnt, BYTE * snr)
{
  if (sprox_ctx_glob->sprox_version)
    return Mf500PiccCascAnticoll(0x93, bcnt, snr);

  {
    WORD    len = 4;
    BYTE    buffer[5];
    buffer[0] = bcnt;
    return SPROX_Function(SPROX_CSB_ANTICOLL, buffer, sizeof(buffer), snr, &len);
  }
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccCascAnticoll(BYTE select_code, BYTE bcnt, BYTE * snr)
{
  BYTE    buffer[2];
  WORD    len = 4;
  buffer[0] = select_code;
  buffer[1] = bcnt;
  return SPROX_Function(SPROX_PICCCASCANTICOLL, buffer, sizeof(buffer), snr, &len);
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccSelect(const BYTE * snr, BYTE * sak)
{
  SWORD   rc;

  if (sprox_ctx_glob->sprox_version)
    rc = Mf500PiccCascSelect(0x93, snr, sak);
  else
  {
    WORD    len = 1;
    rc = SPROX_Function(SPROX_CSB_SELECT, snr, 4, sak, &len);
  }
  if (rc == MI_OK)
    SaveASnr(snr, 4);
  return rc;
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccCascSelect(BYTE select_code, const BYTE * snr, BYTE * sak)
{
  SWORD   rc;
  BYTE    buffer[5];
  WORD    len = 1;
  buffer[0] = select_code;
  memcpy(&buffer[1], snr, 4);
  rc = SPROX_Function(SPROX_PICCCASCSELECT, buffer, sizeof(buffer), sak, &len);
  if (rc == MI_OK)
    SaveASnr(snr, 4);
  return rc;
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccAuthE2(BYTE auth_mode, BYTE * snr, BYTE key_sector, BYTE block)
{
  return SPROX_MifStAuthE2(auth_mode, snr, key_sector, block);
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccAuth(BYTE auth_mode, BYTE key_sector, BYTE block)
{
  return SPROX_MifStAuthE2(auth_mode, NULL, key_sector, block);
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500HostCodeKey(BYTE * uncoded, BYTE * coded)
{
  BYTE    cnt = 0;
  BYTE    ln = 0;
  BYTE    hn = 0;
  for (cnt = 0; cnt < 6; cnt++)
  {
    ln = uncoded[cnt] & 0x0F;
    hn = uncoded[cnt] >> 4;
    coded[cnt * 2 + 1] = (~ln << 4) | ln;
    coded[cnt * 2] = (~hn << 4) | hn;
  }
  return MI_OK;
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PcdLoadKeyE2(BYTE key_type, BYTE sector, BYTE * uncoded_keys)
{
  if (!sprox_ctx_glob->sprox_version)
    return MI_FUNCTION_NOT_AVAILABLE;

  {
    BYTE    buffer[8];
    buffer[0] = key_type;
    buffer[1] = sector;
    memcpy(&buffer[2], uncoded_keys, 6);
    return SPROX_Function(SPROX_PCDLOADKEYE2, buffer, sizeof(buffer), NULL, NULL);
  }
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PcdLoadKeyRam(BYTE key_type, BYTE sector, BYTE * uncoded_keys)
{
  BYTE    buffer[14];

  if ((key_type != PICC_AUTHENT1A) && (key_type != PICC_AUTHENT1B))
    return MI_LIB_CALL_ERROR;

  if (key_type == PICC_AUTHENT1B)
    sector += sprox_ram_key_count / 2;

  if (sector >= sprox_ram_key_count)
    return MI_LIB_CALL_ERROR;

  buffer[0] = sector;
  buffer[1] = 0x00;
  buffer[2] = 0xBD;
  buffer[3] = 0xDE;
  buffer[4] = 0x6F;
  buffer[5] = 0x37;
  buffer[6] = 0x83;
  buffer[7] = 0x83;
  memcpy(&buffer[8], uncoded_keys, 6);
  return SPROX_Function(SPROX_CSB_LOAD_KEY, buffer, sizeof(buffer), NULL, NULL);
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccAuthKey(BYTE auth_mode, const BYTE * snr, BYTE * keys, BYTE block)
{
  if (!sprox_ctx_glob->sprox_version)
    return MI_FUNCTION_NOT_AVAILABLE;

  {
    BYTE    buffer[18];
    buffer[0] = auth_mode;
    if (snr != NULL)
      memcpy(&buffer[1], snr, 4);
    else
      memcpy(&buffer[1], sprox_ctx_glob->mif_snr, 4);
    memcpy(&buffer[5], keys, 12);
    buffer[17] = block;
    return SPROX_Function(SPROX_PICCAUTHKEY, buffer, sizeof(buffer), NULL, NULL);
  }
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccAuthRam(BYTE auth_mode, BYTE key_sector, BYTE block)
{
  return SPROX_MifStAuthRam(auth_mode, key_sector, block);
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccRead(BYTE addr, BYTE * data)
{
  if (sprox_ctx_glob->sprox_version)
    return Mf500PiccCommonRead(PICC_READ16, addr, 16, data);

  {
    WORD    len = 16;
    return SPROX_Function(SPROX_CSB_READ, &addr, 1, data, &len);
  }
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccCommonRead(BYTE cmd, BYTE addr, BYTE datalen, BYTE * data)
{
  BYTE    buffer[3];
  SWORD   rc;
  WORD    len = datalen;
  buffer[0] = cmd;
  buffer[1] = addr;
  buffer[2] = datalen;
  rc = SPROX_Function(SPROX_PICCCOMMONREAD, buffer, sizeof(buffer), data, &len);
  if (rc != MI_OK)
    return rc;
  if (len != datalen)
    return MI_WRONG_LENGTH;
  return MI_OK;
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccWrite(BYTE addr, BYTE * data)
{
  if (sprox_ctx_glob->sprox_version)
    return Mf500PiccCommonWrite(PICC_WRITE16, addr, 16, data);

  {
    BYTE    buffer[17];
    buffer[0] = addr;
    memcpy(&buffer[1], data, 16);
    return SPROX_Function(SPROX_CSB_WRITE, buffer, sizeof(buffer), NULL, NULL);
  }
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccWrite4(BYTE addr, BYTE * data)
{
  return Mf500PiccCommonWrite(PICC_WRITE4, addr, 4, data);
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccCommonWrite(BYTE cmd, BYTE addr, BYTE datalen, BYTE * data)
{
  BYTE    buffer[256 + 3];
  buffer[0] = cmd;
  buffer[1] = addr;
  buffer[2] = datalen;
  memcpy(&buffer[3], data, datalen);
  return SPROX_Function(SPROX_PICCCOMMONWRITE, buffer, (WORD) (3 + datalen), NULL, NULL);
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccValue(BYTE dd_mode, BYTE addr, BYTE * value, BYTE trans_addr)
{
  BYTE    buffer[7];
  buffer[0] = dd_mode;
  buffer[1] = addr;
  memcpy(&buffer[2], value, 4);
  buffer[6] = trans_addr;
  return SPROX_Function(SPROX_PICCVALUE, buffer, sizeof(buffer), NULL, NULL);
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccValueDebit(BYTE dd_mode, BYTE addr, BYTE * value)
{
  BYTE    buffer[6];
  buffer[0] = dd_mode;
  buffer[1] = addr;
  memcpy(&buffer[2], value, 4);
  return SPROX_Function(SPROX_PICCVALUEDEBIT, buffer, sizeof(buffer), NULL, NULL);
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccExchangeBlock_A(BYTE * send_data, WORD send_bytelen, BYTE * rec_data, WORD * rec_bytelen, BYTE append_crc,
                                                             WORD timeout)
{
  return SPROX_A_Exchange(send_data, send_bytelen, rec_data, rec_bytelen, append_crc, timeout);
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccExchangeBlock_B(BYTE *send_data, WORD send_bytelen, BYTE *rec_data, WORD *rec_bytelen, BYTE append_crc, WORD timeout)
{
  return SPROX_B_Exchange(send_data, send_bytelen, rec_data, rec_bytelen, append_crc, timeout);
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccExchangeBlock(BYTE * send_data, WORD send_bytelen, BYTE * rec_data, WORD * rec_bytelen, BYTE append_crc,
                                                           DWORD timeout)
{
  WORD t;
  if (timeout > 0x0000FFFF) t = 0xFFFF; else t = (WORD) timeout;
  return SPROX_A_Exchange(send_data, send_bytelen, rec_data, rec_bytelen, append_crc, t);
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccHalt(void)
{
  return SPROX_A_Halt();
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccActivateIdle(BYTE br, BYTE * atq, BYTE * sak, BYTE * uid, BYTE * uid_len)
{
  SWORD   rc;
  BYTE    buffer[24];
  WORD    len = 24;
  BYTE    i;

  buffer[0] = br;
  rc = SPROX_Function(SPROX_PICCACTIVATEIDLE, buffer, 1, buffer, &len);
  if (rc == MI_OK)
  {
    if (atq != NULL)
    {
      atq[0] = buffer[0];
      atq[1] = buffer[1];
    }
    if (sak != NULL)
      sak[0] = buffer[2];
    if (uid_len != NULL)
      *uid_len = buffer[3];
    if (uid != NULL)
      for (i = 0; i < buffer[3]; i++)
        uid[i] = buffer[4 + i];

    SaveASnr(&buffer[4], buffer[3]);
  }
  return rc;
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500PiccActivateWakeup(BYTE br, BYTE * atq, BYTE * sak, const BYTE * uid, BYTE uid_len)
{
  SWORD   rc;
  BYTE    buffer[24];
  WORD    len = 24;
  BYTE    i;

  buffer[0] = br;
  buffer[1] = uid_len;
  for (i = 0; i < uid_len; i++)
    buffer[2 + i] = uid[i];

  rc = SPROX_Function(SPROX_PICCACTIVATEWAKEUP, buffer, (BYTE) (2 + uid_len), buffer, &len);
  if (rc == MI_OK)
  {
    if (atq != NULL)
    {
      atq[0] = buffer[0];
      atq[1] = buffer[1];
    }
    if (sak != NULL)
      sak[0] = buffer[2];

    SaveASnr(uid, uid_len);
  }
  return rc;
}

SPRINGPROX_LIB SWORD SPRINGPROX_API ReadRIC(BYTE reg, BYTE * value)
{
  BYTE    buffer[1];
  WORD    len = 1;
  buffer[0] = reg;
  return SPROX_Function(SPROX_PCDREADRCREG, buffer, sizeof(buffer), value, &len);
}

SPRINGPROX_LIB SWORD SPRINGPROX_API WriteRIC(BYTE reg, BYTE value)
{
  BYTE    buffer[2];
  buffer[0] = reg;
  buffer[1] = value;
  return SPROX_Function(SPROX_PCDWRITERCREG, buffer, sizeof(buffer), NULL, NULL);
}

SPRINGPROX_LIB SWORD SPRINGPROX_API SetRCBitMask(BYTE reg, BYTE mask)
{
  BYTE    buffer[2];
  buffer[0] = reg;
  buffer[1] = mask;
  return SPROX_Function(SPROX_PCDSETRCREG, buffer, sizeof(buffer), NULL, NULL);
}

SPRINGPROX_LIB SWORD SPRINGPROX_API ClearRCBitMask(BYTE reg, BYTE mask)
{
  BYTE    buffer[2];
  buffer[0] = reg;
  buffer[1] = mask;
  return SPROX_Function(SPROX_PCDCLRRCREG, buffer, sizeof(buffer), NULL, NULL);
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500InterfaceOpen(DWORD mode, DWORD options)
{
  SWORD   rc;
#ifdef WIN32
  if (mode == USB)
  {
    rc = SPROX_ReaderOpen(_T("USB"));
    if (rc != MI_OK)
      return rc;
    SPROX_ControlRF(FALSE);
    return rc;
  }
  if (mode == RS232)
  {
    rc = SPROX_ReaderOpen(NULL);
    if (rc != MI_OK)
      return rc;
    SPROX_ControlRF(FALSE);
    return rc;
  } else if ((mode > RS232) && (mode <= RS232 + 9))
  {
    TCHAR   buffer[16];
#ifdef UNDER_CE
    _stprintf(buffer, _T("COM%d:"), mode - RS232);
#endif
    _stprintf(buffer, _T("COM%d"), mode - RS232);
    rc = SPROX_ReaderOpen(buffer);
    if (rc != MI_OK)
      return rc;
#ifndef UNDER_CE
    SPROX_ControlRF(FALSE);
#endif
    return rc;
  }
#endif
#ifdef LINUX
  if ((mode > RS232) && (mode <= RS232 + 9))
  {
    TCHAR   buffer[16];
    _stprintf(buffer, "/dev/ttyS%d", (int) (mode - RS232 - 1));
    rc = SPROX_ReaderOpen(buffer);
    if (rc != MI_OK)
      return rc;
    SPROX_ControlRF(FALSE);
    return rc;
  }
#endif
  return MI_FUNCTION_NOT_AVAILABLE;
}

SPRINGPROX_LIB SWORD SPRINGPROX_API Mf500InterfaceClose(void)
{
  SPROX_ControlRF(FALSE);
  return SPROX_ReaderClose();
}

SPRINGPROX_LIB SWORD SPRINGPROX_API PcdReset(void)
{
  return SPROX_Function(SPROX_CSB_RESET, NULL, 0, NULL, NULL);
}

SPRINGPROX_LIB SWORD SPRINGPROX_API ExchangeByteStream(BYTE cmd, BYTE * send_data, WORD send_bytelen, BYTE * rec_data, WORD * rec_bytelen)
{
  BYTE    send_buffer[SPROX_FRAME_CONTENT_SIZE];
  BYTE    recv_buffer[SPROX_FRAME_CONTENT_SIZE];
  WORD    len;
  SWORD   res;

  send_buffer[0] = cmd;
  send_buffer[1] = (BYTE) (send_bytelen / 0x0100);
  send_buffer[2] = (BYTE) (send_bytelen % 0x0100);
  if (rec_bytelen != NULL)
  {
    len = *rec_bytelen + 2;
    send_buffer[3] = (BYTE) (*rec_bytelen / 0x0100);
    send_buffer[4] = (BYTE) (*rec_bytelen % 0x0100);
  } else
  {
    len = 2;
    send_buffer[3] = 0;
    send_buffer[4] = 0;
  }
  memcpy(&send_buffer[5], send_data, send_bytelen);

  res = SPROX_Function(SPROX_EXCHANGEBYTESTREAM, send_buffer, (WORD) (send_bytelen + 5), recv_buffer, &len);

  if (res == MI_OK)
  {
    len = recv_buffer[0] * 0x0100 + recv_buffer[1];

    if (rec_bytelen != NULL)
    {
      if (*rec_bytelen < len)
        len = *rec_bytelen;
      *rec_bytelen = len;
    }
    if (rec_data != NULL)
      memcpy(rec_data, &recv_buffer[2], len);
  }

  return res;
}

SPRINGPROX_LIB SWORD SPRINGPROX_API PcdSetTmo(DWORD tmoLength)
{
  BYTE    buffer[4];
  buffer[0] = (BYTE) (tmoLength / 0x01000000);
  buffer[1] = (BYTE) (tmoLength / 0x00010000);
  buffer[2] = (BYTE) (tmoLength / 0x00000100);
  buffer[3] = (BYTE) (tmoLength % 0x00000100);
  return SPROX_Function(SPROX_PCDSETTMO, buffer, sizeof(buffer), NULL, NULL);
}

#endif
