CC=gcc
CFLAGS:=${CFLAGS} -O3
EXEFILES=vectorsort dispatchvectors computeranges
SRCFILES=$(addsuffix .c, $(EXEFILES))

all: $(EXEFILES)

%::%.c
	${CC} ${CFLAGS} -o $@ $^ -lm

clean:
	rm ${EXEFILES} *~ *.o

