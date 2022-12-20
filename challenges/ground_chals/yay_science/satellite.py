import mission 
import copy
class Satellite:
    def __init__( self , name ):
        self.name = name

        self.pos = [0,0,0]
        self.quaternion = [0,0,0,1]
        self.data_generated = []
        self.complete = []
        self.missions = []
        self.name = name
    def add_mission( self , mission):
        m = copy.deepcopy( mission )
        self.missions.append( m )
    def state( self , position, quaternion ):
        self.pos = position
        self.quaternion = quaternion
    def time( self , timeSecs ):
        for m in self.missions:
            makeData = m.evaluate(timeSecs ,  self.pos , self.quaternion )
            