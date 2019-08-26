# Main core openM++

This repository is a part of [OpenM++](http://www.openmpp.org/) open source microsimulation platform.

It conatins openM++ compiler (omc) and run-time, model examples and utilities source code.
Please download and unpack release archive to start using openM++.
Release directory structure would allow you to build and run models, start UI and openM++ web-service (oms):

```
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
          ompp-r/       -> openMpp R package
          ompp-ui/      -> UI front-end source code
```

## Download

Latest release source code and binaries: <https://github.com/openmpp/main/releases/latest>

Desktop version:
 - Linux:   openmpp_centos_YYYYMMDD.tar.gz
 - Windows: openmpp_win_YYYYMMDD.zip 

Cluster version:
 - Linux:   openmpp_centos_mpi_YYYYMMDD.tar.gz
 - Windows: openmpp_win_mpi_YYYYMMDD.zip 

It is recommended to start with desktop version.
Do NOT download _mpi_ binaries unless you have MPI installed on your cluster.
  
Please visit our [wiki](https://ompp.sourceforge.io/wiki/) for more information.

E-mail to: openmpp dot org at gmail dot com.

License: MIT.
