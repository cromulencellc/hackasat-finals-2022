# Three-App-Monte

Type confusion leads to memory corruption

The purpose of this challenge is to provide a vulnerability on the satellite system that could potentially be reached by other teams. This would allow collection of tokens from other teams satellites as a reward or a gameplay mechanism for "control" of the satellite.
The challenge is spread across 3 apps, with the main RE portions in 2 of the 3 apps.

App name mapping from original dev challenge to the integrated finals challenge
GEN_TOK_MSK -> SPACEFLAG
NORMAL_APP  -> SMS
VULN_APP    -> MON

## POC:
0. Get address from debug of GEN_TOK_MSG - leak
1. Enable telemetry, if you forgot
2. Send GEN_TOK_MSG:STRING with SOME_STRING as a "teamname1882381283"
3. Send NORMAL_APP:NORMAL_MSG with INTERNAL_USE = 1722 and MESSAGE: "THIS is my long message to check if it changes my value, but it needs to be long enough"
4. Send NORMAL_APP:EXTENDED_MSG with COUNT_CHK and INTERNAL_USE set to values corresponding to address from GEN_TOK_MSG
   * requires taking an address like 0x94901288 and reversing it to 0x12889490 or 310940816
5. View telemetry for GEN_TOK_MSG:TOKEN_TLM_PKT


## Structure
There are 3 apps: GEN_TOK_MSG, NORMAL_APP, VULN_APP

### GEN_TOK_MSG
This app is used to get a 64 character string from the groundstation and convert it to a 64 character token. The input string from the groundstation is intended to be a unique name for the team. The conversion function is intended to be some one-way, crypto function (such as sha).

1. The app stores the string input via command: `GEN_TOK_MSG_CMD_MID` and function code: `GEN_TOK_MSG_SAVE_CMD_FC`. 
2. The app processes the string and returns the token via a call to the function: `GEN_TOK_MSG_Send_Token`. However, this function is not accessible via the command/function code mechanism, as the function has not been registered with the `CMDMGR_RegisterFunc`
3. The app stores GEN_TOK_MSG_Send_Token into a list of pointers, which is then memprotect to read-only to prevent teams from having full access from the function pointer overwrite.

The intent is for this function to be easily discoverable, but it cannot be called by normal functionality. The teams must determine how to call this function after the string has been populated.

The mechanism for calling the token-providing function is in the following two apps.

### NORMAL_APP
This app provides some normal functionality for the satellite. The purpose of this app is to provide a mechanism for changing a command message id variable. The command message id change is to prevent users from calling the vulnerable app commands directly, adding a necessity to RE both the NORMAL and VULN apps together to solve the challenge.

The app receives a message with the id: `NORMAL_APP_CMD_MID`. This message id is not a `#define` constant like normal, instead it is a variable. 

When this message id is received with a specific value in the `INTERNAL_USE` variable and the string portion of the message is larger than 64 bytes, then the message id is changed by adding 0x10 and the message is ignored.

As a reset mechanism, if this app receives any other message (not housekeeping), then the value of `NORMAL_APP_CMD_MID` is restored to the default value.

There is an additional function: `NORMAL_APP_ExtendedCmd` present in the with the intent that it would be called for this new command code. However, the app never registers the command for its use and thereby prevents it from ever being called. This is intended for the challenge.

This part of the functionality is intended for the teams to easily miss, as it is doing something normal and ignoring actions otherwise. The teams will need to understand why there are additional messages and what the code in this app is doing with those messages. That is they must understand how the command messages work and how this app modifies that behavior.

This app sets up the mechanism for use in the VULN_APP.

The groundstation will send: 
`NORMAL_APP_CMD_MID` (0x1F70) for the struct:
```c
typedef struct
{
    uint8   CmdHeader[CFE_SB_CMD_HDR_SIZE];
    uint32  an_integer;
    uint32  another_integer;
    char    Payload[100];
} NORMAL_APP_DATA_t;
#define NORMAL_APP_DATA_LEN  (sizeof(NORMAL_APP_DATA_t) - CFE_SB_CMD_HDR_SIZE)
```
and `NORMAL_APP_ExtendedCmd` (0x1F80)

