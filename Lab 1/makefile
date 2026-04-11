all: menu_map

count-words: count-words.o
	gcc -m32 -g -Wall -o count-words count-words.o

count-words.o: count-words.c
	gcc -m32 -g -Wall -c -o count-words.o count-words.c

addresses: addresses.o
	gcc -m32 -g -Wall -o addresses addresses.o

addresses.o: addresses.c
	gcc -m32 -g -Wall -c -o addresses.o addresses.c

base: base.o
	gcc -m32 -g -Wall -o base base.o

base.o: base.c
	gcc -m32 -g -Wall -c -o base.o base.c

menu_map: menu_map.o
	gcc -m32 -g -Wall -o menu_map menu_map.o

menu_map.o: menu_map.c
	gcc -m32 -g -Wall -c -o menu_map.o menu_map.c

clean:
	rm -f count-words count-words.o addresses addresses.o base base.o menu_map menu_map.o