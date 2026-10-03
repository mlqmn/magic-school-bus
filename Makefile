all: build run

build:
	gcc -ISIMLIB/ bus.c SIMLIB/simlib.c -lm -o bus

run:
	./bus