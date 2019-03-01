#!/usr/bin/env bash
set -e

# set environment: open MPI
source /usr/share/Modules/init/bash
module load mpi/openmpi-x86_64

#
exec "${@}"

