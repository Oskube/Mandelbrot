CC = gcc
CFLAGS = -Wall
SDL = `sdl2-config --cflags --libs`

all: bin/mandelbrot

debug: CFLAGS += -g
debug: all

bin/mandelbrot: obj/chunks.o src/main.c
	-@ mkdir bin
	$(CC) -o $@ $^ $(SDL) $(CFLAGS)

obj/chunks.o: src/chunks.c
	-@ mkdir obj
	$(CC) -c -o $@ $^ $(CFLAGS)

clean:
	- rm bin/mandelbrot
	- rm obj/*.o
	- rmdir bin
	- rmdir obj
