missionList = tlm("TELESCOPE HOUSEKEEPING MISSIONS")

for index in 0 ... missionList.size
  if missionList[index] != 65535
    m = missionList[index]
    puts("Mission:#{m}")
    cmd("TELESCOPE MISSION_REQUEST with CCSDS_STREAMID 6545, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 3, CCSDS_FUNCCODE 3, CCSDS_CHECKSUM 0, MISSION_NUMBER 0")
    
    duration = tlm("TELESCOPE MISSION DURATION")
    start = tlm("TELESCOPE MISSION START_WINDOW")
    stop = tlm("TELESCOPE MISSION STOP_WINDOW")
    planet = tlm("TELESCOPE MISSION PLANET")
    id = tlm("TELESCOPE MISSION MISSION_ID")
    
    puts( "#{id}: Look at #{planet} for #{duration} secs between #{start} and #{stop}" )
  end
end