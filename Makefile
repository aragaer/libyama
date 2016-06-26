CFLAGS += -Werror -Wall -O3
all: libyama.so yamaclient

test: yamatest yamaclient
	./yamatest -v$(TEST_VERBOSE)
	./test.sh
	@echo PASSED

yamatest: yamatest.o yama.o list.o

clienttest: yamaclient
	./test.sh

yamatest.o: yamatest.c yama.h
list.o: list.c list.h
yama.o: yama.c yama.h
yama.o list.o: CFLAGS+=-fPIC

yamaclient: client.o libyama.so
	$(CC) -o $@ $< -L. -lyama

libyama.so: yama.o list.o yama.h
	$(CC) -shared -o $@ $^
	strip $@

.PHONY: all test clienttest clean

clean:
	-rm -rf *.o yamatest client libyama.so
	-rm -rf test*.yama
