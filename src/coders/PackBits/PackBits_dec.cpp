#include <cstdint>
#include <vector>
#include "coder.h"
using namespace std;

bool RLE_UnpackBits(const vector<unsigned char>& in,
                    vector<unsigned char>& out) {
    const size_t n = in.size();
    size_t i = 0;
    while (i < n) {
        const int8_t h = static_cast<int8_t>(in[i++]);

        if (h == -128) {
            continue;
        }

        if (h >= 0) {
            const size_t count = static_cast<size_t>(h) + 1;
            if (i + count > n) return false;
            out.insert(out.end(), in.begin() + i, in.begin() + i + count);
            i += count;
        } else {
            const size_t count = static_cast<size_t>(-static_cast<int>(h)) + 1;
            if (i + 1 > n) return false;
            const unsigned char byte = in[i++];
            out.insert(out.end(), count, byte);
        }
    }
    return true;
}