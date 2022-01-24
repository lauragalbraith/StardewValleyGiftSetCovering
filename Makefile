# Description: compile code for the StardewValleyGiftSetCovering project
# Author: Laura Galbraith

COMPILER = g++
WARNINGS_FLAGS = -Wall -Werror -Wextra -pedantic -Wshadow -Wconversion
ERRORS_FLAGS = -fmax-errors=3
LEAK_FLAGS = -static-liblsan -fsanitize=leak

all: main.out

main.out: main.cpp
	$(COMPILER) $(ERRORS_FLAGS) $(WARNINGS_FLAGS) $(ERRORS_FLAGS) $^ -o $@

clean:
	rm -f *.out test*.txt

help:
	@echo "This makefile compiles code for the StardewValleyGiftSetCovering project"
	@echo "Try 'make main.out'"
