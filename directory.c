#define DIR_NAME_LENGTH 48

#include <sys/stat.h>
#include <sys/types.h>
#include "directory.h"
#include "string.h"

// adds a directory entry
int directory_put(inode_t *dd, const char *name, int inum) {
	dirent_t new_entry;
	strcpy(new_entry.name, name);
	new_entry.inum = inum;
	dirent_t *new_pointer = (dirent_t*)dd->block + dd->size;
	*new_pointer = new_entry;
	dd->size += 1;
	return inum;
}

// initializes the directory upon first setup
void directory_init() {
	// allocate first inode for root
	alloc_inode();
	// get the inode for /
	inode_t *root_inode = get_inode(0);
	root_inode->mode = 040755;
	// allocate a block for /
	root_inode->block = blocks_get_block(alloc_block());
	void *root_block = root_inode->block;
	// put a self-referential dirent into root
	const char *name = "/";
	directory_put(root_inode, name, 0);
}

// returns the index of the dirent with the passed in name if it exists
// in the passed in inode's directory
int directory_lookup(inode_t *dd, const char *name) {
	for (int i = 0; i < dd->size; i++) {
		dirent_t *dir = (dirent_t*)dd->block + i;
	  if (strcmp(name, dir->name) == 0) {
			return i;
	  }
  }
	return -1;
}

//deletes a directory from our filesystem
int directory_delete(inode_t *dd, const char *name) {
	for (int i = 0; i < dd->size; i++) {
		dirent_t *dir = (dirent_t*)dd->block + i;
		if (strcmp(name, dir->name) == 0) {
			strcpy(dir->name, "");
			return 0;
		}
	}
	return -1;	
}

slist_t *directory_list(const char *path);


//prints the contents of the given directory
void print_directory(inode_t *dd) {
	dirent_t *dir_block = dd->block;
	for (int i = 0; i < dd->size; i++) {
		dirent_t *curr_dir = dir_block + i;
	  printf("%s\n", curr_dir->name);
	}
}