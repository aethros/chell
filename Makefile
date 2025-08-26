.PHONY: clean

BUILD_DIR = ./build
SRC_DIR   = .
CFLAGS    = -Wall -Wextra -Werror -Wpedantic -g --std=gnu11 --debug \
			-fsanitize=address # -DDEBUG=TRUE
LDFLAGS   = -fsanitize=address
SHELL_OBJS = $(BUILD_DIR)/shell.o
UTIL_TEST_OBJS = $(BUILD_DIR)/util_test.o
TARGETS   = shell util_test

all: $(TARGETS)

shell: $(SHELL_OBJS)
	$(CC) $(LDFLAGS) -o $(BUILD_DIR)/$@ $^

util_test: $(UTIL_TEST_OBJS)
	$(CC) $(LDFLAGS) -o $(BUILD_DIR)/$@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -r ./build/*
