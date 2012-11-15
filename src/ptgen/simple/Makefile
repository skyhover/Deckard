# This is a testing directory; may have a lot of errors in the grammar files

DEBUGFLAGS?=-g -pg

CPPFLAGS+=-I../../include
CXX=g++
CXXFLAGS+=$(DEBUGFLAGS)
LDFLAGS+=$(DEBUGFLAGS)
OBJS= lex.yy.o pt_c.tab.o main.o head.o
TARGET= c_ptgen

TARGET: ${OBJS}
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o ${TARGET} ${OBJS}

lex.yy.cc: c.l pt_c.tab.cc
	flex -olex.yy.cc c.l

pt_c.tab.cc: pt_c.y
	bison -d pt_c.y -o pt_c.tab.cc

pt_c.y head.cc: c.y
	../gcc/mainc.py c.y

.PHONY: clean
clean:
	rm -f *.o lex.yy.cc pt_c.tab* pt_c.y head.cc $(TARGET)

