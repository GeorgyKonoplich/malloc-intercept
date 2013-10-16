#include "constants.h"
#include "superblock.h"
#include <unistd.h>
#include <sys/mman.h>
#include <cstddef>
#include <cassert>
#include "utility.h"

using namespace std;

namespace ghoard{

    size_t superblock::get_block_with_meta_sz(){
        return get_block_size(sz_group) + sizeof(ordinary_block);
    }
    size_t superblock::get_block_count(){
        return (SUPERBLOCK_SIZE-sizeof(superblock))/get_block_with_meta_sz();
    }
    void * superblock::get_data_start(){
        return ((char*)this + sizeof(superblock));
    }
    void superblock::initialize_blocks(){
        stack_head = (empty_block*) get_data_start();
        stack_head->next = NULL;
        free_block_cnt = get_block_count();
        for(int i=1; i<free_block_cnt; ++i){
            empty_block* prev = stack_head;
            stack_head = (empty_block*)(((char*)stack_head) + get_block_with_meta_sz());
            stack_head->next = prev;
        }
    }
    bool superblock::has_free_blocks(){
        return stack_head == NULL; 
    }
    void * superblock::get_free_block(){
        if(!has_free_blocks()) return NULL;
        void * result = stack_head;
        stack_head = stack_head->next;
        --free_block_cnt;
        return result;
    }
    void superblock::return_block_to_stack(void * block){
        ((empty_block*) block)->next = stack_head;
        stack_head = (empty_block*) block;
        ++free_block_cnt;
    }
    int superblock::get_fgroup_id(){
        return (get_block_count()-free_block_cnt)*(FGROUP_COUNT-1)/get_block_count();
    }
    void superblock::set_list_pointers_to_null(){
        fnext = fprev = sznext = szprev = NULL;
    }

}
