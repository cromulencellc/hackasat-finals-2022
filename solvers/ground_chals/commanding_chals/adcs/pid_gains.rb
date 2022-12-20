# Note: The constants being set in this script may not be good or bad - but the script can be run with good constants
# These perform a lot better by using some integral and derivative

# Set better pid constants
cmd("ADCS SET_CONST_W_GAINS with CCSDS_STREAMID 6544, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 25, CCSDS_FUNCCODE 3, CCSDS_CHECKSUM 0, P 0.05, I 0.005, D 0.03")
# Set tracking gain
cmd("ADCS SET_Q_TRACKING_GAIN with CCSDS_STREAMID 6544, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 57, CCSDS_FUNCCODE 5, CCSDS_CHECKSUM 0, P_A 0.1, I_A 0, D_A 0, P_W 0.05, I_W 0.005, D_W 0.03, W_LIMIT 1")
