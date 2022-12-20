#include <sys/stat.h>
#include <malloc.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "files.h"
#include "ftp.h"

struct buffer *read_file( struct ftp *clnt, char *name )
{
    struct buffer *out = NULL;
    struct stat st;
    int fd;
    int index;
    char c;

    if ( !clnt || !name ) {
        goto end;
    }

    if ( stat(name, &st ) ) {
        if ( errno == EACCES || errno == ENOENT ) {
            send_error(clnt, 550, "Requested action not taken. File unavailable.");

            goto end;
        } else {
            send_error( clnt, 421, "Server error");

            goto end;
        }
    }

    if ( access( name, R_OK) ) {
        if ( errno == EACCES || errno == ENOENT ) {
            send_error(clnt, 550, "Requested action not taken. File unavailable.");

            goto end;
        } else {
            send_error( clnt, 421, "Server error");

            goto end;
        }
    }
    // Cannot send unless it is a regular file
    if ( !S_ISREG(st.st_mode) ) {
        send_error( clnt, 550, "Requested action not taken. File unavailable");

        goto end;
    }

    fd = open(name, O_RDONLY);

    if ( fd < 0 ) {
        send_error( clnt, 421, "Server error");

        goto end;
    }

    out = calloc(1, sizeof(struct buffer));

    if ( out == NULL ) {
        send_error( clnt, 421, "Server error");

        goto end;
    }

    out->data = calloc(1, st.st_size);

    if ( out->data == NULL ) {
        free(out);

        send_error( clnt, 421, "Server error");

        out = NULL;
        goto end;
    }

    index = 0;

    while ( (read(fd, &c, 1) == 1 ) ) {
        out->data[index++] = c;
    }

    out->length = index;
    
    close(fd);


end:
    return out;
}

void free_buffer( struct buffer *b )
{
    if ( b ) {
        if ( b->data ) {
            free(b->data);
        }
        free(b);
    }

}
