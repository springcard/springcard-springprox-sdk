SPRINGCARD SDK FOR SPRINGPROX 'LEGACY' - C reference samples
------------------------------------------------------------

This software is copyright (c) 2000-2010 PRO ACTIVE SAS.
Read LICENCE.txt for details.


Files in this folder are C source code that are used to test SpringProx API
implementations.

All examples must be linked to the SpringProx API library.
- Linux   : static link with libspringprox.a.
- Windows : dynamic link with springprox.dll

On a micro-controller target, together with the K531 OEM module, those sample
applications can be used to validate your porting of the SpringProx API.

On a computer target, together with a CSB or a SpringProx device, they allow
developers to find a quick reference covering most usages of the contactless
reader.


IMPORTANT DISCLAIMER
--------------------

Most of these sample applications will try to write or erase any smartcard/tag
you may put on the reader while running them. If they succeed, the previous
content of the smartcard/tag will be overwritten or erased.

DO NOT USE THOSE SAMPLE APPLICATIONS WITH ANY SMARTCARD OR A TAG THAT IS NOT
CLEARLY DEDICATED TO DEVELOPMENT AND TESTS !!!




Here's the list of supplied tools :


Linked with SpringProx API only (springprox.dll)
------------------------------------------------

ref_find.c         : basic cards/tags discovery process

ref_find_old.c     : basic cards/tags discovery process, compliant with earlier readers

ref_mifare.c       : demo for NXP Mifare Classic (1k/4k) memory cards

ref_mif_ul.c       : demo for NXP Mifare UltraLight memory cards

ref_15693.c        : demo for ISO 15693 RFID tags
                     (NB: this will work only on a reader featuring an ISO 15693 chipset)

ref_icode1.c       : demo for NXP ICODE1 RFID tags
                     (NB: this will work only on a reader featuring an ISO 15693 chipset)

ref_icode_sli_eas  : EAS management demo for NXP ICODE SLI and SLI-X (classic, S and L types) tags. 										 
										 (NB: this will work only on a reader featuring an ISO 15693 chipset)

ref_jewel.c        : demo for Innovision Jewel or Topaz tags
                     (NB: Topaz = NFC type 1 tag)

ref_askcts256.c    : demo for ASK CTS256B tags
                     (based on ISO 14443-B up to level 2, specific command set)

ref_askcts512.c    : demo for ASK CTS512B tags
                     (based on ISO 14443-B up to level 2, specific command set)

ref_stsr176.c      : demo for ST MicroElectronics SR176 (and compliant) tags
                     (based on ISO 14443-B up to level 2, specific command set)

ref_picopass.c     : demo for InsideContactless PicoPASS tags
                     (based on ISO 14443-B up to level 2, specific command set
                      this also works with HID iTag as it is the same chip)
                      
ref_smartcard.c    : demo for contact (T=0/T=1) smartcards
                     (NB: this will work only on a reader featuring smartcard slots)


Linked with SpringProx API for Mifare UltraLight C (sprox_mifulc.dll)
---------------------------------------------------------------------

ref_mif_ulc.c      : demonstration of operation with an NXP Mifare UltraLight C secure memory card


Linked with SpringProx API for Desfire (sprox_desfire.dll)
----------------------------------------------------------

ref_desfire.c      : demonstration of operation with an NXP Desfire smartcard
                     (single reader version)
                     
ref_desfire_ex.c   : same as above, multi-reader version
                     (linked to springprox_ex.dll and sprox_desfire_ex.dll)


Linked with SpringProx API for Calypso (sprox_calypso.dll)
----------------------------------------------------------

ref_calypso.c      : demonstration of operation with a Calypso transport smartcard
                     (XML dump of all the interesting records of 1TIC.ICA application)



