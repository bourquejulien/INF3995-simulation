#!/bin/bash

shopt -s globstar

style=file
regex="controllers/**/*.cpp communication/**/*.cpp controllers/**/*.h communication/**/*.h"

echo $regex

while [[ $# -gt 0 ]]; do
  case $1 in
    -f)
      format=true
      shift
      ;;
    --default)
      DEFAULT=NO
      shift
      ;;
    -*|--*)
      echo "Unknown option $1"
      exit 1
      ;;
    *)
      POSITIONAL_ARGS+=("$1")
      shift
      ;;
  esac
done

if [ $format ]
then
    echo "==== Formating files ===="
    clang-format -i --verbose -style=$style $regex
    exit 0
fi

clang-format --dry-run -Werror -style=$style $regex

