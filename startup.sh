#!/bin/bash

set -m
trap 'kill $(jobs -p); printf "\nExiting\n"; exit' SIGINT SIGTERM

bash randomize_seed.sh

python3 -m http.server --directory /root/client $WEBVIZ_PORT &
argos3 -c experiments/$1 &

while true
do
  sleep 1
done
