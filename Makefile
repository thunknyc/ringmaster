CFLAGS = -emit-llvm -Werror -Wall -std=c99 -Ofast

a.out: main.o buffmgr.o
	$(CC) -o a.out main.o buffmgr.o

buffmgr.o: buffmgr.c buffmgr.h

main.c: buffmgr.h

clean:
	rm -f *.o a.out
