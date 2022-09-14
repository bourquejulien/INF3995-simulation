#!/bin/bash

set -m

trap 'exit 0' SIGTERM
  
python3 -m http.server --directory /root/webviz/client $WEBVIZ_PORT &

argos3 -c experiments/$1

