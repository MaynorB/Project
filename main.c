#include "stm32l432xx.h"
#include "STM32L432KC_SPI.h"
#include "SD_lowlevel.h"
#include "STM32L432KC_GPIO.h"
#include "ff.h"    // FatFS core
#include "diskio.h"
#include <string.h>
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////
// Debug LED setup
///////////////////////////////////////////////////////////////////////////////
#define LED_PIN PA5   // Change if needed for your board

///////////////////////////////////////////////////////////////////////////////
// Global FatFS variables
///////////////////////////////////////////////////////////////////////////////
FATFS FatFs;      // File system object
FIL file;         // File object
FRESULT fres;     // FatFS function common result code

///////////////////////////////////////////////////////////////////////////////
// Delay helper (simple software delay)
///////////////////////////////////////////////////////////////////////////////
static void delay_ms(uint32_t ms) {
    for (uint32_t i = 0; i < ms * 4000; i++) {
        __NOP();
    }
}

///////////////////////////////////////////////////////////////////////////////
// LED feedback helpers
///////////////////////////////////////////////////////////////////////////////
static void blink(uint8_t times) {
    for (uint8_t i = 0; i < times; i++) {
        digitalWrite(LED_PIN, 1);
        delay_ms(200);
        digitalWrite(LED_PIN, 0);
        delay_ms(200);
    }
}

static void led_on(void) {
    digitalWrite(LED_PIN, 1);
}

static void led_off(void) {
    digitalWrite(LED_PIN, 0);
}

// Function used by printf to send characters to the laptop
int _write(int file, char *ptr, int len) {
  int i = 0;
  for (i = 0; i < len; i++) {
    ITM_SendChar((*ptr++));
  }
  return len;
}

///////////////////////////////////////////////////////////////////////////////
// MAIN
///////////////////////////////////////////////////////////////////////////////
int main(void) {
    // Basic setup
    SystemCoreClockUpdate();
    initSPI(0b111 , 0, 0); // Baud = fPCLK/8, mode 0 (CPOL=0, CPHA=0)
    pinMode(LED_PIN, GPIO_OUTPUT);
    led_off();

    ///////////////////////////////////////////////////////////////////////////
    // 1. Initialize SPI
    ///////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////
    // 2. Initialize SD card (low-level)
    ///////////////////////////////////////////////////////////////////////////
    uint8_t sdStatus = SD_Init();
    if (sdStatus != SD_OK) {
        // 🔴 Blink 3x = SD init failed
        printf("SD FAILED \n");
        blink(3);
        while (1);
    }
    printf("SD INITIALIZED \n");
      ///////////////////////////////////////////////////////////////////////////
      // 3. Mount FAT filesystem and open file
      ///////////////////////////////////////////////////////////////////////////
      fres = f_mount(&FatFs, "", 1);
      if (fres != FR_OK) {
          // 🔴 Blink 4x = Mount failed
          printf("MOUNT FAILED \n");
          blink(4);
          while (1);
      }

   fres = f_open(&file, "Crepe.wav", FA_READ); 
   if (fres != FR_OK) { 
      // 🔴 Blink 5x = File not found 
      blink(5); 
      while (1); 
    }
     printf("FOUND FILE \n");
    ///////////////////////////////////////////////////////////////////////////////
    // 4. Read WAV header (44 bytes)
    ///////////////////////////////////////////////////////////////////////////////
    BYTE header[44];
    UINT bytesRead;

    fres = f_read(&file, header, 44, &bytesRead);
    if (fres != FR_OK || bytesRead != 44) {
        printf("Failed to read WAV header!\n");
        blink(6);
        while (1);
    }

    // Print header
    printf("=== WAV HEADER (44 bytes) ===\n");
    for (int i = 0; i < 44; i++) {
        printf("%02X ", header[i]);
        if (i % 16 == 15) printf("\r\n");
    }
    printf("\n");

    ///////////////////////////////////////////////////////////////////////////////
    // 5. Determine PCM data size
    ///////////////////////////////////////////////////////////////////////////////
    uint32_t dataSize = 
          (header[40] << 0)
        | (header[41] << 8)
        | (header[42] << 16)
        | (header[43] << 24);

    printf("WAV DATA SIZE = %lu bytes\n", dataSize);

    ///////////////////////////////////////////////////////////////////////////////
    // 6. Read entire audio file in 512-byte chunks
    ///////////////////////////////////////////////////////////////////////////////
    BYTE buffer[512];
    UINT br;
    uint32_t totalRead = 0;

    printf("=== BEGIN AUDIO DATA ===\n");

    while (totalRead < dataSize) {
        uint32_t toRead = (dataSize - totalRead);
        if (toRead > 512) toRead = 512;

        fres = f_read(&file, buffer, toRead, &br);
        if (fres != FR_OK || br == 0) {
            printf("ERROR: Failed during streaming audio!\n");
            break;
        }

        totalRead += br;

        // Dump PCM bytes to ITM SWO in hex
        for (int i = 0; i < br; i++) {
            printf("%02X ", buffer[i]);
            if ((i % 32) == 31) printf("\r\n");
        }
    }

    printf("\n=== END AUDIO DATA ===\n");
    printf("Read total %lu bytes.\n", totalRead);

    f_close(&file);

    printf("File Closed");
    while (1){
    }
}
