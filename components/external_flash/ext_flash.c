#include "ext_flash.h" // Include the header with definitions

static const char *TAG = "EXT_FLASH";
spi_device_handle_t spi;


spi_bus_config_t get_spi_bus_config(void) {
    spi_bus_config_t spi_bus_config;
    memset(&spi_bus_config, 0, sizeof(spi_bus_config)); // Clear all fields to 0

    spi_bus_config.mosi_io_num = SPI_PIN_MOSI;
    spi_bus_config.miso_io_num = SPI_PIN_MISO;
    spi_bus_config.sclk_io_num = SPI_PIN_CLK;
    spi_bus_config.quadwp_io_num = -1; // Not using Quad Write Protect (IO2)
    spi_bus_config.quadhd_io_num = -1; // Not using Quad Hold (IO3)
    spi_bus_config.max_transfer_sz = SPI_MAX_TRANSFER_SIZE; // Max size for a single transaction

    return spi_bus_config;
}

spi_device_interface_config_t get_spi_device_config(void) {
    spi_device_interface_config_t devcfg;
    memset(&devcfg, 0, sizeof(devcfg)); 

    devcfg.command_bits = 8; // All commands are 8-bit
    devcfg.address_bits = 0;
    devcfg.dummy_bits = 0;
    devcfg.clock_speed_hz = SPI_CLOCK_SPEED; 
    devcfg.mode = 0; 
    devcfg.spics_io_num = SPI_PIN_CS; 
    devcfg.queue_size = 1; // Only one transaction at a time

    return devcfg;
}

esp_err_t ext_flash_init(void) {

    // Initialize GPIO expander so you can control HOLD and WP pins
    ESP_RETURN_ON_ERROR(gpio_init(), 
                        TAG, "Failed to initialize GPIO"
    );

    SPI_set_HOLD_WP_HIGH(); 
    
    spi_bus_config_t buscfg = get_spi_bus_config();
    spi_device_interface_config_t devcfg = get_spi_device_config();

    // Initialize the SPI bus
    ESP_RETURN_ON_ERROR(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO),
                        TAG, "Failed to initialize SPI bus");

    // Add the flash device to the bus
    ESP_RETURN_ON_ERROR(spi_bus_add_device(SPI2_HOST, &devcfg, &spi),
                        TAG, "Failed to add SPI device");

    // Reset the external flash chip
    ESP_RETURN_ON_ERROR(ext_flash_reset_chip(),
                        TAG, "Failed to reset external flash chip");

    // Unlock the global block
    ESP_RETURN_ON_ERROR(ext_flash_global_block_unlock(),
                        TAG, "Failed to unlock global block");

    ESP_LOGI(TAG, "External flash initialized successfully");
    return ESP_OK;
}

