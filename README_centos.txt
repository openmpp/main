OpenM++ Linux
=============

It is build on CentOS 7, x64, g++ 4.8.5

make RELEASE=1

MPI cluster version build with OpenMPI 1.10.0

make RELEASE=1 OM_MSG_USE=MPI

You may need to install openmpi-x86_64 to run MPI version.
Make sure following commands executed without errors:

module load mpi/openmpi-x86_64

It is also tested on Ubuntu 14.04 and 16.04, 
you can build it or use binaries as is.

It is also possible to use other versions of g++ or Intel c++,
but c++11 support is critical for OpenM++ and compilers below g++ 4.8 
are not going to work.

More information at: http://ompp.sourceforge.net/wiki/

Truly yours, 
amc1999
