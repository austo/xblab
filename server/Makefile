XB_SRC=$(DVH)/c/xblab/src
XB_SRC_C=$(XB_SRC)/common
XB_SRC_S=$(XB_SRC)/server
XB_SRC_P=$(XB_SRC)/protobuf
CC = g++
CFLAGS = -I/usr/local/include -I/usr/local/include/botan-1.10 \
	-I/usr/local/include/pqxx -I${XB_SRC} -Wall -Wextra -Wno-unused-parameter -D XBLAB_NATIVE=1
LDFLAGS = -L/usr/local/lib -lm -luv -lbotan-1.10 -lyajl -lprotobuf-lite -lpcrecpp -lpqxx -lpq


ifeq ($(shell uname),Darwin)
CFLAGS += -mmacosx-version-min=10.6
endif


.PHONY: 	all clean debug

all:    	xblab	

clean:
	rm -f xblab *.o

debug:		CFLAGS += -D DEBUG=1

debug:		xblab

xblab:		crypto.o xblab.pb.o manager.o memberBaton.o batonUtil.o db.o server.o xblab.o
	$(CC) $^ -o $@ $(LDFLAGS)

xblab.o:	$(XB_SRC_S)/xblab.cc $(XB_SRC_S)/server.h $(XB_SRC_C)/macros.h $(XB_SRC_C)/crypto.h
	$(CC) -c -g $(CFLAGS) $(XB_SRC_S)/xblab.cc

server.o:	$(XB_SRC_S)/server.cc $(XB_SRC_S)/server.h $(XB_SRC_S)/memberBaton.h $(XB_SRC_C)/macros.h
	$(CC) -c -g $(CFLAGS) $(XB_SRC_S)/server.cc

manager.o:	$(XB_SRC_S)/manager.cc $(XB_SRC_S)/manager.h $(XB_SRC_C)/crypto.h \
			$(XB_SRC_S)/group.h $(XB_SRC_S)/member.h $(XB_SRC_S)/db.h
	$(CC) -c -g $(CFLAGS) $(XB_SRC_S)/manager.cc

db.o:		$(XB_SRC_S)/db.cc $(XB_SRC_S)/db.h $(XB_SRC_S)/db_exception.h $(XB_SRC_C)/macros.h \
			$(XB_SRC_S)/group.h $(XB_SRC_S)/manager.h
	$(CC) -c -g $(CFLAGS) $(XB_SRC_S)/db.cc

batonUtil.o:	$(XB_SRC_S)/batonUtil.cc $(XB_SRC_C)/util_exception.h $(XB_SRC_S)/manager.h $(XB_SRC_S)/batonUtil.h \
				$(XB_SRC_P)/xblab.pb.h $(XB_SRC_S)/memberBaton.h $(XB_SRC_C)/crypto.h $(XB_SRC_S)/member.h $(XB_SRC_C)/macros.h
	$(CC) -c -g $(CFLAGS) $(XB_SRC_S)/batonUtil.cc

memberBaton.o:	$(XB_SRC_S)/memberBaton.cc $(XB_SRC_S)/memberBaton.h $(XB_SRC_C)/baton.h \
				$(XB_SRC_S)/member.h batonUtil.o
	$(CC) -c -g $(CFLAGS) $(XB_SRC_S)/memberBaton.cc

xblab.pb.o:	$(XB_SRC_P)/xblab.pb.cc $(XB_SRC_P)/xblab.pb.h
	$(CC) -c -g $(CFLAGS) $(XB_SRC_P)/xblab.pb.cc	

crypto.o:	$(XB_SRC_C)/crypto.cc $(XB_SRC_C)/crypto.h $(XB_SRC_C)/macros.h
	$(CC) -c -g $(CFLAGS) $(XB_SRC_C)/crypto.cc
