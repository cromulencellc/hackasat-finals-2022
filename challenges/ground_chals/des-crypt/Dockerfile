FROM ubuntu:focal as build_env


RUN apt update &&\
    apt install -y build-essential


WORKDIR /build

COPY *.c /build/
COPY Makefile /build/

RUN make all 

FROM python:3.10-slim

ARG UID=1000
ARG GUID=1000

RUN pip3 install pyyaml


WORKDIR /challenge

COPY --from=build_env /build/cserv /challenge/
COPY --from=build_env /build/orig_des /challenge/

COPY *.py /challenge/
COPY *.yml /challenge/
VOLUME /keys

# Add less privledged user and change to them
RUN groupadd -r --gid=${GUID} challenge && \
    useradd -m -r -s /bin/bash -g challenge --uid=${UID} challenge && \
    mkdir -p /challenge && \
    chown -R challenge /challenge && \
    chgrp -R challenge /challenge && \
    chmod +x /challenge/all_keys.py

USER challenge

CMD python3 all_keys.py --start_port=8000 --keylocation /keys