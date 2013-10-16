#ifndef GHOARD_CONSTANTS
#define GHOARD_CONSTANTS

#include <unistd.h>
#include <sys/mman.h>
#include <cstddef>

using namespace std;

namespace ghoard{
    int log2_floor(size_t x){
        size_t result = -1;
        while(x > 0) x>>=1, ++result;
        return result;
    }

    const size_t K_THRESHOLD = 10;

    const unsigned long long F_THRESHOLD_DENOMINATOR = 4;
    const unsigned long long F_THRESHOLD_NUMERATOR = 1;

    const size_t PAGE_SIZE = sysconf(_SC_PAGESIZE);
    const size_t SUPERBLOCK_SIZE = 16 * PAGE_SIZE; 
    const int FGROUP_COUNT = 10;

    const int SMALL_SZ_BASE = 8; 
    const int SMALL_SZ_MAX = 256;

    const int SMALL_SZ_CNT = SMALL_SZ_MAX/SMALL_SZ_BASE;
    const size_t BIG_SZ_BASE = SMALL_SZ_MAX*2;
    const size_t BIG_SZ_MAX = SUPERBLOCK_SIZE/4;
    const int BIG_SZ_CNT = log2_floor(BIG_SZ_MAX)-log2_floor(BIG_SZ_BASE)+1;

    const int SZ_CNT = SMALL_SZ_CNT + BIG_SZ_CNT;
};

#endif
