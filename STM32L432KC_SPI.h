#ifndef STM32L4_SPI_H
#define STM32L4_SPI_H

#include <stdint.h>
#include <stm32l432xx.h>

#define SPI_CE   PA11
#define SPI_SCK  PB3
#define SPI_MOSI PB5
#define SPI_MISO PB4

void initSPI(int br, int cpol, int cpha);
char spiSendReceive(char send);

/* SD helper wrappers used by SD_lowlevel.c */
void SD_Select(void);
void SD_Deselect(void);
uint8_t SD_SPI_Transmit(uint8_t data);
uint8_t SD_SPI_Receive(void);

#endif /* STM32L4_SPI_H */
