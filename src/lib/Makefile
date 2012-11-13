
DEBUGFLAGS= #-g -pg
LIBRARIES=
LIBNAME=deckarddata
INCLUDES=-I. -I../include 
CC=g++
CFLAGS:=$(DEBUGFLAGS) ${CFLAGS} $(INCLUDES) 
CXX=g++
CXXFLAGS:=$(DEBUGFLAGS) ${CXXFLAGS} $(INCLUDES) 
LDFLAGS:=$(DEBUGFLAGS) ${LDFLAGS}

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

