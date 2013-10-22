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

    class allocator {
    private:

        struct heap_holder_cls {
            heap * heaps;
            heap_holder_cls();
            heap * get_heap(int i);
            ~heap_holder_cls();
        } heap_holder;
        heap * get_current_heap();
        heap * get_global_heap();
        static void * allocate_large_block(size_t data_size, size_t alignment = DEFAULT_ALIGNMENT);

    public:
        void * allocate(size_t size, size_t alignment = DEFAULT_ALIGNMENT);
        void deallocate(void * ptr);
        void * reallocate(void * ptr, size_t size);
        void trace_debug();
        bool is_empty();
    };
}

#endif

