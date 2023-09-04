#include "sprox_api_i.h"

#ifdef SPROX_API_WITH_BRCD

SWORD Brcd_SendFrame(SPROX_CTX_ST *sprox_ctx, BYTE command, const BYTE payload[], WORD length, BOOL direct)
{
  BYTE buffer[256+3+3];
  BYTE offset = 0;  
  
  if (payload == NULL) length = 0;
      
  if (direct)
  {
  	SPROX_Trace(TRACE_DLG_HI, "Brcd:SendFrame Direct");
  } else
  {
  	SPROX_Trace(TRACE_DLG_HI, "Brcd:SendFrame Encapsulated");
  	
	  buffer[0] = 0xDE;
	  buffer[1] = (BYTE) ((3 + length) / 0x0100);
	  buffer[2] = (BYTE) ((3 + length) % 0x0100);
	  offset = 3;
  }
  
  buffer[offset+0] = command;
  buffer[offset+1] = (BYTE) (length / 0x0100);
  buffer[offset+2] = (BYTE) (length % 0x0100);
  offset += 3;
  
  memcpy(&buffer[offset], payload, length);

  if (!SendBurst(sprox_ctx, buffer, (WORD) (offset+length)))
  {
  	SPROX_Trace(TRACE_DLG_HI, "Brcd:Failed to send command");
    return MI_SER_ACCESS_ERR;
  }

  return MI_OK;
}

SWORD Brcd_RecvFrame(SPROX_CTX_ST *sprox_ctx, BYTE *status, BYTE payload[], WORD max_length, WORD *got_length, BOOL direct)
{
  BYTE buffer[256];
  BYTE header[3];
  WORD length;

again:
  
  SerialSetTimeouts(sprox_ctx, 2000, 150);

  if (direct)
  {
  	SPROX_Trace(TRACE_DLG_HI, "Brcd:RecvFrame Direct");
  } else
  {
  	SPROX_Trace(TRACE_DLG_HI, "Brcd:RecvFrame Encapsulated");
  	
	  if (!RecvBurst(sprox_ctx, header, 3))
	  {
	    /* No answer */
	    SPROX_Trace(TRACE_DLG_HI, "Brcd:Failed to recv encapsulation header");
	    return MI_SER_NORESP_ERR;
	  }

	  if (header[0] != 0xDE)
	  {
	  	SPROX_Trace(TRACE_DLG_HI, "Brcd:Invalid encapsulation header");
      RecvFlush(sprox_ctx);
	    return MI_SER_PROTO_ERR; 	
	  }
	  
    SerialSetTimeouts(sprox_ctx, 150, 100);
  }
  
  /* Recuperation du debut de la reponse (attente autorisee jusqu'a 1s) */
  if (!RecvBurst(sprox_ctx, header, 3))
  {
    /* No answer */
	  SPROX_Trace(TRACE_DLG_HI, "Brcd:Failed to recv command header");
    return MI_SER_NORESP_ERR;
  }

  if (direct)
  {
    SerialSetTimeouts(sprox_ctx, 150, 100);
  }
  
  length = header[1]; length *= 0x0100; length += header[2];
  
  if (length > SPROX_FRAME_CONTENT_SIZE) // JDA a faire corriger par Francois
  {
    /* Fatal overflow ! */
    SPROX_Trace(TRACE_DLG_HI, "Brcd:Returned payload is too long");
    RecvFlush(sprox_ctx); 	
    return MI_SER_PROTO_ERR;
  }

  /* Recuperation de la suite (sans attente) */
  if (!RecvBurst(sprox_ctx, buffer, length))
  {
    /* Timeout */
    SPROX_Trace(TRACE_DLG_HI, "Brcd:Failed to recv the payload");
    RecvFlush(sprox_ctx);
    return MI_SER_TIMEOUT_ERR;
  }

  if (header[0] == (0 - MI_TIME_EXTENSION)) // JDA a faire corriger par Francois
  {
  	SPROX_Trace(TRACE_DLG_HI, "Brcd:Time extension");
    goto again; /* Le lecteur demande plus de temps */
  }

  if (status != NULL)
    *status = header[0];  
  if (got_length != NULL)
    *got_length = length;

  if ((max_length != 0) && (max_length < length))
  {
    /* Le buffer de retour est trop petit */
    SPROX_Trace(TRACE_DLG_HI, "Brcd:Overflow");
    RecvFlush(sprox_ctx);
    return MI_SER_LENGTH_ERR;
  }

  if (payload != NULL)
    memcpy(payload, buffer, length);
    
  return MI_OK;  
}


SWORD SPROX_Brcd_Function(SPROX_PARAM  BYTE command, const BYTE *send_data, WORD send_len, BYTE *recv_data, WORD *recv_len)
{
  BYTE status;
  WORD l;
  SWORD rc;
  
  SPROX_PARAM_TO_CTX;
  
  rc = Brcd_SendFrame(sprox_ctx, command, send_data, send_len, sprox_ctx->settings.bin);
  if (rc != MI_OK) return rc;
  
  if ((recv_data != NULL) && (recv_len != NULL))
  {  
    l = *recv_len; *recv_len = 0;
    rc = Brcd_RecvFrame(sprox_ctx, &status, recv_data, l, recv_len, sprox_ctx->settings.bin);
  } else
  {
  	rc = Brcd_RecvFrame(sprox_ctx, &status, NULL, 0, NULL, sprox_ctx->settings.bin);
  }
  if (rc != MI_OK) return rc;
    
  return (signed char) status;
}


SWORD SPROX_Brcd_ReaderOpen(SPROX_PARAM  const TCHAR device[], BOOL direct)
{
	SWORD rc;
  SPROX_PARAM_TO_CTX;

  if (device != NULL)
  	SPROX_Trace(TRACE_ACCESS, "Brcd:ReaderOpen(%s)", device);
  else
    SPROX_Trace(TRACE_ACCESS, "Brcd:ReaderOpen(NULL)");

  if (!SerialOpen(sprox_ctx, device))
  {
    SPROX_Trace(TRACE_ACCESS, "Brcd:ReaderOpen(%s) ERROR", device);
    return MI_SER_ACCESS_ERR;
  }
  
  /* Try to connect to the specified device */
  if (!SerialSetBaudrate(sprox_ctx, 38400))
  {
  	SerialClose(sprox_ctx);
    SPROX_Trace(TRACE_ACCESS, "Brcd:Set baudrate(%s) ERROR", device);  	
    return MI_SER_ACCESS_ERR;
  }
  
  sprox_ctx->settings.brcd = TRUE;   
  sprox_ctx->settings.bin  = direct;

  rc = SPROX_ReaderGetFirmware(NULL, 0);  
  if (rc != MI_OK)
  {
  	SerialClose(sprox_ctx);
    SPROX_Trace(TRACE_DLG_HI, "Brcd:GetFirmware failed");
    return rc;
  }

  rc = SPROX_ReaderGetFeatures(NULL); 
  if (rc != MI_OK)
  {
  	SerialClose(sprox_ctx);
    SPROX_Trace(TRACE_DLG_HI, "Brcd:GetFeatures failed");
  }

  SPROX_Trace(TRACE_ACCESS, "Brcd:ReaderOpen OK");
  return MI_OK;
}
#endif
