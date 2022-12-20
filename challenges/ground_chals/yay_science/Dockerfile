FROM ubuntu:20.04 as artifacts


RUN apt-get update ;\
    apt-get install -y  wget \
    && apt-get clean \
    && apt-get autoclean \ 
    && rm -rf /var/lib/apt/lists/*
    
ARG COMMS_VERSION=1.0.3
ARG TOKEN
ARG TOKEN_HEADER=PRIVATE-TOKEN

WORKDIR /artifacts
RUN wget --header="${TOKEN_HEADER}: ${TOKEN}" scrubbed-${COMMS_VERSION}-py3-none-any.whl

FROM python:3.10-slim

ARG COMMS_VERSION=1.0.3
COPY reqs.txt /artifacts/
COPY --from=artifacts //artifacts/pycomms-${COMMS_VERSION}-py3-none-any.whl /artifacts/

RUN pip3 install --no-cache-dir -r /artifacts/reqs.txt
RUN pip3 install --no-cache-dir /artifacts/*.whl


WORKDIR /astropoint
COPY *.py /astropoint/
COPY *.bsp /astropoint/
COPY *.yml /astropoint/

VOLUME /game

CMD ["python3","run.py","--interfaces","/game/interfaces.yml","--game","/game/game.yml"]