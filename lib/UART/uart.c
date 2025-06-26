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
    if (!data || len == 0) return ESP_OK;  

    int written = uart_write_bytes(UART_PORT_NUM, data, len);
    if (written != (int)len) return ESP_FAIL; //return error if buffer full

    // Block pooling until the data is sent out
    return uart_wait_tx_done(UART_PORT_NUM,  pdMS_TO_TICKS(50));
}

esp_err_t uart_receive_cmd(uint8_t *buffer, size_t buffer_size){

    char sentence[128];
    int index = 0;
    // Read bytes from the UART RX buffer
    int read_len = uart_read_bytes(UART_PORT_NUM, buffer, buffer_size, pdMS_TO_TICKS(50));

    if (read_len < 0) {
    ESP_LOGE(TAG, "Error reading from UART: %d", read_len);
        return ESP_FAIL; 
    } else if (read_len == 0) {
        ESP_LOGD(TAG, "No data received within timeout.");
        return ESP_ERR_TIMEOUT;
    } else {
        //A qiuck and dirty way to parse sentences, TODO: make seperate function and make it cleaner and more readable
        for (int i = 0; i < read_len; i++) {
            char ch = buffer[i];

            if (ch == '$') {
                index = 0; // Start new sentence
            }

            if (index < (int)sizeof(sentence) - 1) {
                sentence[index++] = ch;
            }

            if (ch == '\n' && index >= 2 && sentence[index - 2] == '\r') {
                sentence[index] = '\0';
                ESP_LOGI("main", "L96 Response: %s", sentence);
                index = 0; // Reset for next sentence
            }
        }
        return ESP_OK;
    }
}
