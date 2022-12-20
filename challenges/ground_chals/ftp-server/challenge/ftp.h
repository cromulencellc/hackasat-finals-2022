#ifndef __FTP_H__
#define __FTP_H__

#include <arpa/inet.h>

typedef enum ftp_cmds {
	UNKN,
	REIN,
	TYPE,
	SIZE,
	REST,
	MLST,
	STOU,
	LIST,
	RMDA,
	EPRT,
	FEAT,
	QUEU,
	STOR,
	RMD,
	CDUP,
	PORT,
	PASS,
	APPE,
	NLST,
	FREE,
	HELP,
	VIEW,
	QUIT,
	MKD,
	MLSD,
	PASV,
	PWD,
	RNTO,
	DELE,
	MODE,
	RETR,
	EXEC,
	CWD,
	EPSV,
	STRU,
	USER,
	RNFR,
	SYST,
	MDTM,
} ftp_cmds;

typedef struct ftp {
	char user[16];

	char type[16];

	char *homedir;

	char *cwd;

	// socket file descriptor for the client control connection
	int sockfd;

	// socket file description for the data connection
	int data_sockfd;
	
	// used as a flag to indicate that the USER command was sent
	int user_sent;

	// Used as a flag to indicate that the PASS has been sent
	int pass_sent;

	// Flag to indicate that auth succeeded
	int auth_succ;

	// Holds a connect back IP address from the PORT command for data connections
	char data_ip_addr[INET_ADDRSTRLEN];

	// Holds the IP address from the control connection for logging purposes
	char control_ip_addr[INET_ADDRSTRLEN];

	// Points to the file from which it will rename from
	char *rnfr;

	// Holds the set data port
	int data_port;

	// Flag to indicate that the PORT command has been received and parsing correctly
	int port_recvd;

} ftp;

typedef struct command {
	// Text command received
	char cmd[16];

	char args[512];

	int len_args;

	ftp_cmds ft;
} command;

#define FD(client)		(client->sockfd)
#define AUTHD(client) 	(client->auth_succ != 0)

void client_handler( int clientfd, struct sockaddr_in * );
int send_error( struct ftp *clnt, int err_code, char *err_txt);
int send_feat( struct ftp *clnt, int err_code, char *err_txt);
struct command *parse_command( int length, char *cmd);
int execute_command( struct ftp *clnt, struct command *ftp_cmd);
enum ftp_cmds str_to_cmd( char *cmd_str );

#endif // __FTP_H__