# Point the spacecraft at the sun
cmd("ADCS ALGO_SELECT with CCSDS_STREAMID 6544, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 3, CCSDS_FUNCCODE 2, CCSDS_CHECKSUM 0, EST_ALGO 0, CTRL_ALGO 1")
cmd("ADCS SET_TARGET_QUATERNION with CCSDS_STREAMID 6544, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 33, CCSDS_FUNCCODE 6, CCSDS_CHECKSUM 0, QX 0, QY 0.25504291, QZ -0.58833778, QS 0.76734071")