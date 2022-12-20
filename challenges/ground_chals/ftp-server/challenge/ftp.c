#include <malloc.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <arpa/inet.h>

#include "ftp.h"
#include "handlers.h"

#define BANNER  "220 HAS3ftp (0.9.0) "

int error = 0;

void free_cmd( struct command *ftp_cmd )
{
    if ( ftp_cmd ) {
        free(ftp_cmd);
    }

    return;
}

struct ftp *init_ftp_struct( int clientfd, struct sockaddr_in *client_in)
{
    struct ftp *ftp_client = malloc(sizeof(struct ftp));

    if ( ftp_client == NULL ) {
        return NULL;
    }

    ftp_client->sockfd = clientfd;
    ftp_client->homedir = NULL;
    ftp_client->cwd = NULL;
    ftp_client->rnfr = NULL;

    ftp_client->user_sent = 0;
    ftp_client->pass_sent = 0;
    ftp_client->auth_succ = 0;
    ftp_client->port_recvd = 0;
    ftp_client->data_port = 0;

    inet_ntop(AF_INET, (void *)&(client_in->sin_addr), ftp_client->control_ip_addr, INET_ADDRSTRLEN);

    return ftp_client;
}

int send_error( struct ftp *clnt, int err_code, char *err_txt)
{
    char output[256];

    if ( clnt == NULL ) {
        return -1;
    }

    snprintf(output, 256, "%d %s\r\n", err_code, err_txt);
    
    printf("Snd: %s: %s", clnt->control_ip_addr, output);

    if ( write(clnt->sockfd, output, strlen(output)) <= 0 ) {
        return -1;
    }

    return 0;
}

// this was implemented because the FEAT response is slightly different than a normal one
int send_feat( struct ftp *clnt, int err_code, char *err_txt)
{
    char output[512];

    if ( clnt == NULL ) {
        return -1;
    }

    // the dash after the numeric code is required by the protocol and is different than others
    snprintf(output, 512, "%d-%s\r\n", err_code, err_txt);
    
    printf("Snd: %s: %s", clnt->control_ip_addr, output);

    if ( write(clnt->sockfd, output, strlen(output)) <= 0 ) {
        return -1;
    }

    return 0;
}



int send_banner( struct ftp *clnt )
{
    struct utsname sysinfo;

    // get the system information because teams will need it to exploit the heap bug
    uname(&sysinfo);

    if ( clnt == NULL ) {
        return -1;
    } 

    write( FD(clnt), BANNER, strlen(BANNER));
    write( FD(clnt), sysinfo.version, strlen(sysinfo.version));
    write( FD(clnt), "\r\n", 2);
    
    return 0;
}

struct command *parse_command( int length, char *cmd)
{
    struct command *nc = NULL;
    int cmd_len = 0, arg_len = 0;

    if ( !cmd || length == 0 ) {
        goto end;
    }

    // At this point we have a good command but need to split it up.
    nc = calloc(1, sizeof(struct command) );

    if ( nc == NULL ) {
        goto end;
    }

    cmd_len = strcspn( cmd, " ");

    // No spaces means the entire thing is a command
    // Make sure that the length is 4 or less
    if ( cmd_len == 0 ) {
        if ( length > 4 ) {
            goto end;
        }

        memcpy(nc->cmd, cmd, strlen(cmd));
    } else {

        if ( cmd_len > 8 ) {
            goto end;
        }

        memcpy(nc->cmd, cmd, cmd_len);
    }
    
    // Calculate the length of the arguments. Account for the space
    arg_len = length - (cmd_len + 1);

    if ( arg_len > 0 ) {
        if ( arg_len >= 512 ) {
            arg_len = 511;
        }

        nc->len_args = arg_len;

        memcpy(nc->args, cmd+cmd_len+1, arg_len );
    }

end:
    return nc;
}

enum ftp_cmds str_to_cmd( char *cmd )
{
    enum ftp_cmds ftype = UNKN;

