FROM scrubbed/has3-riscv/riscv-toolchain:latest
USER root

RUN apt update && apt install -y python3-pip

RUN pip3 install pwntools cosmosc2
