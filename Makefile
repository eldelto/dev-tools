# Use GCC as compiler.
CC := gcc
# Set additional compiler flags.
CFLAGS  := -Wall -Werror -Wextra -pedantic-errors -std=c17 -MMD -MP

bin/hyper-shell: hyper-shell.o

.PHONY: clean
clean:
	rm *.o
	rm *.d
