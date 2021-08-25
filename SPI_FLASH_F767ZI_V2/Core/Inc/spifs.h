/*
 * spifs.h
 *
 *  Created on: 15 Aug 2021
 *      Author: adityasehgal
 */

#ifndef INC_SPIFS_H_
#define INC_SPIFS_H_

#include <stdint.h>

#define SPIFS_FILENAME_LEN      16

// SPI file structure.
typedef struct _spifs_file {
	uint8_t write;
	int16_t file_id;
	int16_t block_id;
	int16_t block_size;
	int16_t block_index;
	int16_t this_id;
	int16_t next_id;
	uint32_t position;
} spifs_file_t;

// SPI directory structure.
typedef struct _spifs_dir {
	int16_t file_id;
	uint32_t file_size;
	char filename[SPIFS_FILENAME_LEN];
} spifs_dir_t;

uint8_t spifs_format(void);
uint8_t spifs_flash_size(uint32_t *flash_size);
uint8_t spifs_delete(const char *filename);
uint8_t spifs_filesize(const char *filename, uint32_t *filesize);
uint8_t spifs_checksum(const char *filename, uint32_t *checksum);
uint8_t spifs_open(const char *filename, uint8_t write, spifs_file_t *file);
uint8_t spifs_close(spifs_file_t *file);
uint32_t spifs_read(spifs_file_t *file, uint8_t *buffer, uint32_t count);
uint32_t spifs_write(spifs_file_t *file, const uint8_t *buffer, uint32_t count);
uint8_t spifs_seek(spifs_file_t *file, uint32_t position);
uint8_t spifs_open_dir(spifs_dir_t *dir);
uint8_t spifs_close_dir(spifs_dir_t *dir);
uint8_t spifs_read_dir(spifs_dir_t *dir);

#endif /* INC_SPIFS_H_ */
