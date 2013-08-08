XB_SRC=$(DVH)/c/xblab/src
XB_SRC_C=$(XB_SRC)/common
XB_SRC_S=$(XB_SRC)/server
XB_SRC_P=$(XB_SRC)/protobuf

CC = g++

CFLAGS = -I/usr/local/include -I/usr/local/include/botan-1.10 \
	-I/usr/local/include/pqxx -I$(XB_SRC) -Wall -Wextra \
	-Wno-unused-parameter -D XBLAB_NATIVE=1

LDFLAGS = -L/usr/local/lib -lm -luv -lbotan-1.10 \
	-lyajl -lprotobuf-lite -lpcrecpp -lpqxx -lpq

ifeq ($(shell uname),Darwin)
CFLAGS += -mmacosx-version-min=10.6
endif

SRCS = $(XB_SRC_C)/crypto.cc $(XB_SRC_P)/xblab.pb.cc \
	$(XB_SRC_S)/memberBaton.cc $(XB_SRC_S)/batonUtil.cc \
	$(XB_SRC_S)/db.cc $(XB_SRC_S)/manager.cc $(XB_SRC_S)/member.cc \
	$(XB_SRC_S)/server.cc $(XB_SRC_S)/xblab.cc

OBJS = $(SRCS:.cc=.o)

TARGET = xblab

.PHONY:	all clean debug

all:	$(TARGET)

clean:
	rm -f core $(TARGET) *.o

debug:	CFLAGS += -D DEBUG=1 -D TRACE=1

debug:	$(TARGET)

$(notdir $(OBJS)):	$(SRCS) $(HDRS)
	$(CC) -c -g $(CFLAGS) $(SRCS)

xblab: $(notdir $(OBJS))
	$(CC) $^ -o $@ $(LDFLAGS)

