OpenM++ Linux
=============

It is build on Debian 11 and 12, Ubuntu 24.04, RedHat 9:

make RELEASE=1
make RELEASE=1 OM_MSG_USE=MPI

Also tested on Debian 10, MX Linux 21 and 23, Ubuntu 22.04, RedHat 8
and expected to work on any mordern Linux distribution.

It is also possible to use other versions of g++ or Intel c++,
but c++17 support is critical for OpenM++ and compilers below g++ 7.3 
are not going to work.

To find out more about build environment or model run environment
please see our Docker Hub:
  https://hub.docker.com/r/openmpp/openmpp-run
  https://hub.docker.com/r/openmpp/openmpp-build
or Dockerfiles:
  https://github.com/openmpp/docker

More information at: 
  https://openmpp.org
  https://github.com/openmpp/openmpp.github.io/wiki

Truly yours, 
amc1999
