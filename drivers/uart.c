#include <uart.h>

static const char *TAG = "UART";
static bool uart_initialized = false;

esp_err_t uart_init(void)
{
    if (uart_initialized) {
        ESP_LOGW(TAG, "UART is already initialized");
        return ESP_OK;
    }

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

    uart_initialized = true;
    return ESP_OK;
}


esp_err_t uart_send_cmd(const void *nmea_sentence, size_t len){ 

    int written = uart_write_bytes(UART_PORT_NUM, nmea_sentence, len);
    if (written != (int)len) return ESP_FAIL; //return error if buffer full
    // Block pooling until the data is sent out
    return uart_wait_tx_done(UART_PORT_NUM,  pdMS_TO_TICKS(UART_TX_WAIT_TIME_MS));
}

esp_err_t uart_receive_cmd(uint8_t *buffer, size_t buffer_size, size_t *out_read_len) {

    // Read bytes from the UART RX buffer
    int read_len = uart_read_bytes(UART_PORT_NUM, buffer, buffer_size, pdMS_TO_TICKS(UART_RX_WAIT_TIME_MS)); 

    if (read_len < 0) {
        ESP_RETURN_ON_ERROR(read_len, TAG, "Error reading from UART");
    } 
    if (read_len == 0) {
        // No data received within timeout.
        return ESP_ERR_TIMEOUT;
    } 
    *out_read_len = read_len; // Return the number of bytes read to be used in parsing
    return ESP_OK;
}

