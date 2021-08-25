/*
 * flash.h
 *
 *  Created on: Jun 2, 2021
 *      Author: adityasehgal
 */

#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#include <stdbool.h>
#include <stdint.h>


/**
 * Library for MX25V1606F 16Mb NOR Flash
 * https://www.macronix.com/Lists/Datasheet/Attachments/7796/MX25V1606F,%202.5V,%2016Mb,%20v1.0.pdf
 */

/** @defgroup FLASH_CMD
 * @{
 */

#define FLASH_CMD_DUMMY 0xA5//0xFF

/** @defgroup FLASH_CMD / Array Access
 * @{
 */
#define FLASH_CMD_READ 			0x03
#define FLASH_CMD_READ_FAST 		0x0B
#define FLASH_CMD_DREAD 			0x3B
#define FLASH_CMD_PAGE_PROGRAM 	0x02
#define FLASH_CMD_SECTOR_ERASE 	0x20
#define FLASH_CMD_BLOCK_ERASE_32K	0x52
#define FLASH_CMD_BLOCK_ERASE_64K	0xD8
#define FLASH_CMD_CHIP_ERASE 		0x60 // OR 0xC7
#define FLASH_CMD_READ_SFDP 		0x5A
/**	CLOSE FLASH_CMD / Array Access
 * @}
 */

/** @defgroup FLASH_CMD / Device Operation
 * @{
 */
#define FLASH_CMD_WREN 		0x06
#define FLASH_CMD_WRDIS 		0x04
#define FLASH_CMD_DEEP_PWR_DWN 		0xB9
#define FLASH_CMD_FMEN 	0x41
/**	CLOSE FLASH_CMD / Device Operation
 * @}
 */

/** @defgroup FLASH_CMD / Register Access
 * @{
 */
#define FLASH_CMD_READ_ID 			0x9F
#define FLASH_CMD_READ_EID 		0xAB	//NRND
#define FLASH_CMD_READ_EMANU_DEVID	0x90
#define FLASH_CMD_READ_STATUS_REG 	0x05
#define FLASH_CMD_WRITE_STATUS_REG 0x01
/**	CLOSE FLASH_CMD / Register Access
 * @}
 */

/**	CLOSE FLASH_CMD
 * @}
 */

/** @defgroup FLASH_EXP //expected values
 * @{
 */
#define FLASH_EXP_MANU_ID 		0xC2
#define FLASH_EXP_MEM_TYPE 	0x20
#define FLASH_EXP_DEVID 	0x15
/**	CLOSE FLASH_EXP
 * @}
 */

/**
 * @desc: Returns flash ID
 * @param: void
 * @param: uint32_t: ret[24:16] = Manufacturers ID
 * 					 ret[15:8] = Memory Type
 * 					 ret[7:0] = Memory Density
 */
uint32_t flashReadId(void);

/**
 * @desc: Erases the entire chip - fills with '1's
 * @param: void
 * @param: void
 */
void flashEraseChip(void);

/**
 * @desc: Erases a 4KB sector
 * @param: addr: absolute address of where the erase should begin
 * @param: void
 */
void flashEraseSector(uint32_t addr);

/**
 * @desc: Erases a 32KB sector
 * @param: addr: absolute address of where the erase should begin
 * @param: void
 */
void flashEraseBlock(uint32_t addr);

/**
 * @desc: Writes a single byte to an address
 * @param: buff: buffer of size 1 containing the value to write
 * @param: addr: absolute address of where byte should be written
 * @param: void
 */
void flashWriteByte(uint8_t buff, uint32_t addr);

/**
 * @desc: Writes the specified number of bytes starting at the address
 * @param: buff: buffer containing the data to write
 * @param: addr: absolute address of where the write should begin
 * @param: size: number of bytes to write
 * @param: void
 */
void flashWriteBytes(uint8_t *buff, uint32_t address, uint32_t size);

/**
 * @desc: Reads a single byte from an address
 * @param: buff: buffer to contain the read byte
 * @param: addr: absolute address of where the byte is read from
 * @param: void
 */
void flashReadByte(uint8_t *buff, uint32_t addr);

/**
 * @desc: Reads a the specified number of bytes starting at the address
 * @param: buff: buffer to contain the read data
 * @param: addr: absolute address of where the data is read from
 * @param: size: number of bytes to be read
 * @param: void
 */
void flashReadBytes(uint8_t *buff, uint32_t addr, uint32_t size);

#endif /* INC_FLASH_H_ */
