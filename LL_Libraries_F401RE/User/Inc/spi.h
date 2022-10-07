/*
 * spi.h
 *
 *  Created on: 16 Mar 2022
 *      Author: adityasehgal
 */

#ifndef INC_SPI_H_
#define INC_SPI_H_

void spi_write(uint8_t *buf, uint32_t size);

void spi_read(uint8_t *buf, uint32_t size);

void spi_readWrite(uint8_t *wbuf, uint32_t wSize, uint8_t *rbuf, uint32_t rSize);

void spi_init(void);

#endif /* INC_SPI_H_ */
