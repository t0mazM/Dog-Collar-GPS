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



void uart_send_data(const char* data) {
    /*
    Dummy implemenation->use printf insted. 
    If you want to use it, cahnge UART_PORT_NUM to: UART_NUM_1
    */
    uart_write_bytes(UART_PORT_NUM, data, strlen(data));
}

uint8_t uart_receive_data(char* data, int length) {
    /*
    This function reads data from the UART. 
    The 'data' parameter is a pointer to a buffer where the received data will be stored, 
    and 'length' is the number of bytes to read.
    */
    uint8_t len = uart_read_bytes(UART_PORT_NUM, (uint8_t*)data, length, 10 / portTICK_PERIOD_MS);
    data[len] = '\0';
    return len;
}

