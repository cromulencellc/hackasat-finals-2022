from curses import window
import random
import mission
import yaml
import skyfield.api as sf 

class MissionGen:
    def __init__(self, epoch , challengeYml  ):
        # load yaml file that has all this info
        f = open(challengeYml,'r')
        txt = f.read()
        settings = yaml.safe_load( txt )["settings"]
        # 
        #self.planets = ["mercury","venus","mars","jupiter","saturn","neptune","uranus","pluto"]
        self.planets = settings["planets"]
        self.minDuration = settings['minDuration']
        self.maxDuration = settings['maxDuration']
        self.epoch  = epoch
        self.windowDelay = settings['windowDelay']
        self.windowLength = settings['windowLength']
        self.accuracyDegrees = settings["accuracyDegrees"]
        self.axis = settings["axis"]
        self.missionCounter = 0 
        if self.windowLength < 1.1 * settings['maxDuration']:
            print("Windows should be at least 1.5x max duration")
            raise ValueError
        self.generatePeriod = settings["generatePeriod"]
        self.ephemeris = sf.load( "de421.bsp" )
        self.lastGeneration = -(self.generatePeriod+100)

    def time_update( self , currentTime ):
        timeSinceLast = currentTime - self.lastGeneration
        out = []
        if timeSinceLast >= self.generatePeriod:
            o = self.generate(currentTime)
            out.append(o)
        return out             

    def generate( self , currentTime ):

        # generate planet
        planet = random.choice( self.planets )
        # generate duration
        duration = random.randrange( self.minDuration , self.maxDuration)
        # generate window length
        c = mission.MissionCfg()
        c.pointAt = planet
        c.epoch = self.epoch
        c.missionStartSeconds = currentTime + self.windowDelay
        c.missionStopSeconds  = c.missionStartSeconds + self.windowLength
        c.durationSeconds     = duration 
        c.accuracyDegrees     = self.accuracyDegrees 
        c.sensorAxisBody      = self.axis
        c.count = self.missionCounter
        self.lastGeneration = currentTime
        o = mission.Mission( c )
        self.missionCounter += 1 
        
        return c