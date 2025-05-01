#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "directory.h"
#include "blocks.h"
#include "inode.h"
#include "slist.h"
#include "bitmap.h"
#include "storage.h"

// function to look up a name in a directory and return its inode number
int
directory_lookup(inode_t* dd, const char* name)
{
    // get the directory entries from the block associated with the inode
    dirent_t* ents = blocks_get_block(dd->block);
    // calculate the number of entries in the directory
    int count = dd->size / sizeof(dirent_t);
    
    // iterate through each directory entry
    for (int ii = 0; ii < count; ++ii) {
        // compare the entry name with the target name
        if (strcmp(ents[ii].name, name) == 0) {
            // return the inode number if a match is found
            return ents[ii].inum;
        }
    }
    
    // return error if the name is not found
    return -ENOENT;
}

// function to add a name and inode number to a directory
int
directory_put(inode_t* dd, const char* name, int inum)
{
    // check if the directory has an allocated block
    if (!dd->block) {
        // allocate a new block for the directory
        int bnum = alloc_block();
        // return error if block allocation fails
        if (bnum < 0) {
            return bnum;
        }
        // assign the allocated block to the directory inode
        dd->block = bnum;
    }
    
    // get the directory entries from the block
    dirent_t* ents = blocks_get_block(dd->block);
    // calculate the current number of entries
    int count = dd->size / sizeof(dirent_t);
    
    // first, try to find an empty slot in the directory
    for (int ii = 0; ii < count; ++ii) {
        // check if the inode number is zero, indicating an empty slot
        if (ents[ii].inum == 0) {
            // copy the name into the entry, ensuring it is null-terminated
            strncpy(ents[ii].name, name, DIR_NAME_LENGTH - 1);
            ents[ii].name[DIR_NAME_LENGTH - 1] = 0;
            // assign the inode number to the entry
            ents[ii].inum = inum;
            // return success
            return 0;
        }
    }
    
    // if no empty slot is found, try to add the entry at the end
    if (count < (BLOCK_SIZE / sizeof(dirent_t))) {
        // copy the name into the new entry, ensuring it is null-terminated
        strncpy(ents[count].name, name, DIR_NAME_LENGTH - 1);
        ents[count].name[DIR_NAME_LENGTH - 1] = 0;
        // assign the inode number to the new entry
        ents[count].inum = inum;
        // update the directory size to include the new entry
        dd->size += sizeof(dirent_t);
        // return success
        return 0;
    }
    
    // return error if there is no space left in the directory
    return -ENOSPC;
}

// function to delete a name from a directory
int
directory_delete(inode_t* dd, const char* name)
{
    // get the directory entries from the block
    dirent_t* ents = blocks_get_block(dd->block);
    // calculate the number of entries in the directory
    int count = dd->size / sizeof(dirent_t);
    
    // iterate through each directory entry
    for (int ii = 0; ii < count; ++ii) {
        // compare the entry name with the target name
        if (strcmp(ents[ii].name, name) == 0) {
            // set the inode number to zero to mark the entry as deleted
            ents[ii].inum = 0;
            // clear the name field by setting it to zero
            memset(ents[ii].name, 0, DIR_NAME_LENGTH);
            // return success
            return 0;
        }
    }
    
    // return error if the name is not found
    return -ENOENT;
}

// function to list all entries in a directory given a path
slist_t*
directory_list(const char* path)
{
    // print the directory listing request
    printf("+ directory_list(%s)\n", path);
    // get the inode number associated with the path
    int inum = get_inode_num(path);
    // return null if the inode number is invalid
    if (inum < 0) {
        return 0;
    }
    
    // retrieve the inode structure using the inode number
    inode_t* dd = get_inode(inum);
    // check if the inode represents a directory by verifying the mode
    if (!(dd->mode & 040000)) {  // check if it's a directory
        return 0;
    }
    
    // initialize an empty singly linked list for the directory entries
    slist_t* ys = 0;
    // get the directory entries from the block
    dirent_t* ents = blocks_get_block(dd->block);
    // calculate the number of entries in the directory
    int count = dd->size / sizeof(dirent_t);
    
    // iterate through each directory entry
    for (int ii = 0; ii < count; ++ii) {
        // check if the entry is valid by verifying the inode number
        if (ents[ii].inum != 0) {
            // print the found file name
            printf("+ found file: %s\n", ents[ii].name);
            // add the file name to the singly linked list
            ys = s_cons(ents[ii].name, ys);
        }
    }
    
    // return the list of directory entries
    return ys;
}
