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

    devcfg.command_bits = 8; // 8-bit command
    devcfg.address_bits = 24; // Set default address bits to 24 for W25Q128JV for commands that use addresses
    devcfg.dummy_bits = 8;   // Set default dummy bits to 8 for Fast Read (0x0B)
    devcfg.clock_speed_hz = SPI_CLOCK_SPEED,
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
    spi_transaction_ext_t t;
    memset(&t, 0, sizeof(t));
    
    // Reset commands have NO address, NO dummy bits, and NO data
    t.base.length = 0;
    t.base.rxlength = 0;
    t.base.flags = SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_VARIABLE_DUMMY;
    t.address_bits = 0;
    t.dummy_bits = 0;

    // Send Enable Reset command
    t.base.cmd = SPI_CMD_ENABLE_RESET;
    CUSTUM_ERROR_CHECK(spi_device_transmit(spi, (spi_transaction_t*)&t));
    ESP_LOGI(TAG, "Sent Enable Reset (0x%02X)", SPI_CMD_ENABLE_RESET);

    // Send Reset Device command
    t.base.cmd = SPI_CMD_RESET_DEVICE;
    CUSTUM_ERROR_CHECK(spi_device_transmit(spi, (spi_transaction_t*)&t));
    ESP_LOGI(TAG, "Sent Reset Device (0x%02X)", SPI_CMD_RESET_DEVICE);
}

esp_err_t ext_flash_read_jedec_data(uint8_t *buf) {
    spi_transaction_ext_t t; // Use spi_transaction_ext_t for per-transaction config
    memset(&t, 0, sizeof(t)); // Clear all fields to 0

    // JEDEC ID command has NO address and NO dummy bits
    t.base.cmd = SPI_CMD_JEDEC_ID;
    t.base.length = SPI_JEDEC_DATA_BITS;
    t.base.rxlength = SPI_JEDEC_DATA_BITS;
    // Set flags to enable per-transaction override for address and dummy bits
    t.base.flags = SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_VARIABLE_DUMMY | SPI_TRANS_USE_TXDATA; 
    t.base.tx_data[0] = 0x00; // Dummy byte (or bytes) to clock out
    t.base.tx_data[1] = 0x00;
    t.base.tx_data[2] = 0x00;
    
    // Override device defaults for this specific transaction
    t.address_bits = 0;  // No address phase for JEDEC ID
    t.dummy_bits = 0;    // No dummy bits for JEDEC ID

    uint8_t received_data[3]; // Buffer to hold the 3 received bytes
    t.base.rx_buffer = received_data;

    // Cast to spi_transaction_t* when transmitting
    esp_err_t ret = spi_device_transmit(spi, (spi_transaction_t*)&t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to transmit JEDEC ID command: %s", esp_err_to_name(ret));
        return ret;
    }

    buf[0] = received_data[0]; // Manufacturer ID
    buf[1] = received_data[1]; // Memory Type
    buf[2] = received_data[2]; // Capacity

    ESP_LOGI(TAG, "JEDEC ID: Manufacturer: 0x%02X, Memory Type: 0x%02X, Capacity: 0x%02X",
             buf[0], buf[1], buf[2]);
    return ESP_OK;
}

// Optional: provide flash handle externally
spi_device_handle_t ext_flash_get_handle(void) {
    return spi;
}

esp_err_t ext_flash_write_enable(void) {
    spi_transaction_ext_t t;
    memset(&t, 0, sizeof(t));
    
    // Write Enable command has NO address, NO dummy bits, and NO data
    t.base.cmd = SPI_CMD_WRITE_ENABLE;
    t.base.length = 0;       // No data to transmit
    t.base.rxlength = 0;     // No data to receive
    t.base.flags = SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_VARIABLE_DUMMY;
    
    // Override device defaults for this transaction
    t.address_bits = 0;      // No address phase for Write Enable
    t.dummy_bits = 0;        // No dummy bits for Write Enable

    esp_err_t ret = spi_device_transmit(spi, (spi_transaction_t*)&t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send Write Enable: %s", esp_err_to_name(ret));
    }
    ESP_LOGI(TAG, "Write Enable command sent (0x%02X)", SPI_CMD_WRITE_ENABLE);
    return ret;
}

