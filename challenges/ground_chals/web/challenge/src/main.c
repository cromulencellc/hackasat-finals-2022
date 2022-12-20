#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <time.h>
// #include <pthread.h>
#include <errno.h>
#include <signal.h>
#include "dictionary.h"
#include "http.h"
#include "config.h"
#include "bst.h"
#include "debug.h"
#include <syslog.h>
#include <poll.h>

// 20 bytes to account for Method, LWS, and HTTP version
#define BUFFER_SIZE (MAX_URI + MAX_QUERY + 200)  

int init_404Error(char *contentFile404);
dictionaryType *loadINI(char *filename);
dictionaryType *loadMIMEtypes(char *filename);
int loadConfig( serverConfigType *serverConfigData, dictionaryType *configDict);

dictionaryType *mimeDict;
bstType *hoststats;
bstType *ua_stats;

typedef struct {

	int s;
	serverConfigType *serverInfo;
	struct sockaddr_in peeraddr;

} connection_info_type;


int readLine( int fd, char *buffer, int maxLen ) {

int writePos;

	writePos = 0;
	int count;

	if ( buffer == 0 ) 
		return -1;

	buffer[0] = 0;

	while( writePos < maxLen ){

		count = read(fd, buffer+writePos, 1);

		if (count == 0) {

			// debug("No more data to read\n");
			return 0;
		}

		if (buffer[writePos] == '\n')
			break;

		++writePos;

	} 

	// we hit the end of the buffer w/o finding a newline, bad line read
	if ( writePos == maxLen )
		return -1;

	else{

		buffer[writePos+1] = 0;
		return writePos;
	}

}


void *request_worker(void *connection_info) {

connection_info_type *connection;
int flag = 1;
int retcode;

	connection = (connection_info_type *)connection_info;

	// debug("Remote address is %s\n", inet_ntoa(connection->peeraddr.sin_addr));

	setsockopt(connection->s, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));

	retcode = handleRequest(connection->serverInfo, &connection->peeraddr, connection->s, connection->s);

	if (retcode != 0) {

		debug("Some sort of bad request was received\n");
	}

	struct timespec current_time;
	struct timespec start_time;
	struct pollfd fds[1];
	int ret;

	clock_gettime(CLOCK_MONOTONIC, &start_time);

	int wait_miliseconds = 250;
	int total_wait_so_far = 0;
	char buffer[1024];

	// wait for a while to allow the socket to be drained before closing it

	shutdown(connection->s, SHUT_WR);

	while( total_wait_so_far < wait_miliseconds) {

		fds[0].fd = connection->s;
		fds[0].events = POLLIN;

		ret = poll(fds, 1, 10);

		if (ret == -1)
			break;

		if (ret == 1 ) {

			if (fds[0].revents & POLLIN) {

				// debug("data still left to read before closing\n");
				ret = read(connection->s, buffer, 1024);

				if (ret <= 0) {

					// debug("Client closed the socket or an error occurred\n");
					break;
				}
				else {

					// debug("Read %d byts from connection\n", ret);
				}

			}

		}

		clock_gettime(CLOCK_MONOTONIC, &current_time);

		total_wait_so_far = (current_time.tv_sec * 1000 + current_time.tv_nsec / 1000000) - (start_time.tv_sec * 1000 + start_time.tv_nsec / 1000000);

		// time_left_to_wait = wait_miliseconds - total_wait_so_far;
		// debug("time left to wait %d\n", time_left_to_wait);

	}

	// ok now close it
	close(connection->s);

	free(connection);

	return((void *)0);
}


