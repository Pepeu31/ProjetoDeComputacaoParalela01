CC=gcc
CFLAGS=-Wall -O2

all: seq par

seq: log_analyzer_seq.c
	$(CC) $(CFLAGS) -o seq log_analyzer_seq.c

par: log_analyzer_par.c
	$(CC) $(CFLAGS) -o par log_analyzer_par.c -pthread

clean:
	rm -f seq par *.o
