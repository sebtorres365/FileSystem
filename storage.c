//Implentation for all storage functions:

#include "storage.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "slist.h"
#include "directory.h"

// given a size and file location it writes the data to the specified offset and updates the inode
int storage_write(const char *path, const char *buf, size_t size, off_t offset) {
	int dirent_index = directory_lookup(get_inode(0), path);
  dirent_t *entry = (dirent_t*)blocks_get_block(2) + dirent_index;
  int inum = entry->inum;
  inode_t *entry_inode = get_inode(inum);
  char *block_start = (char*)entry_inode->block + offset;
  strncpy(block_start, buf, size);
  entry_inode->size += size;
  printf("Wrote %ld bytes to %s.\n", size, path);
  return size;
}

// reads the first size number of bytes from the files data block
int storage_truncate(char *buf, const char *path, off_t size) {
	int direct_index = directory_lookup(get_inode(0), path);
  dirent_t *entry = (dirent_t*)blocks_get_block(2) + direct_index;
  int inum = entry->inum;
  inode_t *entry_inode = get_inode(inum);
  char* block_begin = (char*)entry_inode->block;
  strncpy(buf, block_begin, size); 
  printf("Read %ld bytes from %s.\n", size, path);
  return size;
}

// Creates a new file specified at the path given
// (file is created with the passed in mode)
int storage_mknod(const char *path, int mode) {
	int inode_index = alloc_inode();
  inode_t *new_inode = get_inode(inode_index);
  new_inode->size = 0;
  new_inode->mode = mode;
  new_inode->block = blocks_get_block(alloc_block());
  if (directory_put(get_inode(0), path, inode_index)) {
	  return 0;
  }
  return -ENOENT; 
}

// Deletes the given directory
int storage_unlink(const char *path) {
	directory_delete(get_inode(0), path);
}


int storage_link(const char *from, const char *to);

// rename directory from the 'from' to 'to'
int storage_rename(const char *from, const char *to) {
	int dirent_index = directory_lookup(get_inode(0), from);
  dirent_t *dir_from = blocks_get_block(2) + dirent_index;
  strcpy(dir_from->name, to);
  return 0;
}


int storage_set_time(const char *path, const struct timespec ts[2]);
slist_t *storage_list(const char *path);