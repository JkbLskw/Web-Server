CC=gcc
CFLAGS=-g -ansi -Wall

all: webserver

webserver: webserver.c prozent.c auftrag.c anfrage.c request.c
	$(CC) $(CFLAGS) webserver.c prozent.c auftrag.c anfrage.c request.c -o webserver
