#include <cstdint>
#include <vector>
#include "coder.h"
using namespace std;

void MTF_enc(const vector<unsigned char>& in,
             vector<unsigned char>& out) {
    unsigned char table[256];
    for (int i = 0; i < 256; ++i) {
        table[i] = static_cast<unsigned char>(i);
    }

    const size_t n = in.size();
    out.reserve(out.size() + n);

    for (size_t i = 0; i < n; ++i) {
        const unsigned char symbol = in[i];

        size_t index = 0;
        while (table[index] != symbol) {
            ++index;
        }

        out.push_back(static_cast<unsigned char>(index));

        for (size_t j = index; j > 0; --j) {
            table[j] = table[j - 1];
        }
        table[0] = symbol;
    }
}
