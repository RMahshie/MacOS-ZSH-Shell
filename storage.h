// Disk storage abstracttion.
//
// Feel free to use as inspiration. Provided as-is.

// based on cs3650 starter code

#ifndef STORAGE_H
#define STORAGE_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "slist.h"

// initialize the file system storage
void storage_init(const char* path);

// get file or directory attributes
int storage_stat(const char* path, struct stat* st);

// read data from a file
int storage_read(const char* path, char* buf, size_t size, off_t offset);

// write data to a file
int storage_write(const char* path, const char* buf, size_t size, off_t offset);

// change the size of a file
int storage_truncate(const char* path, off_t size);

// create a new file
int storage_mknod(const char* path, int mode);

// delete a file
int storage_unlink(const char* path);

// create a hard link
int storage_link(const char* from, const char* to);

// rename a file
int storage_rename(const char* from, const char* to);

// set file timestamps
int storage_set_time(const char* path, const struct timespec ts[2]);

// list directory contents
slist_t* storage_list(const char* path);

// create a directory
int storage_mkdir(const char* path, int mode);

// remove a directory
int storage_rmdir(const char* path);

// helper function to get inode number from path
int get_inode_num(const char* path);

#endif