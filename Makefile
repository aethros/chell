.PHONY: clean

all: shell test

shell:
	cc -g shell.c -o ./build/shell

test:
	cc -g util_test.c -o ./build/util_test

clean:
	rm -r ./build/*