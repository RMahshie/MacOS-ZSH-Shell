#include <stdio.h>
#include <string.h>
#include "inode.h"
#include "blocks.h"
#include "bitmap.h"

// function to get the inode structure for a given inode number
inode_t* get_inode(int inum) {
    // get the array of inodes
    inode_t* nodes = get_inodes();
    // return the address of the inode at the given index
    return &(nodes[inum]);
}

// function to allocate a new inode and return its number
int alloc_inode() {
    // get the inode bitmap
    void *ibm = get_inode_bitmap();
    
    // iterate through possible inode numbers
    for (int ii = 0; ii < 256; ++ii) {
        // check if the inode is free
        if (!bitmap_get(ibm, ii)) {
            // mark the inode as used
            bitmap_put(ibm, ii, 1);
            // print the allocated inode number
            printf("+ alloc_inode() -> %d\n", ii);
            // return the allocated inode number
            return ii;
        }
    }
    
    // return -1 if no inode is available
    return -1;
}

// function to free an inode given its number
void free_inode(int inum) {
    // get the inode bitmap
    void *ibm = get_inode_bitmap();
    // mark the inode as free
    bitmap_put(ibm, inum, 0);
}

// function to grow the size of an inode
int grow_inode(inode_t *node, int size) {
    // check if the new size exceeds the maximum allowed
    if (size > 4096) {
        return -1;
    }
    
    // if the new size is greater than the current size
    if (size > node->size) {
        // check if the inode has an allocated block
        if (node->block == 0) {
            // allocate a new block
            int bnum = alloc_block();
            // return error if block allocation fails
            if (bnum < 0) {
                return -1;
            }
            // assign the new block to the inode
            node->block = bnum;
        }
    }
    
    // update the inode size
    node->size = size;
    return 0;
}

// function to shrink the size of an inode
int shrink_inode(inode_t *node, int size) {
    // if the new size is less than the current size
    if (size < node->size) {
        // if the new size is zero
        if (size == 0) {
            // if the inode has an allocated block
            if (node->block != 0) {
                // free the allocated block
                free_block(node->block);
                // set the block number to zero
                node->block = 0;
            }
        }
        // update the inode size
        node->size = size;
    }
    return 0;
}

// function to get the physical block number from a file's page number
int inode_get_pnum(inode_t *node, int fpn) {
    // if the file page number is zero, return the direct block
    if (fpn == 0) {
        return node->block;
    }
    else {
        // return -1 for unsupported page numbers
        return -1;
    }
}
