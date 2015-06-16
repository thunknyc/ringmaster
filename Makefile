CFLAGS = -emit-llvm -Werror -Wall -std=c99 -Ofast

a.out: main.o ringmaster.o
	$(CC) -o a.out main.o ringmaster.o

ringmaster.o: ringmaster.c ringmaster.h

main.c: ringmaster.h

clean:
	rm -f *.o a.out
