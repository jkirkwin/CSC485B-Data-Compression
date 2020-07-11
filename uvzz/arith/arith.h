#ifndef UVZZ_ARITH_H
#define UVZZ_ARITH_H

#include <iostream>

namespace arith {

    /**
     * Encodes the input available via the instream to the outstream.
     */
    void encode(std::istream &inStream = std::cin, std::ostream &outStream = std::cout);

    /**
     * Decodes input from the instream and sends the result to the outstream.
     */
    void decode(std::istream &inStream = std::cin, std::ostream &outStream = std::cout);
}

#endif //UVZZ_ARITH_H
