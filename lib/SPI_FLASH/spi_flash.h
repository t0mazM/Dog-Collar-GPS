#ifndef SPI_FLASH_H
#define SPI_FLASH_H

#include "driver/spi_master.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_err.h"

#define SPI_PIN_NUM_MOSI GPIO_NUM_21
#define SPI_PIN_NUM_MISO GPIO_NUM_0
#define SPI_PIN_NUM_CLK  GPIO_NUM_20
#define SPI_PIN_NUM_CS   GPIO_NUM_3
#define SPI_PIN_NUM_WP   GPIO_NUM_1
#define SPI_PIN_NUM_HOLD GPIO_NUM_10

#define SPI_CMD_JEDEC_ID     0x9F
#define SPI_CMD_ENABLE_RESET 0x66
#define SPI_CMD_RESET_DEVICE 0x99

#define SPI_MAX_TRANSFER_SIZE 512
#define SPI_CLOCK_SPEED 8 * 1000 * 1000 // 8 MHz

void hold_wp_setup(void);
void init_spi_flash(void);
esp_err_t spi_flash_read_jedec(uint8_t *buf);
esp_err_t spi_flash_write(uint32_t addr, uint8_t *data, size_t len);
esp_err_t spi_flash_read(uint32_t addr, uint8_t *data, size_t len);
spi_device_handle_t spi_flash_get_handle(void);



#endif  // SPI_FLASH_H