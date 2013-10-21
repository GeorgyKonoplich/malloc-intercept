#include "../../allocator.h"
#include "../../utility.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace std;

ghoard::allocator allocator;

const int RETRY_CNT = 20000;

void test_1(){
    printf("test_1() started\n");
    void ** ptrs = (void**)malloc(sizeof(void*)*RETRY_CNT);
    for(size_t block_size = 8; block_size < 1024*256; block_size <<= 1){
//        printf("--------- Block size %lu\n", block_size);
        for(int i=0; i < RETRY_CNT; ++i){
//            printf("Retry: %d\n", i);
            ptrs[i] = allocator.allocate(block_size);
  //          allocator.print_debug();
        }
       //allocator.print_debug();
        for(int i=0; i < RETRY_CNT; ++i){
            allocator.deallocate(ptrs[i]);
        }
    }
    allocator.print_debug();
    printf("test_1() finished\n");
}


int main(){
    test_1();
    return 0;
}
