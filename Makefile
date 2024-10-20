.PHONY: clean

CFLAGS = -Wall -Wextra -Wno-unused-parameter -Werror -Wpedantic --std=c11 -g --debug -fdiagnostics-absolute-paths

all: shell test

shell:
	cc $(CFLAGS) shell.c -o ./build/shell

test:
	cc $(CFLAGS) util_test.c -o ./build/util_test

clean:
	rm -r ./build/*