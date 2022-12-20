from unittest import TestCase
import sys
sys.path.append('.')
import mission
import  numpy as np  
class MissionTest( TestCase ):
    def __init__(self):
        c = mission.MissionCfg()
        c.pointAt = "jupiter" 
        c.epoch = "2022-01-01 12:00:00"
        c.missionStartSeconds = 0
        c.missionStopSeconds  = 300
        c.durationSeconds     = 60 
        c.accuracyDegrees     = 10
        c.sensorAxisBody      = np.array([0,0,1])  
        self.mission = mission.Mission(c) 
    def test_off_axis(self):
        timeSec = 10 
        pos = np.array([6378*10,0,0])
        quaternion = np.array( [0,0,0,1] )
        out = self.mission.evaluate(timeSec, pos ,  quaternion  )
        assert out==False,"Shouldnt be pointing anywhere good"
    def test_on_axis( self ):
        timeSec = 10 
        pos = np.array([6378*10,0,0])

        quaternion =np.array( [0,0,0,1])
        out = self.mission.evaluate(timeSec, pos , quaternion  )
        remaining=  self.mission.getRemaining()
        assert (remaining) == (self.duration-10), "Remaining should tick down"
    def test_duration_but_not_window( self):
        pass
    def test_on_axis_complete(self):
        timeSec = 10 
        pos = np.array([0,0,0])
        quaternion = np.array([0,0,0,1])
        out = self.mission.evaluate(timeSec, pos, quaternion  )
        remaining=  self.mission.getRemaining()
        assert (remaining) == (self.duration), "Remaining should not tick down"
        timeSec = 20
        pos = np.array([0,0,0])
        quaternion = np.array([0,0,0,1])
        out = self.mission.evaluate(timeSec, pos, quaternion  )
        remaining=  self.mission.getRemaining()
        assert (remaining) == (self.duration-10), "Remaining should tick down"
        timeSec = 120
        pos = [0,0,0]
        quaternion = [0,0,0,1]
        out = self.mission.evaluate(timeSec, pos, quaternion  )
        assert out==True, "Should be done"
if __name__ == "__main__":
    t = MissionTest()
    t.test_off_axis()
    t.test_on_axis()
    t.test_on_axis_complete()