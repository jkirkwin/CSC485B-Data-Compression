/* uvcompress.cpp
   CSC 485B/CSC 578B/SENG 480B

   Placeholder starter code for UVCompress 

   B. Bird - 05/07/2020
*/

#include <iostream>

int main(){

    //This placeholder code reads bytes from stdin and writes the bitwise complement
    //to stdout (obviously this is useless for the assignment, but maybe it gives some impression
    //of how I/O works in C++)

    char c;
    //The .get() method of std::istream objects (like the standard input stream std::cin) reads a single unformatted
    //byte into the provided character (notice that c is passed by reference and is modified by the method).
    //If the byte cannot be read, the return value of the method is equivalent to the boolean value 'false'.
    while(std::cin.get(c)){

        //The .put() method of std::ostream objects (like std::cout) writes a single unformatted byte.
        char c_complement = ~c;
        std::cout.put(c_complement);
    }

    return 0;
}