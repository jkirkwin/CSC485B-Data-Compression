# Assignment 3 - Custom Compression Pipeline (`uvzz`)
Jamie Kirkwin

## Compression Pipeline Design
The pipeline implemented here is based on and aims to out-perform BZip2. Unfortunately I wasn't actually able to meet 
that goal. I think if I had managed an O(n) BWT implementation then I could have increased the block size enough to get 
close, but no cigar this time.

I also didn't end up having time to integrate Bill's arithmetic coding properly, so I left it out. The code is in an 
almost unaltered form in `arith/arith.cpp`/`.h`

Data is read in blocks of a fixed size. 
Each block is then passed through each stage of the pipeline:
0. An initial simple RLE component comes first. This is used to cut down on the time required for the BWT step for inputs with many duplicated characters.
0. A Burrows-Wheeler transform is run on the block to produce a permuted, compressable version of the data and an index needed for decompression.
0. A Move-to-Front transform is performed next to prepare the block for the following specialized RLE step.
0. The RLE component comes next, encoding only runs of zeros. This is effective because the previous Move-To-Front transform results in runs of zeros whenever its input contains runs of any symbol.

__todo__ add entropy coding item here (and below) once it is in place.
<!-- 0. Finally, entropy coding is done with Arithmetic Coding to squeeze whatever else we can out of the input.
-->
## Implementation

For more documentation of any of the component pieces listed below, have a look at the associated header files (e.g. 
`bwt/bwt.h` for the BWT implementation). All public functions are well documented.

### BWT
I went with a pretty naive implementation of the BWT because the papers describing Ukkonen's and McCreight's algorithms
are very difficult for me to understand and I didn't see the extra notes Bill posted on ConneX until too late.
It uses linear space complexity by using indexes into input string and takes O(nlog(n)) to sort, plus O(n) per 
comparison, so O(n^2 * log(n)) time complexity.

Decoding comes straight from the original paper and is O(n + 256) time and space.

See `bwt/bwt.cpp`

### Move-to-Front 
The Move-to-Front transform implementation (see `mtf/mtf.cpp`) is very simple. I opted to use an array-based index table
to increase the locality of the memory needed compared to a `std::list` in the hopes of better cache performance. A list
would have improved the speed of the interior deletions at the cost of increased time to traverse and find the element 
to delete when moving a symbol to the front of the table. The actual implementation is very simple and can be understood
easily from the code and inline documentation.

### RLE

#### RLE1 (Variable Byte)
The first RLE component is implemented using a conditional, variable byte length encoding scheme to preserve the byte-
orientation of the input data. After the required number of occurrences a length field is encoded into a variable number 
of bytes, where the first bit of each is a flag to indicate whether it is the last byte in the length field, and the 
other 7 bits are the next 7 bits of the binary representation of the encoded length. 

This scheme encodes runs of any character.

See `rle::vb` in `rle/rle.cpp`

#### RLE2 (Variable Bit)
The second RLE component is implemented using a conditional, variable bit length encoding scheme to balance the overhead
for large and small runs. Similar to a scheme shown in class, a unary encoding of the number of bits followed by a 
subset of those bits is encoded after each run of a sufficient amount of duplicate 0s. 

This scheme only encodes 0s.

`rle/rle.cpp`

## Bitstream

The first two bytes are the magic number which identifies the file type.
After this comes a sequence of encoded blocks.

Each block has the form
- Variable Byte encoding of BWT index
- Variable Byte encoding of delta between BWT index and the size of the BWT block to be used
    - This is required due to the initial RLE phase which makes us have different block sizes depending on the input.
- Encoded block data
    - This is a sequence of byte literals and variable length bit fields representing RLE2 lengths.
        - Everything except for these length fields is pushed from LSB to MSB, but the length fields are pushed MSB first.

__todo__
- May need arithmetic coding freq table depending on design choices (e.g. dynamic vs not)
- Should RLE output be byte-oriented in order to improve arithmetic coding? We need to be using some sort of symbols for this.
    - What about using an escape sequence followed by a run length
        - Decompressor would know when to kick out and interpret a length field directly 
            - if arithmetic decoder yields N consecutive zeros, call some function to pull bits from the stream until 
            length field is terminated.
                - this couples the arithmetic decoder to RLE but simplifies things otherwise

## Sources
- Boost library
- Bill's arithmetic coding implementation and bitstream classes
- SO posts used are linked in the code where they were used
- https://en.wikipedia.org/wiki/Move-to-front_transform
- The original BWT paper