    // Figure out which command it is
    if ( !strcasecmp( cmd, "REIN") ) {
        ftype = REIN;
    } else if ( !strcasecmp( cmd, "TYPE") ) { //
        ftype = TYPE;
    } else if ( !strcasecmp( cmd, "SIZE") ) {
        ftype = SIZE;
    } else if ( !strcasecmp( cmd, "REST") ) {
        ftype = REST;
    } else if ( !strcasecmp( cmd, "MLST") ) {
        ftype = MLST;
    } else if ( !strcasecmp( cmd, "STOU") ) { //
        ftype = STOU;
    } else if ( !strcasecmp( cmd, "LIST") ) { //
        ftype = LIST;
    } else if ( !strcasecmp( cmd, "RMDA") ) {
        ftype = RMDA;
    } else if ( !strcasecmp( cmd, "EPRT") ) {
        ftype = EPRT;
    } else if ( !strcasecmp( cmd, "FEAT") ) {
        ftype = FEAT;
    } else if ( !strcasecmp( cmd, "QUEU") ) { //
        ftype = QUEU;
    } else if ( !strcasecmp( cmd, "STOR") ) { //
        ftype = STOR;
    } else if ( !strcasecmp( cmd, "RMD") ) { //
        ftype = RMD;
    } else if ( !strcasecmp( cmd, "CDUP") ) { //
        ftype = CDUP;
    } else if ( !strcasecmp( cmd, "PORT") ) { //
        ftype = PORT;
    } else if ( !strcasecmp( cmd, "PASS") ) { //
        ftype = PASS;
    } else if ( !strcasecmp( cmd, "APPE") ) { //
        ftype = APPE;
    } else if ( !strcasecmp( cmd, "NLST") ) { //
        ftype = NLST;
    } else if ( !strcasecmp( cmd, "FREE") ) { //
        ftype = FREE;
    } else if ( !strcasecmp( cmd, "HELP") ) {
        ftype = HELP;
    } else if ( !strcasecmp( cmd, "VIEW") ) { //
        ftype = VIEW;
    } else if ( !strcasecmp( cmd, "QUIT") ) {
        ftype = QUIT;
    } else if ( !strcasecmp( cmd, "MKD") ) { //
        ftype = MKD;
    } else if ( !strcasecmp( cmd, "MLSD") ) {
        ftype = MLSD;
    } else if ( !strcasecmp( cmd, "PASV") ) {
        ftype = PASV;
    } else if ( !strcasecmp( cmd, "PWD") ) { //
        ftype = PWD;
    } else if ( !strcasecmp( cmd, "RNTO") ) { //
        ftype = RNTO;
    } else if ( !strcasecmp( cmd, "DELE") ) { //
        ftype = DELE;
    } else if ( !strcasecmp( cmd, "MODE") ) {
        ftype = MODE;
    } else if ( !strcasecmp( cmd, "RETR") ) { //
        ftype = RETR;
    } else if ( !strcasecmp( cmd, "EXEC") ) { //
        ftype = EXEC;
    } else if ( !strcasecmp(cmd, "CWD") ) { //
        ftype = CWD;
    } else if ( !strcasecmp( cmd, "EPSV") ) {
        ftype = EPSV;
    } else if ( !strcasecmp( cmd, "STRU") ) {
        ftype = STRU;
    } else if ( !strcasecmp( cmd, "USER") ) { //
        ftype = USER;
    } else if ( !strcasecmp( cmd, "RNFR") ) { //
        ftype = RNFR;
    } else if ( !strcasecmp( cmd, "SYST") ) { //
        ftype = SYST;
    } else if ( !strcasecmp( cmd, "MDTM") ) { //
        ftype = MDTM;
    }

    return ftype;
}

struct command *read_ftp_command( struct ftp *clnt )
{
    int index = 0;

    char line[512];

    char c;

    // Indicates if a \r has been hit. Used to make sure that the next char is a \n
    int recd_r = 0;

    // new line received
    int recd_n = 0;

    struct command *nc = NULL;

    memset(line, 0, 512);

    /// Read until \n or \r\n is hit or 511 bytes are read
    while ( index < 511 ) {
        // Check if there was an error
        if ( read( FD(clnt), &c, 1) <= 0 ) {
            error = 421;
            goto end;
        }

        if ( c == '\n') {
            recd_n = 1;
            break;
        } else if ( c == '\r' && !recd_r) {
            // We received CR must be followed by a NL
            recd_r = 1;
        } else {
            // Check for an invalid format
            if ( recd_r ) {
                send_error( clnt, 501, "CR MUST be followed by NL");
                goto end;
            } else {
                line[index++] = c;
            }
        }
    }

