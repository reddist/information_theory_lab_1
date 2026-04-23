#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
#include "coders/PackBits/coder.h"
#include "coders/AAC_D/coder.h"
#include "coders/MTF/coder.h"
#include "coders/BWT/coder.h"
using namespace std;

int main(int argc, char** argv) {
    if (argc != 3) {
        cerr << "Usage: decoder.exe <input_file> <output_file>" << endl;
        return EXIT_FAILURE;
    }

    ifstream in(argv[1], ios::binary);
    if (!in) {
        cerr << "Error: cannot open input file '" << argv[1] << "'" << endl;
        return EXIT_FAILURE;
    }



    // AAC decode
    uint32_t mtf_len = 0;
    uint32_t bit_len    = 0;
    in.read(reinterpret_cast<char*>(&mtf_len), 4);
    in.read(reinterpret_cast<char*>(&bit_len), 4);
    if (!in) {
        cerr << "Error: truncated header in '" << argv[1] << "'" << endl;
        return EXIT_FAILURE;
    }

    const size_t byte_len = bit_len / 8 + 1;
    vector<unsigned char> code(byte_len + 16, 0);
    in.read(reinterpret_cast<char*>(code.data()), static_cast<streamsize>(byte_len));
    in.close();

    vector<unsigned char> mtf(mtf_len);
    ac_decode_buffer(code.data(), mtf_len, mtf.data());



    // UnpackBits
    // vector<unsigned char> unpacked;
    // unpacked.reserve(static_cast<size_t>(packed_len) * 2);

    // if (!RLE_UnpackBits(packed, unpacked)) {
    //     cerr << "Error: truncated or malformed PackBits stream" << endl;
    //     return EXIT_FAILURE;
    // }



    // MTF decode
    vector<unsigned char> bwt;
    bwt.reserve(mtf.size());

    if (!MTF_dec(mtf, bwt)) {
        cerr << "Error: malformed MTF stream" << endl;
        return EXIT_FAILURE;
    }



    // BWT decode
    vector<unsigned char> output;

    if (!BWT_dec(bwt, output)) {
        cerr << "Error: truncated or malformed BWT stream" << endl;
        return EXIT_FAILURE;
    }



    ofstream out(argv[2], ios::binary);
    if (!out) {
        cerr << "Error: cannot open output file '" << argv[2] << "'" << endl;
        return EXIT_FAILURE;
    }
    if (!output.empty()) {
        out.write(reinterpret_cast<const char*>(output.data()),
                  static_cast<streamsize>(output.size()));
    }
    if (!out) {
        cerr << "Error: failed writing to '" << argv[2] << "'" << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
