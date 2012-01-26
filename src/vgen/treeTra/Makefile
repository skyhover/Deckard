#
# 
# Copyright (c) 2007-2012,
#   Lingxiao Jiang         <lxjiang@ucdavis.edu>
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
CC=gcc #-g -pg
CFLAGS:=${CFLAGS} -O3
CXX=g++ #-g -pg
CXXFLAGS:=${CXXFLAGS} -O3

treeheaders = ../../include/ptree.h
traversalsrc = tree-traversal.C tra-gen.C vgen-utils.c tree-vector.C vgen-config.C \
	token-counter.C sq-tree.C node-vec-gen.C vector-output.C vector-merger.C tree-accessor.C \
	token-tree-map.C clone-context-php.C
traversalobjs:= $(traversalsrc:.C=.o)
traversalobjs:= $(traversalobjs:.c=.o)
traversalmakefiles:= $(traversalsrc:.C=.d)
traversalmakefiles:= $(traversalmakefiles:.c=.d)
traversallib = libvgen.a

.PHONY: all clean cleanmakefiles

all: $(traversalmakefiles) $(traversalobjs) $(traversallib)

$(traversallib): $(traversalobjs)
	ar -qcs libvgen.a $(traversalobjs)

# currently, this Makefile has problems if *.h (not *.c/*.C) is
# changed to include (other) different header files. At such cases, we
# need "rm -f *.d" first (make cleanmakefiles doesn't work, why?),
# then make.

%.d:%.C
	$(CXX) -MM $(CPPFLAGS) $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.d:%.c
	$(CC) -MM $(CPPFLAGS) $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

include $(traversalmakefiles)

clean:
	rm -f *~ *.o *.d *.a core* gmon*

cleanmakefiles:
	rm -f *.d
