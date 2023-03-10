cmd("KIT_TO ENABLE_TELEMETRY with CCSDS_STREAMID 6272, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 17, CCSDS_FUNCCODE 5, CCSDS_CHECKSUM 0, IP_ADDR 192.168.3.1")

to_tlm_rc = tlm("KIT_TO HK_TLM_PKT RECEIVED_COUNT")
wait_check("PUZZLEBOX STATUS_TLM_PKT RECEIVED_COUNT != #{to_tlm_rc}", 10)

cmd("PUZZLEBOX STAGE_1 with CCSDS_STREAMID 6528, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 17, CCSDS_FUNCCODE 2, CCSDS_CHECKSUM 0, HEX_VALUE_1 [  ], HEX_VALUE_2 [  ], HEX_VALUE_3 [  ], HEX_VALUE_4 [  ]")

wait_check("PUZZLEBOX STATUS_TLM_PKT STATUS == 'Awaiting User Start'", 10)

# Start Puzzle
cmd("PUZZLEBOX STAGE_1 with CCSDS_STREAMID 6528, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 17, CCSDS_FUNCCODE 2, CCSDS_CHECKSUM 0, HEX_VALUE_1 [ 1, 2, 3, 4 ], HEX_VALUE_2 [  ], HEX_VALUE_3 [  ], HEX_VALUE_4 [  ]")

wait_check("PUZZLEBOX STATUS_TLM_PKT STATUS == 'Setup Complete'", 10)

# Stage 1
cmd("PUZZLEBOX STAGE_1 with CCSDS_STREAMID 6528, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 17, CCSDS_FUNCCODE 2, CCSDS_CHECKSUM 0, HEX_VALUE_1 [ 84, 104, 105, 115 ], HEX_VALUE_2 [ 95, 49, 115, 110 ], HEX_VALUE_3 [ 116, 95, 116, 104 ], HEX_VALUE_4 [ 101, 95, 97, 78 ]")

wait_check("PUZZLEBOX STATUS_TLM_PKT STAGE_1 == 'Unlocked'", 10)

# Stage 2
cmd("PUZZLEBOX STAGE_2 with CCSDS_STREAMID 6528, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 17, CCSDS_FUNCCODE 3, CCSDS_CHECKSUM 0, HEX_VALUE_1 [ 115, 87, 101, 82 ], HEX_VALUE_2 [ 95, 99, 72, 51 ], HEX_VALUE_3 [ 99, 75, 95, 55 ], HEX_VALUE_4 [ 104, 69, 95, 116 ]")

wait_check("PUZZLEBOX STATUS_TLM_PKT STAGE_2 == 'Unlocked'", 10)

# Stage 3
cmd("PUZZLEBOX STAGE_3 with CCSDS_STREAMID 6528, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 21, CCSDS_FUNCCODE 4, CCSDS_CHECKSUM 0, HEX_VALUE_1 [ 48, 107, 51, 110 ], ENC_ALGO_1 1, HEX_VALUE_2 [ 95, 112, 64, 57 ], ENC_ALGO_2 0, HEX_VALUE_3 [ 101, 95, 112, 82 ], ENC_ALGO_3 1, HEX_VALUE_4 [ 111, 76, 108, 121 ], ENC_ALGO_4 2")

wait_check("PUZZLEBOX STATUS_TLM_PKT STAGE_3 == 'Unlocked'", 10)

# Stage 4 - Part 1
cmd("PUZZLEBOX STAGE_4 with CCSDS_STREAMID 6528, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 17, CCSDS_FUNCCODE 5, CCSDS_CHECKSUM 0, HEX_VALUE_1 [ 95, 115, 48, 109 ], HEX_VALUE_2 [ 101, 116, 104, 33 ], HEX_VALUE_3 [  ], HEX_VALUE_4 [  ]")

pb_tlm_rc = tlm("PUZZLEBOX STATUS_TLM_PKT RECEIVED_COUNT")
wait_check("PUZZLEBOX STATUS_TLM_PKT RECEIVED_COUNT != #{pb_tlm_rc}", 10)

wait(10)

# Stage 4 - Part 2
## Stages 1, 2, 3 must be locked (bad entries reset)
cmd("PUZZLEBOX STAGE_1 with CCSDS_STREAMID 6528, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 17, CCSDS_FUNCCODE 2, CCSDS_CHECKSUM 0, HEX_VALUE_1 [ 84, 104, 105, 115 ], HEX_VALUE_2 [ 95, 49, 115, 110 ], HEX_VALUE_3 [ 116, 95, 116, 104 ], HEX_VALUE_4 [ 101, 95, 97, 65 ]")

