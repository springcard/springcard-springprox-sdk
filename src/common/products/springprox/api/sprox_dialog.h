/*
  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.

  SpringProx API
  --------------

  Copyright (c) 2000-2008 SpringCard SAS, FRANCE - www.springcard.com

  sprox_dlg.h
  -----------
  Dialog with the reader.

*/
#ifndef SPROX_DIALOG_H
#define SPROX_DIALOG_H

/*
 * CARD FUNCTIONS (GEMCORE)
 * ------------------------
 */

#define SPROX_CARD_DIRECT            0x10
#define SPROX_CARD_ISO_EXCHANGE      0x11
#define SPROX_CARD_FUNCTION          0x12

#define SPROX_CARD_FUNC_GET_FIRMWARE 0x01
#define SPROX_CARD_FUNC_DIRECTORY    0x02
#define SPROX_CARD_FUNC_CARD_CONFIG  0x10
#define SPROX_CARD_FUNC_CARD_STATUS  0x11
#define SPROX_CARD_FUNC_POWER_UP     0x12
#define SPROX_CARD_FUNC_POWER_DOWN   0x13
#define SPROX_CARD_FUNC_ISO_OUTPUT   0x21
#define SPROX_CARD_FUNC_ISO_INPUT    0x22

 /*
  * MSO FUNCTIONS (SAGEM fingerprint reader)
  * ---------------------------------------
  */

#define SPROX_MSO_EXCHANGE           0x15
#define SPROX_MSO_FUNCTION           0x16

#define SPROX_MSO_FUNC_OPEN          0x01
#define SPROX_MSO_FUNC_CLOSE         0x02

  /*
   * CSB-3 COMPLIANT FUNCTIONS
   * -------------------------
   */
#define SPROX_CSB_READ_EXT              0x36
#define SPROX_CSB_WRITE_EXT             0x37
#define SPROX_CSB_GET_SECURITY_STATUS   0x38
#define SPROX_CSB_GET_SYSTEM_INFO       0x39

#define SPROX_CSB_SELECT_ANY            0x40
#define SPROX_CSB_REQUEST               0x41
#define SPROX_CSB_ANTICOLL              0x42
#define SPROX_CSB_SELECT                0x43
#define SPROX_CSB_SELECT_AGAIN          0x44
#define SPROX_CSB_HALT                  0x45
#define SPROX_CSB_READ                  0x46
#define SPROX_CSB_WRITE                 0x47
#define SPROX_CSB_READ_SECT             0x48
#define SPROX_CSB_READ_BLOC             0x49
#define SPROX_CSB_WRITE_SECT            0x4A
#define SPROX_CSB_WRITE_BLOC            0x4B
#define SPROX_CSB_LOAD_KEY              0x4C
#define SPROX_CSB_SELECT_IDLE           0x4D
#define SPROX_CSB_RESET                 0x4E
#define SPROX_CSB_GET_INFOS             0x4F
#define SPROX_CSB_GET_CAPABILITIES      0x50
   //#define CSB_CLR_CONTROL_BIT           0x51
#define SPROX_CSB_CONFIG                0x52
//#define CSB_CHECK_WRITE               0x53
//#define CSB_SET_PORT                  0x54
//#define CSB_GET_PORT                  0x55
#define SPROX_CSB_AUTHENTICATION2       0x56
#define SPROX_CSB_VALUE                 0x57
#define SPROX_CSB_LOCK                  0x59


/*
 * Low level control functions
 * ---------------------------
 */

#define SPROX_CONTROL                   0x58
#include "sprox_control.h"

 /*
  * FIND
  * ----
  */
#define SPROX_FIND                      0x60

#define SPROX_FIND_INFO                 0x61
#define SPROX_FIND_INFO_UID             0x01
#define SPROX_FIND_INFO_PROT_BYTES      0x02
#define SPROX_FIND_INFO_ATR             0x03

#define SPROX_STORAGE                   0x62

#define SPROX_LPCD                      0x63


  /*
   * Echo and repeat please
   * ----------------------
   */
#define SPROX_ECHO                      0x7F
#define SPROX_REPEAT_PLEASE             0x80

   /*
	* T=CL functions
	* --------------
	*/

#define SPROX_TCL_FUNC                  0x81
#define SPROX_TCL_EXCHANGE              0x82

#define SPROX_TCL_FUNC_EXCH             0x00
#define SPROX_TCL_FUNC_EXCH_FSC         0x01
#define SPROX_TCL_FUNC_EXCH_FSC_CID     0x02
#define SPROX_TCL_FUNC_EXCH_FSC_CID_NAD 0x03
#define SPROX_TCL_FUNC_RATS             0x10
#define SPROX_TCL_FUNC_PPS              0x11
#define SPROX_TCL_FUNC_DESELECT         0x12
#define SPROX_TCL_FUNC_ATTRIB           0x20
#define SPROX_TCL_FUNC_NFC_ATR_REQ      0x40
#define SPROX_TCL_FUNC_DESFIRE          0xDF

#define SPROX_NFC_FUNC                  0x83
#define SPROX_NFC_FUNC_GET_EVENT        0x00
#define SPROX_NFC_FUNC_MODE_CONTROL     0x10

