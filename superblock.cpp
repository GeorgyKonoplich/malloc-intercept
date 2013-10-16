#include "constants.h"
#include "superblock.h"
#include <unistd.h>
#include <sys/mman.h>
#include <cstddef>
#include <cassert>

using namespace std;

namespace ghoard{

    size_t superblock::get_block_with_meta_sz(){
        return header.block_size+sizeof(ordinary_block);
    }
    size_t superblock::get_block_count(){
        return (SUPERBLOCK_SIZE-sizeof(header_cls))/get_block_with_meta_sz();
    }
    void * superblock::get_data_start(){
        return ((char*)&header) + sizeof(header_cls);
    }
    void superblock::initialize_blocks(){
        header.stack_head = (empty_block*) get_data_start();
        header.stack_head->next = NULL;
        header.free_block_cnt = get_block_count();
        for(int i=1; i<header.free_block_cnt; ++i){
            empty_block* prev = header.stack_head;
            header.stack_head = (empty_block*)(((char*)header.stack_head) + get_block_with_meta_sz());
            header.stack_head->next = prev;
        }
    }
    bool superblock::has_free_blocks(){
        return header.stack_head == NULL; 
    }
    void * superblock::get_free_block(){
        assert(header.stack_head != NULL);
        void * result = header.stack_head;
        header.stack_head = header.stack_head->next;
        --header.free_block_cnt;
        return result;
    }
    void superblock::return_block_to_stack(void * block){
        ((empty_block*) block)->next = header.stack_head;
        header.stack_head = (empty_block*) block;
        ++header.free_block_cnt;
    }
    int superblock::get_fgroup_id(){
        return (get_block_count()-header.free_block_cnt)*FGROUP_COUNT/get_block_count();
    }

}
