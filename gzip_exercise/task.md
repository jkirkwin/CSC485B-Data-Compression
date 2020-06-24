# Description

Create a file `hack.gz` such that
* it decompresses to exactly "V00875987" (9 bytes or 10 ending with newline).
* `gzip -dv` give no warnings or errors
* `hack.gz` is 875987 bytes

# Resources 
[RFC 1952 (GZip File Format)](https://tools.ietf.org/html/rfc1952)
* Section 2.3 describes the file format and shows that we can add an XLEN field or a comment
    * Comment fields must be 0-terminated.