#ifndef GHOARD_ALLOCATOR
#define GHOARD_ALLOCATOR

#include "constants.h"
#include "heap.h"
#include "superblock.h"
#include <unistd.h>
#include <sys/mman.h>
#include <cstddef>

using namespace std;

namespace ghoard {
    const int HEAP_CNT = get_processor_count()*2;

    struct allocator {
    private:

        struct heap_holder_cls {
            heap * heaps;
            heap_holder_cls();
            heap * get_heap(int i);
        } heap_holder;
        size_t get_acceptable_block_size(size_t data_size, size_t meta_size, size_t alignment);
        heap * get_current_heap();
        heap * get_global_heap();
        void * allocate_large_block(size_t data_size, size_t alignment);

    public:
        void * allocate(size_t size, size_t alignment = DEFAULT_ALIGNMENT);
        void deallocate(void * ptr);
        void * reallocate(void * ptr, size_t size);
    };
}

#endif

