#
# 
# Copyright (c) 2007-2018, University of California / Singapore Management University
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

# '?=' gives environment variables (if any) higher priority than definitions in the file
# So, it would be easier to re-define DEBUGFLAGS to be '-g -pg' to enable debugging
DEBUGFLAGS?=-O3

LIBNAME=dotparserc
LIBARC=lib$(LIBNAME).a
LIBRARIES=../../../lib/libdeckarddata.a
# 'make' uses CPPFLAGS for preprocessors
CPPFLAGS+=-I. -I../../../include 
CC=g++
CFLAGS+=$(DEBUGFLAGS)
CXX=g++
CXXFLAGS+=$(DEBUGFLAGS)
LDFLAGS+=$(DEBUGFLAGS) -L/usr/local/lib -L. -L../../../lib
LDLIBS+=-l$(LIBNAME) -ldeckarddata -lantlr3c

LIBSRCS=DotCLexer.c DotCParser.c DotTreeCGraph.c
MAINSRCS=DotTreeCTest.c
SRCS=$(MAINSRCS) $(LIBSRCS)
HEADERS:=$(LIBSRCS:.c=.h) DotTreeC.tokens
LIBOBJS:=$(LIBSRCS:.c=.o)
MAINOBJS:=$(MAINSRCS:.c=.o)
OBJS:=$(SRCS:.c=.o)
EXE = DotTreeCTest

.PHONY: all clean

all: $(LIBARC) $(EXE)

$(LIBARC): $(LIBOBJS)
	ar -csrv $(LIBARC) $(LIBOBJS)

$(EXE): $(LIBARC) $(MAINOBJS) $(LIBRARIES)
	$(CXX) $(MAINOBJS) $(LDFLAGS) $(LDLIBS) -o $(EXE)

clean:
	rm -f $(OBJS) $(LIBARC) $(EXE)

