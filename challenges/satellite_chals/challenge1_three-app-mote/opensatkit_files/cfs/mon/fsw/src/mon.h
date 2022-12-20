/* 
** Purpose: Define a MON application.
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

#ifndef MON_H
#define MON_H

#include "app_cfg.h"

extern MON_Class mon_Object;
extern MON_HkPkt mon_HkPkt;
extern MON_Token_Pkt mon_Token_Pkt;

/* Convience Macros */
#define  CMDMGR_OBJ (&(mon_Object.CmdMgr))
#define  TBLMGR_OBJ (&(mon_Object.TblMgr))

extern char mon_string[MAX_STRLEN+1];
extern int32_t (*validPtrs)[10];

int32_t initializeApp_MON(void);
int32_t appMainLoop_MON(void);


boolean MON_NoOpCmd(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr);
boolean MON_ResetAppCmd(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr);
boolean MON_DebugCmd(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr);

#endif