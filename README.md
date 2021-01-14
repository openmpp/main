# Main core openM++

This repository is a part of [OpenM++](http://www.openmpp.org/) open source microsimulation platform.

It conatins openM++ compiler (omc) and run-time, model examples and utilities source code.
Please download and unpack release archive to start using openM++.
Release directory structure would allow you to build and run models, start UI and openM++ web-service (oms):

```
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
```

## Download

Latest release source code and binaries: <https://github.com/openmpp/main/releases/latest>

Desktop version:
 - Linux:   openmpp_debian_YYYYMMDD.tar.gz
 - Windows: openmpp_win_YYYYMMDD.zip 
 - MacOS:   openmpp_mac_YYYYMMDD.zip 

Cluster version:
 - Linux:   openmpp_debian_mpi_YYYYMMDD.tar.gz
 - Windows: openmpp_win_mpi_YYYYMMDD.zip 
 - CentOS / RedHat version 8.2:  openmpp_centos_mpi_YYYYMMDD.tar.gz
 
It is recommended to start with desktop version.
Do NOT download _mpi_ binaries unless you have MPI installed on your cluster.
  
Please visit our [wiki](https://ompp.sourceforge.io/wiki/) for more information.

E-mail to: _openmpp dot org at gmail dot com_.

License: MIT.
