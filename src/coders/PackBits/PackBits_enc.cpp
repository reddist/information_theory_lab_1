#include <cstdint>
#include <vector>
#include "coder.h"
using namespace std;

void RLE_PackBits(const vector<unsigned char>& in,
                  vector<unsigned char>& out) {
    const size_t n = in.size();

    auto get_same_symbol_count = [in, n](size_t start) -> size_t {
        size_t current = start + 1;
        size_t count = 1;
        while (
            current < n &&
            in[current] == in[start] &&
            count < 128
        ) {
            ++current;
            ++count;
        }
        return count;
    };
    
    size_t i = 0;
    size_t same_already_counted = 0;
    size_t same_symbol_count;

    while (i < n) {
        if (same_already_counted > 0) {
            same_symbol_count = same_already_counted;
            same_already_counted = 0;
        } else {
            same_symbol_count = get_same_symbol_count(i);
        }

        if (same_symbol_count >= 3) {
            size_t minus_counter = -(same_symbol_count - 1);
            out.push_back(static_cast<unsigned char>(static_cast<int8_t>(minus_counter)));
            out.push_back(in[i]);
            i += same_symbol_count;
            continue;
        }

        size_t j = i + same_symbol_count;
        size_t uncoded_count = same_symbol_count;
        size_t same_symbol_count_j;
        
        while (j < n) {
            same_symbol_count_j = get_same_symbol_count(j);
            
            if (same_symbol_count_j >= 3) {
                same_already_counted = same_symbol_count_j;
                break;
            }

            j += same_symbol_count_j;
            uncoded_count += same_symbol_count_j;
            
            if (uncoded_count >= 128) {
                same_already_counted = uncoded_count - 128;
                uncoded_count = 128;
                break;
            }
        }

        size_t plus_counter = uncoded_count - 1;
        out.push_back(static_cast<unsigned char>(static_cast<int8_t>(plus_counter)));
        out.insert(out.end(), in.begin() + i, in.begin() + i + uncoded_count);
        i += uncoded_count;
    }
}