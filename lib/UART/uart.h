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

//UART SPECIFICATIONS
#define TX_PIN GPIO_NUM_1  
#define RX_PIN GPIO_NUM_3  
#define BUF_SIZE 500
#define BAUD_RATE (9600)
#define UART_PORT_NUM UART_NUM_0 //0 is for printf, if you use uart_write_bytes(), then set it to 1

esp_err_t init_uart();
void uart_send_data(const char* data);
uint8_t uart_receive_data(char* data, int length);
void execute_received_data(char* data, uint8_t len);
void handle_uart_interrupt(void);







#endif  // UART_H