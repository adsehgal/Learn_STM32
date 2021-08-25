/*
 * spifs.c
 *
 *  Created on: 15 Aug 2021
 *      Author: adityasehgal
 */

#include <string.h>
#include <ctype.h>

#include "main.h"

#include "flash.h"
#include "spifs.h"

extern RNG_HandleTypeDef hrng;
extern CRC_HandleTypeDef hcrc;

#define SPIFS_SECTOR_FIRST      1       // First sector (others are reserved).
#define SPIFS_SECTOR_LAST       511     // Last sector.
#define SPIFS_SECTOR_SIZE       4096    // Size of sector in bytes.

#define SPIFS_HEADER_SIZE       8       // Size of the header portion of a file block.
#define SPIFS_BLOCK_SIZE        4088    // Size of the data portion of the file block.

#define SPIFS_FILEID_OFFSET     0       // Offset of file id in a file block.
#define SPIFS_BLOCKID_OFFSET    2       // Offset of block id in a file block.
#define SPIFS_BLOCKSIZE_OFFSET  4       // Offset of block size in a file block.
#define SPIFS_NEXTID_OFFSET     6       // Offset of next block id in a file block.
#define SPIFS_FILENAME_OFFSET   8       // Offset of filename in a file block.

// Returns true if sectorid is valid.
#define SPIFS_SECTORID_VALID(sid) \
    ((sid >= SPIFS_SECTOR_FIRST) && (sid <= SPIFS_SECTOR_LAST))

// Returns true if fileid is valid.
#define SPIFS_FILEID_VALID(fid) \
    ((fid >= SPIFS_SECTOR_FIRST) && (fid <= SPIFS_SECTOR_LAST))

// Returns true if block size is valid.
#define SPIFS_BLOCKSIZE_VALID(bs) \
    ((bs > 0) && (bs <= SPIFS_BLOCK_SIZE))

// Converts a sector id to a byte offset in flash.
#define SPIFS_SECTORID_TO_BYTEOFFSET(id) \
    (((uint32_t) id) << 12)

// SPI block header structure.
typedef struct _spifs_file_block {
	int16_t file_id;
	int16_t block_id;
	int16_t block_size;
	int16_t next_id;
} spifs_file_block_t;

