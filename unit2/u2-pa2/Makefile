CC ?= gcc
LD ?= ld
CFLAGS ?= -Wall -Werror
DFLAGS ?= -g -fsanitize=address
LDFLAGS ?= -fsanitize=address 
EXE = time

builddir:
	mkdir -p build

time.o: builddir
	${CC} ${CFLAGS} ${DFLAGS} time.c -c -o build/time.o

tpiper.so: builddir
	${CC} ${CFLAGS} ${DFLAGS} timepipe.c -shared -o build/tpiper.so

tshmm.so: builddir
	${CC} ${CFLAGS} ${DFLAGS} timeshmm.c -shared -o build/tshmm.so

${EXE}: tpiper.so tshmm.so time.o
	${CC} ${LDFLAGS} build/tpiper.so build/time.o -o build/${EXE}p
	${CC} ${LDFLAGS} build/tshmm.so build/time.o -o build/${EXE}s


build: time-pipe time-shmm
#	${LD} build/time-shmm.o build/timepipe.o -o build/${EXE}

clean:
	rm build/*

