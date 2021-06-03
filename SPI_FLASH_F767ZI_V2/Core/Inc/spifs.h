/*
 * spifs.h
 *
 *  Created on: Jun 2, 2021
 *      Author: adityasehgal
 */

#ifndef INC_SPIFS_H_
#define INC_SPIFS_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

	/**
	 * Library for MX25V1606F 16Mb NOR Flash
	 * https://www.macronix.com/Lists/Datasheet/Attachments/7796/MX25V1606F,%202.5V,%2016Mb,%20v1.0.pdf
	 */

	/** @defgroup SPI_FS_CMD
	 * @{
	 */

#define SPI_FS_CMD_DUMMY 0xA5//0xFF

	/** @defgroup SPI_FS_CMD / Array Access
	 * @{
	 */
#define SPI_FS_CMD_READ 			0x03
#define SPI_FS_CMD_FAST_READ 		0x0B
#define SPI_FS_CMD_DREAD 			0x3B
#define SPI_FS_CMD_PAGE_PROGRAM 	0x02
#define SPI_FS_CMD_SECTOR_ERASE 	0x20
#define SPI_FS_CMD_BLOCK_ERASE_32K	0x52
#define SPI_FS_CMD_BLOCK_ERASE_64K	0xD8
#define SPI_FS_CMD_CHIP_ERASE 		0x60 // OR 0xC7
#define SPI_FS_CMD_READ_SFDP 		0x5A
	/**	CLOSE SPI_FS_CMD / Array Access
	 * @}
	 */

	/** @defgroup SPI_FS_CMD / Device Operation
	 * @{
	 */
#define SPI_FS_CMD_WRITE_ENABLE 		0x06
#define SPI_FS_CMD_WRITE_DISABLE 		0x04
#define SPI_FS_CMD_DEEP_PWR_DOWN 		0xB9
#define SPI_FS_CMD_FACTORY_MODE_ENABLE 	0x41
	/**	CLOSE SPI_FS_CMD / Device Operation
	 * @}
	 */

	/** @defgroup SPI_FS_CMD / Register Access
	 * @{
	 */
#define SPI_FS_CMD_READ_ID 			0x9F
#define SPI_FS_CMD_READ_EID 		0xAB	//NRND
#define SPI_FS_CMD_READ_EMANU_DEVID	0x90
#define SPI_FS_CMD_READ_STATUS_REG 	0x05
#define SPI_FS_CMD_WRITE_STATUS_REG 0x01
	/**	CLOSE SPI_FS_CMD / Register Access
	 * @}
	 */

	/**	CLOSE SPI_FS_CMD
	 * @}
	 */

	/** @defgroup SPI_FS_EXP //expected values
	 * @{
	 */
#define SPI_FS_EXP_MANU_ID 		0xC2
#define SPI_FS_EXP_MEM_TYPE 	0x20
#define SPI_FS_EXP_MEM_DENSITY 	0x15
	/**	CLOSE SPI_FS_EXP
	 * @}
	 */

//typedef enum {
//	W25Q10 = 1,
//	W25Q20,
//	W25Q40,
//	W25Q80,
//	W25Q16,
//	W25Q32,
//	W25Q64,
//	W25Q128,
//	W25Q256,
//	W25Q512,
//
//} spifs_ID_t;
//
//typedef struct {
//	spifs_ID_t ID;
//	uint8_t UniqID[8];
//	uint16_t PageSize;
//	uint32_t PageCount;
//	uint32_t SectorSize;
//	uint32_t SectorCount;
//	uint32_t BlockSize;
//	uint32_t BlockCount;
//	uint32_t CapacityInKiloByte;
//	uint8_t StatusRegister1;
//	uint8_t StatusRegister2;
//	uint8_t StatusRegister3;
//	uint8_t Lock;
//
//} w25qxx_t;
//
//extern w25qxx_t w25qxx;
//############################################################################
// in Page,Sector and block read/write functions, can put 0 to read maximum bytes
//############################################################################
	bool spifsInit(void);

	uint32_t spifsReadID(void);

	void spifsEraseChip(void);
	void spifsEraseSector(uint32_t SectorAddr);
	void spifsEraseBlock(uint32_t BlockAddr);

	uint32_t spifsPageToSector(uint32_t PageAddress);
	uint32_t spifsPageToBlock(uint32_t PageAddress);
	uint32_t spifsSectorToBlock(uint32_t SectorAddress);
	uint32_t spifsSectorToPage(uint32_t SectorAddress);
	uint32_t spifsBlockToPage(uint32_t BlockAddress);

	bool spifsIsEmptyPage(uint32_t Page_Address, uint32_t OffsetInByte,
			uint32_t NumByteToCheck_up_to_PageSize);
	bool spifsIsEmptySector(uint32_t Sector_Address, uint32_t OffsetInByte,
			uint32_t NumByteToCheck_up_to_SectorSize);
	bool spifsIsEmptyBlock(uint32_t Block_Address, uint32_t OffsetInByte,
			uint32_t NumByteToCheck_up_to_BlockSize);

	void spifsWriteByte(uint8_t pBuffer, uint32_t Bytes_Address);
	void spifsWritePage(uint8_t *pBuffer, uint32_t Page_Address,
			uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_PageSize);
	void spifsWriteSector(uint8_t *pBuffer, uint32_t Sector_Address,
			uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_SectorSize);
	void spifsWriteBlock(uint8_t *pBuffer, uint32_t Block_Address,
			uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_BlockSize);

	void spifsReadByte(uint8_t *pBuffer, uint32_t Bytes_Address);
	void spifsReadBytes(uint8_t *pBuffer, uint32_t ReadAddr,
			uint32_t NumByteToRead);
	void spifsReadPage(uint8_t *pBuffer, uint32_t Page_Address,
			uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_PageSize);
	void spifsReadSector(uint8_t *pBuffer, uint32_t Sector_Address,
			uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_SectorSize);
	void spifsReadBlock(uint8_t *pBuffer, uint32_t Block_Address,
			uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_BlockSize);
//############################################################################
#ifdef __cplusplus
}
#endif

#endif /* INC_SPIFS_H_ */
