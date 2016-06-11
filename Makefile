all: libyama.so

test: yamatest
	./yamatest

yamatest: yamatest.o yama.o

clean:
	-rm -rf *.o yamatest
