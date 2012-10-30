CC=gcc
CFLAGS:=${CFLAGS}
CXX=g++
CXXFLAGS:=${CXXFLAGS}

treeheaders = ../../include/ptree.h
traversalsrc = tree-traversal.C tra-gen.C vgen-utils.c tree-vector.C vgen-config.C \
	token-counter.C sq-tree.C node-vec-gen.C vector-output.C vector-merger.C tree-accessor.C \
	token-tree-map.C clone-context-php.C
traversalobjs:= $(traversalsrc:.C=.o)
traversalobjs:= $(traversalobjs:.c=.o)
traversalmakefiles:= $(traversalsrc:.C=.d)
traversalmakefiles:= $(traversalmakefiles:.c=.d)
traversallib = libvgen.a

.PHONY: all clean cleanmakefiles

all: $(traversalmakefiles) $(traversalobjs) $(traversallib)

$(traversallib): $(traversalobjs)
	ar -rcsv libvgen.a $(traversalobjs)

# currently, this Makefile has problems if *.h (not *.c/*.C) is
# changed to include (other) different header files. At such cases, we
# need "rm -f *.d" first (make cleanmakefiles doesn't work, why?),
# then make.

%.d:%.C
	$(CXX) -MM $(CPPFLAGS) $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.d:%.c
	$(CC) -MM $(CPPFLAGS) $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

include $(traversalmakefiles)

clean:
	rm -f *~ *.o *.d *.a core* gmon*

cleanmakefiles:
	rm -f *.d
