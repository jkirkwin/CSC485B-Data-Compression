EXTRA_CXXFLAGS=-I include
EXTRA_CFLAGS=
CXXFLAGS=-O3 -Wall -std=c++17 $(EXTRA_CXXFLAGS)
CFLAGS=-O3 -Wall -std=c11 $(EXTRA_CFLAGS)
CXX=g++
CC=$(CXX) # Hack to allow implicit rules to link with g++
OBJECTS=rle.o bwt.o mtf.o
TEST_BINARIES=
BINARIES=$(TEST_BINARIES) uvzz_compress uvzz_decompress

all: uvzz_compress uvzz_decompress

uvzz_compress: uvzz_compress.cpp $(OBJECTS)
	$(CXX) -c $(CXXFLAGS) uvzz_compress.cpp
	$(CXX) -o uvzz_compress $(CXXFLAGS) uvzz_compress.o $(OBJECTS)

uvzz_decompress: uvzz_decompress.cpp $(OBJECTS)
	$(CXX) -c $(CXXFLAGS) uvzz_decompress.cpp
	$(CXX) -o uvzz_decompress $(CXXFLAGS) uvzz_decompress.o $(OBJECTS)

rle.o: rle/rle.cpp rle/rle.h include/binary.h
	$(CXX) -c -o rle.o $(CXXFLAGS) rle/rle.cpp

mtf.o: mtf/mtf.cpp mtf/mtf.h
	$(CXX) -c -o mtf.o $(CXXFLAGS) mtf/mtf.cpp

bwt.o: bwt/bwt.cpp bwt/bwt.h include/binary.h
	$(CXX) -c -o bwt.o $(CXXFLAGS) bwt/bwt.cpp

clean:
	rm -f *.o $(BINARIES)

rebuild: clean all
