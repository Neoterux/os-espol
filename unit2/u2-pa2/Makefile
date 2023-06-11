CC ?= gcc
LD ?= ld
CFLAGS ?= -Wall -Werror
DFLAGS ?= -g -fsanitize=address
LDFLAGS ?= -fsanitize=address
EXE = build/time

builddir:
	mkdir -p build

build/time.o: builddir
	${CC} ${CFLAGS} ${DFLAGS} time.c -c -o $@

build/tpiper.o: builddir
	${CC} ${CFLAGS} ${DFLAGS} timepipe.c -c -o $@

build/tshmm.o: builddir
	${CC} ${CFLAGS} ${DFLAGS} timeshmm.c -c -o $@

$(EXE)p: build/tpiper.o 
	@echo Compiling pipe based executable
	${CC} ${LDFLAGS} time.c $< -o $@

$(EXE)s: build/tshmm.o
	@echo Compiling shared memory based executable
	${CC} ${LDFLAGS} time.c $< -o $@

all: clean $(EXE)s $(EXE)p

clean:
	rm -rf build/*

