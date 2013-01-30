#!/bin/sh

#
# 
# Copyright (c) 2007-2013, University of California / Singapore Management University
#   Lingxiao Jiang         <lxjiang@ucdavis.edu> <lxjiang@smu.edu.sg>
#   Ghassan Misherghi      <ghassanm@ucdavis.edu>
#   Zhendong Su            <su@ucdavis.edu>
#   Stephane Glondu        <steph@glondu.net>
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the University of California nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
#

# Define the following DEBUGFLAGS to enable debug build
# The default value for DEBUGFLAGS may be conditionally defined in the makefile for each module
#export DEBUGFLAGS="-g -pg"
# This way is similar to enable the following two (depending on the way invoked makefiles are defined): 
#export CFLAGS="-g -pg"
#export CXXFLAGS="-g -pg"

# re-compile parse tree generators
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

# re-compile vector generator and vector grouping code
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

# re-compile code for main entries
make clean
make
errcode=$?
if [ $errcode -ne 0 ]; then
	echo "Error: main make failed. Exit."
	exit $errcode
fi

# re-compile LSH
(
cd ../lsh/ || exit 1
make clean_all
make
errcode=$?
if [ $errcode -ne 0 ]; then
	echo "error: lsh make failed. exit."
	exit $errcode
fi
)

# re-compile additional library code for trees and graphs
(
cd ../lib || exit 1
make clean
make
errcode=$?
if [ $errcode -ne 0 ]; then
	echo "error: lib make failed. exit."
	exit $errcode
fi
)

# re-compile .dot parser generator
(
# assume antlr has been run; otherwise, please manually run antlrworks-1.4.3 in the repository first
cd ../dot2d/grammars/output || exit 1
make clean
make
errcode=$?
if [ $errcode -ne 0 ]; then
	echo "error: dot parser make failed. exit."
	exit $errcode
fi
)

# re-compile main entries for deckard 2.x
(
cd ../dot2d || exit 1
make clean
make
errcode=$?
if [ $errcode -ne 0 ]; then
	echo "error: dot2d make failed. exit."
	exit $errcode
fi
)

