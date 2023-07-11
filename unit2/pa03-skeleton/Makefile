GCC = gcc
CFLAGS = -Wall -Wshadow -lm -g
PFLAGS ?= 
OBJS = ex5.o bmp.o filter.o

ex5: $(OBJS)
	$(GCC) $(CFLAGS) $(OBJS) -o $@

.c.o: 
	$(GCC) $(CFLAGS) $(PFLAGS) -c $*.c 

clean:
	rm *.o ex5

test: ex5
	./ex5 testcases/test.bmp outputs/test_out.bmp
	diff outputs/test_out.bmp testcases/test_sol.bmp

custom:
	env PFLAGS='-g -DDEBUG_VERBOSE -DTHREADED_APPLY_PRETEST' make ex5
	
testmem: ex5
	valgrind --tool=memcheck --leak-check=summary ./ex5 testcases/test.bmp outputs/test_out.bmp	
