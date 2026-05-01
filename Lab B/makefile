virusDetector: AntiVirus.o
	gcc -m32 -g -Wall -o virusDetector AntiVirus.o

AntiVirus.o: AntiVirus.c
	gcc -m32 -g -Wall -c -o AntiVirus.o AntiVirus.c

clean:
	rm -f virusDetector AntiVirus.o