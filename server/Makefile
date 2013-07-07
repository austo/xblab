XB_SRC=/Users/austin/devProjects/c/xblab/src
CC = g++
CFLAGS = -I/usr/local/include -I/usr/local/include/botan-1.10 -I/usr/local/include/pqxx -I${XB_SRC} -Wall -Wextra -Wno-unused-parameter
LDFLAGS = -L/usr/local/lib -lm -luv -lbotan-1.10 -lyajl.2 -lprotobuf-lite -lpqxx -lpq


.PHONY: 	all clean

all:    	xblab	

clean:
	rm -f xblab *.o

xblab:		crypto.o xblab.pb.o manager.o util.o db.o server.o xblab.o
	$(CC) $^ -o $@ $(LDFLAGS)

xblab.o:	$(XB_SRC)/xblab.cc $(XB_SRC)/server.h $(XB_SRC)/macros.h
	$(CC) -c -g $(CFLAGS) -D XBLAB_NATIVE=1 $(XB_SRC)/xblab.cc

server.o:	$(XB_SRC)/server.cc $(XB_SRC)/server.h $(XB_SRC)/crypto.h $(XB_SRC)/baton.h $(XB_SRC)/macros.h
	$(CC) -c -g $(CFLAGS) -D XBLAB_NATIVE=1 $(XB_SRC)/server.cc

manager.o:	$(XB_SRC)/native/manager.cc $(XB_SRC)/native/manager.h $(XB_SRC)/crypto.h $(XB_SRC)/group.h $(XB_SRC)/member.h $(XB_SRC)/db.h
	$(CC) -c -g $(CFLAGS) -D XBLAB_NATIVE=1 $(XB_SRC)/native/manager.cc

db.o:		$(XB_SRC)/db.cc $(XB_SRC)/db.h $(XB_SRC)/macros.h $(XB_SRC)/group.h $(XB_SRC)/native/manager.h
	$(CC) -c -g $(CFLAGS) -D XBLAB_NATIVE=1 $(XB_SRC)/db.cc

util.o:		$(XB_SRC)/util.cc $(XB_SRC)/util.h $(XB_SRC)/protobuf/xblab.pb.h $(XB_SRC)/baton.h  \
			$(XB_SRC)/crypto.h $(XB_SRC)/member.h $(XB_SRC)/macros.h $(XB_SRC)/native/manager.h
	$(CC) -c -g $(CFLAGS) -D XBLAB_NATIVE=1 $(XB_SRC)/util.cc

xblab.pb.o:	$(XB_SRC)/protobuf/xblab.pb.cc $(XB_SRC)/protobuf/xblab.pb.h
	$(CC) -c -g $(CFLAGS) -D XBLAB_NATIVE=1 $(XB_SRC)/protobuf/xblab.pb.cc	

crypto.o:	$(XB_SRC)/crypto.cc $(XB_SRC)/crypto.h $(XB_SRC)/macros.h
	$(CC) -c -g $(CFLAGS) -D XBLAB_NATIVE=1 $(XB_SRC)/crypto.cc
