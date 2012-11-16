
# '?=' gives environment variables (if any) higher priority than definitions in the file
# So, it would be easier to re-define DEBUGFLAGS to be '-g -pg' to enable debugging
DEBUGFLAGS?=-O3

LIBNAME=dotparserc
LIBARC=lib$(LIBNAME).a
LIBRARIES=../../../lib/libdeckarddata.a
# 'make' uses CPPFLAGS for preprocessors
CPPFLAGS+=-I. -I../../../include 
CC=g++
CFLAGS+=$(DEBUGFLAGS)
CXX=g++
CXXFLAGS+=$(DEBUGFLAGS)
LDFLAGS+=$(DEBUGFLAGS) -L/usr/local/lib -L. -L../../../lib
LDLIBS+=-l$(LIBNAME) -ldeckarddata -lantlr3c

LIBSRCS=DotCLexer.c DotCParser.c DotTreeCGraph.c
MAINSRCS=DotTreeCTest.c
SRCS=$(MAINSRCS) $(LIBSRCS)
HEADERS:=$(LIBSRCS:.c=.h) DotTreeC.tokens
LIBOBJS:=$(LIBSRCS:.c=.o)
MAINOBJS:=$(MAINSRCS:.c=.o)
OBJS:=$(SRCS:.c=.o)
EXE = DotTreeCTest

.PHONY: all clean

all: $(LIBARC) $(EXE)

$(LIBARC): $(LIBOBJS)
	ar -csrv $(LIBARC) $(LIBOBJS)

$(EXE): $(LIBARC) $(MAINOBJS) $(LIBRARIES)
	$(CXX) $(MAINOBJS) $(LDFLAGS) $(LDLIBS) -o $(EXE)

clean:
	rm -f $(OBJS) $(LIBARC) $(EXE)

