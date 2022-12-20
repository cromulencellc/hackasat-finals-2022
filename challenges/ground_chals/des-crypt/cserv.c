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
#include <sys/wait.h>

// Server socket descriptor
int sockfd_g;

typedef struct des {
	int length;	
	char *des_program;
	
} des;

// Ensure that the keyfile is accessible and is the correct size
int confirm_keyfile( char *keyfile )
{
	struct stat st;

	if (keyfile == NULL ) {
		return 0;
	}

	if ( access(keyfile, R_OK) ) {
		return 0;
	}

	if ( stat(keyfile, &st) ) {
		return 0;
	}

	if (st.st_size != 8) {
		return 0;
	}

	return 1;
}

// Loads the des program into memory
struct des* load_des_program( char *progname )
{
	struct des *result = NULL;
	struct stat st;
	int fd;

	if ( progname == NULL ) {
		return result;
	}

	if ( stat(progname, &st) ) {
		fprintf(stderr, "[-] Failed to stat %s: %s\n", progname, strerror(errno));
		return result;
	}

	fd = open(progname, O_RDONLY);

	if ( fd < 0 ) {
		fprintf(stderr, "[-] Failed to open %s: %s\n", progname, strerror(errno));
		return result;
	}

	result = calloc(1, sizeof(des));

	if ( result == NULL ) {
		fprintf(stderr, "[-] malloc fail: %s\n", strerror(errno));
		return result;
	}

	result->des_program = calloc(1, st.st_size);

	if ( result->des_program == NULL ) {
		fprintf(stderr, "[-] malloc fail: %s\n", strerror(errno));
		free(result);
		result = NULL;
		return result;
	}

	result->length = st.st_size;

	if ( read(fd, result->des_program, result->length) != result->length ) {
		fprintf(stderr, "[-] read fail: %s\n", strerror(errno));
		free(result->des_program);
		free(result);
		close(fd);
		result = NULL;
		return result;
	}

	close(fd);

	return result;
}

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

int read_int( int clientfd )
{
	char buffer[16];

	int offset = 0;

	char c;

	memset(buffer, 0, 16);

	while( offset < 15 ) {
		if ( read( clientfd, &c, 1) <= 0 ) {
			goto end;
		}

		if ( c == '\n') {
			goto end;
		}

		buffer[offset++] = c;
	}

end:
	return strtol((char *)&buffer, NULL, 10);
}

char *parse_keyfile( char *keyfile )
{
	
	char *key = NULL;
	int fd;
	char *filedata = NULL;
	struct stat st;

	char *start, *end;

	if ( keyfile == NULL ) {
		goto end;
	}

	fd = open(keyfile, O_RDONLY);

	if ( fd <= 0) {
		goto end;
	}

	if ( stat(keyfile, &st) ) {
		close(fd);

		return 0;
	}

	filedata = calloc(1, st.st_size);

	if ( filedata == NULL ) {
		close(fd);

		goto end;
	}

	read(fd, filedata, st.st_size);

	close(fd);

	start = filedata;

	while ( (start = strchr(start, '"') ) != NULL ) {
		end = strchr(start + 1, '"');

		if ( end == NULL ) {
			goto end;
		}

		if (memcmp( start+1, "password", (end - start) - 1) == 0) {
			// Now get the actual value
			start = strchr(end + 1, '"');

			if (start == NULL ) {
				goto end;
			}

			end = strchr(start + 1, '"');

			if ( end == NULL ) {
				goto end;
			}

			// Kill the quote
			end[0] = '\x00';

			// Get a copy
			key = strdup( start + 1 );

			if ( !key ) {
				goto end;
			}

			if ( strlen(key) != 8 ) {
				printf("[ERROR] Password is not 8 bytes\n");

				free(key);

				key = NULL;
			}

			goto end;
		}

		start = end + 1;
	}

end:
	if ( filedata ) {
		free(filedata);
	}

	return key;
}

