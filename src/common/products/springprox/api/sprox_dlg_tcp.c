/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  SpringProx API
  --------------

  Copyright (c) 2000-2008 SpringCard SAS, FRANCE - www.springcard.com

  sprox_dlg_tcp.c
  ---------------
  Implementation of the dialog with the reader, using the TCP C/S protocol.

  revision :
  ----------

  JDA 16/07/2014 : created

*/

#include "sprox_api_i.h"

#ifdef SPROX_API_WITH_TCP

#ifdef WIN32
#pragma comment(lib, "WS2_32")
#pragma warning( disable : 4996 )
#endif

#define RDR_TO_PC_LEGACY 0x28
#define PC_TO_RDR_LEGACY 0x29

#ifdef WIN32
static BOOL WinsockStarted = FALSE;
static BOOL WinsockStartup(void)
{
	int err;
	WORD wVersionRequested;
	WSADATA wsaData;

	if (WinsockStarted)
		return TRUE;

	wVersionRequested = MAKEWORD(2, 0);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
		return FALSE;

	if (wsaData.wVersion < wVersionRequested)
	{
		WSACleanup();
		return FALSE;
	}

	WinsockStarted = TRUE;
	return TRUE;
}

static void WinsockCleanup(void)
{
	WinsockStarted = FALSE;
	WSACleanup();
}
#endif

static SWORD TCP_Wait(SOCKET sk, DWORD wait_sec)
{
	fd_set rdfdset;
	int selret;
	struct timeval timeout;

	timeout.tv_sec = wait_sec;
	timeout.tv_usec = 0;

	FD_ZERO(&rdfdset);
	FD_SET(sk, &rdfdset);

	selret = select(sk + 1, &rdfdset, 0, 0, &timeout);
	if (selret < 0)
	{
		/* Disconnected? */
		SPROX_Trace(TRACE_DLG_HI, "TCP:wait error");
		return MI_SER_ACCESS_ERR;
	}

	if (selret == 0)
	{
		/* Timeout */
		SPROX_Trace(TRACE_DLG_HI, "TCP:no response");
		return MI_SER_NORESP_ERR;
	}

	return MI_OK;
}

static SWORD TCP_Send(SOCKET sk, BYTE buffer[], WORD length)
{
	int done;

	done = send(sk, buffer, length, 0);

	if (done < 0)
	{
		SPROX_Trace(TRACE_DLG_HI, "TCP:send error");
		return MI_SER_ACCESS_ERR;
	}
	if (done < length)
	{
		SPROX_Trace(TRACE_DLG_HI, "TCP:send too short");
		return MI_SER_ACCESS_ERR;
	}

	return MI_OK;
}

static SWORD TCP_Recv(SOCKET sk, BYTE buffer[], WORD max_len, WORD* got_len)
{
	int done;

	done = recv(sk, buffer, max_len, 0);
	if (done < 0)
	{
		SPROX_Trace(TRACE_DLG_HI, "TCP:recv error");
		return MI_SER_ACCESS_ERR;
	}

	*got_len = done;
	return MI_OK;
}

SWORD SPROX_TCP_Function(SPROX_CTX_ST* sprox_ctx, BYTE cmd, const BYTE send_buffer[], WORD send_bytelen, BYTE recv_buffer[], WORD* recv_bytelen)
{
	BYTE buffer[512];
	WORD length, t;
	SWORD rc;

	length = 0;
	buffer[length++] = PC_TO_RDR_LEGACY;
	buffer[length++] = sprox_ctx->com_sequence;
	buffer[length++] = cmd;
	buffer[length++] = (BYTE)(send_bytelen / 0x0100);
	buffer[length++] = (BYTE)(send_bytelen % 0x0100);
	if (send_buffer != NULL)
		memcpy(&buffer[length], send_buffer, send_bytelen);
	length += send_bytelen;

	rc = TCP_Send(sprox_ctx->com_socket, buffer, length);
	if (rc != MI_OK)
		goto failed;

	rc = TCP_Wait(sprox_ctx->com_socket, 30000);
	if (rc != MI_OK)
		goto failed;

	length = 0;
	rc = TCP_Recv(sprox_ctx->com_socket, buffer, sizeof(buffer), &length);
	if (rc != MI_OK)
		goto failed;

	if (buffer[0] != RDR_TO_PC_LEGACY)
	{
		rc = MI_SER_PROTO_ERR;
		goto failed;
	}
	if (buffer[1] != sprox_ctx->com_sequence)
	{
		rc = MI_SER_PROTO_ERR;
		goto failed;
	}
	sprox_ctx->com_sequence++;

	rc = (0 - buffer[2]);

	t = buffer[3];
	t *= 0x0100;
	t += buffer[4];

	if (recv_bytelen != NULL)
	{
		if ((*recv_bytelen != 0) && (*recv_bytelen < t))
		{
			rc = MI_RESPONSE_OVERFLOW;
			goto failed;
		}
		*recv_bytelen = t;
	}

	if (recv_buffer != NULL)
		memcpy(recv_buffer, &buffer[5], t);

	return rc;

failed:
	SPROX_TCP_ReaderClose(sprox_ctx);
	return rc;
}

