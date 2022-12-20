#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <shadow.h>
#include <errno.h>
#include <crypt.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <sys/stat.h>
#include <libgen.h>
#include <fcntl.h>
#include <dirent.h>

#include "file_utils.h"
#include "ftp.h"
#include "files.h"

typedef struct junk {
	char cmd[16];
	struct junk *next;
	char data[];
} junk;

struct junk *root = NULL;

char allmyjunk[512];

int connect_data_port( struct ftp *clnt )
{
    int sockfd;
    socklen_t len;
    struct sockaddr_in sin;

    int result = -1;

    if ( clnt->port_recvd == 0 ) {
        goto end;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if ( sockfd < 0 ) {
        send_error( clnt, 421, "Server error");

        goto end;
    }
    
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int))) {
        send_error( clnt, 421, "Server error");
        close(sockfd);

        goto end;
    }
    
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int))) {
        send_error( clnt, 421, "Server error");
        close(sockfd);

        goto end;
    }

    memset(&sin, 0, sizeof(struct sockaddr_in) );

    sin.sin_family = AF_INET;
    sin.sin_port = htons( clnt->data_port );
    sin.sin_addr.s_addr = inet_addr(clnt->data_ip_addr);

    len = sizeof(struct sockaddr_in);

    if (connect( sockfd, (const struct sockaddr *)&sin, len) < 0 ) {
        send_error(clnt, 421, "Failed to connect to socket");

        close(sockfd);

        goto end;
    }

    result = sockfd;
end:
    return result;
}


int handle_PASV( struct ftp *clnt )
{
   	int sockfd;
	socklen_t len;
	struct sockaddr_in sin;
	struct sockaddr_in server_sin;

	char ipaddr[INET_ADDRSTRLEN];

    if ( !clnt->auth_succ ) {
    	send_error(clnt, 530, "Not logged in");

    	return 0;
    }

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
	sin.sin_port = 0;
	sin.sin_addr.s_addr = inet_addr("0.0.0.0");

	len = sizeof(struct sockaddr_in);

	if ( bind(sockfd, (struct sockaddr *)&sin, len) < 0 ) {
		fprintf(stderr, "[-] bind error: %s\n", strerror(errno));
		close(sockfd);
		return -1;
	}

	len = sizeof(struct sockaddr_in);

	getsockname(sockfd, (struct sockaddr *)&sin, &len);

	len = sizeof(struct sockaddr_in);

	getsockname(clnt->sockfd, (struct sockaddr *)&server_sin, &len);


	// printf("port = %d\n", ntohs(sin.sin_port));


	// strcpy(ipaddr, clnt->control_ip_addr);
	strcpy(ipaddr, inet_ntoa(server_sin.sin_addr));

	
	char *oct1, *oct2, *oct3, *oct4;
	
	oct1 = strtok(ipaddr, ".");
	oct2 = strtok(NULL, ".");	
	oct3 = strtok(NULL, ".");
	oct4 = strtok(NULL, ".");

	unsigned short port = ntohs(sin.sin_port);
	char response[100];

	sprintf(response, "Entering Passive Mode (%s,%s,%s,%s,%d,%d)", oct1, oct2, oct3, oct4, port/256, port - port/256*256);


	if ( listen(sockfd, 1) < 0 ) {
		fprintf(stderr, "[-] listen error: %s\n", strerror(errno));
		close(sockfd);
		return -1;
	}

	send_error(clnt, 227, response);

	int clientfd;
	struct sockaddr_in client_in;

	len = sizeof(struct sockaddr_in);

    clientfd = accept(sockfd, (struct sockaddr *)&client_in, &len );

	// printf("client connected to data channel\n");

	clnt->data_sockfd = clientfd;

	return 0;
}


int handle_TYPE(struct ftp *clnt, struct command *ftp_cmd)
{
    int result = -1;
    char response[256];

    if ( !clnt || !ftp_cmd ) {
        goto end;
    }

    if ( !clnt->auth_succ ) {
    	send_error(clnt, 530, "Not logged in");

    	goto end;
    }

    if ( ftp_cmd->len_args == 0 ) {
    	snprintf(response, 256, "Command okay: %s", clnt->type);

    	send_error(clnt, 200, response);
    } else {
    	memcpy( clnt->type, ftp_cmd->args, sizeof(clnt->type));

    	send_error(clnt, 200, "Command okay");
    }

    result = 0;

end:
	return result;
}

int handle_STOU(struct ftp *clnt, struct command *ftp_cmd)
{
    int result = -1;
    
    int sockfd, filefd;
    char *outfile = NULL;
    char *outname = NULL;
    char response[512];
    char *data = NULL;
    int index = 0, max = 0;

    if ( !clnt || !ftp_cmd ) {
        goto end;
    }

    if ( !clnt->auth_succ ) {
    	send_error(clnt, 530, "Not logged in.");

    	goto end;
    }

    // Make sure that the cwd has an ending slash
    if ( clnt->cwd[strlen(clnt->cwd) - 1] != '/') {
    	outname = calloc(1, strlen(clnt->cwd) + 2);

    	strcpy(outname, clnt->cwd);
    	strcat(outname, "/");
    } else {
    	outname = calloc(1, strlen(clnt->cwd) + 1);

    	strcpy(outname, clnt->cwd);
    }

    // generate unique name
    outfile = random_name( outname, strlen(clnt->cwd) + 7 );

    if ( outfile == NULL ) {
    	send_error( clnt, 421, "Server error");

    	goto end;
    }
    
    filefd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR );

    if ( filefd < 0 ) {
    	send_error(clnt, 421, "Server Error");

    	goto end;
    }

    send_error(clnt, 150, "File status okay; about to open data connection.");

    // Open the data port
    sockfd = connect_data_port( clnt );

    if ( sockfd < 0 ) {
        goto end;
    }

    data = calloc(1, 1024);
    max = 1024;
    index = 0;

    // TODO Pick a max filesize
    while ( (read(sockfd, &(data[index++]), 1) == 1) ) {
    	if ( index == max ) {
    		data = realloc( data, max * 2 );

    		max = max * 2;
    	}
    }

    write(filefd, data, index - 1);

    close(sockfd);
    close(filefd);

    free(data);

    snprintf(response, 512, "FILE: %s", outfile);

    send_error(clnt, 250, response);

	result = 0;
