#include "spi_flash.h"


static const char *TAG = "SPI_FLASH";
spi_device_handle_t spi;



void hold_wp_setup(void) {
    ESP_LOGI(TAG, "Configuring WP# and HOLD# pins...");

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PIN_NUM_WP) | (1ULL << PIN_NUM_HOLD),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    gpio_config(&io_conf);

    gpio_set_level(PIN_NUM_WP, 1);   // Set WP# HIGH (disabled)
    gpio_set_level(PIN_NUM_HOLD, 1); // Set HOLD# HIGH (disabled)

    ESP_LOGI(TAG, "WP# and HOLD# configured as outputs and set HIGH.");
}


void init_spi_flash(void) {
    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_NUM_MOSI,    // DI
        .miso_io_num = PIN_NUM_MISO,    // DO
        .sclk_io_num = PIN_NUM_CLK,     // CLK
        .quadwp_io_num = -1,            // Ignore WP
        .quadhd_io_num = -1,            // Ignore HOLD
        .max_transfer_sz = 4096

    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 8 * 1000 * 1000, // 8 MHz
        .mode = 0,                         // SPI mode 0
        .spics_io_num = PIN_NUM_CS,        // CS pin
        .queue_size = 7,
    };

    hold_wp_setup();

    esp_err_t ret;
    printf("Initializing SPI Flash...\n");

    ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    printf("INIT IS:%d\n", ret);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "spi_bus_initialize failed: %s", esp_err_to_name(ret));
        //abort();
    }

    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "spi_bus_add_device failed: %s", esp_err_to_name(ret));
        //abort();
    }

    ESP_LOGI(TAG, "SPI Flash initialized successfully.");
}


// Reset flash chip: 0x66 then 0x99
void spi_flash_send_reset(void) {
    spi_transaction_t t = {
        .length = 8,
        .flags = SPI_TRANS_USE_TXDATA,
    };

    t.tx_data[0] = CMD_ENABLE_RESET;
    spi_device_transmit(spi, &t);
    ESP_LOGI(TAG, "Sent Enable Reset (0x66)");

    t.tx_data[0] = CMD_RESET_DEVICE;
    spi_device_transmit(spi, &t);
    ESP_LOGI(TAG, "Sent Reset Device (0x99)");

    vTaskDelay(pdMS_TO_TICKS(3)); // wait for chip to reset
}

// Read JEDEC ID: returns 3 bytes
esp_err_t spi_flash_read_jedec(uint8_t *buf) {
    spi_flash_send_reset(); // wake/reset chip before reading

    spi_transaction_t t = {
        .length = 32,  // 8 * 4 bits (1 command + 3 bytes read)
        .rxlength = 32,
        .flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA,
    };

    t.tx_data[0] = CMD_JEDEC_ID;

    esp_err_t ret = spi_device_transmit(spi, &t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "JEDEC read failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Skip the first byte which is dummy
    buf[0] = t.rx_data[1]; // Manufacturer ID
    buf[1] = t.rx_data[2]; // Memory Type
    buf[2] = t.rx_data[3]; // Capacity

    ESP_LOGI(TAG, "JEDEC ID: %02X %02X %02X", buf[0], buf[1], buf[2]);
    return ESP_OK;
}

// Optional: provide flash handle externally
spi_device_handle_t spi_flash_get_handle(void) {
    return spi;
}