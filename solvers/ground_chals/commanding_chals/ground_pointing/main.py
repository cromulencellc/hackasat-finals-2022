
import pointing
import time 
import datetime
import pytz
import yaml
import orbital
import argparse
# ORDER MATTERS HERE
import os
os.environ["COSMOS_VERSION"] = "1.1.1"
#os.environ["COSMOS_API_USER"] = "has"
os.environ["COSMOS_API_PASSWORD"] = "has"
os.environ["COSMOS_LOG_LEVEL"] = "DEBUG"
os.environ["COSMOS_API_SCHEMA"] = "http"
os.environ["COSMOS_API_HOSTNAME"] = "cosmos-cmd-tlm-api"
os.environ["COSMOS_API_PORT"] = "2901"
from cosmosc2 import * 
### END ORDER MATTERS

def contact():
    gs = 'sattelitename_svalbard'
    sat = 'satellitename_sat'
    return gs, sat

def loadWorld(path):
    f = open( path , 'rt')
    txt = f.read()
    f.close()
    world = yaml.safe_load( txt )
    return world

def unpack_tlm( inArray ):
    out = {}
    for item in inArray:
        out[item[0]] = item[1]
    return out

import numpy as np
def run(world, gs, sat):
    lat = world['groundstations'][gs]['latitude']
    lon = world['groundstations'][gs]['longitude']
    alt = world['groundstations'][gs]['altitude']
    epochStr = world['game']['start_time']
    t_format = "%Y-%m-%d %H:%M:%S.%f"
    epoch= datetime.datetime.strptime( epochStr , t_format )
    epoch = epoch.replace( tzinfo=pytz.UTC )

    a       = world['satellites'][sat]['a']
    e       = world['satellites'][sat]['e']
    i       = world['satellites'][sat]['i']
    raan    = world['satellites'][sat]['raan']
    omega   = world['satellites'][sat]['omega']
    TA      = world['satellites'][sat]['TA']
    mu = 3.986004418e14




    point = pointing.Pointer( lat, lon , alt)
    keep_going =  True
    id = 0
    
    n = np.sqrt( mu / (a*a*a))
    cosmosTime = epoch
    M0 = orbital.mean_anomaly_from_true( e , np.deg2rad(TA))

    #t = datetime.datetime.strptime( epoch , t_format  )
    #t = tDT.replace( tzinfo=pytz.UTC )

    while( keep_going ):
        id+=1
        # Get time
        tlmArr = get_tlm_packet("GS", "GAME_TIME")
        timeMsg = unpack_tlm( tlmArr )
        cosmostimeStr = timeMsg["GAME_TIME"]
        cosmosTime = datetime.datetime.strptime( cosmostimeStr, t_format )
        cosmosTime = cosmosTime.replace( tzinfo=pytz.utc )

        deltaTime = cosmosTime - epoch
        
        # Calculate time 
        M = n * deltaTime.total_seconds()
        

        orbit = orbital.elements.KeplerianElements( a=a, e=e, i=np.deg2rad(i), raan=np.deg2rad(raan) , arg_pe=np.deg2rad(omega), M0=M0, body=orbital.bodies.earth , ref_epoch=epoch )
        #
        #print(f"Mean anomaly is {M}")
        orbit.propagate_anomaly_by(M=M)
        pos = orbit.r
        #print(f"position is {pos}")
        tStr = cosmosTime.strftime("%Y-%m-%d %H:%M:%S")
        az,el = point.getPointing( cosmosTime , pos )
        print(f"T: {tStr} Az: {az} El: {el}")
        # Format cosmos api command
        cmd(None,f"GS SELECT_STATION with OPCODE 1, STATION_NAME {gs}")
        cmd(None,f"GS STEER_ANTENNA with OPCODE 3, AZIMUTH {az}, ELEVATION {el}")
        # things arent that fast - no need to go crazy 
        time.sleep( 2 )

if __name__=='__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("gs")
    parser.add_argument("sat")
    args = parser.parse_args()
    #gs, sat = contact()
    world = loadWorld('has3world.yml')
    run(world, args.gs, args.sat)