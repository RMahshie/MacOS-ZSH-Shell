#ifndef INODE_H
#define INODE_H

// inode structure
typedef struct inode {
    int refs;   // reference count
    int mode;   // permission & type
    int size;   // bytes
    int block;  // single block pointer (if max file size <= 4K)
} inode_t;

// print inode info for debugging
void print_inode(inode_t* node);

// get inode by number
inode_t* get_inode(int inum);

// allocate a new inode
int alloc_inode();

// free an allocated inode
void free_inode(int inum);

// grow an inode to a specific size
int grow_inode(inode_t* node, int size);

// shrink an inode to a specific size
int shrink_inode(inode_t* node, int size);

// get physical block number for file page
int inode_get_pnum(inode_t* node, int fpn);

#endif