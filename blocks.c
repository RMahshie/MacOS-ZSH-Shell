#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include "blocks.h"
#include "bitmap.h"
#include "inode.h"

const int BLOCK_COUNT = 256;  // Total number of blocks
const int BLOCK_SIZE = 4096;  // Size of each block
static int blocks_fd = -1;    // File descriptor for blocks file
static void* blocks_base = 0; // Base address of mmap'd blocks file

// Initialize the blocks file
void
blocks_init(const char* path)
{
    printf("+ blocks_init(%s)\n", path);
    
    blocks_fd = open(path, O_CREAT | O_RDWR, 0644);
    assert(blocks_fd != -1);

    // Extend the file to the appropriate size
    int rv = ftruncate(blocks_fd, BLOCK_SIZE * BLOCK_COUNT);
    assert(rv == 0);

    // mmap the blocks file
    blocks_base = mmap(0, BLOCK_SIZE * BLOCK_COUNT,
        PROT_READ | PROT_WRITE, MAP_SHARED, blocks_fd, 0);
    assert(blocks_base != MAP_FAILED);

    // Initialize bitmaps if they're empty
    void* bbm = get_blocks_bitmap();
    void* ibm = get_inode_bitmap();
    
    // Mark system blocks as used
    bitmap_put(bbm, 0, 1); // Block bitmap
    bitmap_put(bbm, 1, 1); // Inode bitmap
    bitmap_put(bbm, 2, 1); // Inode table
}

// Clean up the blocks file
void
blocks_free()
{
    int rv = munmap(blocks_base, BLOCK_SIZE * BLOCK_COUNT);
    assert(rv == 0);
    rv = close(blocks_fd);
    assert(rv == 0);
}

// Get the blocks bitmap
void*
get_blocks_bitmap()
{
    return blocks_get_block(0);
}

// Get the inode bitmap
void*
get_inode_bitmap()
{
    return blocks_get_block(1);
}

// Get the inode table
inode_t*
get_inodes()
{
    return (inode_t*)blocks_get_block(2);
}

// Get a pointer to a specific block
void*
blocks_get_block(int bnum)
{
    assert(bnum >= 0 && bnum < BLOCK_COUNT);
    return blocks_base + (BLOCK_SIZE * bnum);
}

// Allocate a new block
int
alloc_block()
{
    void* bbm = get_blocks_bitmap();
    
    for (int ii = 3; ii < BLOCK_COUNT; ++ii) {
        if (!bitmap_get(bbm, ii)) {
            bitmap_put(bbm, ii, 1);
            printf("+ alloc_block() -> %d\n", ii);
            void* block = blocks_get_block(ii);
            memset(block, 0, BLOCK_SIZE);
            return ii;
        }
    }
    
    return -ENOSPC;
}

// Free an allocated block
void
free_block(int bnum)
{
    printf("+ free_block(%d)\n", bnum);
    void* bbm = get_blocks_bitmap();
    bitmap_put(bbm, bnum, 0);
    void* block = blocks_get_block(bnum);
    memset(block, 0, BLOCK_SIZE);
}