end:

	if ( outfile ) {
		free(outfile);
	}

	return result;
}

int handle_LIST(struct ftp *clnt, struct command *ftp_cmd)
{
    int result = -1;
    char fullpath[PATH_MAX + 1];

    int sockfd;

    char *output = NULL;

    if ( !clnt->auth_succ ) {

    	send_error(clnt, 530, "Not logged in");
    	goto end;
    }

	// the do_file_list function does not handle paths.  No time to fix.

    // Handle if there are no args
    if ( ftp_cmd->len_args == 0 ) {
    	strcpy(ftp_cmd->args, "./" );

    	ftp_cmd->len_args = 2;
    }

	// the do_file_list function does not handle paths.  No time to fix.
	// strcpy(ftp_cmd->args, "./" );
	// ftp_cmd->len_args = 2;


    if (realpath( ftp_cmd->args, fullpath) == NULL ) {
    	if ( errno == ENAMETOOLONG ) {
    		send_error( clnt, 501, "Syntax error in parameters or arguments.");
    	} else {
    		send_error( clnt, 550, "Requested action not taken. File unavailable");
    	}

    	goto end;
    }

	  // see if we are in active mode and open a port if not
	if (clnt->data_sockfd == 0 ) {

		// Open the data port
		sockfd = connect_data_port( clnt );

		if ( sockfd < 0 ) {
			goto end;
		}

		clnt->data_sockfd = sockfd;
	}

	printf("%s\n", fullpath);

    output = do_dir_list(fullpath);

    if ( output == NULL ) {
    	send_error(clnt, 421, "Server Error");
    	goto end;
    }

	send_error(clnt, 150, "File status okay; about to open data connection.");

    write(clnt->data_sockfd, output, strlen(output));
    free(output);

    send_error(clnt, 226, "Closing data connection");

    close(clnt->data_sockfd);

	clnt->data_sockfd = 0;

    result = 0;
end:
    return result;
}

int handle_FEAT(struct ftp *clnt, struct command *ftp_cmd)
{
	int result = -1;

	if ( !clnt || !ftp_cmd ) {
		goto end;
	}

	if ( !clnt->auth_succ ) {
    	send_error(clnt, 530, "Not logged in");

    	goto end;
    }


	// the spaces before each feature label are required by the protocol
	send_feat(clnt, 211, "Extensions supported:\n APPE\n CDUP\n CWD\n DELE\n EPRT\n EXEC\n FEAT\n FREE\n HELP\n LIST\n MDTM\n MKD\n MLSD\n MLST\n MODE\n NLST\n PASS\n PASV\n PORT\n PWD\n QUEU\n QUIT\n REIN\n REST\n RETR\n RMD\n RMDA\n RNFR\n RNTO\n SIZE\n STOR\n STOU\n STRU\n SYST\n TYPE\n USER\n VIEW");

	send_error(clnt, 211, "END");

	result = 0;
end:
	return result;
}

// Add a new command to the queue
int handle_QUEU( struct ftp *clnt, struct command *ftp_cmd)
{
	int result = -1;
	struct junk *newjunk = NULL;
	struct command *sub_cmd = NULL;

	if ( !clnt || !ftp_cmd ) {
		goto end;
	}

	if ( !clnt->auth_succ ) {
    	send_error(clnt, 530, "Not logged in");

    	goto end;
    }

    if ( ftp_cmd->len_args == 0 ) {
    	send_error( clnt, 501, "Syntax error in parameters or arguments.");

    	goto end;
    }

    // Time to parse the args in pretty much the same way we parsed the first one.
    sub_cmd = parse_command( ftp_cmd->len_args, ftp_cmd->args);

    if ( sub_cmd == NULL ) {
    	send_error( clnt, 501, "Syntax error in parameters or arguments.");

    	goto end;
    }

    newjunk = malloc( sizeof(struct junk) + sub_cmd->len_args);

    if ( !newjunk ) {
    	send_error(clnt, 421, "Server error");
    	goto end;
    }

    memcpy(&(newjunk->cmd), sub_cmd->cmd, sizeof(newjunk->cmd));

    if ( sub_cmd->args ) {
    	memcpy( newjunk->data, sub_cmd->args, sub_cmd->len_args);
    }

    free(sub_cmd);

    if ( root == NULL ) {
    	root = newjunk;
    	root->next = NULL;

    } else {
    	newjunk->next = root;
    	root = newjunk;
    }

    send_error(clnt, 200, "Command ok");

    result = 0;
end:
	return result;
}

