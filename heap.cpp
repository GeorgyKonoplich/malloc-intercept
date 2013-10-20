#include "heap.h"
#include "constants.h"
#include "superblock.h"
#include <unistd.h>
#include <sys/mman.h>
#include <cstddef>

void ghoard::heap::init() {
    superblock ** sz_heads = (superblock **) ((char*) this +HEAP_SIZE);
    this->sz_heads = sz_heads;
    used_bytes = 0;
    available_bytes = 0;
    for (int i = 0; i < FGROUP_COUNT; ++i) {
        fgroup_heads[i] = NULL;
        for (int j = 0; j < SZ_CNT; ++j) {
            *get_sz_head(i, j) = NULL;
        }
    }
    mutex.init();
}

bool ghoard::heap::is_threshold_passed() {
    return (used_bytes < available_bytes - K_THRESHOLD * SUPERBLOCK_SIZE)
            && (F_THRESHOLD_DENOMINATOR * (available_bytes - used_bytes)
            > F_THRESHOLD_NUMERATOR * available_bytes);
}

superblock ** ghoard::heap::get_sz_head(int fid, int size_group) {
    return sz_heads + fid * SZ_CNT + size_group;
}

superblock * ghoard::heap::get_deletion_candidate() {
    if (is_threshold_passed())
        for (int fid = 0; fid < FGROUP_COUNT; ++fid) {
            if (fgroup_heads[fid] != NULL) return fgroup_heads[fid];
        }
    return NULL;
}

void ghoard::heap::create_add_superblock(void * bytes, int sz_group) {
    superblock * sb = ((superblock*) bytes);
    sb->parent = this;
    sb->sz_group = sz_group;
    sb->initialize_blocks();
    sb->set_list_pointers_to_null();
    sb->mutex.init();
    insert_superblock(sb);
}

void ghoard::heap::remove_superblock(superblock * sb) {
    if (sb->fprev != NULL) sb->fprev->fnext = sb->fnext;
    if (sb->szprev != NULL) sb->szprev->sznext = sb->sznext;
    if (sb->fnext != NULL) sb->fnext->fprev = sb->fprev;
    if (sb->sznext != NULL) sb->sznext->szprev = sb->szprev;
    int fid = sb->get_fgroup_id();
    if (sb == fgroup_heads[fid]) {
        fgroup_heads[fid] = sb->fnext;
    }
    if (sb == *get_sz_head(fid, sb->sz_group)) {
        *get_sz_head(fid, sb->sz_group) = sb->sznext;
    }
    size_t block_size = get_block_size(sb->get_sz_group());
    sb->set_list_pointers_to_null();
    available_bytes -= sb->get_block_count() * block_size;
    used_bytes -= (sb->get_block_count() - sb->free_block_cnt) * block_size;
}

void ghoard::heap::insert_superblock_into_list(superblock * sb, superblock ** head) {
    sb->fprev = NULL;
    sb->fnext = *head;
    if (*head != NULL) (*head)->fprev = sb;
    *head = sb;
}

void ghoard::heap::insert_superblock(superblock * sb) {
    int fid = sb->get_fgroup_id();
    insert_superblock_into_list(sb, &fgroup_heads[fid]);
    insert_superblock_into_list(sb, get_sz_head(fid, sb->sz_group));
    size_t block_size = get_block_size(sb->sz_group);
    available_bytes += sb->get_block_count() * block_size;
    used_bytes += (sb->get_block_count() - sb->free_block_cnt) * block_size;
}

void ghoard::heap::reinsert_superblock(superblock * sb) {
    remove_superblock(sb);
    insert_superblock(sb);
}

void ghoard::heap::push_free_block(void * block, superblock * sb) {
    sb->push_free_block(block);
    used_bytes -= get_block_size(sb->sz_group);
    reinsert_superblock(sb);
}

void* ghoard::heap::pop_free_block(superblock* sb) {
    void * block = sb->pop_free_block();
    used_bytes += get_block_size(sb->sz_group);
    reinsert_superblock(sb);
    return block;
}

ghoard::superblock * ghoard::heap::get_superblock_with_free_block(int sz_group) {
    for (int fid = FGROUP_COUNT - 2; fid >= 0; --fid) {
        superblock * sb = (*get_sz_head(fid, sz_group));
        //sb!=NULL is sufficient, cause we don't take in view totally-full
        //group (with fid=FGROUP_COUNT-1)
        if (sb != NULL) {
            return sb;
        }
    }
    if (fgroup_heads[0] != NULL) return fgroup_heads[0];
    return NULL;
}

void ghoard::heap::lock() {
    mutex.lock();
}

void ghoard::heap::unlock() {
    mutex.lock();
}

