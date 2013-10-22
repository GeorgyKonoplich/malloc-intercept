#include "heap.h"
#include "constants.h"
#include "superblock.h"
#include "tracing.h"
#include <unistd.h>
#include <sys/mman.h>
#include <cstddef>

static const bool PRINT_HEAP_TRACE = false;

void ghoard::heap::trace_debug(){
    trace("[heap used_bytes=",used_bytes," available_bytes=", available_bytes, " ");
    for(int i=FGROUP_COUNT-1; i>=0; --i){
        trace("fid=", i, " ");
        if(fgroup_heads[i] != NULL) trace("head=", fgroup_heads[i], " ", &fgroup_heads[i], " ");
        for(int j=0; j<SZ_CNT; ++j)if(*get_sz_head(i, j)!=NULL) trace("szh[", j, "]=", *get_sz_head(i, j), " ", get_sz_head(i, j), " ");
        trace("; ");
    }
    trace("\n");
}

void ghoard::heap::init() {
    superblock ** sz_heads = (superblock **) ((char*) this + sizeof(heap));
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
    return (used_bytes + K_THRESHOLD * SUPERBLOCK_SIZE < available_bytes)
            && (F_THRESHOLD_DENOMINATOR * (available_bytes - used_bytes)
            > F_THRESHOLD_NUMERATOR * available_bytes);
}

ghoard::superblock ** ghoard::heap::get_sz_head(int fid, int size_group) {
    return sz_heads + fid * SZ_CNT + size_group;
}

ghoard::superblock * ghoard::heap::get_deletion_candidate() {
    if (is_threshold_passed())
        for (int fid = 0; fid < FGROUP_COUNT; ++fid) {
            if (fgroup_heads[fid] != NULL) return fgroup_heads[fid];
        }
    return NULL;
}

void ghoard::heap::resize_superblock(superblock * sb, int sz_group){
    remove_superblock(sb);
    create_add_superblock(sb, sz_group);
}

void ghoard::heap::create_add_superblock(void * bytes, int sz_group) {
    superblock * sb = ((superblock*) bytes);
    sb->parent = this;
    sb->sz_group = sz_group;
    sb->init();
    sb->set_list_pointers_to_null();
    sb->mutex.init();
    insert_superblock(sb);
    if(PRINT_HEAP_TRACE) trace("heap::create_add_superblock(",bytes,", ",sz_group,"): sb->fgroup=", sb->get_fgroup_id(), " -> block_with_meta_sz=", sb->get_block_with_meta_sz(), " block_cnt=", sb->get_block_count(),"\n");
}

void ghoard::heap::remove_superblock(superblock * sb) {
    if(PRINT_HEAP_TRACE) trace("heap::remove_superblock(",sb,")::start: sb->fgroup=", sb->get_fgroup_id(), " used_bytes=",used_bytes," ", &used_bytes, " available_bytes=", available_bytes, " ", &available_bytes, "\n");
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
    if(PRINT_HEAP_TRACE) trace("-- sb=",sb," fid=",fid," head=",fgroup_heads[fid]," ", &fgroup_heads[fid]," sz_head=",*get_sz_head(fid, sb->sz_group)," ", get_sz_head(fid, sb->sz_group), ",  sb->fnext=",sb->fnext, "\n");
    size_t block_size = sb->get_block_with_meta_sz();
    sb->set_list_pointers_to_null();
    available_bytes -= sb->get_block_count() * block_size;
    used_bytes -= (sb->get_block_count() - sb->free_block_cnt) * block_size;
    if(PRINT_HEAP_TRACE) trace("heap::remove_superblock(",sb,")::finish used_bytes=",used_bytes," ", &used_bytes, " available_bytes=", available_bytes, " ", &available_bytes, "\n");
}

