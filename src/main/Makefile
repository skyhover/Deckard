TREEHEADER=../include/ptree.h
TREESRC=ptree.cc

CTREEOBJS=ptreeC.o
JTREEOBJS=ptreeJ.o
PHPTREEOBJS=ptreePHP.o

CVGENOBJS=${CTREEOBJS} ../vgen/treeTra/libvgen.a  # TODO: libvgen may also be language dependant, especially the parts that involve "contextual nodes" (e.g., bugfinding, context locating code)
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
     parseTreeMainC parseTreeMainJ parseTreeMainPHP


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

out2html:${COBJS} out2html.C
	${CXX} ${CXXFLAGS} -o $@ out2html.C ${COBJS}

out2xml:${COBJS} out2xml.C
	${CXX} ${CXXFLAGS} -o $@ out2xml.C ${COBJS}
	
parseTreeMainC:${COBJS} ${CHEADERS} parseTreeMain.cc
	${CXX} ${CXXFLAGS} -DCLANG -o $@ parseTreeMain.cc ${COBJS}

parseTreeMainJ:${JOBJS} ${JHEADERS} parseTreeMain.cc
	${CXX} ${CXXFLAGS} -DJAVA -o $@ parseTreeMain.cc ${JOBJS}

parseTreeMainPHP:${PHPOBJS} ${PHPHEADERS} parseTreeMain.cc
	${CXX} ${CXXFLAGS} -DPHP -o $@ parseTreeMain.cc ${PHPOBJS}

.PHONY: clean
clean:
	rm -f *.o ${EXES}

