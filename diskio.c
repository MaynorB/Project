#include "diskio.h"
#include "SD_lowlevel.h"
#include <string.h>

#define SD_CARD 0

DSTATUS disk_initialize(BYTE pdrv) {
    if (pdrv != SD_CARD) return STA_NOINIT;
    return (SD_Init() == SD_OK) ? 0 : STA_NOINIT;
}

DSTATUS disk_status(BYTE pdrv) {
    if (pdrv != SD_CARD) return STA_NOINIT;
    return 0;
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count) {
    if (pdrv != SD_CARD) return RES_PARERR;
    for (UINT i = 0; i < count; i++) {
        if (SD_ReadBlock(sector + i, buff + (i * 512)) != SD_OK) return RES_ERROR;
    }
    return RES_OK;
}

#if FF_FS_READONLY == 0
DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count) {
    if (pdrv != SD_CARD) return RES_PARERR;
    for (UINT i = 0; i < count; i++) {
        if (SD_WriteBlock(sector + i, buff + (i * 512)) != SD_OK) return RES_ERROR;
    }
    return RES_OK;
}
#endif

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
    if (pdrv != SD_CARD) return RES_PARERR;
    switch (cmd) {
        case CTRL_SYNC: return RES_OK;
        case GET_SECTOR_SIZE: *(WORD*)buff = 512; return RES_OK;
        case GET_BLOCK_SIZE: *(DWORD*)buff = 1; return RES_OK;
        case GET_SECTOR_COUNT: *(DWORD*)buff = 0; return RES_OK; /* optional: implement by reading CSD */
        default: return RES_PARERR;
    }
}
