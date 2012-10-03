USING_LIBUVUDEC=Y
include ../common.mk

CFLAGS += -Wall -Werror

PCAP_PREFIX=/home/mcmaster/document/prefix/libpcap-1.1.1

CFLAGS += -I$(PCAP_PREFIX)/include

PCAP_LIB_DIR = $(PCAP_PREFIX)/lib
LFLAGS += -L$(PCAP_LIB_DIR)
LFLAGS += -Wl,-rpath,$(PCAP_LIB_DIR)
LFLAGS += -lpcap
LFLAGS += -lboost_system
LFLAGS += -lboost_filesystem
LFLAGS += -ldl

all:
	g++ $(CFLAGS) $(LFLAGS) main.cpp -o uvusbreplay

clean:
	rm -f uvusbreplay *~

