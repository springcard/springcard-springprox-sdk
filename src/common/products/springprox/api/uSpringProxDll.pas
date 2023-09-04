(*

  SpringCard SpringProx.dll wrapper for Delphi
  --------------------------------------------
  (c) 2000-2008 SpringCard - www.springcard.com

  This is only a subset of all available springprox.dll functions.
  Please refer to the developer's guide and to the C "springprox.h" header file
  for more info.

  Note that the few function implemented below are available both on CSB-3 and on CSB-4 or
  SpringProx-CF reader.
  Other functions are not available for the CSB-3 family.

*)

unit uSpringProxDll;

interface

type
   // default return type : signed 16-bit word
   sword   = smallint;
   dword   = longword;
   // a Mifare key is 6-byte wide
   mif_key = array[0..5] of byte;

(*

  Unless explicitly specified, all SPROX functions returns a signed word (from -255 to 0).
  0 means success, other values mean error. Use the GetErrorMessage function for error explanation

*)

// Translate return code into string
function SPROX_GetErrorMessage(status : sword) : pchar; cdecl; external 'springprox.dll';
function SPROX_GetErrorMessageStr(status : sword) : string;

// Helpers functions for VB or Delphi users
function SPROX_ArrayToString(str : pchar; buffer : pointer; size : word) : sword; cdecl; external 'springprox.dll';
function SPROX_ArrayToStringStr(buffer : pointer; size : word) : string;
function SPROX_StringToArray(buffer : pointer; str : pchar; size : word) : sword; cdecl; external 'springprox.dll';
function SPROX_StringToArrayStr(buffer : pointer; str : string; size : word) : sword;

// Lookup and open the SpringProx or CSB reader
function SPROX_ReaderOpen(device : pchar) : sword; cdecl; external 'springprox.dll';
// Close the reader
function SPROX_ReaderClose : sword; cdecl; external 'springprox.dll';
// PocketPC only : put the reader in low-power (sleep) mode
function SPROX_ReaderDeactivate : sword; cdecl; external 'springprox.dll';
// PocketPC only : wakeup the reader
function SPROX_ReaderActivate : sword; cdecl; external 'springprox.dll';

function SPROX_ReaderReset : sword; cdecl; external 'springprox.dll';

// Retrieve the device the reader is connected to (mostly 'COMx'...)
function SPROX_ReaderGetDevice(device : pchar; len : word) : sword; cdecl; external 'springprox.dll';
function SPROX_ReaderGetDeviceSettings(var settings : longint) : sword; cdecl; external 'springprox.dll';

//Configure the device
function SPROX_ReaderSetDeviceSettings(settings : dword): sword; cdecl; external 'springprox.dll';

const
  SPROX_SETTINGS_PROTOCOL_MASK       = $00000003;
  SPROX_SETTINGS_PROTOCOL_OSI        = $00000000;
  SPROX_SETTINGS_PROTOCOL_ASCII      = $00000001;
  SPROX_SETTINGS_PROTOCOL_BIN        = $00000002;
  SPROX_SETTINGS_PROTOCOL_BUS        = $00000003;

  SPROX_SETTINGS_HARDWARE_CTRL       = $00000004;

  SPROX_SETTINGS_BAUDRATE_MASK       = $00000008;
  SPROX_SETTINGS_BAUDRATE_38400      = $00000000;
  SPROX_SETTINGS_BAUDRATE_115200     = $00000008;

  SPROX_SETTINGS_CHANNEL_MASK        = $00000060;
  SPROX_SETTINGS_CHANNEL_RS232       = $00000000;
  SPROX_SETTINGS_CHANNEL_RS485       = $00000020;
  SPROX_SETTINGS_CHANNEL_USB         = $00000040;
  SPROX_SETTINGS_CHANNEL_TCP         = $00000060;

  SPROX_SETTINGS_FORCE_CHANNEL_RS485 = $00000020;
  SPROX_SETTINGS_FORCE_BAUDRATE      = $00000010;
  SPROX_SETTINGS_FORCE_PROTOCOL      = $00000004;

