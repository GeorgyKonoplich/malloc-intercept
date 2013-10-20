#include "constants.h"
#include "heap.h"
#include "utility.h"
#include "allocator.h"
#include "superblock.h"
#include <unistd.h>
#include <sys/mman.h>
#include <cstddef>
//#include <cstring>

ghoard::allocator::heap_holder_cls::heap_holder_cls() {
    heaps = (heap*) raw_allocate(HEAP_SIZE * (1 + HEAP_CNT));
    for (int i = 0; i < HEAP_CNT; ++i) ((heap*) get_heap(i))->init();
}

ghoard::heap * ghoard::allocator::heap_holder_cls::get_heap(int i) {
    return (heap*) (((char*) heaps) + HEAP_SIZE * i);

}

void * ghoard::allocator::allocate(size_t data_size, size_t alignment) {
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
        global_heap->lock();
        sb = global_heap->get_superblock_with_free_block(sz_group);
        if (sb == NULL) {
            global_heap->unlock();
            sb = (superblock*) raw_allocate(SUPERBLOCK_SIZE);
            current_heap->create_add_superblock(sb, sz_group);
            reusing_sb = false;
        } else {
            global_heap->remove_superblock(sb);
            global_heap->unlock();
        }
    }
    if (reusing_sb) {
        if (sb->is_empty() && sb->get_sz_group() != sz_group) {
            current_heap->create_add_superblock(sb, sz_group);
        } else {
            current_heap->insert_superblock(sb);
        }
    }
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
    lb->total_size = total_size;
    lb->indent = indent;
    lb->parent = NULL;
    return ++lb;
}

void ghoard::allocator::deallocate(void * ptr) {
    ordinary_block* ob = (ordinary_block*) ((char*) ptr - sizeof (ordinary_block));
    superblock * sb = ob->parent;
    if (sb == NULL) {
        large_block* lb = (large_block*) ((char*) ptr - sizeof (large_block));
        raw_deallocate((char*) lb - lb->indent, lb->total_size);
    } else {
        sb->lock();
        heap * parent_heap = sb->get_parent();
        parent_heap->lock();
        parent_heap->push_free_block((char*) ob - ob->indent, sb);
        superblock * deletion_candidate = parent_heap->get_deletion_candidate();
        if (deletion_candidate != NULL) {
            parent_heap->remove_superblock(deletion_candidate);
            heap * global_heap = get_global_heap();
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
    //        memcpy(new_ptr, ptr, size < old_size ? size : old_size);
    deallocate(ptr);
    return new_ptr;
}


