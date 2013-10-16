#include "utility.h"
#include "constants.h"
#include <cstddef>
#include <cassert>

using namespace std;

namespace ghoard{

    int log2_floor(size_t x){
        size_t result = -1;
        while(x > 0) x>>=1, ++result;
        return result;
    }
    int log2_ceil(size_t x){
        return (is_power_of_2(x)?0:1)+log2_floor(x);
    }
    bool is_power_of_2(size_t n)
    {
        return ((n != 0) && !(n & (n - 1))); 
    }
    int get_sz_group(size_t size){
        if(size > BIG_SZ_MAX) return -1;
        if(size > SMALL_SZ_MAX){
            return SMALL_SZ_CNT + log2_ceil(size) - BIG_SZ_BASE_LOG;
        }
        size >>= SMALL_SZ_BASE_LOG;
        if(size == 0) ++size;
        --size;
        return size;
    }
    size_t get_block_size(int sz_group){
        assert(sz_group>0 && sz_group < SZ_CNT);
        if(sz_group < SMALL_SZ_CNT){
            return (sz_group+1) << SMALL_SZ_BASE_LOG;
        }
        return 1 << (sz_group - SMALL_SZ_CNT + BIG_SZ_BASE_LOG);
    }
}
