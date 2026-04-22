#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
#include "coders/AAC_D/coder.h"
#include "coders/PackBits/coder.h"
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



    // PackBits
    vector<unsigned char> input(
        (istreambuf_iterator<char>(in)),
        istreambuf_iterator<char>()
    );
    in.close();
    
    vector<unsigned char> packed;
    packed.reserve(input.size() + input.size() / 128 + 1);

    RLE_PackBits(input, packed);



    // AAC encode
    const uint32_t packed_len = static_cast<uint32_t>(packed.size());
    vector<unsigned char> code(static_cast<size_t>(packed_len) * 2 + 32, 0);

    const uint32_t bit_len = ac_encode_buffer(packed.data(), packed_len, code.data());
    const uint32_t byte_len = bit_len / 8 + 1;



    ofstream out(argv[2], ios::binary);
    if (!out) {
        cerr << "Error: cannot open output file '" << argv[2] << "'" << endl;
        return EXIT_FAILURE;
    }
    out.write(reinterpret_cast<const char*>(&packed_len), 4);
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
