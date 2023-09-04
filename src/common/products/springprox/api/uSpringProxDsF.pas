unit uSpringProxDsF;

interface

uses uSpringProxDll;

function DesFireCard_Authenticate(key_id : byte; access_key : pointer) : sword; cdecl; external 'sprox_desfire.dll';
function DesFireCard_ChangeKey(key_id : byte; new_key : pointer; old_key : pointer) : sword; cdecl; external 'sprox_desfire.dll';
function DesFireCard_FormatPICC : sword; cdecl; external 'sprox_desfire.dll';
function DesFireCard_GetApplicationIDs(aid_max_count : byte; aid_list : pointer; aid_count : pointer) : sword; cdecl; external 'sprox_desfire.dll';
function DesFireCard_SelectApplication(aid : dword) : sword; cdecl; external 'sprox_desfire.dll';
function DesFireCard_CreateApplication(aid : dword; key_settings : byte; keys_count : byte) : sword; cdecl; external 'sprox_desfire.dll';
function DesFireCard_DeleteFile(file_id : byte) : sword; cdecl; external 'sprox_desfire.dll';
function DesFireCard_CreateStdDataFile(file_id : byte; comm_mode : byte; access_rights : word; file_size : dword) : sword; cdecl; external 'sprox_desfire.dll';
function DesFireCard_CreateCyclicRecordFile(file_id : byte; comm_mode : byte; access_rights : word; item_size : dword; item_count : dword) : sword; cdecl; external 'sprox_desfire.dll';
function DesFireCard_CreateValueFile(file_id : byte; comm_mode : byte; access_rights : word; lower_limit, upper_limit, initial_value : longword; limited_credit_enabled : boolean) : sword; cdecl; external 'sprox_desfire.dll';
function DesFireCard_ReadData(file_id : byte; comm_mode : byte; start : dword; count : dword; buffer : pointer; done : pointer) : sword; cdecl; external 'sprox_desfire.dll';
function DesFireCard_ReadData2(file_id : byte; start : dword; count : dword; buffer : pointer; done : pointer) : sword; cdecl; external 'sprox_desfire.dll';
function DesFireCard_WriteData(file_id : byte; comm_mode : byte; start : dword; count : dword; buffer : pointer) : sword; cdecl; external 'sprox_desfire.dll';
function DesFireCard_WriteData2(file_id : byte; start : dword; count : dword; buffer : pointer) : sword; cdecl; external 'sprox_desfire.dll';

function DesFire_GetErrorMessage(status : sword) : pchar; cdecl; external 'sprox_desfire.dll';

type
   desfire_key = array[0..15] of byte;

const
   DF_COMM_MODE_PLAIN		= $00;
   DF_COMM_MODE_MACED		= $01;
   DF_COMM_MODE_PLAIN2		= $02;
   DF_COMM_MODE_ENCIPHERED	= $03;
   desfire_transport_key : desfire_key = ($00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00);


implementation

end.
