#include "mon.h"


static void _ProcessCommands(void);
static void _SendHouseKeepingPacket(void);

int32_t appMainLoop_MON(void)
{
    OS_printf("%s App Main Loop\n", APPNAME);
    /* Main process loop */
    uint32_t RunStatus = CFE_ES_APP_RUN;
    while (CFE_ES_RunLoop(&RunStatus))
    {
        OS_TaskDelay(MON_RUNLOOP_DELAY);
        _ProcessCommands();
    }

    return RunStatus;
}

/* Function: ProcessCommands */
static void _ProcessCommands(void)
{
   int32           Status;
   CFE_SB_Msg_t*   CmdMsgPtr;
   CFE_SB_MsgId_t  MsgId;
   uint16          FuncCode;

   Status = CFE_SB_RcvMsg(&CmdMsgPtr, mon_Object.CmdPipe, CFE_SB_POLL);

   if (Status == CFE_SUCCESS)
   {
        MsgId = CFE_SB_GetMsgId(CmdMsgPtr);
        FuncCode = CFE_SB_GetCmdCode(CmdMsgPtr);

        if (MsgId == SMS_CMD_MID) {
            OS_printf("%s App received command message(%d)(%d)\n", APPNAME, FuncCode, MsgId);
            CMDMGR_DispatchFunc(CMDMGR_OBJ, CmdMsgPtr);
        }
        else if (MsgId == MON_SEND_HK_MID) {
            _SendHouseKeepingPacket();
        }
        else {
            CFE_EVS_SendEvent(MON_CMD_INVALID_MID_ERR_EID, CFE_EVS_ERROR, "Received invalid command packet,MID = 0x%4X",MsgId);
        }
   }
} /* End _ProcessCommands() */


/******************************************************************************
** Function: _SendHousekeepingPkt
**
*/

static void _SendHouseKeepingPacket(void)
{
    /* Good design practice in case app expands to more than one table */
        // const TBLMGR_Tbl* LastTbl = TBLMGR_GetLastTblStatus(TBLMGR_OBJ);

    /* CMDMGR Data */
    mon_HkPkt.ValidCmdCnt   = mon_Object.CmdMgr.ValidCmdCnt;
    mon_HkPkt.InvalidCmdCnt = mon_Object.CmdMgr.InvalidCmdCnt;

    CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &mon_HkPkt);
    CFE_SB_SendMsg((CFE_SB_Msg_t *) &mon_HkPkt);

} /* End _SendHouseKeepingPacket() */

/******************************************************************************
** Function: MON_NoOpCmd
**
** Function signature must match CMDMGR_CmdFuncPtr typedef 
*/
boolean MON_NoOpCmd(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr)
{
   OS_printf("received NOOP command\n");
   CFE_EVS_SendEvent (MON_CMD_NOOP_INFO_EID,
                      CFE_EVS_INFORMATION,
                      "No operation command received for %s version %d.%d",
                      APPNAME, MON_MAJOR_VERSION, MON_MINOR_VERSION);

   return TRUE;
} /* End MON_NoOpCmd() */


/******************************************************************************
** Function: MON_ResetAppCmd
**
** Function signature must match CMDMGR_CmdFuncPtr typedef 
*/

boolean MON_ResetAppCmd(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr)
{
   OS_printf("%s received ResetApp command\n", APPNAME);
   CMDMGR_ResetStatus(CMDMGR_OBJ);

   return TRUE;
} /* End MON_ResetAppCmd() */

boolean MON_DebugCmd(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr)
{
    OS_printf("received Modify command\n");
    /* Do some debug functionality - is not established for use */
    const DEBUG_STRUCT *Msg = (const DEBUG_STRUCT*) MsgPtr;


    OS_printf("%d stored\n", Msg->countObj);
    OS_printf("%p stored\n", Msg->aptr);
    OS_printf("%d stored\n", Msg->strSize);
    OS_printf("%s stored\n", Msg->msgPayload);
    
    OS_printf("looking for %p\n", (*validPtrs)[0]);
    if (Msg->aptr == NULL)
        return TRUE;
    for (int i = 0; i < 10; i++)
    {
        if (Msg->aptr == (*validPtrs)[i])
        {
            OS_printf("Same, same\n");
            (Msg->aptr)(DataObjPtr, MsgPtr);
            return TRUE;
        }
    }
    return TRUE;
} /* End MON_DebugCmd() */