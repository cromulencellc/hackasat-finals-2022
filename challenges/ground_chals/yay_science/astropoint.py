import comms.pubsub_rabbit as rf 
import numpy as np
import json
import yaml 
import base64
import spacepackets
import queue
import struct
import generator
import broadcaster 
import grader 
import traceback
EXPOSURE_TLM_ID = 0x00D9
TLM_LEN  =214 # off by 7 becuase ccsds
class AstroPoint:
    def __init__( self  , interfaces , stationName , gameFile , challengeFile ):
        # load the missions
        f = open( gameFile ,'r')
        ymlTxt = f.read()
        game = yaml.safe_load( ymlTxt )
        # load the challenge file
        f = open( challengeFile ,'r')
        ymlTxt = f.read()
        self.challenge = yaml.safe_load( ymlTxt )
        # 
        self.missionList = []
        self.gradebook ={}
        for sat in game["satellites"]:
            satName = sat["name"]
            self.gradebook[satName] = grader.Grader( satName)
        self.timeUpdateCount = 0
        self.sub = rf.RabbitSubscriber( config=interfaces , blocking=True)
        self.pub = rf.RabbitPublish( config=interfaces )
        self.stationName = stationName  
        self.sats = {}
        self.gen = generator.MissionGen( game["game"]["start_time"] , challengeFile )
        self.broadcast = broadcaster.MissionBroadcaster( interfaces )
        self.lastBroadcast = -100000000000
        self.broadcastPeriod = self.challenge["settings"]["broadcastPeriod"]
        self.currentTime = 0

        #subscribe to messages
        self.sub.subscribe_to("timelord.advance")
        self.sub.subscribe_to(f"groundstation.{stationName}.rx_bytes.scientists")
        self.sub.subscribe_to("sim.*.state")
        self.sub.connect(queue_basename="astropoint")
    def run( self ):
        keep_going = True 
        while( keep_going ):
            try:
                topic,msg = self.sub.recv()
                topicSplit = topic.split('.')
                if topic == "timelord.advance":
                    self.handleTime( msg )
                elif (topicSplit[0] == "sim") and (topicSplit[2] == "state"):
                    sat = topicSplit[1]
                    self.handleState( sat , msg )
                elif (topicSplit[0] == "groundstation")   and \
                     (topicSplit[1] == self.stationName ) and \
                     (topicSplit[2] == "rx_bytes") and \
                     (topicSplit[3] == "scientists" ):
                    try:
                        self.handleBytes( msg )
                    except:
                        print( "SCIENCE BYTES CRASHED HANDLER" , flush=True)
                        traceback.print_exc()
                else:
                    print("Unknown!",flush=True)
            except:
                print("Really weird upper exception", flush=True)
                traceback.print_exc()
    def handleTime( self , msg ):
        self.timeUpdateCount += 1 
        msgD = json.loads( msg )
        secs = msgD["time"]["seconds"]
        self.currentTime = secs
        if 0 == (self.timeUpdateCount % 60):
            self.setRadio( )
        # Generate any new missions
        newMissionCfgs= self.gen.time_update( secs )
        for m in newMissionCfgs:
            print("Adding a new mission", flush=True)
            print( f"  Mission  {m.count}: From {m.missionStartSeconds} To {m.missionStopSeconds} point at {m.pointAt} for {m.durationSeconds}")
            self.broadcast.add_mission( m )
            for g in self.gradebook:
                self.gradebook[g].add_mission( m )
                self.announce_mission( m )
        # Transmit missions if required
        if (secs - self.lastBroadcast ) > self.broadcastPeriod:
            print("Broadcasting missions",flush=True)
            self.broadcast.broadcast_packets(self.stationName)
            self.lastBroadcast = secs
        # Cleanup any expired missions
        self.broadcast.cleanup( secs )
        # send a pointing command to the antenna
        topic = f"soc.scientists.pointing.{self.stationName}"
        msg = { "time":{"seconds":secs , "nanoseconds":0},
                "payload":{"az":0, "el":90}
              }
        msgTxt = json.dumps( msg )
        self.pub.send( topic , msgTxt )

        # send a report 
        msg = { "time":{"seconds":secs , "nanoseconds":0}
              }
        # Send current grade book 
        for satName,grade in  self.gradebook.items():
            satGrade = grade.getGradeState() 
            for key,item in satGrade.items():
                topic = f"science.{satName}.grade.{key}"
                msg["payload"] = {
                    "tGo": item["mission"].getTgo(),
                    "exposure": base64.b64encode( item["payload"]).decode() ,
                    "toSensor": "yes" if item["payloadSent"] else "no" ,
                    "scored":   "yes" if item["scored"] else "no"}
                msgTxt = json.dumps( msg  )
                self.pub.send( topic , msgTxt )

    def handleState( self , name, msg ):
        msgD = json.loads( msg )
        t = msgD["time"]["seconds"]
        p = msgD["payload"]
        pos = np.array([ p['x'],p['y'],p['z']])
        q = np.array([ p['qx'] ,p['qy'] ,p['qz'] ,p['q0']])
        w = np.array( [p['wx'],p['wy'],p['wz'] ])
        self.gradebook[name].evaluateMissions( t,pos,q )
        ok = True 
        while ok:
            try: 
                out = self.gradebook[name].getPayload()
                outB64 = base64.b64encode( out ).decode()
                topic = f"science.{name}.sensor"
                msg = {"time":{"seconds":t , "nanoseconds":0},
                       "payload": {"raw": outB64} 
                      }
                msgTxt = json.dumps( msg )
                self.pub.send(  topic , msgTxt )
                # send out an interesting message
                topic = f"science.exposure.interesting"
                msg = { "time":{ "seconds":self.currentTime , "nanoseconds":0} , "payload":{}}
                message = f"{name} completed a science exposure. Downlink exposure for points!"
                msg["payload"] = { 
                            "who":name,
                            "timeOfInterest":t-10,
                            "lookAt": name,
                            "message":message    }
                msgTxt = json.dumps( msg )
                self.pub.send(  topic , msgTxt )

            except queue.Empty:
                ok = False 
    def handleBytes( self , msg ):
        msgD = json.loads( msg )
        b64 = msgD["payload"]
        outBytes = base64.b64decode( b64 )
        self.evaluateBytes( outBytes )
    def evaluateBytes( self , byteData ):
        data = byteData[4:] #remove sync word
        packet = spacepackets.SpacePacketHeader.unpack( data )
        payload = data[12:]
        # First just ignore baiscally all other packets
        if packet.apid != EXPOSURE_TLM_ID:
            return
        if packet.data_len != (TLM_LEN):
            print("APID ok - length wrong?")
            return
        
        
        out = struct.unpack( "<H30s152s25s", payload)
        exposure_number = out[0]
        awardTo = out[1].split(b'\x00')[0].decode()
        exposure = out[2]
        message = out[3].split(b'\x00')[0].decode()
        
        for name,g in self.gradebook.items():
            complete = g.gradePayload( exposure )
            if complete:
                completedBy = name 
                
        
                msgOut = { "time":{ "seconds":self.currentTime , "nanoseconds":0.0},
                   "payload": "scored" 
                 }
                topic = f"science.{awardTo}.success"
                msgTxt = json.dumps( msgOut )
                self.pub.send( topic , msgTxt )
                # thats enough work - time to exit
                print(f"Exposure {exposure_number} was successfully turned in at{self.currentTime}" , flush=True)
                self.announce_points(  exposure_number , awardTo , message )
                return 
        print(f"Exposure {exposure_number} was downlinked did not result in a new score" , flush=True)
        self.announce_no_points( exposure_number , awardTo)
    def announce_no_points( self,  exposureNum , awardTo ):
        topic = f"science.score.interesting"
        msg = { "time":{ "seconds":int(self.currentTime) , "nanoseconds":0} , "payload":{}}
        message = f"Exposure {exposureNum} was turned in by {awardTo} from a satellite. No points awarded. Each mission can only be awarded points 1x per satellite"
        msg["payload"] = { 
                    "who":awardTo,
                    "timeOfInterest":self.currentTime,
                    "lookAt": self.stationName,
                    "message":message    }
        msgTxt = json.dumps( msg )
        self.pub.send(  topic , msgTxt )
    def announce_points( self , completedBy , awardTo , funMessage):
        topic = f"science.score.interesting"
        msg = { "time":{ "seconds":int(self.currentTime) , "nanoseconds":0} , "payload":{}}
        message = f"{completedBy} just turned in an exposure. Points have been awarded to {awardTo}. {funMessage}"
        msg["payload"] = { 
                    "who":awardTo,
                    "timeOfInterest":self.currentTime,
                    "lookAt": completedBy,
                    "message":message    }
        msgTxt = json.dumps( msg )
        self.pub.send(  topic , msgTxt )
    def announce_mission( self  , m ):
        # send out a mission message 
        topic = "science.mission"
        msg = { "time":{ "seconds":self.currentTime , "nanoseconds":0}}
        msg["payload"] = {  "target":m.pointAt,
                            "start": m.missionStartSeconds,
                            "stop" : m.missionStopSeconds,
                            "duration": m.durationSeconds }
        msgTxt = json.dumps( msg )
        self.pub.send(  topic , msgTxt )
        # send out an interesting message
        topic = f"science.missions.interesting"
        msg = { "time":{ "seconds":self.currentTime , "nanoseconds":0}}
        message = f"The science station just generated a new mission. Mission will be broadcast via RF"
        msg["payload"] = { 
                    "who":"scientists",
                    "timeOfInterest":self.currentTime,
                    "lookAt": "science-station",
                    "message":message    }
        msgTxt = json.dumps( msg )
        self.pub.send(  topic , msgTxt )
    def setRadio( self  ):
        #print("Conguring radio")
        radio = self.challenge["settings"]["radio"]
        msg = { "time":{ "seconds":self.currentTime , "nanoseconds":0}}
        topic = f"soc.scientists.rx_config.{self.stationName}"
        msg["payload"] = { 
                    "channel_id":radio["channel_id"],
                    "constellation":radio["constellation"],
                    "samples_per_symbol":radio["samples_per_symbol"],
                    "fec_repeat": radio["fec_repeat"],
                    "access_bytes": radio["access_bytes"]   }
        msgTxt = json.dumps( msg )
        self.pub.send(  topic , msgTxt )
        topic = f"soc.scientists.tx_config.{self.stationName}"
        msg["payload"] = { 
                "channel_id":radio["channel_id"],
                "constellation":radio["constellation"],
                "samples_per_symbol":radio["samples_per_symbol"],
                "fec_repeat": radio["fec_repeat"],
                "access_bytes": radio["access_bytes"]   }
        msgTxt = json.dumps( msg )
        self.pub.send(  topic , msgTxt )
        #print("Radio config sent")



