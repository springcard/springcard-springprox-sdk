unit uSpringProxAid;

interface

type
   // Types used in our sample applications
   TInfo1 = packed record                    // The card holder structure, part 1 (one Mifare sector)
      lname : array[1..24] of char;          // Lastname
      fname : array[1..24] of char;          // Firstname
   end;
   TInfo2 = packed record                    // The card holder structure, part 2 (one Mifare sector)
      sex   : char;                          // Sex ('m' or 'f')
      bdate : array[1..3] of byte;           // date of birth (yy/mm/dd)
      other : array[1..44] of char;          // free string buffer
   end;
   TPurse = packed record                    // The structure to fill up one Mifare sector
      value       : longint;                 // current value
      value_padd  : array[1..3] of longint;  // Mifare representation of counter
      value_init  : longint;                 // initial value
      date_init   : array[1..3] of byte;     // date of purse initialization (yy/mm/dd)
      date_expire : array[1..3] of byte;     // purse is only valid untill... (yy/mm/dd)
      reserved    : array[1..16] of byte;    // not used
   end;
   TVaccine = packed record
      name  : array[1..3] of char;
      edate : byte;
   end;
   TVaccineChart = packed record
      vaccines : array[1..12] of TVaccine;
   end;

   TInfoPicture = packed record
      data : array[1..11,1..48] of byte;
   end;

const
  // AIDs used in our sample applications
  aid_info_1   : word = $F007;              // Our card holder structure, part 1
  aid_info_2   : word = $F107;              // Our card holder structure, part 2
  aid_vaccines : word = $F207;              // Out vaccination chart
  aid_purse    : word = $F307;              // Our electronic purse
  aid_picture  : word = $F407;              // Our card holder picture

  picture_size : byte = 11;                 // 10 sectors for one picture !


implementation

end.
