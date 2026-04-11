count-words: count-words.o
	gcc -m32 -g -Wall -o count-words count-words.o

count-words.o: count-words.c
	gcc -m32 -g -Wall -c -o count-words.o count-words.c

clean:
	rm -f count-words count-words.o