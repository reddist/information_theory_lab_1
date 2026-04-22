#ifndef MTF_CODER_H
#define MTF_CODER_H

#include <vector>
using namespace std;

void MTF_enc(const vector<unsigned char>& in,
             vector<unsigned char>& out);

bool MTF_dec(const vector<unsigned char>& in,
             vector<unsigned char>& out);

#endif // !MTF_CODER_H
