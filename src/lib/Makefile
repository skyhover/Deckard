
DEBUGFLAGS=-g -pg
LIBRARIES=
LIBNAME=deckarddata
INCLUDES=-I. -I../include 
CC=gcc $(DEBUGFLAGS)
CFLAGS:=${CFLAGS} $(INCLUDES) 
CXX=g++ $(DEBUGFLAGS)
CXXFLAGS:=${CXXFLAGS} $(INCLUDES) 
LDFLAGS:=${LDFLAGS}

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