// 32-bit checksum CRC table.
static const uint32_t crc_table[] = { 0x00000000, 0x04c11db7, 0x09823b6e,
		0x0d4326d9, 0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005, 0x2608edb8,
		0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61, 0x350c9b64, 0x31cd86d3, 0x3c8ea00a,
		0x384fbdbd, 0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9, 0x5f15adac,
		0x5bd4b01b, 0x569796c2, 0x52568b75, 0x6a1936c8, 0x6ed82b7f, 0x639b0da6,
		0x675a1011, 0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd, 0x9823b6e0,
		0x9ce2ab57, 0x91a18d8e, 0x95609039, 0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52,
		0x8664e6e5, 0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81, 0xad2f2d84,
		0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d, 0xd4326d90, 0xd0f37027, 0xddb056fe,
		0xd9714b49, 0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95, 0xf23a8028,
		0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1, 0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a,
		0xec7dd02d, 0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae, 0x278206ab,
		0x23431b1c, 0x2e003dc5, 0x2ac12072, 0x128e9dcf, 0x164f8078, 0x1b0ca6a1,
		0x1fcdbb16, 0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca, 0x7897ab07,
		0x7c56b6b0, 0x71159069, 0x75d48dde, 0x6b93dddb, 0x6f52c06c, 0x6211e6b5,
		0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066, 0x4d9b3063,
		0x495a2dd4, 0x44190b0d, 0x40d816ba, 0xaca5c697, 0xa864db20, 0xa527fdf9,
		0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692, 0x8aad2b2f,
		0x8e6c3698, 0x832f1041, 0x87ee0df6, 0x99a95df3, 0x9d684044, 0x902b669d,
		0x94ea7b2a, 0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e, 0xf3b06b3b,
		0xf771768c, 0xfa325055, 0xfef34de2, 0xc6bcf05f, 0xc27dede8, 0xcf3ecb31,
		0xcbffd686, 0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a, 0x690ce0ee,
		0x6dcdfd59, 0x608edb80, 0x644fc637, 0x7a089632, 0x7ec98b85, 0x738aad5c,
		0x774bb0eb, 0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f, 0x5c007b8a,
		0x58c1663d, 0x558240e4, 0x51435d53, 0x251d3b9e, 0x21dc2629, 0x2c9f00f0,
		0x285e1d47, 0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b, 0x0315d626,
		0x07d4cb91, 0x0a97ed48, 0x0e56f0ff, 0x1011a0fa, 0x14d0bd4d, 0x19939b94,
		0x1d528623, 0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2,
		0xe6ea3d65, 0xeba91bbc, 0xef68060b, 0xd727bbb6, 0xd3e6a601, 0xdea580d8,
		0xda649d6f, 0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3, 0xbd3e8d7e,
		0xb9ff90c9, 0xb4bcb610, 0xb07daba7, 0xae3afba2, 0xaafbe615, 0xa7b8c0cc,
		0xa379dd7b, 0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f, 0x8832161a,
		0x8cf30bad, 0x81b02d74, 0x857130c3, 0x5d8a9099, 0x594b8d2e, 0x5408abf7,
		0x50c9b640, 0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c, 0x7b827d21,
		0x7f436096, 0x7200464f, 0x76c15bf8, 0x68860bfd, 0x6c47164a, 0x61043093,
		0x65c52d24, 0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30, 0x029f3d35,
		0x065e2082, 0x0b1d065b, 0x0fdc1bec, 0x3793a651, 0x3352bbe6, 0x3e119d3f,
		0x3ad08088, 0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654, 0xc5a92679,
		0xc1683bce, 0xcc2b1d17, 0xc8ea00a0, 0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb,
		0xdbee767c, 0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18, 0xf0a5bd1d,
		0xf464a0aa, 0xf9278673, 0xfde69bc4, 0x89b8fd09, 0x8d79e0be, 0x803ac667,
		0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c, 0xafb010b1,
		0xab710d06, 0xa6322bdf, 0xa2f33668, 0xbcb4666d, 0xb8757bda, 0xb5365d03,
		0xb1f740b4 };

// Cleans the filename of problem characters.
static int clean_filename(const char *src, char *dst) {
	char *p;
	char *e;

	// Point to the two end of the buffer.
	p = dst;
	e = dst + (SPIFS_FILENAME_LEN - 1);

	// Skip leading space.
	while (*src && isspace(*(uint8_t* )src))
		src++;

	// Copy the source into the end ignoring special characters.
	while (*src && (p < e)) {
		// Filter out special ASCII characters: '\/:*?"<>|
		switch (*src) {
		case '\'':
		case '\\':
		case '/':
		case ':':
		case '*':
		case '?':
		case '"':
		case '<':
		case '>':
		case '|':
			// Increment the source.
			src++;
			break;
		default:
			*p++ = *src++;
			break;
		}
	}

	// Back up p.
	p--;

	// Trim trailing space, but don't go too far!
	while ((p > dst) && isspace(*(uint8_t* )p))
		p--;

	// p currently points at the last non-space, skip and terminate.
	*++p = '\0';

	// Return the resulting string length.
	return strlen(dst);
}

// Returns the next sector after the current sector, wrapping as needed.
static int16_t spifs_sector_increment(int16_t sector_id) {
	// Increment the sector.
	++sector_id;

	// Wrap around to the first sector if we wrapped.
	if (sector_id > SPIFS_SECTOR_LAST)
		sector_id = SPIFS_SECTOR_LAST;

	return sector_id;
}

