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

lfs_t lfs;
lfs_file_t file;
struct lfs_config cfg;
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
	c->read_size = 16;		//read 16 bytes at a time
	c->prog_size = 16;		//write 16 bytes at a time
	c->block_size = 4096;	//erase 4KB at a time
	c->block_count = 512;	//512 erasable blocks
	c->cache_size = 16;
	c->lookahead_size = 16;
	c->block_cycles = 500;

//	block device limits
//	c->name_max = 32;		//max 32byte long names

	int err = lfs_mount(&lfs, c);
//	lfs_format(&lfs, c);
	if (err != LFS_ERR_OK) {
		lfs_format(&lfs, c);
		lfs_mount(&lfs, c);
	}

//	read current count
	char bootCount[8] = {0};
	lfs_file_open(&lfs, &file, "bootCount.s", LFS_O_RDWR | LFS_O_CREAT);
	lfs_file_read(&lfs, &file, bootCount, sizeof(bootCount));

	// update boot count
	uint16_t count = atoi(bootCount);
	count++;
	sprintf(bootCount, "%d", count);
	lfs_file_rewind(&lfs, &file);
	lfs_file_write(&lfs, &file, bootCount, sizeof(bootCount));
	printf("boot_count = %s\n", bootCount);

//	remember the storage is not updated until the file is closed successfully
	lfs_file_close(&lfs, &file);

	//	read current count
//	char boot_count = 0;
//	lfs_file_open(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
//	lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));
//
//	// update boot count
//	boot_count += 1;
//	lfs_file_rewind(&lfs, &file);
//	lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));
//	printf("boot_count = %d\n", boot_count);
//
//	//	remember the storage is not updated until the file is closed successfully
//	lfs_file_close(&lfs, &file);

//	release any resources we were using
//	lfs_unmount(&lfs);
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
	printf("Read: %d\n", fileContents[0]);

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
