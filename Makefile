CFLAGS += -Werror -Wall
all: libyama.so yamaclient

test: yamatest
	./yamatest -v$(TEST_VERBOSE)
	$(MAKE) clienttest
	@echo PASSED

yamatest: yamatest.o yama.o

clienttest: yamaclient
	./test.sh

yama.o: yama.c yama.h
yama.o: CFLAGS+=-fPIC

yamaclient: client.o libyama.so
	$(CC) -o $@ $< -L. -lyama

libyama.so: yama.o
	$(CC) -shared -fPIC -o $@ $^

.PHONY: all test clienttest clean

clean:
	-rm -rf *.o yamatest client libyama.so
	-rm -rf test*.yama
