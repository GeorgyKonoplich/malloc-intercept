#ifndef GHOARD_HEAP
#define GHOARD_HEAP

#include "constants.h"
#include "superblock.h"
#include <unistd.h>
#include <sys/mman.h>
#include <cstddef>

using namespace std;

namespace ghoard{
    struct heap{
        size_t used_bytes;
        size_t available_bytes;
        superblock* fgroup_heads[FGROUP_COUNT];
        superblock** sz_heads;

        void init();
        bool is_threshold_passed();
        void create_add_superblock(void * bytes, size_t block_size){
            superblock * sb = ((superblock*) bytes);
            sb->header.parent = this;
            sb->header.block_size = block_size;
            sb->initialize_blocks();
            sb->header.fnext = sb->header.fprev = sb->header.szprev = sb->header.sznext = NULL;

        }
        void reinsert_superblock(superblock * sb){
            if(sb->header.fprev != NULL) sb->header.fprev->header.fnext = sb->header.fnext;
            if(sb->header.szprev != NULL) sb->header.szprev->header.sznext = sb->header.sznext;
            if(sb->header.fnext != NULL) sb->header.fnext->header.fprev = sb->header.fprev;
            if(sb->header.sznext != NULL) sb->header.sznext->header.szprev = sb->header.szprev;
            int fid = sb->get_fgroup_id();

        }
    };
    const size_t HEAP_SIZE = sizeof(heap)+sizeof(superblock*)*SZ_CNT*FGROUP_COUNT;

    heap* create_heap(void * bytes){
        heap * h = (heap*) bytes;
        superblock ** sz_heads = (superblock **)((char*)bytes+HEAP_SIZE);
        h->sz_heads = sz_heads;
        h->init();
    }


}

#endif