function SPROX_ReaderAttachSerial(hComm : thandle) : sword; cdecl; external 'springprox.dll';

// Retrieve springprox.dll version
function SPROX_GetLibrary(librarynfo : pchar; len : word) : sword; cdecl; external 'springprox.dll';
function SPROX_GetLibraryStr : string;
// Retrieve SpringProx or CSB's firmware version. CSB-3 doesn't return its firmware version, so a fake one is provided
function SPROX_ReaderGetFirmware(firmware : pchar; len : word) : sword; cdecl; external 'springprox.dll';
function SPROX_ReaderGetFirmwareStr : string;
// Retrieve the Philips MfRc5xx chipset block ID (type and serial number). Not available with the CSB-3
function SPROX_ReaderGetRc500Id(rc500_type : pointer; rc500_snr : pointer) : sword; cdecl; external 'springprox.dll';
function SPROX_ReaderGetRc500SnrStr : string;

function SPROX_Find(want_protos : word; got_proto : pointer; uid : pointer; uidlen : pointer) : sword; cdecl; external 'springprox.dll';

// Load Mifare keys into the reader EEPROM or RAM. Capacities are :
// - 4 A keys in RAM
// - 4 B keys in RAM
// - 16 A keys in EEPROM
// - 16 B keys in EEPROM
// Use 'A' or 'B' as key type parameter
function SPROX_MifLoadKey(eeprom : boolean; key_type : char; key_num : byte; key_data : pointer) : sword; cdecl; external 'springprox.dll';

// Select any Mifare tag available in the reader's field
function SPROX_MifStSelectAny(snr : pointer; atq : pointer; sak : pointer) : sword; cdecl; external 'springprox.dll';
function SPROX_MifStSelectIdle(snr : pointer; atq : pointer; sak : pointer) : sword; cdecl; external 'springprox.dll';
// Reactivate the specified tag (if still available in the reader's field)
function SPROX_MifStSelectAgain(snr : pointer) : sword; cdecl; external 'springprox.dll';
// Deactivate the specified tag (if still available in the reader's field)
function SPROX_MifStHalt(snr : pointer) : sword; cdecl; external 'springprox.dll';

function SPROX_A_SelectAny(atq : pointer; snr : pointer; snrlen : pointer; sak : pointer) : sword; cdecl; external 'springprox.dll';
function SPROX_A_SelectIdle(atq : pointer; snr : pointer; snrlen : pointer; sak : pointer) : sword; cdecl; external 'springprox.dll';
function SPROX_A_SelectAgain(snr : pointer; snrlen : byte; sak : pointer) : sword; cdecl; external 'springprox.dll';
function SPROX_A_Halt : sword; cdecl; external 'springprox.dll';

function SPROX_B_SelectAny(afi : byte; atq : pointer) : sword; cdecl; external 'springprox.dll';
function SPROX_B_SelectIdle(afi : byte; atq : pointer) : sword; cdecl; external 'springprox.dll';
function SPROX_B_Halt(pupi : pointer) : sword; cdecl; external 'springprox.dll';

// Read one Mifare block (without authentication), or 4 Mifare UltraLight pages
function SPROX_MifRead(snr : pointer; addr : byte; buffer : pointer) : sword; cdecl; external 'springprox.dll';

// Read one Mifare block
// - if key is NULL, all known keys are tried (EEPROM + RAM), as key A, then as key B
// - if key is not NULL, it is tried first as key A, then as key B
// The buffer must be 16-byte wide
function SPROX_MifStReadBlock(snr : pointer; bloc : byte; buffer : pointer; key : pointer) : sword; cdecl; external 'springprox.dll';

