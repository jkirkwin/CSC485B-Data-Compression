EXTRA_CXXFLAGS=-I include
CXXFLAGS=-O3 -Wall -std=c++17 $(EXTRA_CXXFLAGS)
CFLAGS=-O3 -Wall -std=c11 $(EXTRA_CFLAGS)
CXX=g++
CC=$(CXX) # Hack to allow implicit rules to link with g++
OBJECTS=delta.o dct.o # Todo rename lib
TEST_BINARIES=
BINARIES=$(TEST_BINARIES) uvid_compress uvid_decompress

all: uvid_compress uvid_decompress

uvid_compress: uvid_compress.o $(OBJECTS)
	$(CXX) -o uvid_compress $(CXXFLAGS) uvid_compress.o $(OBJECTS)

uvid_decompress: uvid_decompress.o $(OBJECTS)
	$(CXX) -o uvid_decompress $(CXXFLAGS) uvid_decompress.o $(OBJECTS)

dct.o: dct/dct.cpp dct/dct.h
	$(CXX) -c -o dct.o $(CXXFLAGS) dct/dct.cpp

delta.o: delta/delta.cpp delta/delta.h
	$(CXX) -c -o delta.o $(CXXFLAGS) delta/delta.cpp

clean:
	rm -f *.o $(BINARIES)

rebuild: clean all