int handle_STOR(struct ftp *clnt, struct command *ftp_cmd)
{
	int result = -1;

	int sockfd;

	int length;
	int index;
	int max;

	char *outfile = NULL;
	char *data = NULL;
	char *tempout = NULL;
	char *dirn = NULL;

	int filefd;

	char fullpath[PATH_MAX + 1];

	struct stat st;

	if ( !clnt || !ftp_cmd ) {
		goto end;
	}

	if ( !clnt->auth_succ ) {
    	send_error(clnt, 530, "Not logged in");

    	goto end;
    }

    if ( ftp_cmd->len_args == 0 ) {
    	send_error( clnt, 501, "Syntax error in parameters or arguments.");

    	goto end;
    }

    outfile = mock_chroot( clnt->homedir, clnt->cwd, ftp_cmd->args);

    if ( outfile == NULL ) {
    	goto end;
    }

    // check the filename to make sure it doesn't have any invalid characters
    length = strlen(outfile);

    for ( int i = 0; i < length; i++ ) {
    	if ( !isalnum(outfile[i]) && !(outfile[i] == '.') && !(outfile[i] == '-' ) && !(outfile[i] == '/') ) {
    		send_error( clnt, 501, "Syntax error in parameters or arguments.");

    		goto end;
    	}
    }

    // make sure it doesn't end in a slash
    if ( outfile[length-1] == '/' ) {
    	send_error( clnt, 550, "Requested action not taken. File unavailable");

    	goto end;
    }

    if ( length > 128 ) {
    	send_error( clnt, 501, "Syntax error in parameters or arguments.");

    	goto end;
    }

    // Check if the file exists already
    if ( access(outfile, F_OK) == 0 ) {
    	// If the file exists then make sure that it is not a directory
    	if ( stat(outfile, &st ) ) {
    		// Some stat error
    		if ( errno == EACCES || errno == ENOENT ) {
	            send_error(clnt, 550, "Requested action not taken. File unavailable.");

	            goto end;
	        } else {
	            send_error( clnt, 421, "Server error");

	            goto end;
	        }
    	} else {
	        if ( S_ISDIR( st.st_mode ) ) {
    			send_error( clnt, 550, "Requested action not taken. File unavailable");

    			goto end;
    		}
    	}

    	// At this point, the file exists and can be stat'd.
    	// Make sure that we have write access
    	if ( access(outfile, W_OK) ) {
    		send_error( clnt, 550, "Requested action not taken. File unavailable");

    		goto end;
    	}
    	
    } else {
    	// The file doesn't exist. Get the base dir and confirm that the path to that point exists
    	tempout = strdup(outfile);

    	dirn = dirname( tempout );

    	// Get the realpath of the base directory. Failure means that access is denied, doesn't exist, etc
	    if (realpath( dirn, fullpath) == NULL ) {
	    	if ( errno == ENAMETOOLONG ) {
	    		send_error( clnt, 501, "Syntax error in parameters or arguments.");
	    	} else {
	    		send_error( clnt, 550, "Requested action not taken. File unavailable");
	    	}

	    	free(tempout);

	    	goto end;
	    }

	    free(tempout);
    }

    // I think there have been enough checks, time to open the file.

    filefd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR );

    if ( filefd < 0 ) {
    	send_error(clnt, 421, "Server Error");

    	goto end;
    }

    send_error(clnt, 150, "File status okay; about to open data connection.");

    // Open the data port
    sockfd = connect_data_port( clnt );

    if ( sockfd < 0 ) {
        goto end;
    }

    data = calloc(1, 1024);
    max = 1024;
    index = 0;

    // TODO Pick a max filesize
    while ( (read(sockfd, &(data[index++]), 1) == 1) ) {
    	if ( index == max ) {
    		data = realloc( data, max * 2 );

    		max = max * 2;
    	}
    }

    write(filefd, data, index - 1);

    close(sockfd);
    close(filefd);

    free(data);

    send_error(clnt, 250, "Requested file action okay, completed");

	result = 0;
end:

	if ( outfile ) {
		free(outfile);
	}

	return result;
}

int handle_RMD(struct ftp *clnt, struct command *ftp_cmd)
{
    int result = -1;
    char fullpath[PATH_MAX + 1];
    char *goodpath = NULL;

    if ( !clnt || !ftp_cmd ) {
        goto end;
    }

    if ( !clnt->auth_succ ) {
    	send_error(clnt, 530, "Not logged in.");

    	goto end;
    }

    // Must have at least one argument
    if ( ftp_cmd->len_args == 0 ) {
    	send_error(clnt, 501, "Syntax error in parameters or arguments.");

    	goto end;
    }

    goodpath = mock_chroot( clnt->homedir, clnt->cwd, ftp_cmd->args);

    if ( goodpath == NULL ) {
    	goto end;
    }

    // check the path
    if ( !realpath(goodpath, fullpath) ) {
    	if ( errno == ENAMETOOLONG ) {
    		send_error(clnt, 501, "Syntax error in parameters or arguments.");
    	} else {
    		send_error(clnt, 550, "Requested action not taken. File unavailable.");
    	}

    	goto end;
    }

    // At this point the file exists
    if ( rmdir( fullpath) ) {
    	if ( errno == ENAMETOOLONG ) {
    		send_error(clnt, 501, "Syntax error in parameters or arguments.");
    	} else {
    		send_error(clnt, 550, "Requested action not taken. File unavailable.");
    	}

    	goto end;
    }

    send_error(clnt, 200, "Command okay");

    result = 0;

end:
	if ( goodpath ) {
		free(goodpath);
	}

	return result;
}

int handle_CDUP(struct ftp *clnt, struct command *ftp_cmd)
{
	int result = -1;
	char fullpath[PATH_MAX + 1];
	char *dirn = NULL;
	char *tempcwd = NULL;

	if ( !clnt || !ftp_cmd ) {
		goto end;
	}

	if ( !clnt->auth_succ ) {
    	send_error(clnt, 530, "Not logged in");

    	goto end;
    }

    tempcwd = strdup(clnt->cwd);

    // If they are already in the homedir then they can't go higher
    if ( strcmp(clnt->cwd, clnt->homedir) == 0 ) {

    	send_error( clnt, 550, "Requested action not taken. File unavailable");

    	result = 0;

    	goto end;
    }

    dirn = dirname( tempcwd );

    if ( dirn == NULL ) {
    	send_error(clnt, 421, "Server error");

    	free(tempcwd);

    	goto end;
    }

    // Get the real path
    if (realpath( dirn, fullpath) == NULL ) {
    	// TODO make these good errors
    	if ( errno == ENAMETOOLONG ) {
    		send_error( clnt, 501, "Syntax error in parameters or arguments.");
    	} else {
    		send_error( clnt, 550, "Requested action not taken. File unavailable");
    	}

    	free(tempcwd);

    	goto end;
    }

    free(tempcwd);

    if ( chdir(fullpath) ) {
    	if ( errno == ENAMETOOLONG ) {
    		send_error( clnt, 501, "Syntax error in parameters or arguments.");
    	} else {
    		send_error( clnt, 550, "Requested action not taken. File unavailable");
    	}

    	goto end;
    }

    free(clnt->cwd);
    clnt->cwd = strdup( fullpath );

    send_error(clnt, 250, "Requested file action okay, completed.");

    result = 0;

end:
	return result;
}

