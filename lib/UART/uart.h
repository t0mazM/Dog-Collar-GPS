#ifndef UART_H
#define UART_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include <string.h>
#include <errno.h>
#include <sys/unistd.h> 
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_log.h"

//UART SPECIFICATIONS
#define UART_TX_PIN GPIO_NUM_1  
#define UART_RX_PIN GPIO_NUM_3  
#define UART_RX_BUF_SIZE 500
#define UART_BAUD_RATE (9600)
#define UART_PORT_NUM UART_NUM_0 

esp_err_t uart_init(void);
esp_err_t uart_send_cmd(const void *data, size_t len);
esp_err_t uart_receive_cmd(uint8_t *buffer, size_t buffer_size);
esp_err_t parse_uart_data(const uint8_t *buffer, size_t read_len);





#endif  // UART_H