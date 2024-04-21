CC=gcc
CFLAGS=-Wall -Wextra -pedantic

heap: app.c cuslib.c cuslib.h
	$(CC) $(CFLAGS) -o cuslib app.c cuslib.c