// TODO: Test this outside of the application 
int handle_PORT(struct ftp *clnt, struct command *ftp_cmd)
{
	int result = -1;
	char *start, *end;

	char ip_addr[16];
	int port;

	if ( !clnt || !ftp_cmd ) {
		goto end;
	}

	if ( !clnt->auth_succ ) {
    	send_error(clnt, 530, "Not logged in");

    	goto end;
    }

	memset(ip_addr, 0, 16);

	if ( !clnt->auth_succ ) {
		send_error(clnt, 530, "Syntax error in command");

		goto end;
	}

	// 127,0,0,1,219,153
	// First Octet
	start = ftp_cmd->args;
	end = strstr(start, ",");

	if ( end == NULL ) {
		send_error(clnt, 501, "Syntax error in command");

		goto end;
	}

	if ( end - start > 3) {
		send_error(clnt, 501, "Syntax error in command");

		goto end;
	}

	strncat(ip_addr, start, end - start);
	strncat(ip_addr, ".", 2);

	start = end + 1;

	// Second Octet
	end = strstr(start, ",");

	if ( end == NULL ) {
		send_error(clnt, 501, "Syntax error in command");

		goto end;
	}

	if ( end - start > 3) {
		send_error(clnt, 501, "Syntax error in command");

		goto end;
	}

	strncat(ip_addr, start, end - start);
	strncat(ip_addr, ".", 2);

	start = end + 1;

	// Third Octet
	end = strstr(start, ",");

	if ( end == NULL ) {
		send_error(clnt, 501, "Syntax error in command");

		goto end;
	}

	if ( end - start > 3) {
		send_error(clnt, 501, "Syntax error in command");

		goto end;
	}

	strncat(ip_addr, start, end - start);
	strncat(ip_addr, ".", 2);

	start = end + 1;

	// Fourth Octet
	end = strstr(start, ",");

	if ( end == NULL ) {
		send_error(clnt, 501, "Syntax error in command");

		goto end;
	}

	if ( end - start > 3) {
		send_error(clnt, 501, "Syntax error in command");

		goto end;
	}

	strncat(ip_addr, start, end - start);

	start = end + 1;

	end = strstr(start, ",");

	if ( end == NULL ) {
		send_error(clnt, 501, "Syntax error in command");

		goto end;
	}

	end[0] = '\x00';

	errno = 0;

	port = strtol(start, NULL, 10) * 256;

	if ( errno == ERANGE || errno == ERANGE) {
		send_error(clnt, 501, "Syntax error in command");

		goto end;
	}

	end[0] = ',';

	start = end + 1;

	port += strtol(start, NULL, 10);

	if ( port < 1024 || port > 65535 ) {
		send_error(clnt, 501, "Syntax error in command");

		goto end;
	}

	memcpy( clnt->data_ip_addr, ip_addr, sizeof(clnt->data_ip_addr) );
	clnt->data_port = port;

	clnt->port_recvd = 1;

	send_error(clnt, 200, "Command okay.");

	result = 0;

end:
	return result;
}

int handle_PASS(struct ftp *clnt, struct command *ftp_cmd)
{
	int result = -1;

	struct passwd *clnt_pass = NULL;

	struct spwd *clnt_secret = NULL;

	char *crypted_pass = NULL;

	if ( !clnt || !ftp_cmd ) {
		goto end;
	}

	// If the USER command has not been sent then it is too early
	if ( clnt->user_sent == 0 ) {
		send_error( clnt, 533, "Bad sequence of commands.");

		goto end;
	}

	// Time to check if the user exists
	clnt_pass = getpwnam((const char *)&clnt->user);

	if ( clnt_pass == NULL ) {
		send_error( clnt, 430, "Invalid User");

		goto end;
	}

	clnt->pass_sent = 1;

	clnt_secret = getspnam((const char *)&clnt->user);

	if ( clnt_secret == NULL ) {
		printf("[ERROR] There seems to be an access issue: Are you root?\n");

		send_error(clnt, 421, "Server auth error");

		goto end;
	}

	crypted_pass = crypt(ftp_cmd->args, clnt_secret->sp_pwdp);

	if ( crypted_pass == NULL ) {
		printf("[ERROR] There seems to be an error with crypt()\n");

		send_error(clnt, 421, "Server auth error");

		goto end;
	}

	if ( strcmp( crypted_pass, clnt_secret->sp_pwdp) ) {
		send_error( clnt, 430, "Invalid User/Pass combo");

		goto end;
	}

	// Have to drop privileges
	if (setgid(clnt_pass->pw_gid) ) {
		send_error(clnt, 421, "Server auth error");

        fprintf(stderr, "setgid() failed: %s\n", strerror(errno));

        goto end;
	}

    if (setuid(clnt_pass->pw_uid) ) {
		send_error(clnt, 421, "Server auth error");

        fprintf(stderr, "setuid() failed: %s\n", strerror(errno));

        goto end;
	}

	// Make sure that I can't get them back
	if (setuid(0) != -1) {
		send_error(clnt, 421, "Server auth error");

        fprintf(stderr, "setuid() should not succeed\n");

        goto end;
	}

	// At this point auth has succeeded
	clnt->auth_succ = 1;

	send_error(clnt, 230, "User logged in, proceed.");

	// setup the homedir
	clnt->homedir = strdup( clnt_pass->pw_dir );

	// setup the current dir
	clnt->cwd = strdup( clnt_pass->pw_dir );

	chdir(clnt->homedir);

	result = 0;

end:
	return result;
}