// Returns the file id associated with the filename or -1;
static int16_t spifs_find_filename(char *filename) {
	int16_t sector_id;
	uint32_t sector_offset;
	spifs_file_block_t file_block;
	char sectorFilename[SPIFS_FILENAME_LEN];

	// Loop over each sector to check for the file.
	for (sector_id = SPIFS_SECTOR_FIRST; sector_id <= SPIFS_SECTOR_LAST;
			++sector_id) {
		// Get the byte offset of the sector.
		sector_offset = SPIFS_SECTORID_TO_BYTEOFFSET(sector_id);

		// Read the fileblock at the start of the sector.
//		spiflash_read(sector_offset, (uint8_t*) &file_block,
//				sizeof(file_block));
		flashReadBytes((uint8_t*) &file_block, sector_offset,
				sizeof(file_block));

		// Is this the first block in a file?
		if (SPIFS_FILEID_VALID(file_block.file_id)
				&& (file_block.block_id == 0)) {
			// Set the sector offset to the file name.
			sector_offset += sizeof(file_block);

			// Read the filename.
//			spiflash_read(sector_offset, (uint8_t*) sectorFilename,
//					SPIFS_FILENAME_LEN);
			flashReadBytes((uint8_t*) &sectorFilename, sector_offset,
			SPIFS_FILENAME_LEN);

			// If the filename matches, return the file id.
			if (!strncmp(filename, sectorFilename, SPIFS_FILENAME_LEN))
				return file_block.file_id;
		}
	}

	return -1;
}

// Returns a random free sector to use as a file block.
static int16_t spifs_free_block(void) {
	bool found;
	int16_t random_id;
	int16_t sector_id;
	uint32_t sector_offset;
	spifs_file_block_t file_block;

	// Get a random sector id.
//	random_id = (random_get() % (SPIFS_SECTOR_LAST - SPIFS_SECTOR_FIRST + 1))
//			+ SPIFS_SECTOR_FIRST;

	random_id =
			(HAL_RNG_GetRandomNumber(&hrng)
					% (SPIFS_SECTOR_LAST - SPIFS_SECTOR_FIRST + 1))
					+ SPIFS_SECTOR_FIRST;

	// We start searching from the random_id.
	sector_id = random_id;

	// We initially haven't yet found a free sector.
	found = false;

	// Loop over each sector looking for a free sector.
	do {
		// Get the byte offset of the sector.
		sector_offset = SPIFS_SECTORID_TO_BYTEOFFSET(sector_id);

		// Read the fileblock at the start of the sector.
//		spiflash_read(sector_offset, (uint8_t*) &file_block,
//				sizeof(file_block));
		flashReadBytes((uint8_t*) &file_block, sector_offset,
				sizeof(file_block));

		// Is this a free sector?  This is indicated with an invalid fileid.
		if (!SPIFS_FILEID_VALID(file_block.file_id)) {
			// Make sure the block appears erased.  This will hopefully
			// help reduce instances of a corrupt file system.
			if ((file_block.file_id != -1) || (file_block.block_id != -1)
					|| (file_block.block_size != -1)
					|| (file_block.next_id != -1)) {
				// Erase the sector.
//				spiflash_erase_4K(sector_id, 1);
				flashEraseSector(sector_id);
			}

			// We found a free sector.
			found = true;
		} else {
			// Increment the sector id.
			sector_id = spifs_sector_increment(sector_id);
		}
	} while (!found && (sector_id != random_id));

	// If not found, return -1 for the sector id of the block.
	if (!found)
		sector_id = -1;

	return sector_id;
}

// Erase each block associated with the file id.
static void spifs_erase_blocks(int16_t file_id) {
	int16_t sector_id;
	uint32_t sector_offset;
	spifs_file_block_t file_block;

	// The first sector id is the same as the file id.
	sector_id = file_id;

	// Loop over each sector in the file and erase the sector.
	while (SPIFS_SECTORID_VALID(sector_id)) {
		// Get the byte offset of the sector.
		sector_offset = SPIFS_SECTORID_TO_BYTEOFFSET(sector_id);

		// Read the fileblock at the start of the sector.
//		spiflash_read(sector_offset, (uint8_t*) &file_block,
//				sizeof(file_block));
		flashReadBytes((uint8_t*) &file_block, sector_offset,
				sizeof(file_block));

		// Erase the sector.
//		spiflash_erase_4K(sector_id, 1);
		flashEraseSector(sector_id);

		// Set the sector id to the next sector id.
		sector_id = file_block.next_id;
	}
}

