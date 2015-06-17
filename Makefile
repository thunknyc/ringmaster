#CFLAGS = -Werror -Wall -std=c99 -g -O0            # debug-friendly
CFLAGS = -emit-llvm -Werror -Wall -std=c99 -Ofast # performance

a.out: main.o ringmaster.o
	$(CC) -o a.out main.o ringmaster.o

ringmaster.o: ringmaster.c ringmaster.h

main.c: ringmaster.h

clean:
	rm -f *.o a.out