// Read one Mifare sector
// - if key is NULL, all known keys are tried (EEPROM + RAM), as key A, then as key B
// - if key is not NULL, it is tried first as key A, then as key B
// The buffer must be 48-byte wide
function SPROX_MifStReadSector(snr : pointer; sect : byte; buffer : pointer; key : pointer) : sword; cdecl; external 'springprox.dll';

// Write one Mifare block
// - if key is NULL, all known keys are tried (EEPROM + RAM), as key B, then as key A
// - if key is not NULL, it is tried first as key B, then as key A
// The buffer must be 16-byte wide
//
// WARNING : do not try to write the block 3 of any sector without a perfect understanding
//           of the Mifare access control block. It is safer to use the dedicated function
//           SPROX_MifStUpdateAccessBlock with suggested parameters.
//
function SPROX_MifStWriteBlock(snr : pointer; bloc : byte; buffer : pointer; key : pointer) : sword; cdecl; external 'springprox.dll';

// Write one Mifare sector
// - if key is NULL, all known keys are tried (EEPROM + RAM), as key B, then as key A
// - if key is not NULL, it is tried first as key B, then as key A
// The buffer must be 48-byte wide, the block 3 (access control) CAN'T be written through
// this function.
function SPROX_MifStWriteSector(snr : pointer; sect : byte; buffer : pointer; key : pointer) : sword; cdecl; external 'springprox.dll';

function SPROX_MifStReadCounter(snr : pointer; bloc : byte; var counter : longint; key : pointer) : sword; cdecl; external 'springprox.dll';
function SPROX_MifStWriteCounter(snr : pointer; bloc : byte; counter : longint; key : pointer) : sword; cdecl; external 'springprox.dll';
function SPROX_MifStDecrementCounter(snr : pointer; bloc : byte; value : longint; key : pointer) : sword; cdecl; external 'springprox.dll';

// Format and write one Mifare access block (block 3 of the sector)
// - if old_key is NULL, all known keys are tried (EEPROM + RAM), as key A, then as key B
// - if old_key is not NULL, it is tried first as key A, then as key B
// - new_key_a and new_key_b must be 6-byte buffers
// - ac0, ac1 and ac2 are access conditions for data blocks, acc_normal and acc_counter are valid values
// - ac3 is the access conditions for the security bloc, acc_security is a valid value
//
// WARNING : setting a wrong value for ac3, or setting invalid A or B keys will lead to the sector
//           being permanently unavailable. Please refer to the Mifare tags documentation for more
//           info.
//
function SPROX_MifStUpdateAccessBlock(snr : pointer;
                                      sect : byte;
                                      old_key : pointer;
                                      new_key_a : pointer;
                                      new_key_b : pointer;
                                      ac0 : byte; ac1 : byte; ac2 : byte; ac3 : byte) : sword; cdecl; external 'springprox.dll';

// ISO 14443 configuration (1->A, 2->B)
function SPROX_SetConfig(cfg : byte) : sword; cdecl; external 'springprox.dll';

function SPROX_Debug_SetApiConfig(cfg : byte) : sword; cdecl; external 'springprox.dll';

// T=CL (A) API entries, added 25/10/2003
function SPROX_TclA_ActivateIdle(atq : pointer; snr : pointer; snrlen : pointer; sak : pointer) : sword; cdecl; external 'springprox.dll';
function SPROX_TclA_ActivateAny(atq : pointer; snr : pointer; snrlen : pointer; sak : pointer) : sword; cdecl; external 'springprox.dll';
function SPROX_TclA_Halt : sword; cdecl; external 'springprox.dll';
function SPROX_TclA_Deselect(cid : byte) : sword; cdecl; external 'springprox.dll';
function SPROX_TclA_GetAts(cid : byte; ats : pointer; atslen : pointer) : sword; cdecl; external 'springprox.dll';
function SPROX_TclA_Pps(cid : byte; dsi : byte; dri : byte) : sword; cdecl; external 'springprox.dll';
function SPROX_TclA_Exchange(fsd : byte; cid : byte; nad : byte; send_buffer : pointer; send_len : word; recv_buffer : pointer; recv_len : pointer) : sword; cdecl; external 'springprox.dll';

