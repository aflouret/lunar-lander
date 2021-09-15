CC := gcc
CFLAGS := -Wall -c -pedantic -std=c99 -g
LFLAGS := -lSDL2 -lm
INSTALL_DIR := /usr/sbin

all: lunar_lander

main.o: main.c config.h naves.h
	$(CC) $(CFLAGS) main.c

caracteres.o: caracteres.c caracteres.h
	$(CC) $(CFLAGS) caracteres.c

naves.o: naves.c naves.h
	$(CC) $(CFLAGS) naves.c

vector.o: vector.c vector.h config.h
	$(CC) $(CFLAGS) vector.c	

fisica.o: fisica.c fisica.h
	$(CC) $(CFLAGS) fisica.c

diccionario.o: diccionario.c diccionario.h caracteres.h
	$(CC) $(CFLAGS) diccionario.c

lunar_lander: main.o caracteres.o naves.o vector.o fisica.o diccionario.o
	$(CC) $^ -o lunar_lander $(LFLAGS)

clean:
	rm *.o

install: lunar_lander
	cp $^ $(INSTALL_DIR)

valgrind: lunar_lander
	valgrind --leak-check=full ./lunar_lander
