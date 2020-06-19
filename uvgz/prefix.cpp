#include "prefix.h"
#include "lzss_backref.h"
#include <queue>

namespace huffman {

    typedef struct HuffmanTreeNode {
        explicit HuffmanTreeNode(int key, u32 weight, HuffmanTreeNode* left=nullptr, HuffmanTreeNode* right=nullptr):
            weight(weight), key(key), left(left), right(right) {

        }

        // True if this node has a lower weight than the one passed in
        bool operator<(const struct HuffmanTreeNode& other) const {
            return this->weight < other.weight;
        }

        bool operator>(const struct HuffmanTreeNode& other) const {
            return other < *this;
        }

        HuffmanTreeNode* left;
        HuffmanTreeNode* right;
        u32 weight;
        u32 key;
    } node_t;

    typedef struct HuffmanNodeComparator {
        bool operator()(node_t* first, node_t* second) {
            // Use > so that the lowest weights are at the front of the queue
            return first->weight > second->weight;
        }
    } comparator_t;

    void computeCodeLengths(const node_t* root, std::vector<u32> &codeLengths, const u32 depth=0) {
        assert (root != nullptr);

        bool hasLeftChild = root->left != nullptr;
        bool hasRightChild = root->right != nullptr;
        assert (hasLeftChild == hasRightChild); // No interior node should have a single child

        if (!hasLeftChild) { // Base case: leaf node
            codeLengths.at(root->key) += depth;
        }
        else { // Recursive case
            computeCodeLengths(root->left, codeLengths, depth + 1);
            computeCodeLengths(root->right, codeLengths, depth + 1);
        }
    }

    node_t* buildTree(const std::vector<u32>& weights) {
        // Build a priority queue of leaf nodes for all non-zero weights
        std::priority_queue<node_t*, std::vector<node_t*>, comparator_t> queue;
        for(int i = 0; i < weights.size(); ++i) {
            if (weights.at(i) > 0) {
                auto nodePtr = new node_t(i, weights.at(i));
                queue.push(nodePtr);
            }
        }

        // Build the tree
        while(queue.size() >= 2) {
            // Get the two smallest elements and combine them.
            auto first = queue.top();
            queue.pop();
            auto second = queue.top();
            queue.pop();
            auto combined = new node_t(-1, first->weight + second->weight, first, second);
            queue.push(combined);
        }

        assert (queue.size() == 1);
        return queue.top();
    }

    void deleteTree(node_t* root) {
        if (root != nullptr) {
            deleteTree(root->left);
            deleteTree(root->right);
            delete root;
        }
    }

    std::vector<u32> getCodeLengths(std::vector<u32>& weights) {
        auto root = buildTree(weights);
        std::vector<u32> codeLengths(weights.size(), 0);
        computeCodeLengths(root, codeLengths);

        deleteTree(root);

        return codeLengths;
    }
}



// LL codeword lengths for type 1:
// 0-143 -> 8 bits
// 144-255 -> 9 bits
// 256-279 -> 7 bits
// 280-287 -> 8 bits
constexpr std::array<std::array<u32, 3>, 4> fixedLLCodeLengths {{
    {{0, 143, 8}},
    {{144, 255, 9}},
    {{256, 279, 7}},
    {{280, 287, 8}}
}};

/**
 * To be used with constructCanonicalCode()
 * @return The lengths used for the fixed block type 1 LL code.
 */
std::vector<u32> getFixedLLCodeLengths() {
    std::vector<u32> lengths {};

    const auto entryCount = fixedLLCodeLengths.size();
    const auto lastEntry = fixedLLCodeLengths.at(entryCount - 1);
    const auto totalLengths = lastEntry.at(1) + 1;
    lengths.reserve(totalLengths);

    for (auto range : fixedLLCodeLengths) {
        const auto rangeStart {range.at(0)};
        const auto rangeEnd {range.at(1)};
        const auto bits {range.at(2)};
        for(int i = rangeStart; i <= rangeEnd; i++) {
            lengths.push_back(bits);
        }
    }
    return lengths;
}

std::vector<bitset> getFixedLLCode() {
    return constructCanonicalCode(getFixedLLCodeLengths());
}

/**
 * To be used with constructCanonicalCode()
 * @return The lengths used for the fixed block type 1 Distance code.
 */
