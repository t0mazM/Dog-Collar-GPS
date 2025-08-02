/*
 * Copyright © 2025 Tomaz Miklavcic
 *
 * Use this code for whatever you want. No restrictions, no warranty.
 * Attribution appreciated but not required.
 */

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
#define UART_TX_PIN GPIO_NUM_5  
#define UART_RX_PIN GPIO_NUM_4  
#define UART_RX_BUF_SIZE 1024
#define UART_BAUD_RATE (9600)
#define UART_PORT_NUM UART_NUM_0 
#define UART_RX_WAIT_TIME_MS 100
#define UART_TX_WAIT_TIME_MS 100




/**
 * @brief Initializes UART peripheral.
 * 
 * This function configures the UART.
 *
 * @note If it was already initialized, it will not reinitialize it.
 *
 * @return
 * - ESP_OK on success
 * - ESP_FAIL if initialization fails.
 */
esp_err_t uart_init(void);

/**
 * @brief Sends a command (nmea sentence) via UART.
 *
 * @param nmea_sentence Pointer to the NMEA sentence to be sent.
 * @param len Length of the NMEA sentence to be sent.
 * @return ESP_OK on success, ESP_FAIL if the buffer is full or transmission times out.
 */
esp_err_t uart_send_cmd(const void *nmea_sentence, size_t len);

/**
 * @brief Receives data from UART and stores raw it in the provided buffer.
 *
 * @param buffer Pointer to the buffer where received raw data will be stored.
 * @param buffer_size Size of the buffer.
 * @param out_read_len Pointer to a variable that will hold the number of bytes read.
 * @return ESP_OK on success, ESP_ERR_TIMEOUT if no data is received within the timeout period, or an error code if reading fails.
 */
esp_err_t uart_receive_cmd(uint8_t *buffer, size_t buffer_size, size_t *out_read_len);






#endif  // UART_H