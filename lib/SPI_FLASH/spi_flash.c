#include "spi_flash.h"


static const char *TAG = "SPI_FLASH";
spi_device_handle_t spi;

void init_spi_flash(void) {
    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = PIN_NUM_WP,      
        .quadhd_io_num = PIN_NUM_HOLD,    
        .max_transfer_sz = 4096
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 8 * 1000 * 1000, // 8 MHz
        .mode = 0,                         // SPI mode 0
        .spics_io_num = PIN_NUM_CS,        // CS pin
        .queue_size = 7,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi));

    ESP_LOGI(TAG, "SPI Flash initialized.");
}



esp_err_t spi_flash_read_jedec(uint8_t *buf) {
    spi_transaction_t t = {
        .length = 8 * 4,           // total bits: 1 command + 3 bytes read
        .rxlength = 8 * 4,         // same length for read part
        .flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA
    };

    // Send JEDEC command (0x9F), rest of tx_buffer doesn't matter
    t.tx_data[0] = CMD_JEDEC_ID;

    esp_err_t ret = spi_device_transmit(spi, &t);  // Transmit command
    if (ret != ESP_OK) return ret;

    // First byte in rx_data is response to 0x9F itself, skip it
    buf[0] = t.rx_data[1];  // Manufacturer ID
    buf[1] = t.rx_data[2];  // Memory type
    buf[2] = t.rx_data[3];  // Capacity

    ESP_LOGI(TAG, "JEDEC ID: %02X %02X %02X", buf[0], buf[1], buf[2]);

    return ESP_OK;
}


spi_device_handle_t spi_flash_get_handle(void) {
    return spi;
}