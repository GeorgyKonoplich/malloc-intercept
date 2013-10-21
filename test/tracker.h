#ifndef GHOARD_TEST_TRACKER
#define GHOARD_TEST_TRACKER

#include <map>
#include <utility>
#include <cstddef>
#include <cstdlib>
#include <cstdio>

using namespace std;

namespace ghoard{
    typedef pair<void *, size_t> alloc_pair;

    map<void *, size_t> allocs;
    void* raw_allocate(size_t size){
        printf("raw_allocate(%lu)\n", size);
        void * ptr = malloc(size);
        allocs.insert(alloc_pair(ptr, size));
    }

    void raw_deallocate(void * ptr, size_t size){
        printf("raw_deallocate(%p, %lu)", ptr, size);
        if(allocs.find(ptr) != allocs.end()){
            fprintf(stderr, "no allocation was perfored for %p\n", ptr);
            std::abort();
        }
        size_t was_taken = allocs[ptr];
        if(was_taken != size){
            fprintf(stderr, "for %p was taken %lu, returned only %lu\n",ptr, was_taken, size);
            std::abort();
        }
        free(ptr);
    }

};

#endif
