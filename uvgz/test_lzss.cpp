#define CATCH_CONFIG_MAIN

#include "catch/catch.hpp"
#include "lzss.h"
#include <iostream>
#include <queue>

// todo move this to a shared testing header file
template <class T>
class MockDownstreamConsumer {

public:

    /**
     * Gets a function that can be passed to the upstream source to inject the
     * Mock.
     * @return A std::function<void(T)> wrapper around this->getDatum()
     */
    std::function<void(bitset)> getConsumerFunction() {
        using namespace std::placeholders;
        return std::bind(&MockDownstreamConsumer::acceptDatum, this, _1);
    }

    void acceptDatum(T datum) {
        receivedData = true;
        q.push(datum);
    }

    /**
     * @return True if every input datum has been removed.
     */
    bool outputDepleted() {
        return q.empty();
    }

    /**
     * false iff no input has been received.
     */
    bool receivedData = false;

    /**
     * Throw away any stored elements. outputDepleted() will return true after
     * this (assuming no input is received first).
     */
    void clearData() {
        // https://stackoverflow.com/questions/709146/how-do-i-clear-the-stdqueue-efficiently
        std::queue<T> empty;
        q.swap(empty);
    }

    /**
     * Get the next datum in the buffer and remove it.
     */
    T popNextDatum() {
        auto front = peekNextDatum();
        q.pop();
        return front;
    }

    /**
     * @return The next datum in the buffer.
     */
    T peekNextDatum() {
        return q.front();
    }

private:
    std::queue<T> q;
};

// todo add clion configuration to run all of these tests at once

void requireEqualBackref(u32 len, u32 dist, MockDownstreamConsumer<bitset>& downstreamConsumer) {
    REQUIRE(!downstreamConsumer.outputDepleted());
    const auto lengthBase = downstreamConsumer.popNextDatum();
    REQUIRE(!downstreamConsumer.outputDepleted());
    const auto lengthOffset = downstreamConsumer.popNextDatum();
    REQUIRE(!downstreamConsumer.outputDepleted());
    const auto distanceBase = downstreamConsumer.popNextDatum();
    REQUIRE(!downstreamConsumer.outputDepleted());
    const auto distanceOffset = downstreamConsumer.popNextDatum();

    REQUIRE(getBackrefLength(lengthBase, lengthOffset) == len);
    REQUIRE(getBackrefDistance(distanceBase, distanceOffset) == dist);
}

TEST_CASE("Backref length conversions are correct", "[lzss] [backref]") {
    SECTION("Min length") {
        const auto length = 3;
        const bitset baseSymbol(LENGTH_BASE_BITS, 257);
        const bitset offsetSymbol;

        SECTION("convert symbols -> length") {
            REQUIRE(getBackrefLength(baseSymbol, offsetSymbol) == length);
        }
        SECTION("convert length -> symbols") {
            const auto backref = getLengthBackref(length);
            REQUIRE(backref.first == baseSymbol);
            REQUIRE(backref.second == offsetSymbol);
        }
    }
    SECTION("Middling length, min offset") {
        const auto length = 43;
        const bitset baseSymbol(LENGTH_BASE_BITS, 274);
        const bitset offsetSymbol(3, 0);

        SECTION("convert symbols -> length") {
            REQUIRE(getBackrefLength(baseSymbol, offsetSymbol) == length);
        }
        SECTION("convert length -> symbols") {
            const auto backref = getLengthBackref(length);
            REQUIRE(backref.first == baseSymbol);
            REQUIRE(backref.second == offsetSymbol);
        }
    }
    SECTION("Middling length, middling offset") {
        const auto length = 46;
        const bitset baseSymbol(LENGTH_BASE_BITS, 274);
        const bitset offsetSymbol(3, 3);

        SECTION("convert symbols -> length") {
            REQUIRE(getBackrefLength(baseSymbol, offsetSymbol) == length);
        }
        SECTION("convert length -> symbols") {
            const auto backref = getLengthBackref(length);
            REQUIRE(backref.first == baseSymbol);
            REQUIRE(backref.second == offsetSymbol);
        }
    }
    SECTION("Max length") {
        const auto length = 258;
        const bitset baseSymbol(LENGTH_BASE_BITS, 285);
        const bitset offsetSymbol;

        SECTION("convert symbols -> length") {
            REQUIRE(getBackrefLength(baseSymbol, offsetSymbol) == length);
        }
        SECTION("convert length -> symbols") {
            const auto backref = getLengthBackref(length);
            REQUIRE(backref.first == baseSymbol);
            REQUIRE(backref.second == offsetSymbol);
        }
    }
}

