#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <cassert>
#include <unistd.h>

using namespace std;

const int RETRY_CNT = 5000;
const int MIN_BLOCK_SIZE_LOG = 3;
const int MAX_BLOCK_SIZE_LOG = 14;
const int THREAD_CNT = 8;


void* test_from_thread(void * data){
    void ** pointers = (void**) malloc(sizeof(void*)*(MAX_BLOCK_SIZE_LOG-MIN_BLOCK_SIZE_LOG+1)*RETRY_CNT);
    void ** last_ptr = pointers;
    for(int i=0; i<RETRY_CNT; ++i){
        for(int log=MIN_BLOCK_SIZE_LOG; log<=MAX_BLOCK_SIZE_LOG; ++log, ++last_ptr){
            size_t block_size = 1<<log;
            *last_ptr = malloc(block_size);
        }
    }
    while(last_ptr != pointers){
        void * ptr = *(--last_ptr);
        if(ptr != NULL)
            free(ptr);
    }
    return NULL;
}

int main(){
    pthread_t threads[THREAD_CNT];
    for(int i=0; i<THREAD_CNT; ++i){
        pthread_create(&threads[i], NULL, test_from_thread, NULL);
    }
    for(int i=0; i<THREAD_CNT; ++i){
        pthread_join(threads[i], NULL);
    }
    return 0;
}
