#!/bin/bash
sed -i -E -e "s/random_seed=\".+\"/random_seed=\"$RANDOM\"/" experiments/main_simulation.argos
