
DEBUGFLAGS=-g -pg
LIBRARIES=../../../lib/libdeckarddata.a
LIBNAME=dotparserc
INCLUDES=-I. -I../../../include 
CC=g++ $(DEBUGFLAGS)
CFLAGS:=${CFLAGS} $(INCLUDES) 
CXX=g++ $(DEBUGFLAGS)
CXXFLAGS:=${CXXFLAGS} $(INCLUDES) 
LDFLAGS:=${LDFLAGS} -L/usr/local/lib -L. -L../../../lib -l$(LIBNAME) -lantlr3c -ldeckarddata

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
	$(CXX) $(MAINOBJS) $(LDFLAGS) -o $(EXE)

clean:
	rm -f $(OBJS) $(LIBARC) $(EXE)

