all: mypipeline myshell

mypipeline: mypipeline.o
	gcc -m32 -g -Wall -o mypipeline mypipeline.o

mypipeline.o: mypipeline.c
	gcc -m32 -g -Wall -c -o mypipeline.o mypipeline.c

myshell: myshell.o
	gcc -m32 -g -Wall -o myshell myshell.o

myshell.o: myshell.c
	gcc -m32 -g -Wall -c -o myshell.o myshell.c

clean:
	rm -f mypipeline mypipeline.o myshell myshell.o