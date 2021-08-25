/*
 * packets.h
 *
 *  Created on: Jun 22, 2021
 *      Author: adityasehgal
 */

#ifndef INC_PACKETS_H_
#define INC_PACKETS_H_

#define MASTER_IP "192.168.1.255"//...255 will broadcast

/**
 * NUMS PACKET: OPEN
 */

// IP address that the nums packets are transmitted to.
#define NUMS_IP                      MASTER_IP

// UDP/IP port that nums packets are transmitted to.
#define NUMS_PORT                    6000

// Maximum number of 8 byte num frames per NUMS packet.
#define NUMS_MAX_FRAMES              16

// Battery Management System (BMS) packet.
typedef struct numsPacket
{

  uint8_t sequenceNum;
  uint8_t packetVersion;
  uint8_t frames[NUMS_MAX_FRAMES][8];  // data frames.

} numsPacket_t;

/**
 * NUMS PACKET: CLOSE
 */

/**
 * DEVICE PACKET: OPEN
 */
// IP address that power packets are transmitted to.
#define DEVICE_IP                    MASTER_IP

// UDP/IP port that power packets are transmitted to.
#define DEVICE_PORT                  6001

typedef struct devicePacket
{

  uint8_t sequenceNum;
  uint8_t reserved0;	//kept for symmetry across packet
  uint8_t reserved1;	//kept for symmetry across packet
  uint8_t reserved2;	//kept for symmetry across packet
  uint32_t halVersion;
  uint32_t deviceRevID;
  uint32_t deviceID;
  uint32_t deviceUID0;
  uint32_t deviceUID1;
  uint32_t deviceUID2;
  uint32_t reserved3;	//kept as a separator
  uint32_t halTick;

} devicePacket_t;
/**
 * DEVICE PACKET: OPEN
 */

#endif /* INC_PACKETS_H_ */
