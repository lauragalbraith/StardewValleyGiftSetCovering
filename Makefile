# Description: compile code for the StardewValleyGiftSetCovering project
# Author: Laura Galbraith

COMPILER = g++
WARNINGS_FLAGS = -Wall -Werror -Wextra -pedantic -Wshadow -Wconversion
ERRORS_FLAGS = -fmax-errors=3
LEAK_FLAGS = -static-liblsan -fsanitize=leak
LINK_LIBCURL_FLAGS = -lcurl

all: determine_gifts.out

determine_gifts.out: curl.out main.out
	$(COMPILER) -o $@ $^ $(LINK_LIBCURL_FLAGS)

determine_gifts_debug.out: curl_debug.out main_debug.out
	$(COMPILER) -o $@ $^ $(LINK_LIBCURL_FLAGS)

curl.out: curl.cpp
	$(COMPILER) $(ERRORS_FLAGS) $(WARNINGS_FLAGS) $(LEAK_FLAGS) -c $^ -o $@ $(LINK_LIBCURL_FLAGS)

curl_debug.out: curl.cpp
	$(COMPILER) $(WARNINGS_FLAGS) $(LEAK_FLAGS) -c $^ -o $@ $(LINK_LIBCURL_FLAGS)

main.out: main.cpp
	$(COMPILER) $(ERRORS_FLAGS) $(WARNINGS_FLAGS) $(LEAK_FLAGS) -c $^ -o $@

main_debug.out: main.cpp
	$(COMPILER) $(WARNINGS_FLAGS) $(LEAK_FLAGS) -c $^ -o $@

clean:
	rm -f *.out test*.txt

help:
	@echo "This makefile compiles code for the StardewValleyGiftSetCovering project"
	@echo "Try 'make main.out'"
