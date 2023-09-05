/**h* SpringProx/SpringProx.API.Ex
 *
 * NAME
 *   SpringProx.API.Ex -- SpringCard unified API for SpringProx, CSB and Kxxx contactless readers
 *
 * DESCRIPTION
 *   springprox_ex.h provides prototypes for multi-activation functions
 *
 * COPYRIGHT
 *   Copyright (c) 2000-2008 SpringCard SAS, FRANCE - www.springcard.com
 *
 * NOTES
 *   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
 *   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
 *   TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 *   PARTICULAR PURPOSE.
 *
 * AUTHOR
 *   Johann.D / SpringCard
 *
 * REMARKS
 *   see springprox.h (SpringProx/SpringProx) for history and details
 *
 **/
#ifndef SPRINGPROX_EX_H
#define SPRINGPROX_EX_H

#include "products/springprox/api/springprox.h"

 /* The SpringProx instance abstract data type */
typedef struct _SPROX_CTX_ST* SPROX_INSTANCE;

#ifdef __cplusplus
/* SpringProx API is pure C */
extern  "C"
{
#endif

	/* Library identification */
	/* ---------------------- */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_GetLibrary(TCHAR library[], WORD len);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_GetLibraryW(wchar_t library[], WORD len);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_GetLibraryA(char library[], WORD len);

	/* Return code to error message translation */
	/* ---------------------------------------- */
	SPRINGPROX_LIB const TCHAR* SPRINGPROX_API SPROXx_GetErrorMessage(SWORD status);
	SPRINGPROX_LIB const wchar_t* SPRINGPROX_API SPROXx_GetErrorMessageW(SWORD status);
	SPRINGPROX_LIB const char* SPRINGPROX_API SPROXx_GetErrorMessageA(SWORD status);

	/* Instance manipulation stuff */
	/* --------------------------- */
	SPRINGPROX_LIB SPROX_INSTANCE SPRINGPROX_API SPROXx_CreateInstance(void);
	SPRINGPROX_LIB void SPRINGPROX_API SPROXx_DestroyInstance(SPROX_INSTANCE rInst);

	/* How to mix SpringProx API with SpringProx EX... */
	/* ----------------------------------------------- */
	SPRINGPROX_LIB void SPRINGPROX_API SPROX_SelectInstance(SPROX_INSTANCE rInst);

	/* Reader access functions */
	/* ----------------------- */

	/* Open the reader (this library is not reentrant, we work with one reader at a time   */
	/* Set device to the communication port name ("COM1", "USB", "/dev/ttyS2" ...) or NULL */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ReaderOpen(SPROX_INSTANCE rInst, const TCHAR device[]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ReaderOpenW(SPROX_INSTANCE rInst, const wchar_t device[]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ReaderOpenA(SPROX_INSTANCE rInst, const char device[]);

	/* Close the reader */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ReaderClose(SPROX_INSTANCE rInst);

	/* Suspend the reader, without destroying the handle to access it */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ReaderDeactivate(SPROX_INSTANCE rInst);

	/* Resume the reader */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ReaderActivate(SPROX_INSTANCE rInst);

	/* Discover the reader on a previously opened communication port */
#ifdef WIN32
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ReaderAttachSerial(SPROX_INSTANCE rInst, HANDLE hComm);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ReaderAttachUSB(SPROX_INSTANCE rInst, HANDLE hComm);
#endif

	/* Enumerate the compliant devices found on USB */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_EnumUSBDevices(DWORD idx, TCHAR device[64], TCHAR description[64]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_EnumUSBDevicesW(DWORD idx, wchar_t device[64], wchar_t description[64]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_EnumUSBDevicesA(DWORD idx, char device[64], char description[64]);

	/* Select the address (RS-485 bus mode only) */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ReaderSelectAddress(SPROX_INSTANCE rInst, BYTE address);

	/* Reader information functions */
	/* ---------------------------- */

	/* Retrieve name of the communication port */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ReaderGetDevice(SPROX_INSTANCE rInst, TCHAR device[], WORD len);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ReaderGetDeviceW(SPROX_INSTANCE rInst, wchar_t device[], WORD len);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ReaderGetDeviceA(SPROX_INSTANCE rInst, char device[], WORD len);

	/* Retrieve reader's firmware (type - version) */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ReaderGetFirmware(SPROX_INSTANCE rInst, TCHAR firmware[], WORD len);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ReaderGetFirmwareW(SPROX_INSTANCE rInst, wchar_t firmware[], WORD len);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ReaderGetFirmwareA(SPROX_INSTANCE rInst, char firmware[], WORD len);

	/* Retrieve actual features of reader's firmware */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ReaderGetFeatures(SPROX_INSTANCE rInst, DWORD* features);

	/* Retrieve communication settings between host and reader */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ReaderGetDeviceSettings(SPROX_INSTANCE rInst, DWORD* settings);
	/* Select communication settings between host and reader */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ReaderSetDeviceSettings(SPROX_INSTANCE rInst, DWORD settings);

	/* Read configuration constants of the reader */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ReaderGetConsts(SPROX_INSTANCE rInst, DWORD* consts);
	/* Write configuration constants of the reader */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ReaderSetConsts(SPROX_INSTANCE rInst, DWORD consts);

	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ReaderGetConstsEx(SPROX_INSTANCE rInst, BYTE ident, BYTE consts[], WORD* length);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ReaderSetConstsEx(SPROX_INSTANCE rInst, BYTE ident, const BYTE consts[], WORD length);

	/* Miscellaneous reader functions */
	/* ------------------------------ */

	/* Reset the reader */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ReaderReset(SPROX_INSTANCE rInst);

	/* Drive the LEDs */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ControlLed(SPROX_INSTANCE rInst, BYTE led_r, BYTE led_g);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ControlLedY(SPROX_INSTANCE rInst, BYTE led_r, BYTE led_g, BYTE led_y);

	/* Drive the buzzer */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ControlBuzzer(SPROX_INSTANCE rInst, WORD time_ms);

	/* Drive or read the USER pin */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ControlUserIO(SPROX_INSTANCE rInst, BOOL is_output, BOOL out_value, BOOL* in_value);

	/* Read the MODE pin */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ControlReadModeIO(SPROX_INSTANCE rInst, BOOL* in_value);

	/* Drive the RF field */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ControlRF(SPROX_INSTANCE rInst, BYTE param);

	/* Configure the reader for ISO/IEC 14443-A or 14443-B or 15693 or ICODE1 operation */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_SetConfig(SPROX_INSTANCE rInst, BYTE mode);

	/* Retrieve RF chipset's information (NXP RC type and serial number) */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ReaderGetRc500Id(SPROX_INSTANCE rInst, BYTE micore_type[5], BYTE micore_snr[4]);

	/* Send a raw control command to the reader */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ControlEx(SPROX_INSTANCE rInst, const BYTE send_buffer[], WORD send_bytelen, BYTE recv_buffer[], WORD* recv_bytelen);

	/* Send a raw command to the reader */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Function(SPROX_INSTANCE rInst, BYTE cmd, const BYTE send_buffer[], WORD send_bytelen, BYTE recv_buffer[], WORD* recv_bytelen);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_FunctionWaitResp(SPROX_INSTANCE rInst, BYTE recv_buffer[], WORD* recv_bytelen, WORD timeout_s);

	/* Test communication with the reader */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Echo(SPROX_INSTANCE rInst, WORD len);

	/* Access to reader's non-volatile memory */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ReadStorage(SPROX_INSTANCE rInst, DWORD address, BYTE buffer[], WORD length);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_WriteStorage(SPROX_INSTANCE rInst, DWORD address, const BYTE buffer[], WORD length);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_StorageSize(SPROX_INSTANCE rInst, DWORD* size);

	/* Low level registry access */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_WriteRCRegister(SPROX_INSTANCE rInst, BYTE reg, BYTE value);

	/* ISO/IEC 14443-A functions */
	/* ------------------------- */

	/* Layer 3 */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_A_Request(SPROX_INSTANCE rInst, BYTE req_code, BYTE atq[2]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_A_RequestAny(SPROX_INSTANCE rInst, BYTE atq[2]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_A_RequestIdle(SPROX_INSTANCE rInst, BYTE atq[2]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_A_SelectAny(SPROX_INSTANCE rInst, BYTE atq[2], BYTE snr[], BYTE* snrlen, BYTE sak[1]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_A_SelectIdle(SPROX_INSTANCE rInst, BYTE atq[2], BYTE snr[], BYTE* snrlen, BYTE sak[1]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_A_SelectAgain(SPROX_INSTANCE rInst, const BYTE snr[], BYTE snrlen);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_A_Halt(SPROX_INSTANCE rInst);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_A_Exchange(SPROX_INSTANCE rInst, const BYTE send_buffer[], WORD send_bytelen, BYTE recv_buffer[], WORD* recv_bytelen, BYTE append_crc, WORD timeout);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_A_ExchangeRaw(SPROX_INSTANCE rInst, const BYTE send_buffer[], WORD send_bitslen, BYTE recv_buffer[], WORD* recv_bitslen, WORD timeout);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_A_ExchangeRawEx(SPROX_INSTANCE rInst, BYTE param1, BYTE param2, const BYTE send_data[], WORD send_bitslen, BYTE recv_data[], WORD* recv_bitslen, WORD timeout);


	/* Layer 4 (T=CL) */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_TclA_ActivateAny(SPROX_INSTANCE rInst, BYTE atq[2], BYTE snr[], BYTE* snrlen, BYTE sak[1]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_TclA_ActivateIdle(SPROX_INSTANCE rInst, BYTE atq[2], BYTE snr[], BYTE* snrlen, BYTE sak[1]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_TclA_ActivateAgain(SPROX_INSTANCE rInst, const BYTE snr[], BYTE snrlen);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_TclA_Halt(SPROX_INSTANCE rInst);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_TclA_Deselect(SPROX_INSTANCE rInst, BYTE cid);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_TclA_GetAts(SPROX_INSTANCE rInst, BYTE cid, BYTE ats[], BYTE* atslen);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_TclA_Pps(SPROX_INSTANCE rInst, BYTE cid, BYTE dsi, BYTE dri);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_TclA_Exchange(SPROX_INSTANCE rInst, BYTE fsci, BYTE cid, BYTE nad, const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD* recv_len);

	/* ISO/IEC 14443-B functions */
	/* ------------------------- */

	/* Layer 3 */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_B_SelectAny(SPROX_INSTANCE rInst, BYTE afi, BYTE atq[11]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_B_SelectIdle(SPROX_INSTANCE rInst, BYTE afi, BYTE atq[11]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_B_AnticollAny(SPROX_INSTANCE rInst, BYTE slots, BYTE afi, BYTE atq[11]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_B_AnticollIdle(SPROX_INSTANCE rInst, BYTE slots, BYTE afi, BYTE atq[11]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_B_AnticollSlot(SPROX_INSTANCE rInst, BYTE slot, BYTE atq[11]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_B_Exchange(SPROX_INSTANCE rInst, const BYTE send_buffer[], WORD send_bytelen, BYTE recv_buffer[], WORD* recv_bytelen, BYTE append_crc, WORD timeout);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_B_Halt(SPROX_INSTANCE rInst, const BYTE pupi[4]);

	/* Layer 4 (T=CL) */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_TclB_ActivateAny(SPROX_INSTANCE rInst, BYTE afi, BYTE atq[11]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_TclB_ActivateIdle(SPROX_INSTANCE rInst, BYTE afi, BYTE atq[11]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_TclB_Attrib(SPROX_INSTANCE rInst, const BYTE pupi[4], BYTE cid);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_TclB_AttribEx(SPROX_INSTANCE rInst, const BYTE pupi[4], BYTE cid, BYTE dsi, BYTE dri);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_TclB_Deselect(SPROX_INSTANCE rInst, BYTE cid);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_TclB_Halt(SPROX_INSTANCE rInst, const BYTE pupi[4]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_TclB_Exchange(SPROX_INSTANCE rInst, BYTE fsci, BYTE cid, BYTE nad, const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD* recv_len);

	/* ISO/IEC 14443 type independant functions */
	/* ------------------------------------------ */

	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Tcl_Exchange(SPROX_INSTANCE rInst, BYTE cid, const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD* recv_len);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Tcl_Deselect(SPROX_INSTANCE rInst, BYTE cid);

	/* B' (Innovatron) */
	/* --------------- */

	/* Those function are only avalaible for Calypso-enabled readers */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Bi_Apgen(SPROX_INSTANCE rInst, BYTE uid[4], BYTE atr[], BYTE* atrlen);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Bi_Attrib(SPROX_INSTANCE rInst, BYTE uid[4]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Bi_Exchange(SPROX_INSTANCE rInst, const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD* recv_len);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Bi_Disc(SPROX_INSTANCE rInst);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Bi_SamFastSpeed(SPROX_INSTANCE rInst, BYTE card_slot);

	/* ISO/IEC 15693 functions */
	/* ----------------------- */

	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Iso15693_SelectAny(SPROX_INSTANCE rInst, BYTE afi, BYTE snr[8]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Iso15693_SelectAgain(SPROX_INSTANCE rInst, BYTE snr[8]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Iso15693_Halt(SPROX_INSTANCE rInst);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Iso15693_ReadSingleBlock(SPROX_INSTANCE rInst, BYTE snr[8], BYTE addr, BYTE data[], WORD* datalen);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Iso15693_WriteSingleBlock(SPROX_INSTANCE rInst, BYTE snr[8], BYTE addr, const BYTE data[], WORD datalen);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Iso15693_LockBlock(SPROX_INSTANCE rInst, BYTE snr[8], BYTE addr);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Iso15693_ReadMultipleBlocks(SPROX_INSTANCE rInst, BYTE snr[8], BYTE addr, BYTE count, BYTE data[], WORD* datalen);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Iso15693_WriteMultipleBlocks(SPROX_INSTANCE rInst, BYTE snr[8], BYTE addr, BYTE count, const BYTE data[], WORD datalen);
	// SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Iso15693_WriteAFI(SPROX_INSTANCE rInst, BYTE snr[8], BYTE afi);
	// SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Iso15693_LockAFI(SPROX_INSTANCE rInst, BYTE snr[8]);
	// SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Iso15693_WriteDSFID(SPROX_INSTANCE rInst, BYTE snr[8], BYTE afi);
	// SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Iso15693_LockDSFID(SPROX_INSTANCE rInst, BYTE snr[8]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Iso15693_GetSystemInformation(SPROX_INSTANCE rInst, BYTE snr[8], BYTE data[], WORD* datalen);
	// SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Iso15693_GetMultipleBlockSecurityStatus(SPROX_INSTANCE rInst, BYTE snr[8], BYTE addr, BYTE count, BYTE data[], WORD *datalen);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Iso15693_ExtendedReadSingleBlock(SPROX_INSTANCE rInst, BYTE snr[8], WORD addr, BYTE data[], WORD* datalen);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Iso15693_ExtendedWriteSingleBlock(SPROX_INSTANCE rInst, BYTE snr[8], WORD addr, const BYTE data[], WORD datalen);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Iso15693_ExtendedLockBlock(SPROX_INSTANCE rInst, BYTE snr[8], WORD addr);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Iso15693_ExtendedReadMultipleBlocks(SPROX_INSTANCE rInst, BYTE snr[8], WORD addr, WORD count, BYTE data[], WORD* datalen);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Iso15693_ExtendedWriteMultipleBlocks(SPROX_INSTANCE rInst, BYTE snr[8], WORD addr, WORD count, const BYTE data[], WORD datalen);
	// SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Iso15693_ExtendedGetMultipleBlockSecurityStatus(SPROX_INSTANCE rInst, BYTE snr[8], WORD addr, WORD count, BYTE data[], WORD *datalen);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Iso15693_Exchange(SPROX_INSTANCE rInst, const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD* recv_len, WORD timeout);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Iso15693_ExchangeStdCommand(SPROX_INSTANCE rInst, BOOL opt_flag, BYTE snr[8], BYTE cmd_opcode, const BYTE cmd_params[], WORD cmd_params_len, BYTE recv_buffer[], WORD* recv_len, WORD timeout);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Iso15693_ExchangeCustomCommand(SPROX_INSTANCE rInst, BOOL opt_flag, BYTE mfg_id, BYTE snr[8], BYTE cmd_opcode, const BYTE cmd_params[], WORD cmd_params_len, BYTE recv_buffer[], WORD* recv_len, WORD timeout);

	/* ICODE1 functions */
	/* ---------------- */

	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ICode1_SelectAny(SPROX_INSTANCE rInst, BYTE afi, BYTE snr[8]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ICode1_UnselectedRead(SPROX_INSTANCE rInst, BYTE bloc, BYTE nb_bloc, BYTE data[], WORD* datalen);

	/* Inside Pico family/HID iClass */
	/* ----------------------------- */

	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Pico_Exchange14443(SPROX_INSTANCE rInst, const BYTE* send_data, WORD send_bytelen, BYTE* recv_data, WORD* recv_bytelen, BYTE append_crc, WORD timeout);
#define SPROXx_Pico_Exchange SPROXx_Pico_Exchange14443
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Pico_Exchange15693(SPROX_INSTANCE rInst, const BYTE* send_data, WORD send_bytelen, BYTE* recv_data, WORD* recv_bytelen, BYTE append_crc, WORD timeout);

	/* Find functions */
	/* -------------- */

	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Find(SPROX_INSTANCE rInst, WORD want_protos, WORD* got_proto, BYTE uid[10], BYTE* uidlen);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_FindEx(SPROX_INSTANCE rInst, WORD want_protos, WORD* got_proto, BYTE uid[10], BYTE* uidlen, BYTE info[32], BYTE* infolen);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_FindWait(SPROX_INSTANCE rInst, WORD want_protos, WORD* got_proto, BYTE uid[10], BYTE* uidlen, WORD timeout_s, WORD interval_ms);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_FindWaitEx(SPROX_INSTANCE rInst, WORD want_protos, WORD* got_proto, BYTE uid[10], BYTE* uidlen, BYTE info[32], BYTE* infolen, WORD timeout_s, WORD interval_ms);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_FindWaitCancel(SPROX_INSTANCE rInst);


	/* Legacy Mifare functions */
	/* ----------------------- */

	/* Mifare is a registered trademark of Philips.                            */
	/* Please refer to Philips documentation for any explanation on this part. */

	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifStSelectAny(SPROX_INSTANCE rInst, BYTE snr[4], BYTE atq[2], BYTE sak[1]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifStSelectIdle(SPROX_INSTANCE rInst, BYTE snr[4], BYTE atq[2], BYTE sak[1]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifStSelectAgain(SPROX_INSTANCE rInst, const BYTE snr[4]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifStHalt(SPROX_INSTANCE rInst);

	/* Mifare read, without authentication (Mifare UltraLight cards) */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifRead(SPROX_INSTANCE rInst, const BYTE snr[4], BYTE addr, BYTE data[16]);

	/* Mifare write, without authentication (Mifare UltraLight cards) */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifWrite(SPROX_INSTANCE rInst, const BYTE snr[4], BYTE addr, const BYTE data[16]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifWrite4(SPROX_INSTANCE rInst, const BYTE snr[4], BYTE addr, const BYTE data[4]);

	/* Mifare standard authenticate and read */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifStReadBlock(SPROX_INSTANCE rInst, const BYTE snr[4], BYTE bloc, BYTE data[16], const BYTE key_val[6]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifStReadBlock2(SPROX_INSTANCE rInst, const BYTE snr[4], BYTE bloc, BYTE data[16], BYTE key_idx);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifStReadSector(SPROX_INSTANCE rInst, const BYTE snr[4], BYTE sect, BYTE data[], const BYTE key_val[6]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifStReadSector2(SPROX_INSTANCE rInst, const BYTE snr[4], BYTE sect, BYTE data[], BYTE key_idx);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifStReadTag768(SPROX_INSTANCE rInst, const BYTE snr[4], WORD* sectors, BYTE data[]);

	/* Mifare standard authenticate and write */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifStWriteBlock(SPROX_INSTANCE rInst, const BYTE snr[4], BYTE bloc, const BYTE data[16], const BYTE key_val[6]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifStWriteBlock2(SPROX_INSTANCE rInst, const BYTE snr[4], BYTE bloc, const BYTE data[16], BYTE key_idx);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifStWriteSector(SPROX_INSTANCE rInst, const BYTE snr[4], BYTE sect, const BYTE data[], const BYTE key[6]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifStWriteSector2(SPROX_INSTANCE rInst, const BYTE snr[4], BYTE sect, const BYTE data[], BYTE key_idx);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifStWriteTag768(SPROX_INSTANCE rInst, const BYTE snr[4], WORD* sectors, const BYTE data[]);

	/* Mifare standard counter manipulation */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifStReadCounter(SPROX_INSTANCE rInst, const BYTE snr[4], BYTE bloc, SDWORD* value, const BYTE key_val[6]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifStReadCounter2(SPROX_INSTANCE rInst, const BYTE snr[4], BYTE bloc, SDWORD* value, BYTE key_idx);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifStWriteCounter(SPROX_INSTANCE rInst, const BYTE snr[4], BYTE bloc, SDWORD value, const BYTE key_val[6]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifStWriteCounter2(SPROX_INSTANCE rInst, const BYTE snr[4], BYTE bloc, SDWORD value, BYTE key_idx);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifStDecrementCounter(SPROX_INSTANCE rInst, const BYTE snr[4], BYTE bloc, DWORD value, const BYTE key_val[6]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifStDecrementCounter2(SPROX_INSTANCE rInst, const BYTE snr[4], BYTE bloc, DWORD value, BYTE key_idx);

	/* Mifare standard sector trailers formatting */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifStUpdateAccessBlock(SPROX_INSTANCE rInst, const BYTE snr[4], BYTE sect, const BYTE old_key[6], const BYTE new_key_A[6], const BYTE new_key_B[6], BYTE ac0, BYTE ac1, BYTE ac2, BYTE ac3);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifStUpdateAccessBlock2(SPROX_INSTANCE rInst, const BYTE snr[4], BYTE sect, BYTE old_key_idx, const BYTE new_key_A[6], const BYTE new_key_B[6], BYTE ac0, BYTE ac1, BYTE ac2, BYTE ac3);

	/* Perform Mifare standard authentication */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifStAuthKey(SPROX_INSTANCE rInst, BYTE auth_mode, const BYTE snr[4], const BYTE key_val[6], BYTE block);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifStAuthKey2(SPROX_INSTANCE rInst, const BYTE snr[4], BYTE key_idx, BYTE block);

	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifStAuthE2(SPROX_INSTANCE rInst, BYTE auth_mode, const BYTE snr[4], BYTE key_sector, BYTE block);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifStAuthRam(SPROX_INSTANCE rInst, BYTE auth_mode, BYTE key_sector, BYTE block);

	/* Load a Mifare standard key into reader's memory (RAM or EEPROM) */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifLoadKey(SPROX_INSTANCE rInst, BOOL eeprom, BYTE key_type, BYTE key_offset, const BYTE key_val[6]);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifLoadKeyEx(SPROX_INSTANCE rInst, BYTE key_idx, const BYTE key_val[6]);

	/* Who is the last authentication key successfully used ? */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MifLastAuthKey(SPROX_INSTANCE rInst, BYTE* info);

	/* Smartcard related functions (readers with embedded GemPlus GemCore smartcard reader) */
	/* ------------------------------------------------------------------------------------ */

	/* Power up the card in a slot */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Card_PowerUp(SPROX_INSTANCE rInst, BYTE slot, BYTE config, BYTE atr[], WORD* atr_len);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Card_PowerUp_Auto(SPROX_INSTANCE rInst, BYTE slot, BYTE atr[], WORD* atr_len);

	/* Power down the card in a slot */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Card_PowerDown(SPROX_INSTANCE rInst, BYTE slot);

	/* Exchange with the card in a slot */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Card_Exchange(SPROX_INSTANCE rInst, BYTE slot, const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD* recv_len);

	/* Get status of a slot */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Card_Status(SPROX_INSTANCE rInst, BYTE slot, BYTE* stat, BYTE* type, BYTE config[4]);

	/* Configure / retrieve actual configuration of a slot */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Card_SetConfig(SPROX_INSTANCE rInst, BYTE slot, BYTE mode, BYTE type);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Card_GetConfig(SPROX_INSTANCE rInst, BYTE slot, BYTE* mode, BYTE* type);

	/* Retrieve smartcard coupler's firmware (type - version) */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Card_GetFirmware(SPROX_INSTANCE rInst, TCHAR firmware[], WORD len);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Card_GetFirmwareW(SPROX_INSTANCE rInst, wchar_t firmware[], WORD len);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Card_GetFirmwareA(SPROX_INSTANCE rInst, char firmware[], WORD len);

	/* Transparent exchange between host and smartcard coupler */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Card_Control(SPROX_INSTANCE rInst, const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD* recv_len);

	/* Fingerprint related functions (reader with embedded Sagem MorphoSmart CBM module) */
	/* --------------------------------------------------------------------------------- */

	/* Power up the MorphoSmart */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MSO_Open(SPROX_INSTANCE rInst, TCHAR* mso_product, TCHAR* mso_firmware, TCHAR* mso_sensor);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MSO_OpenW(SPROX_INSTANCE rInst, wchar_t* mso_product, wchar_t* mso_firmware, wchar_t* mso_sensor);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MSO_OpenA(SPROX_INSTANCE rInst, char* mso_product, char* mso_firmware, char* mso_sensor);

	/* Power down the MorphoSmart */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MSO_Close(SPROX_INSTANCE rInst);

	/* Transparent exchange between host and MorphoSmart */
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_MSO_Exchange(SPROX_INSTANCE rInst, const BYTE send_buffer[], WORD send_len, BYTE recv_buffer[], WORD* recv_len, DWORD timeout, BYTE* async);


	/* Miscelleanous utilities, helpers for VB or Delphi users */
	/* ------------------------------------------------------- */

	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Malloc(BYTE** buffer, WORD size);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_Free(BYTE* buffer);
	SPRINGPROX_LIB WORD  SPRINGPROX_API SPROXx_StrLen(const TCHAR* buffer);
	SPRINGPROX_LIB WORD  SPRINGPROX_API SPROXx_StrLenW(const wchar_t* buffer);
	SPRINGPROX_LIB WORD  SPRINGPROX_API SPROXx_StrLenA(const char* buffer);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ArrayToString(TCHAR* string, const BYTE* buffer, WORD size);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ArrayToStringW(wchar_t* string, const BYTE* buffer, WORD size);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ArrayToStringA(char* string, const BYTE* buffer, WORD size);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_StringToArray(BYTE* buffer, const TCHAR* string, WORD size);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_StringToArrayW(BYTE* buffer, const wchar_t* string, WORD size);
	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_StringToArrayA(BYTE* buffer, const char* string, WORD size);


	SPRINGPROX_LIB SWORD SPRINGPROX_API SPROXx_ReaderFdt(SPROX_INSTANCE rInst, DWORD* Fdt);

#ifdef __cplusplus
}
#endif

#endif
