/*
 * lfs_interface.h
 *
 *  Created on: Oct 11, 2021
 *      Author: root
 */

#ifndef INC_LFS_INTERFACE_H_
#define INC_LFS_INTERFACE_H_

#include "lfs.h"

#define SPIFS_DBG 1

#if SPIFS_DBG
#define SPIFS_WARN_YES 1
#define SPIFS_ERR_YES 1
#else
#define SPIFS_WARN_YES 0
#define SPIFS_WARN_YES 0
#endif

#if SPIFS_DBG

#if SPIFS_WARN_YES
#define SPIFS_WARN_(fmt, ...) \
    printf("%s:%d:warn: " fmt "%s\n", __FILE__, __LINE__, __VA_ARGS__)

#define SPIFS_WARN(...) SPIFS_WARN_(__VA_ARGS__, "")

#else
#define SPIFS_WARN(...)
#endif	// SPIFS_WARN

#if SPIFS_ERR_YES
#define SPIFS_ERR_(fmt, ...) \
    printf("%s:%d:error: " fmt "%s\n", __FILE__, __LINE__, __VA_ARGS__)

#define SPIFS_ERR(...) SPIFS_ERR_(__VA_ARGS__, "")

#else
#define SPIFS_ERR_(...)
#endif	// SPIFS_ERR

#endif	// SPIFS_DBG

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

void lfsConfig(struct lfs_config *c);

void lfsCreateFile(char *name);

uint32_t lfsGetFileSize(char *name);

int lfsReadFile(char *name, uint8_t *buff);

void lfsEraseDevice(void);

int lfsLs(void);

void lfsRemoveFile(char *name);

void lfsRenameFile(char *old, char *new);

void lfsOpenFileForWrite(char *name);

int lfsWriteFile(uint8_t *pBuff, uint32_t size);

void lfsCloseFileForWrite(void);

/***********************************************************************
 ***********************************************************************
 **********************************************************************/

#endif /* INC_LFS_INTERFACE_H_ */
