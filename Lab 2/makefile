all: myshell mypipe

myshell: myshell.o LineParser.o
	gcc -m32 -g -Wall -o myshell myshell.o LineParser.o

myshell.o: myshell.c
	gcc -m32 -g -Wall -c -o myshell.o myshell.c

LineParser.o: LineParser.c
	gcc -m32 -g -Wall -c -o LineParser.o LineParser.c

mypipe: mypipe.o
	gcc -m32 -g -Wall -o mypipe mypipe.o

mypipe.o: mypipe.c
	gcc -m32 -g -Wall -c -o mypipe.o mypipe.c

clean:
	rm -f myshell myshell.o LineParser.o mypipe mypipe.o