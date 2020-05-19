#include <cassert>
#include "binary_field.h"
#include "lzw_encoder.h"
#include <iostream>
#include <queue>
#include <functional>
#include <limits.h>
#include <vector>

unsigned char getRandomChar() {
    unsigned char c = rand() % 0x100;
    return c;
}

class MockDownstreamEncoder {

    public:
        void acceptData(BinaryField bf) {
            receivedData = true;
            q.push(bf);
        }

        bool outputDepleted() {
            return q.empty();
        }

        bool receivedData = false;
        
        BinaryField getNextDatum() {
            auto front = q.front();
            q.pop();
            return front;
        }
    
    private:
        std::queue<BinaryField> q;
};

consumer_t getConsumer(MockDownstreamEncoder& e) {
    using namespace std::placeholders;
    return std::bind(&MockDownstreamEncoder::acceptData, &e, _1);
}

/*
 * No output should be sent if an encoder is created and immediately flushed.
 */ 
void testCreateFlushNoOutput() {
    MockDownstreamEncoder mdse;
    consumer_t adapter = getConsumer(mdse);
    LzwEncoder encoder(adapter);

    assert (! mdse.receivedData);
    encoder.flush();
    assert (! mdse.receivedData);
}

/*
 * Given a single character of input, the encoder shouldn't output anything 
 * until flushed. When flushed, a single symbol should be output which is equal
 * to the character input using 9 bits.
 */ 
void testOneCharNoOutputUntilFlush() {
    MockDownstreamEncoder mdse;
    consumer_t adapter = getConsumer(mdse);
    LzwEncoder encoder(adapter);

    encoder.acceptChar('a');
    assert (! mdse.receivedData);
    
    encoder.flush();
    assert (mdse.receivedData);

    auto result = mdse.getNextDatum();
    BinaryField expected ('a', 9);
    assert ( result == expected );
    assert ( mdse.outputDepleted() );
}
/*
 * Given an input abab, the first and and b should be output separately. When 
 * flushed, the second a and b should produce a single symbol which is the 
 * first open index in the symbol table after the reset symbol. 
 */  
void testTwoCharEntryInSymbolTable() {
    MockDownstreamEncoder mdse;
    consumer_t adapter = getConsumer(mdse);
    LzwEncoder encoder(adapter);

    unsigned char a = getRandomChar();
    unsigned char b = getRandomChar();

    // Feed in a, expect no output
    encoder.acceptChar(a);
    assert ( !mdse.receivedData );

    // Feed in b, expect a as output, ab should be in the table now.
    encoder.acceptChar(b);
    assert ( mdse.receivedData );
    
    auto expected = BinaryField(a, 9);
    auto received = mdse.getNextDatum();
    assert ( received ==  expected);
    assert ( mdse.outputDepleted() );

    // Feed in a, expect b as output, ba should be in the table now. 
    // Working string should be a.
    encoder.acceptChar(a);
    assert ( !mdse.outputDepleted() );
    assert ( mdse.getNextDatum() == BinaryField(b, 9) );
    assert ( mdse.outputDepleted() );

    // Feed in b, expect no output as ab should be in the table.
    encoder.acceptChar(b);
    assert ( mdse.outputDepleted() );

    // Flush the encoder, should output one symbol for ab.
    encoder.flush();
    assert ( !mdse.outputDepleted() );
    expected = BinaryField(257, 9);
    assert ( mdse.getNextDatum() == expected );
    assert ( mdse.outputDepleted() );
}

/* (From LZW slides, modified to accomodate for compress utility reset marker)
 *
 * Given the sequence "a salad; a salsa; alaska" the output (flushing the 
 * encoder after the final 'a') should be: 
 *          "a salad;[257][259]l[259][264][260]aska"
 * with the single characters representing their binary values.
 */
void testShortSequence() {
    MockDownstreamEncoder mdse;
    consumer_t adapter = getConsumer(mdse);
    LzwEncoder encoder(adapter);
    
    std::string input = "a salad; a salsa; alaska";
    // https://www.techiedelight.com/iterate-over-characters-string-cpp/
    for (char const& c: input) {
        encoder.acceptChar(c);
    }
    encoder.flush();
    
    std::vector<unsigned int> expectedValues 
        {'a', ' ', 's', 'a', 'l', 'a', 'd', ';', ' ', 257, 259, 'l', 259, 264,
         260, 'a', 's', 'k', 'a'};
    std::vector<BinaryField> expectedResults;
    for (unsigned int &v : expectedValues) {
        expectedResults.push_back(BinaryField(v, 9));
    }

    for (BinaryField& result : expectedResults) {
        assert (! mdse.outputDepleted());
        assert (mdse.getNextDatum() == result);
    }
    assert (mdse.outputDepleted());
}

/*
 * Once all indexes which can be made using 9 bits are allocated, the encoder
 * should start outputting 10-bit symbols.
 */ 
void testIncreaseNumBits() {
    assert (false); // todo
}

/**
 * Once the symbol table has grown to hold all possible 16-bit values, new 
 * strings should no longer be recorded.
 */
void testNumBitsNoOverflow() {
    assert (false); // todo
}

// todo refactor this (and update local env) to use boost test lib
int main() {
    std::cout << "LzwEncoder -- Running Tests...." << std::endl;
    
    auto seed = time(NULL);
    std::cout << "Seeding rand with " << seed << std::endl;
    srand(seed);
    
    testCreateFlushNoOutput();
    testOneCharNoOutputUntilFlush();
    testTwoCharEntryInSymbolTable();
    testShortSequence();
    // testIncreaseNumBits();
    // testNumBitsNoOverflow();

    std::cout << "LzwEncoder -- All Tests Passed!" << std::endl;

}