#ifndef GHOARD_TEST_TRACKER
#define GHOARD_TEST_TRACKER

#include <map>
#include <utility>
#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include "../tracing.h"

const bool PRINT_TRACKER_TRACE = 0;

using namespace std;

const int SUPERBLOCK_SIZE = 65536;
namespace ghoard{
    typedef pair<void *, size_t> alloc_pair;

    map<void *, size_t> allocs;
    void* raw_allocate(size_t size){
        void * ptr = malloc(size);
         if(PRINT_TRACKER_TRACE) trace("raw_allocate(", size,") -> (", ptr,")\n");
        if(ptr != NULL) allocs.insert(alloc_pair(ptr, size));
        return ptr;
    }
    void raw_deallocate(void * ptr, size_t size){
        if(PRINT_TRACKER_TRACE) trace("raw_deallocate(", ptr,", ", size, ")\n");
        if(allocs.find(ptr) == allocs.end()){
            print("no allocation was performed for ", ptr, "\n");
            std::abort();
        }
        size_t was_taken = allocs[ptr];
        if(was_taken != size){
            print("for ", ptr, " was taken ",was_taken,", returned only ",size, "\n");
            std::abort();
        }
        allocs.erase(ptr);
        free(ptr);
    }

};

#endif
