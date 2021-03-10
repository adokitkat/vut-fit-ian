CC=gcc
CFLAGS=-std=c17 -Wall -Wpedantic# -Werror
DEBUG=-g
RELEASE=-O3
LIBS=-lelf

#TMPDIR=tmp/
#DOCDIR=doc/
SRCDIR=src/
BINDIR=

NAME=ian-proj1
OUTPUT=ian-proj1

#TOKEN=

.PHONY: all build build-debug release debug pack archive tar targz zip clean

all: build

# Building the program
build: release
build-debug: debug

# Release building
release: $(wildcard $(SRCDIR)*.c)
	$(CC) -o $(BINDIR)$(OUTPUT) $(CFLAGS) $(LIBS) $(RELEASE) $^

# Debug building
debug: $(wildcard $(SRCDIR)*.c)
	$(CC) -o $(BINDIR)$(OUTPUT) $(CFLAGS) $(LIBS) $(DEBUG) $^

# Archive creation
pack: archive
archive: targz
tar:
	tar -cvf $(NAME).tar README.md Makefile $(SRCDIR)*

targz:
	tar -czvf $(NAME).tar.gz README.md Makefile $(SRCDIR)*

zip:
	zip -r $(NAME).zip README.md Makefile $(SRCDIR)*

# Cleaning
clean:
	rm -rf $(NAME).* $(OUTPUT)
