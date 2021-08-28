/*
 * spifs.c
 *
 *  Created on: 26 Aug 2021
 *      Author: adityasehgal
 */

#include "flash.h"
#include "spifs.h"
#include <stdio.h>
#include <string.h>

fileSystem fs = { .fsMounted = SPIFS_NO_MOUNT, };

static uint8_t spifsGetNumFiles(void) {
	uint8_t num = 0;
	flashReadByte(&num, FS_FILES_LIST_NUM_FILES_ADDR);
	return num;
}

static uint8_t spifsIncrementFileNum(void) {
	uint8_t currNum = 0;
	uint8_t newNum = 0;
	uint8_t fileListSector[FLASH_ATTR_SECTOR_SIZE];
	uint8_t fileListSectorNew[FLASH_ATTR_SECTOR_SIZE];
	memset(fileListSector, 0, FLASH_ATTR_SECTOR_SIZE);

	flashReadBytes(fileListSector, FS_FILES_LIST_ADDR,
	FLASH_ATTR_SECTOR_SIZE);
	for (int i = 0; i < FLASH_ATTR_SECTOR_SIZE; i++) {
		fileListSectorNew[i] = fileListSector[i];
	}
	currNum = fileListSectorNew[FS_FILES_LIST_NUM_FILES_ADDR];

	if (currNum >= SPIFS_MAX_FILES) {
		printf("Too many files, delete some!\n");
		return SPIFS_TOO_MANY_FILES;
	}

	flashEraseSector(FS_FILES_LIST_NUM_FILES_ADDR);

	fileListSectorNew[FS_FILES_LIST_NUM_FILES_ADDR] = currNum + 1;
	flashWriteSector(fileListSectorNew, FS_FILES_LIST_ADDR, 1);

	flashReadBytes(fileListSector, FS_FILES_LIST_ADDR,
	FLASH_ATTR_SECTOR_SIZE);
	newNum = fileListSector[FS_FILES_LIST_NUM_FILES_ADDR];

	if (newNum != currNum + 1) {
		return SPIFS_ERR;
	}
	return SPIFS_OK;
}

uint8_t spifsMount(void) {
	/**
	 * What mounting does:
	 *	* Check if the files list data is present
	 *	*	*	Here we need
	 *	*	*	* Bytes[6:0] = "$SPIFS#"
	 *	*	*	* Byte[7] = #  of files present
	 *	*	*	* Bytes[255:251] = "#END$"
	 *	* If the data is not present, then format and write it
	 */

	const char *FS_FILES_START_SEQ = "$SPIFS#";
	const char *FS_FILES_END_SEQ = "#END$";

	uint32_t size = FLASH_ATTR_PAGE_SIZE;
	uint8_t read[size];
	uint8_t write[size];
	flashReadBytes(read, FS_FILES_LIST_ADDR + FS_FILES_LIST_OFFSET, size);

	uint8_t startSeqLen = strlen(FS_FILES_START_SEQ) - 0;
	uint8_t endSeqLen = strlen(FS_FILES_END_SEQ) - 0;
	uint8_t startSeq[startSeqLen];
	uint8_t endSeq[endSeqLen];

	uint16_t initCount = 0;
	//Get start sequence
	for (int i = 0; i < startSeqLen; i++) {
		startSeq[i] = read[i + FS_FILES_START_SEQ_OFFSET];
		if (startSeq[i] != FS_FILES_START_SEQ[i]) {
			initCount++;
		}
	}

	for (int i = 0; i < endSeqLen; i++) {
		endSeq[i] = read[i + FS_FILES_END_SEQ_OFFSET];
		if (endSeq[i] != FS_FILES_END_SEQ[i]) {
			initCount++;
		}
	}

	if (initCount) {
		printf("File format not found, formatting and reinitializing\n");
		flashEraseChip();

		//start sequence
		write[0] = '$';
		write[1] = 'S';
		write[2] = 'P';
		write[3] = 'I';
		write[4] = 'F';
		write[5] = 'S';
		write[6] = '#';

		//num of files
		write[7] = 0;

		for (int i = 8; i < 251; i++) {
			write[i] = 0;
		}

		//end seq
		write[251] = '#';
		write[252] = 'E';
		write[253] = 'N';
		write[254] = 'D';
		write[255] = '$';

		flashWriteBytes(write, FS_FILES_LIST_ADDR + FS_FILES_LIST_OFFSET, size);

		memset(read, 0, size);

		flashReadBytes(read, FS_FILES_LIST_ADDR + FS_FILES_LIST_OFFSET, size);

		for (int i = 0; i < startSeqLen; i++) {
			startSeq[i] = read[i + FS_FILES_START_SEQ_OFFSET];
			if (startSeq[i] != FS_FILES_START_SEQ[i]) {
				initCount++;
			}
		}

		for (int i = 0; i < endSeqLen; i++) {
			endSeq[i] = read[i + FS_FILES_END_SEQ_OFFSET];
			if (endSeq[i] != FS_FILES_END_SEQ[i]) {
				initCount++;
			}
		}

		if (initCount) {
			fs.fsMounted = SPIFS_NO_MOUNT;
			return SPIFS_NO_MOUNT;
		}

	} else {
		fs.fsMounted = SPIFS_OK;
		return SPIFS_OK;
	}
	fs.fsMounted = SPIFS_OK;
	return SPIFS_OK;
}

