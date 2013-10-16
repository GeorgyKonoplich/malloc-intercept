#include "heap.h"
#include "constants.h"
#include "superblock.h"
#include <unistd.h>
#include <sys/mman.h>
#include <cstddef>

using namespace std;

namespace ghoard{
    void heap::init(){
        used_bytes = 0;
        available_bytes = 0;
        for(int i=0; i< FGROUP_COUNT; ++i){
            fgroup_heads[i] = NULL;
            for(int j = 0; j< SZ_CNT; ++j){
                *(sz_heads+i*SZ_CNT+j) = NULL;
            }
        }
    }
    bool heap::is_threshold_passed(){
        return (used_bytes < available_bytes - K_THRESHOLD*SUPERBLOCK_SIZE)
            && (F_THRESHOLD_DENOMINATOR*(available_bytes-used_bytes)
                    > F_THRESHOLD_NUMERATOR*available_bytes);
    }


}

