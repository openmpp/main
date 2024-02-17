#!/usr/bin/env bash

if [ -z "$OM_ROOT" ] ;
then
  source ../start-model-ui-linux.sh
else
  source "$OM_ROOT"/models/start-model-ui-linux.sh
fi
