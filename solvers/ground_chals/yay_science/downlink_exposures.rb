exposureList = tlm("TELESCOPE HOUSEKEEPING EXPOSURES")

for index in 0 ... exposureList.size
  if exposureList[index] != 65535
    e = exposureList[index]
    puts("Exposure:#{e}")
    cmd("TELESCOPE EXPOSURE_REQUEST with CCSDS_STREAMID 6545, CCSDS_SEQUENCE 49152, CCSDS_LENGTH 3, CCSDS_FUNCCODE 2, CCSDS_CHECKSUM 0, EXPOSURE_NUMBER #{e}")    
    
    num =tlm("TELESCOPE EXPOSURE EXPOSURE_NUMBER")
    submitted = tlm("TELESCOPE EXPOSURE SUBMITTED_BY")
    data = tlm("TELESCOPE EXPOSURE DATA")
    fun = tlm("TELESCOPE EXPOSURE FUN")

    
    puts( "#{num}: submitted by #{submitted} with message #{fun}" )
  end
end