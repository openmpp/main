=====================
To build openM++ run:

  docker run .... <image-name> ./build-all
  
  Examples:
  docker run -v $HOME/build:/home/ompp/build ompp-build-centos ./build-all
  docker run -v $HOME/build:/home/ompp/build -e MODEL_DIRS=RiskPaths,IDMM      ompp-build-centos ./build-all
  docker run -v $HOME/build:/home/ompp/build -e OM_BUILD_CONFIGS=RELEASE,DEBUG ompp-build-centos ./build-all
  docker run -v $HOME/build:/home/ompp/build -e OM_MSG_USE=MPI                 ompp-build-centos ./build-all

  Environment variables:
  OM_BUILD_CONFIGS=RELEASE,DEBUG (default: RELEASE,DEBUG for libraries and RELEASE for models)
  OM_MSG_USE=MPI                 (default: EMPTY)
  MODEL_DIRS=modelOne,NewCaseBased,NewTimeBased,NewCaseBased_bilingual,NewTimeBased_bilingual,IDMM,OzProj,OzProjGen,RiskPaths

To build openM++ libraries and omc compiler run:

  docker run .... <image-name> ./build-openm
  
  Environment variables to control "build-openm": OM_BUILD_CONFIGS, OM_MSG_USE

To build models run:

  docker run .... <image-name> ./build-modles
  
  Environment variables to control "build-modles": OM_BUILD_CONFIGS, OM_MSG_USE, MODEL_DIRS

To build openM++ tools run any of:

  docker run .... <image-name> ./build-go   # Go oms web-service and dbcopy utility
  docker run .... <image-name> ./build-r    # openMpp R package
  docker run .... <image-name> ./build-ui   # openM++ UI (alpha)
  
To create openmpp_centos_YYYYMMDD.tar.gz archive:

  docker run .... <image-name> ./build-tar-gz
  
  Environment variables to control "build-tar-gz": OM_MSG_USE, MODEL_DIRS

To open shell command prompt:

  docker run .... -it <image-name> bash
