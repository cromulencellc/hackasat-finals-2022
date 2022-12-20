CFLAGS := -Wall
CC := gcc

all:
	$(CC) $(CFLAGS) cserv.c ftp.c handlers.c file_utils.c files.c -lcrypt -o ftp_serv
	strip ftp_serv

clean:
	rm ftp_serv

test:
	docker compose pull
	docker compose up