esp_err_t ext_flash_reset_chip(void) {

    /* Initialize the transaction structure */
    spi_transaction_ext_t t = {0}; 
    t.base.length = 0;      
    t.base.rxlength = 0;    
    t.base.flags = SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_VARIABLE_DUMMY; 
    t.address_bits = 0;     
    t.dummy_bits = 0;       
    t.base.cmd = SPI_CMD_ENABLE_RESET;

    // Send the Enable Reset command
    esp_err_t ret = spi_device_transmit(spi, (spi_transaction_t*)&t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send Enable Reset (0x%02X) command: %s", SPI_CMD_ENABLE_RESET, esp_err_to_name(ret));
        return ret;
    }

    /* Reset the device by sending the Reset Device command */
    t.base.cmd = SPI_CMD_RESET_DEVICE;
    ret = spi_device_transmit(spi, (spi_transaction_t*)&t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send Reset Device (0x%02X) command: %s", SPI_CMD_RESET_DEVICE, esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

esp_err_t ext_flash_read_jedec_data(uint8_t *buf) {
    spi_transaction_ext_t t = {0}; 

    t.base.cmd = SPI_CMD_JEDEC_ID;
    t.base.rxlength = SPI_JEDEC_DATA_BITS; 
    t.base.length = SPI_JEDEC_DATA_BITS; 
    
    t.base.flags = SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_VARIABLE_DUMMY; 
    t.address_bits = 0;    
    t.dummy_bits = 0;      

    uint8_t received_data[3]; // Buffer to hold the 3 received bytes
    t.base.rx_buffer = received_data;

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


spi_device_handle_t ext_flash_get_handle(void) {
    return spi;
}

esp_err_t ext_flash_write_enable(void) {
    spi_transaction_ext_t t = {0}; 
    
    t.base.cmd = SPI_CMD_WRITE_ENABLE;
    t.base.length = 0;      
    t.base.rxlength = 0;    

    t.base.flags = SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_VARIABLE_DUMMY; 
    t.address_bits = 0;      
    t.dummy_bits = 0;        

    esp_err_t ret = spi_device_transmit(spi, (spi_transaction_t*)&t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send Write Enable (0x%02X) command: %s", SPI_CMD_WRITE_ENABLE, esp_err_to_name(ret));
    }
    ESP_LOGI(TAG, "Write Enable command sent (0x%02X)", SPI_CMD_WRITE_ENABLE);
    return ret;
}

esp_err_t ext_flash_read_status_register(uint8_t *status_reg_value) {
    spi_transaction_ext_t t = {0}; 
    
    t.base.cmd = SPI_CMD_READ_STATUS_REG1;
    t.base.length = 8;       
    t.base.rxlength = 8;     

    t.base.flags = SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_VARIABLE_DUMMY; 
    t.address_bits = 0;
    t.dummy_bits = 0;       

    uint8_t received_status;
    t.base.rx_buffer = &received_status;

    esp_err_t ret = spi_device_transmit(spi, (spi_transaction_t*)&t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read Status Register-1 (0x%02X): %s", SPI_CMD_READ_STATUS_REG1, esp_err_to_name(ret));
    } else {
        *status_reg_value = received_status;
        ESP_LOGI(TAG, "Read Status Register-1: 0x%02X", *status_reg_value);
    }
    return ret;
}

esp_err_t ext_flash_wait_for_idle(int timeout_ms) {
    uint8_t status_reg;
    TickType_t start_time = xTaskGetTickCount(); // Start time for timeout calculation

    do {
        esp_err_t ret = ext_flash_read_status_register(&status_reg);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read status register while waiting for flash operation to complete: %s", esp_err_to_name(ret));
            return ret; // Return the error
        }

        // Check the BUSY bit (LSB of status register, S0). It's 0 when idle.
        if (!(status_reg & (1 << 0))) { 
            ESP_LOGI(TAG, "Flash is not busy. Status: 0x%02X", status_reg);
            return ESP_OK;
        }

        vTaskDelay(pdMS_TO_TICKS(1)); // Wait 1ms before polling again

        // Check for timeout
        if ((xTaskGetTickCount() - start_time) * portTICK_PERIOD_MS > timeout_ms) {
            ESP_LOGE(TAG, "Timeout waiting for flash operation to complete. Final Status: 0x%02X", status_reg);
            return ESP_ERR_TIMEOUT;
        }
    } while (true); 
}
esp_err_t ext_flash_read(uint32_t address, uint8_t *buffer, uint32_t size) {
    if (size == 0) {
        return ESP_OK; // Nothing to read
    }
    if (size > SPI_MAX_TRANSFER_SIZE) {
        ESP_LOGE(TAG, "Read size (%lu) exceeds max transfer size (%d). Consider breaking into smaller reads.", size, SPI_MAX_TRANSFER_SIZE);
        return ESP_ERR_INVALID_SIZE;
    }

    spi_transaction_ext_t t = {0}; 

    t.base.cmd = SPI_CMD_FAST_READ; 
    t.base.addr = address;          

    t.base.rx_buffer = buffer;      // Buffer to receive data
    t.base.rxlength = size * 8;     // Total bits to receive
    t.base.length = size * 8;       // Set length equal to rxlength for full-duplex read

    t.base.flags = SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_VARIABLE_DUMMY;
    t.address_bits = 24; 
    t.dummy_bits = 8;    

    esp_err_t ret = spi_device_transmit(spi, (spi_transaction_t*)&t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read %lu bytes from 0x%06lX: %s", size, address, esp_err_to_name(ret));
    } else {
        ESP_LOGD(TAG, "Read %lu bytes from 0x%06lX", size, address);
    }
    return ret;
}
esp_err_t ext_flash_write(uint32_t address, const uint8_t *buffer, uint32_t size) {
    // Ensure write size does not exceed page size
    if (size == 0 || size > W25Q128JV_PAGE_SIZE) {
        ESP_LOGE(TAG, "Invalid write size: %lu bytes. Must be 1 to %d bytes.", size, W25Q128JV_PAGE_SIZE);
        return ESP_ERR_INVALID_SIZE;
    }

    // 1. Send Write Enable command
    esp_err_t ret = ext_flash_write_enable();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable write for address 0x%06lX", address);
        return ret;
    }
    ret = ext_flash_wait_for_idle(5000); 
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Flash not idle after Write Enable for write operation to 0x%06lX.", address);
        return ret;
    }

    spi_transaction_ext_t t = {0}; 
    t.base.cmd = SPI_CMD_PAGE_PROGRAM; 
    t.base.addr = address;             // 24-bit address for the write operation
    t.base.tx_buffer = (void*)buffer;  // Buffer containing data to transmit
    t.base.length = size * 8;          // Total bits to transmit

    t.base.flags = SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_VARIABLE_DUMMY;
    t.address_bits = 24; 
    t.dummy_bits = 0;    

    ret = spi_device_transmit(spi, (spi_transaction_t*)&t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send write command for 0x%06lX: %s", address, esp_err_to_name(ret));
        return ret;
    } else {
        ESP_LOGI(TAG, "Sent write command for 0x%06lX", address);
    }

    // 2. Wait for the program operation to complete
    ret = ext_flash_wait_for_idle(5000); // Increased timeout for write (tPP max 3ms)
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Flash not idle after write operation to 0x%06lX", address);
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
        ESP_LOGE(TAG, "Failed to enable write for sector erase at 0x%06lX", address);
        return ret;
    }

    ret = ext_flash_wait_for_idle(5000); 
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Flash not idle after Write Enable for erase operation at 0x%06lX.", address);
        return ret;
    }

    spi_transaction_ext_t t = {0}; 

    t.base.cmd = SPI_CMD_SECTOR_ERASE; // Command: Sector Erase (0x20)
    t.base.addr = address;             // 24-bit address of the sector to erase

    t.base.length = 0;     
    t.base.rxlength = 0;   

    t.base.flags = SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_VARIABLE_DUMMY;
    t.address_bits = 24; 
    t.dummy_bits = 0;    

    ret = spi_device_transmit(spi, (spi_transaction_t*)&t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send Sector Erase command for 0x%06lX: %s", address, esp_err_to_name(ret));
        return ret;
    } else {
        ESP_LOGI(TAG, "Sent Sector Erase command for 0x%06lX", address);
    }

    ret = ext_flash_wait_for_idle(5000); 
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Flash not idle after sector erase operation at 0x%06lX.", address);
    }
    return ret;
}


