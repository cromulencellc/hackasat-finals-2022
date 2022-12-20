import skyfield.api as sf
import skyfield.framelib as frames
import pytz
import numpy as np
import datetime
import argparse
from scipy.spatial.transform import Rotation as R

planets  = ["sun","mercury","venus", "earth","mars","jupiter","saturn","neptune", "uranus" , "pluto"]


time_format = "%Y-%m-%d %H:%M:%S"
ephemeris =sf.load( "de421.bsp" )

ts = sf.load.timescale()


def planet_position( currentDateTime ,planet ):
    T = ts.from_datetime( currentDateTime )
    if planet != "sun":
        planetObj =  ephemeris[planet + " barycenter" ]
    else:
        planetObj = ephemeris[planet]
    earth = ephemeris["earth"]
    O = earth.at(T).observe(planetObj).position.m
    return O

def z_axis_quat( pos  ):
    z = [0,0,1]
    
    p_unit = pos / np.linalg.norm( pos )
    #print( p_unit )
    # form an ortonormal basis 
    n = np.cross( z , p_unit )
    n = n / np.linalg.norm( n )
    o = np.cross( p_unit , n )
    dcm = np.matrix( [  n , o , p_unit  ]).transpose()
    #dcm = np.matrix( [ p_unit, n , o   ]).transpose()
    rot = R.from_matrix( dcm )
    q = rot.as_quat( )
    return q
def run( time ):
    dt = datetime.datetime.strptime( time ,time_format )
    dt = dt.replace(tzinfo=pytz.utc)
    for planet in planets: 
        pos = planet_position( dt ,planet)
        q = z_axis_quat( pos )
        qStr=[str(i) for i in q]
        print( f"planets['{planet}']=[{', '.join(qStr)}]")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--time", default="2023-01-01 00:00:00")#required=True)
    args = parser.parse_args()
    run( args.time )