#ifndef GHOARD_SUPERBLOCK
#define GHOARD_SUPERBLOCK

#include "constants.h"
#include <unistd.h>
#include <sys/mman.h>
#include <cstddef>

using namespace std;

namespace ghoard{
    struct heap;
    struct superblock;

    struct large_block{
        size_t block_size;
        size_t indent;
        superblock * parent;
    };
    struct ordinary_block{
        size_t indent;
        superblock * parent;
    }; 
    struct empty_block{
        empty_block * next;
    };

    struct superblock
    {
        struct header_cls{
            heap * parent;
            size_t block_size;
            size_t free_block_cnt;
            superblock * fnext;
            superblock * fprev;
            superblock * szprev;
            superblock * sznext;
            empty_block * stack_head;
        } header;
        size_t get_block_with_meta_sz();
        size_t get_block_count();
        void * get_data_start();
        void initialize_blocks();
        bool has_free_blocks();
        void * get_free_block();
        void return_block_to_stack(void * block);
        int get_fgroup_id();
    };
}

#endif
