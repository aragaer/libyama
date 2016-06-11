all: libyama.so

test: yamatest
	./yamatest

yamatest: yamatest.o yama.o
