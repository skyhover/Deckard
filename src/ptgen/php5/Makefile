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
CXX= g++ -I../../include
OBJS= lex.yy.o pt_zend_language_parser.tab.o head.o
#TARGET=php_ptgen # this is just a test driver
TARGET=phpptgen.a

all: $(TARGET)

phpptgen.a: $(OBJS)
	ar -qcs $@ $(OBJS)

php_ptgen: $(OBJS) main.o
	$(CXX) -o $@ $(OBJS) main.o

#${TARGET}: ${OBJS} main.o
#	$(CXX) -o ${TARGET} ${OBJS} main.o

pt_zend_language_parser.tab.cc: pt_zend_language_parser.y
	bison -d pt_zend_language_parser.y -o pt_zend_language_parser.tab.cc

lex.yy.cc: zend_language_scanner.l pt_zend_language_parser.tab.cc
	flex -i -olex.yy.cc zend_language_scanner.l

head.cc pt_zend_language_parser.y: zend_language_parser.y zend_language_parser.y.head zend_language_parser.y.foot
	./mainphp.py zend_language_parser.y
	sed -i "s/'\"'/'\\\\\"'/" head.cc

.PHONY: clean
clean:
	rm -f *.o lex.yy.cc pt_zend_language_parser.tab* pt_zend_language_parser.y head.cc $(TARGET)
