ifeq ($(origin CC), default)
CC = gcc
endif
LFLAGS   ?= -flto -ffunction-sections -fdata-sections $(GFLAGS)
GFLAGS   ?=
CFLAGS   ?= -std=c99 -Wpedantic -ffunction-sections -fdata-sections -c $(GFLAGS)
OFLAGS   ?= -o
TESTCASE ?= test/main.imp

all: build/impc

debug: FORCE
	$(MAKE) -B GFLAGS="-g"

build/impc: build/main.o build/imp_lex.o build/imp_parse.o build/imp_compile.o | build
	$(CC) $(LFLAGS) $(OFLAGS) $@ $^

build/main.o: src/main.c | build
	$(CC) $(CFLAGS) $(OFLAGS) $@ $<

build/imp_lex.o: src/imp_lex.c | build
	$(CC) $(CFLAGS) $(OFLAGS) $@ $<

build/imp_parse.o: src/imp_parse.c | build
	$(CC) $(CFLAGS) $(OFLAGS) $@ $<

build/imp_compile.o: src/imp_compile.c | build
	$(CC) $(CFLAGS) $(OFLAGS) $@ $<

build:
	mkdir -p $@

clean: FORCE
	rm build/*.o

fclean: FORCE
	rm build/*.o
	rm test/*.asm
	rm test/*.bin

test: all FORCE
	build/impc $(TESTCASE) -o test/main.asm --debug
	fasm test/main.asm test/main.bin
	qemu-system-x86_64 -drive file=test/main.bin,format=raw

dtest: debug FORCE
	valgrind --leak-check=full --show-leak-kinds=all build/impc test/main.imp -o test/main.asm --debug
	rm vgcore.*

vim: FORCE | ~/.vim/ftdetect ~/.vim/syntax
	cp vim/ftdetect/imp.vim ~/.vim/ftdetect/imp.vim
	cp vim/syntax/imp.vim ~/.vim/syntax/imp.vim

~/.vim/ftdetect: | ~/.vim
	mkdir -p $@

~/.vim/syntax: | ~/.vim
	mkdir -p $@

~/.vim:
	mkdir -p $@

FORCE:
