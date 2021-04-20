OpenM++ Linux
=============

It is build on Debian 10 stable, x64, g++ 8.3.0:

make RELEASE=1
make RELEASE=1 OM_MSG_USE=MPI

Tested on CentOS 8.2, Ubuntu 20.04, Debian 10, MX Linux 19
and expected to work on any mordern Linux distribution.

It is also possible to use other versions of g++ or Intel c++,
but c++17 support is critical for OpenM++ and compilers below g++ 7.3 
are not going to work.

CentOS and RedHat:
CentOS 8.2 MPI cluster version build with OpenMPI 4.0.2.
You may need to install openmpi-x86_64 to run MPI version.
Make sure following commands executed without errors:

module load mpi/openmpi-x86_64

More information at: 
https://openmpp.org
https://github.com/openmpp/openmpp.github.io/wiki

Truly yours, 
amc1999
