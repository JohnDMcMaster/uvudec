# UVNet Utils (uvutils)
# Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
# Licensed under GPL V3+, see COPYING for details
# Original code from
# http://www.woodmann.com/forum/showthread.php?7517-IDA-signature-file-format

DEBUG_FLAGS=-g
WARNING_FLAGS=-Wall
FLAGS_SHARED += -c $(WARNING_FLAGS) $(INCLUDES) $(DEBUG_FLAGS) $(OPTIMIZATION_LEVEL)
CCFLAGS += $(FLAGS_SHARED)
CXXFLAGS += $(FLAGS_SHARED)

CXX_SRCS = main.cpp
OBJS = $(CC_SRCS:.c=.o) $(CXX_SRCS:.cpp=.o)

OUTPUT=rxdumpsig

default: all

$(shell touch Makefile.depend)
include Makefile.depend


%.o: %.c
	$(CC) $(CCFLAGS) $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

all: depend $(OBJS)
	$(CXX) -o $(OUTPUT) $(OBJS)

clean:
	rm -f *.o $(OUTPUT) Makefile.depend

depend:
	 $(CXX) -MM $(CXX_SRCS) >Makefile.depend

