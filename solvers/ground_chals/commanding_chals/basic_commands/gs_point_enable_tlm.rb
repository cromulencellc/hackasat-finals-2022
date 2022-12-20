# First: Select the current groundstation and set it to a point at a specific azimuth and elevation
# Second: Set the groundstations radio settings to match the radio settings of the satellite we'd like to communciate with
# Third: Enable telemetry on the satellite

puts 'Running spaceradio script'

station = "Kathmandu"
az = '180.0'
el = '70.0'
password = "DefaultPassword"
satChannel = '0'
scienceChannel = '15'
channel       = '1'
constellation = 'BPSK'
sps           = '12'
fec_repeat    = '4'
accesscode    = '0x1122'

# Select ground station
cmd('GS SELECT_STATION with OPCODE 1, STATION_NAME '+station)

# Request access
cmd("GS ACCESS_REQUEST with OPCODE 2, STATION_NAME "+station+", PAYLOAD Optional")
wait_check("GS ACCESS_STATUS ACCESS == \"GRANTED\"", 5)

# Configure Groundstation TX/RX
cmd('GS SET_STATION_RX_CONFIG with OPCODE 5, CHANNEL '+channel+', CONSTELLATION '+constellation+', SAMPLE_PER_SYMBOL '+sps+', FEC_REPEAT '+fec_repeat+', ACCESS_BYTES '+accesscode)
cmd('GS SET_STATION_TX_CONFIG with OPCODE 4, CHANNEL '+channel+', CONSTELLATION '+constellation+', SAMPLE_PER_SYMBOL '+sps+', FEC_REPEAT '+fec_repeat+', ACCESS_BYTES '+accesscode)

# Steer antenna
cmd('GS STEER_ANTENNA with OPCODE 3, AZIMUTH '+az+', ELEVATION '+el)

wait(5)
# Enable telemetry
cmd("KIT_TO ENABLE_TELEMETRY with CCSDS_STREAMID 6272, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 17, CCSDS_FUNCCODE 5, CCSDS_CHECKSUM 0, IP_ADDR 192.168.3.1")
# Make sure we are getting telemetry before proceeding. Did we actually establish contact with the satellite?