/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  SpringProx API
  --------------

  Copyright (c) 2000-2008 SpringCard SAS, FRANCE - www.springcard.com

  sprox_14443-4.h
  ---------------
  Helpers functions for T=CL (ISO 14443-4) tags

*/
#ifndef __SPROX_14443_4_H__
#define __SPROX_14443_4_H__

#define PICC_B_ATQ_PUPI_OFFSET 0
#define PICC_B_ATQ_APPLICATION_DATA_OFFSET 4
#define PICC_B_ATQ_PROTOCOL_INFO_OFFSET 8
#define PICC_B_ATQ_BITRATE_OFFSET PICC_B_ATQ_PROTOCOL_INFO_OFFSET+0
#define PICC_B_ATQ_FSCI_BYTE_OFFSET PICC_B_ATQ_PROTOCOL_INFO_OFFSET+1
#define PICC_B_ATQ_PROTOCOL_TYPE_BYTE_OFFSET PICC_B_ATQ_PROTOCOL_INFO_OFFSET+1
#define PICC_B_ATQ_FWI_BYTE_OFFSET PICC_B_ATQ_PROTOCOL_INFO_OFFSET+2
#define PICC_B_ATQ_ADC_BYTE_OFFSET PICC_B_ATQ_PROTOCOL_INFO_OFFSET+2
#define PICC_B_ATQ_FO_BYTE_OFFSET PICC_B_ATQ_PROTOCOL_INFO_OFFSET+2
#define PICC_B_ATQ_SFGI_BYTE_OFFSET PICC_B_ATQ_PROTOCOL_INFO_OFFSET+3

/* Max number of CID (0 to 14 = 15) */
#define TCL_CID_COUNT     15

/* Mark that we do not use CID */
#define TCL_UNUSED_CID    0xFF
/* Mark that we do not use NAD */
#define TCL_UNUSED_NAD    0xFF

/* Identification of a T=CL card into type A SAK */
#define PICC_SAK_ISO4     0x20

/* Identification of a NFCIP-1 object into type A SAK */
#define PICC_SAK_NFCIP1   0x40

/* Type mask in block header */
#define PCB_BLOCK_MASK    0xC2

/* Block type is I */
#define PCB_IS_I_BLOCK    0x02
/* Block type is R */
#define PCB_IS_R_BLOCK    0x82
/* Block type is S */
#define PCB_IS_S_BLOCK    0xC2

/* Define a new I block */
#define PCB_MK_I_BLOCK    0x02
/* Define a new R block */
#define PCB_MK_R_BLOCK    0xA2
/* Define a new S block */
#define PCB_MK_S_BLOCK    0xC2

/* Chaining indicator in block header */
#define PCB_CHAINING      0x10
/* CID indicator in block header */
#define PCB_CID_FOLLOWING 0x08
/* NAD indicator in block header */
#define PCB_NAD_FOLLOWING 0x04
/* BLOCK NUMBER flag */
#define PCB_BLOCK_NUMBER  0x01

/* ACK indicator in R block header */
#define PCB_R_ACK         0x00
/* NAK indicator in R block header */
#define PCB_R_NAK         0x10

/* DESELECT indicator in S block header */
#define PCB_S_DESELECT    0x00
/* WTX indicator in S block header */
#define PCB_S_WTX         0x30

#endif