```c
typedef struct
{
    uint8   CmdHeader[CFE_SB_CMD_HDR_SIZE];
    uint32  an_integer;
    uint32  another_integer;
    char    longPayload[200];
} NORMAL_APP_EXTEND_DATA_t;
#define NORMAL_APP_EXT_DATA_LEN  (sizeof(NORMAL_APP_EXTEND_DATA_t) - CFE_SB_CMD_HDR_SIZE)
```

### VULN_APP
This app provides the functionality for vulnerability and exploitation. It has a type confusion vulnerability. It incorrectly uses the command id modified in the `NORMAL_APP` and by doing so applies an incorrect struct to the payload of the message. This allows teams do take control of a function pointer. The process included in this app could be added into an app that already exists.

Normally, message id: `NORMAL_APP_ExtendedCmd` (0x1F80) uses the struct shown in previous section.
however in this app uses the struct:
```c
typedef struct
{
    uint8   CmdHeader[CFE_SB_CMD_HDR_SIZE];
    uint16  countObj;
    void    (*aptr)();
    uint16  strSize;
    char    msgPayload[200];
} OS_PACK DEBUG_STRUCT;
#define DEBUG_STRUCT_LEN  (sizeof(DEBUG_STRUCT) - CFE_SB_CMD_HDR_SIZE)
```
The uint32 values in the `NORMAL_APP_EXTEND_DATA_t` will modify the void*aptr (in a reverse byte order). This allows a team to enter an address to which the function pointer will be used when called.  The `aptr` is checked versus pointers list defined and made read-only in gen_tok_msg app to prevent uncontrolled memory access from the function pointer corruption. If the address is the address for the function `GEN_TOK_MSG_Send_Token` then the token message will be sent. If this is a sequence performed on another teams satellite, then the token for that team will be returned. Huzzah! Gameplay setup will be required for groundstation control of other satellites. 


### Changing app names, easily
Any app's name can be changed by following these directions:
using   `<ORI_NAME>`: original name all capital letters, such as GEN_TOK_MSG
        `<ori_name>`: orignal name all lower case letters, such as gen_tok_msg
        `<NEW_NAME>`: new name all capital letters, such as DIFFERENT_NAME
        `<new_name>`: new name all lower case letters, such as different_name
1. Navigate to the directory `<ori_name>` in terminal
2. install rename, if required: sudo apt install rename
3. find . -exec rename 's/normal_app/vuln_app/' {} +
4. find ./ -type f -exec sed -i -e 's/`<ORI_NAME>`/`<NEW_NAME>`/g' {} \;
5. find ./ -type f -exec sed -i -e 's/`<ori_name>`/`<new_name>`/g' {} \;
6. modify top level directory name
```shell
cd ../
mv <ori_name> <new_name>
```
This app will have the exact same functionality, message ids, commands and telemetry functions. 

NOTE: names will need to be updated in the files defined in the following section.



### Getting the messages to work
#### OPENSATKIT
##### TELEMETRY 
file: `cfs/defcon_defs/cpu1_osk_to_pkt_tbl.json`
added telemetry housekeeping messages for each app and one telemetry message for token response

```json
"packet": {
        "name": "GEN_TOK_MSG_TLM_HK_MID",
        "stream-id": "\u0372",
        "dec-id": 882,
        "priority": 0,
        "reliability": 0,
        "buf-limit": 4,
        "filter": { "type": 2, "X": 1, "N": 1, "O": 0}
     },
     "packet": {
        "name": "GEN_TOK_MSG_TLM_TOK_MID",
        "stream-id": "\u0337",
        "dec-id": 823,
        "priority": 0,
        "reliability": 0,
        "buf-limit": 4,
        "filter": { "type": 2, "X": 1, "N": 1, "O": 0}
     },
     "packet": {
        "name": "NORMAL_APP_TLM_HK_MID",
        "stream-id": "\u0369",
        "dec-id": 873,
        "priority": 0,
        "reliability": 0,
        "buf-limit": 4,
        "filter": { "type": 2, "X": 1, "N": 1, "O": 0}
     },
     "packet": {
        "name": "VULN_APP_TLM_HK_MID",
        "stream-id": "\u0366",
        "dec-id": 870,
        "priority": 0,
        "reliability": 0,
        "buf-limit": 4,
        "filter": { "type": 2, "X": 1, "N": 1, "O": 0}
     },
```
* dec-id/stream-id probably ought to be randomized

