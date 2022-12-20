from binascii import unhexlify
from pwn import *

import padcrack
import sys
import argparse

#context.log_level = "debug"

def main( hostname , port ):
    findMe = input("What groundstation key are you trying to get?").strip()

    r = remote( hostname , port )

    # Get the keys 
    r.recvuntil(b"Choice?")
    r.sendline(b"1")
    
    r.recvuntil(b"--------")
    keys = r.recvuntil(b"\n\nOptions\n", drop=True)
    keysNoTag = keys.replace(b"Key: ",b"")
    keysHex = keysNoTag.split(b"\n")[1:]
    keyToBreak = None
    for key in keysHex:
         r.recvuntil(b"Choice?")
         r.sendline(b"2")
         r.recvuntil(b"key:")
         r.sendline( key )
         r.recvuntil(b"for ")
         gsName = r.recvline( ).strip()
         if gsName.decode() == findMe:
            keyToBreak = key 
            print(f"{gsName} encrypted key is {key}")
            break
    if keyToBreak == None:
        print("Umm I didnt find it")
        sys.exit(-1)
    c = padcrack.PaddingCracker( r )
    binaryKey = unhexlify( keyToBreak )
    decryptMe = binaryKey
    decrypted = b''
    while len( decryptMe ) > 16:
        out = c.decrypt_last_block( decryptMe )
        decrypted = out + decrypted
        decryptMe = decryptMe[:-16]



    print("AES256 is poorly implemented and I pwned it...here is your key")
    print(decrypted)




if __name__ == "__main__":
    #a = argparse.ArgumentParser()
    #a.add_argument("--port",type=int)
    #a.add_argument("--hostname")
    #args =a.parse_args()
    hostname='localhost'    # Scrubbed. Should be set to server IP address
    port='13100'
    main( hostname , port )