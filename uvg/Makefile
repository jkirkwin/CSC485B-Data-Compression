EXTRA_CXXFLAGS=-I include
CXXFLAGS=-O3 -Wall -std=c++17 $(EXTRA_CXXFLAGS)
CFLAGS=-O3 -Wall -std=c11 $(EXTRA_CFLAGS)
CXX=g++
CC=$(CXX) # Hack to allow implicit rules to link with g++
OBJECTS=delta.o dct.o
TEST_BINARIES=
BINARIES=$(TEST_BINARIES) uvg_compress uvg_decompress

all: uvg_compress uvg_decompress

uvg_compress: uvg_compress.o $(OBJECTS)
	$(CXX) -o uvg_compress $(CXXFLAGS) uvg_compress.o $(OBJECTS)

uvg_decompress: uvg_decompress.o $(OBJECTS)
	$(CXX) -o uvg_decompress $(CXXFLAGS) uvg_decompress.o $(OBJECTS)

dct.o: dct/dct.cpp dct/dct.h
	$(CXX) -c -o dct.o $(CXXFLAGS) dct/dct.cpp

delta.o: delta/delta.cpp delta/delta.h
	$(CXX) -c -o delta.o $(CXXFLAGS) delta/delta.cpp

clean:
	rm -f *.o $(BINARIES)

rebuild: clean all
