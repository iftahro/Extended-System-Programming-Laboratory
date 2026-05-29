all: hexeditplus task4.o

hexeditplus: task1.c
	gcc -m32 -g -Wall -fno-pie -no-pie -fno-stack-protector -o hexeditplus task1.c

task4.o: task4.c
	gcc -m32 -g -Wall -fno-pie -fno-stack-protector -c task4.c -o task4.o

clean:
	rm -f hexeditplus task4.o