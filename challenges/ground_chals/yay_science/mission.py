import skyfield.api as sf 
import skyfield.framelib as frames
import numpy as np
import pytz
from scipy.spatial.transform import Rotation as R
import datetime 
import secrets 
import struct

ephemeris = sf.load( "de421.bsp")

class MissionCfg:
    pointAt = None 
    epoch = None
    missionStartSeconds = None
    missionStopSeconds  = None
    durationSeconds     = None 
    accuracyDegrees     = None 
    sensorAxisBody      = None 
    count = None 
    def valid( self ):
        if self.pointAt == None or \
           self.epoch   == None or  \
           self.missionStartSeconds == None or  \
           self.missionStopSeconds == None or  \
           self.durationSeconds == None or \
           self.accuracyDegrees == None or \
           self.count == None or \
           self.sensorAxisBody is None:
           raise NotImplemented
        
class Mission:
    def __init__(self, config  ):
        self.tGo = 100000
        config.valid()
        self.startSeconds = config.missionStartSeconds
        self.endSeconds = config.missionStopSeconds
        self.pointAt = config.pointAt 
        self.duration = config.durationSeconds
        self.accuracyDegrees = config.accuracyDegrees
        self.lastSeconds = 0.0
        self.secondsToGO = config.durationSeconds
        self.sensorAxis = config.sensorAxisBody 
        self.missionId = config.count 
        self.epoch = datetime.datetime.strptime( config.epoch , "%Y-%m-%d %H:%M:%S.%f")
        planetName = self.pointAt + " barycenter"
        ephemeris =sf.load( "de421.bsp" )
        self.earth = ephemeris["earth"]
        self.target = ephemeris[planetName]
        timeFormat = "%Y-%m-%d %H:%M:%S" 
        startDateTime = self.epoch + datetime.timedelta( seconds= self.startSeconds )
        endDateTime = self.epoch + datetime.timedelta( seconds= self.endSeconds )

        endTimeStr = endDateTime.strftime( timeFormat ).encode()
        startTimeStr = startDateTime.strftime( timeFormat ).encode()

        packMe=  [self.missionId , int(self.duration), planetName.encode(), startTimeStr , endTimeStr ]
        self.packed = struct.pack("HH25s20s20s", *packMe )
        self.ts = sf.load.timescale()


    def getRemaining( self ):
        return self.timeToGo
    def getPackedMission( self ):
        return bytes(self.packed )

    def getId(self):
        return self.missionId
    def getTgo( self ):
        return self.tGo 
    def inWindow( self , currentTime ):
        return ( currentTime > self.startSeconds) and (currentTime < self.endSeconds )
    def evaluate( self, currentTime, position , quaternion ):
        # Check if we are within the mission windows
        timeInWindow = (currentTime >= self.startSeconds ) and \
                       (currentTime <= self.endSeconds   )
        
        # Check if we are pointing at the planet with sufficient accuracy
        rot = R.from_quat( quaternion )
        DCM_body_to_inertial = rot.as_matrix( )
        currentDateTime = self.epoch + datetime.timedelta( seconds=currentTime)
        currentDateTime = currentDateTime.replace(tzinfo=pytz.UTC)
        T = self.ts.from_datetime( currentDateTime )

        posTarget = self.earth.at(T).observe(self.target).position.m

        losEci = -position  + posTarget 
        sensorAxisEci  = np.matmul( DCM_body_to_inertial , self.sensorAxis ) 
        losMag = np.linalg.norm( losEci )
        axisMag = np.linalg.norm( sensorAxisEci )
        D = np.dot( sensorAxisEci , losEci ) /( axisMag * losMag )
        D = np.clip( D , -1.0, 1.0)
        offAxis = np.arccos( D )*180.0/np.pi

        pointingAccurate = np.abs( offAxis ) < self.accuracyDegrees
        
        # Check if we are notEclipsed 
        RS = np.linalg.norm( position ) # Radial distance 
        
        RE = 6378000.0 # major radius of earth m
        ratio = RE / RS
        ratio = np.clip( ratio, -1 , 1 )
        eclipsingAngle = np.arcsin( ratio ) # all angles less than this are eclipsed
        losRatio = np.dot( losEci  , -position ) / ( RS  * losMag )
        losRatio = np.clip(losRatio,-1.0 , 1.0)
        losAngle = np.arccos( losRatio ) 
        eclipsed = losAngle < eclipsingAngle 
        
        if pointingAccurate and timeInWindow and (not eclipsed):
            timeObserved = currentTime - self.lastBadTime 
            self.tGo = self.duration - timeObserved
            if self.tGo < 0:
                self.lastBadTime = currentTime # reset
                return True
        else:
            self.lastBadTime = currentTime
            self.tGo = self.duration
        return False