static void TCP_Close(SOCKET sk)
{
#ifdef WIN32
	closesocket(sk);
#else
	close(sk);
#endif  
}

static SWORD TCP_Open(struct sockaddr_in* sa, SOCKET* sk)
{
	SOCKET new_sk;

	new_sk = socket(AF_INET, SOCK_STREAM, 0);
	if (new_sk < 0)
	{
		SPROX_Trace(TRACE_DLG_HI, "TCP:socket error");
		return MI_READER_CONNECT_FAILED;
	}

	if (connect(new_sk, (struct sockaddr*)sa, sizeof(struct sockaddr_in)) < 0)
	{
		SPROX_Trace(TRACE_DLG_HI, "TCP:connect error");
		TCP_Close(new_sk); /* Oups ca manquait ! */
		return MI_READER_CONNECT_FAILED;
	}

	*sk = new_sk;

	return MI_OK;
}

SWORD SPROX_TCP_ReaderOpen(SPROX_CTX_ST* sprox_ctx, const TCHAR conn_string[])
{
	struct sockaddr_in sa;
	struct hostent* hp;
	SWORD rc;

#ifdef WIN32
	if (!WinsockStartup())
	{
		SPROX_Trace(TRACE_DLG_HI, "TCP:Winsock error");
		return MI_LIB_INTERNAL_ERROR;
	}
#endif

	hp = gethostbyname(conn_string);
	if (hp == NULL)
	{
		SPROX_Trace(TRACE_DLG_HI, "TCP:host not found or invalid");
		return MI_READER_NAME_INVALID;
	}

	memset(&sa, 0, sizeof(struct sockaddr_in));

	if (hp->h_addrtype == AF_INET)
	{
		sa.sin_family = AF_INET;
		memcpy(&sa.sin_addr, hp->h_addr_list[0], 4);
		sa.sin_port = htons(3999);
	}
	else
	{
		SPROX_Trace(TRACE_DLG_HI, "TCP:invalid address type");
		return MI_READER_NAME_INVALID;
	}

	rc = TCP_Open(&sa, &sprox_ctx->com_socket);
	if (rc != MI_OK)
		return rc;

	sprox_ctx->com_settings &= ~COM_INTERFACE_MASK;
	sprox_ctx->com_settings |= COM_INTERFACE_TCP;

#ifdef SPROX_API_REENTRANT
	rc = SPROXx_ReaderGetFirmware(sprox_ctx, NULL, 0);
#else
	rc = SPROX_ReaderGetFirmware(NULL, 0);
#endif
	if (rc != MI_OK)
	{
		SPROX_Trace(TRACE_DLG_HI, "TCP:GetFirmware failed");
		goto failed;
	}

#ifdef SPROX_API_REENTRANT
	rc = SPROXx_ReaderGetFeatures(sprox_ctx, NULL);
#else
	rc = SPROX_ReaderGetFeatures(NULL);
#endif
	if (rc != MI_OK)
	{
		SPROX_Trace(TRACE_DLG_HI, "TCP:GetFeatures failed");
		goto failed;
	}

	SPROX_Trace(TRACE_ACCESS, "TCP:ReaderOpen OK");
	return MI_OK;

failed:
	SPROX_TCP_ReaderClose(sprox_ctx);
	return rc;
}

SWORD SPROX_TCP_ReaderClose(SPROX_CTX_ST* sprox_ctx)
{
	if ((sprox_ctx->com_settings & COM_INTERFACE_MASK) == COM_INTERFACE_TCP)
	{
		SPROX_Trace(TRACE_ACCESS, "TCP:ReaderClose");
		TCP_Close(sprox_ctx->com_socket);
#ifdef WIN32
		WinsockCleanup();
#endif
	}

	return MI_OK;
}

#endif
