# has3-web-challenge

A custom webserver with authenticated folder access, directory listings, and a bunch of legit webserver features for your reverse engineering pleasure. For the competition, this webserver was used to legitimately serve up content that explained how the Science Missions work. It also contained an access controlled folder where ground station setting would be leaked once a method of bypassing the access control feature is found. 

## Starting the server

Simply start the server binary as root so it can bind to port 80. After that, it will drop privs to UID/GID 1000. The server.ini file should be in the working directory of the server. All other settings are configured in this file.


## The Vulnerability

The main vulnerability in this server is that it calls system() to create directory listings when a user requests an existing folder that does not have an index.html file, and is not access controlled. Many of the commonly used delimiter chars used in command injects are filtered, but not all of them... 

The server is also vulnerable to directory traversals, if the '..' is urlencoded as '%2E%2E'. Using this bug and the directory listing feature, attackers can find and download the server's binary for reverse engineering to find the path to reach the call to system().


