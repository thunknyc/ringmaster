CFLAGS = -emit-llvm -Werror -Wall -std=c99 -Ofast

a.out: main.o ringmgr.o
	$(CC) -o a.out main.o ringmgr.o

ringmgr.o: ringmgr.c ringmgr.h

main.c: ringmgr.h

clean:
	rm -f *.o a.out
