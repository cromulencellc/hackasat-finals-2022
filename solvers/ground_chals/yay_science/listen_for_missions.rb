puts 'Running spaceradio script'
station = "some_station"
password = "scrubbed"

scienceChannel = '15'
channel       = '1'
constellation = 'BPSK'
sps           = '12'
fec_repeat    = '4'
accesscode    = '0x78EE'
# Select ground station
cmd('GS SELECT_STATION with OPCODE 1, STATION_NAME '+station)

# Request access
cmd("GS ACCESS_REQUEST with OPCODE 2, STATION_NAME "+station+", PAYLOAD Optional")
wait_check("GS ACCESS_STATUS ACCESS == \"GRANTED\"", 5)

# Configure Groundstation TX/RX
cmd('GS SET_STATION_RX_CONFIG with OPCODE 5, CHANNEL '+channel+', CONSTELLATION '+constellation+', SAMPLE_PER_SYMBOL '+sps+', FEC_REPEAT '+fec_repeat+', ACCESS_BYTES '+accesscode)
cmd('GS SET_STATION_TX_CONFIG with OPCODE 4, CHANNEL '+channel+', CONSTELLATION '+constellation+', SAMPLE_PER_SYMBOL '+sps+', FEC_REPEAT '+fec_repeat+', ACCESS_BYTES '+accesscode)

# Enable telemetry
cmd("KIT_TO ENABLE_TELEMETRY with CCSDS_STREAMID 6272, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 17, CCSDS_FUNCCODE 5, CCSDS_CHECKSUM 0, IP_ADDR 192.168.3.1")
# Make sure we are getting telemetry before proceeding. Did we actually establish contact with the satellite?
wait_packet('EPS', 'BATTERY', 1, 10, 0.1)

# Now lets change to some new radio settings on the spacecraft
scienceChannel = '15' # 1 is the science station
scienceConstellation = 'QPSK'
scienceSps           = '4'
scienceAccesscode    = '0x69A1'

# Change the space radio config to w
cmd("RADIO UPDATE_RADIO_CONFIG with CCSDS_STREAMID 6547, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 22, CCSDS_FUNCCODE 3, CCSDS_CHECKSUM 0, ACCESS_BYTES "+scienceAccesscode+", FREQUENCY 1, SAMPLE_PER_SYMBOL "+scienceSps+", CONSTELLATION "+scienceConstellation + ", PASSWORD "+password)
# Configure Groundstation TX/RX
cmd('GS SET_STATION_RX_CONFIG with OPCODE 5, CHANNEL '+scienceChannel+', CONSTELLATION '+scienceConstellation+', SAMPLE_PER_SYMBOL '+scienceSps+', FEC_REPEAT '+fec_repeat+', ACCESS_BYTES '+scienceAccesscode)
cmd('GS SET_STATION_TX_CONFIG with OPCODE 4, CHANNEL '+scienceChannel+', CONSTELLATION '+scienceConstellation+', SAMPLE_PER_SYMBOL '+scienceSps+', FEC_REPEAT '+fec_repeat+', ACCESS_BYTES '+scienceAccesscode)
# Have we re-established communications using the new radio settings?
wait( 70 )

cmd("RADIO UPDATE_RADIO_CONFIG with CCSDS_STREAMID 6547, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 22, CCSDS_FUNCCODE 3, CCSDS_CHECKSUM 0, ACCESS_BYTES "+accesscode+", FREQUENCY 0, SAMPLE_PER_SYMBOL "+sps+", CONSTELLATION "+constellation + ", PASSWORD "+password)

# Configure Groundstation TX/RX
cmd('GS SET_STATION_RX_CONFIG with OPCODE 5, CHANNEL '+channel+', CONSTELLATION '+constellation+', SAMPLE_PER_SYMBOL '+sps+', FEC_REPEAT '+fec_repeat+', ACCESS_BYTES '+accesscode)
cmd('GS SET_STATION_TX_CONFIG with OPCODE 4, CHANNEL '+channel+', CONSTELLATION '+constellation+', SAMPLE_PER_SYMBOL '+sps+', FEC_REPEAT '+fec_repeat+', ACCESS_BYTES '+accesscode)
