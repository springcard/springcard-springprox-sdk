/*
  Micore Library
  --------------

  Copyright (c) 2000-2008 SpringCard - www.springcard.com

  micore_picc.h
  --------------
  ISO 14443 defined PICC commands

  revision :
  ----------

  JDA 13/10/2003 : created for micore1
  JDA 07/10/2005 : routine cleanup
  LTC 06/04/2009 : included into micore2

*/

#ifndef __MICORE_PICC_H__
#define __MICORE_PICC_H__

#define PICC_REQIDL        0x26 //!< request idle
#define PICC_REQALL        0x52 //!< request all
#define PICC_ANTICOLL1     0x93 //!< anticollision level 1 106 kBaud
#define PICC_ANTICOLL11    0x92 //!< anticollision level 1 212 kBaud
#define PICC_ANTICOLL12    0x94 //!< anticollision level 1 424 kBaud
#define PICC_ANTICOLL13    0x98 //!< anticollision level 1 848 kBaud
#define PICC_ANTICOLL2     0x95 //!< anticollision level 2
#define PICC_ANTICOLL3     0x97 //!< anticollision level 3
#define PICC_AUTHENT1A     0x60 //!< authentication using key A
#define PICC_AUTHENT1B     0x61 //!< authentication using key B
#define PICC_READ16        0x30 //!< read 16 byte block
#define PICC_WRITE16       0xA0 //!< write 16 byte block
#define PICC_WRITE4        0xA2 //!< write 4 byte block
#define PICC_DECREMENT     0xC0 //!< decrement value
#define PICC_INCREMENT     0xC1 //!< increment value
#define PICC_RESTORE       0xC2 //!< restore command code
#define PICC_TRANSFER      0xB0 //!< transfer command code
#define PICC_HALT          0x50 //!< halt

#define PICC_TCL_RATS_A    0xE0
#define PICC_TCL_PPSS_A    0xD0

#endif
