build: main.c
	gcc -o snascii main.c -Wall -Wextra -ggdb -std=c99

run: build
	./snascii