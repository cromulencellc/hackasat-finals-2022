
from pydoc import plain
from Crypto import Random
from Crypto.Cipher import AES
import hmac
import hashlib
import base64
import json


class AuthError(Exception):
    pass
class PaddingError(Exception ):
    pass
def unpad( s ):
    last_pad = s[-1]
    #print(s)
    if (last_pad < 1) or ( last_pad>16):
        raise PaddingError
    for k in range( 1,  last_pad+1 ):
        pad = s[-k]
        if last_pad != pad: 
            raise PaddingError
    return s[:-last_pad]
def pad( s ):
    bs = AES.block_size
    padding_bytes = bs -  (len(s)%bs)
    if padding_bytes == 0: 
        padding_bytes = bs 
    pad = padding_bytes*chr( padding_bytes )
    out = s + pad.encode() 
    #print(f"padded {out}")
    return out 

class Cipher:
    def __init__(self , secret):
        self.secret = secret
        self.aesKey =hashlib.sha256(secret.encode()).digest()
        

    def encrypt( self, data ):
        raw = pad(data)
        iv = Random.new().read(AES.block_size)
        cipher = AES.new(self.aesKey, AES.MODE_CBC, iv)
        return iv + cipher.encrypt(raw)
    def decrypt(self, enc ):
        
        iv= enc[:AES.block_size]
        cipher =AES.new( self.aesKey , AES.MODE_CBC , iv )
        padded = cipher.decrypt( enc[AES.block_size:])
        plaintext = unpad( padded )

        return plaintext
