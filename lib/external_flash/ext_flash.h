#ifndef EXT_FLASH_H
#define EXT_FLASH_H

#include "driver/spi_master.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "external_flash_gpio.h" // For WP and HOLD pin definitions
#include "error_handling.h" 


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

void ext_flash_reset_chip(void);
void ext_flash_init(void);
esp_err_t ext_flash_read_jedec_data(uint8_t *buf);
esp_err_t ext_flash_write(uint32_t addr, uint8_t *data, size_t len);
esp_err_t sext_flash_read(uint32_t addr, uint8_t *data, size_t len);
spi_device_handle_t ext_flash_get_spi_handle(void);



#endif  // EXT_FLASH_H