#ifndef SD_LOWLEVEL_H
#define SD_LOWLEVEL_H

#include <stdint.h>
#include <stdbool.h>

/* SD low-level API for STM32L432KC
 * Single-block read/write (512 bytes per block) */

#ifdef __cplusplus
extern "C" {
#endif

uint8_t SD_Init(void);
uint8_t SD_ReadBlock(uint32_t sector, uint8_t* buffer);   /* read single 512-byte block */
uint8_t SD_WriteBlock(uint32_t sector, const uint8_t* buffer); /* write single 512-byte block */
uint8_t SD_GetCardType(void);

#define SD_OK      0
#define SD_TIMEOUT 1
#define SD_ERROR   2

#ifdef __cplusplus
}
#endif

#endif /* SD_LOWLEVEL_H */
