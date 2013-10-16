#include <stdio.h>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <sys/mman.h>
#include <algorithm>
#include <cerrno>

const size_t PAGE_SIZE = sysconf(_SC_PAGESIZE);

int main(void) 
{
    printf("The page size for this system is %ld bytes.\n", PAGE_SIZE)  ; /* _SC_PAGE_SIZE is OK too. */
    printf("%lld %lld\n", sizeof(size_t), sizeof(void*));

    
    return 0 ;
}
