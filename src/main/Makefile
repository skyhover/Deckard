
# '?=' gives environment variables (if any) higher priority than definitions in the file
# So, it would be easier to re-define DEBUGFLAGS to be '-g -pg' to enable debugging
DEBUGFLAGS?=-O3

TREEHEADER=../include/ptree.h
TREESRC=ptree.cc

CTREEOBJS=ptreeC.o
JTREEOBJS=ptreeJ.o
PHPTREEOBJS=ptreePHP.o
SOLTREEOBJS=ptreeSOL.o

CVGENOBJS=${CTREEOBJS} ../vgen/treeTra/libvgen.a  # TODO: libvgen may also be language dependent, especially the parts that involve "contextual nodes" (e.g., bugfinding, context locating code)
# A better architecture would make 'vgen' language independent, put all language-dependent code into ptgen/ptree but provide a language-independent interface for accessing tree structures;
# 'bugfinding' (the buggy code scoring algorithm in particular) should also be language-independent, accessing language-dependent tree structures only through a language-independent interface.
JVGENOBJS=${JTREEOBJS} ../vgen/treeTra/libvgen.a
PHPVGENOBJS=${PHPTREEOBJS} ../vgen/treeTra/libvgen.a
SOLVGENOBJS=${SOLTREEOBJS} ../vgen/treeTra/libvgen.a

COBJS=${CVGENOBJS} ../ptgen/gcc/gccptgen.a
JOBJS=${JVGENOBJS} ../ptgen/java/javaptgen.a
PHPOBJS=${PHPVGENOBJS} ../ptgen/php5/phpptgen.a
SOLOBJS=${SOLVGENOBJS} ../ptgen/sol/solidityptgen.a

CHEADERS=../ptgen/gcc/crelevantNodes.h ../ptgen/gcc/catomicNodes.h ../ptgen/gcc/cparentNodes.h ../ptgen/gcc/ccontextualNodes.h
JHEADERS=../ptgen/java/jrelevantNodes.h ../ptgen/java/jatomicNodes.h ../ptgen/java/jparentNodes.h ../ptgen/java/jcontextualNodes.h
PHPHEADERS=../ptgen/php5/phprelevantNodes.h ../ptgen/php5/phpatomicNodes.h ../ptgen/php5/phpparentNodes.h ../ptgen/php5/phpcontextualNodes.h
SOLHEADERS=../ptgen/sol/solrelevantNodes.h ../ptgen/sol/solatomicNodes.h ../ptgen/sol/solparentNodes.h ../ptgen/sol/solcontextualNodes.h

CC=g++
CXX=g++ 
CPPFLAGS+=-I../include -I../vgen/treeTra
CFLAGS+=$(DEBUGFLAGS)
CXXFLAGS+=$(DEBUGFLAGS)
LDFLAGS+=$(DEBUGFLAGS)

EXES=cvecgen jvecgen cbugfilters jbugfilters out2html \
     phpvecgen phpbugfilters out2xml \
     cParseTreeMain jParseTreeMain phpParseTreeMain \
     solvecgen solParseTreeMain


TARGET:${EXES}

#NOTE: this Makefile may not be good for parallel compilation,
#      because intermediate files may be overwritten.

${CTREEOBJS}:${TREESRC} ${TREEHEADER}
	$(CXX) -o $@ $(CXXFLAGS) $(CPPFLAGS) -c -DCLANG ${TREESRC}

${JTREEOBJS}:${TREESRC} ${TREEHEADER}
	$(CXX) -o $@ $(CXXFLAGS) $(CPPFLAGS) -c -DJAVA ${TREESRC}

${PHPTREEOBJS}:${TREESRC} ${TREEHEADER}
	$(CXX) -o $@ $(CXXFLAGS) $(CPPFLAGS) -c -DPHP ${TREESRC}

${SOLTREEOBJS}:${TREESRC} ${TREEHEADER}
	$(CXX) -o $@ $(CXXFLAGS) $(CPPFLAGS) -c -DSOLIDITY ${TREESRC}

cvecgen:${COBJS} ${CHEADERS} main.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -DCLANG main.cc
	$(CXX) -o $@ $(LDFLAGS) main.o ${COBJS}

jvecgen:${JOBJS} ${JHEADERS} main.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -DJAVA main.cc
	$(CXX) -o $@ $(LDFLAGS) main.o ${JOBJS}

phpvecgen:${PHPOBJS} ${PHPHEADERS} main.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -DPHP main.cc
	$(CXX) -o $@ $(LDFLAGS) main.o ${PHPOBJS}

solvecgen:${SOLOBJS} ${SOLHEADERS} main.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -DSOLIDITY main.cc
	$(CXX) -o $@ $(LDFLAGS) main.o ${SOLOBJS}

cbugfilters:${COBJS} ${CHEADERS} bugmain.cc
	${CXX} ${CXXFLAGS} $(CPPFLAGS) -c -DCLANG bugmain.cc
	${CXX} ${LDFLAGS} -o $@ bugmain.o ${COBJS}

jbugfilters:${JOBJS} ${JHEADERS} bugmain.cc
	${CXX} ${CXXFLAGS} $(CPPFLAGS) -c -DJAVA bugmain.cc
	${CXX} ${LDFLAGS} -o $@ bugmain.o ${JOBJS}

phpbugfilters:${PHPOBJS} ${PHPHEADERS} bugmain.cc
	${CXX} ${CXXFLAGS} $(CPPFLAGS) -c -DPHP bugmain.cc
	${CXX} ${LDFLAGS} -o $@ bugmain.o ${PHPOBJS}

out2html:${COBJS} out2html.C  # don't matter to use COBJS or others
	${CXX} ${CXXFLAGS} $(CPPFLAGS) -o $@ out2html.C ${COBJS}

out2xml:${COBJS} out2xml.C  # don't matter to use COBJS or others
	${CXX} ${CXXFLAGS} $(CPPFLAGS) -o $@ out2xml.C ${COBJS}
	
cParseTreeMain:${COBJS} ${CHEADERS} parseTreeMain.cc
	${CXX} ${CXXFLAGS} $(CPPFLAGS) -DCLANG -o $@ parseTreeMain.cc ${COBJS}

jParseTreeMain:${JOBJS} ${JHEADERS} parseTreeMain.cc
	${CXX} ${CXXFLAGS} $(CPPFLAGS) -DJAVA -o $@ parseTreeMain.cc ${JOBJS}

phpParseTreeMain:${PHPOBJS} ${PHPHEADERS} parseTreeMain.cc
	${CXX} ${CXXFLAGS} $(CPPFLAGS) -DPHP -o $@ parseTreeMain.cc ${PHPOBJS}

solParseTreeMain:${SOLOBJS} ${SOLHEADERS} parseTreeMain.cc
	${CXX} ${CXXFLAGS} $(CPPFLAGS) -DSOLIDITY -o $@ parseTreeMain.cc ${SOLOBJS}

.PHONY: clean
clean:
	rm -f *.o ${EXES}