#define SPROX_NFC_FUNC_ICCD_CONTROL     0x90

#define SPROX_NFC_EXCHANGE              0x84

#define SPROX_ICCD_FUNC                 0x85
#define SPROX_ICCD_APDU                 0x86

	/*
	 * Pegoda compatibility
	 * --------------------
	 */

#define SPROX_PICCCOMMONREQUEST         0x8A
#define SPROX_PICCCASCSELECT            0x8B
#define SPROX_PICCAUTHE2                0x8C
#define SPROX_PICCAUTHKEY               0x8D
#define SPROX_PICCCOMMONREAD            0x8E
#define SPROX_PICCCOMMONWRITE           0x8F
#define SPROX_PICCVALUE                 0x90
#define SPROX_PICCVALUEDEBIT            0x91
#define SPROX_PICCHALT                  0x92
#define SPROX_PICCCASCANTICOLL          0x93
#define SPROX_PICCEXCHANGEBLOCK_A       0x94
#define SPROX_PICCACTIVATEIDLE          0x95
#define SPROX_PICCACTIVATEWAKEUP        0x96
#define SPROX_PICCEXCHANGEBLOCK_B       0x97
#define SPROX_PICCEXCHANGEBLOCK_V       0x98
#define SPROX_PICCEXCHANGEBLOCK_A_RAW   0x99

#define SPROX_PCDCONFIG                 0x9E
#define SPROX_PCDRESET                  0x9F

#define SPROX_PCDLOADKEYE2              0xA8
#define SPROX_PCDREADE2                 0xA9
#define SPROX_PCDWRITEE2                0xAA
#define SPROX_PCDREADRCREG              0xAB
#define SPROX_PCDWRITERCREG             0xAC
#define SPROX_PCDCLRRCREG               0xAD
#define SPROX_PCDSETRCREG               0xAE
#define SPROX_PCDSETTMO                 0xAF
#define SPROX_EXCHANGEBYTESTREAM        0xB0

#define SPROX_PCDSETDEFAULTATTRIB       0xB2
#define SPROX_PCDSETATTRIB              0xB3
#define SPROX_PCDGETATTRIB              0xB4


#define SPROX_PCDREADEFDT               0xFE


	 /*
	  * Communication protocol defines
	  * ------------------------------
	  */

	  /* Buffer size */
#ifdef BUILD_SPROX_FIRMWARE
#define SPROX_FRAME_CONTENT_SIZE     (256+20)
#endif
#ifdef WIN32
#define SPROX_FRAME_CONTENT_SIZE     (1024+20)
#endif
#ifdef __linux__
#define SPROX_FRAME_CONTENT_SIZE     (1024+20)
#endif
#ifndef SPROX_FRAME_CONTENT_SIZE
#define SPROX_FRAME_CONTENT_SIZE     (256+20)
#endif

#define ASCII_SOH  0x01               /* Start of header  */
#define ASCII_STX  0x02               /* Start of text    */
#define ASCII_ETX  0x03               /* End of text      */
#define ASCII_EOT  0x04               /* End of text      */
#define ASCII_ENQ  0x05               /* Enquire          */
#define ASCII_ACK  0x06               /* Acknowledge      */
#define ASCII_BEL  0x07               /* Bell             */
#define ASCII_BS   0x08               /* Delete           */
#define ASCII_HS   0x09               /*                  */
#define ASCII_LF   0x0A               /* Line feed        */
#define ASCII_VT   0x0B               /*                  */
#define ASCII_FF   0x0C               /*                  */
#define ASCII_CR   0x0D               /* Carriage Return  */
#define ASCII_SO   0x0E               /*                  */
#define ASCII_SI   0x0F               /*                  */

#define ASCII_DLE  0x10               /* Data link escape */

#define ASCII_XON  0x11
#define ASCII_XOFF 0x13

#define ASCII_DC1  0x11               /*                  */
#define ASCII_DC2  0x12               /*                  */
#define ASCII_DC3  0x13               /*                  */
#define ASCII_DC4  0x14               /*                  */
#define ASCII_NAK  0x15               /* Not acknowledge  */
#define ASCII_SYN  0x16               /* Sync             */
#define ASCII_ETB  0x17               /*                  */
#define ASCII_CAN  0x18               /*                  */
#define ASCII_EM   0x19               /*                  */
#define ASCII_SUB  0x1A               /*                  */
#define ASCII_ESC  0x1B               /*                  */
#define ASCII_FS   0x1C               /*                  */
#define ASCII_GS   0x1D               /*                  */
#define ASCII_RS   0x1E               /*                  */
#define ASCII_US   0x1F               /*                  */

/*
 * NAK explanation (when applicable)
 * ---------------------------------
 */

#define NACK_OVERRUN   0x09
#define NACK_WRONG_LEN 0x0A
#define NACK_WRONG_CRC 0x0B
#define NACK_OVERFLOW  0x0C
#define NACK_PROTOCOL  0x0D
#define NACK_TIMEOUT   0x0E
#define NACK_HARDWARE  0x0F

#endif