static void readFileName(uint8_t id, char *name) {
	flashReadBytes((uint8_t*) name,
			(id * SPIFS_NAME_OFFSET) + FLASH_ATTR_PAGE_SIZE,
			SPIFS_FILE_NAME_LEN_MAX);
}

static uint8_t findFileName(char *name) {
	uint8_t fileNum = spifsGetNumFiles();
	uint8_t nameTemp[SPIFS_FILE_NAME_LEN_MAX];

	for (int i = 0; i < fileNum + 2; i++) {
		readFileName(i, (char*) nameTemp);
		if (!strcmp(name, (char*) nameTemp)) {
			return SPIFS_OK;
		}
	}
	return SPIFS_ERR;
}

static uint8_t spifsCreateFile(char *name) {

	uint32_t size = FLASH_ATTR_PAGE_SIZE;
//	uint8_t read[size];
	uint8_t write[size];

	uint8_t fileNum = spifsGetNumFiles();

	printf("# of files: %d\n", fileNum);

	//check if file exists
	if (findFileName(name) == SPIFS_OK) {
		printf("File already exists, returning\n");
		return SPIFS_OK;
	}

	spifsIncrementFileNum();
	fileNum = spifsGetNumFiles();
	printf("# of files: %d\n", fileNum);

	fs.fileId = fileNum;
	if (fileNum == 0) {
		fs.fileAddr = fs.fileId * SPIFS_ID_OFFSET;
	}
	strcpy((char*) fs.fileName, name);
	printf("File name: %s\n", fs.fileName);

	memset(write, 0, size);

	write[SPIFS_ID_OFFSET] = fs.fileId;

	write[SPIFS_NAME_OFFSET + 0] = fs.fileName[0];
	write[SPIFS_NAME_OFFSET + 1] = fs.fileName[1];
	write[SPIFS_NAME_OFFSET + 2] = fs.fileName[2];
	write[SPIFS_NAME_OFFSET + 3] = fs.fileName[3];
	write[SPIFS_NAME_OFFSET + 4] = fs.fileName[4];
	write[SPIFS_NAME_OFFSET + 5] = fs.fileName[5];
	write[SPIFS_NAME_OFFSET + 6] = fs.fileName[6];
	write[SPIFS_NAME_OFFSET + 7] = fs.fileName[7];
	write[SPIFS_NAME_OFFSET + 8] = fs.fileName[8];
	write[SPIFS_NAME_OFFSET + 9] = fs.fileName[9];
	write[SPIFS_NAME_OFFSET + 10] = fs.fileName[10];
	write[SPIFS_NAME_OFFSET + 11] = fs.fileName[11];
	write[SPIFS_NAME_OFFSET + 12] = fs.fileName[12];
	write[SPIFS_NAME_OFFSET + 13] = fs.fileName[13];
	write[SPIFS_NAME_OFFSET + 14] = fs.fileName[14];

	flashWriteBytes(write, FLASH_ATTR_PAGE_SIZE + fs.fileAddr, size);

	return SPIFS_OK;

}

uint8_t spifsOpenFile(char *name, uint8_t rdrw) {
	if (1) { //fs.fsMounted != SPIFS_OK) {
		spifsCreateFile(name);

	} else {
		printf("FS not mounted\n");
		return SPIFS_ERR;
	}
	return SPIFS_OK;
}

