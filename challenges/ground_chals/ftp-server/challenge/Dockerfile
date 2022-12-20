
FROM ubuntu:20.04 as builder
RUN DEBIAN_FRONTEND=noninteractive \
  apt -y update && apt -y upgrade \
  && apt -y install build-essential \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /src
ADD Makefile *.c *.h /src/
RUN make

FROM ubuntu:20.04
RUN DEBIAN_FRONTEND=noninteractive \
  apt -y update && apt -y upgrade \
  && rm -rf /var/lib/apt/lists/*
RUN useradd -ms /bin/bash hasftpd && echo "hasftpd:L@bm0nkey2delta" | chpasswd
WORKDIR /home/hasftpd
RUN mkdir flight_software && touch flight_software/it_will_not_be_that_easy
COPY --from=builder /src/ftp_serv ./has-ftpd
RUN useradd -ms /bin/bash hasfsw && echo "hasfsw:M@nB3@rP1g" | chpasswd
RUN echo "temp flag data" > /home/hasfsw/flag.txt
CMD ["./has-ftpd"]
