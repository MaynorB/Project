#include "SD_lowlevel.h"
#include "STM32L432KC_SPI.h"
#include <stddef.h>
#include <string.h>

// Command definitions (SPI mode)
#define CMD0    (0x40+0)   // GO_IDLE_STATE
#define CMD8    (0x40+8)   // SEND_IF_COND
#define CMD17   (0x40+17)  // READ_SINGLE_BLOCK
#define CMD24   (0x40+24)  // WRITE_SINGLE_BLOCK
#define CMD55   (0x40+55)  // APP_CMD
#define CMD58   (0x40+58)  // READ_OCR
#define ACMD41  (0x40+41)  // APP_SEND_OP_COND

// Data tokens
#define TOKEN_START_BLOCK 0xFE

// Card types
#define CARD_UNKNOWN 0
#define CARD_SDv1    1
#define CARD_SDv2    2
#define CARD_SDHC    3

// Static internal variable
static uint8_t cardType = CARD_UNKNOWN;

// Helper functions
static uint8_t SD_SendCommand(uint8_t cmd, uint32_t arg, uint8_t crc);
static uint8_t SD_WaitReady(void);
static void SD_DummyClock(uint8_t n);

// ---------------------------------------------------------------------------
// Initialize SD card into SPI mode
// ---------------------------------------------------------------------------
uint8_t SD_Init(void) {
    uint8_t r;
    uint8_t ocr[4];

    SD_Deselect();
    SD_DummyClock(10); // at least 74 clock cycles

    // CMD0: go to idle state
    if (SD_SendCommand(CMD0, 0, 0x95) != 1) return SD_ERROR;

    // CMD8: check voltage range
    r = SD_SendCommand(CMD8, 0x1AA, 0x87);
    if (r == 1) {
        // SDv2
        for (int i = 0; i < 4; i++) ocr[i] = SD_SPI_Receive();
        if (ocr[2] == 0x01 && ocr[3] == 0xAA) {
            // ACMD41 loop
            do {
                SD_SendCommand(CMD55, 0, 0x01);
                r = SD_SendCommand(ACMD41, 0x40000000, 0x01);
            } while (r != 0);

            // CMD58: check CCS bit
            SD_SendCommand(CMD58, 0, 0x01);
            for (int i = 0; i < 4; i++) ocr[i] = SD_SPI_Receive();
            cardType = (ocr[0] & 0x40) ? CARD_SDHC : CARD_SDv2;
        }
    } else {
        // SDv1 or MMC
        SD_SendCommand(CMD55, 0, 0x01);
        r = SD_SendCommand(ACMD41, 0, 0x01);
        if (r <= 1) {
            cardType = CARD_SDv1;
            while (r) {
                SD_SendCommand(CMD55, 0, 0x01);
                r = SD_SendCommand(ACMD41, 0, 0x01);
            }
        } else {
            cardType = CARD_UNKNOWN;
            return SD_ERROR;
        }
    }

    SD_Deselect();
    SD_SPI_Receive(); // extra clock
    return SD_OK;
}

// ---------------------------------------------------------------------------
// Read a single 512-byte sector
// ---------------------------------------------------------------------------
uint8_t SD_ReadBlock(uint32_t sector, uint8_t* buffer) {
    if (cardType != CARD_SDHC) sector *= 512; // convert to byte address

    if (SD_SendCommand(CMD17, sector, 0x01) != 0) {
        SD_Deselect();
        return SD_ERROR;
    }

    // Wait for data token
    uint16_t timeout = 0xFFFF;
    uint8_t token;
    do {
        token = SD_SPI_Receive();
    } while (token == 0xFF && --timeout);

    if (token != TOKEN_START_BLOCK) {
        SD_Deselect();
        return SD_TIMEOUT;
    }

    // Read data
    for (int i = 0; i < 512; i++) {
        buffer[i] = SD_SPI_Receive();
    }

    // Discard CRC
    SD_SPI_Receive();
    SD_SPI_Receive();

    SD_Deselect();
    SD_SPI_Receive(); // dummy clock

    return SD_OK;
}

// ---------------------------------------------------------------------------
// Write a single 512-byte sector
// ---------------------------------------------------------------------------
uint8_t SD_WriteBlock(uint32_t sector, const uint8_t* buffer) {
    if (cardType != CARD_SDHC) sector *= 512;

    if (SD_SendCommand(CMD24, sector, 0x01) != 0) {
        SD_Deselect();
        return SD_ERROR;
    }

    SD_SPI_Transmit(TOKEN_START_BLOCK);

    for (int i = 0; i < 512; i++) {
        SD_SPI_Transmit(buffer[i]);
    }

    // Dummy CRC
    SD_SPI_Transmit(0xFF);
    SD_SPI_Transmit(0xFF);

    // Get data response
    uint8_t response = SD_SPI_Receive();
    if ((response & 0x1F) != 0x05) {
        SD_Deselect();
        return SD_ERROR;
    }

    // Wait for card ready
    if (SD_WaitReady() != 0) {
        SD_Deselect();
        return SD_TIMEOUT;
    }

    SD_Deselect();
    SD_SPI_Receive();

    return SD_OK;
}

// ---------------------------------------------------------------------------
// Get card type (for debug)
// ---------------------------------------------------------------------------
uint8_t SD_GetCardType(void) {
    return cardType;
}

// ---------------------------------------------------------------------------
// Send a command packet (CMDx + args + CRC)
// ---------------------------------------------------------------------------
static uint8_t SD_SendCommand(uint8_t cmd, uint32_t arg, uint8_t crc) {
    uint8_t res;
    SD_Deselect();
    SD_Select();

    SD_SPI_Transmit(cmd);
    SD_SPI_Transmit(arg >> 24);
    SD_SPI_Transmit(arg >> 16);
    SD_SPI_Transmit(arg >> 8);
    SD_SPI_Transmit(arg);
    SD_SPI_Transmit(crc);

    // Wait for response (MSB = 0)
    for (int i = 0; i < 10; i++) {
        res = SD_SPI_Receive();
        if (!(res & 0x80)) break;
    }

    return res;
}

// ---------------------------------------------------------------------------
// Wait for the card to be ready (response 0xFF)
// ---------------------------------------------------------------------------
static uint8_t SD_WaitReady(void) {
    uint16_t timeout = 0xFFFF;
    uint8_t res;
    do {
        res = SD_SPI_Receive();
    } while (res != 0xFF && --timeout);
    return (timeout == 0) ? 1 : 0;
}

// ---------------------------------------------------------------------------
// Send dummy clocks with MOSI high (needed before/after commands)
// ---------------------------------------------------------------------------
static void SD_DummyClock(uint8_t n) {
    SD_Deselect();
    for (uint8_t i = 0; i < n; i++) {
        SD_SPI_Receive();
    }
}
