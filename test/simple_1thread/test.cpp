#include "../../allocator.h"
#include "../../utility.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace std;

ghoard::allocator allocator;

const int RETRY_CNT = 20000;
const int max_block_size = 50*1024;

void test_1(){
    printf("test_1() started\n");
    void ** ptrs = (void**)malloc(sizeof(void*)*RETRY_CNT);
    for(size_t block_size = 8; block_size < max_block_size; block_size <<= 1){
        int i;
        for(i=0; i < RETRY_CNT; ++i){
            ptrs[i] = allocator.allocate(block_size);
            if(ptrs[i] == NULL){
                printf("NULL ptr returned by allocate\n");
                break;
            }
        }
        for(; i >= 0; --i){
            if(ptrs[i] != NULL) allocator.deallocate(ptrs[i]);
        }
    }
    if(!allocator.is_empty()){
        fprintf(stderr, "allocator isn't empty!\n");
        std::abort();
    }
    printf("test_1() finished\n");
}

int main(){
    test_1();
    return 0;
}
