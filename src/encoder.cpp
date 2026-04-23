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
        cerr << "Usage: encoder.exe <input_file> <output_file>" << endl;
        return EXIT_FAILURE;
    }

    ifstream in(argv[1], ios::binary);
    if (!in) {
        cerr << "Error: cannot open input file '" << argv[1] << "'" << endl;
        return EXIT_FAILURE;
    }



    // BWT encode
    vector<unsigned char> input(
        (istreambuf_iterator<char>(in)),
        istreambuf_iterator<char>()
    );
    in.close();

    vector<unsigned char> bwt;
    bwt.reserve(input.size() + 4);

    BWT_enc(input, bwt);



    // MTF encode
    vector<unsigned char> mtf;
    mtf.reserve(bwt.size());

    MTF_enc(bwt, mtf);



    // PackBits
    // vector<unsigned char> packed;
    // packed.reserve(mtf.size() + mtf.size() / 128 + 1);

    // RLE_PackBits(mtf, packed);



    // AAC encode
    const uint32_t mtf_len = static_cast<uint32_t>(mtf.size());
    vector<unsigned char> code(static_cast<size_t>(mtf_len) * 2 + 32, 0);

    const uint32_t bit_len = ac_encode_buffer(mtf.data(), mtf_len, code.data());
    const uint32_t byte_len = bit_len / 8 + 1;



    ofstream out(argv[2], ios::binary);
    if (!out) {
        cerr << "Error: cannot open output file '" << argv[2] << "'" << endl;
        return EXIT_FAILURE;
    }
    out.write(reinterpret_cast<const char*>(&mtf_len), 4);
    out.write(reinterpret_cast<const char*>(&bit_len), 4);
    out.write(
        reinterpret_cast<const char*>(code.data()),
        static_cast<streamsize>(byte_len)
    );
    if (!out) {
        cerr << "Error: failed writing to '" << argv[2] << "'" << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
