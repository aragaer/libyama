CFLAGS += -Werror -Wall
all: libyama.so yamaclient

test: yamatest
	./yamatest
	$(MAKE) clienttest

yamatest: yamatest.o yama.o yama.h

clienttest: yamaclient
	./test.sh

yama.o: CFLAGS+=-fPIC

yamaclient: client.o libyama.so
	$(CC) -o $@ $< -L. -lyama

libyama.so: yama.o
	$(CC) -shared -fPIC -o $@ $^

.PHONY: all test clienttest clean

clean:
	-rm -rf *.o yamatest client libyama.so
	-rm -rf test*.yama
