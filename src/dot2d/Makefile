
# '?=' gives environment variables (if any) higher priority than definitions in the file
# So, it would be easier to re-define DEBUGFLAGS to be '-g -pg' to enable debugging
DEBUGFLAGS?=-O3

LIBRARIES=../lib/libdeckarddata.a
LIBNAME=dotparserc
CPPFLAGS+=-I. -I../include -I../vgen/treeTra -I./grammars/output
CC=g++
CFLAGS+=$(DEBUGFLAGS)
CXX=g++
CXXFLAGS+=$(DEBUGFLAGS)
LDFLAGS+=$(DEBUGFLAGS) -L/usr/local/lib -L. -L../lib -L./grammars/output -L../vgen/treeTra -l$(LIBNAME) -lantlr3c -ldeckarddata -lvgen

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

