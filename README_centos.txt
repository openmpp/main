OpenM++ Linux
=============

It is build on CentOS 8.1, x64, g++ 8.2.1

make RELEASE=1
make RELEASE=1 OM_MSG_USE=MPI

MPI cluster version for CentOS 8.1 build with MPICH
MPI cluster version for CentOS 7 build with OpenMPI 3.1.2

You may need to install openmpi-x86_64 to run MPI version.
Make sure following commands executed without errors:

module load mpi/openmpi-x86_64

On CentOS 8.1 MPICH is recommended instead of openMPI.

OpenM++ also tested on CentOS 7, Ubuntu 18.04, MX Linux 19
and expected to work on any mordern Linux distribution.

It is also possible to use other versions of g++ or Intel c++,
but c++17 support is critical for OpenM++ and compilers below g++ 7.3 
are not going to work.

More information at: 
https://openmpp.org
https://ompp.sourceforge.net/wiki/

Truly yours, 
amc1999
