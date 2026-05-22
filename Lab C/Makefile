all: mypipeline myshell

mypipeline: mypipeline.o
	gcc -m32 -g -Wall -o mypipeline mypipeline.o

mypipeline.o: mypipeline.c
	gcc -m32 -g -Wall -c -o mypipeline.o mypipeline.c

myshell: myshell.o LineParser.o
	gcc -m32 -g -Wall -o myshell myshell.o LineParser.o

myshell.o: myshell.c LineParser.h
	gcc -m32 -g -Wall -c -o myshell.o myshell.c

LineParser.o: LineParser.c LineParser.h
	gcc -m32 -g -Wall -c -o LineParser.o LineParser.c

clean:
	rm -f mypipeline mypipeline.o myshell myshell.o LineParser.o