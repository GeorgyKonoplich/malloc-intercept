#include "utility.h"
#include "constants.h"
#include "tracing.h"
#include <cstddef>
#include <pthread.h>

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
        if(size == 0) return -2;
        if (size > BIG_SZ_MAX) return -1;
        if (size > SMALL_SZ_MAX) {
            return SMALL_SZ_CNT + log2_ceil(size) - BIG_SZ_BASE_LOG;
        }
        --size;
        size >>= SMALL_SZ_BASE_LOG;
        return size;
    }

    size_t get_block_size(int sz_group) {
        if(!(sz_group >= 0 && sz_group < SZ_CNT)){
            print("Wrong sz_group!\n");
            std::abort();
        }
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
    int get_current_thread_id(){
        return get_current_thread_id(HEAP_CNT_LOG);
    }
    int get_current_thread_id(int mod_log) {
        static __thread int res = -1;
        if( res < 0 ){
            pthread_t pt = pthread_self();
            int id = (int)pt;
            if(id < 0) id = - ++id;
            int mod = ((1<<mod_log)-1);
            res = id&mod;
            id >>= mod_log;
            while (id > 0) {
                res ^= id&mod;
                id >>= mod_log;
            }
        }
        return res; 
    }

    bool is_valid_alignment(size_t alignment) {
        if ((alignment % sizeof (void*)) != 0)
            return false;
        if (!is_power_of_2(alignment))
            return false;
        return true;
    }
}
