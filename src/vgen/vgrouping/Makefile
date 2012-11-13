
DEBUGFLAGS?=-O3

CC=gcc
CXX=g++
CFLAGS+=$(DEBUGFLAGS)
CXXFLAGS+=$(DEBUGFLAGS)
LDFLAGS+=$(DEBUGFLAGS) -lm

EXEFILES=vectorsort dispatchvectors computeranges
SRCFILES=$(addsuffix .c, $(EXEFILES))

all: $(EXEFILES)

#%::%.c
#	${CC} ${CFLAGS} -o $@ $^ -lm

clean:
	rm -f ${EXEFILES} *~ *.o

