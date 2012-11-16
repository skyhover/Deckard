
# '?=' gives environment variables (if any) higher priority than definitions in the file
# So, it would be easier to re-define DEBUGFLAGS to be '-g -pg' to enable debugging
DEBUGFLAGS?=-O3

LIBRARIES=./grammars/output/libdotparserc.a ../vgen/treeTra/libvgen.a ../lib/libdeckarddata.a
CPPFLAGS+=-I. -I../include -I../vgen/treeTra -I./grammars/output
CC=g++
CFLAGS+=$(DEBUGFLAGS)
CXX=g++
CXXFLAGS+=$(DEBUGFLAGS)
# NOTE: 'make' uses the command "$(CC) $(LDFLAGS) x.o $(LOADLIBES) $(LDLIBS)" when linking
# pay attention to the ordering of the linked libraries. E.g., -lm should come after x.o
# This behavior is compiler-depedent; some newer versions of g++ (not all versions of gcc) work well no matter what the ordering is.
LDFLAGS+=$(DEBUGFLAGS) -L/usr/local/lib -L. -L../lib -L./grammars/output -L../vgen/treeTra
LDLIBS+=-ldotparserc -lvgen -ldeckarddata -lantlr3c

LIBSRCS=ptree.cc node-counter.cc
MAINSRCS=dotvgen.cc
SRCS=$(MAINSRCS) $(LIBSRCS)
HEADERS:=$(LIBSRCS:.cc=.h)
LIBOBJS:=$(LIBSRCS:.cc=.o)
MAINOBJS:=$(MAINSRCS:.cc=.o)
OBJS:=$(MAINOBJS) $(LIBOBJS)
EXE = dotvgen

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJS) $(LIBRARIES)
	$(CXX) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $(EXE)

clean:
	rm -f $(OBJS) $(EXE)

