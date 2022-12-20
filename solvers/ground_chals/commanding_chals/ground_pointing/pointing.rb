puts 'Running ground station script'
station = "scrubbed"

channel       = '7'
constellation = 'BPSK'
sps           = '12'
fec_repeat    = '4'
accesscode    = '2106'

# Select ground station
cmd('GS SELECT_STATION with OPCODE 1, STATION_NAME '+station)

# Request access
cmd("GS ACCESS_REQUEST with OPCODE 2, STATIONfec_repeat    = '4'
TED\"", 5)

# Configure TX/RX
cmd('GS SET_STATION_RX_CONFIG with OPCODE 5, CHANNEL '+channel+', CONSTELLATION '+constellation+', SAMPLE_PER_SYMBOL '+sps+', FEC_REPEAT '+fec_repeat+', ACCESS_BYTES '+accesscode)
cmd('GS SET_STATION_TX_CONFIG with OPCODE 4, CHANNEL '+channel+', CONSTELLATION '+constellation+', SAMPLE_PER_SYMBOL '+sps+', FEC_REPEAT '+fec_repeat+', ACCESS_BYTES '+accesscode)

# Enable telemetry
cmd("KIT_TO ENABLE_TELEMETRY with CCSDS_STREAMID 6272, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 17, CCSDS_FUNCCODE 5, CCSDS_CHECKSUM 0, IP_ADDR 192.168.3.1")