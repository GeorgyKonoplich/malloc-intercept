#ifndef GHOARD_UTILITY
#define GHOARD_UTILITY

#include <cstddef>
#include <pthread.h>

using namespace std;

namespace ghoard {
    int log2_floor(size_t x);
    int log2_ceil(size_t x);
    bool is_power_of_2(size_t n);
    int get_sz_group(size_t size);
    size_t get_block_size(int sz_group);
    void* raw_allocate(size_t size);
    void raw_deallocate(void * start_address, size_t total_size);
    int get_processor_count();

    struct mutex_lock {
    private:
        pthread_mutex_t mutex;

    public:
        void init();
        void lock();
        void unlock();
    };
    size_t get_current_thread_id();
    bool is_valid_alignment(size_t alignment);
};

#endif
