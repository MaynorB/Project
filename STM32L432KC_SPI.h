#ifndef STM32L432KC_SPI_H
#define STM32L432KC_SPI_H

#include <stdint.h>

#define SPI_SCK   PB3
#define SPI_MISO  PB4
#define SPI_MOSI  PB5
#define SPI_CE    PA11

void initSPI(int br, int cpol, int cpha);
char spiSendReceive(char send);

void SD_Select(void);
void SD_Deselect(void);
uint8_t SD_SPI_Transmit(uint8_t data);
uint8_t SD_SPI_Receive(void);

#endif

