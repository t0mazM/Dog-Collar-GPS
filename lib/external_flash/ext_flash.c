#include "ext_flash.h"



static const char *TAG = "EXT_FLASH";
spi_device_handle_t spi;

spi_bus_config_t get_spi_bus_config(void) {
    spi_bus_config_t spi_bus_config;
    memset(&spi_bus_config, 0, sizeof(spi_bus_config)); // Clear all

    spi_bus_config.mosi_io_num = SPI_PIN_MOSI;
    spi_bus_config.miso_io_num = SPI_PIN_MISO;
    spi_bus_config.sclk_io_num = SPI_PIN_CLK;
    spi_bus_config.quadwp_io_num = -1;
    spi_bus_config.quadhd_io_num = -1;
    spi_bus_config.max_transfer_sz = SPI_MAX_TRANSFER_SIZE;

    return spi_bus_config;
}

spi_device_interface_config_t get_spi_device_config(void) {
    spi_device_interface_config_t devcfg;
    memset(&devcfg, 0, sizeof(devcfg)); // Clear all fields to 0

    devcfg.command_bits = 8;
    devcfg.clock_speed_hz = SPI_CLOCK_SPEED;
    devcfg.mode = 0;
    devcfg.spics_io_num = SPI_PIN_CS;
    devcfg.queue_size = 1;

    return devcfg;
}


void ext_flash_init(void) {

    SPI_set_HOLD_WP_HIGH();
    
    spi_bus_config_t buscfg = get_spi_bus_config();
    spi_device_interface_config_t devcfg = get_spi_device_config();


    CUSTUM_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // Add device to the bus
    CUSTUM_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi));

    ESP_LOGI(TAG, "SPI Flash initialized successfully and device added.");

    ext_flash_reset_chip();// Wake/reset flash chip
}

void ext_flash_reset_chip(void) {
    spi_transaction_t t;
    memset(&t, 0, sizeof(t)); // we only send command

    t.cmd = SPI_CMD_ENABLE_RESET; 
    CUSTUM_ERROR_CHECK(spi_device_transmit(spi, &t) );

    ESP_LOGI(TAG, "Sent Enable Reset (0x%02X)", SPI_CMD_ENABLE_RESET);

    t.cmd = SPI_CMD_RESET_DEVICE; 
    CUSTUM_ERROR_CHECK(spi_device_transmit(spi, &t));
    ESP_LOGI(TAG, "Sent Reset Device (0x%02X)", SPI_CMD_RESET_DEVICE);
}

// Read JEDEC ID: returns 3 bytes
esp_err_t ext_flash_read_jedec_data(uint8_t *buf) {
    spi_transaction_t t;
    memset(&t, 0, sizeof(t)); // Clear all fields to 0

    // Transaction for JEDEC ID command (0x9F) and 3 bytes of data
    t.length = SPI_JEDEC_DATA_BITS;
    t.rxlength = SPI_JEDEC_DATA_BITS;
    t.cmd = SPI_CMD_JEDEC_ID;

    uint8_t recived_data[3]; // Buffer to hold the 3 received bytes from the flash chip
    t.rx_buffer = recived_data;

    CUSTUM_ERROR_CHECK( spi_device_transmit(spi, &t));

    buf[0] = recived_data[0]; // Manufacturer ID
    buf[1] = recived_data[1]; // Memory Type
    buf[2] = recived_data[2]; // Capacity

    ESP_LOGI(TAG, "JEDEC ID: %02X %02X %02X", buf[0], buf[1], buf[2]);
    return ESP_OK;
}

// Optional: provide flash handle externally
spi_device_handle_t ext_flash_get_handle(void) {
    return spi;
}

esp_err_t ext_flash_write_enable(void) {
    spi_transaction_t t = {
        .length = 0,
        .cmd = SPI_CMD_WRITE_ENABLE,
    };
    esp_err_t ret = spi_device_transmit(spi, &t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send Write Enable: %s", esp_err_to_name(ret));
    }
    ESP_LOGI(TAG, "Write Enable command sent (0x%02X)", SPI_CMD_WRITE_ENABLE);
    return ret;
}

esp_err_t ext_flash_read_status_register(uint8_t *status_reg_value) {
    spi_transaction_t t = {
        .length = 8, // dummy bits
        .rxlength = 8, // Expect 1 byte (8 bits) back
        .cmd = SPI_CMD_READ_STATUS_REG1, // Command: Read Status Register-1 (0x05)
        .flags = 0,
    };

    uint8_t received_status;
    t.rx_buffer = &received_status;

    esp_err_t ret = spi_device_transmit(spi, &t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read Status Register-1: %s", esp_err_to_name(ret));
    } else {
        *status_reg_value = received_status;
        ESP_LOGI(TAG, "Read Status Register-1: 0x%02X", *status_reg_value);
    }
    return ret;
}

esp_err_t ext_flash_wait_for_idle(int timeout_ms) {
    uint8_t status_reg;

    // Start time for timeout calculation
    TickType_t start_time = xTaskGetTickCount();

    do {
        ESP_RETURN_ON_ERROR(
            ext_flash_read_status_register(&status_reg),
            TAG, "Failed to read status register while waiting for flash operation to complete");

        // Check the BUSY bit (LSB of status register)
        if (!(status_reg & (1 << 0))) { 
            ESP_LOGI(TAG, "Flash is not bussy.");
            return ESP_OK;
        }

        vTaskDelay(pdMS_TO_TICKS(1)); // Wait 1ms before polling again

        if ((xTaskGetTickCount() - start_time) * portTICK_PERIOD_MS > timeout_ms) {
            ESP_LOGE(TAG, "Timeout waiting for flash operation to complete. Status: 0x%02X", status_reg);
            return ESP_ERR_TIMEOUT;
        }
    } while (true); 

    return ESP_OK; 
}
