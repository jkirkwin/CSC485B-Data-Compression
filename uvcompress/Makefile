CXXFLAGS=-O3 -Wall -std=c++17

all: uvcompress

test: test_binary_field

test_binary_field: binary_field test_binary_field.cpp
	$(CXX) -c $(CXXFLAGS) test_binary_field.cpp
	$(CXX) -o test_binary_field $(CXXFLAGS) test_binary_field.o binary_field.o
	./test_binary_field

binary_field: binary_field.cpp binary_field.h
	$(CXX) -c $(CXXFLAGS) binary_field.cpp

clean:
	rm -f uvcompress *.o test*.exe
