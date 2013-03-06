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

DEBUGFLAGS?=-O3

CPPFLAGS+=-I../../include
CXX=g++
CXXFLAGS+=$(DEBUGFLAGS)
LDFLAGS+=$(DEBUGFLAGS)
OBJS= lex.yy.o pt_c.tab.o head.o
#TARGET=c_ptgen # this is just a test driver
TARGET=gccptgen.a

all: $(TARGET)

gccptgen.a: $(OBJS)
	ar -csrv $@ $(OBJS)

c_ptgen: $(OBJS) main.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o c_ptgen $(OBJS) main.o

#${TARGET}: ${OBJS} main.o
#	$(CXX) -o ${TARGET} ${OBJS} main.o

lex.yy.cc: c.l pt_c.tab.cc
	flex -olex.yy.cc c.l

pt_c.tab.cc: pt_c.y
	bison -d pt_c.y -o pt_c.tab.cc

head.cc pt_c.y: c.y c.y.head c.y.foot
	./mainc.py c.y

.PHONY: clean
clean:
	rm -f *.o lex.yy.cc pt_c.tab* pt_c.y head.cc $(TARGET)

