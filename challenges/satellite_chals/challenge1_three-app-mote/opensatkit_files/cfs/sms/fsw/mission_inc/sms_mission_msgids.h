/*
** Purpose: Define message IDs for the SMS application
**
** Notes:
**   None
**
** License:
**   Written by David McComas, licensed under the copyleft GNU General
**   Public License (GPL).
**
** References:
**   1. OpenSatKit Object-based Application Developer's Guide.
**   2. cFS Application Developer's Guide.
**
*/
#ifndef _sms_msgids_
#define _sms_msgids_

/*
** Command Message IDs
*/

#define  SMS_CMD_MID_DEF    0x1F70
extern uint32_t SMS_CMD_MID;
#define  SMS_CMD_MID_EXTEND 0x1F80
#define  SMS_SEND_HK_MID    0x1885  /* Same as KIT_CI to simplify integration */

/*
** Telemetry Message IDs
*/

#define  SMS_TLM_HK_MID     0x0369
// #define  SMS_TLM_TOK_MID    0x0337

#endif /* _sms_msgids_ */
