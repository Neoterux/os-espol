CC ?= gcc
LD ?= ld
CFLAGS ?= -Wall -Werror
DFLAGS ?= -g -fsanitize=address
LDFLAGS ?= -Wall
EXE = time

builddir:
	mkdir -p build

time: builddir
	${CC} ${CFLAGS} ${DFLAGS} time.c -o build/time

time-pipe: builddir
	${CC} ${CFLAGS} ${DFLAGS} timepipe.c -o build/timepipe

time-shmm: builddir
	${CC} ${CFLAGS} ${DFLAGS} timeshmm.c -o build/time-shmm

build: time-pipe time-shmm
#	${LD} build/time-shmm.o build/timepipe.o -o build/${EXE}

clean:
	rm build/*

