CXXFLAGS=-O3 -Wall -std=c++17

all: uvcompress test

test: test_binary_field

uvcompress: lzw_encoder compress_fs_encoder binary_field uvcompress.cpp
	$(CXX) -c $(CXXFLAGS) uvcompress.cpp
	$(CXX) -o uvcompress $(CXXFLAGS) uvcompress.o lzw_encoder.o compress_fs_encoder.o binary_field.o 
	# TODO clean this up

lzw_encoder: lzw_encoder.h lzw_encoder.cpp compress_fs_encoder.h # TODO break this dependency
	$(CXX) -c $(CXXFLAGS) lzw_encoder.cpp

compress_fs_encoder: compress_fs_encoder.h compress_fs_encoder.cpp
	$(CXX) -c $(CXXFLAGS) compress_fs_encoder.cpp

test_binary_field: binary_field test_binary_field.cpp
	$(CXX) -c $(CXXFLAGS) test_binary_field.cpp
	$(CXX) -o test_binary_field $(CXXFLAGS) test_binary_field.o binary_field.o
	./test_binary_field

binary_field: binary_field.cpp binary_field.h
	$(CXX) -c $(CXXFLAGS) binary_field.cpp

clean:
	rm -f uvcompress *.o test*.exe *.stackdump
