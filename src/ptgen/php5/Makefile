
DEBUGFLAGS?=-O3

CPPFLAGS+=-I../../include
CXX=g++
CXXFLAGS+=$(DEBUGFLAGS)
LDFLAGS+=$(DEBUGFLAGS)
OBJS= lex.yy.o pt_zend_language_parser.tab.o head.o
#TARGET=php_ptgen # this is just a test driver
TARGET=phpptgen.a

all: $(TARGET)

phpptgen.a: $(OBJS)
	ar -csrv $@ $(OBJS)

php_ptgen: $(OBJS) main.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $(OBJS) main.o

#${TARGET}: ${OBJS} main.o
#	$(CXX) -o ${TARGET} ${OBJS} main.o

pt_zend_language_parser.tab.cc: pt_zend_language_parser.y
	bison -d pt_zend_language_parser.y -o pt_zend_language_parser.tab.cc

lex.yy.cc: zend_language_scanner.l pt_zend_language_parser.tab.cc
	flex -i -olex.yy.cc zend_language_scanner.l

head.cc pt_zend_language_parser.y: zend_language_parser.y zend_language_parser.y.head zend_language_parser.y.foot
	./mainphp.py zend_language_parser.y
	sed -i "s/'\"'/'\\\\\"'/" head.cc

.PHONY: clean
clean:
	rm -f *.o lex.yy.cc pt_zend_language_parser.tab* pt_zend_language_parser.y head.cc $(TARGET)