// Determine the file size associated with the file id.
static uint32_t spifs_get_file_size(int16_t file_id) {
	int16_t sector_id;
	uint32_t file_size;
	uint32_t sector_offset;
	spifs_file_block_t file_block;

	// Initialize the filesize.
	file_size = 0;

	// The first sector id is the same as the file id.
	sector_id = file_id;

	// Loop over each sector in the file and add up the block sizes.
	while (SPIFS_SECTORID_VALID(sector_id)) {
		// Get the byte offset of the sector.
		sector_offset = SPIFS_SECTORID_TO_BYTEOFFSET(sector_id);

		// Read the fileblock at the start of the sector.
//		spiflash_read(sector_offset, (uint8_t*) &file_block,
//				sizeof(file_block));
		flashReadBytes((uint8_t*) &file_block, sector_offset,
				sizeof(file_block));

		// Add the block size to the file size.
		file_size += (uint32_t) file_block.block_size;

		// Set the sector id to the next sector id.
		sector_id = file_block.next_id;
	}

	// Subtract the filename.
	if (file_size >= SPIFS_FILENAME_LEN)
		file_size -= SPIFS_FILENAME_LEN;

	return file_size;
}

// Determine the file CRC checksum associated with the file id.
// Note: This checksum matches the 'cksum' command implemented in
// Ubuntu linux.  See 'man cksum' for a detailed description of the
// specific 32bit checksum algorithm used.
static uint32_t spifs_get_file_crc(int16_t file_id) {
	int16_t sector_id;
	uint16_t i;
	uint32_t file_crc;
	uint32_t file_size;
	uint32_t read_index;
	uint32_t read_count;
	uint32_t read_offset;
	spifs_file_block_t file_block;
	uint8_t buffer[64];

	// Initialize the checksum and size.
	file_crc = 0;
	file_size = 0;

	// The first sector id is the same as the file id.
	sector_id = file_id;

	// Loop over each sector in the file and add up the block sizes.
	while (SPIFS_SECTORID_VALID(sector_id)) {
		// Get the byte offset of the sector.
		read_offset = SPIFS_SECTORID_TO_BYTEOFFSET(sector_id);

		// Read the fileblock at the start of the sector.
//		spiflash_read(read_offset, (uint8_t*) &file_block, sizeof(file_block));
		flashReadBytes((uint8_t*) &file_block, read_offset, sizeof(file_block));

		// Is this the first block in a file?
		if (SPIFS_FILEID_VALID(file_block.file_id)
				&& (file_block.block_id == 0)) {
			// Don't checksum the filename in the block.
			read_index = SPIFS_FILENAME_LEN;

			// Add the block size to the file size.
			file_size +=
					(uint32_t) (file_block.block_size - SPIFS_FILENAME_LEN);
		} else {
			// Checksum the entire block.
			read_index = 0;

			// Add the block size to the file size.
			file_size += (uint32_t) file_block.block_size;
		}

		// Read the entire block.
		while (read_index < file_block.block_size) {
			// Get the number of bytes to read.
			read_count = file_block.block_size - read_index;

			// Do not exceed buffer size.
			if (read_count > sizeof(buffer))
				read_count = sizeof(buffer);

			// Get the byte offset within the current sector to read from.
			read_offset = SPIFS_SECTORID_TO_BYTEOFFSET(sector_id)
					+ SPIFS_HEADER_SIZE + read_index;

			// Read in the data from the block.
//			spiflash_read(read_offset, (uint8_t*) buffer, read_count);
			flashReadBytes((uint8_t*) &buffer, read_offset, sizeof(read_count));

			// Add the data to the checksum.
			for (i = 0; i < read_count; ++i) {
				// Use the CRC table to update the CRC for the indexed byte.
				file_crc = (file_crc << 8)
						^ crc_table[(file_crc >> 24) ^ ((uint32_t) buffer[i])];
			}

			// Add to the block index.
			read_index += read_count;
		}

		// Set the sector id to the next sector id.
		sector_id = file_block.next_id;
	}

	// Extend with the length of the file.
	while (file_size != 0) {
		file_crc = (file_crc << 8)
				^ crc_table[(file_crc >> 24) ^ (file_size & 0xff)];
		file_size >>= 8;
	}

	return ~file_crc;
}

uint8_t spifs_format(void) {
//	// Open the SPI flash device.
//	spiflash_open();
//
//	// Erase the chip.
//	spiflash_erase_chip();
//
//	// Close the SPI flash device.
//	spiflash_close();
	flashEraseChip();

	return 1;
}

