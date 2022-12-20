/*
** Purpose: Define platform configurations for the PUZZLEBOX application
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

#ifndef _puzzlebox_platform_cfg_
#define _puzzlebox_platform_cfg_

/******************************************************************************
** Application Macros
*/
#define  PUZZLEBOX_RUNLOOP_DELAY              500  /* Delay in milliseconds */
#define  PUZZLEBOX_CMD_PIPE_DEPTH             10
#define  PUZZLEBOX_CMD_PIPE_NAME              APPNAME /* name must be 1 to 15 characters in length */


/*
** Macro Definitions
*/

#define PUZZLEBOX_INIT_INFO_EID               (PUZZLEBOX_BASE_EID + 0)
#define PUZZLEBOX_EXIT_ERR_EID                (PUZZLEBOX_BASE_EID + 1)
#define PUZZLEBOX_CMD_NOOP_INFO_EID           (PUZZLEBOX_BASE_EID + 2)
#define PUZZLEBOX_CMD_INVALID_MID_ERR_EID     (PUZZLEBOX_BASE_EID + 3)

#define PUZZLEBOX_STAGE1_CMD_FC               (CMDMGR_APP_START_FC + 0)
#define PUZZLEBOX_STAGE2_CMD_FC               (CMDMGR_APP_START_FC + 1)
#define PUZZLEBOX_STAGE3_CMD_FC               (CMDMGR_APP_START_FC + 2)
#define PUZZLEBOX_STAGE4_CMD_FC               (CMDMGR_APP_START_FC + 3)
#endif /* _puzzlebox_platform_cfg_ */