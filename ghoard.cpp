#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <sys/mman.h>
#include <algorithm>


namespace ghoard{
    const PAGE_SIZE = sysconf(_SC_PAGESIZE);
    const BLOCK_SIZE = 8;
    const SUPERBLOCK_SIZE = 16 * PAGE_SIZE; 
    const SUPERBLOCK_BLOCK_COUNT = SUPER_BLOCK_SIZE/BLOCK_SIZE;
    const FGROUP_COUNT = 10;

    struct heap;
    struct superblock;

    struct large_block{
        size_t block_size;
        size_t indent;
        superblock * parent;
    };
    struct ordinary_block{
        size_t indent;
        superblock * parent;
    }; 
    struct empty_block{
        empty_block * next;
    };

    struct superblock
    {
        struct header_cls{
            heap * parent;
            size_t block_size;
            size_t free_block_cnt;
            superblock * flist_next;
            superblock * flist_prev;
            superblock * szlist_prev;
            superblock * szlist_next;
            empty_block * stack_head;
        } header;
        inline size_t get_block_with_meta_sz(){
            return header.block_size+sizeof(ordinary_block);
        }
        inline size_t get_block_count(){
            return (SUPER_BLOCK_SIZE-sizeof(header_cls))/get_block_with_meta_sz();
        }
        inline void * get_data_start(){
            return &header + sizeof(header_cls);
        }
        void initialize_blocks(){
            header.stack_head = get_data_start();
            header.stack_head->next = NULL;
            header.free_block_cnt = get_block_count();
            for(int i=1; i<header.free_block_cnt; ++i)
                (header.stack_head += get_block_with_meta_sz())->next = header.stack_head;
        }
        inline bool has_free_blocks(){
            return header.stack_head == NULL; 
        }
        void * get_free_block(){
            assert(header.stack_head != NULL);
            void * result = header.stack_head;
            header.stack_head = result->next;
            --header.free_block_cnt;
            return result;
        }
        void return_block_to_stack(void * block){
            ((empty_block*) block)->next = header.stack_head;
            header.stack = block;
            ++header.free_block_cnt;
        }
        inline int get_fgroup_id(){
            return (get_block_count()-free_block_cnt)*FGROUP_COUNT/get_block_count();
        }
    };
}


namespace ghoard 
{
    
}


