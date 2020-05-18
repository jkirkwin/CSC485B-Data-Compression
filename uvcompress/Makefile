CXXFLAGS=-O3 -Wall -std=c++17

uvcompress: lzw_encoder compress_fs_encoder binary_field uvcompress.cpp
	$(CXX) -c $(CXXFLAGS) uvcompress.cpp
	$(CXX) -o uvcompress $(CXXFLAGS) uvcompress.o lzw_encoder.o compress_fs_encoder.o binary_field.o 
	# TODO clean this up

all: uvcompress test

test: test_binary_field test_fs_encoder

lzw_encoder: lzw_encoder.h lzw_encoder.cpp binary_field
	$(CXX) -c $(CXXFLAGS) lzw_encoder.cpp

compress_fs_encoder: compress_fs_encoder.h compress_fs_encoder.cpp
	$(CXX) -c $(CXXFLAGS) compress_fs_encoder.cpp

test_fs_encoder: compress_fs_encoder test_compress_fs_encoder.cpp binary_field
	$(CXX) -c $(CXXFLAGS) test_compress_fs_encoder.cpp
	$(CXX) -o test_fs_encoder $(CXXFLAGS) test_compress_fs_encoder.o binary_field.o compress_fs_encoder.o
	./test_fs_encoder

binary_field: binary_field.cpp binary_field.h
	$(CXX) -c $(CXXFLAGS) binary_field.cpp

test_binary_field: binary_field test_binary_field.cpp
	$(CXX) -c $(CXXFLAGS) test_binary_field.cpp
	$(CXX) -o test_binary_field $(CXXFLAGS) test_binary_field.o binary_field.o
	./test_binary_field

clean:
	rm -f uvcompress test_binary_field *.o test*.exe *.stackdump vgcore*