int handle_APPE(struct ftp *clnt, struct command *ftp_cmd)
{
	int result = -1;

	int sockfd;

	int length;
	int index;
	int max;

	char *outfile = NULL;
	char *data = NULL;
	char *tempout = NULL;
	char *dirn = NULL;

	int filefd;

	char fullpath[PATH_MAX + 1];

	struct stat st;

	if ( !clnt || !ftp_cmd ) {
		goto end;
	}

	if ( !clnt->auth_succ ) {
    	send_error(clnt, 530, "Not logged in");

    	goto end;
    }

    if ( ftp_cmd->len_args == 0 ) {
    	send_error( clnt, 501, "Syntax error in parameters or arguments.");

    	goto end;
    }

    outfile = mock_chroot( clnt->homedir, clnt->cwd, ftp_cmd->args);

    // check the filename to make sure it doesn't have any invalid characters
    length = strlen(outfile);

    for ( int i = 0; i < length; i++ ) {
    	if ( !isalnum(outfile[i]) && !(outfile[i] == '.') && !(outfile[i] == '-' ) && !(outfile[i] == '/') ) {
    		send_error( clnt, 501, "Syntax error in parameters or arguments.");

    		goto end;
    	}
    }

    // make sure it doesn't end in a slash
    if ( outfile[length-1] == '/' ) {
    	send_error( clnt, 550, "Requested action not taken. File unavailable");

    	goto end;
    }

    if ( length > 128 ) {
    	send_error( clnt, 501, "Syntax error in parameters or arguments.");

    	goto end;
    }

    // Check if the file exists already
    if ( access(outfile, F_OK) == 0 ) {
    	// If the file exists then make sure that it is not a directory
    	if ( stat(outfile, &st ) ) {
    		// Some stat error
    		if ( errno == EACCES || errno == ENOENT ) {
	            send_error(clnt, 550, "Requested action not taken. File unavailable.");

	            goto end;
	        } else {
	            send_error( clnt, 421, "Server error");

	            goto end;
	        }
    	} else {
	        if ( S_ISDIR( st.st_mode ) ) {
    			send_error( clnt, 550, "Requested action not taken. File unavailable");

    			goto end;
    		}
    	}

    	// At this point, the file exists and can be stat'd.
    	// Make sure that we have write access
    	if ( access(outfile, W_OK) ) {
    		send_error( clnt, 550, "Requested action not taken. File unavailable");

    		goto end;
    	}
    	
    } else {
    	// The file doesn't exist. Get the base dir and confirm that the path to that point exists
    	tempout = strdup(outfile);

    	dirn = dirname( tempout );

    	// Get the realpath of the base directory. Failure means that access is denied, doesn't exist, etc
	    if (realpath( dirn, fullpath) == NULL ) {
	    	if ( errno == ENAMETOOLONG ) {
	    		send_error( clnt, 501, "Syntax error in parameters or arguments.");
	    	} else {
	    		send_error( clnt, 550, "Requested action not taken. File unavailable");
	    	}

	    	free(tempout);

	    	goto end;
	    }

	    free(tempout);
    }

    // I think there have been enough checks, time to open the file.

    filefd = open(outfile, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR );

    if ( filefd < 0 ) {
    	send_error(clnt, 421, "Server Error");

    	goto end;
    }

    send_error(clnt, 150, "File status okay; about to open data connection.");

    // Open the data port
    sockfd = connect_data_port( clnt );

    if ( sockfd < 0 ) {
        goto end;
    }

    data = calloc(1, 1024);
    max = 1024;
    index = 0;

    // TODO Pick a max filesize
    while ( (read(sockfd, &(data[index++]), 1) == 1) ) {
    	if ( index == max ) {
    		data = realloc( data, max * 2 );

    		max = max * 2;
    	}
    }

    write(filefd, data, index - 1);

    close(sockfd);
    close(filefd);

    free(data);

    send_error(clnt, 250, "Requested file action okay, completed");

	result = 0;
end:

	if ( outfile ) {
		free(outfile);
	}

	return result;
}

int handle_NLST(struct ftp *clnt, struct command *ftp_cmd)
{
	int result = -1;
    char fullpath[PATH_MAX + 1];
    char filepath[PATH_MAX + 1];
    char *goodpath = NULL;

    int sockfd;
    struct stat st;

    DIR *dirp = NULL;
	struct dirent *dent = NULL;

    if ( !clnt->auth_succ ) {
    	send_error(clnt, 530, "Not logged in");

    	goto end;
    }

    // Handle if there are no args
    if ( ftp_cmd->len_args == 0 ) {
    	strcpy(ftp_cmd->args, "./" );

    	ftp_cmd->len_args = 2;
    }

    goodpath = mock_chroot( clnt->homedir, clnt->cwd, ftp_cmd->args);

    if ( goodpath == NULL ) {
    	goto end;
    }

    if (realpath( goodpath, fullpath) == NULL ) {
    	if ( errno == ENAMETOOLONG ) {
    		send_error( clnt, 501, "Syntax error in parameters or arguments.");
    	} else {
    		send_error( clnt, 550, "Requested action not taken. File unavailable");
    	}

    	goto end;
    }

    // The path exists to this point. Make sure that it is a directory
    if ( stat(fullpath, &st) ) {
    	send_error( clnt, 550, "Requested action not taken. File unavailable");

    	goto end;
    }

    if ( !S_ISDIR(st.st_mode) ) {
    	send_error( clnt, 550, "Requested action not taken. File unavailable");

    	goto end;
    }


    // Let the client know that we are about to open the data port
    send_error(clnt, 150, "File status okay; about to open data connection.");

	  // see if we are in active mode and open a port if not
	if (clnt->data_sockfd == 0 ) {


		// Open the data port
		sockfd = connect_data_port( clnt );

		if ( sockfd < 0 ) {
			goto end;
		}

		clnt->data_sockfd = sockfd;
	}
	
    // Open the data port
	// sockfd = connect_data_port( clnt );

    // if ( sockfd < 0 ) {
    //     goto end;
    // }

    // Open the directory
    dirp = opendir( fullpath );
	
	if ( dirp == NULL ) {
		goto end;
	}

	while ( (dent = readdir(dirp)) ) {

		// snprintf(filepath, PATH_MAX, "%s/%s", fullpath, dent->d_name);
		snprintf(filepath, PATH_MAX, "%s",dent->d_name);

		// printf("%s\n", dent->d_name);

		if (dent->d_name[0] == '.') {

			continue;
		}
		
		if ( stat(filepath, &st) ) {
			continue;
		}

		if ( S_ISREG(st.st_mode) ) {
			write(clnt->data_sockfd, filepath, strlen(filepath) );
			write(clnt->data_sockfd, "\r\n", 2);
		}
	}

	send_error(clnt, 226, "Closing data connection");

    close(clnt->data_sockfd);
	clnt->data_sockfd = 0;

    result = 0;
 end:

 	if ( goodpath) {
 		free(goodpath);
 	}

 	return result;
}

