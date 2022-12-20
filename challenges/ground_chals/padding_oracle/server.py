import asyncio, socket
import binascii
import argparse
import cipher 
import glob, os
from binascii import hexlify,unhexlify
C = None
def generate_key_dictionary(  ):
    fileSearch = f"{args.key_files}/*.key"
    D = {} 
    for file in glob.glob(fileSearch):
        with open( file ) as f:
            fileTxt = f.read()
            gsName = file.split('/')[-1].replace(".key" , "")
            D[fileTxt] = gsName
    return D
def print_keys( WL ):
    o = b"Hexlified keys below are encrypted with AES256"
    o += b"\n--------\n"
    for key in WL.keys( ):
        encrypted = C.encrypt( key.encode() )
        o+=b"Key: "
        o += hexlify(encrypted)
        o+=b"\n"
    o+=b"\n"
    return o 
def which_key_is_this( input , WL ):
    try: 
        inputRaw = unhexlify( input ) 
        decrypted = C.decrypt( inputRaw )
        
        # open all the key files
        out = 'Not found'
        for key,item in WL.items():
            if key == decrypted.decode():
                out = f"This is the key for {item}"
                return out + "\n" 
           
    except cipher.PaddingError:
        out = "Encryption Invalid: PADDING"
    except:
        out = "Error: bad data"
    return out + "\n" 

async def handle_client(reader, writer):
    keep_going = True 
    while True == keep_going:
        WL = generate_key_dictionary()

        menu = "Options\n"
        menu+= "-------\n"
        menu+= "1) List keys\n"
        menu+= "2) What key is this?\n"
        menu+= "3) Quit\n"
        menu+= "Choice?"
        writer.write( menu.encode() )
        await writer.drain()
        choice = (await reader.read(255)).decode('utf8')
        choice = choice.replace("\n","")
        if choice == "1":
            out = print_keys(WL )
            writer.write( out )
        elif choice == "2":
            writer.write("Enter hexlified encrypted key:".encode())
            await writer.drain() 
            keyIn = (await reader.read(255)).decode('utf8')
            out = which_key_is_this( keyIn.strip('\n') , WL )
            writer.write( out.encode() )
        elif choice == "3":
            keep_going = False
        else:
            writer.write("Invalid choice\n".encode())
        await writer.drain() 
    writer.close()

async def run_server(args):
    server = await asyncio.start_server(handle_client, args.hostname , args.port)
    async with server:
        await server.serve_forever()

if __name__ == "__main__":
    a = argparse.ArgumentParser()
    a.add_argument('--port' , default=10002)
    a.add_argument('--hostname', default="localhost")
    a.add_argument('--key_files', default='/home/mike/Finals/padding_oracle/test_keys')
    a.add_argument('--aes_secret', default="whateva")
    args = a.parse_args()
    

    # Load the cipher
    C = cipher.Cipher( args.aes_secret )
    asyncio.run(run_server(args))