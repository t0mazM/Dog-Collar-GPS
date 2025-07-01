#include "ext_flash.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


static const char *TAG = "EXT_FLASH";
spi_device_handle_t spi;

spi_bus_config_t get_spi_bus_config(void) {
    return (spi_bus_config_t) {
        .mosi_io_num = SPI_PIN_MOSI,
        .miso_io_num = SPI_PIN_MISO,
        .sclk_io_num = SPI_PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = SPI_MAX_TRANSFER_SIZE,
    };
}

spi_device_interface_config_t get_spi_device_config(void) {
    return (spi_device_interface_config_t) {
        .command_bits = 8,
        .clock_speed_hz = SPI_CLOCK_SPEED,
        .mode = 0,
        .spics_io_num = SPI_PIN_CS,
        .queue_size = 1,
    };
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
    spi_transaction_t t = {
        .length = 0, 
        .rxlength = 0, 
        .flags = 0,  
    };

    t.cmd = SPI_CMD_ENABLE_RESET; 
    CUSTUM_ERROR_CHECK(spi_device_transmit(spi, &t) );

    ESP_LOGI(TAG, "Sent Enable Reset (0x%02X)", SPI_CMD_ENABLE_RESET);

    t.cmd = SPI_CMD_RESET_DEVICE; 
    CUSTUM_ERROR_CHECK(spi_device_transmit(spi, &t));
    ESP_LOGI(TAG, "Sent Reset Device (0x%02X)", SPI_CMD_RESET_DEVICE);
}

// Read JEDEC ID: returns 3 bytes
esp_err_t ext_flash_read_jedec_data(uint8_t *buf) {
    
    // Transaction for JEDEC ID command (0x9F) and 3 bytes of data
    spi_transaction_t t = {
        .length = SPI_JEDEC_DATA_BITS, 
        .rxlength = SPI_JEDEC_DATA_BITS, 
        .cmd = SPI_CMD_JEDEC_ID, 
        .flags = 0,  
    };

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
    return ret;
}

/**
 * @brief Reads Status Register-1 (0x05) from the flash chip.
 * @param status Pointer to a uint8_t to store the status register value.
 * @return esp_err_t ESP_OK on success, error otherwise.
 */
esp_err_t ext_flash_read_status_register(uint8_t *status) {
    spi_transaction_t t = {
        .length = 1 * 8, // 1 byte to receive
        .rxlength = 1 * 8,
        .cmd = SPI_CMD_READ_STATUS_REG1,
        .rx_buffer = status,
    };
    esp_err_t ret = spi_device_transmit(spi, &t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read Status Register: %s", esp_err_to_name(ret));
    }
    return ret;
}

/**
 * @brief Waits for the flash chip to become idle (BUSY bit in Status Register-1 is 0).
 * @return esp_err_t ESP_OK if chip becomes idle, error if timeout or communication issue.
 */
esp_err_t ext_flash_wait_for_idle(void) {
    uint8_t status;
    int timeout_ms = 1000; // Max timeout for operations (e.g., chip erase can take seconds)
    int elapsed_ms = 0;
    const int delay_ms = 1; // Check every 1ms

    do {
        esp_err_t ret = ext_flash_read_status_register(&status);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read status while waiting for idle.");
            return ret;
        }
        // Check the BUSY bit (S0)
        if (! (status & 0x01)) { // If BUSY bit is 0
            return ESP_OK;
        }
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
        elapsed_ms += delay_ms;
    } while (elapsed_ms < timeout_ms);

    ESP_LOGE(TAG, "Flash chip busy timeout!");
    return ESP_ERR_TIMEOUT;
}

/**
 * @brief Reads data from the external flash chip.
 * @param address Starting address to read from.
 * @param buffer Pointer to the buffer to store read data.
 * @param size Number of bytes to read.
 * @return esp_err_t ESP_OK on success, error otherwise.
 */
esp_err_t ext_flash_read(uint32_t address, uint8_t *buffer, uint32_t size) {
    if (size == 0) return ESP_OK; // Nothing to read

    spi_transaction_t t = {
        .cmd = SPI_CMD_READ_DATA,
        .addr = address,
        .length = size * 8, // Data length in bits
        .rxlength = size * 8,
        .rx_buffer = buffer,
        .flags = 0,
    };

    esp_err_t ret = spi_device_transmit(spi, &t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read data from 0x%06X, size %u: %s", address, size, esp_err_to_name(ret));
    }
    return ret;
}


