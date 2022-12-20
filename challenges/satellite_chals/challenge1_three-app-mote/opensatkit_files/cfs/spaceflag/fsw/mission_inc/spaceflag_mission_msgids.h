/*
** Purpose: Define message IDs for the SPACEFLAG application
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
#ifndef _spaceflag_msgids_
#define _spaceflag_msgids_

/*
** Command Message IDs
*/

#define  SPACEFLAG_CMD_MID        0x1F60
#define  SPACEFLAG_SEND_HK_MID    0x1885  /* Same as KIT_CI to simplify integration */

/*
** Telemetry Message IDs
*/

#define  SPACEFLAG_TLM_HK_MID     0x0372
#define  SPACEFLAG_TLM_TOK_MID    0x0337

#endif /* _spaceflag_msgids_ */
