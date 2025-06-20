#include "spi_flash.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


static const char *TAG = "SPI_FLASH";
spi_device_handle_t spi;

void hold_wp_setup(void) {

    // Configure WP# as output high
    gpio_config_t wp_conf = {
        .pin_bit_mask = (1ULL << SPI_PIN_NUM_WP),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&wp_conf);
    gpio_set_level(SPI_PIN_NUM_WP, 1);

// Configure HOLD# as output high
    gpio_config_t hold_conf = {
        .pin_bit_mask = (1ULL << SPI_PIN_NUM_HOLD),
        .mode = GPIO_MODE_OUTPUT, 
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&hold_conf);
}

void init_spi_flash(void) {
    spi_bus_config_t buscfg = {
        .mosi_io_num = SPI_PIN_NUM_MOSI,
        .miso_io_num = SPI_PIN_NUM_MISO,
        .sclk_io_num = SPI_PIN_NUM_CLK,
        .quadwp_io_num = -1, // WP is manually set to HIGH
        .quadhd_io_num = -1, // HOLD is manually set to HIGH 
        .max_transfer_sz = SPI_MAX_TRANSFER_SIZE, 
    };

    spi_device_interface_config_t devcfg = {
        .command_bits = 8, 
        .clock_speed_hz = SPI_CLOCK_SPEED,
        .mode = 0,                         
        .spics_io_num = SPI_PIN_NUM_CS,       
        .queue_size = 1,
    };

    hold_wp_setup();

    esp_err_t ret;
    printf("Initializing SPI Flash bus...\n");

    ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    printf("spi_bus_initialize status: %s (%d)\n", esp_err_to_name(ret), ret);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "spi_bus_initialize failed: %s", esp_err_to_name(ret));
        // abort() 
    }

    // Add device to the bus
    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi);
    printf("spi_bus_add_device status: %s (%d)\n", esp_err_to_name(ret), ret);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "spi_bus_add_device failed: %s", esp_err_to_name(ret));
        // abort() 
    }

    ESP_LOGI(TAG, "SPI Flash initialized successfully and device added.");
}

void spi_flash_send_reset(void) {
    spi_transaction_t t = {
        .length = 0, 
        .rxlength = 0, 
        .flags = 0,  
    };
    esp_err_t ret;

    t.cmd = SPI_CMD_ENABLE_RESET; 
    ret = spi_device_transmit(spi, &t);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "Enable Reset failed: %s", esp_err_to_name(ret)); return; }
    ESP_LOGI(TAG, "Sent Enable Reset (0x%02X)", SPI_CMD_ENABLE_RESET);

    t.cmd = SPI_CMD_RESET_DEVICE; 
    ret = spi_device_transmit(spi, &t);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "Reset Device failed: %s", esp_err_to_name(ret)); return; }
    ESP_LOGI(TAG, "Sent Reset Device (0x%02X)", SPI_CMD_RESET_DEVICE);

}

// Read JEDEC ID: returns 3 bytes
esp_err_t spi_flash_read_jedec(uint8_t *buf) {
    spi_flash_send_reset(); // Wake/reset chip before reading

    // Transaction for JEDEC ID command (0x9F) and 3 bytes of data
    spi_transaction_t t = {
        .length = 3 * 8, 
        .rxlength = 3 * 8, 
        .cmd = SPI_CMD_JEDEC_ID, 
        .flags = 0,  
    };
    esp_err_t ret;

    uint8_t rx_data[3]; // Buffer to hold the 3 received bytes
    t.rx_buffer = rx_data;

    ret = spi_device_transmit(spi, &t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "JEDEC read failed: %s", esp_err_to_name(ret));
        return ret;
    }

    buf[0] = rx_data[0]; // Manufacturer ID
    buf[1] = rx_data[1]; // Memory Type
    buf[2] = rx_data[2]; // Capacity

    ESP_LOGI(TAG, "JEDEC ID: %02X %02X %02X", buf[0], buf[1], buf[2]);
    return ESP_OK;
}

// Optional: provide flash handle externally
spi_device_handle_t spi_flash_get_handle(void) {
    return spi;
}