// T=CL (B) API entries, added 14/11/2005
function SPROX_TclB_ActivateIdle(afi : byte; atq : pointer) : sword; cdecl; external 'springprox.dll';
function SPROX_TclB_ActivateAny(afi : byte; atq : pointer) : sword; cdecl; external 'springprox.dll';
function SPROX_TclB_Halt(pupi : pointer) : sword; cdecl; external 'springprox.dll';
function SPROX_TclB_Deselect(cid : byte) : sword; cdecl; external 'springprox.dll';
function SPROX_TclB_Attrib(pupi : pointer; cid : byte) : sword; cdecl; external 'springprox.dll';
function SPROX_TclB_Exchange(fsd : byte; cid : byte; nad : byte; send_buffer : pointer; send_len : word; recv_buffer : pointer; recv_len : pointer) : sword; cdecl; external 'springprox.dll';

function SPROX_Tcl_Exchange(cid : byte; send_buffer : pointer; send_len : word; recv_buffer : pointer; recv_len : pointer) : sword; cdecl; external 'springprox.dll';

function SPROX_Bi_Attrib(uid : pointer) : sword; cdecl; external 'springprox.dll';
function SPROX_Bi_Exchange(send_buffer : pointer; send_len : word; recv_buffer : pointer; recv_len : pointer) : sword; cdecl; external 'springprox.dll';
function SPROX_Bi_Disc() : sword; cdecl; external 'springprox.dll';

// LED control utility, added 25/10/2003
function SPROX_ControlLed(led_red : byte; led_green : byte) : sword; cdecl; external 'springprox.dll';
function SPROX_ControlLedY(led_red : byte; led_green : byte; led_yellow : byte) : sword; cdecl; external 'springprox.dll';

// User IO, added 18/01/2011
function SPROX_ControlReadModeIO(var value : boolean) : sword; cdecl; external 'springprox.dll';

function SPROX_ControlEx(send_buffer : pointer; send_bytelen : word; recv_buffer : pointer; recv_bytelen : pointer) : sword; cdecl; external 'springprox.dll';
function SPROX_ControlBuzzer(duration_ms : word) : sword;

function SPROX_ControlRF(rf : byte) : sword; cdecl; external 'springprox.dll';
function SPROX_ReaderGetConsts(var ct : longword) : sword; cdecl; external 'springprox.dll';
function SPROX_ReaderSetConsts(ct : longword) : sword; cdecl; external 'springprox.dll';

function SPROX_ReaderGetConstsEx(ident : byte; value : pointer; length : pointer) : sword; cdecl; external 'springprox.dll';
function SPROX_ReaderSetConstsEx(ident : byte; value : pointer; length : word) : sword; cdecl; external 'springprox.dll';

function SPROX_ReaderGetFeatures(features : pointer) : sword; cdecl; external 'springprox.dll';

function SPROX_Echo(len : sword) : sword; cdecl; external 'springprox.dll';

function SPROX_A_Exchange(send : pointer; sendlen : word; recv : pointer; recvlen : pointer; append_crc : byte; timeout : word) : sword; cdecl; external 'springprox.dll';
function SPROX_B_Exchange(send : pointer; sendlen : word; recv : pointer; recvlen : pointer; append_crc : byte; timeout : word) : sword; cdecl; external 'springprox.dll';

// Low level API calls, available for demonstration purpose only
function Mf500PiccAuthRam(auth_mode : byte; key_sector : byte; block : byte) : sword; cdecl; external 'springprox.dll';
function Mf500PiccAuth(auth_mode : byte; key_sector : byte; block : byte) : sword; cdecl; external 'springprox.dll';

function WriteRIC(reg : byte; value : byte) : sword; cdecl; external 'springprox.dll';

