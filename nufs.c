#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>
#include "blocks.h"
#include "inode.h"
#include "directory.h"
#include "bitmap.h"
#include "storage.h"

// implementation for: man 2 access
// Checks if a file exists.
int nufs_access(const char *path, int mask) {
  // Only the root directory and our simulated file are accessible for now...
  int rv = directory_lookup(get_inode(0), path);

  printf("access(%s, %04o) -> %d\n", path, mask, rv);
  if (rv != -1) {
	  return 0;
  }

  return -ENOENT;
}

// Gets an object's attributes (type, permissions, size, etc).
// Implementation for: man 2 stat
// This is a crucial function.
int nufs_getattr(const char *path, struct stat *st) {
  // Return some metadata for the root directory...
   int direct_index = directory_lookup(get_inode(0), path);
   if (direct_index != -1) {
     dirent_t *entry = (dirent_t*)blocks_get_block(2) + direct_index;
     int inum = entry->inum;
     inode_t *entry_inode = get_inode(inum);
     st->st_mode = entry_inode->mode;
     st->st_size = entry_inode->size;
     printf("File: %s\tMode: %d\tSize: %ld\n", path, st->st_mode, st->st_size);
     return 0;
   }
  printf("No object of name: %s exists.\n", path);
  return -ENOENT;
}

// implementation for: man 2 readdir
// lists the contents of a directory
int nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                 off_t offset, struct fuse_file_info *fi) {
  int rv = 0;
  printf("Contents of %s:\n", path);
  print_directory(get_inode(0));
  return rv;
}

// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
int nufs_mknod(const char *path, mode_t mode, dev_t rdev) {
  int rv = -1;
  rv = storage_mknod(path, mode);
  printf("mknod(%s, %04o) -> %d\n", path, mode, rv);
  return rv; 
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
int nufs_mkdir(const char *path, mode_t mode) {
  int rv = nufs_mknod(path, mode | 040000, 0);
  printf("mkdir(%s) -> %d\n", path, rv);
  return rv;
}

// unlinks the passed in node from its parent
int nufs_unlink(const char *path) {
	int rv = -1;
  printf("unlink(%s) -> %d\n", path, rv);
	// Calling unlink from storage
  rv = storage_unlink(path);
  return rv;
}

// creates a new link to an existing file
// effectively makes a new name for a file
// *unimplemented*
int nufs_link(const char *from, const char *to) {
  int rv = -1;
  printf("link(%s => %s) -> %d\n", from, to, rv);
  return rv;
}

// removes the passed in directory if it is an empty directory
// *unimplemented*
int nufs_rmdir(const char *path) {
  int rv = -1;
  printf("rmdir(%s) -> %d\n", path, rv);
  return rv;
}

// implements: man 2 rename
// called to move a file within the same filesystem
int nufs_rename(const char *from, const char *to) {
  int rv = -1;
  rv = storage_rename(from, to);
	printf("rename(%s => %s) -> %d\n", from, to, rv);
}

// changes the mode of the file at the given path
// *unimplemented*
int nufs_chmod(const char *path, mode_t mode) {
  int rv = -1;
  printf("chmod(%s, %04o) -> %d\n", path, mode, rv);
  return rv;
}

// truncates the file to the passed in size
// *unimplemented*
int nufs_truncate(const char *path, off_t size) {
  int rv = -1;
  printf("truncate(%s, %ld bytes) -> %d\n", path, size, rv);
  return rv;
}

// This is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
// You can just check whether the file is accessible.
int nufs_open(const char *path, struct fuse_file_info *fi) {
  int rv = 0;
  rv = nufs_access(path, 0);
  printf("open(%s) -> %d\n", path, rv);
  return rv;
}

// Actually read data
int nufs_read(const char *path, char *buf, size_t size, off_t offset,
              struct fuse_file_info *fi) {
	int rv = -1;
	rv = storage_truncate(buf, path, size);
  printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
  return rv;
}

// Actually write data
int nufs_write(const char *path, const char *buf, size_t size, off_t offset,
               struct fuse_file_info *fi) {
  int rv = -1;
	rv = storage_write(path, buf, size, offset);
  printf("write(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
  return rv;
}

// Update the timestamps on a file or directory.
// *unimplemented
int nufs_utimens(const char *path, const struct timespec ts[2]) {
  return 0;
}

// Extended operations
// *unimplemented*
int nufs_ioctl(const char *path, int cmd, void *arg, struct fuse_file_info *fi,
               unsigned int flags, void *data) {
  int rv = -1;
  printf("ioctl(%s, %d, ...) -> %d\n", path, cmd, rv);
  return rv;
}

// Initializes nufs operations to be called when their corresponding system calls are made
void nufs_init_ops(struct fuse_operations *ops) {
  memset(ops, 0, sizeof(struct fuse_operations));
  ops->access = nufs_access;
  ops->getattr = nufs_getattr;
  ops->readdir = nufs_readdir;
  ops->mknod = nufs_mknod;
  // ops->create   = nufs_create; // alternative to mknod
  ops->mkdir = nufs_mkdir;
  ops->link = nufs_link;
  ops->unlink = nufs_unlink;
  ops->rmdir = nufs_rmdir;
  ops->rename = nufs_rename;
  ops->chmod = nufs_chmod;
  ops->truncate = nufs_truncate;
  ops->open = nufs_open;
  ops->read = nufs_read;
  ops->write = nufs_write;
  ops->utimens = nufs_utimens;
  ops->ioctl = nufs_ioctl;
};

struct fuse_operations nufs_ops;

// Main execution:
int main(int argc, char *argv[]) {
  assert(argc > 2 && argc < 6);
  blocks_init(argv[--argc]);
  nufs_init_ops(&nufs_ops);
  return fuse_main(argc, argv, &nufs_ops, NULL);
}
