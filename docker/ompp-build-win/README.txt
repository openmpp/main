=====================
To build openM++ run:

  docker run .... <image-name> build-all

  Examples:
  docker run --isolation process -v C:\my\build:C:\build ompp-build-win build-all
  docker run --isolation process -v C:\my\build:C:\build -e OM_BUILD_PLATFORMS=x64 ompp-build-win build-all
  docker run --isolation process -v C:\my\build:C:\build -e MODEL_DIRS=RiskPaths   ompp-build-win build-all

  Environment variables:
  set OM_BUILD_CONFIGS=Release,Debug (default: Release)
  set OM_BUILD_PLATFORMS=Win32,x64   (default: Win32)
  set OM_MSG_USE=MPI                 (default: EMPTY)
  set MODEL_DIRS=modelOne,NewCaseBased,NewTimeBased,NewCaseBased_bilingual,NewTimeBased_bilingual,IDMM,OzProj,OzProjGen,RiskPaths

To build openM++ libraries and omc compiler run:

  docker run .... <image-name> build-openm
  
  Environment variables to control "build-openm": OM_BUILD_CONFIGS, OM_BUILD_PLATFORMS, OM_MSG_USE

To build models run:

  docker run .... <image-name> build-modles
  
  Environment variables to control "build-modles": OM_BUILD_CONFIGS, OM_BUILD_PLATFORMS, OM_MSG_USE, MODEL_DIRS

To build openM++ tools run any of:

  docker run .... <image-name> build-go   # Go oms web-service and dbcopy utility
  docker run .... <image-name> build-r    # openMpp R package
  docker run .... <image-name> build-perl # Perl utilities
  docker run .... <image-name> build-ui   # openM++ UI (alpha)

To create openmpp_win_YYYYMMDD.zip archive:

  docker run .... <image-name> build-zip
  
  Environment variables to control "build-zip": OM_MSG_USE, MODEL_DIRS

To open cmd command prompt or Perl command prompt:

  docker run .... -it <image-name> cmd
  docker run .... -it <image-name> C:\perl\portableshell
