import socket
import subprocess
portNum = 8001

def connect_get_cipher_patched():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(("localhost", portNum))

    data = b'\x00' * 8

    ## patch it
    s.sendall(str(0x1ae6).encode('utf-8') + b'\n')
    s.sendall(b'\x00')

    s.sendall( b'8\n')
    s.sendall(data)

    cipher = s.recv(8)

    s.close()

    return cipher

def connect_get_cipher():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(("localhost", portNum))

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

null_cipher = connect_get_cipher_patched()

f = open('nullcipher', 'wb')
f.write(null_cipher)
f.close()

input('---:')

f = open('plain', 'wb')
f.write(b'msbrown3')
f.close()

goodcipher = connect_get_cipher()
f = open('goodcipher', 'wb')
f.write(goodcipher)
f.close()

subprocess.run(["./crack-des-key", "-k", "keyfile", "-p", "plain", "-c", "goodcipher", "-n", "nullcipher"])
