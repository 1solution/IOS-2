CFLAGS=-O2 -std=gnu99 -g -Wall -Wextra -Werror -pedantic -lpthread

all: proj2
 
run: all
	./proj2 5 2 0 1
del:
	rm /dev/shm/sem.ridefinished
	rm /dev/shm/sem.mutex
	rm /dev/shm/sem.multiplex
	rm /dev/shm/sem.bus
	rm /dev/shm/sem.allaboard
	
proj2: proj2.c
	gcc $(CFLAGS) proj2.c -o proj2