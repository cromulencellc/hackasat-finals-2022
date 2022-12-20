#!/bin/bash

# BASE_ADDR=$1
IMAGE_TAG=scrubbed/has3-riscv/riscv-toolchain:rop_solver

if [[ "$(docker images -q $IMAGE_TAG 2> /dev/null)" == "" ]]; then
    echo "Container doesn't exist. Building..."
    echo $IMAGE_TAG
    sleep 1
    docker build . -t $IMAGE_TAG
fi

docker run -it --entrypoint=python3 -v $(pwd):/solver/ -w /solver --network host $IMAGE_TAG /solver/solve.py