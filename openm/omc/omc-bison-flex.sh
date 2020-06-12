#!/bin/bash
#
# Xcode omc project: do bison and flex to make parser and scanner
#
set -x
set -e

if [ ! -d ${BUILD_SRC_DIR} ] ; then mkdir -p ${BUILD_SRC_DIR} ; fi

#
if [ -n "$OMC_BISON_PATH" ]; then
  export PATH="${OMC_BISON_PATH}:${PATH}"
fi

if [ -n "$OMC_BISON_LDFLAGS" ]; then
  export LDFLAGS="-L${OMC_BISON_LDFLAGS} ${LDFLAGS}"
fi

bison -b parser -o ${BUILD_SRC_DIR}/parser.cpp parser.y

#
flex -Cem -o ${BUILD_SRC_DIR}/scanner.cpp scanner.l

# done
