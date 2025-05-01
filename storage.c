// storage.c: handles high-level file system operations

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <libgen.h>
#include "storage.h"
#include "slist.h"
#include "directory.h"
#include "inode.h"
#include "blocks.h"
#include "bitmap.h"

// helper function to get the minimum of two size_t values
static inline size_t min(size_t x, size_t y) {
    return (x < y) ? x : y;
}

// initialize the storage system
void
storage_init(const char* path)
{
    // initialize the block system with the given path
    blocks_init(path);
    
    // get the inode bitmap to track used and free inodes
    void* ibm = get_inode_bitmap();
    
    // check if the root inode (inode 0) is already initialized
    if (!bitmap_get(ibm, 0)) {  // only initialize if root doesn't exist
        // mark root inode as used in the bitmap
        bitmap_put(ibm, 0, 1);  
        
        // setup the root directory inode
        inode_t* root = get_inode(0);
        root->mode = 040755;  // set mode to directory with standard permissions
        root->refs = 1;       // initialize reference count to 1
        
        // allocate a block for the root directory
        int bnum = alloc_block();
        printf("+ storage_init: root block = %d\n", bnum);
        root->block = bnum;
        
        // initialize "." and ".." entries in the root directory
        directory_put(root, ".", 0);
        directory_put(root, "..", 0);
        printf("Directory initialized\n");
    }
}

// get the parent directory inode and the name of the target within it
int
get_parent_dir(const char* path, inode_t** dd, char* name)
{
    printf("+ get_parent_dir(%s)\n", path);
    
    // handle the special case where the path is the root directory
    if (strcmp(path, "/") == 0) {
        *dd = get_inode(0);       // root inode
        strcpy(name, "");         // no name since it's the root
        return 0;
    }
    
    // duplicate the path to manipulate dirname and basename
    char* tmp1 = strdup(path);
    char* tmp2 = strdup(path);
    
    // extract the directory name and base name from the path
    char* dname = dirname(tmp1);
    char* bname = basename(tmp2);
    
    // get the inode number of the parent directory
    int pnum = get_inode_num(dname);
    if (pnum < 0) {
        free(tmp1);
        free(tmp2);
        return pnum;  // return error if parent directory doesn't exist
    }
    
    // set the parent directory inode
    *dd = get_inode(pnum);
    // copy the base name (target name) into the provided buffer
    strcpy(name, bname);
    
    // free the duplicated strings
    free(tmp1);
    free(tmp2);
    return 0;
}

// get the inode number associated with a given path
int
get_inode_num(const char* path)
{
    printf("+ get_inode_num(%s)\n", path);
    
    // handle the root directory
    if (strcmp(path, "/") == 0) {
        return 0;  // root inode number is 0
    }
    
    // skip the leading slash for processing
    if (path[0] == '/') {
        path++;
    }
    
    // duplicate the path to safely tokenize it
    char* tmp = strdup(path);
    char* rest = tmp;
    char* part;
    int current_inum = 0;  // start traversal from the root inode
    
    // iterate through each component of the path
    while ((part = strsep(&rest, "/"))) {
        // skip empty components resulting from consecutive slashes
        if (strlen(part) == 0) {
            continue;
        }
        
        // get the current directory inode
        inode_t* dd = get_inode(current_inum);
        // look up the next part in the current directory
        int inum = directory_lookup(dd, part);
        if (inum < 0) {
            free(tmp);
            return inum;  // return error if the component is not found
        }
        // move to the inode number of the found component
        current_inum = inum;
    }
    
    // free the duplicated path string
    free(tmp);
    return current_inum;  // return the final inode number
}

// get the file attributes for a given path
int
storage_stat(const char* path, struct stat* st)
{
    printf("+ storage_stat(%s)\n", path);
    memset(st, 0, sizeof(struct stat));  // clear the stat structure
    
    // handle the root directory
    if (strcmp(path, "/") == 0) {
        inode_t* root = get_inode(0);
        st->st_mode = root->mode;       // set mode from inode
        st->st_size = root->size;       // set size from inode
        st->st_uid = getuid();           // set user ID
        st->st_nlink = root->refs;      // set number of links
        return 0;                        // success
    }
    
    // get the inode number for the given path
    int inum = get_inode_num(path);
    if (inum < 0) {
        return -ENOENT;  // return error if inode is not found
    }
    
    // get the inode structure
    inode_t* node = get_inode(inum);
    st->st_mode = node->mode;    // set mode from inode
    st->st_size = node->size;    // set size from inode
    st->st_uid = getuid();        // set user ID
    st->st_nlink = node->refs;   // set number of links
    
    return 0;  // success
}

