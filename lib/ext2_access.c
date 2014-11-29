#include <string.h>

// ext2 definitions from the real driver in the Linux kernel.
#include "ext2fs.h"

// This header allows your project to link against the reference library. If you
// complete the entire project, you should be able to remove this directive and
// still compile your code.
#include "reference_implementation.h"

// Definitions for ext2cat to compile against.
#include "ext2_access.h"



///////////////////////////////////////////////////////////
//  Accessors for the basic components of ext2.
///////////////////////////////////////////////////////////

// Return a pointer to the primary superblock of a filesystem.
struct ext2_super_block * get_super_block(void * fs) {
    // FIXME: Uses reference implementation.

    return (struct ext2_super_block*)((char*)fs + 1024);
}


// Return the block size for a filesystem.
__u32 get_block_size(void * fs) {
    // FIXME: Uses reference implementation.
    void *temp = (void*)((char*)get_super_block(fs) + 24);
    __u32 blk_size = 1024 << *(int*)temp;
    return blk_size;
}


// Return a pointer to a block given its number.
// get_block(fs, 0) == fs;
void * get_block(void * fs, __u32 block_num) {
    // FIXME: Uses reference implementation.
    void *blk = (char*)fs + get_block_size(fs) * block_num;
    return blk;
}


// Return a pointer to the first block group descriptor in a filesystem. Real
// ext2 filesystems will have several of these, but, for simplicity, we will
// assume there is only one.
struct ext2_group_desc * get_block_group(void * fs, __u32 block_group_num) {
    // FIXME: Uses reference implementation.
    int i=0;
    while((char*)fs + i * get_block_size(fs) < (char*)get_super_block(fs) + 1024) {
      i++;
    }
    void *temp = (char*)fs + i * get_block_size(fs);
    return (struct ext2_group_desc*)temp;
}


// Return a pointer to an inode given its number. In a real filesystem, this
// would require finding the correct block group, but you may assume it's in the
// first one.
struct ext2_inode * get_inode(void * fs, __u32 inode_num) {
    // FIXME: Uses reference implementation.
    __u32 *inode_table = (__u32*)((char*)get_block_group(fs, 0) + 8);
    void *ptr = get_block(fs, *inode_table); 
    void *inode = (char*)ptr + 128 * (inode_num - 1);
    return (struct ext2_inode*)inode;
}



///////////////////////////////////////////////////////////
//  High-level code for accessing filesystem components by path.
///////////////////////////////////////////////////////////

// Chunk a filename into pieces.
// split_path("/a/b/c") will return {"a", "b", "c"}.
//
// This one's a freebie.
char ** split_path(char * path) {
    int num_slashes = 0;
    for (char * slash = path; slash != NULL; slash = strchr(slash + 1, '/')) {
        num_slashes++;
    }

    // Copy out each piece by advancing two pointers (piece_start and slash).
    char ** parts = (char **) calloc(num_slashes, sizeof(char *));
    char * piece_start = path + 1;
    int i = 0;
    for (char * slash = strchr(path + 1, '/');
         slash != NULL;
         slash = strchr(slash + 1, '/')) {
        int part_len = slash - piece_start;
        parts[i] = (char *) calloc(part_len + 1, sizeof(char));
        strncpy(parts[i], piece_start, part_len);
        piece_start = slash + 1;
        i++;
    }
    // Get the last piece.
    parts[i] = (char *) calloc(strlen(piece_start) + 1, sizeof(char));
    strncpy(parts[i], piece_start, strlen(piece_start));

    /* manual add NULL at end of parts. */
    i++;
    parts[i] = NULL;
    return parts;
}


// Convenience function to get the inode of the root directory.
struct ext2_inode * get_root_dir(void * fs) {
    return get_inode(fs, EXT2_ROOT_INO);
}


// Given the inode for a directory and a filename, return the inode number of
// that file inside that directory, or 0 if it doesn't exist there.
// 
// name should be a single component: "foo.txt", not "/files/foo.txt".
__u32 get_inode_from_dir(void * fs, struct ext2_inode * dir, 
        char * name) {
    // FIXME: Uses reference implementation.
    struct ext2_dir_entry_2 *dir_entry = NULL;
    int name_length;
    void *curr_blk, *curr_ptr, *end;
    int i=0;

    name_length = strlen(name);
    for(i=0; i<15; i++) {
      curr_blk = get_block(fs, dir->i_block[i]);
      end = (char*)curr_blk + get_block_size(fs);
      curr_ptr = curr_blk;

      while(curr_ptr != end) {
        dir_entry = (struct ext2_dir_entry_2*)curr_ptr;

        if(dir_entry->name_len == name_length && strncmp(dir_entry->name, name, name_length) == 0){
          return dir_entry->inode;
        }
          
        curr_ptr = (char*)curr_ptr + dir_entry->rec_len;
      }
        
    }


    return 0;
}


// Find the inode number for a file by its full path.
// This is the functionality that ext2cat ultimately needs.
__u32 get_inode_by_path(void * fs, char * path) {
    // FIXME: Uses reference implementation.
    struct ext2_inode *root_node = get_root_dir(fs);
    struct ext2_inode *parent_node;
    int index_i;
    int i=0;
    char **parts = split_path(path);

    parent_node = root_node;
    while(parts[i]) {
      index_i = get_inode_from_dir(fs, parent_node, parts[i]);

      if(index_i != 0) {
        parent_node = get_inode(fs, index_i);
        i++;
      }else {
        break;
      }

        
    }

    return index_i;
}

