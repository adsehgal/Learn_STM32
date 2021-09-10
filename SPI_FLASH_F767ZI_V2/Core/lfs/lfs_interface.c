/*
 * lfs_interface.c
 *
 *  Created on: Aug 10, 2021
 *      Author: adityasehgal
 */

#include "main.h"
#include "flash.h"
#include "lfs_interface.h"
#include "lfs.h"

extern lfs_t lfs;
extern lfs_file_t file;
extern struct lfs_config cfg;
lfs_dir_t dir;

int lfsRead(const struct lfs_config *c, lfs_block_t block, lfs_off_t off,
		void *buffer, lfs_size_t size) {

	uint32_t addr = block * c->block_size + off;
	flashReadBytes((uint8_t*) buffer, addr, size);
	return 0;
}

int lfsProg(const struct lfs_config *c, lfs_block_t block, lfs_off_t off,
		const void *buffer, lfs_size_t size) {

	uint32_t addr = block * c->block_size + off;
	flashWriteBytes((uint8_t*) buffer, addr, size);
	return 0;
}

int lfsErase(const struct lfs_config *c, lfs_block_t block) {

//	block here is just an index, using the sector erase function,
//	you want to actually provide the absolute address from which
//	to erase from... therefore address = block*block_size
	flashEraseSector(block * c->block_size);
	return 0;
}

int lfsSync(const struct lfs_config *c) {
	return 0;
}

/***********************************************************************
 ***********************************************************************
 **********************************************************************/

void lfsConfig(struct lfs_config *c) {
//	 block device operations - user provided
	c->read = lfsRead;
	c->prog = lfsProg;
	c->erase = lfsErase;
	c->sync = lfsSync;

//	 block device configuration
	c->read_size = 256;	//
	c->prog_size = 256;	//
	c->block_size = 4096;	//
	c->block_count = 512;	//
	c->cache_size = 256;	//
	c->lookahead_size = 16;	//
	c->block_cycles = 500;	//

//	block device limits
//	c->name_max = 32;		//max 32byte long names

	// mount the filesystem
	int err = lfs_mount(&lfs, &cfg);

	// reformat if we can't mount the filesystem
	// this should only happen on the first boot
	if (err) {
		lfs_format(&lfs, &cfg);
		err = lfs_mount(&lfs, &cfg);
	}

	if (err) {
		printf("Format and mount failed!\n");
		printf("Need a HW reset!\n\n");
		while (1)
			;
	}

	// read current count
	uint32_t boot_count = 0;
	lfs_file_open(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
	lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));

	// update boot count
	boot_count += 1;
	lfs_file_rewind(&lfs, &file);
	lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));

	// remember the storage is not updated until the file is closed successfully
	lfs_file_close(&lfs, &file);

	// release any resources we were using
//	lfs_unmount(&lfs);

	// print the boot count
	printf("boot_count: %ld\n", boot_count);

}

int lfsReadFile(char *name, char *buff) {
//	lfsConfig(&cfg);
//	lfs_dir_open(&lfs, &dir, LFS_ROOT_PATH);

	lfs_file_open(&lfs, &file, name, LFS_O_RDWR | LFS_O_CREAT);
	lfs_soff_t size = lfs_file_size(&lfs, &file);

	char fileContents[size];

	lfs_file_read(&lfs, &file, fileContents, size);

	lfs_file_close(&lfs, &file);

//	lfs_unmount(&lfs);
	printf("Read [%d]: %s\n", size, fileContents);

	memcpy(buff, fileContents, size);

	return size;
}

void lfsEraseDevice(void) {
//	lfsConfig(&cfg);
	lfs_format(&lfs, &cfg);
}

int lfsLs(void) {
	return 0;
}
