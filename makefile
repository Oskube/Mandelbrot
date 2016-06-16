CC = gcc
CFLAGS = -Wall
SDL = `sdl2-config --cflags --libs`

all: bin/mandelbrot

debug: CFLAGS += -g
debug: all

bin/mandelbrot: obj/chunks.o src/main.c
	$(CC) -o $@ $^ $(SDL) $(CFLAGS)

obj/chunks.o: src/chunks.c
	$(CC) -c -o $@ $^ $(CFLAGS)

clean:
	- rm bin/mandelbrot
	- rm obj/*.o
