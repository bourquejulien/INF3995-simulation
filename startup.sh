#!/bin/bash

set -m
trap 'kill $(jobs -p); echo "Received sigint: exiting"; exit' int

python3 -m http.server --directory /root/webviz/client $WEBVIZ_PORT &
argos3 -c experiments/$1 &

while true
do
  sleep 1
done

