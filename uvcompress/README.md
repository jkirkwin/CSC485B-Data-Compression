# CSC 485B Assignment 1: `uvcompress`

The architecture of the code is simple. I have split the compression process into two portions:
1. Performing the LZW algorithm
2. Performing the bit-stream to byte-stream conversion as done by `compress`.

`uvcompress` creates an instance of `LzwEncoder` and an instance of `FSEncoder`, chains them together, and feeds the input (from stdin) into the `LzwEncoder`. 

When the `LzwEncoder` instance receives a character in, it *may* output a symbol (a chunk of bits of size 9-16 inclusive) in the form of a `BinaryField` which will then be received by the downstream `FSEncoder`. 

When the `FSEncoder` receives these bits, it performs the appropriate buffering, reversal, and tokenization into bytes to generate a bytestream which makes up the final result.

Once `uvcompress` has read the entirety of the input, it will flush the `LzwEncoder` to ensure that any working string is converted to an index and passed downstream. Then, the `FSEncoder` is flushed, ensuring that any buffered data is padded to size and outputted. 