CXXFLAGS= -I../../include -g
OBJS= lex.yy.o pt_c.tab.o main.o head.o
TARGET= c_ptgen

TARGET: ${OBJS}
	g++ -o ${TARGET} ${OBJS}

lex.yy.cc: c.l pt_c.tab.cc
	flex -olex.yy.cc c.l
pt_c.tab.cc: pt_c.y
	bison -d pt_c.y -o pt_c.tab.cc

pt_c.y head.cc: c.y
	../mainc.py c.y


.PHONY: clean
clean:
	rm -f *.o lex.yy.cc pt_c.tab.cc pt_c.y
