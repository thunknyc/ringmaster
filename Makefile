a.out: main.o obstructor.o
	$(CC) -o a.out main.o obstructor.o

obstructor.o: obstructor.c

obstructor.c: obstructor.h

main.c: obstructor.h

clean:
	rm -f *.o a.out