# CSC 485B Assignment 1: `uvcompress`

## General
This project is an implementation of the 
[LZW algorithm](https://en.wikipedia.org/wiki/Lempel%E2%80%93Ziv%E2%80%93Welch)
intended to mimic the unix [compress utility](https://en.wikipedia.org/wiki/Compress)'s 
encoder (decoding is not part of this project). The output is compatible with the compress utility's decoder (`compress -d`).

The main departure from the original utility (besides being slower and written in C++) is that this one does not use a reset flag to indicate to the decoder to clear its symbol table. This is omitted because there are historical bugs in the original utility. If the symbol table becomes full then it remains so and new strings are not added. This will likely have a noticable effect (i.e. a worse compression ratio) when compressing very large files.

## Disclaimer
While I am quite pleased that I got this working, I would like to note to anyone looking at the source code that this is my first time ever using C++ (and I haven't touched C in over a year) so I'm sure that there are a good number of things that could be done more efficiently or idiomatically. Bear with me, and hopefully the subsequent assignments in this repo will be in better style.


## Structure

<!-- http://asciiflow.com/ -->
```
                    +------------+   BinaryField   +------------+
+-------+   chars   |            |    instances    |            |   chars   +-------+
| stdin | +-------> | LZWEncoder | +-------------> | FSEncoder  | +-------> | stout |
+-------+           |            |                 |            |           +-------+
                    +------------+                 +------------+
```

The architecture of the code is simple. The compression is split into two pieces:
1. Performing the LZW algorithm (with some configuration/implementation decisions influenced by `compress`)
2. Performing the bit-stream to byte-stream conversion as done by `compress`.

`uvcompress` creates an instance of `LzwEncoder` and an instance of `FSEncoder`, chains them together, and feeds the input (from stdin) into the `LzwEncoder`. 

When the `LzwEncoder` instance receives a character in, it *may* output a symbol (a chunk of bits of size 9-16 inclusive) in the form of a `BinaryField` which will then be received by the downstream `FSEncoder`. 

When the `FSEncoder` receives these bits, it performs the appropriate buffering, reversal, and tokenization into bytes to generate a bytestream which makes up the final result.

Once `uvcompress` has read the entirety of the input, it will flush the `LzwEncoder` to ensure that any working string is converted to an index and passed downstream. Then, the `FSEncoder` is flushed, ensuring that any buffered data is padded to size and output. 

## Build and usage

I hacked together a Makefile from (distant) memory in what is certainly not best practice, but it seems to do the trick for this simple project.

Build the `uvcompress` utility with:
```
make
``` 

Run the tests with 
```
make test
```

Build the `uvcompress` utility and run the tests with:
```
make all
```

The program exclusively accepts input from `stdin` and output is sent to `stdout`.

## Documentation (To the marker:)
In addition to the high-level explaination of the different components above, most of the non-trivial functions and classes have docstrings that should make the code easily understandable.

The places where the code I wrote was influenced from an external source other than https://en.cppreference.com/ or http://www.cplusplus.com/reference have been marked with inline comments linking to the appropriate site.

## Testing
I added a handful of unit tests for the LZW encoder and BinaryField library, but didn't end up having time to cleanly test the FS encoder. 

I didn't want to bother installing boost on my windows machine, so I used asserts instead to do the verification. I will definitely not be doing that again.

`make test` will run the tests.