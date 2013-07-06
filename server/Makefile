XB_SRC=/Users/austin/devProjects/c/xblab/src
CC = g++
CFLAGS = -I/usr/local/include -I/usr/local/include/botan-1.10 -I${XB_SRC} -Wall -Wextra -Wno-unused-parameter
LDFLAGS = -L/usr/local/lib -lm -luv -lbotan-1.10 -lyajl.2


.PHONY: all clean

all:    xblab	

clean:	
	rm -f xblab *.o

xblab:     crypto.o server.o xblab.o
	$(CC) $^ -o $@ $(LDFLAGS)

xblab.o:	$(XB_SRC)/xblab.cc $(XB_SRC)/server.h $(XB_SRC)/macros.h
	$(CC) -c -g $(CFLAGS) -D XBLAB_NATIVE=1 $(XB_SRC)/xblab.cc

server.o:	$(XB_SRC)/server.cc $(XB_SRC)/server.h $(XB_SRC)/crypto.h $(XB_SRC)/baton.h $(XB_SRC)/macros.h
	$(CC) -c -g $(CFLAGS) -D XBLAB_NATIVE=1 $(XB_SRC)/server.cc

crypto.o:		$(XB_SRC)/crypto.cc $(XB_SRC)/crypto.h $(XB_SRC)/macros.h
	$(CC) -c -g $(CFLAGS) -D XBLAB_NATIVE=1 $(XB_SRC)/crypto.cc
