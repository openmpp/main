OpenM++
=======

Desktop version:
  Linux:   openmpp_centos_YYYYMMDD.tar.gz
  Windows: openmpp_win_YYYYMMDD.zip 

Cluster version:
  Linux:   openmpp_centos_mpi_YYYYMMDD.tar.gz
  Windows: openmpp_win_mpi_YYYYMMDD.zip 

More information at: http://ompp.sourceforge.net/wiki/

2017_06_05:
    major release: parameters sub-values for probabilistic statistical analysis
    minor changes and bug fixes, update for Visual Studio 2017

2016_12_29:
    minor release: fix R test script and dbcopy utility bug in delete model

2016_12_24:
    localizataion: support for translated messages in models and omc

2016_11_22:
    minor release to fix performance issues for large parameters csv and compilation

2016_11_15:
    minor release to improve dbcopy utility and fix some bugs

2016_09_28:
    support for model "publish": 
      import-export into and from json and csv files for model, input parameters set, run results
      direct copy between databases (eg: model using SQLite local.db and central storage is MySQL)
    model can directly use csv file(s) for input parameters, no database required
    data access library, API in Go, import-export tools (alpha)

2016_03_22:
    minor release to improve modeling task support and R package

2016_02_16:
    compiler and openM++ model code is stable now
    support modeling task: run model with 1000x sets of input parameters
      Linux build:   g++ version 4.8.5
      Linux MPI:     OpenMPI 1.10.0
      Windows build: Visual Studio 2015 update 1
      Windows MPI:   Microsoft MPI v7

2015_01_06:
    support multiple modelling threads
    many changes in compiler

2014_10_07:
    many fixes and updates in compiler, runtime and models build
    export to Excel from model.sqlite database
    database schema now compatible with SQLite, MySQL, PostgreSQL, MSSQL, DB2

2014_09_11:
    beta version build for Windows and Linux (CentOS)

2014_03_08:
    beta version of R openMpp package to read and write OpenM++ database.

2013_11_20:
    minor changes to alpha version, one database column renamed
    initial version of R openMpp package to read and write OpenM++ database.

2013_10_22:
    alpha version build for Windows and Linux (CentOS)

Enjoy,
amc1999
