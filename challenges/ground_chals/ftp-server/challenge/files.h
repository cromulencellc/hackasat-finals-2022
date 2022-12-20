#ifndef __FILES_H__
#define __FILES_H__

#include "ftp.h"

typedef struct buffer {
    char *data;

    int length;
} buffer;

struct buffer *read_file( struct ftp *clnt, char *name );

void free_buffer( struct buffer *b );

#endif // __FILES_H__
