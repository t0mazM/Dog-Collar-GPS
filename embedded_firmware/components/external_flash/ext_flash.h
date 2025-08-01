/*
 * Copyright © 2025 Tomaz Miklavcic
 *
 * Use this code for whatever you want. No restrictions, no warranty.
 * Attribution appreciated but not required.
 */

#ifndef EXT_FLASH_H
#define EXT_FLASH_H

#include "driver/spi_master.h"
#include "../gpio_expander/gpio_expander.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "external_flash_gpio.h" // For WP and HOLD pin definitions
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
#define SPI_CMD_GLOBAL_BLOCK_UNLOCK 0x98 // Global Block Unlock (W25Q128JV specific)
// --- W25Q128JV Flash Parameters ---
#define W25Q128JV_PAGE_SIZE         256      // Bytes per page
#define W25Q128JV_SECTOR_SIZE       4096     // Bytes per 4KB sector
#define W25Q128JV_TOTAL_SIZE_BYTES  (16 * 1024 * 1024) // 128M-bit = 16M-byte

esp_err_t ext_flash_init(void);

/**
 * @brief Reset the external flash chip.
 * 
 * This function sends the Enable Reset command followed by 
 * the Reset Device command to reset the external flash chip.

   @note Reset is not neccessary but a good practice to ensure the chip is in a known state.
 */ 
esp_err_t ext_flash_reset_chip(void);


/**
 * @brief Read the JEDEC ID of the external flash chip.
 * 
 * This function sends the JEDEC ID command and reads and prints the 
 * Manufacturer ID, Memory Type, and Capacity.
 * 
 * @note Used for testing and verification of the flash chip.
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t ext_flash_read_jedec_data(void);

/* ----------------- functions for littlefs integration ----------------- */

/**
 * @brief Send te write enable command to the external flash chip
 * 
 * This function sends the enable command so that the chip is ready for write operations. 
 * 
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t ext_flash_write_enable(void);

/**
 * @brief Read the status register of the external flash chip.
 * 
 * This function reads the Status Register-1 of the external flash chip.
 * If the external flash chip is not busy (usually erasing or writing) it will register value 0x00.
 * 
 * @param status Pointer to a variable where the status value will be stored.
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t ext_flash_read_status_register(uint8_t *status);

/**
 * @brief Wait for the external flash chip to be idle.
 * 
 * This function waits for the external flash chip to be idle (not busy).
 * With polling (while loop) it checks the status register until the BUSY bit is cleared or a timeout occurs.
 * 
 * @param timeout_ms Timeout in milliseconds to wait for the flash to become idle.
 * @note Some block deletion commands may take even 1-2 seconds, so a timeout of 1000 ms is recommended.
 * @return ESP_OK if the flash is idle, or an error code on timeout or failure.
 */
esp_err_t ext_flash_wait_for_idle(int timeout_ms);


/**
 * @brief Read data from the external flash chip.
 * 
 * This function reads a specified number of bytes from the external flash chip starting at a given address.
 * 
 * @param address The starting address to read from.
 * @param buffer Pointer to the buffer where the read data will be stored.
 * @param size Number of bytes to read.
 * @return ESP_OK on success, or an error code on failure.
 */

 /* --------------------------Functions for littlefs integration ---------------------------*/
 /**
  * @brief Read data from the external flash chip.
  * This function reads a specified number of bytes from the external flash chip starting at a given address.
  * @param address The starting address to read from.
  * @param buffer Pointer to the buffer where the read data will be stored.
  * @param size Number of bytes to read.
  * @return ESP_OK on success, or an error code on failure.
  */
esp_err_t ext_flash_read(uint32_t address, uint8_t *buffer, uint32_t size);

/**
 * @brief Write data to the external flash chip.
 * 
 * This function writes a specified number of bytes to the external flash chip starting at a given address.
 * @note size must be less than or equal to the page size (256 bytes for W25Q128JV).
 * 
 * @param address The starting address to write to.
 * @param buffer Pointer to the buffer containing the data to write.
 * @param size Number of bytes to write (must be <= 256).
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t ext_flash_write(uint32_t address, const uint8_t *buffer, uint32_t size);

/**
 * @brief Erase a sector of the external flash chip.
 * 
 * This function erases a 4KB sector of the external flash chip at the specified address.
 * The address must be aligned to a 4KB boundary.
 * 
 * @param address The starting address of the sector to erase (must be sector-aligned).
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t ext_flash_erase_sector(uint32_t address);
esp_err_t ext_flash_chip_erase(void);
esp_err_t ext_flash_global_block_unlock(void);


void ext_flash_complete_test(void); //Just a quick test function to check if the flash is working correctly

#endif  // EXT_FLASH_H