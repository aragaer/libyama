CFLAGS += -Werror -Wall -O3
LIB_SRC = yama.c list.o
LIB_OBJ = $(subst .c,.o,$(LIB_SRC))

all: libyama.so yamaclient

test: yamatest yamaclient
	./yamatest -v$(TEST_VERBOSE)
	./test.sh
	@echo PASSED

yamatest: yamatest.o $(LIB_OBJ)

clienttest: yamaclient
	./test.sh

yama.o list.o: CFLAGS+=-fPIC

yamaclient: client.o libyama.so
	$(CC) -o $@ $< -L. -lyama

libyama.so: $(LIB_OBJ)
	$(CC) -shared -o $@ $^
	strip $@

.PHONY: all test clienttest clean

clean:
	-rm -rf *.o yamatest yamaclient libyama.so
	-rm -rf test*.yama
