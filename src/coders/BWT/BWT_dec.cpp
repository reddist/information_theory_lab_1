#include <cstdint>
#include <vector>
#include <iostream>
#include "coder.h"
using namespace std;

// Для чтения 4 байтов из потока (позиция primaryIndex)
static uint32_t readU32(const vector<unsigned char>& in, size_t off) {
    return (static_cast<uint32_t>(in[off]) << 24)
         | (static_cast<uint32_t>(in[off + 1]) << 16)
         | (static_cast<uint32_t>(in[off + 2]) << 8)
         | (static_cast<uint32_t>(in[off + 3]));
}

bool BWT_dec(const vector<unsigned char>& in,
             vector<unsigned char>& out) {
    const size_t n = in.size() - 4;
    const size_t total = n + 1;
    if (n <= 0) {
        return false;
    }
    const uint32_t primaryIndex = readU32(in, 0);
    if (primaryIndex > n) {
        return false;
    }

    const unsigned char* bwt = in.data() + 4;

    // Количество вхождений каждого символа в BWT
    vector<uint32_t> count(256, 0);
    for (uint32_t i = 0; i < n; ++i) {
        count[bwt[i]]++;
    }

    // Кумулятивная сумма вхождений символов, лексикографически меньших символа c
    vector<uint32_t> cumul(256, 0);
    uint32_t running = 1;
    for (int c = 0; c < 256; ++c) {
        cumul[c] = running;
        running += count[c];
    }

    // Частота символа с в подстроке BWT[0..j-1], постепенно увеличивается для каждого символа
    vector<uint32_t> freq(256, 0);
    // Индекс символа в полном BWT (с разделителем) + индекс следующего индекса L в изначальной строке с конца
    vector<uint32_t> L(total);
    for (size_t j = 0; j < total; ++j) {
        if (j == primaryIndex) {
            L[j] = 0;
            continue;
        }
        const unsigned char c = (j < primaryIndex) ? bwt[j] : bwt[j - 1];
        L[j] = freq[c] + cumul[c];
        freq[c]++;
    }

    out.resize(n);

    // Восстанавливаем строку с конца, прыгая по L
    uint32_t idx = L[primaryIndex];
    for (uint32_t i = 0; i < n; i++) {
        const unsigned char c = (idx < primaryIndex) ? bwt[idx] : bwt[idx - 1];
        out[n - i - 1] = c;
        idx = L[idx];
    }

    return true;
}
