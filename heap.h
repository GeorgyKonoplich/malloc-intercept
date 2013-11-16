#ifndef GHOARD_HEAP
#define GHOARD_HEAP

#include "constants.h"
#include "superblock.h"
#include <unistd.h>
#include <sys/mman.h>
#include <cstddef>
#include "utility.h"

using namespace std;

namespace ghoard {
    const int HEAP_MAGICK = 0x319fce20;

    class heap {
    private:
        size_t used_bytes;
        size_t available_bytes;
        superblock * fgroup_heads[FGROUP_COUNT];
        superblock** sz_heads;
        mutex_lock mutex;
        int magick; 
        bool is_threshold_passed();
        superblock ** get_sz_head(int fid, int size_group);

    public:
        void check_magick();
        void trace_debug();
        void create_add_superblock(void * bytes, int sz_group);
        void resize_superblock(superblock * sb, int sz_group);
        superblock * get_deletion_candidate();
        void remove_superblock(superblock * sb);
        void insert_superblock(superblock * sb);
        superblock * get_superblock_with_free_block(int sz_group);
        void push_free_block(void * block, superblock * sb);
        void * pop_free_block(superblock* sb);
        void lock();
        void unlock();
        void init();
        void deallocate_all_superblocks();
        int get_available_bytes();
        int get_used_bytes();
    };
    const size_t HEAP_SIZE = sizeof (heap) + sizeof (superblock*) * SZ_CNT*FGROUP_COUNT;


}

#endif