esp_err_t ext_flash_read_status_register(uint8_t *status_reg_value) {
    spi_transaction_ext_t t;
    memset(&t, 0, sizeof(t));
    
    // Status Register Read command has NO address and NO dummy bits
    t.base.cmd = SPI_CMD_READ_STATUS_REG1;
    t.base.length = 8;       // 1 byte (8 bits) to receive
    t.base.rxlength = 8;     // 1 byte (8 bits) to receive
    t.base.flags = SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_VARIABLE_DUMMY;
    
    // Override device defaults for this transaction
    t.address_bits = 0;      // No address phase for Status Register Read
    t.dummy_bits = 0;        // No dummy bits for Status Register Read

    uint8_t received_status;
    t.base.rx_buffer = &received_status;

    esp_err_t ret = spi_device_transmit(spi, (spi_transaction_t*)&t);
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


/**
 * @brief Reads data from the flash chip at a specified address.
 * Uses the Fast Read (0x0B) command for efficiency.
 * @param address The 24-bit address to start reading from.
 * @param buffer Pointer to the buffer to store the read data.
 * @param size The number of bytes to read.
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t ext_flash_read(uint32_t address, uint8_t *buffer, uint32_t size) {
    if (size == 0) {
        return ESP_OK; // Nothing to read
    }
    if (size > SPI_MAX_TRANSFER_SIZE) {
        ESP_LOGE(TAG, "Read size (%lu) exceeds max transfer size (%d). Consider breaking into smaller reads.", size, SPI_MAX_TRANSFER_SIZE);
        return ESP_ERR_INVALID_SIZE;
    }

    spi_transaction_t t;
    memset(&t, 0, sizeof(t)); // Clear all fields to 0

    t.cmd = SPI_CMD_FAST_READ; // Command: Fast Read (0x0B)
    t.addr = address;          // 24-bit address. address_bits and dummy_bits are now set in dev config.

    t.rx_buffer = buffer;      // Buffer to receive data
    t.rxlength = size * 8;     // Total bits to receive
    t.length = size * 8;       // <--- FIX: Set length equal to rxlength for full-duplex read

    esp_err_t ret = spi_device_transmit(spi, &t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read %lu bytes from 0x%06lX: %s", size, address, esp_err_to_name(ret));
    } else {
        ESP_LOGD(TAG, "Read %lu bytes from 0x%06lX", size, address);
    }
    return ret;
}
esp_err_t ext_flash_write(uint32_t address, const uint8_t *buffer, uint32_t size) {
    if (size == 0) {
        return ESP_OK; // Nothing to write
    }
    if (size > W25Q128JV_PAGE_SIZE) {
        ESP_LOGE(TAG, "Write size (%lu) exceeds page size (%d). Use multiple calls for larger data.", size, W25Q128JV_PAGE_SIZE);
        return ESP_ERR_INVALID_SIZE;
    }
    // Ensure address is page-aligned if strictness is desired, though chip handles misaligned writes within page.
    if (address % W25Q128JV_PAGE_SIZE != 0) {
        ESP_LOGW(TAG, "Write address 0x%06lX is not page-aligned. Data will be written within the page.", address);
    }

    esp_err_t ret;

    // 1. Send Write Enable command
    ret = ext_flash_write_enable();
    if (ret != ESP_OK) {
        return ret;
    }

    // --- FIX: Wait for idle after Write Enable and before Page Program ---
    ret = ext_flash_wait_for_idle(5000);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Flash not idle after Write Enable for write operation.");
        return ret;
    }

    spi_transaction_t t;
    memset(&t, 0, sizeof(t)); // Clear all fields to 0

    t.cmd = SPI_CMD_PAGE_PROGRAM; // Command: Page Program (0x02)
    t.addr = address;             // 24-bit address. address_bits is set in dev config.
    // Dummy bits are 0 for Page Program, correctly handled by dev config default.

    t.tx_buffer = (void *)buffer; // Data to transmit
    t.length = size * 8;          // Total bits to transmit
    t.rxlength = 0;               // No data received

    // No specific flags like SPI_TRANS_USE_TXDATA needed when using tx_buffer
    // unless size is <= 4 bytes and you want to use tx_data directly.
    // For general purpose, tx_buffer is used.

    ret = spi_device_transmit(spi, &t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write %lu bytes to 0x%06lX: %s", size, address, esp_err_to_name(ret));
        return ret;
    } else {
        ESP_LOGD(TAG, "Sent Page Program command for %lu bytes to 0x%06lX", size, address);
    }

    // 2. Wait for the write operation to complete
    ret = ext_flash_wait_for_idle(5000);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Flash not idle after write operation.");
    }
    return ret;
}

esp_err_t ext_flash_erase_sector(uint32_t address) {
    // Ensure the address is sector-aligned
    if (address % W25Q128JV_SECTOR_SIZE != 0) {
        ESP_LOGE(TAG, "Erase address 0x%06lX is not sector-aligned (4KB).", address);
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret;

    // 1. Send Write Enable command
    ret = ext_flash_write_enable();
    if (ret != ESP_OK) {
        return ret;
    }

    // --- FIX: Wait for idle after Write Enable and before Sector Erase ---
    ret = ext_flash_wait_for_idle(5000);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Flash not idle after Write Enable for erase operation.");
        return ret;
    }

    spi_transaction_t t;
    memset(&t, 0, sizeof(t)); // Clear all fields to 0

    t.cmd = SPI_CMD_SECTOR_ERASE; // Command: Sector Erase (0x20)
    t.addr = address;             // 24-bit address of the sector. address_bits is set in dev config.
    // Dummy bits are 0 for Sector Erase, correctly handled by dev config default.

    t.length = 0;    // No data to transmit
    t.rxlength = 0;  // No data to receive

    ret = spi_device_transmit(spi, &t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send Sector Erase command for 0x%06lX: %s", address, esp_err_to_name(ret));
        return ret;
    } else {
        ESP_LOGI(TAG, "Sent Sector Erase command for 0x%06lX", address);
    }

    // 2. Wait for the erase operation to complete
    ret = ext_flash_wait_for_idle(5000);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Flash not idle after sector erase operation.");
    }
    return ret;
}


esp_err_t ext_flash_chip_erase(void) {
    esp_err_t ret;

    // 1. Send Write Enable command
    ret = ext_flash_write_enable();
    if (ret != ESP_OK) {
        return ret;
    }

    // --- FIX: Wait for idle after Write Enable and before Chip Erase ---
    ret = ext_flash_wait_for_idle(5000);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Flash not idle after Write Enable for chip erase operation.");
        return ret;
    }

    spi_transaction_t t;
    memset(&t, 0, sizeof(t)); // Clear all fields to 0

    t.cmd = SPI_CMD_CHIP_ERASE; // Command: Chip Erase (0xC7)
    // No address or dummy bits for Chip Erase.

    t.length = 0;    // No data to transmit
    t.rxlength = 0;  // No data to receive

    ret = spi_device_transmit(spi, &t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send Chip Erase command: %s", esp_err_to_name(ret));
        return ret;
    } else {
        ESP_LOGI(TAG, "Sent Chip Erase command.");
    }

    // 2. Wait for the erase operation to complete
    ret = ext_flash_wait_for_idle(5000);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Flash not idle after chip erase operation.");
    }
    return ret;
}