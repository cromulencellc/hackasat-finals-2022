/*
** Purpose: Define platform configurations for the MON application
**
** Notes:
**   None
**
** License:
**   Template written by David McComas and licensed under the GNU
**   Lesser General Public License (LGPL).
**
** References:
**   1. OpenSatKit Object-based Application Developer's Guide.
**   2. cFS Application Developer's Guide.
**
*/

#ifndef _mon_platform_cfg_
#define _mon_platform_cfg_

/******************************************************************************
** Application Macros
*/
#define  MON_RUNLOOP_DELAY              500  /* Delay in milliseconds */
#define  MON_CMD_PIPE_DEPTH             10
#define  MON_CMD_PIPE_NAME              APPNAME /* name must be 1 to 15 characters in length */


/*
** Macro Definitions
*/

#define MON_INIT_INFO_EID               (MON_BASE_EID + 0)
#define MON_EXIT_ERR_EID                (MON_BASE_EID + 1)
#define MON_CMD_NOOP_INFO_EID           (MON_BASE_EID + 2)
#define MON_CMD_INVALID_MID_ERR_EID     (MON_BASE_EID + 3)

#define MON_MODIFY_CMD_FC               2
#define MON_EXTEND_CMD_FC               3
#endif /* _mon_platform_cfg_ */