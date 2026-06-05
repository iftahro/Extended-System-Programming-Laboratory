all: multi

multi: multi.o
	gcc -m32 multi.o -o multi

multi.o: multi.s
	nasm -f elf32 multi.s -o multi.o

clean:
	rm -f *.o multi