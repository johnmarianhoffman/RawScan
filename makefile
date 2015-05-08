CC=gcc
CFLAGS=-Wall -g -std=c99 -w
SOURCES=main.c raw_scan.c

all:
	$(CC) $(CFLAGS) $(SOURCES) -o test
