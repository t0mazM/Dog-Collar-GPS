#include <uart.h>

static const char *TAG = "UART";


esp_err_t uart_init(void)
{
    const uart_config_t cfg = {
        .baud_rate  = UART_BAUD_RATE,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
    };

    ESP_RETURN_ON_ERROR(uart_driver_install(UART_PORT_NUM, UART_RX_BUF_SIZE, 0, 0, NULL, 0), TAG, "driver install fail");
    ESP_RETURN_ON_ERROR(uart_param_config(UART_PORT_NUM, &cfg), TAG, "config fail");
    ESP_RETURN_ON_ERROR(uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE), TAG, "pin map fail");
    ESP_LOGI(TAG, "UART%d ready @ %d bps", UART_PORT_NUM, UART_BAUD_RATE);
    return ESP_OK;
}


esp_err_t uart_send_cmd(const void *data, size_t len){ 

    int written = uart_write_bytes(UART_PORT_NUM, data, len);
    if (written != (int)len) return ESP_FAIL; //return error if buffer full
    // Block pooling until the data is sent out
    return uart_wait_tx_done(UART_PORT_NUM,  pdMS_TO_TICKS(50));
}

esp_err_t uart_receive_cmd(uint8_t *buffer, size_t buffer_size){

    // Read bytes from the UART RX buffer
    int read_len = uart_read_bytes(UART_PORT_NUM, buffer, buffer_size, pdMS_TO_TICKS(50));

    if (read_len < 0) {
        ESP_LOGI(TAG, "Error reading from UART: %d", read_len);
        return ESP_FAIL; 
    } 
    if (read_len == 0) {
        ESP_LOGI(TAG, "No data received within timeout.");
        return ESP_ERR_TIMEOUT;
    } 
    printf("Read %d bytes from UART\n", read_len);
    // No error and data received, we can parse it
    ESP_RETURN_ON_ERROR(parse_uart_data(buffer, read_len), TAG, "parse data fail");
    return ESP_OK;
}

esp_err_t parse_uart_data(const uint8_t *buffer, size_t read_len) {
    printf("Parsing UART data: %.*s\n", (int)read_len, buffer);
    char NMEA_sentence[255]; // Buffer to hold the sentence/command
    int sentence_idx = 0;

    for (int i = 0; i < read_len; i++) {
        char ch = buffer[i];

        if (ch == '$') {
            sentence_idx = 0; // Start new sentence
        }

        if (sentence_idx < (int)sizeof(NMEA_sentence) - 1) {
            NMEA_sentence[sentence_idx++] = ch;
        }

        if (ch == '\n' && sentence_idx >= 2 && NMEA_sentence[sentence_idx - 2] == '\r') {
            NMEA_sentence[sentence_idx] = '\0';
            ESP_LOGI("main", "L96 Response: %s", NMEA_sentence);
            sentence_idx = 0; // Reset for next sentence
        }
    }
    return ESP_OK;
    
}