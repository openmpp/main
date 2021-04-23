#!/bin/bash
#
# Xcode omc project: do bison and flex to make parser and scanner
#
set -x
set -e

if [ ! -d ${BUILD_SRC_DIR} ] ; then mkdir -p ${BUILD_SRC_DIR} ; fi

# search for bison
#
if [ "${OMC_BISON_SEARCH}" = "true" ] || [ "${OMC_BISON_SEARCH}" = "yes" ] || [ "${OMC_BISON_SEARCH}" = "1" ] ;
then

  if [ -z ${PATH##*"bison"*} ] ; then

    echo "Found bison in PATH"
  
  elif [ -n "${OMC_BISON_HOME_PATH}" ] && [ -x "${OMC_BISON_HOME_PATH}/bison" ] ; then

    echo "Found bison at ${OMC_BISON_HOME_PATH}"
    export PATH="${OMC_BISON_HOME_PATH}:${PATH}"
    export LDFLAGS="-L${OMC_BISON_HOME_LDFLAGS} ${LDFLAGS}"
  
  elif [ -n "${OMC_BISON_BREW_x86_PATH}" ] && [ -x "${OMC_BISON_BREW_x86_PATH}/bison" ] ; then

    echo "Found bison at ${OMC_BISON_BREW_x86_PATH}"
    export PATH="${OMC_BISON_BREW_x86_PATH}:${PATH}"
    export LDFLAGS="-L${OMC_BISON_BREW_x86_LDFLAGS} ${LDFLAGS}"

  elif [ -n "${OMC_BISON_BREW_ARM64_PATH}" ] && [ -x "${OMC_BISON_BREW_ARM64_PATH}/bison" ] ; then

    echo "Found bison at ${OMC_BISON_BREW_ARM64_PATH}"
    export PATH="${OMC_BISON_BREW_ARM64_PATH}:${PATH}"
    export LDFLAGS="-L${OMC_BISON_BREW_ARM64_LDFLAGS} ${LDFLAGS}"

  else
    echo "Using default bison"
  fi
fi

#
# do bison and flex
#

bison -b parser -o ${BUILD_SRC_DIR}/parser.cpp parser.y

flex -Cem -o ${BUILD_SRC_DIR}/scanner.cpp scanner.l

# done
