#ifndef PACKBITS_CODER_H
#define PACKBITS_CODER_H

#include <vector>
using namespace std;

void RLE_PackBits(const vector<unsigned char>& in,
                  vector<unsigned char>& out);

bool RLE_UnpackBits(const vector<unsigned char>& in,
                    vector<unsigned char>& out);

#endif // !PACKBITS_CODER_H