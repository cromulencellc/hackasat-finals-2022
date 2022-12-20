/* 
** Purpose: Define a PUZZLEBOX application.
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

#ifndef PUZZLEBOX_H
#define PUZZLEBOX_H

#include "app_cfg.h"

#define AWAITING_USER           "Awaiting User Start\0"
#define AWAITING_USER_LEN       sizeof(AWAITING_USER)
#define SETUP_COMPLETE          "Setup Complete\0"
#define SETUP_COMPLETE_LEN      sizeof(SETUP_COMPLETE)
#define LOCKED_STATUS           "It's Locked\0"
#define LOCKED_STATUS_LEN       sizeof(LOCKED_STATUS)
#define UNLOCKED_STATUS         "Unlocked\0"
#define UNLOCKED_STATUS_LEN     sizeof(UNLOCKED_STATUS)


extern PUZZLEBOX_Class puzzlebox_Object;
extern PUZZLEBOX_HkPkt puzzlebox_HkPkt;
extern PUZZLEBOX_Status_Pkt puzzlebox_StatusPkt;

/* Convience Macros */
#define  CMDMGR_OBJ (&(puzzlebox_Object.CmdMgr))
#define  TBLMGR_OBJ (&(puzzlebox_Object.TblMgr))

extern char puzzlebox_string[MAX_STRLEN+1];

int32_t initializeApp_PUZZLEBOX(void);
int32_t appMainLoop_PUZZLEBOX(void);


boolean PUZZLEBOX_NoOpCmd(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr);
boolean PUZZLEBOX_ResetAppCmd(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr);

boolean PUZZLEBOX_Stage1Function(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr);
boolean PUZZLEBOX_Stage2Function(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr);
boolean PUZZLEBOX_Stage3Function(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr);
boolean PUZZLEBOX_Stage4Function(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr);

void _SendStatusPkt(void);

#endif