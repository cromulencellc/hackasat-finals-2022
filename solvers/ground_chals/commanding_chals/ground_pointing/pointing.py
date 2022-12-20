import skyfield.api as sf 
import skyfield.positionlib as sfp
import numpy as np 
import pyorbital.orbital as po
class Pointer:
    def __init__( self,  latDegrees, lonDegrees, altitude ):
        self.gs = sf.wgs84.latlon( latDegrees, lonDegrees, altitude )
        self.ts = sf.load.timescale()
    def getPointing( self , timeObj , posIcrf):
        t = self.ts.from_datetime( timeObj)
        dAu = np.array( [posIcrf[0],posIcrf[1],posIcrf[2]] )/ 149597870700
        pos = sfp.ICRF( dAu , [0,0,0] , t , 399)
        geoPos = sf.wgs84.geographic_position_of( pos )


        satLon = geoPos.longitude.degrees
        satLat = geoPos.latitude.degrees
        satAlt = geoPos.elevation.km
        lat  = self.gs.latitude.degrees
        lon = self.gs.longitude.degrees
        alt = self.gs.elevation.km
        print(f"Satellite is over: {satLon} {satLat}")
        az,el = po.get_observer_look(satLon, satLat, satAlt, timeObj, lon, lat, alt)
        if( el < 0 ):
            el = 0
        return (az,el)
