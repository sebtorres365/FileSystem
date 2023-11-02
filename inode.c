#include <stdio.h>
#include "bitmap.h"
#include "inode.h"



//Prints all of the inode components
void print_inode(inode_t *node) {
	printf("Reference Count: %d\n", node->refs);
	printf("Permission and Type: %d\n", node->mode);
	printf("Size: %d\n", node->size);
	printf("Block: %p\n", node->block);
}

//Takes an inode number and gets the beginning of the inode_bitmap 
//Then returns the inode requested
inode_t *get_inode(int inum) {
	void *ibm = get_inode_bitmap();
	inode_t *table_block = blocks_get_block(1);
	return table_block + inum;
}

//allocates a new inode
int alloc_inode() {
	void *ibm = get_inode_bitmap();
	for (int i = 1; i < 256; i++) {
		if (!bitmap_get(ibm, i)) {
			bitmap_put(ibm, i, 1);
			return i;
		}
	}
	return -1;
}

//frees a given node
void free_inode(int inum) {
	void *ibm = get_inode_bitmap();
	bitmap_put(ibm, inum, 0);
}

//grows the size component of a given inode
int grow_inode(inode_t *node, int size) {
	if (node->size + size <= 4096) {
		node->size += size;
		return 0;
	}
	printf("too big\n");
	return -1;
}

//shrinks the size of the node by the given parameter size
int shrink_inode(inode_t *node, int size) {
	if (node->size >= size) {
		node->size -= size;
		return 0;
	}
	printf("too small\n");
	return -1;
}

int inode_get_pnum(inode_t *node, int fpn);