#
# 
# Copyright (c) 2007-2010,
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
TREEHEADER=../include/ptree.h
TREESRC=ptree.cc

CTREEOBJS=ptreeC.o
JTREEOBJS=ptreeJ.o
PHPTREEOBJS=ptreePHP.o

CVGENOBJS=${CTREEOBJS} ../vgen/treeTra/libvgen.a  # TODO: libvgen may also be language dependent, especially the parts that involve "contextual nodes" (e.g., bugfinding, context locating code)
# A better architecture would make 'vgen' language independent, put all language-dependent code into ptgen/ptree but provide a language-independent interface for accessing tree structures;
# 'bugfinding' (the buggy code scoring algorithm in particular) should also be language-independent, accessing language-dependent tree structures only through a language-independent interface.
JVGENOBJS=${JTREEOBJS} ../vgen/treeTra/libvgen.a
PHPVGENOBJS=${PHPTREEOBJS} ../vgen/treeTra/libvgen.a

COBJS=${CVGENOBJS} ../ptgen/gcc/gccptgen.a
JOBJS=${JVGENOBJS} ../ptgen/java/javaptgen.a
PHPOBJS=${PHPVGENOBJS} ../ptgen/php5/phpptgen.a

CHEADERS=../ptgen/gcc/crelevantNodes.h ../ptgen/gcc/catomicNodes.h ../ptgen/gcc/cparentNodes.h ../ptgen/gcc/ccontextualNodes.h
JHEADERS=../ptgen/java/jrelevantNodes.h ../ptgen/java/jatomicNodes.h ../ptgen/java/jparentNodes.h ../ptgen/java/jcontextualNodes.h
PHPHEADERS=../ptgen/php5/phprelevantNodes.h ../ptgen/php5/phpatomicNodes.h ../ptgen/php5/phpparentNodes.h ../ptgen/php5/phpcontextualNodes.h

CXX= g++ -I../include -I../vgen/treeTra # -g
CXXFLAGS= -O3

EXES=cvecgen jvecgen cbugfilters jbugfilters out2html \
     phpvecgen phpbugfilters out2xml \
     cParseTreeMain jParseTreeMain phpParseTreeMain


TARGET:${EXES}

#NOTE: this Makefile may not be good for parallel compilation,
#      because intermediate files may be overwritten.

${CTREEOBJS}:${TREESRC} ${TREEHEADER}
	$(CXX) -o $@ $(CXXFLAGS) -c -DCLANG ${TREESRC}

${JTREEOBJS}:${TREESRC} ${TREEHEADER}
	$(CXX) -o $@ $(CXXFLAGS) -c -DJAVA ${TREESRC}

${PHPTREEOBJS}:${TREESRC} ${TREEHEADER}
	$(CXX) -o $@ $(CXXFLAGS) -c -DPHP ${TREESRC}

cvecgen:${COBJS} ${CHEADERS} main.cc
	$(CXX) $(CXXFLAGS) -c -DCLANG main.cc
	$(CXX) -o $@ $(CXXFLAGS) main.o ${COBJS}

jvecgen:${JOBJS} ${JHEADERS} main.cc
	$(CXX) $(CXXFLAGS) -c -DJAVA main.cc
	$(CXX) -o $@ $(CXXFLAGS) main.o ${JOBJS}

phpvecgen:${PHPOBJS} ${PHPHEADERS} main.cc
	$(CXX) $(CXXFLAGS) -c -DPHP main.cc
	$(CXX) -o $@ $(CXXFLAGS) main.o ${PHPOBJS}

cbugfilters:${COBJS} ${CHEADERS} bugmain.cc
	${CXX} ${CXXFLAGS} -c -DCLANG bugmain.cc
	${CXX} ${CXXFLAGS} -o $@ bugmain.o ${COBJS}

jbugfilters:${JOBJS} ${JHEADERS} bugmain.cc
	${CXX} ${CXXFLAGS} -c -DJAVA bugmain.cc
	${CXX} ${CXXFLAGS} -o $@ bugmain.o ${JOBJS}

phpbugfilters:${PHPOBJS} ${PHPHEADERS} bugmain.cc
	${CXX} ${CXXFLAGS} -c -DPHP bugmain.cc
	${CXX} ${CXXFLAGS} -o $@ bugmain.o ${PHPOBJS}

out2html:${COBJS} out2html.C  # don't matter to use COBJS or others
	${CXX} ${CXXFLAGS} -o $@ out2html.C ${COBJS}

out2xml:${COBJS} out2xml.C  # don't matter to use COBJS or others
	${CXX} ${CXXFLAGS} -o $@ out2xml.C ${COBJS}
	
cParseTreeMain:${COBJS} ${CHEADERS} parseTreeMain.cc
	${CXX} ${CXXFLAGS} -DCLANG -o $@ parseTreeMain.cc ${COBJS}

jParseTreeMain:${JOBJS} ${JHEADERS} parseTreeMain.cc
	${CXX} ${CXXFLAGS} -DJAVA -o $@ parseTreeMain.cc ${JOBJS}

phpParseTreeMain:${PHPOBJS} ${PHPHEADERS} parseTreeMain.cc
	${CXX} ${CXXFLAGS} -DPHP -o $@ parseTreeMain.cc ${PHPOBJS}

.PHONY: clean
clean:
	rm -f *.o ${EXES}

