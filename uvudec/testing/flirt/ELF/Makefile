# UVNet Universal Decompiler (uvudec)
# FLIRT .pat file unit test data generation
# Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
# Licensed under GPL V3+, see COPYING for details

USED=uvudec
UVUDEC_USED=uvudec
FLAIR_USED=flair

ifeq ($(USED),$(UVUDEC_USED))
UVOBJ2PAT=../../../bin/uvobj2pat.dynamic
endif

ifeq ($(USED),$(FLAIR_USED))
PELF=pelf
endif

default: all

CC_SRCS=$(shell ls *.c)
CXX_SRCS=$(shell ls *.cpp)

OBJS = $(CC_SRCS:.c=.o) $(CXX_SRCS:.cpp=.o)
STATIC_LIBS = libm.a

PATS = $(OBJS:.o=.pat) $(STATIC_LIBS:.a=.pat)

%.pat: %.o %.a
ifeq ($(USED),$(UVUDEC_USED))
	$(UVOBJ2PAT) $< $@
endif
ifeq ($(USED),$(FLAIR_USED))
	pelf $< $@
endif

%.o: %.c
	$(CC) -c $< -o $@

%.o: %.cpp
	$(CXX) -c $< -o $@

# If we don't omit the frame pointer, the function is much larger
# If we omit -Os, it will use a move statement
short.o:
	$(CC) -c -Os -fomit-frame-pointer $< -o $@	
	
# Of course, only run this if you are REALLY sure this is what the output should be
pats: $(PATS)	
	@true

objs: $(OBJS)
	@true

flair:
	$(MAKE) all USED=flair
	
uvudec:
	$(MAKE) all USED=uvudec

all: objs pats
	@(echo 'updated unit test targets')

info:
	@(echo "CXX:$(CXX)")
	@(echo "OBJS: $(OBJS)")

