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

} MON_Class;

typedef struct
{

   uint8    Header[CFE_SB_TLM_HDR_SIZE];

   /*
   ** CMDMGR Data
   */
   uint16   ValidCmdCnt;
   uint16   InvalidCmdCnt;
} MON_HkPkt;
#define MON_TLM_HK_PKT_LEN sizeof (MON_HkPkt)

typedef struct
{

   uint8    Header[CFE_SB_TLM_HDR_SIZE];
 
   /*
   ** Buffer Save Data
   */

   uint8   token[MAX_TOKLEN];
} MON_Token_Pkt;
#define MON_TLM_TKN_PKT_LEN sizeof (MON_Token_Pkt)

typedef struct
{
    uint8   CmdHeader[CFE_SB_CMD_HDR_SIZE];
    uint16  countObj;
    void    (*aptr)();
    uint16  strSize;
    char    msgPayload[200];
} OS_PACK DEBUG_STRUCT;
#define DEBUG_STRUCT_LEN  (sizeof(DEBUG_STRUCT) - CFE_SB_CMD_HDR_SIZE)

#endif