#include "constants.h"
#include "heap.h"
#include "utility.h"
#include "allocator.h"
#include "superblock.h"
#include <unistd.h>
#include <sys/mman.h>
#include <cstddef>
#include <cstring>
#include "tracing.h"

static const bool PRINT_ALLOCATOR_TRACE = 1;

void ghoard::allocator::trace_debug(){
    trace("[Allocator]\n");
    for(int i=0; i<HEAP_CNT+1; ++i) heap_holder.get_heap(i)->trace_debug();
}

ghoard::allocator::heap_holder_cls::heap_holder_cls() {
    void * ptr = raw_allocate(HEAP_SIZE * (1 + HEAP_CNT));
    heaps = (heap*) ptr;
    for (int i = 0; i < HEAP_CNT+1; ++i) get_heap(i)->init();
}

ghoard::allocator::heap_holder_cls::~heap_holder_cls(){
    for (int i=0; i<HEAP_CNT+1; ++i){
        get_heap(i)->deallocate_all_superblocks();
    }
    raw_deallocate(heaps, HEAP_SIZE * (1 + HEAP_CNT));
}

ghoard::heap * ghoard::allocator::heap_holder_cls::get_heap(int i) {
    return (heap*) (((char*) heaps) + HEAP_SIZE * i);

}

bool ghoard::allocator::is_empty(){
    for (int i=0; i<HEAP_CNT+1; ++i){
        if(heap_holder.get_heap(i)->get_used_bytes() != 0) return false;
    }
    return true;
}

void * ghoard::allocator::allocate(size_t data_size, size_t alignment) {
    if(PRINT_ALLOCATOR_TRACE) trace("allocator.allocate(",data_size, ", ", alignment, ")\n");
    size_t ordinary_size = data_size + sizeof(ordinary_block) + alignment - 1;
    if (ordinary_size > BIG_SZ_MAX) {
        return allocate_large_block(data_size, alignment);
    }
    int sz_group = get_sz_group(ordinary_size);
    heap * global_heap = get_global_heap();
    heap * current_heap = get_current_heap();
    current_heap->lock();
    superblock * sb = current_heap->get_superblock_with_free_block(sz_group);
    if (sb == NULL) {
        if(PRINT_ALLOCATOR_TRACE) trace("No free superblocks in current thread's heap\n");
        global_heap->lock();
        sb = global_heap->get_superblock_with_free_block(sz_group);
        if (sb == NULL) {
            if(PRINT_ALLOCATOR_TRACE) trace("No free superblocks in global heap\n");
            global_heap->unlock();
            void * raw_ptr = raw_allocate(SUPERBLOCK_SIZE);
            if(raw_ptr == NULL) {
                current_heap->unlock();
                return raw_ptr;
            }
            sb = (superblock*) raw_ptr;
            current_heap->create_add_superblock(sb, sz_group);
        } else {
            global_heap->remove_superblock(sb);
            global_heap->unlock();
            if (sb->is_empty() && sb->get_sz_group() != sz_group) {
                current_heap->create_add_superblock(sb, sz_group);
            } else {
                current_heap->insert_superblock(sb);
            }
        }
    }else if(sb->is_empty() && sb->get_sz_group() != sz_group)
        current_heap->resize_superblock(sb, sz_group);
    void * raw_ptr = current_heap->pop_free_block(sb);
    current_heap->unlock();
    size_t indent = alignment - ((size_t) raw_ptr + sizeof (ordinary_block)) % alignment;
    ordinary_block* ob = (ordinary_block*) ((char*) raw_ptr + indent);
    ob->indent = indent;
    ob->parent = sb;
    return ++ob;
}

void * ghoard::allocator::allocate_large_block(size_t data_size, size_t alignment) {
    size_t total_size = data_size + sizeof(large_block) + alignment - 1;
    void * raw_ptr = raw_allocate(total_size);
    if(raw_ptr == NULL){
        return NULL;
    }
    size_t indent = alignment - ((size_t) raw_ptr + sizeof (large_block)) % alignment;
    large_block * lb = (large_block*) ((char*) raw_ptr + indent);
    if(PRINT_ALLOCATOR_TRACE) trace("allocating large block ", raw_ptr, " ", indent, " ", lb, " ", lb+1, " ", total_size, "\n");
    lb->total_size = total_size;
    lb->indent = indent;
    lb->parent = NULL;
    return ++lb;
}

void ghoard::allocator::deallocate(void * ptr) {
    if(PRINT_ALLOCATOR_TRACE) trace("allocator.deallocate(", ptr, ")\n");
    if(ptr == NULL) return;
    ordinary_block* ob = (ordinary_block*) ((char*) ptr - sizeof (ordinary_block));
    superblock * sb = ob->parent;
    if (sb == NULL) {
        large_block* lb = (large_block*) ((char*) ptr - sizeof (large_block));
        void * ptr = (void*)(((char*) lb) - lb->indent);
        if(PRINT_ALLOCATOR_TRACE) trace("deallocating large block ", ptr," ", lb->total_size, "\n");
        raw_deallocate(ptr, lb->total_size);
    } else {
        sb->lock();
        heap * parent_heap = sb->get_parent();
        parent_heap->lock();
        parent_heap->push_free_block((char*) ob - ob->indent, sb);
        superblock * deletion_candidate = parent_heap->get_deletion_candidate();
        heap * global_heap = get_global_heap();
        if (deletion_candidate != NULL) {
            if(parent_heap != global_heap){
                parent_heap->remove_superblock(deletion_candidate);
                global_heap->lock();
                global_heap->insert_superblock(deletion_candidate);
                global_heap->unlock();
            }else if(RETURN_SUPERBLOCKS && deletion_candidate->is_empty()){
                parent_heap->remove_superblock(sb);
                raw_deallocate(sb, SUPERBLOCK_SIZE);
            }
        }
        parent_heap->unlock();
        sb->unlock();
    }
}

ghoard::heap * ghoard::allocator::get_global_heap() {
    return heap_holder.get_heap(0);
}

ghoard::heap * ghoard::allocator::get_current_heap() {
    return heap_holder.get_heap(get_current_thread_id() + 1);
}

void * ghoard::allocator::reallocate(void * ptr, size_t size) {
    void * new_ptr = allocate(size);
    size_t old_size = get_size(ptr);
    memcpy(new_ptr, ptr, size < old_size ? size : old_size);
    deallocate(ptr);
    return new_ptr;
}


