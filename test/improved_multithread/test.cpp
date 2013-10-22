#include "../../allocator.h"
#include "../../utility.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <set>

using namespace std;

ghoard::allocator ghoard_allocator;

const int RETRY_CNT = 2000;
const int MIN_BLOCK_SIZE_LOG = 5;
const int MAX_BLOCK_SIZE_LOG = 14; 
const int THREAD_CNT = 8;

const int PTRS_SIZE = (MAX_BLOCK_SIZE_LOG-MIN_BLOCK_SIZE_LOG+1)*RETRY_CNT;
void * pointers[PTRS_SIZE*THREAD_CNT];

void* allocate_from_thread(void * data){
    void** ptrs = (void **)data;
    for(int block_size_log = MIN_BLOCK_SIZE_LOG; block_size_log <= MAX_BLOCK_SIZE_LOG; block_size_log++){
        size_t block_size = 1<<block_size_log;
        for(int i=0; i < RETRY_CNT; ++i, ptrs++){
            *ptrs = ghoard_allocator.allocate(block_size);
            if(*ptrs==NULL) printf("NULL allocated\n");
        }
    }
    return NULL;
}
void* deallocate_from_thread(void * data){
    void** ptrs = (void **)data;
    for(int block_size_log = MIN_BLOCK_SIZE_LOG; block_size_log <= MAX_BLOCK_SIZE_LOG; block_size_log++){
        size_t block_size = 1<<block_size_log;
        for(int i=0; i < RETRY_CNT; ++i, ptrs++){
            if(*ptrs != NULL) ghoard_allocator.deallocate(*ptrs);
        }
    }
    return NULL;
}

set<void*> pset;

int main(){
    pthread_t threads[THREAD_CNT];
    for(int i=0; i<THREAD_CNT; ++i){
        pthread_create(&threads[i], NULL, allocate_from_thread, pointers+PTRS_SIZE*i);
    }
    for(int i=0; i<THREAD_CNT; ++i){
        pthread_join(threads[i], NULL);
    }
    int null_allocations = 0;
    for(void ** ptr = pointers; ptr<pointers+PTRS_SIZE*THREAD_CNT; ++ptr){
        if(*ptr == NULL){
            null_allocations++;
            continue;
        }
        if(pset.find(*ptr) != pset.end()){
            fprintf(stderr, "Duplicate allocation: %p\n", *ptr);
            std::abort();
        }
        pset.insert(*ptr);
    }
    printf("Null allocation count: %d\n", null_allocations);
    for(int i=0; i<THREAD_CNT; ++i){
        pthread_create(&threads[i], NULL, deallocate_from_thread, pointers+PTRS_SIZE*i);
    }
    for(int i=0; i<THREAD_CNT; ++i){
        pthread_join(threads[i], NULL);
    }
    return 0;
}
