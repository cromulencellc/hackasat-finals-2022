#ifndef STRUCTS_H
#define STRUCTS_H

#define MAX_STRLEN      100
#define MAX_NORM_STRLEN 64
#define MAX_TOKLEN      50

typedef struct
{

   CMDMGR_Class  CmdMgr;
   TBLMGR_Class  TblMgr;
   uint32_t BufferSaveCnt;

   CFE_SB_PipeId_t CmdPipe;

} PUZZLEBOX_Class;

typedef struct
{

   uint8    Header[CFE_SB_TLM_HDR_SIZE];

   /*
   ** CMDMGR Data
   */
   uint16   ValidCmdCnt;
   uint16   InvalidCmdCnt;
} PUZZLEBOX_HkPkt;
#define PUZZLEBOX_TLM_HK_PKT_LEN sizeof (PUZZLEBOX_HkPkt)
typedef struct
{
    uint8 Header[CFE_SB_TLM_HDR_SIZE];
    uint8 overall_status[MAX_STRLEN];
    uint8 stage1_status[MAX_STRLEN];
    uint8 stage2_status[MAX_STRLEN];
    uint8 stage3_status[MAX_STRLEN];
    uint8 stage4_status[MAX_STRLEN];
    uint8 token[MAX_STRLEN];
} PUZZLEBOX_Status_Pkt;
#define PUZZLEBOX_TLM_STS_PKT_LEN sizeof (PUZZLEBOX_Status_Pkt)

typedef struct
{
   uint8 CmdHeader[CFE_SB_CMD_HDR_SIZE];
   uint8 bytesA[4];
   uint8 bytesB[4];
   uint8 bytesC[4];
   uint8 bytesD[4];
} PUZZLEBOX_StVals_t;
#define PUZZLEBOX_STVALS_LEN  (sizeof(PUZZLEBOX_StVals_t) - CFE_SB_CMD_HDR_SIZE)
typedef struct
{
   uint8 CmdHeader[CFE_SB_CMD_HDR_SIZE];
   uint8 bytesA[4];
   uint8 op_a;
   uint8 bytesB[4];
   uint8 op_b;
   uint8 bytesC[4];
   uint8 op_c;
   uint8 bytesD[4];
   uint8 op_d;
} PUZZLEBOX_StValsOps_t;
#define PUZZLEBOX_STVALSOPS_LEN  (sizeof(PUZZLEBOX_StValsOps_t) - CFE_SB_CMD_HDR_SIZE)

#endif