    printf("Rcv: %s: %s\n", clnt->control_ip_addr, line);
    // Make sure that there was a new line
    if ( !recd_n ) {
        send_error( clnt, 501, "Command too long");

        goto end;
    }

    // Make sure that the command was at least 3 bytes
    if ( index < 3 ) {
        send_error( clnt, 501, "Unknown command");

        goto end;
    }

    nc = parse_command(index, line);

    if ( nc == NULL ) {
        send_error(clnt, 501, "Unknown command");

        goto end;
    }

    nc->ft = str_to_cmd( nc->cmd );

    if ( nc->ft == UNKN ) {
        free(nc);
        nc = NULL;

        send_error(clnt, 501, "Unknown command");

        goto end;
    }

end:
    return nc;
}

int execute_command( struct ftp *clnt, struct command *ftp_cmd)
{
    int result = -1;

    switch (ftp_cmd->ft) {
        case PASV:

            result = handle_PASV(clnt, ftp_cmd);
            break;

        case APPE:
            result = handle_APPE( clnt, ftp_cmd);
            break;
        case CDUP:
            result = handle_CDUP( clnt, ftp_cmd);
            break;
        case CWD:
            result = handle_CWD( clnt, ftp_cmd);
            break;
        case DELE:
            result = handle_DELE( clnt, ftp_cmd);
            break;
        case EXEC:
            result = handle_EXEC( clnt, ftp_cmd);
            break;
        case FEAT:
            result = handle_FEAT( clnt, ftp_cmd);
            break;
        case FREE:
            result = handle_FREE( clnt, ftp_cmd);
            break;
        case LIST:
            result = handle_LIST( clnt, ftp_cmd);
            break;
        case MDTM:
            result = handle_MDTM( clnt, ftp_cmd);
            break;
        case MKD:
            result = handle_MKD( clnt, ftp_cmd);
            break;
        case NLST:
            result = handle_NLST( clnt, ftp_cmd);
            break;
        case PASS:
            result = handle_PASS( clnt, ftp_cmd);
            break;
        case PORT:
            result = handle_PORT( clnt, ftp_cmd);
            break;
        case PWD:
            result = handle_PWD( clnt, ftp_cmd);
            break;
        case QUEU:
            result = handle_QUEU( clnt, ftp_cmd);
            break;
        case QUIT:
            error = 421;
            break;
        case RETR:
            result = handle_RETR( clnt, ftp_cmd);
            break;
        case RMD:
            result = handle_RMD( clnt, ftp_cmd);
            break;
        case RNFR:
            result = handle_RNFR( clnt, ftp_cmd);
            break;
        case RNTO:
            result = handle_RNTO( clnt, ftp_cmd);
            break;
        case STOR:
            result = handle_STOR( clnt, ftp_cmd);
            break;
        case STOU:
            result = handle_STOU( clnt, ftp_cmd);
            break;
        case SYST:
            result = handle_SYST( clnt, ftp_cmd);
            break;
        case TYPE:
            result = handle_TYPE( clnt, ftp_cmd);
            break;
        case USER:
            result = handle_USER( clnt, ftp_cmd);
            break;
        case VIEW:
            result = handle_VIEW( clnt, ftp_cmd);
            break;
        default:
            // printf("unknown or unimplemented command\n");
            send_error(clnt, 502, "Command not implemented");
            break;
    };

    return result;
}

// TODO Set socket timeout
void client_handler( int clientfd, struct sockaddr_in *clientaddr )
{
    struct command *ftp_cmd = NULL;

    struct ftp *clnt = init_ftp_struct(clientfd, clientaddr );

    send_banner( clnt );

    while ( !error ) {
        ftp_cmd = read_ftp_command( clnt );

        if ( !ftp_cmd ) {
            continue;
        }

        execute_command(clnt, ftp_cmd);

        free_cmd(ftp_cmd);
    }
    
	close(clientfd);

	exit(0);
}
