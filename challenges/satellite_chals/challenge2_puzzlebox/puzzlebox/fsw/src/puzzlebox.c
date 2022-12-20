#include "puzzlebox.h"


static void _ProcessCommands(void);
static void _SendHouseKeepingPacket(void);
void do_work(void);

static bool setupCompleted = FALSE;
static bool stage1Completed = FALSE;
static bool stage2Completed = FALSE;
static bool stage3Completed = FALSE;
static bool stage4Completed = FALSE;
static int inProgress = 0;

uint8_t stage_1();
uint8_t stage_2();
uint8_t stage_3(uint8_t idx);
uint8_t stage_4();
uint8_t encode1(uint8_t n);
uint8_t encode2(uint8_t n);
uint8_t encode3(uint8_t n);
void setToken(PUZZLEBOX_Status_Pkt *pkt);

uint8_t differs_1[16] =   {0x0d, 0x21, 0x22, 0x2c, 0x23, 0x51, 0x0f, 0x14, 0x2a, 0xeb, 0x2a, 0xe2, 0x22, 0xD8, 0x26, 0xc7};
uint8_t differs_2[16] =   {0x17, 0x63, 0xfc, 0x08, 0xfd, 0x91, 0x21, 0x17, 0x07, 0xa9, 0xf6, 0x13, 0xf4, 0x51, 0x0a, 0x2a };
uint8_t differs_3[16] =   {0xcb, 0x42, 0x21, 0x06, 0x06, 0xc3, 0xd2, 0xc5, 0x00, 0x4e, 0xa2, 0x22, 0x0a, 0x9f, 0x5a, 0x05 };

uint8_t stage_message[17];
uint8_t stage_ops[4];

uint8_t stage1Encrypt[17]={0x33, 0x33, 0x33, 0x33, 0x40, 0x40, 0x40, 0x40, 0x32, 0x32, 0x32, 0x32, 0x35, 0x35, 0x35, 0x35, 0x00}; 
uint8_t stage2Encrypt[17]={0x33, 0x33, 0x33, 0x33, 0x45, 0x45, 0x45, 0x45, 0x44, 0x44, 0x44, 0x44, 0x79, 0x79, 0x79, 0x79, 0x00}; 
uint8_t stage3Encrypt[17]={0x2b, 0x74, 0x51, 0x73, 0x2b, 0x74, 0x51, 0x73, 0x2b, 0x74, 0x51, 0x73, 0x2b, 0x74, 0x51, 0x73, 0x00}; 
uint8_t stage4Encrypt[17]={0x52, 0x3B, 0x2D, 0x65, 0x6A, 0x68, 0x22, 0x79, 0x3E, 0x36, 0x2A, 0x68, 0x5C, 0x22, 0x3C, 0x28, 0x00}; 
uint8_t addFunc(uint8_t a, uint8_t b)
{
    return a + stage_message[b];
}
uint8_t subFunc(uint8_t a, uint8_t b)
{
    return stage_message[b] - a;
}
uint8_t addFun(uint8_t a, uint8_t b)
{
    return a + b;
}
uint8_t subFun(uint8_t a, uint8_t b)
{
    return b - a;
}
uint8_t (*functions[16])( uint8_t, uint8_t ) = { subFunc, subFunc, subFunc, subFunc, addFunc, addFunc, addFunc, addFunc, subFunc, addFunc, subFunc, addFunc, addFunc, subFunc, addFunc, subFunc };
uint8_t (*encFunctions[4])(uint8_t) = { encode2, encode1, encode2, encode3 };
uint8_t (*encFuncs[3])(uint8_t) = { encode1, encode2, encode3 };

//stage4
uint8_t stage1vals[17] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t stage2vals[17] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t stage3vals[17] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t (*asFun4[8])( uint8_t, uint8_t ) = {addFun, subFun, subFun, subFun, addFun, addFun, addFun, subFun};
uint8_t (*whichStage4[8])[17] = {stage2vals, stage2vals, stage1vals, stage3vals, stage3vals, stage1vals, stage1vals, stage3vals};
uint8_t (*encFun4a[8])(uint8_t) = {encode2, encode3, encode2, encode1, encode2, encode1, encode1, encode3};
uint8_t (*encFun4b[8])(uint8_t) = {encode2, encode3, encode2, encode3, encode2, encode1, encode3, encode3};
uint8_t offset4[8] = {1,0,1,1,0,1,0,1};
uint8_t reverse4[8] = {0,0,1,0,0,0,0,1};

