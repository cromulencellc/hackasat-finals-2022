#include "mon.h"

int32_t initializeApp_MON(void)
{
    int32 status = 0;

    /* Initialize cFE interfaces */
    /* create a pipe for this app to receive commands, and subscribe to specific command values */
    status = CFE_SB_CreatePipe(&mon_Object.CmdPipe, MON_CMD_PIPE_DEPTH, MON_CMD_PIPE_NAME);
    if (status != CFE_SUCCESS)
        return status;
    status = CFE_SB_Subscribe(MON_CMD_MID_DEF, mon_Object.CmdPipe);
    if (status != CFE_SUCCESS)
        return status;
    // CFE_SB_Subscribe(MADE_UP_CONSTANT+0x10, thisObject.CmdPipe);
    CFE_SB_Subscribe(MON_SEND_HK_MID, mon_Object.CmdPipe);
    
    /* Initialize App Framework Components */
    CMDMGR_Constructor(CMDMGR_OBJ);

    CMDMGR_RegisterFunc(CMDMGR_OBJ, CMDMGR_NOOP_CMD_FC,  NULL, MON_NoOpCmd,     0);
    CMDMGR_RegisterFunc(CMDMGR_OBJ, CMDMGR_RESET_CMD_FC, NULL, MON_ResetAppCmd, 0);
    CMDMGR_RegisterFunc(CMDMGR_OBJ, MON_EXTEND_CMD_FC, NULL, MON_DebugCmd, DEBUG_STRUCT_LEN);

    CFE_SB_InitMsg(&mon_HkPkt, MON_TLM_HK_MID, MON_TLM_HK_PKT_LEN, TRUE);

    /* Application startup event message */
    status = CFE_EVS_SendEvent(MON_INIT_INFO_EID,
                               CFE_EVS_INFORMATION,
                               "%s Initialized. Version %d.%d.%d.%d",
                               APPNAME,
                               MON_MAJOR_VERSION,
                               MON_MINOR_VERSION,
                               MON_REVISION,
                               MON_MISSION_REV);
    if (status != CFE_SUCCESS)
        return status;

    OS_printf("%s initialization finished.\n", APPNAME);
    return CFE_SUCCESS;
} /* End of InitApp() */

