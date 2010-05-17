#!/bin/sh

#export CXXFLAGS="-pg -g"
#export CFLAGS="-pg -g"
export CXXFLAGS="-O3"
export CFLAGS="-O3"

pushd .

cd ../ptgen/gcc
make clean
make

popd

pushd .

cd ../ptgen/java
make clean
make

popd

pushd .
cd ../ptgen/php5
make clean
make
popd

pushd .

cd ../vgen/treeTra/
make clean
make
cd ../sort/
make clean
make

popd

make clean
make

pushd .

cd bugfinding
make clean
make

popd

pushd .

cd ../lsh
make clean
make

popd

