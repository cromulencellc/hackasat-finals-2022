/*
** Purpose: Define message IDs for the MON application
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
#ifndef _mon_msgids_
#define _mon_msgids_

/*
** Command Message IDs
*/

#define  MON_CMD_MID_DEF    0x1F80
extern uint32_t SMS_CMD_MID;
#define  MON_SEND_HK_MID    0x1885  /* Same as KIT_CI to simplify integration */

/*
** Telemetry Message IDs
*/

#define  MON_TLM_HK_MID     0x0366

#endif /* _mon_msgids_ */
