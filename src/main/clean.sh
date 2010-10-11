#!/bin/sh

#export CXXFLAGS="-pg -g"
#export CFLAGS="-pg -g"
export CXXFLAGS="-O3"
export CFLAGS="-O3"

(
cd ../ptgen
make clean
)

(
cd ../vgen/treeTra/
make clean
cd ../sort/
make clean
)

make clean

(
cd ../lsh
make clean
)
