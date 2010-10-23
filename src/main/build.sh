#!/bin/sh

#export CXXFLAGS="-pg -g"
#export CFLAGS="-pg -g"
export CXXFLAGS="-O3"
export CFLAGS="-O3"

(
cd ../ptgen/ || exit 1
make clean
make
errcode=$?
if [ $errcode -ne 0 ]; then
	echo "Error: ptgen make failed. Exit."
	exit $errcode
fi
)

(
cd ../vgen/treeTra/ || exit 1
make clean
make
errcode=$?
if [ $errcode -ne 0 ]; then
	echo "Error: vcgen make failed. Exit."
	exit 1
fi
cd ../vgrouping/ || exit 1
make clean
make
errcode=$?
if [ $errcode -ne 0 ]; then
	echo "Error: vgrouping make failed. Exit."
	exit $errcode
fi
)

make clean
make
errcode=$?
if [ $errcode -ne 0 ]; then
	echo "Error: main make failed. Exit."
	exit $errcode
fi

(
cd ../lsh/ || exit 1
make clean_all
make
errcode=$?
if [ $errcode -ne 0 ]; then
	echo "Error: lsh make failed. Exit."
	exit $errcode
fi
)

