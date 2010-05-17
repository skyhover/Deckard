CC=gcc #-pg -g
CFLAGS:=${CFLAGS} -O3
CXX=g++ #-pg -g
CXXFLAGS:=${CXXFLAGS} -I../../include -I../../vgen/treeTra -O3

headers = ../../include/ptree.h ../../vgen/treeTra/vgen-config.h ../../vgen/treeTra/token-counter.h
srcs = bugmain.C clone-cluster.C
objs:= $(srcs:.C=.o)
objs:= $(objs:.c=.o)
makefiles:= $(srcs:.C=.d)
makefiles:= $(makefiles:.c=.d)
exes = cbugfilters jbugfilters
linkobjs = bugmain.o
libs = ../ptree.o ../../vgen/treeTra/libvgen.a
clibs = $(libs) ../../ptgen/gcc/gccptgen.a
jlibs = $(libs) ../../ptgen/java/gccptgen.a

.PHONY: all clean cleanmakefiles

all: $(makefiles) $(exes)

cbugfilters:$(clibs) $(srcs) $(headers)
	${CXX} ${CXXFLAGS} -c bugmain.C
	${CXX} ${CXXFLAGS} -o $@ $(linkobjs) $(clibs)

jbugfilters:$(jlibs) $(srcs) $(headers)
	${CXX} ${CXXFLAGS} -c -DJAVA bugmain.C
	${CXX} ${CXXFLAGS} -o $@ $(linkobjs) $(jlibs)



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

include $(makefiles)

clean:
	rm -f $(exes) *~ *.o *.d *.a core* gmon*

cleanmakefiles:
	rm -f *.d
