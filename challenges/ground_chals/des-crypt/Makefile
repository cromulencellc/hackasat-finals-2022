REGISTRY := local

CFLAGS := -Wall -pedantic
CC := gcc

all:
	$(CC) $(CFLAGS) cserv.c -o cserv
	$(CC) $(CFLAGS) des_encrypt.c -o orig_des

clean:
	rm cserv
	rm orig_des

containers:
	docker build . -t $(REGISTRY)/descrypt:challenge
	docker run -p 8000:8000 -v ~/Finals/des-crypt/example_keys:/keys local/descrypt:challenge