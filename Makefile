CFLAGS=-O2 -std=gnu99 -g -Wall -Wextra -Werror -pedantic -lpthread

all: proj2
 
run: all
	./proj2 5 2 0 1
	
proj2: proj2.c
	gcc $(CFLAGS) proj2.c -o proj2
