
from binascii import hexlify
from Crypto.Cipher import AES

class RemotePaddingError( Exception ):
    pass

def modify( original  , byte_number, xor_by ):
    N = byte_number - AES.block_size
    B = original[N]
    
    NewB = B ^ xor_by
    modified = bytearray( original ) 
    modified[N] = NewB
    return bytes( modified )

class PaddingCracker:
    def __init__( self,pwnRemote ):
        self.r = pwnRemote


    def test_decrypt(self, cipher ):
        self.r.recvuntil(b"Choice?" )
        self.r.sendline(b"2")
        self.r.recvuntil(b"key:")
        hexlifyCipher = hexlify( cipher )
        self.r.sendline( hexlifyCipher )
        response = self.r.recvuntil(b"Options\n",drop=True)
        if b"PADDING" in response:
            raise RemotePaddingError
        elif b"bad data" in response:
            raise ValueError

    def decrypt_byte( self, encrypted , byte_number  , pad_byte ):
        truth = pad_byte
        Cnt = 0 
        for k in range(1,256):
            #rint(f"Trying {0}")
            
            modified = modify( encrypted , byte_number , k)
            try:
                self.test_decrypt( modified )
                #print(f"Attempt {hex(k)} was decrypted")
                #print(  out )
                
                truth = pad_byte^k
                #print( f"XOR {k}  to get truth {truth}")
                Cnt+=1
                return truth           
            except RemotePaddingError as e:
                pass
        self.test_decrypt( encrypted )
        return truth 
    def decrypt_last_block(self, encrypted  ):
        decrypted = b""
        last2Enc = encrypted[-32:]
        truth = []
        truth_pad_value = None
        for k in range( 0 , 16):
            # modify all previous bytes
            attackByte = (32-k-1)
            pad_value = (k+1)
            newEncrypted = last2Enc
            # ALWAYS FAIL ON SECOND PAD BYTE
            for z in range(1,k+1):
                M = attackByte + z 
                X = pad_value ^ truth[-z]
                #print(f"M: {M} X:{X} T:{truth[-z]}")
                newEncrypted = modify( newEncrypted ,M , X)
            newByte = self.decrypt_byte( newEncrypted , attackByte , pad_value )
            truth.append( newByte )
            decrypted = (newByte.to_bytes(1,"big")) + decrypted
            print( decrypted )
        return decrypted