wait_check("PUZZLEBOX STATUS_TLM_PKT STAGE_1 == 'It\\'s Locked'", 10)
cmd("PUZZLEBOX STAGE_2 with CCSDS_STREAMID 6528, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 17, CCSDS_FUNCCODE 3, CCSDS_CHECKSUM 0, HEX_VALUE_1 [ 115, 87, 101, 82 ], HEX_VALUE_2 [ 95, 99, 72, 51 ], HEX_VALUE_3 [ 99, 75, 95, 55 ], HEX_VALUE_4 [ 104, 69, 95, 113 ]")
wait_check("PUZZLEBOX STATUS_TLM_PKT STAGE_2 == 'It\\'s Locked'", 10)
cmd("PUZZLEBOX STAGE_3 with CCSDS_STREAMID 6528, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 21, CCSDS_FUNCCODE 4, CCSDS_CHECKSUM 0, HEX_VALUE_1 [ 48, 107, 51, 110 ], ENC_ALGO_1 1, HEX_VALUE_2 [ 95, 112, 64, 57 ], ENC_ALGO_2 0, HEX_VALUE_3 [ 101, 95, 112, 82 ], ENC_ALGO_3 1, HEX_VALUE_4 [ 111, 76, 108, 113 ], ENC_ALGO_4 2")
wait_check("PUZZLEBOX STATUS_TLM_PKT STAGE_3 == 'It\\'s Locked'", 10)

# Stage 4 Values:
cmd("PUZZLEBOX STAGE_4 with CCSDS_STREAMID 6528, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 17, CCSDS_FUNCCODE 5, CCSDS_CHECKSUM 0, HEX_VALUE_1 [ 95, 115, 48, 109 ], HEX_VALUE_2 [ 101, 116, 104, 33 ], HEX_VALUE_3 [ 110, 95, 116, 72 ], HEX_VALUE_4 [ 51, 114, 101, 46 ]")

wait_check("PUZZLEBOX STATUS_TLM_PKT STAGE_4 == 'Unlocked'", 10)

# Re-enter proper values for stages 1,2,3
cmd("PUZZLEBOX STAGE_1 with CCSDS_STREAMID 6528, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 17, CCSDS_FUNCCODE 2, CCSDS_CHECKSUM 0, HEX_VALUE_1 [ 84, 104, 105, 115 ], HEX_VALUE_2 [ 95, 49, 115, 110 ], HEX_VALUE_3 [ 116, 95, 116, 104 ], HEX_VALUE_4 [ 101, 95, 97, 78 ]")
wait_check("PUZZLEBOX STATUS_TLM_PKT STAGE_1 == 'Unlocked'", 10)
cmd("PUZZLEBOX STAGE_2 with CCSDS_STREAMID 6528, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 17, CCSDS_FUNCCODE 3, CCSDS_CHECKSUM 0, HEX_VALUE_1 [ 115, 87, 101, 82 ], HEX_VALUE_2 [ 95, 99, 72, 51 ], HEX_VALUE_3 [ 99, 75, 95, 55 ], HEX_VALUE_4 [ 104, 69, 95, 116 ]")
wait_check("PUZZLEBOX STATUS_TLM_PKT STAGE_2 == 'Unlocked'", 10)
cmd("PUZZLEBOX STAGE_3 with CCSDS_STREAMID 6528, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 21, CCSDS_FUNCCODE 4, CCSDS_CHECKSUM 0, HEX_VALUE_1 [ 48, 107, 51, 110 ], ENC_ALGO_1 1, HEX_VALUE_2 [ 95, 112, 64, 57 ], ENC_ALGO_2 0, HEX_VALUE_3 [ 101, 95, 112, 82 ], ENC_ALGO_3 1, HEX_VALUE_4 [ 111, 76, 108, 121 ], ENC_ALGO_4 2")
wait_check("PUZZLEBOX STATUS_TLM_PKT STAGE_3 == 'Unlocked'", 10)

pb_token = tlm("PUZZLEBOX STATUS_TLM_PKT TOKEN")

if (pb_token and pb_token.length > 0)then
  puts "Solved puzzlebox challenge!: #{pb_token}"
else
  puts "WARNING: Could not solve challenge. Got empty token."
end
  