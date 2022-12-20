import argparse
import multiprocessing 
import subprocess
import time
import glob
def des_process( keyfile , port  ):
    
    cmd = ["./cserv", "-k", keyfile, "-d", "orig_des", "-p", str(port) ]
    print( f"calling {' '.join(cmd)}")
    subprocess.call( cmd  )
    print(f"CSERV died for port {port}  --- reopening")
    print("Exiting --- how did this happen")

def whitelisted_keys( keylocation ):

    # Look in the key directory
    pathPattern = f"{keylocation}/*.key"
    fileList = glob.glob( pathPattern )    
    out = []
    for file in fileList:
        out.append( file )
    return out

def run( args ):
    print("Challenge online" , flush=True )
    port = args.start_port
    procList = {}
    keepGoing = True 
    
    while True == keepGoing: 
        file_list = whitelisted_keys( args.keylocation) 
        for item in file_list:
            if item not in procList.keys():
                print( f"{item} on {port}", flush=True )
                proc = multiprocessing.Process( target=des_process, args=(item , port ) ) 
                proc.start()
                port = port + 1 
                procList[item] = proc 
        time.sleep(30)
    for item in procList:
        item.join()
    print("Callenge Exiting", flush=True)
if __name__ == "__main__":
    a = argparse.ArgumentParser()
    a.add_argument("--start_port" , type=int)
    a.add_argument("--keylocation")
    args = a.parse_args()
    run( args  )