uint8_t spifs_flash_size(uint32_t *flash_size) {
	UNUSED(flash_size);
//	uint8_t flash_flags;
//
//	// Open the SPI flash device.
//	spiflash_open();
//
//	// Get the chip information where the file system resides.
//	spiflash_get_chip_info(flash_size, &flash_flags);
//
//	// Close the SPI flash device.
//	spiflash_close();
//
	return 1;
}

uint8_t spifs_delete(const char *filename) {
	uint16_t file_id;
	char cleanname[SPIFS_FILENAME_LEN];

	// Clean the filename of problem characters.
	if (!clean_filename(filename, cleanname))
		return false;

	// Find a file id associated with the filename.
	file_id = spifs_find_filename(cleanname);

	// Erase each fileblock associated with the file.
	if (SPIFS_FILEID_VALID(file_id))
		spifs_erase_blocks(file_id);

	return 1;
}

uint8_t spifs_filesize(const char *filename, uint32_t *filesize) {
	uint8_t retval = 0;
	uint16_t file_id;
	char cleanname[SPIFS_FILENAME_LEN];

	// Clean the filename of problem characters.
	if (!clean_filename(filename, cleanname))
		return retval;

	// Find a file id associated with the filename.
	file_id = spifs_find_filename(cleanname);

	// Validate that a file was found.
	retval = SPIFS_FILEID_VALID(file_id) ? true : false;

	// Did we get a valid file id?
	if (retval) {
		// Get the file size.
		*filesize = spifs_get_file_size(file_id);
	}

	return retval;
}

uint8_t spifs_checksum(const char *filename, uint32_t *checksum) {
	uint8_t retval = 0;
	uint16_t file_id;
	char cleanname[SPIFS_FILENAME_LEN];

	// Clean the filename of problem characters.
	if (!clean_filename(filename, cleanname))
		return retval;

	// Find a file id associated with the filename.
	file_id = spifs_find_filename(cleanname);

	// Validate that a file was found.
	retval = SPIFS_FILEID_VALID(file_id) ? true : false;

	// Did we get a valid file id?
	if (retval) {
		// Get the file checksum.
		*checksum = spifs_get_file_crc(file_id);
	}

	return retval;
}

// Open the named file for reading or writing.
uint8_t spifs_open(const char *filename, uint8_t write, spifs_file_t *file) {
	uint8_t retval;
	int16_t file_id;
	uint32_t sector_offset;
	spifs_file_block_t file_block;
	char cleanname[SPIFS_FILENAME_LEN];

	// Assume we fail.
	retval = 0;

	// Initialize the file structure.
	memset(file, 0, sizeof(spifs_file_t));

	// Clean the filename of problem characters.
	if (!clean_filename(filename, cleanname))
		return false;

	// Get the file id of any file with the same filename.
	file_id = spifs_find_filename(cleanname);

	// Are we opening the file for writing?
	if (write) {
		// Erase each fileblock associated with the existing file.
		if (SPIFS_FILEID_VALID(file_id))
			spifs_erase_blocks(file_id);

		// Find a free sector.
		file_id = spifs_free_block();

		// Did we find a free sector?
		if (SPIFS_FILEID_VALID(file_id)) {
			// Prepare the sector for writing.
			file->write = true;
			file->file_id = file_id;
			file->block_id = 0;
			file->block_size = SPIFS_FILENAME_LEN;
			file->block_index = SPIFS_FILENAME_LEN;
			file->this_id = file_id;
			file->next_id = -1;
			file->position = SPIFS_FILENAME_LEN;

			// Get the byte offset of the sector at the start of the file.
			// The sector id is equal to the file id.
			sector_offset = SPIFS_SECTORID_TO_BYTEOFFSET(file_id);

			// Write the file id.
//			spiflash_write(sector_offset + SPIFS_FILEID_OFFSET,
//					(uint8_t*) &file->file_id, sizeof(int16_t));
			flashWriteBytes((uint8_t*) &file->file_id,
					sector_offset + SPIFS_FILEID_OFFSET, sizeof(int16_t));

			// Write the block id.
//			spiflash_write(sector_offset + SPIFS_BLOCKID_OFFSET,
//					(uint8_t*) &file->block_id, sizeof(int16_t));
			flashWriteBytes((uint8_t*) &file->block_id,
					sector_offset + SPIFS_BLOCKID_OFFSET, sizeof(int16_t));

			// Write the file name.
//			spiflash_write(sector_offset + SPIFS_FILENAME_OFFSET,
//					(uint8_t*) cleanname, SPIFS_FILENAME_LEN);
			flashWriteBytes((uint8_t*) cleanname,
					sector_offset + SPIFS_FILENAME_OFFSET, SPIFS_FILENAME_LEN);

			// We won't write the other header information until we begin
			// writing the next file block or the file is closed.

			// We succeeded opening the file for writing.
			retval = 1;
		}
	} else {
		// If we are reading a file we must have a valid file id.
		if (SPIFS_FILEID_VALID(file_id)) {
			// Get the byte offset of the sector at the start of the file.
			// The sector id is equal to the file id.
			sector_offset = SPIFS_SECTORID_TO_BYTEOFFSET(file_id);

			// Read in the first block header.
//			spiflash_read(sector_offset, (uint8_t*) &file_block,
//					sizeof(file_block));
			flashReadBytes((uint8_t*) &file_block, sector_offset,
					sizeof(file_block));

			// Initialize the file structure.
			file->write = false;
			file->file_id = file_block.file_id;
			file->block_id = file_block.block_id;
			file->block_size = file_block.block_size;
			file->block_index = SPIFS_FILENAME_LEN;
			file->this_id = file_block.file_id;
			file->next_id = file_block.next_id;
			file->position = SPIFS_FILENAME_LEN;

			// Sanity check the block size.
			if (!SPIFS_BLOCKSIZE_VALID(file->block_size))
				file->block_size = SPIFS_BLOCK_SIZE;

			// We succeeded opening the file for reading.
			retval = 1;
		}
	}

	return retval;
}

