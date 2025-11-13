#include "diskio.h"
#include "STM32L432KC_SPI.h"
#include "SD_lowlevel.h"   // your SD command functions

#define SD_CARD  0

DSTATUS disk_initialize(BYTE pdrv) {
    if (pdrv != SD_CARD) return STA_NOINIT;
    if (SD_Init() == SD_OK) return 0;
    return STA_NOINIT;
}

DSTATUS disk_status(BYTE pdrv) {
    if (pdrv != SD_CARD) return STA_NOINIT;
    return 0;
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count) {
    if (pdrv != SD_CARD) return RES_PARERR;
    return (SD_ReadBlock(sector, buff, count) == SD_OK) ? RES_OK : RES_ERROR;
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count) {
    if (pdrv != SD_CARD) return RES_PARERR;
    return (SD_WriteBlock(sector, buff, count) == SD_OK) ? RES_OK : RES_ERROR;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
    // handle CTRL_SYNC, GET_SECTOR_COUNT, etc. if needed
    return RES_OK;
}
