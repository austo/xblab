XB_SRC=$(DVH)/c/xblab/src
XB_SRC_N=$(DVH)/c/xblab/src/native
XB_SRC_P=$(DVH)/c/xblab/src/protobuf
CC = g++
CFLAGS = -I/usr/local/include -I/usr/local/include/botan-1.10 \
	-I/usr/local/include/pqxx -I${XB_SRC} -Wall -Wextra -Wno-unused-parameter -D XBLAB_NATIVE=1
LDFLAGS = -L/usr/local/lib -lm -luv -lbotan-1.10 -lyajl -lprotobuf-lite -lpqxx -lpq


ifeq ($(shell uname),Darwin)
CFLAGS += -mmacosx-version-min=10.6
endif


.PHONY: 	all clean

all:    	xblab	

clean:
	rm -f xblab *.o

xblab:		crypto.o xblab.pb.o manager.o clientBaton.o util.o db.o server.o xblab.o
	$(CC) $^ -o $@ $(LDFLAGS)

xblab.o:	$(XB_SRC_N)/xblab.cc $(XB_SRC_N)/server.h $(XB_SRC)/macros.h
	$(CC) -c -g $(CFLAGS) $(XB_SRC_N)/xblab.cc

server.o:	$(XB_SRC_N)/server.cc $(XB_SRC_N)/util.h $(XB_SRC_N)/server.h \
			$(XB_SRC)/crypto.h $(XB_SRC_N)/clientBaton.h $(XB_SRC)/macros.h
	$(CC) -c -g $(CFLAGS) $(XB_SRC_N)/server.cc

manager.o:	$(XB_SRC_N)/manager.cc $(XB_SRC_N)/manager.h $(XB_SRC)/crypto.h \
			$(XB_SRC)/group.h $(XB_SRC)/member.h $(XB_SRC)/db.h
	$(CC) -c -g $(CFLAGS) $(XB_SRC_N)/manager.cc

db.o:		$(XB_SRC)/db.cc $(XB_SRC)/db.h $(XB_SRC)/macros.h $(XB_SRC)/group.h $(XB_SRC_N)/manager.h
	$(CC) -c -g $(CFLAGS) $(XB_SRC)/db.cc

util.o:		$(XB_SRC_N)/util.cc $(XB_SRC_N)/manager.h $(XB_SRC_N)/util.h $(XB_SRC_P)/xblab.pb.h \
			$(XB_SRC_N)/clientBaton.h $(XB_SRC)/crypto.h $(XB_SRC)/member.h $(XB_SRC)/macros.h
	$(CC) -c -g $(CFLAGS) $(XB_SRC_N)/util.cc

clientBaton.o:	$(XB_SRC_N)/clientBaton.cc $(XB_SRC_N)/clientBaton.h $(XB_SRC)/baton.h \
				$(XB_SRC)/member.h util.o
	$(CC) -c -g $(CFLAGS) $(XB_SRC_N)/clientBaton.cc

xblab.pb.o:	$(XB_SRC_P)/xblab.pb.cc $(XB_SRC_P)/xblab.pb.h
	$(CC) -c -g $(CFLAGS) $(XB_SRC_P)/xblab.pb.cc	

crypto.o:	$(XB_SRC)/crypto.cc $(XB_SRC)/crypto.h $(XB_SRC)/macros.h
	$(CC) -c -g $(CFLAGS) $(XB_SRC)/crypto.cc
