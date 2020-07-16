OpenM++
=======

Desktop version:
  Linux:   openmpp_debian_YYYYMMDD.tar.gz
  Windows: openmpp_win_YYYYMMDD.zip 

Cluster version:
  Linux:      openmpp_debian_mpi_YYYYMMDD.tar.gz
  Windows:    openmpp_win_mpi_YYYYMMDD.zip 
  CentOS 8.2: openmpp_centos_mpi_YYYYMMDD.tar.gz

It is recommended to start with desktop version.
Do NOT download _mpi_ binaries unless you have MPI installed on your cluster.
  
Directory structure:
--------------------

${OM_ROOT}/       -> openM++ "root" folder
          include/  -> include files to build models and openM++ itself
          openm/    -> openM++ compiler and run-tile source code
          use/      -> models include files
          sql/      -> openM++ sql scripts to initialize database
          props/    -> models Visual Studio property sheets
          bin/      -> openM++ executables: omc.exe, oms.exe, ompp_ui.bat and others
          lib/      -> libraries to build openM++ models
          etc/      -> extra configuration files
          Excel/    -> Excel PivotMaker utility
          Perl/     -> Perl scripts and utilities
          html/     -> openM++ UI front-end
          log/      -> log files
          licenses/ -> licenses and contributors
          models/     -> models source code and binaries
                bin/  -> model executables, SQLite databases, ini-files
                sql/  -> model sql scripts
                log/  -> model run log files
                ModelName/ -> model(s) source code and project files
          ompp-docker/  -> Docker files to build openM++ and/or models and to run the models
          ompp-go/      -> Go source code: oms web-service, dbcopy utility and GO API
          ompp-python/  -> Python examples: how to use openM++ from Python
          ompp-r/       -> openMpp R package
          ompp-ui/      -> UI front-end source code
          Xcode/        -> Xcode project files for openM++ model

More information at: http://ompp.sourceforge.net/wiki/
E-mail to: _openmpp dot org at gmail dot com_.

Enjoy,
amc1999
