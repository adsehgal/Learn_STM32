/*
 * spifs.h
 *
 *  Created on: Apr 29, 2021
 *      Author: adityasehgal
 */

#ifndef INC_SPIFS_H_
#define INC_SPIFS_H_

/**
 * Library for MX25V1606F 16Mb NOR Flash
 * https://www.macronix.com/Lists/Datasheet/Attachments/7796/MX25V1606F,%202.5V,%2016Mb,%20v1.0.pdf
 */

/** @defgroup SPI_FS_CMD
 * @{
 */

#define SPI_FS_CMD_DUMMY 0xFF

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

//function prototypes:
HAL_StatusTypeDef spifsGetID(uint8_t *manufacturerId, uint8_t *memType,
		uint8_t *memDensity);

#endif /* INC_SPIFS_H_ */
