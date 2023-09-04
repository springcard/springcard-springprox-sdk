(*

  SpringCard SpringProx+Mifare helpers for Delphi
  -----------------------------------------------
  (c) 2000-2008 SpringCard - www.springcard.com

  This file defines various class and records to ease the Mifare tags handling process

*)

unit uSpringProxMif;

interface

uses Sysutils, uSpringProxDll;

type
   TMifStBlock = array[0..15] of byte;
   TMifStSector = array[0..2] of TMifStBlock;
   TMifStMadSector = packed record
      snr  : array[0..3] of byte;  //         4
      manu : array[0..11] of byte; //        12
      crc  : array[0..0] of byte;  //         1
      info : array[0..0] of byte;  //         1
      aids : array[1..15] of word; // 15*2 = 30
   end;
   TMifStTagInfo = packed record
      snr    : array[0..3] of byte;
      atq    : array[0..1] of byte;
      ats    : array[0..0] of byte;
   end;

function MifStGetSectorAid(mad : TMifStMadSector; sector : byte; var aid : word) : boolean;
function MifStSetSectorAid(var mad : TMifStMadSector; sector : byte; aid : word) : boolean;
function MifStGetFreeSector(mad : TMifStMadSector; var sector : byte) : boolean;
function MifStGetFreeSectorS(mad : TMifStMadSector; seccount : byte; var sector : byte) : boolean;
function MifStGetAidSector(mad : TMifStMadSector; aid : word; var sector : byte) : boolean;

implementation

function CalcMifCrc(buffer : PByteArray; len : byte) : byte;
var
   crc : byte;
   i,j : byte;
begin
	crc := $E3;
	for j := 0 to len do
   begin
	  crc := crc xor buffer[j];
	  for i := 0 to 8 do
     begin
		 if (crc and $80) > 0 then crc := (crc shl 1) xor $1D
                            else crc := (crc shl 1);
     end;
   end;
   result := crc;
end;

(* Retrieve sector application *)
function MifStGetSectorAid(mad : TMifStMadSector; sector : byte; var aid : word) : boolean;
begin
   result := false;
   if (sector >= 1) and (sector <= 15) then
   begin
      aid := mad.aids[sector];
      result := true;
   end;
end;

(* Set sector application *)
function MifStSetSectorAid(var mad : TMifStMadSector; sector : byte; aid : word) : boolean;
var
   p : PByteArray;
begin
   result := false;
   if (sector >= 1) and (sector <= 15) then
   begin
      // set the new aid
      mad.aids[sector] := aid;
      // update the crc
      p := @(mad.info[0]);
      mad.crc[0] := CalcMifCrc(p, 31);
      // ok
      result := true;
   end;
end;

(* Find a specific application *)
function MifStGetAidSector(mad : TMifStMadSector; aid : word; var sector : byte) : boolean;
var
   a : word;
   s : byte;
begin
   result := false;
   for s := 1 to 15 do
   begin
      if MifStGetSectorAid(mad, s, a) then
      begin
         if a = aid then
         begin
            sector := s;
            result := true;
            exit;
         end;
      end;
   end;
end;

(* Find a free sector *)
function MifStGetFreeSector(mad : TMifStMadSector; var sector : byte) : boolean;
begin
   result := MifStGetAidSector(mad, $0000, sector);
end;

(* Find many free contiguous sectors *)
function MifStGetFreeSectorS(mad : TMifStMadSector; seccount : byte; var sector : byte) : boolean;
var
   a : word;
   s_b, s_e : byte;
begin
   result := false;
   for s_b := 1 to 15 do
   begin
      for s_e := s_b to s_b + seccount do
      begin
         if s_e > 15 then exit;
         if MifStGetSectorAid(mad, s_e, a) then
         begin
            if a <> $0000 then break;
         end;
         if s_e - s_b = seccount then
         begin
            sector := s_b;
            result := true;
            exit;
         end;
      end;
   end;
end;

end.