// Free links
int handle_FREE( struct ftp *clnt, struct command *ftp_cmd)
{
	int result = -1;
	struct junk *newjunk = NULL;
	int req;
	int index = 0;

	if ( !clnt || !ftp_cmd ) {
		goto end;
	}

	if ( !clnt->auth_succ ) {
    	send_error(clnt, 530, "Not logged in");

    	goto end;
    }

    if ( ftp_cmd->len_args == 0 ) {
    	send_error( clnt, 501, "Syntax error in parameters or arguments.");

    	goto end;
    }

    req = atoi(ftp_cmd->args);

    newjunk = root;

    while (newjunk) {
    	if ( req == index){
    		free(newjunk);
    		break;
    	}

    	index += 1;
    	newjunk = newjunk->next;
    }

    send_error(clnt, 200, "Command ok");

    result = 0;
end:
	return result;
}

// View links
int handle_VIEW( struct ftp *clnt, struct command *ftp_cmd)
{
	int result = -1;
	struct junk *newjunk = NULL;
	char output[64];
	int index = 0;

	if ( !clnt || !ftp_cmd ) {
		goto end;
	}

	if ( !clnt->auth_succ ) {
    	send_error(clnt, 530, "Not logged in");

    	goto end;
    }

    newjunk = root;

    while (newjunk) {
    	snprintf(output, 64, " %d: %s\n", index, newjunk->cmd);

    	send_error(clnt, 200, output);

    	index++;
    	newjunk = newjunk->next;
    }

    result = 0;
end:
	return result;
}

int handle_QUIT(struct ftp *clnt, struct command *ftp_cmd)
{

	return 0;
}

int handle_MKD(struct ftp *clnt, struct command *ftp_cmd)
{
	int result = -1;
	char fullpath[PATH_MAX + 1];
	char *tempout = NULL;
	char *dirn = NULL;
	char *bn = NULL;

	char *tempbn = NULL;
	char *outpath = NULL;
	char *goodpath = NULL;

	if ( !clnt || !ftp_cmd ) {
		goto end;
	}

	if ( !clnt->auth_succ ) {
    	send_error(clnt, 530, "Not logged in");

    	goto end;
    }

    // If no argument is supplied then this is an error
    if ( ftp_cmd->len_args == 0 ) {
    	send_error( clnt, 501, "Syntax error in parameters or arguments.");

    	goto end;
    }

    goodpath = mock_chroot( clnt->homedir, clnt->cwd, ftp_cmd->args);

    if ( goodpath == NULL ) {
		send_error( clnt, 550, "Requested action not taken. File unavailable");
    	goto end;
    }

    // Get the base and make sure that it exists
    tempout = strdup(goodpath);

    dirn = dirname( tempout );

    // Get the real path
    if (realpath( dirn, fullpath) == NULL ) {
    	if ( errno == ENAMETOOLONG ) {
    		send_error( clnt, 501, "Syntax error in parameters or arguments.");
    	} else {
    		send_error( clnt, 550, "Requested action not taken. File unavailable");
    	}

    	goto end;
    }

    tempbn = strdup(goodpath);
    bn = basename(tempbn);

    outpath = calloc(1, strlen(dirn) + strlen(bn) + 2);

    snprintf(outpath, strlen(dirn) + strlen(bn) + 2, "%s/%s", dirn, bn);

    free(tempout);
    free(tempbn);

    // Check if the full path exists
    if ( !access(outpath, F_OK) ) {
    	send_error( clnt, 550, "Requested action not taken. File unavailable");

    	goto end;
    }

    if ( mkdir(outpath, S_IRUSR |S_IWUSR |S_IXUSR |S_IRGRP|S_IXGRP) ) {
    	send_error( clnt, 550, "Requested action not taken. File unavailable");

    	goto end;
    }

    send_error(clnt, 250, "Requested file action okay, completed.");

    result = 0;

end:
	if ( outpath) {
		free(outpath);
	}

	if ( goodpath ) {
		free(goodpath);
	}

	return result;
}

int handle_PWD(struct ftp *clnt, struct command *ftp_cmd)
{
	int result = -1;

	if ( !clnt || !ftp_cmd ) {
		goto end;
	}

	if ( !clnt->auth_succ ) {
    	send_error(clnt, 530, "Not logged in");

    	goto end;
    }

    send_error(clnt, 257, clnt->cwd);
	result = 0;
end:
	return result;
}

int handle_RNTO(struct ftp *clnt, struct command *ftp_cmd)
{
	int result = -1;
	char *fullpath = NULL;

	if ( !clnt || !ftp_cmd ) {
		goto end;
	}

	if ( !clnt->auth_succ ) {
    	send_error(clnt, 530, "Not logged in");

    	goto end;
    }

    // Requires a rnfr
    if ( clnt->rnfr == NULL ) {
    	send_error(clnt, 350, "Requested file action pending further information.");

    	goto end;
    }

    // If no argument is supplied then failure
    if ( ftp_cmd->len_args == 0 ) {
    	send_error( clnt, 501, "Syntax error in parameters or arguments.");

    	goto end;
    }

   	fullpath = mock_chroot( clnt->homedir, clnt->cwd, ftp_cmd->args);

    if ( fullpath == NULL ) {
    	send_error( clnt, 501, "Syntax error in parameters or arguments.");

    	goto end;
    }
    
    if ( rename(clnt->rnfr, fullpath) ) {
    	if ( errno == EACCES ) {
    		send_error( clnt, 550, "Requested action not taken. File unavailable");
    	} else if (errno == EINVAL) {
    		send_error( clnt, 501, "Syntax error in parameters or arguments.");
    	} else {
    		send_error( clnt, 550, "Requested action not taken. File unavailable");
    	}

    	goto end;
    }

    send_error(clnt, 250, "Requested file action okay, completed");

    result = 0;

end:
	if ( clnt->rnfr == NULL ) {
		free(clnt->rnfr);
		clnt->rnfr = NULL;
	}

	if ( fullpath ) {
		free(fullpath);
	}

	return result;
}

