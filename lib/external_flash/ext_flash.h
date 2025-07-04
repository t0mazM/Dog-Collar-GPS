#ifndef EXT_FLASH_H
#define EXT_FLASH_H

#include "driver/spi_master.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "external_flash_gpio.h" // For WP and HOLD pin definitions
#include "error_handling.h" 
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"

#include <string.h>

#define SPI_PIN_MOSI GPIO_NUM_21
#define SPI_PIN_MISO GPIO_NUM_0
#define SPI_PIN_CLK  GPIO_NUM_20
#define SPI_PIN_CS   GPIO_NUM_3
// WP and HOLD pins are defined in spi_gpio_config.h , since they are manually controlled

#define SPI_CMD_JEDEC_ID     0x9F
#define SPI_CMD_ENABLE_RESET 0x66
#define SPI_CMD_RESET_DEVICE 0x99

#define SPI_JEDEC_DATA_BITS 24 // JEDEC ID is 3 bytes (3*8 bits)

#define SPI_MAX_TRANSFER_SIZE 512
#define SPI_CLOCK_SPEED 8 * 1000 * 1000 // 8 MHz


// --- W25Q128JV Specific Commands (from datasheet) ---
#define SPI_CMD_WRITE_ENABLE        0x06 // Write Enable
#define SPI_CMD_WRITE_DISABLE       0x04 // Write Disable
#define SPI_CMD_READ_STATUS_REG1    0x05 // Read Status Register-1
#define SPI_CMD_READ_DATA           0x03 // Read Data (Standard SPI)
#define SPI_CMD_FAST_READ           0x0B // Fast Read (Standard SPI)
#define SPI_CMD_PAGE_PROGRAM        0x02 // Page Program
#define SPI_CMD_SECTOR_ERASE        0x20 // Sector Erase (4KB)
#define SPI_CMD_BLOCK_ERASE_32KB    0x52 // Block Erase (32KB)
#define SPI_CMD_BLOCK_ERASE_64KB    0xD8 // Block Erase (64KB)
#define SPI_CMD_CHIP_ERASE          0xC7 // Chip Erase (or 0x60)
#define SPI_CMD_JEDEC_ID            0x9F // Read JEDEC ID
#define SPI_CMD_ENABLE_RESET        0x66 // Enable Reset
#define SPI_CMD_RESET_DEVICE        0x99 // Reset Device

// --- W25Q128JV Flash Parameters ---
#define W25Q128JV_PAGE_SIZE         256      // Bytes per page
#define W25Q128JV_SECTOR_SIZE       4096     // Bytes per 4KB sector
#define W25Q128JV_TOTAL_SIZE_BYTES  (16 * 1024 * 1024) // 128M-bit = 16M-byte


void ext_flash_reset_chip(void);
void ext_flash_init(void);
esp_err_t ext_flash_read_jedec_data(uint8_t *buf);
spi_device_handle_t ext_flash_get_spi_handle(void);

// New functions for LittleFS integration
esp_err_t ext_flash_write_enable(void);
esp_err_t ext_flash_read_status_register(uint8_t *status);
esp_err_t ext_flash_wait_for_idle(int timeout_ms);
esp_err_t ext_flash_read(uint32_t address, uint8_t *buffer, uint32_t size);
esp_err_t ext_flash_write(uint32_t address, const uint8_t *buffer, uint32_t size);
esp_err_t ext_flash_erase_sector(uint32_t address);
esp_err_t ext_flash_chip_erase(void);


void ext_flash_complete_test(void); //Just a quick test function to check if the flash is working correctly

#endif  // EXT_FLASH_H