OpenM++
=======

Desktop version:
  Linux:   openmpp_debian_YYYYMMDD.tar.gz
  Windows: openmpp_win_YYYYMMDD.zip 
  MacOS:   openmpp_mac_YYYYMMDD.zip 

Cluster version:
  Linux:   openmpp_debian_mpi_YYYYMMDD.tar.gz
  Windows: openmpp_win_mpi_YYYYMMDD.zip 
  CentOS / RedHat version 8.2:  openmpp_centos_mpi_YYYYMMDD.tar.gz

It is recommended to start with desktop version.
Do NOT download _mpi_ binaries unless you have MPI installed on your cluster.
  
Directory structure:
--------------------

${OM_ROOT}/      -> openM++ "root" folder
          bin/      -> openM++ executables: omc.exe, oms.exe, ompp_ui.bat and others
          etc/      -> extra configuration files
          Excel/    -> Excel PivotMaker utility
          html/     -> UI front-end
          include/  -> c++ include files to build models and openM++ itself
          lib/      -> c++ libraries to build models
          licenses/ -> licenses and contributors
          log/      -> log files
          models/     -> models source code and binaries
                bin/  -> model executables, SQLite databases, ini-files
                sql/  -> model sql scripts
                log/  -> model run log files
                ModelName/ -> model(s) source code and project files
          openm/    -> c++ source code for openM++ compiler and model run-time
          Perl/     -> Perl scripts and utilities
          props/    -> models Visual Studio property sheets
          sql/      -> sql scripts to initialize database
          use/      -> models include files
          Xcode/        -> Xcode project files for openM++ model
          ompp-docker/  -> Docker files to build openM++ and/or models and to run the models
          ompp-mac/     -> MacOS specific files
          ompp-go/      -> Go source code: oms web-service, dbcopy utility and GO API
          ompp-python/  -> Python examples: how to use openM++ from Python
          ompp-r/       -> openMpp R package
          ompp-ui/      -> UI front-end source code

More information at: https://github.com/openmpp/openmpp.github.io/wiki
E-mail to: _openmpp dot org at gmail dot com_.

Enjoy,
amc1999
