XB_SRC=/Users/austin/devProjects/c/xblab/src
CC = g++
CFLAGS = -I/usr/local/include -I/usr/local/include/botan-1.10 -I${XB_SRC} -Wall -Wextra -Wno-unused-parameter
LDFLAGS = -L/usr/local/lib -lm -luv -lbotan-1.10


.PHONY: all clean

all:    native.server	

clean:	
	rm -f native.server *.o

native.server:     crypto.o native.server.o
	$(CC) $^ -o $@ $(LDFLAGS)

native.server.o:	$(XB_SRC)/native.server.cc $(XB_SRC)/crypto.h
	$(CC) -c -g $(CFLAGS) -D XBLAB_NATIVE=1 $(XB_SRC)/native.server.cc

crypto.o:		$(XB_SRC)/crypto.cc $(XB_SRC)/crypto.h $(XB_SRC)/macros.h
	$(CC) -c -g $(CFLAGS) -D XBLAB_NATIVE=1 $(XB_SRC)/crypto.cc
