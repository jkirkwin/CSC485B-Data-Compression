#include "lzss_backref.h"
#include "lzss.h"

void LzssEncoder::acceptByte(u8 item) {
    if (lookahead.full()) {
        encodeFromLookahead();
    }
    lookahead.push_back(item);
}

void LzssEncoder::flush() {
    while(!lookahead.empty()) {
        encodeFromLookahead();
    }
}

/*
 * Runs a naive linear search to find a backreference if possible. Otherwise
 * emits a literal.
 */
void LzssEncoder::encodeFromLookahead() {
    assert (!lookahead.empty());

    const auto minToMatch = 3;
    if (minToMatch <= lookahead.size()) {
        std::vector<u8> prefix;
        for (int i = 0; i < minToMatch; ++i) {
            prefix.push_back(lookahead.at(i));
        }
        auto backref = searchHistoryFor(prefix);
        if (backref.first == 0) { // No match found
            const auto next = popLookahead();
            outputLiteral(next);
        }
        else {
            auto len = backref.first;
            auto dist = backref.second;
            outputBackref(len, dist);
            for(int i = 0; i < len; ++i) {
                popLookahead();
            }
        }
    }
    else {
        const auto next = popLookahead(); // todo factor this out as a function
        outputLiteral(next);
    }
}

/*
 * Returns a {len, distance} pair for the first backreference found. If none is found,
 * length is set to 0.
 */
std::pair<u32, u32> LzssEncoder::searchHistoryFor(std::vector<u8> &sequence) {
    for(int i = 0; i < history.size(); ++i) {
        u32 matchStartIndex = history.size() - i - 1;

        // Check for a match starting here.
        bool match = true;
        for (int j = 0; j < sequence.size(); ++j) {
            // j is the index of the character in the lookahead
            auto historyIndex = matchStartIndex + j;
            if (! isMatch(historyIndex, j)) {
                match = false;
                break;
            }
        }

        if(match) { // Backreference found
            auto dist = i + 1;
            auto len = sequence.size();

            // look for more matches (make the backref as long as possible)
            u32 maxLookaheadIndex = lookahead.size() - 1;
            auto maxLen = std::min(MAX_BACKREF_LEN, maxLookaheadIndex);
            for (; len <= maxLen; ++len) {
                auto hIndex = matchStartIndex + len;
                auto lIndex = len;
                if (! isMatch(hIndex, lIndex)) {
                    break;
                }
            }

            return {len, dist};
        }
    }
    return {0, 0};
}

/*
 * Checks if the history buffer and lookahead agree on the value at the given
 * indexes. If the history index is larger than the size of the history buffer,
 * the difference will be used as an offset into the lookahead.
 *
 * E.g.     isMatch(history.size(), 0) will always return true.
 */
bool LzssEncoder::isMatch(u32 historyIndex, u32 lookaheadIndex) {
    u8 historyItem;
    if(historyIndex < history.size()) {
        historyItem = history.at(historyIndex);
    }
    else {
        auto index = historyIndex - history.size();
        historyItem = lookahead.at(index);
    }

    return historyItem == lookahead.at(lookaheadIndex);
}

void LzssEncoder::outputLiteral(u8 literalValue) {
    const bitset literalEncoding(LITERAL_BITS, literalValue);
    writeSymbol(literalEncoding);
}

void LzssEncoder::outputBackref(u32 len, u32 dist) {
    auto lenEncoding = getLengthBackref(len);
    auto distEncoding = getDistanceBackref(dist);

    writeSymbol(lenEncoding.first);
    writeSymbol(lenEncoding.second);
    writeSymbol(distEncoding.first);
    writeSymbol(distEncoding.second);
}

u8 LzssEncoder::popLookahead() {
        const auto front = lookahead.front();
        lookahead.pop_front();
        history.push_back(front);
        return front;
}

void LzssEncoder::reset() {
    this->history.clear();
    this->lookahead.clear();
}