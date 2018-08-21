
DEBUGFLAGS?=-O3

CC=gcc
CXX=g++
CFLAGS+=$(DEBUGFLAGS)
CXXFLAGS+=$(DEBUGFLAGS)
# NOTE: 'make' uses the command "$(CC) $(LDFLAGS) x.o $(LOADLIBES) $(LDLIBS)" when linking
# pay attention to the ordering of the linked libraries. E.g., -lm should come after x.o
# This behavior is compiler-depedent; some newer versions of g++ (not all versions of gcc) work well no matter what the ordering is.
LDFLAGS+=$(DEBUGFLAGS)
LDLIBS+=-lm

EXEFILES=vectorsort dispatchvectors computeranges
SRCFILES=$(addsuffix .c, $(EXEFILES))

all: $(EXEFILES)

#%::%.c
#	${CC} ${CFLAGS} -o $@ $^ -lm

clean:
	rm -f ${EXEFILES} *~ *.o

