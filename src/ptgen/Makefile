
TARGET:
	make -C gcc
	make -C java
	make -C php5
	make -C sol

.PHONY: clean
clean:
	rm -f *.pyc
	make -C simple clean
	make -C gcc clean
	make -C java clean
	make -C php5 clean
	make -C sol clean

