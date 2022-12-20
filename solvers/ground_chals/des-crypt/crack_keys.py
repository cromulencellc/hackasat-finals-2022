from ast import arg
from pwn import *
import socket
import subprocess
import argparse
def connect_get_cipher_patched( hostname , portNum ):
    
    s = remote( hostname,portNum)
    
    o = s.recv(64)
    data = b'\x00' * 8

    ## patch it
    s.send(str(0x1ae6).encode('utf-8') + b'\n')
    s.send(b'\x00')

    s.send( b'8\n')
    s.send(data)

    cipher = s.recv(8)

    s.close()

    return cipher

def list_all_possible_keys( cracked ):
    cracked = cracked.encode()
    for k in range( 0,255):
        bitStr = format( k , '08b')
        intValue  = k.to_bytes(1,"big")
        bitlist = [int(digit) for digit in bitStr ]
        newKey = b""
        for z in range(0,8):
            newKey += (cracked[z] ^ bitlist[z]).to_bytes(1,'big')
        print( newKey.decode() )
def connect_get_cipher( hostname, portnum):
    s = remote( hostname,portnum)
    o = s.recv(64)

    ## bypass patch
    s.send(str(-1).encode('utf-8') + b'\n')

    data = b'msbrown3'
    s.send( str(len(data)).encode('utf-8') + b'\n')
    s.send(data)

    back_len = len(data)

    if back_len % 8:
        back_len = back_len + (8 - (back_len %8))

    print("Length: %d" %back_len)

    cipher = s.recv(back_len)

    s.close()

    return cipher


def crack( hostname,  portNum ):
    null_cipher = connect_get_cipher_patched( hostname, portNum )

    f = open('nullcipher', 'wb')
    f.write(null_cipher)
    f.close()

    input('---:')

    f = open('plain', 'wb')
    f.write(b'msbrown3')
    f.close()

    goodcipher = connect_get_cipher(hostname, portNum )
    f = open('goodcipher', 'wb')
    f.write(goodcipher)
    f.close()

    subprocess.run(["./crack-des-key", "-k", "keyfile", "-p", "plain", "-c", "goodcipher", "-n", "nullcipher"])

    f = open("keyfile",'r')
    cracked = f.read()
    f.close()

    list_all_possible_keys( cracked )
if __name__ == "__main__":
    a = argparse.ArgumentParser( )
    #a.add_argument( "--hostname", required=True)
    #a.add_argument( "--port", type=int, required=True)
    a.add_argument( "--hostname", default="localhost")
    a.add_argument( "--port", type=int, default=13000)
    args = a.parse_args()
    crack(args.hostname, args.port)
