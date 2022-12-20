#ifndef __HANDLERS_H__
#define __HANDLERS_H__

int handle_APPE(struct ftp *clnt, struct command *ftp_cmd);
int handle_CDUP(struct ftp *clnt, struct command *ftp_cmd);
int handle_CWD(struct ftp *clnt, struct command *ftp_cmd);
int handle_DELE(struct ftp *clnt, struct command *ftp_cmd);
int handle_FEAT(struct ftp *clnt, struct command *ftp_cmd);
int handle_LIST(struct ftp *clnt, struct command *ftp_cmd);
int handle_MDTM(struct ftp *clnt, struct command *ftp_cmd);
int handle_MKD(struct ftp *clnt, struct command *ftp_cmd);
int handle_MLST(struct ftp *clnt, struct command *ftp_cmd);
int handle_NLST(struct ftp *clnt, struct command *ftp_cmd);
int handle_PASS(struct ftp *clnt, struct command *ftp_cmd);
int handle_PORT(struct ftp *clnt, struct command *ftp_cmd);
int handle_PWD(struct ftp *clnt, struct command *ftp_cmd);
int handle_QUIT(struct ftp *clnt, struct command *ftp_cmd);
int handle_RETR(struct ftp *clnt, struct command *ftp_cmd);
int handle_RNFR(struct ftp *clnt, struct command *ftp_cmd);
int handle_RNTO(struct ftp *clnt, struct command *ftp_cmd);
int handle_RMD(struct ftp *clnt, struct command *ftp_cmd);
int handle_STOR(struct ftp *clnt, struct command *ftp_cmd);
int handle_STOU(struct ftp *clnt, struct command *ftp_cmd);
int handle_SYST(struct ftp *clnt, struct command *ftp_cmd);
int handle_TYPE(struct ftp *clnt, struct command *ftp_cmd);
int handle_USER(struct ftp *clnt, struct command *ftp_cmd);
int handle_QUEU(struct ftp *clnt, struct command *ftp_cmd);
int handle_VIEW(struct ftp *clnt, struct command *ftp_cmd);
int handle_FREE(struct ftp *clnt, struct command *ftp_cmd);
int handle_EXEC(struct ftp *clnt, struct command *ftp_cmd);
int handle_PASV(struct ftp *clnt, struct command *ftp_cmd);

#endif //__HANDLERS_H__
