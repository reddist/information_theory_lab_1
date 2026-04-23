#include <vector>
#include <cmath>
#include <fstream>
#include <cstdio>
#include <iostream>
#include <unordered_map>
#include <tuple>

using namespace std;

// Хеш для pair
struct PairHash {
    size_t operator()(const pair<unsigned char, unsigned char>& p) const {
        return (p.first << 8) | p.second;
    }
};

// Хеш для tuple
struct TripleHash {
    size_t operator()(const tuple<unsigned char, unsigned char, unsigned char>& t) const {
        return (get<0>(t) << 16) | (get<1>(t) << 8) | get<2>(t);
    }
};

struct EntropyResult {
    double Hx;
    double Hx_given_prev;
    double Hx_given_prev2;
};

EntropyResult computeEntropy(const vector<unsigned char>& input) {
    size_t N = input.size();

    if (N == 0) return {0, 0, 0};

    // Частоты
    vector<size_t> count1(256, 0);
    unordered_map<pair<unsigned char, unsigned char>, size_t, PairHash> count2;
    unordered_map<tuple<unsigned char, unsigned char, unsigned char>, size_t, TripleHash> count3;

    // Подсчёт
    for (size_t i = 0; i < N; ++i) {
        count1[input[i]]++;

        if (i >= 1) {
            count2[{input[i - 1], input[i]}]++;
        }
        if (i >= 2) {
            count3[{input[i - 2], input[i - 1], input[i]}]++;
        }
    }

    // --- H(X) ---
    double Hx = 0.0;
    for (size_t c : count1) {
        if (c == 0) continue;
        double p = (double)c / N;
        Hx -= p * log2(p);
    }

    // --- H(X_n | X_{n-1}) ---
    double Hx_given_prev = 0.0;
    if (N > 1) {
        double totalPairs = N - 1;

        for (auto& kv : count2) {
            auto [prev, curr] = kv.first;
            double p_xy = kv.second / totalPairs;
            double p_y = (double)count1[prev] / N;

            Hx_given_prev -= p_xy * log2(p_xy / p_y);
        }
    }

    // --- H(X_n | X_{n-1}, X_{n-2}) ---
    double Hx_given_prev2 = 0.0;
    if (N > 2) {
        double totalTriples = N - 2;

        for (auto& kv : count3) {
            auto [a, b, c] = kv.first;
            double p_xyz = kv.second / totalTriples;

            double pair_count = count2[{a, b}];
            double p_yz = pair_count / (double)(N - 1);

            Hx_given_prev2 -= p_xyz * log2(p_xyz / p_yz);
        }
    }

    return {Hx, Hx_given_prev, Hx_given_prev2};
}


using namespace std;

int main(int argc, char** argv) {
    if (argc != 3) {
        cerr << "Usage: report.exe <input_file> <zipped_file>" << endl;
        return EXIT_FAILURE;
    }

    ifstream in(argv[1], ios::binary);
    if (!in) {
        cerr << "Error: cannot open input file '" << argv[1] << "'" << endl;
        return EXIT_FAILURE;
    }
    vector<unsigned char> input(
        (istreambuf_iterator<char>(in)),
        istreambuf_iterator<char>()
    );
    in.close();

    ifstream in_zip(argv[2], ios::binary);
    if (!in_zip) {
        cerr << "Error: cannot open zipped file '" << argv[2] << "'" << endl;
        return EXIT_FAILURE;
    }
    vector<unsigned char> zip(
        (istreambuf_iterator<char>(in_zip)),
        istreambuf_iterator<char>()
    );
    in_zip.close();

    EntropyResult result = computeEntropy(input);

    printf(
        "%-10s %-10s %-11s %-11s %-22s %-15s\n",
        "H(X)", "H(X|X)", "H(X|XX)",
        "Size(B)", "Compressed Size(B)", "Bits/symbol"
    );
    printf(
        "%-10.4f %-10.4f %-11.4f %-11d %-22d %-15.4f\n",
        result.Hx, result.Hx_given_prev, result.Hx_given_prev2,
        input.size(), zip.size(), zip.size() * 8.0 / input.size()
    );
}