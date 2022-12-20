#include "puzzlebox.h"

int32_t initializeApp_PUZZLEBOX(void)
{
    int32 status = 0;

    /* Initialize cFE interfaces */
    /* create a pipe for this app to receive commands, and subscribe to specific command values */
    status = CFE_SB_CreatePipe(&puzzlebox_Object.CmdPipe, PUZZLEBOX_CMD_PIPE_DEPTH, PUZZLEBOX_CMD_PIPE_NAME);
    if (status != CFE_SUCCESS)
        return status;
    status = CFE_SB_Subscribe(PUZZLEBOX_CMD_MID, puzzlebox_Object.CmdPipe);
    if (status != CFE_SUCCESS)
        return status;
    CFE_SB_Subscribe(PUZZLEBOX_SEND_HK_MID, puzzlebox_Object.CmdPipe);
    
    /* Initialize App Framework Components */
    CMDMGR_Constructor(CMDMGR_OBJ);

    CMDMGR_RegisterFunc(CMDMGR_OBJ, CMDMGR_NOOP_CMD_FC,  NULL, PUZZLEBOX_NoOpCmd,     0);
    CMDMGR_RegisterFunc(CMDMGR_OBJ, CMDMGR_RESET_CMD_FC, NULL, PUZZLEBOX_ResetAppCmd, 0);
    CMDMGR_RegisterFunc(CMDMGR_OBJ, PUZZLEBOX_STAGE1_CMD_FC, NULL, PUZZLEBOX_Stage1Function, PUZZLEBOX_STVALS_LEN);
    CMDMGR_RegisterFunc(CMDMGR_OBJ, PUZZLEBOX_STAGE2_CMD_FC, NULL, PUZZLEBOX_Stage2Function, PUZZLEBOX_STVALS_LEN);
    CMDMGR_RegisterFunc(CMDMGR_OBJ, PUZZLEBOX_STAGE3_CMD_FC, NULL, PUZZLEBOX_Stage3Function, PUZZLEBOX_STVALSOPS_LEN);
    CMDMGR_RegisterFunc(CMDMGR_OBJ, PUZZLEBOX_STAGE4_CMD_FC, NULL, PUZZLEBOX_Stage4Function, PUZZLEBOX_STVALS_LEN);
    
    CFE_SB_InitMsg(&puzzlebox_HkPkt, PUZZLEBOX_TLM_HK_MID, PUZZLEBOX_TLM_HK_PKT_LEN, TRUE);
    CFE_SB_InitMsg(&puzzlebox_StatusPkt, PUZZLEBOX_STATUS_TLM_MID, PUZZLEBOX_TLM_STS_PKT_LEN, TRUE);

    /* Application startup event message */
    status = CFE_EVS_SendEvent(PUZZLEBOX_INIT_INFO_EID,
                               CFE_EVS_INFORMATION,
                               "%s Initialized. Version %d.%d.%d.%d",
                               APPNAME,
                               PUZZLEBOX_MAJOR_VERSION,
                               PUZZLEBOX_MINOR_VERSION,
                               PUZZLEBOX_REVISION,
                               PUZZLEBOX_MISSION_REV);
    if (status != CFE_SUCCESS)
        return status;

    OS_printf("%s initialization finished.\n", APPNAME);
    return CFE_SUCCESS;
} /* End of InitApp() */

