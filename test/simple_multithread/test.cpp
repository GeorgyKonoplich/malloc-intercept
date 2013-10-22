#include "../../allocator.h"
#include "../../utility.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pthread.h>

using namespace std;

ghoard::allocator allocator;

const int RETRY_CNT = 2000;
const int MAX_BLOCK_SIZE = 16*1024;
const int THREAD_CNT = 8;

void* test_from_thread(void * data){
    pthread_t cur_thread = *(pthread_t*)data;
    printf("test_from_thread(0x%lx) started\n", cur_thread);
    void ** ptrs = (void**)malloc(sizeof(void*)*RETRY_CNT);
    for(size_t block_size = 8; block_size < MAX_BLOCK_SIZE; block_size <<= 1){
        int i;
        for(i=0; i < RETRY_CNT; ++i){
            ptrs[i] = allocator.allocate(block_size);
            if(ptrs[i] == NULL){
                printf("NULL ptr returned by allocate\n");
                break;
            }
        }
        for(--i; i >= 0; --i){
            allocator.deallocate(ptrs[i]);
        }
    }
    printf("test_from_thread(0x%lx) finished\n", cur_thread);
    return NULL;
}


int main(){
    pthread_t threads[THREAD_CNT];
    for(int i=0; i<THREAD_CNT; ++i){
        pthread_create(&threads[i], NULL, test_from_thread, &threads[i]);
    }
    void ** result[THREAD_CNT];
    for(int i=0; i<THREAD_CNT; ++i){
        pthread_join(threads[i], NULL);
    }
    allocator.trace_debug();
    return 0;
}
