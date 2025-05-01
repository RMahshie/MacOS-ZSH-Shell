// based on cs3650 starter code

#define _FILE_OFFSET_BITS 64

#include <assert.h>
#include <bsd/string.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fuse.h>
#include <stdlib.h>

#include "storage.h"
#include "slist.h"
#include "nufs.h"

// check if a file exists and has the required permissions
static int nufs_access(const char *path, int mask) {
    // log the access attempt with the file path and permission mask
    printf("access(%s, %04o)\n", path, mask);
    // always allow access for simplicity
    return 0;
}

// retrieve the details of a file or directory
static int nufs_getattr(const char *path, struct stat *st) {
    // log the request to get attributes
    printf("\n=== FUSE: nufs_getattr(%s) ===\n", path);
    // get the file's attributes from storage
    int rv = storage_stat(path, st);
    // log the result of the attribute retrieval
    printf("getattr(%s) -> %d\n", path, rv);
    return rv;
}

// create a new file or device
static int nufs_mknod(const char *path, mode_t mode, dev_t rdev) {
    // log the creation request with the file path and mode
    printf("\n=== FUSE: nufs_mknod(%s, %04o) ===\n", path, mode);
    // create the node in storage
    int rv = storage_mknod(path, mode);
    // log the result of the creation
    printf("mknod(%s, %04o) -> %d\n", path, mode, rv);
    return rv;
}

// write data to a file at a specific offset
static int nufs_write(const char *path, const char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi) {
    // log the write operation with details
    printf("\n=== FUSE: nufs_write(%s, %ld bytes, @%ld) ===\n", path, size, offset);
    // perform the write in storage
    return storage_write(path, buf, size, offset);
}

// read data from a file at a specific offset
static int nufs_read(const char *path, char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi) {
    // log the read operation with details
    printf("\n=== FUSE: nufs_read(%s, %ld bytes, @%ld) ===\n", path, size, offset);
    // perform the read from storage
    return storage_read(path, buf, size, offset);
}

// initialize the filesystem when fuse mounts it
void *nufs_init(struct fuse_conn_info *conn) {
    // set up storage using the specified data file
    storage_init("data.nufs");
    return NULL;
}

// list the contents of a directory
int nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                off_t offset, struct fuse_file_info *fi) {
    // log the directory listing request
    printf("\n=== FUSE: nufs_readdir(%s) ===\n", path);
    struct stat st;
    // get the directory's attributes
    int rv = storage_stat(path, &st);
    // log the result of the attribute retrieval
    printf("readdir(%s) -> %d\n", path, rv);
    
    // get all items in the directory
    slist_t *items = storage_list(path);
    // go through each item and add it to the buffer
    for (slist_t *xs = items; xs != 0; xs = xs->next) {
        // log the attribute request for each item
        printf("+ storage_stat(//%s)\n", xs->data);
        // get the item's attributes
        storage_stat(xs->data, &st);
        // add the item to the fuse directory listing
        filler(buf, xs->data, &st, 0);
    }
    // free the list of items after processing
    s_free(items);
    
    return 0;
}

// create a new directory
static int nufs_mkdir(const char *path, mode_t mode) {
    // log the directory creation request
    printf("\n=== FUSE: nufs_mkdir(%s, %04o) ===\n", path, mode);
    // create the directory in storage with directory permissions
    return storage_mkdir(path, mode);
}

// remove a file from the filesystem
static int nufs_unlink(const char *path) {
    // log the file removal request
    printf("\n=== FUSE: nufs_unlink(%s) ===\n", path);
    // remove the file from storage
    int rv = storage_unlink(path);
    // log the result of the removal
    printf("unlink(%s) -> %d\n", path, rv);
    return rv;
}

// create a hard link to a file
static int nufs_link(const char *from, const char *to) {
    // log the link creation request
    printf("\n=== FUSE: nufs_link(%s => %s) ===\n", from, to);
    // create the hard link in storage
    int rv = storage_link(from, to);
    // log the result of the link creation
    printf("link(%s => %s) -> %d\n", from, to, rv);
    return rv;
}

// remove a directory from the filesystem
static int nufs_rmdir(const char *path) {
    // log the directory removal request
    printf("\n=== FUSE: nufs_rmdir(%s) ===\n", path);
    // remove the directory from storage
    return storage_rmdir(path);
}

// rename or move a file within the filesystem
static int nufs_rename(const char *from, const char *to) {
    // log the rename operation
    printf("\n=== FUSE: nufs_rename(%s => %s) ===\n", from, to);
    // rename the file in storage
    int rv = storage_rename(from, to);
    // log the result of the rename
    printf("rename(%s => %s) -> %d\n", from, to, rv);
    return rv;
}

// change the permissions of a file or directory
static int nufs_chmod(const char *path, mode_t mode) {
    // log the permission change request
    printf("\n=== FUSE: nufs_chmod(%s, %04o) ===\n", path, mode);
    // currently does nothing and always succeeds
    return 0;
}

// change the size of a file
static int nufs_truncate(const char *path, off_t size) {
    // log the truncate request
    printf("\n=== FUSE: nufs_truncate(%s, %ld) ===\n", path, size);
    // adjust the file size in storage
    return storage_truncate(path, size);
}

// open a file (no action needed for this simple filesystem)
static int nufs_open(const char *path, struct fuse_file_info *fi) {
    // log the open request
    printf("\n=== FUSE: nufs_open(%s) ===\n", path);
    // always succeed for simplicity
    return 0;
}

// update the access and modification times of a file
static int nufs_utimens(const char *path, const struct timespec ts[2]) {
    // log the time update request
    printf("\n=== FUSE: nufs_utimens(%s) ===\n", path);
    // update the file times in storage
    return storage_set_time(path, ts);
}

// define the operations that fuse will handle and link them to our functions
static struct fuse_operations nufs_ops = {
    .init = nufs_init,
    .access = nufs_access,
    .getattr = nufs_getattr,
    .readdir = nufs_readdir,
    .mknod = nufs_mknod,
    .mkdir = nufs_mkdir,
    .write = nufs_write,
    .read = nufs_read,
    .unlink = nufs_unlink,
    .rmdir = nufs_rmdir,
    .rename = nufs_rename,
    .truncate = nufs_truncate,
    .open = nufs_open,
    .utimens = nufs_utimens,
    .link = nufs_link,
};

// start the fuse filesystem
int main(int argc, char *argv[]) {
    // ensure the program has the right number of arguments
    assert(argc > 2 && argc < 6);
    // log the mount point
    printf("todo: mount %s as data file\n", argv[argc-1]);
    // initialize the storage with the specified data file
    storage_init(argv[argc-1]);
    // launch the fuse main loop with our defined operations
    return fuse_main(argc-1, argv, &nufs_ops, NULL);
}