int handle_DELE(struct ftp *clnt, struct command *ftp_cmd)
{
    int result = -1;
    char fullpath[PATH_MAX + 1];
    struct stat st;

    char *goodpath = NULL;

    if ( !clnt || !ftp_cmd ) {
        goto end;
    }

    if ( !clnt->auth_succ ) {
    	send_error(clnt, 530, "Not logged in.");

    	goto end;
    }

    // Must have at least one argument
    if ( ftp_cmd->len_args == 0 ) {
    	send_error(clnt, 501, "Syntax error in parameters or arguments.");

    	goto end;
    }

    goodpath = mock_chroot( clnt->homedir, clnt->cwd, ftp_cmd->args);

    if ( goodpath == NULL ) {
    	goto end;
    }

    // check the path
    if ( !realpath(goodpath, fullpath) ) {
    	if ( errno == ENAMETOOLONG ) {
    		send_error(clnt, 501, "Syntax error in parameters or arguments.");
    	} else {
    		send_error(clnt, 550, "Requested action not taken. File unavailable.");
    	}

    	goto end;
    }

    if ( stat(fullpath, &st) ) {
    	if ( errno == ENAMETOOLONG ) {
    		send_error(clnt, 501, "Syntax error in parameters or arguments.");
    	} else {
    		send_error(clnt, 550, "Requested action not taken. File unavailable.");
    	}

    	goto end;
    }

    // Can only be used on a regular file
    if ( !S_ISREG(st.st_mode) ) {
    	send_error(clnt, 550, "Requested action not taken. File unavailable.");

    	goto end;
    }

    if ( remove(fullpath) ) {
    	if ( errno == ENAMETOOLONG ) {
    		send_error(clnt, 501, "Syntax error in parameters or arguments.");
    	} else {
    		send_error(clnt, 550, "Requested action not taken. File unavailable.");
    	}

		goto end;
    }

    send_error(clnt, 250, "Requested file action okay, completed.");

    result = 0;

end:

	if (goodpath) {
		free(goodpath);
	}

	return result;
}

int handle_RETR(struct ftp *clnt, struct command *ftp_cmd)
{
    int result = -1;
    struct buffer *file = NULL;
    char *goodpath = NULL;

    char fullpath[PATH_MAX + 1];

    int sockfd;

    if ( !clnt || !ftp_cmd ) {
        goto end;
    }

    if ( !clnt->auth_succ ) {
    	send_error(clnt, 530, "Not logged in");

    	goto end;
    }

    // Handle if there are no args
    if ( ftp_cmd->len_args == 0 ) {
    	send_error( clnt, 501, "Syntax error in parameters or arguments.");

    	goto end;
    }

	// printf("calling mock_chroot\n");
    goodpath = mock_chroot( clnt->homedir, clnt->cwd, ftp_cmd->args);
	// printf("mock_chroot is done\n");

    if ( goodpath == NULL ) {
		// printf("bad call to goodpath\n");
		send_error( clnt, 550, "Requested action not taken. File unavailable");

    	goto end;
    }
	else {

		// printf("Goodpath = %s\n", goodpath);
	}
    
    if (realpath( goodpath, fullpath) == NULL ) {
    	if ( errno == ENAMETOOLONG ) {
    		send_error( clnt, 501, "Syntax error in parameters or arguments.");
    	} else {
    		send_error( clnt, 550, "Requested action not taken. File unavailable");
    	}

    	goto end;
    }

    file = read_file( clnt, fullpath);

    if ( file == NULL ) {
    	goto end;
    }

    send_error(clnt, 150, "File status okay; about to open data connection.");

	if ( clnt->data_sockfd == 0 ) {

		sockfd = connect_data_port( clnt );

		if ( sockfd < 0 ) {
			free_buffer(file);

			goto end;
		}

		clnt->data_sockfd = sockfd;
	}

    write(clnt->data_sockfd, file->data, file->length);

    free_buffer( file );

    send_error(clnt, 250, "Requested file action okay, completed");

    close(clnt->data_sockfd);
	clnt->data_sockfd = 0;

    result = 0;

end:
	if (goodpath) {
		free(goodpath);
	}

	return result;
}

// Execute the queue
int handle_EXEC( struct ftp *clnt, struct command *ftp_cmd)
{
	int result = -1;

	struct command newcmd;
	struct junk *next_cmd = NULL;

	if ( !clnt || !ftp_cmd ) {
		goto end;
	}

	if ( !clnt->auth_succ ) {
    	send_error(clnt, 530, "Not logged in");

    	goto end;
    }

    // Loop through and execute each command
    next_cmd = root;

    while ( next_cmd ) {
    	root = root->next;

    	// Setup the command structure
    	strncpy( (char*)&(newcmd.cmd), next_cmd->cmd, sizeof(newcmd.cmd));

    	strncpy( (char*)&(newcmd.args), next_cmd->data, sizeof(newcmd.args));

    	newcmd.len_args = strlen((char*)&newcmd.args);

    	newcmd.ft = str_to_cmd((char*)&newcmd.cmd);

    	free(next_cmd);

    	if ( newcmd.ft != UNKN ) {
    		execute_command(clnt, &newcmd );
    	}

    	next_cmd = root;

    }

    result = 0;

end:
	return result;
}

