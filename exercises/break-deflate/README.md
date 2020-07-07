The goal of this exercise is to create an input file `broken.bin` of between 10kb and 10mb such that the output of `gzip -9 broken.bin` is at least 5 times larger than the output of `bzip2 -9 broken.bin`.

Key idea: `bzip2 -9` causes BZip2 to use a much larger block size than the history buffer used in the `gzip` DEFLATE implementation. If we can construct a highly repetitive input in which the pattern(s) fit inside a BZip2 block but not within a `gzip` history buffer, then we can get much better performance with BZip. In order to decrease the likelilhood of smaller accidental repetitions inside the larger pattern we generate random data for the pattern.

_Question_: Are more numerous but shorter patterns better (for BZip2) or do fewer, longer patterns give better compression?
_Answer_: From experimentation (using patterns of only one more than the GZip look-behind and 3 times that amount) more numerous but shorter patterns are better.


A 900kb file yields a ratio of 14.1 which is much larger than we needed to begin with!
