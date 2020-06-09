EXTRA_CXXFLAGS=-I ..
EXTRA_CFLAGS=
CXXFLAGS=-O3 -Wall -std=c++17 $(EXTRA_CXXFLAGS)
CFLAGS=-O3 -Wall -std=c11 $(EXTRA_CFLAGS)
CXX=g++
CC=$(CXX) # Hack to allow implicit rules to link with g++

###
# LZSS
###

lzss.o: lzss.h lzss.cpp

test_lzss: test_lzss.o lzss.o

test_lzss.o: lzss.h test_lzss.cpp

###
# Utils
###

clean:
	rm -f *.o test_lzss

all: test_lzss
