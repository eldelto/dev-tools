# Use GCC as compiler.
CC := gcc
# Set additional compiler flags.
CFLAGS  := -Wall -Werror -Wextra -pedantic-errors -std=c17 -MMD -MP

.PHONY: all
all: bin/hyper-shell bin/hyper-git

bin/hyper-shell: hyper-shell.o
	$(CC) $^ -o $@ $(LDFLAGS)

bin/hyper-git: hyper-git.o deps/inih/ini.o
	$(CC) $^ -o $@ $(LDFLAGS)

# Dependencies
.PHONY: deps
deps: deps/inih/ini.o

.PHONY: clean
clean:
	rm -f bin/*
	rm -f *.o
	rm -f *.d

-include *.d
