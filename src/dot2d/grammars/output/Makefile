
# '?=' gives environment variables (if any) higher priority than definitions in the file
# So, it would be easier to re-define DEBUGFLAGS to be '-g -pg' to enable debugging
DEBUGFLAGS?=-O3

LIBRARIES=../../../lib/libdeckarddata.a
LIBNAME=dotparserc
# 'make' uses CPPFLAGS for preprocessors
CPPFLAGS+=-I. -I../../../include 
CC=g++
CFLAGS+=$(DEBUGFLAGS)
CXX=g++
CXXFLAGS+=$(DEBUGFLAGS)
LDFLAGS+=$(DEBUGFLAGS) -L/usr/local/lib -L. -L../../../lib
LDLIBS+=-l$(LIBNAME) -lantlr3c -ldeckarddata

LIBSRCS=DotCLexer.c DotCParser.c DotTreeCGraph.c
MAINSRCS=DotTreeCTest.c
SRCS=$(LIBSRCS) $(MAINSRCS)
HEADERS:=$(LIBSRCS:.c=.h) DotTreeC.tokens
LIBOBJS:=$(LIBSRCS:.c=.o)
MAINOBJS:=$(MAINSRCS:.c=.o)
OBJS:=$(SRCS:.c=.o)
LIBARC = lib$(LIBNAME).a
EXE = DotTreeCTest

.PHONY: all clean

all: $(OBJS) $(LIBARC) $(EXE)

$(LIBARC): $(LIBOBJS)
	ar -csrv $(LIBARC) $(LIBOBJS)

$(EXE): $(LIBARC) $(MAINOBJS) $(LIBRARIES)
	$(CXX) $(MAINOBJS) $(LDFLAGS) $(LDLIBS) -o $(EXE)

clean:
	rm -f $(OBJS) $(LIBARC) $(EXE)

