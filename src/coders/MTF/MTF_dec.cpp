#include <cstdint>
#include <vector>
#include "coder.h"
using namespace std;

bool MTF_dec(const vector<unsigned char>& in,
             vector<unsigned char>& out) {
    unsigned char table[256];
    for (int i = 0; i < 256; ++i) {
        table[i] = static_cast<unsigned char>(i);
    }

    const size_t n = in.size();
    out.reserve(out.size() + n);

    for (size_t i = 0; i < n; ++i) {
        const size_t index = static_cast<size_t>(in[i]);
        const unsigned char symbol = table[index];

        out.push_back(symbol);

        for (size_t j = index; j > 0; --j) {
            table[j] = table[j - 1];
        }
        table[0] = symbol;
    }

    return true;
}
