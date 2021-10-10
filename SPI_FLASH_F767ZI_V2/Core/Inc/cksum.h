/*
 * cksum.h
 *
 *  Created on: Oct 9, 2021
 *      Author: adityasehgal
 */

#ifndef INC_CKSUM_H_
#define INC_CKSUM_H_

uint32_t cksumCalcCksum(uint8_t *data, uint32_t size);

uint32_t cksumCalcCrc32(uint8_t *data, uint32_t size);

#endif /* INC_CKSUM_H_ */
