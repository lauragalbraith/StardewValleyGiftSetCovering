# Description: compile code for the StardewValleyGiftSetCovering project
# Author: Laura Galbraith

COMPILER = g++
WARNINGS_FLAGS = -Wall -Werror -Wextra -pedantic -Wshadow -Wconversion
ERRORS_FLAGS = -fmax-errors=3
LEAK_FLAGS = -static-liblsan -fsanitize=leak
LINK_LIBCURL_FLAGS = -lcurl
LINK_XML_FLAGS = -I/usr/include/libxml2 -lxml2

all: determine_gifts.out

determine_gifts.out: curl.out xmlparse.out valleyfacts.out main.out
	$(COMPILER) -o $@ $^ $(LINK_LIBCURL_FLAGS) $(LINK_XML_FLAGS)

determine_gifts_debug.out: curl_debug.out xmlparse_debug.out valleyfacts_debug.out main_debug.out
	$(COMPILER) -o $@ $^ $(LINK_LIBCURL_FLAGS) $(LINK_XML_FLAGS)

curl.out: curl.cpp
	$(COMPILER) $(ERRORS_FLAGS) $(WARNINGS_FLAGS) -c $^ -o $@ $(LINK_LIBCURL_FLAGS)

curl_debug.out: curl.cpp
	$(COMPILER) $(WARNINGS_FLAGS) $(LEAK_FLAGS) -c $^ -o $@ $(LINK_LIBCURL_FLAGS)

xmlparse.out: xmlparse.cpp
	$(COMPILER) $(ERRORS_FLAGS) $(WARNINGS_FLAGS) -c $^ -o $@ $(LINK_XML_FLAGS)

xmlparse_debug.out: xmlparse.cpp
	$(COMPILER) $(WARNINGS_FLAGS) $(LEAK_FLAGS) -c $^ -o $@ $(LINK_XML_FLAGS)

valleyfacts.out: valleyfacts.cpp
	$(COMPILER) $(ERRORS_FLAGS) $(WARNINGS_FLAGS) -c $^ -o $@

valleyfacts_debug.out: valleyfacts.cpp
	$(COMPILER) $(WARNINGS_FLAGS) $(LEAK_FLAGS) -c $^ -o $@

main.out: main.cpp
	$(COMPILER) $(ERRORS_FLAGS) $(WARNINGS_FLAGS) -c $^ -o $@

main_debug.out: main.cpp
	$(COMPILER) $(WARNINGS_FLAGS) $(LEAK_FLAGS) -c $^ -o $@

clean:
	rm -f *.out test*.txt

help:
	@echo "This makefile compiles code for the StardewValleyGiftSetCovering project"
	@echo "Try 'make determine_gifts.out'"
