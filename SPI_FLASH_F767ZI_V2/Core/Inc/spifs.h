/*
 * spifs.h
 *
 *  Created on: 26 Aug 2021
 *      Author: adityasehgal
 */

#ifndef INC_SPIFS_H_
#define INC_SPIFS_H_

#define FS_FILES_LIST_OFFSET 0	//offset at which the list containing FS details are
#define FS_FILES_LIST_ADDR 0	//address at which the list containing FS details are
#define FS_FILES_LIST_NUM_FILES_ADDR 7	//address at which number of stored files is
#define FS_FILES_START_SEQ_OFFSET 0
#define FS_FILES_END_SEQ_OFFSET 251

#define SPIFS_FILE_NAME_LEN_MAX 15
#define SPIFS_FILE_ID_LEN_MAX 1
#define SPIFS_FILE_SIZE_LEN_MAX 1
#define SPIFS_FILE_LEN_MAX_BYTES (256 - SPIFS_FILE_NAME_LEN_MAX - SPIFS_FILE_ID_LEN_MAX - SPIFS_FILE_SIZE_LEN_MAX)
#define SPIFS_MAX_FILES 16

#define SPIFS_ID_OFFSET 0
#define SPIFS_NAME_OFFSET (SPIFS_ID_OFFSET + 1)
#define SPIFS_SIZE_OFFSET (SPIFS_NAME_OFFSET + SPIFS_FILE_NAME_LEN_MAX + 1)
#define SPIFS_DATA_OFFSET (SPIFS_ID_OFFSET + SPIFS_NAME_OFFSET + SPIFS_SIZE_OFFSET + 1)

#define SPIFS_OK 0
#define SPIFS_ERR 1
#define SPIFS_NO_MOUNT 2
#define SPIFS_TOO_MANY_FILES 3


typedef struct fileSystem_t {
	uint8_t fsMounted;	//flag to make sure fs is mounted
	uint8_t fileAddr;
	uint8_t fileId;
	uint8_t fileName[SPIFS_FILE_NAME_LEN_MAX];
	uint8_t size;
	uint8_t fileData[SPIFS_MAX_FILES];
} fileSystem;

uint8_t spifsMount(void);

uint8_t spifsOpenFile(char *name, uint8_t rdrw);

#endif /* INC_SPIFS_H_ */
