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

#-p -pg for debugging (need rebuild TREEOBJS with debugging options too); -O3 for release
DEBUGFLAGS?=-O3

CPPFLAGS+=-I../../include
CXX=g++
CXXFLAGS+=$(DEBUGFLAGS)
LDFLAGS+=$(DEBUGFLAGS)
OBJS= lex.yy.o pt_solidity.tab.o head.o
TREEOBJS= ../../main/ptreeSOL.o  ../../vgen/treeTra/libvgen.a
TARGET=solidityptgen.a

all: $(TARGET)

solidityptgen.a: $(OBJS)
	ar -csrv $@ $(OBJS)

# generate a test driver for the parser:
test: solidity_ptgen

solidity_ptgen: $(OBJS) $(TREEOBJS) main.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o solidity_ptgen main.cc $(OBJS) $(TREEOBJS)

#main.o: main.cc
#	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o main.o main.cc

lex.yy.cc: solidity.l pt_solidity.tab.cc
	flex -olex.yy.cc solidity.l

# --debug or -t to enable tracing in bison-generated parser
pt_solidity.tab.cc: pt_solidity.y
	bison -d pt_solidity.y -o pt_solidity.tab.cc -v -g

head.cc pt_solidity.y: solidity.y solidity.y.head solidity.y.foot
	./mainsol.py solidity.y

.PHONY: clean
clean:
	rm -f *.o lex.yy.cc pt_solidity.* head.cc $(TARGET)

