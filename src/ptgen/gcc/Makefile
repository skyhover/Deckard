CXX= g++ -I../../include
OBJS= lex.yy.o pt_c.tab.o head.o
#TARGET=c_ptgen # this is just a test driver
TARGET=gccptgen.a

all: $(TARGET)

gccptgen.a: $(OBJS)
	ar -qcs $@ $(OBJS)

c_ptgen: $(OBJS) main.o
	$(CXX) -o c_ptgen $(OBJS) main.o

#${TARGET}: ${OBJS} main.o
#	$(CXX) -o ${TARGET} ${OBJS} main.o

lex.yy.cc: c.l pt_c.tab.cc
	flex -olex.yy.cc c.l

pt_c.tab.cc: pt_c.y
	bison -d pt_c.y -o pt_c.tab.cc

head.cc pt_c.y: c.y c.y.head c.y.foot
	./mainc.py c.y

.PHONY: clean
clean:
	rm -f *.o lex.yy.cc pt_c.tab* pt_c.y head.cc $(TARGET)