int main(int argc, char **argv) 
{
serverConfigType serverInfo;
dictionaryType *configDict;
int s;
int pid;

	int ls;
	struct sockaddr_in peeraddr;
	unsigned int addrlen = sizeof(peeraddr);
	connection_info_type *this_connection;

	debug("%s", "Webserver starting up!\n");

    // Reap the children
    signal(SIGCHLD, SIG_IGN);

	// hoststats = 0;
	// ua_stats = 0;

	configDict = loadINI("./server.ini");
	// printDict(configDict);

	mimeDict = loadMIMEtypes("/etc/mime.types");
	// printDict(mimeDict);

	if (loadConfig(&serverInfo, configDict) != 0 )  {
		
		debug( "Problem loading configuration data\n");
		return -1;
	}

	init_404Error("www/html/404.html");

	s = socket(AF_INET, SOCK_STREAM, 0);

	if (s == -1) {

		debug( "Unable to create socket for server process\n");
		return -1;
	}

    int optval = 1;

    setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	if ( bind (s, serverInfo.listenAddr, serverInfo.addr_len) == -1) {

		debug( "Unable to bind to socket %d\n", serverInfo.port);
		return -1;
	}

	if ( listen(s, 100) == -1 ) {

		debug( "Unable to listen for connections\n");
		return -1;
	}

	if (getuid() == 0) {

		debug("Running as root--dropping privileges\n");

		/* process is running as root, drop privileges */
		if (setgid(1000) != 0)
			debug("setgid: Unable to drop group privileges: %s", strerror(errno));
		if (setuid(1000) != 0)
			debug("setuid: Unable to drop user privileges: %s", strerror(errno));

	}

	struct timespec request;
	struct timespec remaining;
	double difference;
	double min_request_interval;
	struct timespec current_time;
	struct timespec previous_request_time;

	previous_request_time.tv_nsec = 0;
	previous_request_time.tv_sec = 0;

	if ( serverInfo.max_requests_per_second ) {

		min_request_interval = 1.0 / serverInfo.max_requests_per_second;
	}
	else {

		min_request_interval = 0.0;
	}
	

	while(1) {

		ls = accept (s, (struct sockaddr *)&peeraddr, &addrlen);

		if ( ls < 0 ) {

			debug("%s", "Some sort of socket error occurred\n");
			perror("has3-web");
			return -1;

		}

		pid = fork();

		if ( pid == -1 ) {
    		debug("[-] fork error: %s\n", strerror(errno));
    		close(s);
    		return -1;
    	}

    	if ( pid ) {
    		// debug("Client %s connected.\n", inet_ntoa(ls.sin_addr));
    		close(ls);
    	} 
		else {

			close(s);
		
			this_connection = malloc(sizeof(connection_info_type));

			if (this_connection == 0 ) {

				debug("%s", "Error calling malloc()");
				return (-1);
			}

			this_connection->s = ls;
			this_connection->serverInfo = &serverInfo;

			memcpy((void*)&this_connection->peeraddr, (void *)&peeraddr, addrlen);

			request_worker(this_connection);

			return 0;


			// **********************
			// originally threaded but decided forking is better in case a memory corruption bug is found
			// **********************

			// pthread_t thread_id;
			// pthread_create(&thread_id, NULL, request_worker, (void *)this_connection);
			// pthread_detach(thread_id);
		
		}


		if ( serverInfo.max_requests_per_second ) {

			clock_gettime(CLOCK_MONOTONIC, &current_time);

			difference = ( (double)current_time.tv_sec + (double)(current_time.tv_nsec)/1000000000) - ((double)previous_request_time.tv_sec + (double)(previous_request_time.tv_nsec)/1000000000);

			// debug("the difference is %f\n", difference);
			// debug("ths min request interval is %f\n", min_request_interval);
			if ( difference < min_request_interval ) {

					// debug( "Request too soon, delaying for a bit\n");
					double delay = min_request_interval - difference;

					request.tv_nsec = 1000000000 * delay;
					request.tv_sec = 0;

					while ( nanosleep(&request, &remaining) != 0 ) {

						request.tv_sec = remaining.tv_sec;
						request.tv_nsec = remaining.tv_nsec;
					}

			}

			clock_gettime(CLOCK_MONOTONIC, &previous_request_time);

		}

		// remove this due to the expected hammering the service will get during Finals
		// addBstNode(&hoststats, peeraddr.sin_addr.s_addr, 0);

	} //while

} //main

