#!/bin/sh

#export CXXFLAGS="-pg -g"
#export CFLAGS="-pg -g"
export CXXFLAGS="-O3"
export CFLAGS="-O3"

(
cd ../ptgen/ || exit 1
make clean
)

(
cd ../vgen/treeTra/ || exit 1
make clean
cd ../vgrouping/ || exit 1
make clean
)

make clean

(
cd ../lsh/ || exit 1
make clean_all
)

(
cd ../lib || exit 1
make clean
)

