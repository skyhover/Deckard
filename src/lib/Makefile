
# '?=' gives environment variables (if any) higher priority than definitions in the file
# So, it would be easier to re-define DEBUGFLAGS to be '-g -pg' to enable debugging
DEBUGFLAGS?=-O3

LIBRARIES=
LIBNAME=deckarddata
CPPFLAGS+=-I. -I../include 
CC=g++
CFLAGS+=$(DEBUGFLAGS)
CXX=g++
CXXFLAGS+=$(DEBUGFLAGS)
LDFLAGS+=$(DEBUGFLAGS)

LIBSRCS=namemap.cpp graph.cpp graphslicer.cpp graphptreemap.cpp utils.cpp
SRCS=$(LIBSRCS)
HEADERS:=$(SRCS:.cpp=.h)
LIBOBJS:=$(LIBSRCS:.cpp=.o)
OBJS:=$(SRCS:.cpp=.o)
LIBARC = lib$(LIBNAME).a

.PHONY: all clean

all: $(OBJS) $(LIBARC)

$(LIBARC): $(LIBOBJS)
	ar -csrv $(LIBARC) $(LIBOBJS)

clean:
	rm -f $(OBJS) $(LIBARC)