// read data from a file at a given offset
int
storage_read(const char* path, char* buf, size_t size, off_t offset)
{
    printf("+ storage_read(%s, %ld bytes, @%ld)\n", path, size, offset);
    // get the inode number for the path
    int inum = get_inode_num(path);
    if (inum < 0) {
        return inum;  // return error if inode is not found
    }
    
    // get the inode structure
    inode_t* node = get_inode(inum);
    // if the offset is beyond the file size, nothing to read
    if (offset >= node->size) {
        return 0;
    }
    
    // if there's no block allocated, nothing to read
    if (!node->block) {
        return 0;
    }
    
    // adjust the size to read only up to the file size
    size = min(size, node->size - offset);
    // get the data block
    void* data = blocks_get_block(node->block);
    // copy the data from the block to the buffer
    memcpy(buf, data + offset, size);
    
    return size;  // return the number of bytes read
}

// write data to a file at a given offset
int
storage_write(const char* path, const char* buf, size_t size, off_t offset)
{
    printf("+ storage_write(%s, %ld bytes, @%ld)\n", path, size, offset);
    // get the inode number for the path
    int inum = get_inode_num(path);
    if (inum < 0) {
        return inum;  // return error if inode is not found
    }
    
    // get the inode structure
    inode_t* node = get_inode(inum);
    
    // allocate a block if the file doesn't have one yet
    if (!node->block) {
        int bnum = alloc_block();
        if (bnum < 0) {
            return bnum;  // return error if block allocation fails
        }
        node->block = bnum;  // assign the new block to the inode
    }
    
    // grow the file size if the write exceeds the current size
    if (offset + size > node->size) {
        node->size = offset + size;
    }
    
    // get the data block
    void* data = blocks_get_block(node->block);
    // copy the data from the buffer to the block at the specified offset
    memcpy(data + offset, buf, size);
    
    return size;  // return the number of bytes written
}

// change the size of a file
int
storage_truncate(const char* path, off_t size)
{
    // get the inode number for the path
    int inum = get_inode_num(path);
    if (inum < 0) {
        return inum;  // return error if inode is not found
    }
    
    // get the inode structure
    inode_t* node = get_inode(inum);
    
    // check if the new size exceeds the maximum allowed size
    if (size > 4096) {
        return -EFBIG;  // file too large
    }
    
    // if the new size is smaller, shrink the inode
    if (size < node->size) {
        return shrink_inode(node, size);
    }
    else {  // otherwise, grow the inode
        return grow_inode(node, size);
    }
}

// create a filesystem node (file, device special file, etc.)
int
storage_mknod(const char* path, int mode)
{
    printf("+ storage_mknod(%s, %04o)\n", path, mode);
    // check if the inode for the path already exists
    if (get_inode_num(path) >= 0) {
        return -EEXIST;  // file already exists
    }
    
    inode_t* dd;
    char name[DIR_NAME_LENGTH];
    // get the parent directory inode and the target name
    int rv = get_parent_dir(path, &dd, name);
    if (rv < 0) {
        return rv;  // return error if parent directory is not found
    }
    
    // allocate a new inode for the file
    int inum = alloc_inode();
    if (inum < 0) {
        return inum;  // return error if inode allocation fails
    }
    
    // initialize the new inode
    inode_t* node = get_inode(inum);
    node->mode = mode;    // set the mode (file type and permissions)
    node->size = 0;       // initialize size to 0
    node->refs = 1;       // set reference count to 1
    printf("+ storage_mknod: created inode %d\n", inum);
    
    // add the new inode to the parent directory
    rv = directory_put(dd, name, inum);
    if (rv < 0) {
        free_inode(inum);  // free the inode if adding to directory fails
        return rv;         // return the error
    }
    
    return 0;  // success
}

