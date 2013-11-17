#include "utility.h"
#include "constants.h"
#include <unistd.h>
#include <sys/mman.h>
#include <cstddef>
#include <cstdlib>

using namespace std;

namespace ghoard {

    const int HEAP_CNT_LOG = log2_ceil(get_processor_count()*2);
    const int HEAP_CNT = 1<<HEAP_CNT_LOG;

    const bool RETURN_SUPERBLOCKS = getenv("GHOARD_RETURN_SUPERBLOCKS") != NULL;

    const size_t PAGE_SIZE = sysconf(_SC_PAGESIZE);
    const size_t SUPERBLOCK_SIZE = 16 * PAGE_SIZE;

    const size_t BIG_SZ_BASE_LOG = log2_floor(BIG_SZ_BASE);
    const size_t BIG_SZ_MAX = SUPERBLOCK_SIZE / 4;
    const int BIG_SZ_CNT = log2_floor(BIG_SZ_MAX) - BIG_SZ_BASE_LOG + 1;

    const int SZ_CNT = SMALL_SZ_CNT + BIG_SZ_CNT;
};

