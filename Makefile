CFLAGS += -Werror -Wall -O3
LDFLAGS += -flto
LIB_SRC = yama.c list.o
LIB_OBJ = $(subst .c,.o,$(LIB_SRC))

TEST_SRC = $(wildcard test/*.c)
TEST_OBJ = $(subst .c,.o,$(TEST_SRC))

all: libyama.so yamaclient

test: yamatest yamaclient
	./yamatest -v$(TEST_VERBOSE)
	./test.sh
	@echo PASSED

yamatest: $(TEST_OBJ) $(LIB_OBJ)
	$(CC) $^ -o $@

clienttest: yamaclient
	./test.sh

$(LIB_OBJ): CFLAGS+=-fPIC
$(TEST_OBJ): CFLAGS+=-I.

yamaclient: client.o libyama.so
	$(CC) -o $@ $< -L. -lyama

libyama.so: $(LIB_OBJ)
	$(CC) -shared -o $@ $^
	strip $@

.PHONY: all test clienttest clean

clean:
	-rm -rf $(LIB_OBJ) $(TEST_OBJ) *.o
	-rm -rf yamatest yamaclient libyama.so
	-rm -rf test*.yama
