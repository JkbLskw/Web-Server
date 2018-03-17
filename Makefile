CC=gcc
CFLAGS=-g -ansi -Wall

all: webserver

webserver: webserver.c percent_processor.c splitter.c response.c request_processor.c
	$(CC) $(CFLAGS) webserver.c percent_processor.c splitter.c response.c request_processor.c -o webserver
