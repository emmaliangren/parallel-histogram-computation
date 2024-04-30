CC = gcc
CFLAGS = -Wall -std=c11 -g

A1.o: A1.c 
	$(CC) $(CFLAGS) A1.c -o A1

clean: 
	rm A1 *.hist