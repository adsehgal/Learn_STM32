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
extern lfs_dir_t dir;
lfs_dir_t dir;
const char *rootDir = "/";

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

	// mount the filesystem
	int err = lfs_mount(&lfs, &cfg);

	// reformat if we can't mount the filesystem
	// this should only happen on the first boot
	if (err) {
		printf("Could not mount, formatting and mounting again\n");
		lfs_format(&lfs, &cfg);
		err = lfs_mount(&lfs, &cfg);
	}

	if (err) {
		printf("Format and mount failed!\n");
		printf("Need a HW reset!\n\n");
		while (1)
			;
	}

	err = lfs_mkdir(&lfs, rootDir);
	err = lfs_dir_open(&lfs, &dir, rootDir);
	if (err) {
		printf("Failed to open directory\n");
		err = lfs_mkdir(&lfs, rootDir);
		err = lfs_dir_open(&lfs, &dir, rootDir);
		if (err) {
			printf(
					"Failed to create directory, formatting and mounting again\n");
			lfs_format(&lfs, &cfg);
			err = lfs_mount(&lfs, &cfg);
			err = lfs_mkdir(&lfs, rootDir);
			err = lfs_dir_open(&lfs, &dir, rootDir);
			if (err) {
				printf("Failed to open root directory\n");
				while (1)
					;
			}
		}
	}

	// read current count
	uint32_t bootCount = 0;
	lfs_file_open(&lfs, &file, "bootCount", LFS_O_RDWR | LFS_O_CREAT);
	lfs_file_read(&lfs, &file, &bootCount, sizeof(bootCount));

	// update boot count
	bootCount += 1;
	lfs_file_rewind(&lfs, &file);
	lfs_file_write(&lfs, &file, &bootCount, sizeof(bootCount));

	// remember the storage is not updated until the file is closed successfully
	lfs_file_close(&lfs, &file);

	// print the boot count
	printf("Boot Count: %ld\n", bootCount);

}

int lfsReadFile(char *name, char *buff) {
//	lfsConfig(&cfg);
//	lfs_dir_open(&lfs, &dir, LFS_ROOT_PATH);

	int err = lfs_file_open(&lfs, &file, name, LFS_O_RDWR);
	if (err == LFS_ERR_NOENT) {
		printf("Non-existent file name, try with a valid file\n");
		return 0;
	}
	lfs_soff_t size = lfs_file_size(&lfs, &file);

	char fileContents[size];

	lfs_file_read(&lfs, &file, fileContents, size);

	lfs_file_close(&lfs, &file);

//	printf("Read [%ld]: %s\n", size, fileContents);

	memcpy(buff, fileContents, size);

	return size;
}

void lfsEraseDevice(void) {
//	lfsConfig(&cfg);
	lfs_format(&lfs, &cfg);
}

int lfsLs(void) {
	struct lfs_info info;
	while (1) {
		int res = lfs_dir_read(&lfs, &dir, &info);
		if (res < 0) {
			printf("Failed to read root\n");
			while (1)
				;
		}

		if (res == 0) {
//			End of dir, break out of read loop
			break;
		}

//		Print file type
		switch (info.type) {
		case LFS_TYPE_REG:
			printf("File ");
			break;
		case LFS_TYPE_DIR:
//			Dont list directories since everything will be in root
//			printf("dir ");
			break;
		default:
			printf("UNKWN ");
			break;
		}

//		Print file size
		static const char *prefixes[] = { "", "K", "M", "G" };
		for (int i = sizeof(prefixes) / sizeof(prefixes[0]) - 1; i >= 0; i--) {
			if (info.size >= (1 << 10 * i) - 1) {
				if (info.type != LFS_TYPE_DIR) {
					printf("%*lu%sB ", 4 - (i != 0), info.size >> 10 * i,
							prefixes[i]);
					break;
				}
			}
		}

//		Print file name
		if (info.type != LFS_TYPE_DIR) {
			printf("%s\n", info.name);
		}
	}
	printf("\n");

//	Rewind dir for future dir reads
	lfs_dir_rewind(&lfs, &dir);


	return 0;
}
