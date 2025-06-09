#include <uart.h>

static const char *TAG = "UART"; // Used for logging

esp_err_t init_uart() {
    uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    ESP_RETURN_ON_ERROR(uart_param_config(UART_PORT_NUM, &uart_config), TAG, "Failed to configure UART");
    ESP_RETURN_ON_ERROR(uart_set_pin(UART_PORT_NUM, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE), TAG, "Failed to set UART pins");
    ESP_RETURN_ON_ERROR(uart_driver_install(UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, 0), TAG, "Failed to install UART driver");
    return ESP_OK;
}


int _write(int file, const char* data, int len) {
    /*
    _write is re-written so printf redirects the data to uart
    */
    uart_write_bytes(UART_PORT_NUM, data, len);
    return len;
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


void handle_uart_interrupt(void) {

    char data[BUF_SIZE];

    // Receive data from UART
    uint8_t len = uart_receive_data(data, BUF_SIZE);

    
    // Execute command based on received data and print it 
    execute_received_data(data, len);

}