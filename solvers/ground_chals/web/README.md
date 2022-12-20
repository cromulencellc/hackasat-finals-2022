# Web Challenge

## The Vulnerability

The main vulnerability in this server is that it calls system() to create directory listings when a user requests an existing folder that does not have an index.html file, and is not access controlled. Many of the commonly used delimiter chars used in command injects are filtered, but not all of them... 

The server is also vulnerable to directory traversals, if the '..' is urlencoded as '%2E%2E'. Using this bug and the directory listing feature, attackers can find and download the server's binary for reverse engineering to find the path to reach the call to system().