uint8_t spifs_close(spifs_file_t *file) {
	uint32_t sector_offset;

	// Return if file isn't open.
	if (!SPIFS_FILEID_VALID(file->file_id))
		return 0;

	// Is this file open for writing?
	if (file->write) {
		// Finalize the current sector.

		// Get the byte offset of the current sector.
		sector_offset = SPIFS_SECTORID_TO_BYTEOFFSET(file->this_id);

		// Write the block size.
//		spiflash_write(sector_offset + SPIFS_BLOCKSIZE_OFFSET,
//				(uint8_t*) &file->block_size, sizeof(int16_t));
		flashWriteBytes((uint8_t*) &file->block_size,
				sector_offset + SPIFS_BLOCKSIZE_OFFSET, sizeof(int16_t));

		// This is the final block in the file so leave the next id at -1.
	}

	// Clear the file structure.
	memset(file, 0, sizeof(spifs_file_t));

	return 1;
}

uint32_t spifs_read(spifs_file_t *file, uint8_t *buffer, uint32_t count) {
	uint32_t read_count;
	uint32_t byte_count;
	uint32_t byte_offset;
	spifs_file_block_t file_block;

	// Keep track of how much data has been read.
	read_count = 0;

	// Return if file isn't open.
	if (!SPIFS_FILEID_VALID(file->file_id))
		return false;

	// Return error if the file is not open for reading.
	if (file->write)
		return false;

	// Keep going while there is data left to read.
	while (count > 0) {
		// Do we have data still to read in this block?
		if (file->block_index < file->block_size) {
			// Get the byte offset within the current sector to read from.
			byte_offset = SPIFS_SECTORID_TO_BYTEOFFSET(file->this_id)
					+ SPIFS_HEADER_SIZE + file->block_index;

			// Determine the byte count to read.
			byte_count =
					(count > (uint32_t) (file->block_size - file->block_index)) ?
							(uint32_t) (file->block_size - file->block_index) :
							count;

			// Read in the data from the block.
//			spiflash_read(byte_offset, (uint8_t*) buffer, byte_count);
			flashReadBytes((uint8_t*) &buffer, byte_offset, byte_count);

			// Adjust the buffers, indices and counts.
			count -= byte_count;
			buffer += byte_count;
			read_count += byte_count;
			file->block_index += byte_count;
			file->position += byte_count;
		} else {
			// Make sure there is another buffer to read.
			if (SPIFS_FILEID_VALID(file->next_id)) {
				// Get the byte offset of the next sector.
				byte_offset = SPIFS_SECTORID_TO_BYTEOFFSET(file->next_id);

				// Read in the block header.
//				spiflash_read(byte_offset, (uint8_t*) &file_block,
//						sizeof(file_block));
				flashReadBytes((uint8_t*) &file_block, byte_offset,
						sizeof(file_block));

				// Update the file structure.
				file->block_id = file_block.block_id;
				file->block_size = file_block.block_size;
				file->block_index = 0;
				file->this_id = file->next_id;
				file->next_id = file_block.next_id;

				// Sanity check the block size.
				if (!SPIFS_BLOCKSIZE_VALID(file->block_size))
					file->block_size = SPIFS_BLOCK_SIZE;
			} else {
				// No more data to read in the file.
				count = 0;
			}
		}
	}

	// Return the number of bytes read.
	return read_count;
}

