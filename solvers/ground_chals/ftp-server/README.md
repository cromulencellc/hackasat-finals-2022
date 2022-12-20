# FTP-Server

## The Vulnerability

This server implements a nonstandard feature of queueing up commands and stores them in a linked list. Management of this list is broken and at a minumum use after free (UAF) and double free bugs exist and are exploitable. The most straightforward method of exploiting this server is to use the broken linked list bugs to overwrite the poorly implemented chroot feature which allows a logined user from escaping out of their own home directory.
