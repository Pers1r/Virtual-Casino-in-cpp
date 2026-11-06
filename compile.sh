#!/bin/bash

BUILD_TYPE="Debug"

if [ "$1" == "r" ]; then
  echo "=== RELEASE MODE ==="
  BUILD_TYPE="Release"
  sleep 1
elif [ "$1" == "d" ]; then
  echo "=== DEBUG MODE ==="
  BUILD_TYPE="Debug"
  sleep 1
elif [ "$1" == "c" ]; then
  echo "=== REBUILD BUILD DIR ==="
  sleep 2
  rm -rf build
fi

mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" .. && make && ./bin/VCasino

if [ "$1" == "v" ] || [ "$2" == "v" ]; then
  echo "=== VALGRIND Check ==="
  sleep 1
  valgrind ./bin/VCasino
fi