uint32_t spifs_write(spifs_file_t *file, const uint8_t *buffer, uint32_t count) {
	int16_t next_id;
	uint32_t wrote;
	uint32_t byte_count;
	uint32_t byte_offset;

	// Keep track of how much data has been written.
	wrote = 0;

	// Return if file isn't open.
	if (!SPIFS_FILEID_VALID(file->file_id))
		return 0;

	// Return error if the file is not open for writing.
	if (!file->write)
		return 0;

	// Keep going while there is data left to write.
	while (count > 0) {
		// Is there still room to write in this block?
		if (file->block_index < SPIFS_BLOCK_SIZE) {
			// Get the byte offset within the current sector to write to.
			byte_offset = SPIFS_SECTORID_TO_BYTEOFFSET(file->this_id)
					+ SPIFS_HEADER_SIZE + file->block_index;

			// Determine the byte count to write.
			byte_count =
					(count > (uint32_t) (SPIFS_BLOCK_SIZE - file->block_index)) ?
							(uint32_t) (SPIFS_BLOCK_SIZE - file->block_index) :
							count;

			// Write out the data to the block.
//			spiflash_write(byte_offset, (uint8_t*) buffer, byte_count);
			flashWriteBytes((uint8_t*) &buffer, byte_offset, byte_count);

			// Adjust the buffers, indices and counts.
			count -= byte_count;
			wrote += byte_count;
			buffer += byte_count;
			file->block_index += byte_count;
			file->block_size = file->block_index;
			file->position += byte_count;
		} else {
			// Find a free sector.
			next_id = spifs_free_block();

			// Make sure there is another buffer to write.
			if (SPIFS_FILEID_VALID(next_id)) {
				// Close out the current block.

				// Get the byte offset of the current sector.
				byte_offset = SPIFS_SECTORID_TO_BYTEOFFSET(file->this_id);

				// Write the block size.
//				spiflash_write(byte_offset + SPIFS_BLOCKSIZE_OFFSET,
//						(uint8_t*) &file->block_size, sizeof(int16_t));
				flashWriteBytes((uint8_t*) &file->block_size,
						byte_offset + SPIFS_BLOCKSIZE_OFFSET, sizeof(int16_t));

				// Write the next sector id.
//				spiflash_write(byte_offset + SPIFS_NEXTID_OFFSET,
//						(uint8_t*) &next_id, sizeof(int16_t));
				flashWriteBytes((uint8_t*) &next_id,
						byte_offset + SPIFS_NEXTID_OFFSET, sizeof(int16_t));

				// Prepare this next sector for writing.
				file->block_id = file->block_id + 1;
				file->block_size = 0;
				file->block_index = 0;
				file->this_id = next_id;
				file->next_id = -1;

				// Get the byte offset of the sector at the start of the file.
				// The sector id is equal to the file id.
				byte_offset = SPIFS_SECTORID_TO_BYTEOFFSET(next_id);

				// Write the file id.
//				spiflash_write(byte_offset + SPIFS_FILEID_OFFSET,
//						(uint8_t*) &file->file_id, sizeof(int16_t));
				flashWriteBytes((uint8_t*) &file->file_id,
						byte_offset + SPIFS_FILEID_OFFSET, sizeof(int16_t));

				// Write the block id.
//				spiflash_write(byte_offset + SPIFS_BLOCKID_OFFSET,
//						(uint8_t*) &file->block_id, sizeof(int16_t));
				flashWriteBytes((uint8_t*) &file->block_id,
						byte_offset + SPIFS_BLOCKID_OFFSET, sizeof(int16_t));

				// We won't write the other header information until we begin
				// writing the next file block or the file is closed.
			} else {
				// Unable to write more data to the file system.
				count = 0;
			}
		}
	}

	// Return the number of bytes written.
	return wrote;
}

