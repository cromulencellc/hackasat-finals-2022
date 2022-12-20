# Assumptions
# The list of planets below is correct
planets = {}
planets['mercury']=[0.79945982 0.22258322 0.14965385 0.53751691]
planets['venus']=[0.79400144 0.16899794 0.12156743 0.57115915]
planets['earth']=[ 0.84228325 -0.05783623 -0.03671327  0.5346644 ]
planets['mars']=[ 0.82098161 -0.13202863 -0.08819791  0.54843301]
planets['jupiter']=[0.66472209 0.40558055 0.32679064 0.53559017]
planets['saturn']=[0.74952468 0.30556082 0.22168583 0.54378372]
planets['neptune']=[0.55744925 0.48121949 0.44207376 0.51210246]
# mission number
missionNumber =0

skewTime = 120
buffer = 80
# Request a certain mission

duration = 0
timeUntilWindow = 0

# Wait until the window occurs

q = [0,0,0,1]
# Point the satellite
cmd("ADCS SET_TARGET_QUATERNION with CCSDS_STREAMID 6544, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 33, CCSDS_FUNCCODE 6, CCSDS_CHECKSUM 0, QX " +q[0] +", QY "+q[1]+", QZ "+q[2]+", QS "+q[3])
cmd("ADCS ALGO_SELECT with CCSDS_STREAMID 6544, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 3, CCSDS_FUNCCODE 2, CCSDS_CHECKSUM 0, EST_ALGO 0, CTRL_ALGO 1")

# Wait for the duration of the mission
wait( skewTime )
wait( duration )
wait( buffer )

# Wait until you are above a certain latitude (sciene station are polar)
while( keep_going ):
    x=0
    y=0
    z=0
    lat = atan( z / sqrt( y*y + x*x))*180/3.1415
    keep_going 
# change your radio settings to match those of the science station

# requrest the exposure so that you and science get it

# change your radio settings again to avoid pwn