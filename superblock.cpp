#include "constants.h"
#include "superblock.h"
#include <unistd.h>
#include <sys/mman.h>
#include <cstddef>
#include <cassert>
#include "utility.h"

size_t ghoard::superblock::get_block_with_meta_sz() {
    return block_with_meta_sz;
}

size_t ghoard::superblock::get_block_count() {
    return block_count;
}

void ghoard::superblock::init() {
    block_with_meta_sz = get_block_size(sz_group) + sizeof (ordinary_block);
    block_count = (SUPERBLOCK_SIZE - sizeof (superblock)) / block_with_meta_sz;
    next_uninitialized_block_start = ((char*) this + sizeof (superblock));
    initialized_cnt = 0;
    stack_head = NULL;
    free_block_cnt = get_block_count();
    /*for (int i = 1; i < free_block_cnt; ++i) {
        empty_block* prev = stack_head;
        stack_head = (empty_block*) (((char*) stack_head) + get_block_with_meta_sz());
        stack_head->next = prev;
    }*/
}

bool ghoard::superblock::has_free_blocks() {
    return stack_head != NULL || initialized_cnt < block_count;
}

void * ghoard::superblock::pop_free_block() {
    void * result = stack_head;
    if(stack_head == NULL && initialized_cnt < block_count){
        ++initialized_cnt;
        --free_block_cnt;
        result = next_uninitialized_block_start;
        next_uninitialized_block_start = (char*) next_uninitialized_block_start + block_with_meta_sz;
    }else if(stack_head != NULL){
        stack_head = stack_head->next;
        --free_block_cnt;
    }
    return result;
}

void ghoard::superblock::push_free_block(void * block) {
    ((empty_block*) block)->next = stack_head;
    stack_head = (empty_block*) block;
    ++free_block_cnt;
}

int ghoard::superblock::get_fgroup_id() {
    if(is_empty()) return 0;
    return 1 + (block_count - free_block_cnt)*(FGROUP_COUNT - 2) / block_count; 
}

void ghoard::superblock::set_list_pointers_to_null() {
    fnext = fprev = sznext = szprev = NULL;
}

bool ghoard::superblock::is_empty() {
    return free_block_cnt == block_count;
}

ghoard::heap * ghoard::superblock::get_parent() {
    return parent;
}

void ghoard::superblock::lock() {
    mutex.lock();
}

void ghoard::superblock::unlock() {
    mutex.unlock();
}

int ghoard::superblock::get_sz_group() {
    return sz_group;
}

size_t ghoard::get_size(void * ptr) {
    ordinary_block* ob = (ordinary_block*) ((char*) ptr - sizeof (ordinary_block));
    if (ob->parent == NULL) {
        large_block* lb = (large_block*) ((char*) ptr - sizeof (large_block));
        return lb->total_size - lb->indent - sizeof (large_block);
    } else {
        return get_block_size(ob->parent->get_sz_group());
    }
}