esp_err_t ext_flash_chip_erase(void) {
    esp_err_t ret;

    // 1. Send Write Enable command
    ret = ext_flash_write_enable();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable write for chip erase.");
        return ret;
    }

    ret = ext_flash_wait_for_idle(5000); 
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Flash not idle after Write Enable for chip erase operation.");
        return ret;
    }

    spi_transaction_ext_t t = {0}; 

    t.base.cmd = SPI_CMD_CHIP_ERASE; 
    t.base.length = 0;      
    t.base.rxlength = 0;    

    t.base.flags = SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_VARIABLE_DUMMY;
    t.address_bits = 0; 
    t.dummy_bits = 0;  

    ret = spi_device_transmit(spi, (spi_transaction_t*)&t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send Chip Erase command (0x%02X): %s", SPI_CMD_CHIP_ERASE, esp_err_to_name(ret));
        return ret;
    } else {
        ESP_LOGI(TAG, "Sent Chip Erase command (0x%02X).", SPI_CMD_CHIP_ERASE);
    }

    // 2. Wait for the erase operation to complete
    ret = ext_flash_wait_for_idle(200000); // 200 seconds timeout
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Flash not idle after chip erase operation.");
    }
    return ret;
}

esp_err_t ext_flash_global_block_unlock(void) {
    esp_err_t ret;

    // Must send Write Enable before Global Block Unlock
    ret = ext_flash_write_enable();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable write for global block unlock.");
        return ret;
    }
    // Wait for WEL to be set and flash to be ready
    ret = ext_flash_wait_for_idle(5000); 
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Flash not idle after Write Enable for global block unlock.");
        return ret;
    }

    spi_transaction_ext_t t = {0}; 
    t.base.cmd = SPI_CMD_GLOBAL_BLOCK_UNLOCK; 

    t.base.length = 0;      // No data to transmit
    t.base.rxlength = 0;    // No data to receive

    t.base.flags = SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_VARIABLE_DUMMY;
    t.address_bits = 0; 
    t.dummy_bits = 0;   

    ret = spi_device_transmit(spi, (spi_transaction_t*)&t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send Global Block Unlock (0x%02X) command: %s", SPI_CMD_GLOBAL_BLOCK_UNLOCK, esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "Global Block Unlock (0x%02X) sent.", SPI_CMD_GLOBAL_BLOCK_UNLOCK);

    // Wait for the unlock operation to complete (it's usually very fast, but good practice)
    ret = ext_flash_wait_for_idle(5000); 
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Flash not idle after global block unlock operation.");
    }
    return ret;
}
void ext_flash_complete_test(void) {

    uint32_t test_address = 0x000000; 
    uint8_t write_data[W25Q128JV_PAGE_SIZE]; 
    uint8_t read_buffer[W25Q128JV_PAGE_SIZE]; 

    // Fill the write_data buffer with some data 
    for (int i = 0; i < W25Q128JV_PAGE_SIZE; i++) {
        write_data[i] =  i; 
    }

    //1. Erase the target sector
    ESP_LOGI(TAG, "Erasing sector at 0x%06lX...", test_address);
    if (ext_flash_erase_sector(test_address) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to erase sector at 0x%06lX. Aborting test.", test_address);
        return;
    }
    ESP_LOGI(TAG, "Sector at 0x%06lX erased successfully.", test_address);

    // 2. Write data to the flash
    ESP_LOGI(TAG, "Writing %d bytes to 0x%06lX...", W25Q128JV_PAGE_SIZE, test_address);
    if (ext_flash_write(test_address, write_data, W25Q128JV_PAGE_SIZE) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write data to 0x%06lX. Aborting test.", test_address);
        return;
    }
    ESP_LOGI(TAG, "Data written successfully to 0x%06lX.", test_address);

    // 3. Read data back from the flash
    ESP_LOGI(TAG, "Reading %d bytes from 0x%06lX...", W25Q128JV_PAGE_SIZE, test_address);
    memset(read_buffer, 0, sizeof(read_buffer)); // Clear the read buffer to ensure no old data interferes
    if (ext_flash_read(test_address, read_buffer, W25Q128JV_PAGE_SIZE) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read data from 0x%06lX. Aborting test.", test_address);
        return;
    }
    ESP_LOGI(TAG, "Data read successfully from 0x%06lX.", test_address);
    
    // Print write buffer
    ESP_LOGI(TAG, "Write Buffer:");
    for (int i = 0; i < 32; i += 16) {
        char hex_line[64] = {0};
        char *ptr = hex_line;
        for (int j = 0; j < 16 && (i + j) < 32; j++) {
            ptr += sprintf(ptr, "%02X ", write_data[i + j]);
        }
        ESP_LOGI(TAG, "[%02d-%02d]: %s", i, i + 15, hex_line);
    }
    
    // Print read buffer
    ESP_LOGI(TAG, "Read Buffer:");
    for (int i = 0; i < 32; i += 16) {
        char hex_line[64] = {0};
        char *ptr = hex_line;
        for (int j = 0; j < 16 && (i + j) < 32; j++) {
            ptr += sprintf(ptr, "%02X ", read_buffer[i + j]);
        }
        ESP_LOGI(TAG, "[%02d-%02d]: %s", i, i + 15, hex_line);
    }
}