
.PHONY: all src/main

all: src/main

src/main: src/vgen/treeTra/libvgen.a src/ptgen/gcc/gccptgen.a src/ptgen/java/javaptgen.a src/ptgen/php5/phpptgen.a
	$(MAKE) -C $@

src/vgen/treeTra/libvgen.a src/ptgen/gcc/gccptgen.a src/ptgen/java/javaptgen.a src/ptgen/php5/phpptgen.a:
	$(MAKE) -C `dirname $@`
 

