CC=gcc
CFLAGS=-O3
LD=$(CC)
LDFLAGS=-lstdc++ -lsupc++ -lbz2

all:	indexer

indexer: indexer.o char_convert.o 
	$(LD) $(LDFLAGS) -o $@ $^

%.o:	%.cpp
		$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

clean:
		rm -f *.o indexer

indexer.o: indexer.h
