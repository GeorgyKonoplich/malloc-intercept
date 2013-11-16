#ifndef GHOARD_CONSTANTS
#define GHOARD_CONSTANTS

#include "utility.h"
#include <unistd.h>
#include <sys/mman.h>
#include <cstddef>
#include <cstdlib>

using namespace std;

namespace ghoard {

    extern const int HEAP_CNT_LOG;
    extern const int HEAP_CNT;

    const size_t DEFAULT_ALIGNMENT = 8;
    const size_t K_THRESHOLD = 10;
    const unsigned long long F_THRESHOLD_NUMERATOR = 1;
    const unsigned long long F_THRESHOLD_DENOMINATOR = 4;

    extern const bool RETURN_SUPERBLOCKS;

    extern const size_t PAGE_SIZE;
    extern const size_t SUPERBLOCK_SIZE;

    const int FGROUP_COUNT = 10;

    const int SMALL_SZ_BASE_LOG = 3;
    const int SMALL_SZ_BASE = 1 << SMALL_SZ_BASE_LOG;
    const int SMALL_SZ_MAX = 256;

    const int SMALL_SZ_CNT = SMALL_SZ_MAX / SMALL_SZ_BASE;
    const size_t BIG_SZ_BASE = SMALL_SZ_MAX * 2;
    
    extern const size_t BIG_SZ_BASE_LOG;
    extern const size_t BIG_SZ_MAX;
    extern const int BIG_SZ_CNT;

    extern const int SZ_CNT;
};

#endif
