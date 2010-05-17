VGENS=ptree.o ../vgen/treeTra/libvgen.a
COBJS=${VGENS} ../ptgen/gcc/gccptgen.a
JOBJS=${VGENS} ../ptgen/java/gccptgen.a
PHPOBJS=${VGENS} ../ptgen/php5/gccptgen.a
PHPHEADERS=../ptgen/php5/phprelevantNodes.h ../ptgen/php5/phpatomicNodes.h ../ptgen/php5/phpparentNodes.h
CXX= g++ -I../include -I../vgen/treeTra # -g
CXXFLAGS= -O3
EXES=cvecgen jvecgen cbugfilters jbugfilters out2html \
     phpvecgen phpbugfilters out2xml


TARGET:${EXES}

#NOTE: this Makefile may not be good for parallel compilation,
#      because intermediate files may be overwritten.
cvecgen:${COBJS} crelevantNodes.h main.cc
	$(CXX) $(CXXFLAGS) -c -DCLANG main.cc
	$(CXX) -o $@ $(CXXFLAGS) main.o ${COBJS}

jvecgen:${JOBJS} jrelevantNodes.h main.cc
	$(CXX) $(CXXFLAGS) -c -DJAVA main.cc
	$(CXX) -o $@ $(CXXFLAGS) main.o ${JOBJS}

phpvecgen:${PHPOBJS} ${PHPHEADERS} main.cc
	$(CXX) $(CXXFLAGS) -c -DPHP main.cc
	$(CXX) -o $@ $(CXXFLAGS) main.o ${PHPOBJS}

cbugfilters:${COBJS} ccontextualNodes.h bugmain.cc
	${CXX} ${CXXFLAGS} -c -DCLANG bugmain.cc
	${CXX} ${CXXFLAGS} -o $@ bugmain.o ${COBJS}

jbugfilters:${COBJS} jcontextualNodes.h bugmain.cc
	${CXX} ${CXXFLAGS} -c -DJAVA bugmain.cc
	${CXX} ${CXXFLAGS} -o $@ bugmain.o ${JOBJS}

phpbugfilters:${COBJS} ../ptgen/php5/phpcontextualNodes.h bugmain.cc
	${CXX} ${CXXFLAGS} -c -DPHP bugmain.cc
	${CXX} ${CXXFLAGS} -o $@ bugmain.o ${PHPOBJS}

out2html:${COBJS} out2html.C
	${CXX} ${CXXFLAGS} -o $@ out2html.C ${COBJS}

out2xml:${COBJS} out2xml.C
	${CXX} ${CXXFLAGS} -o $@ out2xml.C ${COBJS}
	
.PHONY: clean
clean:
	rm -f *.o ${EXES}

