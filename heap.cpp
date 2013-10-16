#include "heap.h"
#include "constants.h"
#include "superblock.h"
#include <unistd.h>
#include <sys/mman.h>
#include <cstddef>

using namespace std;

namespace ghoard{
    void heap::init(){
        used_bytes = 0;
        available_bytes = 0;
        for(int i=0; i< FGROUP_COUNT; ++i){
            fgroup_heads[i] = NULL;
            for(int j = 0; j< SZ_CNT; ++j){
                *get_sz_head(i, j) = NULL;
            }
        }
    }
    bool heap::is_threshold_passed(){
        return (used_bytes < available_bytes - K_THRESHOLD*SUPERBLOCK_SIZE)
            && (F_THRESHOLD_DENOMINATOR*(available_bytes-used_bytes)
                    > F_THRESHOLD_NUMERATOR*available_bytes);
    }
    superblock ** heap::get_sz_head(int fid, int size_group){
        return sz_heads + fid * SZ_CNT + size_group;
    }
    superblock * heap::get_deletion_candidate(){
        if(is_threshold_passed())
            for(int fid = 0; fid < FGROUP_COUNT; ++fid){
                if(fgroup_heads[fid] != NULL) return fgroup_heads[fid];
            }
        return NULL;
    }
    void heap::create_add_superblock(void * bytes, int sz_group){
        superblock * sb = ((superblock*) bytes);
        sb->parent = this;
        sb->sz_group = sz_group;
        sb->initialize_blocks();
        sb->set_list_pointers_to_null();
        reinsert_superblock(sb);
    }
    void heap::remove_superblock(superblock * sb){
        if(sb->fprev != NULL) sb->fprev->fnext = sb->fnext;
        if(sb->szprev != NULL) sb->szprev->sznext = sb->sznext;
        if(sb->fnext != NULL) sb->fnext->fprev = sb->fprev;
        if(sb->sznext != NULL) sb->sznext->szprev = sb->szprev;
        int fid = sb->get_fgroup_id();
        if(sb == fgroup_heads[fid]){
            fgroup_heads[fid] = sb->fnext;
        }
        if(sb == *get_sz_head(fid, sb->sz_group)){
            *get_sz_head(fid, sb->sz_group) = sb->sznext;
        }
        sb->set_list_pointers_to_null();
    }

    void heap::insert_superblock_into_list(superblock * sb, superblock ** head){
        sb->fprev = NULL;
        sb->fnext = *head;
        if(*head != NULL) (*head)->fprev = sb;
        *head = sb;
    }
    void heap::insert_superblock(superblock * sb){
        int fid = sb->get_fgroup_id();
        insert_superblock_into_list(sb, &fgroup_heads[fid]);
        insert_superblock_into_list(sb, get_sz_head(fid, sb->sz_group));
    }
    void heap::reinsert_superblock(superblock * sb){
        remove_superblock(sb);
        insert_superblock(sb);
    }

    superblock * heap::get_superblock_with_free_block(int sz_group){
        for(int fid = FGROUP_COUNT-2; fid>=0; --fid){
            superblock * sb = (*get_sz_head(fid, sz_group));
            //sb!=NULL is sufficient, cause we don't take in view totally-full
            //group (with fid=FGROUP_COUNT-1)
            if (sb != NULL){
                return sb;
            }
        }
        return NULL;
    }
}

