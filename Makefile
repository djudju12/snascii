build: main.c
	gcc -o snascii main.c -Wall -Wextra

run: build
	./snascii