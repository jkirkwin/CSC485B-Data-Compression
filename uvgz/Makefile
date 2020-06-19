EXTRA_CXXFLAGS=
EXTRA_CFLAGS=
CXXFLAGS=-O3 -Wall -std=c++17 $(EXTRA_CXXFLAGS)
CFLAGS=-O3 -Wall -std=c11 $(EXTRA_CFLAGS)
CXX=g++
CC=$(CXX) # Hack to allow implicit rules to link with g++

TEST_BINARIES=
BINARIES=$(TEST_BINARIES) uvgz

all: main

###
# LZSS
###
lzss.o: lzss.cpp lzss.h lzss_backref.h shared/binary.h shared/output_stream.hpp

###
# Prefix
###
prefix.o: prefix.cpp prefix.h shared/binary.h

###
# GZip
###
gzip.o: gzip.cpp gzip.h lzss.o prefix.o shared/output_stream.hpp prefix.h
###
# Main
###
main: main.cpp gzip.o
	$(CXX) -c $(CXXFLAGS) main.cpp
	$(CXX) -o uvgz $(CXXFLAGS) main.o gzip.o lzss.o prefix.o

clean:
	rm -f *.o $(BINARIES)

# TODO Holy god there are a lot of compiler warnings.

# TODO enable debug/asserts - add a new target