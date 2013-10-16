#ifndef GHOARD_UTILITY
#define GHOARD_UTILITY

#include <cstddef>

using namespace std;

namespace ghoard{
    int log2_floor(size_t x);
    int log2_ceil(size_t x);
    bool is_power_of_2(size_t n);
    int get_sz_group(size_t size);
    size_t get_block_size(int sz_group);
};

#endif
