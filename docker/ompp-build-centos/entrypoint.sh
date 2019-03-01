#!/usr/bin/env bash
set -e

# display help text prompt if user want to see it
if [ "$1" = '-?' ] || [ "$1" = '-h' ] || [ "$1" = '--help' ]; then
  exec cat /scripts/README.txt
fi

# copy build scripts
cp -uv \
 /scripts/build-all \
 /scripts/build-openm \
 /scripts/build-models \
 /scripts/build-go \
 /scripts/build-r \
 /scripts/build-ui \
 /scripts/build-tar-gz \
 /scripts/README.txt \
 $HOME/build

# set environment: open MPI, Go, node.js, R
source /usr/share/Modules/init/bash
module load mpi/openmpi-x86_64

export GOROOT=/go
export GOPATH=$HOME/build/ompp/ompp-go

export PATH=$GOROOT/bin:$GOPATH/bin:/node/bin:$PATH

# set c++17 environment
all_args="${@}"
scl enable devtoolset-7 "bash -c \"${all_args}\""

