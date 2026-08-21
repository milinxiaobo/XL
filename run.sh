#!/bin/bash
export PATH=/opt/cmake-4.4.2-linux-x86_64/bin:/opt/ninja-linux:$PATH

rm -rf build
mkdir -p build

cmake -G Ninja -B build -S .
cmake --build build

./build/xl