std::vector<u32> getFixedDistanceCodeLengths() {
    // 32 lengths at 5 bits each
    std::vector<u32> lengths {};
    lengths.reserve(32);
    const u32 bits = 5;

    for(int i = 0; i < 32; i++) {
        lengths.push_back(bits);
    }
    return lengths;
}

std::vector<bitset> getFixedDistanceCode() {
    return constructCanonicalCode(getFixedDistanceCodeLengths());
}

// Minimally adapted from the block2 starter code provided by Bill.
std::vector<bitset> constructCanonicalCode(std::vector<u32> const & lengths){

    unsigned int size = lengths.size();
    std::vector< unsigned int > length_counts(16,0); //Lengths must be less than 16 for DEFLATE
    u32 max_length = 0;
    for(auto i: lengths){
        assert(i <= 15);
        length_counts.at(i)++;
        max_length = std::max(i, max_length);
    }
    length_counts.at(0) = 0; //Disregard any codes with alleged zero length

    std::vector< bitset > result_codes(size);

    //The algorithm below follows the pseudocode in RFC 1951
    std::vector< unsigned int > next_code(size,0);
    {
        //Step 1: Determine the first code for each length
        unsigned int code = 0;
        for(unsigned int i = 1; i <= max_length; i++){
            code = (code+length_counts.at(i-1))<<1;
            next_code.at(i) = code;
        }
    }
    {
        //Step 2: Assign the code for each symbol, with codes of the same length being
        //        consecutive and ordered lexicographically by the symbol to which they are assigned.
        for(unsigned int symbol = 0; symbol < size; symbol++){
            unsigned int length = lengths.at(symbol);
            if (length > 0) {
                result_codes.at(symbol) = bitset(length, next_code.at(length)++);
            }
        }
    }
    return result_codes;
}

std::pair<freq_table_t, freq_table_t> getLzssSymbolFrequencies(const bitset_vec& lzssSymbols) {
    // todo consider using arrays instead

    // 285 entries for LL code
    freq_table_t llFreqs(285);

    // 29 entries for Dist code
    freq_table_t distFreqs(29);

    for(int i = 0; i < lzssSymbols.size(); ++i) {
        const auto symbol = lzssSymbols.at(i);
        if (symbol.size() != LITERAL_BITS) { // backref
            llFreqs.at(symbol.to_ulong())++;

            const auto distSymbol = lzssSymbols.at(i+2);
            distFreqs.at(distSymbol.to_ulong())++;

            assert (lzssSymbols.size() > i+3); // Sanity check
            i+=3; // Skip past the rest of the backreference.
        }
        else { // literal
            llFreqs.at(symbol.to_ulong())++;
        }
    }

    // Since the EOB marker is not in the LZSS input in my implementation, we must manually add it here.
    assert(llFreqs.at(256) == 0);
    llFreqs.at(256)++;

    return {llFreqs, distFreqs};
}

std::vector<u32> frequenciesToLengths(const std::vector<u32>& frequencies) {
    // todo this is where we need to run huffman.
    // we will need to run huffman again later for CL code, so it must be generic.
}

std::pair<std::vector<u32>, std::vector<u32>> getDynamicCodeLengths(const bitset_vec& lzssSymbols) {
//    const auto freqs = getLzssSymbolFrequencies(lzssSymbols);
//    const auto llFreqs = freqs.first;
//    const auto distFreqs = freqs.second;
//
//    const auto llCodeLengths = frequenciesToLengths(llFreqs);
//    const auto distCodeLengths = frequenciesToLengths(distFreqs);

    // todo actually generate dynamic codes



    // Using dummy lengths for now
    std::vector<u32> llCodeLengths;
    for(unsigned int i = 0; i <= 225; i++) {
        llCodeLengths.push_back(8);
    }
    for(unsigned int i = 226; i <= 285; i++) {
        llCodeLengths.push_back(9);
    }

    // Using dummy lengths for now
    std::vector<u32> distCodeLengths;
    for(unsigned int i = 0; i <= 1; i++) {
        distCodeLengths.push_back(4);
    }
    for(unsigned int i = 2; i <= 29; i++) {
        distCodeLengths.push_back(5);
    }

    return {llCodeLengths, distCodeLengths};
}

std::vector<u32> getCLCodeLengths(const std::vector<u32> &llCodeLengths, const std::vector<u32> &distCodeLengths) {
    // todo implement this properly
    return {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5};
}