##### COMMANDS
file: `cfs/apps/kit_ci/fsw/tables`
add lines for command messages for apps:
```
{   .MsgId             = 0x1F60},
{   .MsgId             = 0x1F70},
{   .MsgId             = 0x1F80},
```
* probably ought to be randomized

##### Build and Run opensatkit
file: `cfs/defcon_defs/targets.make`

add lowercase appnames to `SET(TGT1_APPLIST ...)` list
```
gen_tok_msg normal_app vuln_app
```

###### Minimum for functionality
```
SET(TGT1_APPLIST  cfs_lib osk_app_fw kit_ci kit_to kit_sch gen_tok_msg normal_app vuln_app)
```

file: `cfs/defcon_defs/cpu1_cfe_es_startup.scr`
add lines for each app
```
CFE_APP, /cf/gen_tok_msg.so,    appStart_GEN_TOK_MSG,       GEN_TOK_MSG,    101, 16384, 0x0, 0;
CFE_APP, /cf/normal_app.so,     appStart_NORMAL_APP,        NORMAL_APP,     102, 16384, 0x0, 0;
CFE_APP, /cf/vuln_app.so,       appStart_VULN_APP,          VULN_APP,       103, 16384, 0x0, 0;
```

###### Minimum for functionality
```
CFE_LIB, /cf/cfs_lib.so,        CFS_LibInit,                CFS_LIB,          0,      0, 0x0, 0;
CFE_LIB, /cf/osk_app_lib.so,    OSK_APP_FwInit,             OSK_APP_FW,       0,   8192, 0x0, 0;
CFE_APP, /cf/kit_to.so,         KIT_TO_AppMain,             KIT_TO,          21,  81920, 0x0, 0;
CFE_APP, /cf/kit_ci.so,         KIT_CI_AppMain,             KIT_CI,          20,  16384, 0x0, 0;
CFE_APP, /cf/kit_sch.so,        KIT_SCH_AppMain,            KIT_SCH,         10,  16384, 0x0, 0;

CFE_APP, /cf/gen_tok_msg.so,    appStart_GEN_TOK_MSG,       GEN_TOK_MSG,    110, 16384, 0x0, 0;
CFE_APP, /cf/normal_app.so,     appStart_NORMAL_APP,        NORMAL_APP,     111, 16384, 0x0, 0;
!
```

#### COSMOS-HAS3-SAT

##### TARGETS

file: `plugin.txt`

add entries for apps
```
TARGET GEN_TOK_MSG GEN_TOK_MSG
TARGET NORMAL_APP NORMAL_APP
TARGET VULN_APP VULN_APP

MAP_TARGET GEN_TOK_MSG
MAP_TARGET NORMAL_APP
MAP_TARGET VULN_APP
```

##### Message IDs

file `lib/fsw_msg_id.rb`

add entry for ids to be used by apps, names are used in seperate files
```
###################
##  GEN_TOK_MSG  ##
###################

GEN_TOK_MSG_CMD_MID       = "0x1F60"
GEN_TOK_MSG_HK_TLM_MID    = "0x0372"
GEN_TOK_MSG_TOK_TLM_MID   = "0x0337"

##################
##  NORMAL_APP  ##
##################

NORMAL_APP_CMD_MID       = "0x1F70"
NORMAL_APP_EXT_CMD_MID   = "0x1F80"
NORMAL_APP_HK_TLM_MID    = "0x0369"


##################
##  VULN_APP  ##
##################

VULN_APP_CMD_MID         = "0x1F80"
VULN_APP_HK_TLM_MID      = "0x0366"
```
* probably ought to be randomized

Add directories for each app in targets directory
```shell
cd targets
mkdir -p GEN_TOK_APP/cmd_tlm
mkdir NORMAL_APP/cmd_tlm
mkdir VULN_APP/cmd_tlm
```
Add `*_cmd.txt` and `*_tlm.txt` files to the appropriate directories