// TODO Set socket timeout
void client_handler( int clientfd, struct des* enc_prog, char *keyfile )
{
	// File descriptor for the temp file
	int fd;

	// Name for the temp file
	char tmpfile[16];
	strcpy((char*)&tmpfile, "/tmp/desXXXXXX");

	// Name for the plaintext/ciphertext file
	char user_data_file[16];
	strcpy((char*)&user_data_file, "/tmp/pcXXXXXX");

	// offset into the binary that will be written
	int offset;

	// byte that will be written
	char byte;

	// Length of the buffer to receive
	int length;

	// buffer for the plain text
	char *plaintext = NULL;

	// Buffer for the encryption command
	char *enc_cmd = NULL;

	// Key data
	char *key = NULL;

	// Buffer for the encrypted text
	char *ciphertext = NULL;

	int pid;
	int status;

	if ( enc_prog == NULL ) {
		goto end;
	}

	if ( enc_prog->des_program == NULL ) {
		goto end;
	}

	key = parse_keyfile( keyfile );

	if ( key == NULL ) {
		write(clientfd, "key parse fail: server error\n", 13);
		close(clientfd);

		goto end;
	}
	char out[64];
	memset( out , 0 , 64);
	snprintf( out , 64 , "Keyfile: %s\n", keyfile);
	write( clientfd ,out , 64 );
	//fprintf( clientfd , "Keyfile %s\n", keyfile );
	// Read the offset
	offset = read_int( clientfd );

	// If the offset is within a valid range then read the char
	if ( 0 <= offset && offset < enc_prog->length) {
		read(clientfd, &byte, 1);

		enc_prog->des_program[offset] = byte;
	}

	// Read the size of the buffer to read
	length = read_int( clientfd );

	if ( length <= 0 || length > 256) {
		write(clientfd, "invalid value\n", 14);
		close(clientfd);

		goto end;
	}

	// Read the data
	plaintext = calloc(1, length);
	read(clientfd, plaintext, length);

	// Open the file for writing the data
	fd = mkstemp( (char*)&user_data_file );

	if ( fd < 0 ) {
		goto end;
	}

	write(fd, plaintext, length);
	close(fd);

	fd = mkstemp( (char*)&tmpfile );

	if ( fd < 0 ) {
		goto end;
	}

	write(fd, enc_prog->des_program, enc_prog->length );

	// Set RWX for the binary for the user
	fchmod( fd, S_IRUSR | S_IWUSR | S_IXUSR );

	close(fd);

	char *args[] = {tmpfile, "-k", key, "-p", user_data_file, "-o", user_data_file, NULL};

	pid = fork();

	if ( pid == 0 ) {
		execve( tmpfile, args, NULL);

		printf("Error failed to exed: %s\n", strerror(errno));

		exit(0);
	}

	// Wait for the child to complete
	waitpid( pid, &status, 0);

	// Calculate ciphertext length
	if ( length % 8 ) {
		length = length + (8 - (length %8) );
	}

	ciphertext = calloc(1, length);

	// Time to open the encrypted data and send it back.
	fd = open(user_data_file, O_RDONLY);

	if ( fd < 0 ) {
		write(clientfd, "open() failed: server error\n", 13);
		close(clientfd);

		goto end;
	}

	read(fd, ciphertext, length);

	close(fd);

	write(clientfd, ciphertext, length);
	close(clientfd);

end:
	if ( key ) {
		free(key);
	}

	remove((const char *)&tmpfile);
	remove((const char *)&user_data_file);

	if ( plaintext ) {
		free(plaintext);
	}

	if ( enc_cmd ) {
		free(enc_cmd);
	}

	if ( ciphertext ) {
		free(ciphertext);
	}

	if (enc_prog) {
		if ( enc_prog->des_program) {
			free(enc_prog->des_program);
		}

		free(enc_prog);
	}

	exit(0);
}

int main(int argc, char **argv)
{
	int opt, pid, clientfd;
	socklen_t len;
	struct sockaddr_in client_in;

	// Name of the program used to perform the des encryption
	char *des_prog = NULL;

	// Name of the keyfile
	char *keyfile = NULL;

	// Port on which to listen
	int port = 8489;

	struct des *enc_prog = NULL;

	while((opt = getopt(argc, argv, "d:p:k:")) != -1) { 
        switch(opt) { 
            case 'd': 
                if ( des_prog ) {
                    printf("duplicate des application\n");
                    break;
                }

                des_prog = strdup(optarg);

                break;
            case 'k': 
                if ( keyfile ) {
                    printf("duplicate keyfile\n");
                    break;
                }

                keyfile = strdup(optarg);

                break;
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

    if ( des_prog == NULL ) {
    	fprintf(stderr, "[-] -d option is required\n");
    	return -1;
    }

    if ( keyfile == NULL ) {
    	fprintf(stderr, "[-] -k option is required\n");
    	return -1;
    }

    // Read in the des file
    enc_prog = load_des_program( des_prog );

    if ( enc_prog == NULL ) {
    	free(des_prog);
    	return -1;
    }

    // Start the listening server
    sockfd_g = setup_socket( port );

    if ( sockfd_g < 0 ) {
    	free(des_prog);
    	return -1;
    }

    // Since we made it this far, setup the SIGINT handler to properly close the socket
    signal(SIGINT, handler);

    len = sizeof(struct sockaddr_in);

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
    		printf("[+] Client connected.\n");
    		close(clientfd);
    		continue;
    	} else {
    		close(sockfd_g);
    		client_handler(clientfd, enc_prog, keyfile);
    	}
    }

    return 0;
}
