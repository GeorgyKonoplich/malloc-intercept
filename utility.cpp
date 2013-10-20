#include "utility.h"
#include "constants.h"
#include <cstddef>
#include <cassert>
#include <pthread.h>

using namespace std;

namespace ghoard {

    int log2_floor(size_t x) {
        size_t result = -1;
        while (x > 0) x >>= 1, ++result;
        return result;
    }

    int log2_ceil(size_t x) {
        return (is_power_of_2(x) ? 0 : 1)+log2_floor(x);
    }

    bool is_power_of_2(size_t n) {
        return ((n != 0) && !(n & (n - 1)));
    }

    int get_sz_group(size_t size) {
        if (size > BIG_SZ_MAX) return -1;
        if (size > SMALL_SZ_MAX) {
            return SMALL_SZ_CNT + log2_ceil(size) - BIG_SZ_BASE_LOG;
        }
        size >>= SMALL_SZ_BASE_LOG;
        if (size == 0) ++size;
        --size;
        return size;
    }

    size_t get_block_size(int sz_group) {
        assert(sz_group > 0 && sz_group < SZ_CNT);
        if (sz_group < SMALL_SZ_CNT) {
            return (sz_group + 1) << SMALL_SZ_BASE_LOG;
        }
        return 1 << (sz_group - SMALL_SZ_CNT + BIG_SZ_BASE_LOG);
    }

    /* System-specific handlers. Implementation below is for posix*/

    void* raw_allocate(size_t size) {
        return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    }

    void raw_deallocate(void * start_address, size_t total_size) {
        munmap(start_address, total_size);
    }

    void mutex_lock::init() {
        pthread_mutex_init(&mutex, NULL);
    }

    void mutex_lock::lock() {
        pthread_mutex_lock(&mutex);
    }

    void mutex_lock::unlock() {
        pthread_mutex_unlock(&mutex);
    }

    int get_processor_count() {
        return sysconf(_SC_NPROCESSORS_ONLN);
    }

    size_t get_current_thread_id() {
        pthread_t pt = pthread_self();
    }

    bool is_valid_alignment(size_t alignment) {
        if ((alignment % sizeof (void*)) != 0)
            return false;
        if (!is_power_of_2(alignment))
            return false;
        return true;
    }
}