void ghoard::heap::insert_superblock(superblock * sb) {
    if(PRINT_HEAP_TRACE) trace("heap::insert_superblock(",sb,")::start: sb->fgroup=", sb->get_fgroup_id(), " used_bytes=",used_bytes," ", &used_bytes, " available_bytes=", available_bytes, " ", &available_bytes, "\n");
    int fid = sb->get_fgroup_id();

    if(PRINT_HEAP_TRACE) trace("-- 1 sb=",sb," fid=",fid," head=",fgroup_heads[fid]," ", &fgroup_heads[fid]," sz_head=",*get_sz_head(fid, sb->sz_group)," ", get_sz_head(fid, sb->sz_group), ",  sb->fnext=",sb->fnext, "\n");

    sb->fprev = NULL;
    sb->fnext = fgroup_heads[fid];
    if (fgroup_heads[fid] != NULL) fgroup_heads[fid]->fprev = sb;
    fgroup_heads[fid] = sb;

    superblock ** head = get_sz_head(fid, sb->sz_group);
    sb->szprev = NULL;
    sb->sznext = *head;
    if(*head != NULL) (*head)->szprev = sb;
    *head = sb;

    if(PRINT_HEAP_TRACE) trace("-- 2 sb=",sb," fid=",fid," head=",fgroup_heads[fid]," ", &fgroup_heads[fid]," sz_head=",*get_sz_head(fid, sb->sz_group)," ", get_sz_head(fid, sb->sz_group), ",  sb->fnext=",sb->fnext, "\n");
    
    size_t block_size = sb->get_block_with_meta_sz();
    available_bytes += sb->get_block_count() * block_size;
    used_bytes += (sb->get_block_count() - sb->free_block_cnt) * block_size;
    if(PRINT_HEAP_TRACE) trace("heap::insert_superblock(",sb,")::finish: sb->fgroup=", sb->get_fgroup_id(), " used_bytes=",used_bytes," ", &used_bytes, " available_bytes=", available_bytes, " ", &available_bytes, "\n");
    sb->parent = this;
}

void ghoard::heap::push_free_block(void * block, superblock * sb) {
    if(PRINT_HEAP_TRACE) trace("heap::push_free_block(", block,", ", sb, "): sb->fgroup=", sb->get_fgroup_id(), " used_bytes=",used_bytes," ", &used_bytes, " available_bytes=", available_bytes, " ", &available_bytes, "\n");
    remove_superblock(sb);
    sb->push_free_block(block);
    used_bytes -= sb->get_block_with_meta_sz();
    insert_superblock(sb);
}

void* ghoard::heap::pop_free_block(superblock* sb) {
    remove_superblock(sb);
    void * block = sb->pop_free_block();
    used_bytes += sb->get_block_with_meta_sz();
    insert_superblock(sb);
    if(PRINT_HEAP_TRACE) trace("heap::pop_free_block(", sb, "): sb->fgroup=", sb->get_fgroup_id(), " -> ", block, ")\n");
    return block;
}

ghoard::superblock * ghoard::heap::get_superblock_with_free_block(int sz_group) {
    for (int fid = FGROUP_COUNT - 2; fid >= 0; --fid) {
        superblock * sb = (*get_sz_head(fid, sz_group));
        //sb!=NULL is sufficient, cause we don't take in view totally-full
        //group (with fid=FGROUP_COUNT-1)
        if (sb != NULL) {
            if(PRINT_HEAP_TRACE) trace("found superblock: ", sb, "\n");
            return sb;
        }
    }
    return fgroup_heads[0];
}

void ghoard::heap::lock() {
    mutex.lock();
}

void ghoard::heap::unlock() {
    mutex.unlock();
}

int ghoard::heap::get_used_bytes(){
    return used_bytes;
}

int ghoard::heap::get_available_bytes(){
    return available_bytes;
}

void ghoard::heap::deallocate_all_superblocks(){
    for(int i= 0; i<FGROUP_COUNT; ++i){
        superblock * sb = fgroup_heads[i];
        while(sb != NULL){
            remove_superblock(sb);
            raw_deallocate(sb, SUPERBLOCK_SIZE);
            sb = fgroup_heads[i];
        }
    }
}
