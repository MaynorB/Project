#ifndef SD_LOWLEVEL_H
#define SD_LOWLEVEL_H

#include <stdint.h>
#include <stdbool.h>

// SD Card command function prototypes
uint8_t SD_Init(void);
uint8_t SD_ReadBlock(uint32_t sector, uint8_t* buffer);
uint8_t SD_WriteBlock(uint32_t sector, const uint8_t* buffer);
uint8_t SD_GetCardType(void);

// SD return codes
#define SD_OK          0
#define SD_TIMEOUT     1
#define SD_ERROR       2

#endif
