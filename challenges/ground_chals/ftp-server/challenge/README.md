# has3-ftp-challenge

A custom ftp server that is more than minimally compliant with the protocol spec. There are certainly features that are not implemented but enough are that a standard ftp client can talk to this server. During the competition, this server can serve some of the flight software binaries that include challenges to be solved. Gaining access to the user account with these binaries will require exploiting the server in some way.

## Starting the server

Simply start the server binary as root so it can bind to port 21. After that, it will drop privs. The server uses the host OS for authentication, so only accounts that exist as users can login to the server. Anonymous logins are not supported.


## The Vulnerability

This server implements a nonstandard feature of queueing up commands and stores them in a linked list. Management of this list is broken and at a minumum use after free (UAF) and double free bugs exist and are exploitable. The most straightforward method of exploiting this server is to use the broken linked list bugs to overwrite the poorly implemented chroot feature which allows a logined user from escaping out of their own home directory.


