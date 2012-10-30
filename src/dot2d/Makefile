
DEBUGFLAGS=-g -pg
LIBRARIES=../lib/libdeckarddata.a
LIBNAME=dotparserc
INCLUDES=-I. -I../include -I../vgen/treeTra -I./grammars/output
CC=g++ $(DEBUGFLAGS)
CFLAGS:=${CFLAGS} $(INCLUDES) 
CXX=g++ $(DEBUGFLAGS)
CXXFLAGS:=${CXXFLAGS} $(INCLUDES) 
LDFLAGS:=${LDFLAGS} -L/usr/local/lib -L. -L../lib -L./grammars/output -L../vgen/treeTra -l$(LIBNAME) -lantlr3c -ldeckarddata -lvgen

LIBSRCS=ptree.cc node-counter.cc
MAINSRCS=dotvgen.cc
SRCS=$(LIBSRCS) $(MAINSRCS)
HEADERS:=$(LIBSRCS:.cc=.h)
LIBOBJS:=$(LIBSRCS:.cc=.o)
MAINOBJS:=$(MAINSRCS:.cc=.o)
OBJS:=$(MAINOBJS) $(LIBOBJS)
EXE = dotvgen

.PHONY: all clean

all: $(OBJS) $(EXE)

$(EXE): $(OBJS) $(LIBRARIES)
	$(CXX) $(OBJS) $(LDFLAGS) -o $(EXE)

clean:
	rm -f $(OBJS) $(EXE)

