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

static const bool PRINT_ALLOCATOR_TRACE = false;

void ghoard::allocator::print_debug(){
    print("[Allocator]\n");
    for(int i=0; i<HEAP_CNT+1; ++i) heap_holder.get_heap(i)->print_debug();
}

ghoard::allocator::heap_holder_cls::heap_holder_cls() {
    size_t heaps_size = HEAP_SIZE * (1 + HEAP_CNT);
    void * ptr = raw_allocate(heaps_size);
    heaps = (heap*) ptr;
    for (int i = 0; i < HEAP_CNT+1; ++i) get_heap(i)->init();
}

ghoard::heap * ghoard::allocator::heap_holder_cls::get_heap(int i) {
    return (heap*) (((char*) heaps) + HEAP_SIZE * i);

}

void * ghoard::allocator::allocate(size_t data_size, size_t alignment) {
    if(PRINT_ALLOCATOR_TRACE) print("allocator.allocate(",data_size, ", ", alignment, ")\n");
    size_t ordinary_size = get_acceptable_block_size(data_size, sizeof (ordinary_block), alignment);
    if (ordinary_size > BIG_SZ_MAX) {
        return allocate_large_block(data_size, alignment);
    }
    int sz_group = get_sz_group(ordinary_size);
    heap * global_heap = get_global_heap();
    heap * current_heap = get_current_heap();
    current_heap->lock();
    superblock * sb = current_heap->get_superblock_with_free_block(sz_group);
    bool reusing_sb = true;
    if (sb == NULL) {
        if(PRINT_ALLOCATOR_TRACE) print("No free superblocks in current thread's heap\n");
        global_heap->lock();
        sb = global_heap->get_superblock_with_free_block(sz_group);
        if (sb == NULL) {
            if(PRINT_ALLOCATOR_TRACE) print("No free superblocks in global heap\n");
            global_heap->unlock();
            sb = (superblock*) raw_allocate(SUPERBLOCK_SIZE);
            current_heap->create_add_superblock(sb, sz_group);
            reusing_sb = false;
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

size_t ghoard::allocator::get_acceptable_block_size(size_t data_size, size_t meta_size, size_t alignment) {
    return data_size + meta_size + alignment - 1;
}

void * ghoard::allocator::allocate_large_block(size_t data_size, size_t alignment = DEFAULT_ALIGNMENT) {
    size_t total_size = get_acceptable_block_size(data_size, sizeof (large_block), alignment);
    void * raw_ptr = raw_allocate(total_size);
    size_t indent = alignment - ((size_t) raw_ptr + sizeof (large_block)) % alignment;
    large_block * lb = (large_block*) ((char*) raw_ptr + indent);
    if(PRINT_ALLOCATOR_TRACE) print("allocating large block ", raw_ptr, " ", indent, " ", lb, " ", lb+1, " ", total_size, "\n");
    lb->total_size = total_size;
    lb->indent = indent;
    lb->parent = NULL;
    return ++lb;
}

void ghoard::allocator::deallocate(void * ptr) {
    if(PRINT_ALLOCATOR_TRACE) print("allocator.deallocate(", ptr, ")\n");
    ordinary_block* ob = (ordinary_block*) ((char*) ptr - sizeof (ordinary_block));
    superblock * sb = ob->parent;
    if (sb == NULL) {
        large_block* lb = (large_block*) ((char*) ptr - sizeof (large_block));
        void * ptr = (void*)(((char*) lb) - lb->indent);
        if(PRINT_ALLOCATOR_TRACE) print("deallocating large block ", ptr," ", lb->total_size, "\n");
        raw_deallocate(ptr, lb->total_size);
    } else {
        sb->lock();
        heap * parent_heap = sb->get_parent();
        parent_heap->lock();
        parent_heap->push_free_block((char*) ob - ob->indent, sb);
        superblock * deletion_candidate = parent_heap->get_deletion_candidate();
        heap * global_heap = get_global_heap();
        if (deletion_candidate != NULL && parent_heap != global_heap) {
            parent_heap->remove_superblock(deletion_candidate);
            global_heap->lock();
            global_heap->insert_superblock(deletion_candidate);
            global_heap->unlock();
        }
        parent_heap->unlock();

        sb->unlock();
    }
}

ghoard::heap * ghoard::allocator::get_global_heap() {
    return heap_holder.get_heap(0);
}

ghoard::heap * ghoard::allocator::get_current_heap() {
    int id = (int) get_current_thread_id() % HEAP_CNT + 1;
    return heap_holder.get_heap(id);
}

void * ghoard::allocator::reallocate(void * ptr, size_t size) {
    void * new_ptr = allocate(size);
    size_t old_size = get_size(ptr);
    memcpy(new_ptr, ptr, size < old_size ? size : old_size);
    deallocate(ptr);
    return new_ptr;
}


