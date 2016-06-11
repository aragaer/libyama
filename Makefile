all: libyama.so

test: yamatest
	./yamatest

yamatest: yamatest.o yama.o yama.h

clean:
	-rm -rf *.o yamatest
