import json
import unittest
import sys
sys.path.append('.')
import multiprocessing
import astropoint
import comms.pubsub_rabbit as rf
import time 
testInterfaces = "unittest/testInterfaces.yml"
gameFile = "unittest/testGame.yml"
missionFile = "unittest/testChallenge.yml"
def run_astro():
    #interfaces , stationName , gameFile , challengeFile 
    astro = astropoint.AstroPoint( testInterfaces , "IceStationZebra" , gameFile , missionFile )
    astro.run()

def timeMsg( secs ):
    msg = {"time": { "seconds":secs , "nanoseconds":0 },
               "payload":{ "seconds":secs , "nanoseconds":0 },
              }
            
    return json.dumps( msg )
class AstroTest( unittest.TestCase ):
    def advance_time( self , secs ):
        oTopic = "timelord.advance"
        msgTxt = timeMsg( secs )
        self.pub.send( oTopic , msgTxt )
    def basic( self ):
        proc = multiprocessing.Process( target=run_astro)
        proc.start() 
        self.pub = rf.RabbitPublish( config=testInterfaces )
        self.sub = rf.RabbitSubscriber( config=testInterfaces , blocking=True)
        self.sub.subscribe_to("soc.scientists.tx_bytes.science_station")
        self.sub.subscribe_to("science.status")
        self.sub.connect( queue_basename="test1")
        time.sleep( 10 )
    def test_3_missions( self ):
        self.basic()
        oTopic = "timelord.advance"
        
        msgTxt = timeMsg( 1001 )
        self.pub.send( oTopic , msgTxt )
        topic,msg = self.sub.recv() 
        assert topic=="soc.scientists.tx_bytes.science_station"

        msgTxt = timeMsg( 2001 )
        self.pub.send( oTopic , msgTxt )
        for k in range(0,2):
            topic,msg = self.sub.recv() 
            assert topic=="soc.scientists.tx_bytes.science_station"

        msgTxt = timeMsg( 3001 )
        self.pub.send( oTopic , msgTxt )
        for k in range(0,3):
            topic,msg = self.sub.recv() 
            assert topic=="soc.scientists.tx_bytes.science_station"

        # now expire the first one?
        msgTxt = timeMsg( 5201 )
        self.pub.send( oTopic , msgTxt )
        for k in range(0,3):
            topic,msg = self.sub.recv() 
            assert topic=="soc.scientists.tx_bytes.science_station"
    def test_turn_in(  self ):
        self.basic()
        self.advance_time() 
        # Get the science mission from the status
        topic, msg = self.sub.recv ()
    def test_bad_turn_in( self ):
        self.basic()
        self.advance_time() 
        # Get the science mission from the status
        topic, msg = self.sub.recv ()
    def test_double_turn_in() 
if __name__ == "__main__":
    print("testin'")
    t = AstroTest()
    t.test_3_missions()