TEST_CASE("Backref distance conversions are correct", "[lzss] [backref]") {
    SECTION("Min distance") {
        const auto dist = 1;
        const bitset baseSymbol(DISTANCE_BASE_BITS, 0);
        const bitset offsetSymbol;

        SECTION("convert symbols -> length") {
            REQUIRE(getBackrefDistance(baseSymbol, offsetSymbol) == dist);
        }
        SECTION("convert length -> symbols") {
            const auto backref = getDistanceBackref(dist);
            REQUIRE(backref.first == baseSymbol);
            REQUIRE(backref.second == offsetSymbol);
        }
    }
    SECTION("Max distance") {
        const auto dist = 32768u;
        const bitset baseSymbol(5u, 29);
        const bitset offsetSymbol(13u, 0x1FFF);

        SECTION("convert symbols -> length") {
            REQUIRE(getBackrefDistance(baseSymbol, offsetSymbol) == dist);
        }
        SECTION("convert length -> symbols") {
            const auto backref = getDistanceBackref(dist);
            REQUIRE(backref.first == baseSymbol);
            REQUIRE(backref.second == offsetSymbol);
        }
    }
}

TEST_CASE("Flush empties buffer to the output function", "[lzss] [flush]") {
    MockDownstreamConsumer<bitset> mockConsumer; // todo make this and the encoder test fixtures
    LzssEncoder::symbol_consumer_t consumerFunction = mockConsumer.getConsumerFunction();
    LzssEncoder encoder(consumerFunction);

    SECTION("No input -> No output") {
        encoder.flush();
        REQUIRE(!mockConsumer.receivedData);
    }
    SECTION("Single byte in -> Single literal out") {
        const u8 input = 0x4F;
        const bitset oracle(LITERAL_BITS, input);

        encoder.acceptByte(input);
        REQUIRE(!mockConsumer.receivedData);

        encoder.flush();

        REQUIRE(mockConsumer.receivedData);
        REQUIRE(mockConsumer.popNextDatum() == oracle);
        REQUIRE(mockConsumer.outputDepleted());
    }
    SECTION("N inputs -> N literals") {
        const u8 limit = 0x0F;

        for (u8 input = 0; input <= limit; ++input) {
            encoder.acceptByte(input);
        }
        REQUIRE(!mockConsumer.receivedData);

        encoder.flush();
        for(u8 val = 0; val <= limit; ++val) {
            const bitset oracle(LITERAL_BITS, val);
            REQUIRE(mockConsumer.popNextDatum() == oracle);
        }
        REQUIRE(mockConsumer.outputDepleted());
    }
}

TEST_CASE("Obvious patterns in simple stream yield back-references", "[backref] [lzss]") {
    MockDownstreamConsumer<bitset> mockConsumer; // todo make this and the encoder test fixtures
    LzssEncoder::symbol_consumer_t consumerFunction = mockConsumer.getConsumerFunction();
    LzssEncoder encoder(consumerFunction);

    SECTION("2 * 10 character pattern -> one backref") {
        const std::string pattern = "1234567890";
        for (u8 byte : pattern) {
            encoder.acceptByte(byte);
        }
        for (u8 byte : pattern) {
            encoder.acceptByte(byte);
        }
        REQUIRE(!mockConsumer.receivedData);

        // Relies on flush working.
        encoder.flush();
        REQUIRE(mockConsumer.receivedData);

        for (u8 byte : pattern) {
            const bitset oracle(LITERAL_BITS, byte);
            REQUIRE(oracle == mockConsumer.popNextDatum());
        }

        requireEqualBackref(pattern.length(), pattern.length(), mockConsumer);
        REQUIRE(mockConsumer.outputDepleted());
    }
    SECTION("RLE Sucessful") {
        const u8 first = 0x0F;
        const u8 repeated = 0xF0;
        const int repeatCount = 15;
        const u8 last = 0xFF;

        encoder.acceptByte(first);
        for(int i = 0; i < repeatCount; ++i) {
            encoder.acceptByte(repeated);
        }
        encoder.acceptByte(last);
        REQUIRE(! mockConsumer.receivedData);

        encoder.flush();
        REQUIRE(mockConsumer.receivedData);

        auto firstResult = mockConsumer.popNextDatum();
        REQUIRE(firstResult == bitset(LITERAL_BITS, first));

        auto firstRepeated = mockConsumer.popNextDatum();
        REQUIRE(firstRepeated == bitset(LITERAL_BITS, repeated));

        // Now we should see one long backreference
        const auto expectedLength = repeatCount - 1;
        const auto expectedDistance = 1;
        requireEqualBackref(expectedLength, expectedDistance, mockConsumer);

        auto lastResult = mockConsumer.popNextDatum();
        REQUIRE(lastResult == bitset(LITERAL_BITS, last));

        REQUIRE(mockConsumer.outputDepleted());
    }
}
