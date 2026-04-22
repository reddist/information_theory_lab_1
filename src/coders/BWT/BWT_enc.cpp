#include <cstdint>
#include <iostream>
#include <vector>
#include "coder.h"
using namespace std;



// SA-IS
static bool isLMS(const vector<bool>& t, int i) {
    return i > 0 && t[i] && !t[i - 1];
}

static void getBuckets(const vector<int>& bkt,
                       vector<int>& out,
                       int K,
                       bool end) {
    int sum = 0;
    for (int i = 0; i < K; ++i) {
        sum += bkt[i];
        out[i] = end ? sum : sum - bkt[i];
    }
}

static void induceSA(const vector<int>& s,
                     vector<int>& SA,
                     const vector<bool>& t,
                     const vector<int>& bkt,
                     int K) {
    const int n = static_cast<int>(s.size());
    vector<int> buckets(K);

    getBuckets(bkt, buckets, K, false);
    for (int i = 0; i < n; ++i) {
        const int j = SA[i] - 1;
        if (j >= 0 && !t[j]) {
            SA[buckets[s[j]]++] = j;
        }
    }

    getBuckets(bkt, buckets, K, true);
    for (int i = n - 1; i >= 0; --i) {
        const int j = SA[i] - 1;
        if (j >= 0 && t[j]) {
            SA[--buckets[s[j]]] = j;
        }
    }
}

static void SA_IS(const vector<int>& s, vector<int>& SA, int K) {
    const int n = static_cast<int>(s.size());
    SA.assign(n, -1);

    vector<bool> t(n);
    t[n - 1] = true;
    for (int i = n - 2; i >= 0; --i) {
        if (s[i] < s[i + 1]) {
            t[i] = true;
        } else if (s[i] > s[i + 1]) {
            t[i] = false;
        } else {
            t[i] = t[i + 1];
        }
    }

    vector<int> bkt(K, 0);
    for (int i = 0; i < n; ++i) {
        bkt[s[i]]++;
    }

    vector<int> buckets(K);
    getBuckets(bkt, buckets, K, true);
    for (int i = 0; i < n; ++i) {
        if (isLMS(t, i)) {
            SA[--buckets[s[i]]] = i;
        }
    }

    induceSA(s, SA, t, bkt, K);

    int n1 = 0;
    for (int i = 0; i < n; ++i) {
        if (isLMS(t, SA[i])) {
            SA[n1++] = SA[i];
        }
    }
    for (int i = n1; i < n; ++i) {
        SA[i] = -1;
    }

    int name = 0;
    int prev = -1;
    for (int i = 0; i < n1; ++i) {
        const int pos = SA[i];
        bool diff = false;
        if (prev == -1) {
            diff = true;
        } else {
            for (int d = 0; ; ++d) {
                if (pos + d >= n || prev + d >= n) {
                    diff = true;
                    break;
                }
                if (s[pos + d] != s[prev + d] || t[pos + d] != t[prev + d]) {
                    diff = true;
                    break;
                }
                if (d > 0) {
                    const bool a = isLMS(t, pos + d);
                    const bool b = isLMS(t, prev + d);
                    if (a && b) {
                        break;
                    }
                    if (a != b) {
                        diff = true;
                        break;
                    }
                }
            }
        }
        if (diff) {
            ++name;
            prev = pos;
        }
        SA[n1 + pos / 2] = name - 1;
    }

    vector<int> s1;
    s1.reserve(n1);
    for (int i = n1; i < n; ++i) {
        if (SA[i] != -1) {
            s1.push_back(SA[i]);
        }
    }

    vector<int> SA1(n1);
    if (name < n1) {
        SA_IS(s1, SA1, name);
    } else {
        for (int i = 0; i < n1; ++i) {
            SA1[s1[i]] = i;
        }
    }

    vector<int> lmsPositions;
    lmsPositions.reserve(n1);
    for (int i = 0; i < n; ++i) {
        if (isLMS(t, i)) {
            lmsPositions.push_back(i);
        }
    }
    for (int i = 0; i < n1; ++i) {
        SA1[i] = lmsPositions[SA1[i]];
    }

    for (int i = 0; i < n; ++i) {
        SA[i] = -1;
    }
    getBuckets(bkt, buckets, K, true);
    for (int i = n1 - 1; i >= 0; --i) {
        const int j = SA1[i];
        SA[--buckets[s[j]]] = j;
    }

    induceSA(s, SA, t, bkt, K);
}



// BWT

// Для перезаписи 4 байтов в поток на определенном месте
static inline void patchU32(vector<unsigned char>& out, size_t off, uint32_t v) {
    out[off + 0] = static_cast<unsigned char>((v >> 24) & 0xFF);
    out[off + 1] = static_cast<unsigned char>((v >> 16) & 0xFF);
    out[off + 2] = static_cast<unsigned char>((v >> 8) & 0xFF);
    out[off + 3] = static_cast<unsigned char>(v & 0xFF);
}


void BWT_enc(const vector<unsigned char>& in,
             vector<unsigned char>& out) {
    const size_t n = in.size();
    const size_t total = n + 1;

    out.resize(4 + n);
    patchU32(out, 0, 0);

    vector<int> T;
    T.reserve(total);
    for (size_t i = 0; i < n; ++i) {
        T.push_back(static_cast<int>(in[i]) + 1); // 0..255 -> 1..256
    }
    T.push_back(0);

    vector<int> SA;
    SA_IS(T, SA, 257);


    size_t bwt_curr = 4;
    uint32_t primaryIndex = 0;
    for (size_t i = 0; i < total; ++i) {
        if (SA[i] == 0) {
            primaryIndex = static_cast<uint32_t>(i);
            continue;
        }
        const int p = (SA[i] - 1 + total) % total;
        out[bwt_curr] = static_cast<unsigned char>(T[p] - 1); // 1..256 -> 0..255
        ++bwt_curr;
    }

    patchU32(out, 0, primaryIndex);
}