uint8_t spifs_seek(spifs_file_t *file, uint32_t position) {
	uint8_t done;
	int16_t this_id;
	spifs_file_block_t file_block;

	// Return if file isn't open.
	if (!SPIFS_FILEID_VALID(file->file_id))
		return 0;

	// We can only seek while reading a file.
	if (file->write)
		return 0;

	// Add the filename to the position.
	position += SPIFS_FILENAME_LEN;

	// If we are already at the position, then don't bother.
	if (file->position == position)
		return 1;

	// Get the first sector in the file from the file id.
	this_id = file->file_id;

	// We are not yet done.
	done = 0;

	// Loop over each sector in the file to find the right position.
	while (!done && SPIFS_SECTORID_VALID(this_id)) {
		// Read the fileblock for this sector.
//		spiflash_read(SPIFS_SECTORID_TO_BYTEOFFSET(this_id),
//				(uint8_t*) &file_block, sizeof(file_block));
		flashReadBytes((uint8_t*) &file_block,
				SPIFS_SECTORID_TO_BYTEOFFSET(this_id), sizeof(file_block));

		// Update the file structure.
		file->block_id = file_block.block_id;
		file->block_size = file_block.block_size;
		file->block_index = file_block.block_size;
		file->this_id = this_id;
		file->next_id = file_block.next_id;

		// Is the position within this block?
		if (position <= (file->position + (uint32_t) file_block.block_size)) {
			// Update the block index based on the position.
			file->block_index = position - file->position;

			// Set the file position.
			file->position = position;

			// We are done.
			done = 1;
		} else {
			// Add the block size to the file position.
			file->position += (uint32_t) file_block.block_size;

			// Set this id to the next id.
			this_id = file_block.next_id;
		}
	}
	return 1;
}

uint8_t spifs_open_dir(spifs_dir_t *dir) {
	// Initialize the file directory structure.
	memset(dir, 0, sizeof(spifs_dir_t));

	return 1;
}

uint8_t spifs_close_dir(spifs_dir_t *dir) {
	// Reset the file directory structure.
	memset(dir, 0, sizeof(spifs_dir_t));

	return 1;
}

uint8_t spifs_read_dir(spifs_dir_t *dir) {
	uint8_t retval;
	uint32_t sector_offset;
	spifs_file_block_t file_block;

	// Increment the file id.
	dir->file_id += 1;

	// We haven't found the next file yet.
	retval = 0;

	// Loop over each sector to check for the file.
	while (!retval && SPIFS_FILEID_VALID(dir->file_id)) {
		// Get the byte offset of the sector.
		sector_offset = SPIFS_SECTORID_TO_BYTEOFFSET(dir->file_id);

		// Read the fileblock at the start of the sector.
//		spiflash_read(sector_offset, (uint8_t*) &file_block,
//				sizeof(file_block));
		flashReadBytes((uint8_t*) &file_block, sector_offset,
				sizeof(file_block));

		// Is this the first block in a file?
		if (SPIFS_FILEID_VALID(file_block.file_id)
				&& (file_block.block_id == 0)) {
			// Yes. Set the sector offset to the file name.
			sector_offset += sizeof(file_block);

			// Read the filename.
//			spiflash_read(sector_offset, (uint8_t*) dir->filename,
//			SPIFS_FILENAME_LEN);
			flashReadBytes((uint8_t*) &dir->filename, sector_offset,
					SPIFS_FILENAME_LEN);

			// Read the file size.
			dir->file_size = spifs_get_file_size(dir->file_id);

			// We found the next filename.
			retval = 1;
		} else {
			// No. Increment to the next file id.
			dir->file_id += 1;
		}
	}

	return retval;
}
