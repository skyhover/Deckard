CXX= g++ -I../../include
OBJS= lex.yy.o pt_j.tab.o head.o
#TARGET=j_ptgen # this is just a test driver
TARGET=javaptgen.a

all: $(TARGET)

javaptgen.a: $(OBJS)
	ar -qcs $@ $(OBJS)

j_ptgen: $(OBJS) main.o
	$(CXX) -o $@ $(OBJS) main.o

#${TARGET}: ${OBJS} main.o
#	$(CXX) -o ${TARGET} ${OBJS} main.o

lex.yy.cc: j.l pt_j.tab.cc
	flex -olex.yy.cc j.l

pt_j.tab.cc: pt_j.y
	bison -d pt_j.y -o pt_j.tab.cc

head.cc pt_j.y: j.y j.y.head j.y.foot
	./mainj.py j.y

.PHONY: clean
clean:
	rm -f *.o lex.yy.cc pt_j.tab* pt_j.y head.cc $(TARGET)