esp_err_t ext_flash_write(uint32_t address, const uint8_t *buffer, uint32_t size) {
    if (size == 0) return ESP_OK; // Nothing to write

    uint32_t current_address = address;
    uint32_t bytes_remaining = size;
    const uint8_t *current_buffer = buffer;

    while (bytes_remaining > 0) {
        // Calculate bytes to write in current page
        uint32_t page_offset = current_address % W25Q128JV_PAGE_SIZE;
        uint32_t bytes_to_program_in_page = W25Q128JV_PAGE_SIZE - page_offset;
        if (bytes_to_program_in_page > bytes_remaining) {
            bytes_to_program_in_page = bytes_remaining;
        }

        // 1. Send Write Enable
        CUSTUM_ERROR_CHECK(ext_flash_write_enable());
        
        // 2. Prepare transaction for Page Program
        spi_transaction_t t = {
            .cmd = SPI_CMD_PAGE_PROGRAM,
            .addr = current_address,
            .length = bytes_to_program_in_page * 8, // Data length in bits
            .tx_buffer = current_buffer,
            .flags = 0,
        };

        // 3. Transmit Page Program command and data
        esp_err_t ret = spi_device_transmit(spi, &t);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to program page at 0x%06X, size %u: %s", current_address, bytes_to_program_in_page, esp_err_to_name(ret));
            return ret;
        }

        // 4. Wait for the chip to become idle after programming
        CUSTUM_ERROR_CHECK(ext_flash_wait_for_idle());

        current_address += bytes_to_program_in_page;
        current_buffer += bytes_to_program_in_page;
        bytes_remaining -= bytes_to_program_in_page;
    }
    ESP_LOGD(TAG, "Successfully wrote %u bytes to 0x%06X", size, address);
    return ESP_OK;
}

/**
 * @brief Erases a 4KB sector on the external flash chip.
 * @param address Starting address of the sector to erase. Must be 4KB aligned.
 * @return esp_err_t ESP_OK on success, error otherwise.
 */
esp_err_t ext_flash_erase_sector(uint32_t address) {
    if (address % W25Q128JV_SECTOR_SIZE != 0) {
        ESP_LOGE(TAG, "Erase address 0x%06X is not 4KB aligned!", address);
        return ESP_ERR_INVALID_ARG;
    }

    // 1. Send Write Enable
    CUSTUM_ERROR_CHECK(ext_flash_write_enable());

    // 2. Prepare transaction for Sector Erase
    spi_transaction_t t = {
        .cmd = SPI_CMD_SECTOR_ERASE,
        .addr = address,
        .length = 0, // No data to send
        .flags = 0,
    };

    // 3. Transmit Sector Erase command
    esp_err_t ret = spi_device_transmit(spi, &t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to erase sector at 0x%06X: %s", address, esp_err_to_name(ret));
        return ret;
    }

    // 4. Wait for the chip to become idle after erasing
    CUSTUM_ERROR_CHECK(ext_flash_wait_for_idle());
    ESP_LOGD(TAG, "Successfully erased sector at 0x%06X", address);
    return ESP_OK;
}

/**
 * @brief Erases the entire external flash chip.
 * @return esp_err_t ESP_OK on success, error otherwise.
 */
esp_err_t ext_flash_chip_erase(void) {
    ESP_LOGW(TAG, "Performing full chip erase. This will take some time...");

    // 1. Send Write Enable
    CUSTUM_ERROR_CHECK(ext_flash_write_enable());

    // 2. Prepare transaction for Chip Erase
    spi_transaction_t t = {
        .cmd = SPI_CMD_CHIP_ERASE,
        .length = 0, // No data to send
        .flags = 0,
    };

    // 3. Transmit Chip Erase command
    esp_err_t ret = spi_device_transmit(spi, &t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to chip erase: %s", esp_err_to_name(ret));
        return ret;
    }

    // 4. Wait for the chip to become idle after erasing
    CUSTUM_ERROR_CHECK(ext_flash_wait_for_idle());
    ESP_LOGI(TAG, "Chip erase completed successfully.");
    return ESP_OK;
}