#include <stdio.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>

#include "ftp.h"

// Server socket descriptor
int sockfd_g;

// Handler to catch interrupts and properly close the socket
void handler( int sig )
{
	close(sockfd_g);

	exit(0);
}

// Initialize the listener socket. This calls listen() but it doesn't
//	begin the accept loop.
int setup_socket( int port )
{
	int sockfd;
	int len;
	struct sockaddr_in sin;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if ( sockfd < 0 ) {
		fprintf(stderr, "[-] socket error: %s\n", strerror(errno));
		return sockfd;
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
		fprintf(stderr, "[-] setsockopt error: %s\n", strerror(errno));
		close(sockfd);
		return -1;
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int)) < 0) {
		fprintf(stderr, "[-] setsockopt error: %s\n", strerror(errno));
		close(sockfd);
		return -1;
	}

	memset(&sin, 0, sizeof(struct sockaddr_in) );

	sin.sin_family = AF_INET;
	sin.sin_port = htons( port );
	sin.sin_addr.s_addr = inet_addr("0.0.0.0");

	len = sizeof(struct sockaddr_in);

	if ( bind(sockfd, (struct sockaddr *)&sin, len) < 0 ) {
		fprintf(stderr, "[-] bind error: %s\n", strerror(errno));
		close(sockfd);
		return -1;
	}

	if ( listen(sockfd, 30) < 0 ) {
		fprintf(stderr, "[-] listen error: %s\n", strerror(errno));
		close(sockfd);
		return -1;
	}

	printf("[+] Listening on: %d\n", port);

	return sockfd;
}


int main(int argc, char **argv)
{
	int opt, pid, clientfd;
	socklen_t len;
	struct sockaddr_in client_in;

	// Port on which to listen
	int port = 21;

	while((opt = getopt(argc, argv, "p:")) != -1) {
		switch(opt) {
            case 'p': 
                port = strtol(optarg, NULL, 10);

                break;
            case ':': 
                printf("option needs a value\n"); 
                break; 
            case '?': 
                printf("unknown option: %c\n", optopt);
                break; 
        } 
    }

    // Start the listening server
    sockfd_g = setup_socket( port );

    if ( sockfd_g < 0 ) {
    	return -1;
    }

    // Since we made it this far, setup the SIGINT handler to properly close the socket
    signal(SIGINT, handler);

    len = sizeof(struct sockaddr_in);

    // Reap the children
    signal(SIGCHLD, SIG_IGN);
 
    // Main accept loop
    while ( 1 ) {
		
    	clientfd = accept(sockfd_g, (struct sockaddr *)&client_in, &len );

    	pid = fork();

    	if ( pid == -1 ) {
    		fprintf(stderr, "[-] fork error: %s\n", strerror(errno));
    		close(sockfd_g);
    		return -1;
    	}

    	if ( pid ) {
    		printf("[+] Client %s connected.\n", inet_ntoa(client_in.sin_addr));
    		close(clientfd);
    		continue;
    	} else {
    		close(sockfd_g);
    		client_handler(clientfd, &client_in);
    	}
    }

    return 0;
}