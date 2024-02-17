#!/usr/bin/env bash

if [ -z "$OM_ROOT" ] ;
then
  source ../stop-model-ui-mac.sh
else
  source "$OM_ROOT"/models/stop-model-ui-mac.sh
fi
