#ifndef http_h
#define http_h

#include "dictionary.h"
#include "config.h"
#include <netinet/in.h>
#include <arpa/inet.h>

#define  METHOD_GET 1
#define  METHOD_PUT 2
#define METHOD_POST 3
#define METHOD_HEAD 4
#define METHOD_DELETE 5
#define MAX_URI 2048
#define MAX_QUERY 2048

enum state { METHOD = 0, URI, QUERY, VERSION, HEADERS, BODY};


int handleRequest(serverConfigType *config, struct sockaddr_in *peeraddr, int s, int t);
int serveFile(int fd, serverConfigType *config, char *uri, dictionaryType *headers );
int sendError(int fd, int errorCode);
int send401Error( int fd, char *realm );
int handlePost(int fdin, int fdout, serverConfigType *config, char *uri, dictionaryType *headers, char *version);
int headFile(int fd, serverConfigType *config, char *uri, dictionaryType *headers );
char *handle_encoded_form_data( char *data);
char *form_decode(char *form_data);

#define HTTP_BUFFER_SIZE 4096
#define MAX_PATH_LEN 256

#endif 