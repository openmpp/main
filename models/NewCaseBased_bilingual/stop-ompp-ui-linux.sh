#!/usr/bin/env bash

if [ -z "$OM_ROOT" ] ;
then
  source ../stop-model-ui-linux.sh
else
  source "$OM_ROOT"/models/stop-model-ui-linux.sh
fi
