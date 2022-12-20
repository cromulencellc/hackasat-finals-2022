
#ifndef http_errors_h

#define http_errors_h


int send401Error( int fd, char *realm );
int sendError(int fd, int errorCode);



#endif