#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
using namespace std;

static bool RLE_UnpackBits(const vector<unsigned char>& in,
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

int main(int argc, char** argv) {
    if (argc != 3) {
        cerr << "Usage: decoder.exe <input_file> <output_file>" << endl;
        return EXIT_FAILURE;
    }

    ifstream in(argv[1], ios::binary);
    if (!in) {
        cerr << "Error: cannot open input file '" << argv[1] << "'\n";
        return EXIT_FAILURE;
    }

    vector<unsigned char> input(
        (istreambuf_iterator<char>(in)),
        istreambuf_iterator<char>()
    );
    in.close();

    vector<unsigned char> output;
    output.reserve(input.size() * 2);

    if (!RLE_UnpackBits(input, output)) {
        cerr << "Error: truncated or malformed PackBits stream\n";
        return EXIT_FAILURE;
    }

    ofstream out(argv[2], ios::binary);
    if (!out) {
        cerr << "Error: cannot open output file '" << argv[2] << "'\n";
        return EXIT_FAILURE;
    }
    if (!output.empty()) {
        out.write(reinterpret_cast<const char*>(output.data()),
                  static_cast<streamsize>(output.size()));
    }
    if (!out) {
        cerr << "Error: failed writing to '" << argv[2] << "'\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