uint8_t results[17];

int32_t appMainLoop_PUZZLEBOX(void)
{
    OS_printf("%s App Main Loop\n", APPNAME);
    /* Main process loop */
    memset(stage_message, '\0', 17);
    uint32_t RunStatus = CFE_ES_APP_RUN;
    while (CFE_ES_RunLoop(&RunStatus))
    {
        OS_TaskDelay(PUZZLEBOX_RUNLOOP_DELAY);
        _ProcessCommands();
        do_work();
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

   Status = CFE_SB_RcvMsg(&CmdMsgPtr, puzzlebox_Object.CmdPipe, CFE_SB_POLL);

   if (Status == CFE_SUCCESS)
   {
        MsgId = CFE_SB_GetMsgId(CmdMsgPtr);
        FuncCode = CFE_SB_GetCmdCode(CmdMsgPtr);
        switch (MsgId)
        {
            case PUZZLEBOX_CMD_MID:
                OS_printf("%s App received command message(%d)(%d)\n", APPNAME, FuncCode, MsgId);
                
                CMDMGR_DispatchFunc(CMDMGR_OBJ, CmdMsgPtr);
                break;
            case PUZZLEBOX_SEND_HK_MID:
                _SendHouseKeepingPacket();
                break;
            default:
                CFE_EVS_SendEvent(PUZZLEBOX_CMD_INVALID_MID_ERR_EID, CFE_EVS_ERROR, "Received invalid command packet,MID = 0x%4X",MsgId);
                break;
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
    puzzlebox_HkPkt.ValidCmdCnt   = puzzlebox_Object.CmdMgr.ValidCmdCnt;
    puzzlebox_HkPkt.InvalidCmdCnt = puzzlebox_Object.CmdMgr.InvalidCmdCnt;

    CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &puzzlebox_HkPkt);
    CFE_SB_SendMsg((CFE_SB_Msg_t *) &puzzlebox_HkPkt);

} /* End _SendHouseKeepingPacket() */

static void determineStatus(PUZZLEBOX_Status_Pkt *pkt)
{
    if (setupCompleted)
    {
        memcpy(pkt->overall_status, SETUP_COMPLETE, SETUP_COMPLETE_LEN);
        if (stage1Completed)
            memcpy(pkt->stage1_status, UNLOCKED_STATUS, UNLOCKED_STATUS_LEN);
        else
            memcpy(pkt->stage1_status, LOCKED_STATUS, LOCKED_STATUS_LEN);
        if (stage2Completed)
            memcpy(pkt->stage2_status, UNLOCKED_STATUS, UNLOCKED_STATUS_LEN);
        else
            memcpy(pkt->stage2_status, LOCKED_STATUS, LOCKED_STATUS_LEN);
        if (stage3Completed)
            memcpy(pkt->stage3_status, UNLOCKED_STATUS, UNLOCKED_STATUS_LEN);
        else
            memcpy(pkt->stage3_status, LOCKED_STATUS, LOCKED_STATUS_LEN);
        if (stage4Completed)
            memcpy(pkt->stage4_status, UNLOCKED_STATUS, UNLOCKED_STATUS_LEN);
        else
            memcpy(pkt->stage4_status, LOCKED_STATUS, LOCKED_STATUS_LEN);
    }
    else
    {
        memcpy(pkt->overall_status, AWAITING_USER, AWAITING_USER_LEN);
        memcpy(pkt->stage1_status, LOCKED_STATUS, LOCKED_STATUS_LEN);
        memcpy(pkt->stage2_status, LOCKED_STATUS, LOCKED_STATUS_LEN);
        memcpy(pkt->stage3_status, LOCKED_STATUS, LOCKED_STATUS_LEN);
        memcpy(pkt->stage4_status, LOCKED_STATUS, LOCKED_STATUS_LEN);
    }
    if (stage1Completed && stage2Completed && stage3Completed && stage4Completed)
        setToken(pkt);
}

/******************************************************************************
** Function: _SendStatusPkt
**
*/
void _SendStatusPkt(void)
{
    determineStatus(&puzzlebox_StatusPkt);
    CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &puzzlebox_StatusPkt);
    CFE_SB_SendMsg((CFE_SB_Msg_t *) &puzzlebox_StatusPkt);
} /* End _SendStatusPkt() */

/******************************************************************************
** Function: PUZZLEBOX_NoOpCmd
**
** Function signature must match CMDMGR_CmdFuncPtr typedef 
*/
boolean PUZZLEBOX_NoOpCmd(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr)
{
    OS_printf("received NOOP command\n");
    CFE_EVS_SendEvent (PUZZLEBOX_CMD_NOOP_INFO_EID,
                        CFE_EVS_INFORMATION,
                        "No operation command received for %s version %d.%d",
                        APPNAME, PUZZLEBOX_MAJOR_VERSION, PUZZLEBOX_MINOR_VERSION);

    return TRUE;
} /* End PUZZLEBOX_NoOpCmd() */

/******************************************************************************
** Function: PUZZLEBOX_ResetAppCmd
**
** Function signature must match CMDMGR_CmdFuncPtr typedef 
*/
boolean PUZZLEBOX_ResetAppCmd(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr)
{
    OS_printf("%s received ResetApp command\n", APPNAME);
    CMDMGR_ResetStatus(CMDMGR_OBJ);
    inProgress = 0;
    stage1Completed = FALSE;
    stage2Completed = FALSE;
    stage3Completed = FALSE;
    stage4Completed = FALSE;
    memset(stage1vals, '\0', 16);
    memset(stage2vals, '\0', 16);
    memset(stage3vals, '\0', 16);
    memset(results, '\0', 16);
    memset(stage_message, '\0', 16);
    return TRUE;
} /* End PUZZLEBOX_ResetAppCmd() */

static uint32 op_a(const uint8  bytes[4], int mod);
static uint32 op_b(const uint8  bytes[4], int mod);
static uint32 op_c(const uint8  bytes[4], int mod);
static uint32 op_d(const uint8  bytes[4], int mod);

boolean PUZZLEBOX_Stage1Function(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr)
{
    OS_printf("received STAGE1 command\n");
    const PUZZLEBOX_StVals_t *Msg = (const PUZZLEBOX_StVals_t*) MsgPtr;

    if (inProgress)
        return TRUE;

    inProgress = 1;
    memcpy(stage_message, Msg->bytesA, 4);
    memcpy(&stage_message[4], Msg->bytesB, 4);
    memcpy(&stage_message[8], Msg->bytesC, 4);
    memcpy(&stage_message[12], Msg->bytesD, 4);
    return TRUE;
}
boolean PUZZLEBOX_Stage2Function(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr)
{
    OS_printf("received STAGE2 command\n");
    const PUZZLEBOX_StVals_t *Msg = (const PUZZLEBOX_StVals_t*) MsgPtr;

    if (inProgress)
        return TRUE;

    inProgress = 2;
    memcpy(stage_message, Msg->bytesA, 4);
    memcpy(&stage_message[4], Msg->bytesB, 4);
    memcpy(&stage_message[8], Msg->bytesC, 4);
    memcpy(&stage_message[12], Msg->bytesD, 4);
    return TRUE;
}
boolean PUZZLEBOX_Stage3Function(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr)
{
    OS_printf("received STAGE3 command\n");
    const PUZZLEBOX_StValsOps_t *Msg = (const PUZZLEBOX_StValsOps_t*) MsgPtr;

    if (inProgress)
        return TRUE;

    inProgress = 3;
    memcpy(stage_message, Msg->bytesA, 4);
    memcpy(&stage_message[4], Msg->bytesB, 4);
    memcpy(&stage_message[8], Msg->bytesC, 4);
    memcpy(&stage_message[12], Msg->bytesD, 4);
    stage_ops[0] = Msg->op_a;
    stage_ops[1] = Msg->op_b;
    stage_ops[2] = Msg->op_c;
    stage_ops[3] = Msg->op_d;
    return TRUE;
}
boolean PUZZLEBOX_Stage4Function(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr)
{
    OS_printf("received STAGE4 command\n");
    const PUZZLEBOX_StVals_t *Msg = (const PUZZLEBOX_StVals_t*) MsgPtr;

    if (inProgress)
        return TRUE;

    inProgress = 4;
    memcpy(stage_message, Msg->bytesA, 4);
    memcpy(&stage_message[4], Msg->bytesB, 4);
    memcpy(&stage_message[8], Msg->bytesC, 4);
    memcpy(&stage_message[12], Msg->bytesD, 4);
    return TRUE;
}

static uint32 op_a(const uint8  bytes[4], int mod)
{
    uint32 result = 1;

    switch(mod)
    {
        case 48:
            result = op_b(bytes, 0);
            break;
        case 44:
            result = op_b(bytes, 1);
            break;
        case 32:
            result = op_b(bytes, 48);
            break;
        case 1:
            // does work
            OS_printf("a is done\n");
            result = 372;
            break;
        case 0:
            result = op_b(bytes, 32);
            break;
        break;
        default:
            result = 0;
    }
    return result;
}
static uint32 op_b(const uint8  bytes[4], int mod)
{
    uint32 result = 1;

    switch(mod)
    {
        case 48:
            result = op_c(bytes, 0);
            break;
        case 44:
            result = op_c(bytes, 48);
            break;
        case 32:
            result = op_c(bytes, 1);
            break;
        case 1:
            // does work
            OS_printf("b is done\n");
            result = 372;
            break;
        case 0:
            result = op_c(bytes, 44);
            break;
        break;
        default:
            result = 0;
    }
    return result;
}
static uint32 op_c(const uint8  bytes[4], int mod)
{
    uint32 result = 1;

    switch(mod)
    {
        case 48:
            result = op_d(bytes, 32);
            break;
        case 44:
            result = op_d(bytes, 48);
            break;
        case 32:
            result = op_d(bytes, 44);
            break;
        case 1:
            // does work
            OS_printf("c is done\n");
            result = 372;
            break;
        case 0:
            result = op_d(bytes, 1);
            break;
        break;
        default:
            result = 0;
    }
    return result;
}
static uint32 op_d(const uint8  bytes[4], int mod)
{
    uint32 result = 1;

    switch(mod)
    {
        case 48:
            result = op_a(bytes, 1);
            break;
        case 44:
            result = op_a(bytes, 0);
            break;
        case 32:
            result = op_a(bytes, 44);
            break;
        case 1:
            // does work
            OS_printf("d is done\n");
            result = 372;
            break;
        case 0:
            result = op_d(bytes, 32);
            break;
        break;
        default:
            result = 0;
    }
    return result;
}

uint32 do_setup(void);
void check_setup(uint32 setupComplete);
bool check_stage(uint8_t *stage);

void do_work(void)
{
    memset(results, '\0', 17);
    if (!inProgress)
        return;
    if (!setupCompleted)
    {
        OS_printf("do setup\n");
        check_setup(do_setup());
    }
    else
    {
        if (inProgress == 1)
        {
            OS_printf("do stage 1 (%s)\n", stage_message);
            if (stage_1())
            {
                OS_printf("Check stage 1 (%s)\n", results);
                stage1Completed = check_stage(stage1Encrypt);
            }
        }
        if (inProgress == 2)
        {
            OS_printf("do stage 2 (%s)\n", stage_message);
            if (stage_2())
            {
                OS_printf("Check stage 2 (%s)\n", results);
                stage2Completed = check_stage(stage2Encrypt);
            }
        }
        if (inProgress == 3)
        {
            OS_printf("do stage 3 (%s)\n", stage_message);
            for (int i = 0; i < 4; i++)
                stage_3(i);
            OS_printf("Check stage 3 (%s)\n", results);
            stage3Completed = check_stage(stage3Encrypt);
        }
        if (inProgress == 4)
        {
            OS_printf("do stage 4 (%s)\n", stage_message);
            stage_4();
            OS_printf("Check stage 4 (%s)\n", results);
            stage4Completed = check_stage(stage4Encrypt);
        }
    }
    inProgress=0;
    memset(stage_message, '\0', 17);
    _SendStatusPkt();
}

uint32 do_setup(void)
{
    int i = 0;
    int invalid = 0;
    uint32 setupComplete = 0;
    for (int i=0; i < 4; i++)
        setupComplete += stage_message[i] << i;
    for (i=0; i<4; i++)
        invalid+= stage_message[4+i];
    for (i=0; i<4; i++)
        invalid+= stage_message[8+i];
    for (i=0; i<4; i++)
        invalid+= stage_message[12+i];
    if (invalid)
        setupComplete = 0;
    OS_printf("Setup: %d\n", setupComplete);
    return setupComplete;
}

void check_setup(uint32 setupComplete)
{
    int i=0;
    setupComplete -= ((i + 1) << i++);
    int resa = op_a(stage_message, setupComplete); // a/48
    setupComplete -= ((i + 1) << i++);
    int resb = op_b(&stage_message[4], setupComplete); // b/44
    setupComplete -= ((i + 1) << i++);
    int resc = op_c(&stage_message[8], setupComplete); // c/32
    setupComplete -= ((i + 1) << i++);
    int resd = op_d(&stage_message[12], setupComplete); // d/0

    if (resa == 372 && resb == 372 && resc == 372 && resd == 372)
    {
        OS_printf("Successful Setup\n");
        setupCompleted = TRUE;
    }
}

bool check_stage(uint8_t *stage)
{
    OS_printf("CHECK: %s vs %s\n", results, stage);
    if (strncmp((char*)results,(char*)stage,16) == 0)
    {
        OS_printf("you did the stage\n");
        return TRUE;
    }
    return FALSE;
}

uint8_t stage_1()
{
    int i;
    for (i = 0; i < 4; i++)
    {
        uint8_t chVal = functions[i](differs_1[i], i);
        if (chVal > 0x7f)
            return 0;
        results[i] = encode1(chVal);
    }
    for (i=4; i < 8; i++)
    {
        uint8_t chVal = functions[i](differs_1[i], i);
        if (chVal < 0x7f)
            return 0;
        results[i] = encode1(chVal);
    }
    for (i=8; i < 12; i++)
    {
        uint8_t chVal = functions[i](differs_1[i], i);
        if (chVal > 0x7f)
            return 0;
        results[i] = encode1(chVal);
    }
    for (i=12; i < 16; i++)
    {
        uint8_t chVal = functions[i](differs_1[i], i);
        if (chVal > 0x9f)
            return 0;
        results[i] = encode1(chVal);
    }
    memcpy(stage1vals, stage_message,16);
    return 1;
}

uint8_t stage_2()
{
    int i;
    int j=0;

    for (i = 0; i < 16; i+=4)
    {
        uint8_t chVal = functions[i](differs_2[i], i);
        if (chVal > 0x7f)
        {
            printf("failed\n");
            return 0;
        }
        uint8_t enc1Val= encode1(chVal);
        if (enc1Val<= 0x20 || enc1Val >= 0x7f )
            return 0;
        results[j++] = encode2(enc1Val);
        if (stage_message[j-1] <= 0x20 || stage_message[j-1] >= 0x7f)
            results[j-1] = 0x0;
        //printf("%x: %x: %x: %x \n", stage_message[i], chVal, enc1Val, results[i]);
    }
    for (i = 1; i < 16; i+=4)
    {
        uint8_t chVal = functions[i](differs_2[i], i);
        if (chVal < 0x7f)
        {
            printf("failed\n");
            return 0;
        }
        uint8_t enc1Val= encode1(chVal);
        if (enc1Val<= 0x20 || enc1Val >= 0x7f )
            return 0;
        enc1Val = (enc1Val >> 4) + ((enc1Val & 0xf) << 4); // flipbytes
        results[j++] = encode2(enc1Val);
        if (stage_message[j-1] <= 0x20 || stage_message[j-1] >= 0x7f)
            results[j-1] = 0x0;
        //printf("%x: %x: %x: %x \n", stage_message[i], chVal, enc1Val, results[i]);
    }
    for (i = 2; i < 16; i+=4)
    {
        uint8_t chVal = functions[i](differs_2[i], i);
        if (chVal > 0x6f)
        {
            printf("failed\n");
            return 0;
        }
        uint8_t enc1Val= encode1(chVal);
        if (enc1Val<= 0x20 || enc1Val >= 0x7f )
            return 0;
        results[j++] = encode2(enc1Val);
        if (stage_message[j-1] <= 0x20 || stage_message[j-1] >= 0x7f)
            results[j-1] = 0x0;
        //printf("%x: %x: %x: %x \n", stage_message[i], chVal, enc1Val, results[i]);
    }
    for (i = 3; i < 16; i+=4)
    {
        uint8_t chVal = functions[i](differs_2[i], i);
        if (chVal > 0x6f)
        {
            printf("failed\n");
            return 0;
        }
        uint8_t enc1Val= encode1(chVal);
        if (enc1Val<= 0x20 || enc1Val >= 0x7f )
            return 0;
        results[j++] = encode2(enc1Val);
        if (stage_message[j-1] <= 0x20 || stage_message[j-1] >= 0x7f)
            results[j-1] = 0x0;
        //printf("%x: %x: %x: %x \n", stage_message[i], chVal, enc1Val, results[i]);
    }
    memcpy(stage2vals, stage_message,16);
    return 1;
}

uint8_t stage_3(uint8_t idx)
{
    for (int i = idx; i < 16; i += 4)
    {
        uint8_t nxVal = functions[15-i](differs_3[i], i);
        if (nxVal >= 0x6f && encFuncs[stage_ops[idx]] == (void*)encode2)
            return 0;
        results[i] = encFuncs[stage_ops[idx]](nxVal);
    }
    memcpy(stage3vals, stage_message,16);
    return 1;
}

uint8_t stage_4()
{
    static uint8_t result[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    static bool stage4aRun = FALSE;
    static bool didStage4a = FALSE;
    static bool stage4bRun = FALSE;

    if (stage1Completed && stage2Completed && stage3Completed)
    {
        OS_printf("Running with all stages complete\n");
        stage4bRun = FALSE;
        stage4aRun = TRUE;
    }
    else
    {
        OS_printf("Running with no stages complete\n");
        stage4bRun = TRUE;
        stage4aRun = FALSE;
    }

    if (stage4aRun)
    {
        for (int i = 0; i < 8; i++)
        {
            uint8_t valuea = asFun4[i]((*whichStage4[i])[i], stage_message[i]);
            OS_printf("%d: 0x%x w/ 0x%x => 0x%x = ", i, stage_message[i], (*whichStage4[i])[i], valuea);
            result[i] = encFun4a[i](valuea);
            OS_printf("0x%x (%c)\n", result[i],result[i]);
        }
        stage4aRun = FALSE;
        stage4bRun = FALSE;
        didStage4a = TRUE;
    }
    if (stage4bRun)
    {
        if (!didStage4a)
            return 0;
        for (int i = 8; i < 16; i++)
        {
            uint8_t valueb = addFun(addFun(result[i-8], stage_message[i]),offset4[i-8]);
            OS_printf("%d: 0x%x = ", i, valueb);
            result[i] = encFun4b[i-8](valueb);
            if (reverse4[i-8])
                result[i] = (result[i] >> 4) + ((result[i] & 0xf) << 4);
            OS_printf("0x%x (%c) \n", result[i], result[i]);
        }
        memcpy(results, result,16);
    }
    return 1;
}

uint8_t encode1(uint8_t n) 
{  /* 7,9,13 triplet from http://www.retroprogramming.com/2017/07/xorshift-pseudorandom-numbers-in-z80.html */
    uint16_t start_state = n;
    uint16_t lfsr = start_state;
    uint16_t period = 0;
    do {
        lfsr ^= lfsr >> 7;
        lfsr ^= lfsr << 9;
        lfsr ^= lfsr >> 13;
        ++period;
    } while (lfsr != start_state && period != start_state);
    uint8_t res= ((lfsr >> 4) & 0xff);
    return res;
}
uint8_t encode2(uint8_t n) 
{
    uint16_t seed = n;
    uint16_t period = 0;
    do
    {
        uint16_t lsb = seed & 1;
        seed >>= 1;
        if (lsb)
            seed ^= 0x1EE7u;
        ++period;
    } while (period != seed);
    uint8_t res= ((seed >> 2) & 0xff);
  return res;
}
uint8_t encode3(uint8_t n)
{  /* taps: 16 14 13 11; feedback polynomial: x^16 + x^14 + x^13 + x^11 + 1 */
    uint16_t start_state = n;
    uint16_t lfsr = start_state;
    uint16_t bit;
    uint16_t period = 0;
    do { 
        bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1u;
        lfsr = (lfsr >> 1) | (bit << 15);
        ++period;
    } while (lfsr != start_state && period != start_state);
    uint8_t res= ((lfsr >> 3) & 0xff);
    return res;
}

void setToken(PUZZLEBOX_Status_Pkt *pkt)
{
    //open file read in token
    memcpy(pkt->token, "HAS3{HERE_IS_A_NEWLY_MINTED_TOKEN:)}\0", 37);
}