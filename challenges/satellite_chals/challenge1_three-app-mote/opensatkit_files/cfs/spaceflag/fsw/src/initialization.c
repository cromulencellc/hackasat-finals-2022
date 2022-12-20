#include "spaceflag.h"

int32_t initializeApp_SPACEFLAG(void)
{
    int32 status = 0;

    /* Initialize cFE interfaces */
    /* create a pipe for this app to receive commands, and subscribe to specific command values */
    status = CFE_SB_CreatePipe(&spaceflag_Object.CmdPipe, SPACEFLAG_CMD_PIPE_DEPTH, SPACEFLAG_CMD_PIPE_NAME);
    if (status != CFE_SUCCESS)
        return status;
    status = CFE_SB_Subscribe(SPACEFLAG_CMD_MID, spaceflag_Object.CmdPipe);
    if (status != CFE_SUCCESS)
        return status;
    // CFE_SB_Subscribe(MADE_UP_CONSTANT+0x10, thisObject.CmdPipe);
    CFE_SB_Subscribe(SPACEFLAG_SEND_HK_MID, spaceflag_Object.CmdPipe);
    
    /* Initialize App Framework Components */
    CMDMGR_Constructor(CMDMGR_OBJ);

    CMDMGR_RegisterFunc(CMDMGR_OBJ, CMDMGR_NOOP_CMD_FC,  NULL, SPACEFLAG_NoOpCmd,     0);
    CMDMGR_RegisterFunc(CMDMGR_OBJ, CMDMGR_RESET_CMD_FC, NULL, SPACEFLAG_ResetAppCmd, 0);
    CMDMGR_RegisterFunc(CMDMGR_OBJ, SPACEFLAG_SAVE_CMD_FC,  TBLMGR_OBJ, SPACEFLAG_String, SPACEFLAG_STRING_LEN);
    // REMOVED, so that users cannot call it, via normal means. CMDMGR_RegisterFunc(CMDMGR_OBJ, SPACEFLAG_GEN_CMD_FC,  NULL, SPACEFLAG_Send_Token, 0);

    CFE_SB_InitMsg(&spaceflag_HkPkt, SPACEFLAG_TLM_HK_MID, SPACEFLAG_TLM_HK_PKT_LEN, TRUE);
    CFE_SB_InitMsg(&spaceflag_Token_Pkt, SPACEFLAG_TLM_TOK_MID, SPACEFLAG_TLM_TKN_PKT_LEN, TRUE);

    /* Application startup event message */
    status = CFE_EVS_SendEvent(SPACEFLAG_INIT_INFO_EID,
                               CFE_EVS_INFORMATION,
                               "%s Initialized. Version %d.%d.%d.%d",
                               APPNAME,
                               SPACEFLAG_MAJOR_VERSION,
                               SPACEFLAG_MINOR_VERSION,
                               SPACEFLAG_REVISION,
                               SPACEFLAG_MISSION_REV);
    if (status != CFE_SUCCESS)
        return status;

    OS_printf("%s initialization finished.\n", APPNAME);
    return CFE_SUCCESS;
} /* End of InitApp() */