// remove a file from the filesystem
int
storage_unlink(const char* path)
{
    printf("+ storage_unlink(%s)\n", path);
    // get the inode number for the path
    int inum = get_inode_num(path);
    if (inum < 0) {
        return inum;  // return error if inode is not found
    }
    
    // get the inode structure
    inode_t* node = get_inode(inum);
    node->refs -= 1;  // decrement the reference count
    
    // if no more references, free the inode and its block
    if (node->refs <= 0) {
        if (node->block) {
            free_block(node->block);  // free the data block
            node->block = 0;           // reset the block number
        }
        free_inode(inum);  // free the inode
    }
    
    inode_t* dd;
    char name[DIR_NAME_LENGTH];
    // get the parent directory inode and the target name
    int rv = get_parent_dir(path, &dd, name);
    if (rv < 0) {
        return rv;  // return error if parent directory is not found
    }
    
    // delete the entry from the parent directory
    return directory_delete(dd, name);
}

// create a hard link from one path to another
int
storage_link(const char* from, const char* to)
{
    // get the inode number of the source path
    int from_inum = get_inode_num(from);
    if (from_inum < 0) {
        return from_inum;  // return error if source inode is not found
    }
    
    // check if the destination path already exists
    if (get_inode_num(to) >= 0) {
        return -EEXIST;  // destination already exists
    }
    
    // get the inode structure of the source
    inode_t* node = get_inode(from_inum);
    node->refs += 1;  // increment the reference count
    
    inode_t* dd;
    char name[DIR_NAME_LENGTH];
    // get the parent directory inode and the destination name
    int rv = get_parent_dir(to, &dd, name);
    if (rv < 0) {
        return rv;  // return error if parent directory is not found
    }
    
    // add the hard link to the destination directory
    return directory_put(dd, name, from_inum);
}

// rename or move a file within the filesystem
int
storage_rename(const char* from, const char* to)
{
    // create a hard link to the new destination
    int rv = storage_link(from, to);
    if (rv < 0) {
        return rv;  // return error if linking fails
    }
    // remove the original link
    return storage_unlink(from);
}

// set the access and modification times of a file
int
storage_set_time(const char* path, const struct timespec ts[2])
{
    // currently does nothing and returns success
    return 0;
}

// list the contents of a directory
slist_t*
storage_list(const char* path)
{
    // delegate to the directory listing function
    return directory_list(path);
}

// create a new directory
int
storage_mkdir(const char* path, int mode)
{
    printf("+ storage_mkdir(%s)\n", path);
    // create the directory node with directory mode
    int rv = storage_mknod(path, mode | 040000);
    if (rv < 0) {
        return rv;  // return error if mknod fails
    }
    
    // get the inode number of the newly created directory
    int inum = get_inode_num(path);
    inode_t* node = get_inode(inum);
    
    // initialize "." entry pointing to itself
    directory_put(node, ".", inum);
    
    inode_t* dd;
    char name[DIR_NAME_LENGTH];
    // get the parent directory inode and the directory name
    get_parent_dir(path, &dd, name);
    // initialize ".." entry pointing to the parent directory
    directory_put(node, "..", get_inode_num(path));
    
    return 0;  // success
}

// remove a directory from the filesystem
int
storage_rmdir(const char* path)
{
    // get the inode number for the path
    int inum = get_inode_num(path);
    if (inum < 0) {
        return inum;  // return error if inode is not found
    }
    
    // get the inode structure
    inode_t* node = get_inode(inum);
    // verify that the inode is a directory
    if (!(node->mode & 040000)) {
        return -ENOTDIR;  // not a directory
    }
    
    // get the directory entries from the block
    dirent_t* ents = blocks_get_block(node->block);
    int count = node->size / sizeof(dirent_t);
    
    // iterate through the directory entries to check if it's empty
    for (int ii = 0; ii < count; ++ii) {
        // skip "." and ".." entries
        if (ents[ii].inum != 0 && 
            strcmp(ents[ii].name, ".") != 0 && 
            strcmp(ents[ii].name, "..") != 0) {
            return -ENOTEMPTY;  // directory is not empty
        }
    }
    
    // unlink the directory after confirming it's empty
    return storage_unlink(path);
}