function SPROX_Card_PowerUp_Auto(slot : byte; atr : pointer; atr_len : pointer) : sword; cdecl; external 'springprox.dll';
function SPROX_Card_PowerDown(slot : byte) : sword; cdecl; external 'springprox.dll';
function SPROX_Card_Exchange(slot : byte; send_buffer : pointer; send_len : word; recv_buffer : pointer; recv_len : pointer) : sword; cdecl; external 'springprox.dll';

const
   key_type_a : char = 'A';
   key_type_b : char = 'B';

   // Mifare sample access keys
   // -------------------------

   key_aaaaaa : mif_key = ($A0, $A1, $A2, $A3, $A4, $A5); // standardized 'base A' key
   key_bbbbbb : mif_key = ($B0, $B1, $B2, $B3, $B4, $B5); // standardized 'base B' key
   key_ffffff : mif_key = ($FF, $FF, $FF, $FF, $FF, $FF); // commonly used transport key
   key_000000 : mif_key = ($00, $00, $00, $00, $00, $00); // commonly used transport key

   key_demo_a : mif_key = ($44, $45, $4D, $4F, $5F, $41); // the word 'DEMO_A'
   key_demo_b : mif_key = ($44, $45, $4D, $4F, $5F, $42); // the word 'DEMO_B'

   key_demo_a_ : mif_key = ($41, $5F, $4F, $4D, $45, $44); // the word 'DEMO_B'
   key_demo_b_ : mif_key = ($42, $5F, $4F, $4D, $45, $44); // the word 'DEMO_B'

   key_test_a : mif_key = ($54, $45, $53, $54, $5F, $41); // the word 'TEST_A'
   key_test_b : mif_key = ($54, $45, $53, $54, $5F, $42); // the word 'TEST_B'

   // access condition for Mifare data blocks (blocks 0 to 2 of each sector)
   acc_normal   : byte  = $04; // use keyA for read, key B for read/write
   acc_counter  : byte  = $06; // use keyA for read/decrement, key B for read/write/decrement/increment

   // access condition for Mifare access control blocks (block 3 of each sector)
   acc_security : byte  = $03; // keyB is sector's master key

implementation

uses SysUtils;

function SPROX_GetErrorMessageStr(status : sword) : string;
begin
   result := StrPas(SPROX_GetErrorMessage(status));
end;

function SPROX_GetLibraryStr : string;
var
   sb : array[1..256] of char;
begin
   result := '';
   if SPROX_GetLibrary(pchar(@sb), sizeof(sb)) = 0 then result := StrPas(pchar(@sb));
end;

function SPROX_ReaderGetFirmwareStr : string;
var
   sb : array[1..256] of char;
begin
   result := '';
   if SPROX_ReaderGetFirmware(pchar(@sb), sizeof(sb)) = 0 then result := StrPas(pchar(@sb));
end;

function SPROX_ReaderGetRc500SnrStr : string;
var
   rc500_type : array[1..5] of byte;
   rc500_snr  : array[1..4] of byte;
   i : integer;
begin
   result := '';
   if SPROX_ReaderGetRc500Id(@rc500_type, @rc500_snr) = 0 then
      for i := 1 to 4 do result := result + IntToHex(rc500_snr[i], 2);
end;

function SPROX_ArrayToStringStr(buffer : pointer; size : word) : string;
var
   sb : array[1..1024] of char;
begin
   result := '';
   if SPROX_ArrayToString(pchar(@sb), buffer, size) = 0 then result := StrPas(pchar(@sb));
end;

function SPROX_StringToArrayStr(buffer : pointer; str : string; size : word) : sword;
begin
   result := SPROX_StringToArray(buffer, pchar(str), size);
end;

function SPROX_ControlBuzzer(duration_ms : word) : sword;
var
   buf : array[0..2] of byte;
begin
   buf[0] := $1C;
   buf[1] := duration_ms div 256;
   buf[2] := duration_ms mod 256;
   result := SPROX_ControlEx(@buf, 3, nil, nil);
end;

end.


