#!/bin/sh

#export CXXFLAGS="-pg -g"
#export CFLAGS="-pg -g"
export CXXFLAGS="-O3"
export CFLAGS="-O3"

(
cd ../ptgen
make clean
make
)

(
cd ../vgen/treeTra/
make clean
make
cd ../vgrouping/
make clean
make
)

make clean
make

(
cd ../lsh
make clean
make
)

