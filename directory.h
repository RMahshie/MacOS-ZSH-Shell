// Directory manipulation functions.
//
// Feel free to use as inspiration. Provided as-is.

// Based on cs3650 starter code
#ifndef DIRECTORY_H
#define DIRECTORY_H

#define DIR_NAME_LENGTH 48

#include "inode.h"
#include "slist.h"

// directory entry structure
typedef struct directory_entry {
    char name[DIR_NAME_LENGTH];  // filename
    int inum;                    // inode number
} dirent_t;

// look up a file in a directory
int directory_lookup(inode_t* dd, const char* name);

// add an entry to a directory
int directory_put(inode_t* dd, const char* name, int inum);

// remove an entry from a directory
int directory_delete(inode_t* dd, const char* name);

// list contents of a directory
slist_t* directory_list(const char* path);

#endif