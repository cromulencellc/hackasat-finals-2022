/*
** Purpose: Define message IDs for the PUZZLEBOX application
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
#ifndef _puzzlebox_msgids_
#define _puzzlebox_msgids_

/*
** Command Message IDs
*/

#define  PUZZLEBOX_CMD_MID              0x1F50
#define  PUZZLEBOX_SEND_HK_MID          0x1885  /* Same as KIT_CI to simplify integration */

/*
** Telemetry Message IDs
*/

#define  PUZZLEBOX_TLM_HK_MID              0x0372
#define  PUZZLEBOX_STATUS_TLM_MID          0x0369
#endif /* _puzzlebox_msgids_ */
