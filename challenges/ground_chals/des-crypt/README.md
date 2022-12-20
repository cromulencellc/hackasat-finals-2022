## Overview

This program allows introducing a glitch into DES that makes is possible to determine the key.

## Building

Run `make` in the main directory. This will output two applications: `cserv` and `orig_des`

## cserv

This is the portion of the challenge that listens on the network. It needs to be provided with the location of the keyfile,
the location of the program responsible for DES encryption, and optionally a port. If no port is specificied then it
defaults to 8498.

### Running cserv

`cserv -k <keyfile location> -d <des prog location> -p <port>`

### Overview


Subject to change after I figure out a good way to do the overwrite. 

The application begins by loading the DES program into memory. The cserv application listens on the network for a connection on some port. 
When a new user connects the application forks. It then expects a decimal, ascii integer value with a maximum length of 10 bytes for the offset 
to write to. If this value is between 0 and the size of the DES program then it will read a single byte from the socket. The received byte is
written to the specified offset within the saved DES program. If the offset is outside this range then it will skip reading the extra byte.

It then reads a size field which must be a decimal, ascii integer value with a maximum length of 10 bytes. If this is greater than 0 and less 
than 256 bytes then cserv will read the specified number of bytes from the wire. This data is then written to a temp file along with the
DES program which may or may not have been modified. The application then launches the DES program to encrypt the received bytes. Finally,
the encrypted bytes are returned to the client and the socket is closed.

### Exploitation

I start out by getting a good, known plaintext and known ciphertext pair. I use this to confirm that the key that I found is legitimate. 
My patch at offset 0x1ae6 which is at “des_encypt:328” at the beginning of the loop that performs the DES rounds and breaks after only a single round has been performed. If this is done with a plaintext consisting of 8 NULL bytes then the encrypted block received contains data entirely based on the key material.

With the ciphertext encrypted with the glitch and the 8 NULL bytes, the first step is to undo the final permutation which is straight-forward since you can just redo the initial permutation. This gives you the result of the “des substitution box” function. The problem with this is that the S-boxes result in a many-to-one translation. For each 4-bit S-box there are 4 6-bit input values that could have resulted in a given output value. With 8 S- boxes there are a total of 4^8 = 65536 possible combinations that could have resulted in the S-box that I have at this point. 

Prior to going further down the brute force path, I had to determine which bits affected which S-boxes in the first round of DES. This was a pain-staking process that I probably should have automated. Here is what I found: 
2 

|S-Box | Bits            |
|:-----|:----------------| 
|0     |14 23 38 53 55 60| 
|1     | 6 15 21 39 46 63|
|2     | 5 30 31 37 44 62| 
|3     | 7 22 29 36 47 61| 
|4     | 4 18 28 33 35 50| 
|5     | 3 17 26 27 41 51| 
|6     | 9 19 20 34 57 59| 
|7     |10 11 25 43 49 58|
|None  | 0  1  2  8 12 13 16 24 32 40 42 45 48 52 54 56| 
 
There are some absences, namely the parity bits as well as the 8 bits lost when DES performed the compression permutation. To handle this, I had to brute force an additional 8 bits which brings the total number of possible keys up to 4^8 * 2^8 = 16777216. This still isn’t a large number and can be completed in less than 6 minutes in a Virtual Machine on my laptop. 
The brute force is just a nasty set of 9 nested for loops. Each of the 4 possible bytes for each of the 8 S-boxes are tested along with each possible combination of the remaining 8 bits. To confirm that a key is found I encrypt the known plaintext using the key that I am testing and compare it to the known ciphertext. If these two are equal then I have found the key. 
The final bit of cleanup is to correctly set the parity bits so that when a byte-wise comparison of the original key and the leaked key is performed there isn’t a false negative due to differing bits that don’t affect encryption. 

