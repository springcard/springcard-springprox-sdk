#ifndef SPRINGPROX_ERRNO_H
#define SPRINGPROX_ERRNO_H

/* Error codes */
/* ----------- */

#ifndef MI_OK

#define MI_OK                           (0)        /* Success */

/* Status -1 to -128 are returned by the coupler itself */
/* ---------------------------------------------------- */

#define MI_NOTAGERR                     (-1)       /* No answer (no card / card is mute) */
#define MI_CRCERR                       (-2)       /* Invalid CRC in card's response */
#define MI_EMPTY                        (-3)       /* No frame received (NFC mode) */
#define MI_AUTHERR                      (-4)       /* Card: Authentication failed or access denied */
#define MI_PARITYERR                    (-5)       /* Invalid parity bit(s) in card's response */
#define MI_CODEERR                      (-6)       /* NACK or status indicating error */
#define MI_CASCLEVEX                    (-7)       /* Too many anticollision loops */
#define MI_SERNRERR                     (-8)       /* Wrong LRC in card's serial number */
#define MI_LOCKED                       (-9)       /* Card or block locked */
#define MI_NOTAUTHERR                   (-10)      /* Card: Authentication must be performed first */
#define MI_BITCOUNTERR                  (-11)      /* Wrong number of bits in card's answer */
#define MI_BYTECOUNTERR                 (-12)      /* Wrong number of bytes in card's answer */
#define MI_VALUEERR                     (-13)      /* Card: Counter is invalid */
#define MI_TRANSERR                     (-14)      /* Card: Transaction error */
#define MI_WRITEERR                     (-15)      /* Card: Write failed */
#define MI_INCRERR                      (-16)      /* Card: Counter increase failed */
#define MI_DECRERR                      (-17)      /* Card: Counter decrease failed */
#define MI_READERR                      (-18)      /* Card: Read failed */
#define MI_OVFLERR                      (-19)      /* RC: FIFO overflow */
#define MI_POLLING                      (-20)      /* Polling mode pending */
#define MI_FRAMINGERR                   (-21)      /* Invalid framing in card's response */
#define MI_ACCESSERR                    (-22)      /* Card: Access error (bad address or denied) */
#define MI_UNKNOWN_COMMAND              (-23)      /* RC: Unknown command */
#define MI_COLLERR                      (-24)      /* A collision has occurred */
#define MI_COMMAND_FAILED               (-25)      /* Command execution failed */
#define MI_INTERFACEERR                 (-26)      /* Hardware error */
#define MI_ACCESSTIMEOUT                (-27)      /* RC: timeout */
#define MI_NOBITWISEANTICOLL            (-28)      /* More than one card found, but at least one does not support anticollision */
#define MI_EXTERNAL_FIELD               (-29)      /* An external RF field has been detected */
#define MI_QUIT                         (-30)      /* Polling terminated (timeout or break) */
#define MI_CODINGERR                    (-31)      /* Bogus status in card's response */
#define MI_CUSTERR                      (-32)      /* Card: Vendor specific error */
#define MI_CMDSUPERR                    (-33)      /* Card: Command not supported */
#define MI_CMDFMTERR                    (-34)      /* Card: Format of command invalid */
#define MI_CMDOPTERR                    (-35)      /* Card: Option(s) of command invalid */
#define MI_OTHERERR                     (-36)      /* Card: other error */

#define MI_TARGET_NOT_ACTIVE            (-38)      /* 26 The target active bit is not set */
#define MI_FIELD_DOWN                   (-39)      /* 27 The external RF field has been lost */

#define MI_WRONG_MODE                   (-59)      /* Command not available in this mode */
#define MI_WRONG_PARAMETER              (-60)      /* Wrong parameter for the command */

#define MI_CID_NOT_ACTIVE               (-71)      /* No active card with this CID */
#define MI_BAD_ATS_LENGTH               (-75)      /* Length error in card's ATS */
#define MI_ATTRIB_ERROR                 (-76)      /* Error in card's response to ATTRIB */
#define MI_BAD_ATS_FORMAT               (-77)      /* Format error in card's ATS */
#define MI_TCL_PROTOCOL                 (-78)      /* Fatal protocol error in card's response */
#define MI_TCL_TOO_MANY_RETRIES         (-79)      /* Too many errors while communicating with the card */
#define MI_BAD_PPS_FORMAT               (-87)      /* Format error in card's PPS response */
#define MI_PPS_ERROR                    (-88)      /* Other error in card's PPS response */
#define MI_CID_NOT_SUPPORTED            (-92)      /* The card doesn't support the CID feature */
#define MI_CID_ALREADY_ACTIVE           (-93)      /* A card is already active with this CID */


#define MI_UNKNOWN_FUNCTION            (-100)      /* Command not supported by the coupler */
#define MI_INTERNAL_ERROR              (-111)      /* Internal error in the coupler */
#define MI_BUFFER_OVERFLOW             (-112)      /* Internal buffer overflow */
#define MI_WRONG_LENGTH                (-125)      /* Wrong data length for the command */

#define MI_TIME_EXTENSION              (-128)      /* More time needed to process the command */

/* Status -129 to -255 are set by the library */
/* ------------------------------------------ */

#define MI_CARD_NOT_TCL                (-131)      /* Library: The found card doesn't support ISO 14443-4 */
#define MI_RESPONSE_OVERFLOW           (-132)      /* Library: Coupler's response is longer than application's buffer */
#define MI_RESPONSE_INVALID            (-133)      /* Library: Coupler's response is not formated as required */
#define MI_COMMAND_OVERFLOW            (-134)      /* Library: Command buffer is longer than allowed */

#define MI_FUNCTION_NOT_AVAILABLE      (-240)      /* Library: The coupler doesn't support this function */
#define MI_SER_LENGTH_ERR              (-241)      /* Library: Wrong length in Coupler's response */
#define MI_SER_CHECKSUM_ERR            (-242)      /* Library: Wrong checksum in Coupler's response */
#define MI_SER_PROTO_ERR               (-243)      /* Library: Protocol error in Coupler's response */
#define MI_SER_PROTO_NAK               (-244)      /* Library: The coupler has sent a NACK */
#define MI_SER_ACCESS_ERR              (-245)      /* Library: Access to the communication device failed */
#define MI_SER_TIMEOUT_ERR             (-246)      /* Library: Coupler communication timeout */
#define MI_SER_NORESP_ERR              (-247)      /* Library: No response from coupler */
#define MI_LIB_CALL_ERROR              (-248)      /* Library: Invalid function call */
#define MI_OUT_OF_MEMORY_ERROR         (-249)      /* Library: Memory allocation failed */
#define MI_READER_NAME_INVALID         (-250)      /* Library: The device or reader name parameter is invalid */
#define MI_READER_CONNECT_FAILED       (-251)      /* Library: Failed to connect to a remote reader */
#define MI_INVALID_READER_CONTEXT      (-252)      /* Library: The sprox_ctx parameter is invalid */
#define MI_LIB_INTERNAL_ERROR          (-253)      /* Library: An internal error has occured */


#endif
#endif
