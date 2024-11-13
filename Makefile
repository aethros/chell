.PHONY: clean

CFLAGS = -Wall -Wextra -Werror -Wpedantic --std=gnu11 -g --debug #-DDEBUG=TRUE

all: shell test

shell:
	cc $(CFLAGS) shell.c -o ./build/shell

test:
	cc $(CFLAGS) util_test.c -o ./build/util_test

clean:
	rm -r ./build/*