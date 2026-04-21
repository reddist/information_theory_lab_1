#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
using namespace std;

static void RLE_PackBits(const vector<unsigned char>& in,
                         vector<unsigned char>& out) {
    const size_t n = in.size();

    auto get_same_symbol_count = [in, n](size_t start) -> size_t {
        size_t current = start + 1;
        size_t count = 1;
        while (
            current < n &&
            in[current] == in[start] &&
            count < 128
        ) {
            ++current;
            ++count;
        }
        return count;
    };
    
    size_t i = 0;
    size_t same_already_counted = 0;
    size_t same_symbol_count;

    while (i < n) {
        if (same_already_counted > 0) {
            same_symbol_count = same_already_counted;
            same_already_counted = 0;
        } else {
            same_symbol_count = get_same_symbol_count(i);
        }

        if (same_symbol_count >= 3) {
            size_t minus_counter = -(same_symbol_count - 1);
            out.push_back(static_cast<unsigned char>(static_cast<int8_t>(minus_counter)));
            out.push_back(in[i]);
            i += same_symbol_count;
            continue;
        }

        size_t j = i + same_symbol_count;
        size_t uncoded_count = same_symbol_count;
        
        while (j < n) {
            size_t same_symbol_count_j = get_same_symbol_count(j);
            
            if (same_symbol_count_j >= 3) {
                same_already_counted = same_symbol_count_j;
                break;
            }

            j += same_symbol_count_j;
            uncoded_count += same_symbol_count_j;
            
            if (uncoded_count >= 128) {
                uncoded_count = 128;
                break;
            }
        }

        size_t plus_counter = uncoded_count - 1;
        out.push_back(static_cast<unsigned char>(static_cast<int8_t>(plus_counter)));
        out.insert(out.end(), in.begin() + i, in.begin() + i + uncoded_count);
        i += uncoded_count;
    }
}

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

    vector<unsigned char> input(
        (istreambuf_iterator<char>(in)),
        istreambuf_iterator<char>()
    );
    in.close();

    vector<unsigned char> output;
    output.reserve(input.size() + input.size() / 128 + 1);

    RLE_PackBits(input, output);

    ofstream out(argv[2], ios::binary);
    if (!out) {
        cerr << "Error: cannot open output file '" << argv[2] << "'" << endl;
        return EXIT_FAILURE;
    }
    if (!output.empty()) {
        out.write(
            reinterpret_cast<const char*>(output.data()),
            static_cast<streamsize>(output.size())
        );
    }
    if (!out) {
        cerr << "Error: failed writing to '" << argv[2] << "'" << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
