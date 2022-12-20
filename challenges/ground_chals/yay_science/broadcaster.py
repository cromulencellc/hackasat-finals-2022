from spacepackets.ccsds.spacepacket import SpacePacketHeader, PacketType
import comms.pubsub_rabbit as rf
import base64
import json
import mission
import struct 
TELESCOPE_CMD_ID = 0x0191

def checksum( bytes ):
    #cry
    checksum = 0xFF
    for byte in bytes: 
        checksum ^= byte 
    return checksum



class MissionBroadcaster:
    # Ingest all the missions and broadcast them
    def __init__( self , interfaces ):
        self.missionList = {}
        self.count = 0 
        self.pub = rf.RabbitPublish( interfaces )
        
    def add_mission( self , missionCfg ):
        m = mission.Mission( missionCfg  )
        payloadBytes =  m.getPackedMission() 
        dataLenBytes = len(payloadBytes)-1+2 # -1 because CCSDS
        header = SpacePacketHeader( PacketType.TC , TELESCOPE_CMD_ID , self.count, dataLenBytes, sec_header_flag=True )
        header_as_bytes = bytes( header.pack( ) ) 
        
        secheader = [ 4 , 0 ]
        secHeaderBytes = struct.pack( "BB", *secheader)
        dataBytes = bytearray( header_as_bytes + secHeaderBytes + payloadBytes ) 

        checkSum = checksum(dataBytes)
        dataBytes[7] = checkSum
        dataBytes = bytes( dataBytes)
        self.missionList[self.count] = { 
            "tx_bytes":dataBytes,
            "endTime": m.endSeconds
        } 
        self.count = self.count+1
    def broadcast_packets( self , stationname ):
        topic = f"soc.scientists.tx_bytes.{stationname}"
        for key,mission in self.missionList.items():
            dataBytes = mission['tx_bytes']
            dataB64 = base64.b64encode( dataBytes ).decode()
            msg = { "time": {"seconds":0 , "nanoseconds":0}, 
                    "payload": dataB64 
                  }
            msgTxt = json.dumps(msg )

            self.pub.send( topic , msgTxt )
    
    def cleanup( self , currentSeconds ):
        keysToRemove = []
        for key,mission in self.missionList.items():
            endTime = mission["endTime"]
        
            if currentSeconds > endTime: 
                keysToRemove.append( key )
        for key in keysToRemove:
            self.missionList.pop( key )