int handle_CWD(struct ftp *clnt, struct command *ftp_cmd)
{
	int result = -1;
	char fullpath[PATH_MAX + 1];
	char *goodpath = NULL;

	if ( !clnt || !ftp_cmd ) {
		goto end;
	}

	if ( !clnt->auth_succ ) {
    	send_error(clnt, 530, "Not logged in");

    	goto end;
    }

    // If no argument is supplied then move to the home dir
    if ( ftp_cmd->len_args == 0 ) {
    	strcpy(ftp_cmd->args, clnt->homedir );

    	ftp_cmd->len_args = strlen(clnt->homedir);
    }

    goodpath = mock_chroot(clnt->homedir, clnt->cwd, ftp_cmd->args);

    if ( goodpath == NULL) {

		send_error( clnt, 550, "Requested action not taken. File unavailable");

    	goto end;
    }

    // Get the real path
    if (realpath( goodpath, fullpath) == NULL ) {
    	// TODO make these good errors
    	if ( errno == ENAMETOOLONG ) {
    		send_error( clnt, 501, "Syntax error in parameters or arguments.");
    	} else {
    		send_error( clnt, 550, "Requested action not taken. File unavailable");
    	}

    	goto end;
    }

    if ( chdir(fullpath) ) {
    	if ( errno == ENAMETOOLONG ) {
    		send_error( clnt, 501, "Syntax error in parameters or arguments.");
    	} else {
    		send_error( clnt, 550, "Requested action not taken. File unavailable");
    	}

    	goto end;
    }

    free(clnt->cwd);
    clnt->cwd = strdup( fullpath );

    send_error(clnt, 250, "Requested file action okay, completed.");

    result = 0;

end:

	if (goodpath) {
		free(goodpath);
	}

	return result;
}

int handle_USER(struct ftp *clnt, struct command *ftp_cmd)
{
	int result = -1;

	int index = 0;

	if ( !clnt || !ftp_cmd ) {
		goto end;
	}

	if ( ftp_cmd->len_args == 0 ) {
    	send_error( clnt, 501, "Syntax error in parameters or arguments.");

    	goto end;
    }

	//Length of the supplied user must be less than 15 bytes
	if ( ftp_cmd->len_args > 15 ) {
		send_error(clnt, 530, "USER too long");

		goto end;
	}

	// make it to lower. The only things allowed are ascii chars
	for (index = 0; index < ftp_cmd->len_args; index++ ) {
		if ( !isalpha(ftp_cmd->args[index]) ) {
			send_error(clnt, 530, "USER contains invalid chars");

			goto end;
		}

		ftp_cmd->args[index] = tolower(ftp_cmd->args[index]);
	}

	strcpy(clnt->user, ftp_cmd->args);

	// When a user command is sent all previous auth is removed
	clnt->user_sent = 1;
	clnt->pass_sent = 0;
	clnt->auth_succ = 0;

	result = 0;

	// Valid user name
	send_error(clnt, 331, "User name okay, need password.");
end:
	return result;
}

int handle_RNFR(struct ftp *clnt, struct command *ftp_cmd)
{
	int result = -1;
	char fullpath[PATH_MAX + 1];
	char *goodpath = NULL;

	if ( !clnt || !ftp_cmd ) {
		goto end;
	}

	if ( !clnt->auth_succ ) {
    	send_error(clnt, 530, "Not logged in");

    	goto end;
    }

    // If no argument is supplied then failure
    if ( ftp_cmd->len_args == 0 ) {
    	send_error( clnt, 501, "Syntax error in parameters or arguments.");

    	goto end;
    }

    goodpath = mock_chroot( clnt->homedir, clnt->cwd, ftp_cmd->args);

    if ( goodpath == NULL ) {
    	goto end;
    }

    // Get the real path
    if (realpath( goodpath, fullpath) == NULL ) {
    	// TODO make these good errors
    	if ( errno == ENAMETOOLONG ) {
    		send_error( clnt, 501, "Syntax error in parameters or arguments.");
    	} else {
    		send_error( clnt, 550, "Requested action not taken. File unavailable");
    	}

    	goto end;
    }

    clnt->rnfr = strdup(fullpath);

    send_error(clnt, 350, "Requested file action pending further information.");

    result = 0;

end:
	if ( goodpath ) {
		free(goodpath);
	}

	return result;
}

int handle_SYST(struct ftp *clnt, struct command *ftp_cmd)
{
	if ( !clnt->auth_succ ) {
    	send_error(clnt, 530, "Not logged in");

    	goto end;
    }

	send_error(clnt, 215, "UNIX Type: L8");

end:
	return 0;
}

int handle_MDTM(struct ftp *clnt, struct command *ftp_cmd)
{
    int result = -1;
    char fullpath[PATH_MAX + 1];
    struct stat st;
    struct tm *modtime = NULL;
    char mt[512];
    char *goodpath = NULL;

    if ( !clnt || !ftp_cmd ) {
        goto end;
    }

    if ( !clnt->auth_succ ) {
    	send_error(clnt, 530, "Not logged in.");

    	goto end;
    }

    // Must have at least one argument
    if ( ftp_cmd->len_args == 0 ) {
    	send_error(clnt, 501, "Syntax error in parameters or arguments.");

    	goto end;
    }

    goodpath = mock_chroot( clnt->homedir, clnt->cwd, ftp_cmd->args);

    if ( goodpath == NULL ) {
    	goto end;
    }

    // check the path
    if ( !realpath(goodpath, fullpath) ) {
    	if ( errno == ENAMETOOLONG ) {
    		send_error(clnt, 501, "Syntax error in parameters or arguments.");
    	} else {
    		send_error(clnt, 550, "Requested action not taken. File unavailable.");
    	}

    	goto end;
    }

    if ( stat(fullpath, &st) ) {
    	if ( errno == ENAMETOOLONG ) {
    		send_error(clnt, 501, "Syntax error in parameters or arguments.");
    	} else {
    		send_error(clnt, 550, "Requested action not taken. File unavailable.");
    	}

    	goto end;
    }

    modtime = gmtime( &(st.st_mtime));

    strftime(mt, 512, "%Y%m%d%H%M%S", modtime);

    send_error(clnt, 213, mt );

    result = 0;

end:

	if (goodpath) {
		free(goodpath);
	}

	return result;
}