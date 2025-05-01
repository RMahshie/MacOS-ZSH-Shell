#ifndef BLOCKS_H
#define BLOCKS_H

#include "inode.h"

// total number of blocks in file system
extern const int BLOCK_COUNT;

// size of each block in bytes
extern const int BLOCK_SIZE;

// initialize the blocks system
void blocks_init(const char* path);

// clean up the blocks system
void blocks_free();

// get pointer to a specific block
void* blocks_get_block(int bnum);

// allocate a new block
int alloc_block();

// free an allocated block
void free_block(int bnum);

// get the blocks bitmap
void* get_blocks_bitmap();

// get the inode bitmap
void* get_inode_bitmap();

// get the inode table
inode_t* get_inodes();

#endif