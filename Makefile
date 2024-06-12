build: main.c
	gcc -o snascii main.c -Wall -Wextra -ggdb

run: build
	./snascii