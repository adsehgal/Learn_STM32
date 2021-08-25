/*
 * lfs_interfface.h
 *
 *  Created on: Aug 10, 2021
 *      Author: adityasehgal
 */

#ifndef LFS_LFS_INTERFACE_H_
#define LFS_LFS_INTERFACE_H_

#include "lfs.h"

#define LFS_ROOT_PATH "root"
#define LFS_LIST_FILES_PATH "files"

/**
 * User provided Read, Write, Erase and Sync functions for LFS to use
 */
// Read a region in a block. Negative error codes are propogated
// to the user.
int lfsRead(const struct lfs_config *c, lfs_block_t block, lfs_off_t off,
		void *buffer, lfs_size_t size);

// Program a region in a block. The block must have previously
// been erased. Negative error codes are propogated to the user.
// May return LFS_ERR_CORRUPT if the block should be considered bad.
int lfsProg(const struct lfs_config *c, lfs_block_t block, lfs_off_t off,
		const void *buffer, lfs_size_t size);

// Erase a block. A block must be erased before being programmed.
// The state of an erased block is undefined. Negative error codes
// are propogated to the user.
// May return LFS_ERR_CORRUPT if the block should be considered bad.
int lfsErase(const struct lfs_config *c, lfs_block_t block);

// Sync the state of the underlying block device. Negative error codes
// are propogated to the user.
int lfsSync(const struct lfs_config *c);
/**
 *
 */

/***********************************************************************
 ***********************************************************************
 **********************************************************************/

/**
 * Wrapper for LFS
 */

void readBoot(void);

void lfsConfig(struct lfs_config *c);

int lfsReadFile(char *name, char *buff);

void lfsEraseDevice(void);

int lfsLs(void);

/***********************************************************************
 ***********************************************************************
 **********************************************************************/

#endif /* LFS_LFS_INTERFACE_H_ */
