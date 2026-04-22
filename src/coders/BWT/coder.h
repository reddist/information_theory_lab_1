#ifndef BWT_CODER_H
#define BWT_CODER_H

#include <vector>
using namespace std;

void BWT_enc(const vector<unsigned char>& in,
             vector<unsigned char>& out);

bool BWT_dec(const vector<unsigned char>& in,
             vector<unsigned char>& out);

#endif // !BWT_CODER_H
