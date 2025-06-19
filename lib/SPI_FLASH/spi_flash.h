#ifndef SPI_FLASH_H
#define SPI_FLASH_H

#include "driver/spi_master.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_err.h"

#define PIN_NUM_MISO 18
#define PIN_NUM_MOSI 12
#define PIN_NUM_CLK  11
#define PIN_NUM_CS   15
#define PIN_NUM_WP   17
#define PIN_NUM_HOLD 10

#define CMD_JEDEC_ID 0x9F
void init_spi_flash(void);
esp_err_t spi_flash_read_jedec(uint8_t *buf);
esp_err_t spi_flash_write(uint32_t addr, uint8_t *data, size_t len);
esp_err_t spi_flash_read(uint32_t addr, uint8_t *data, size_t len);
spi_device_handle_t spi_flash_get_handle(void);



#endif  // SPI_FLASH_H