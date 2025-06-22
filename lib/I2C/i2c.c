#include "i2c.h"

static const char *TAG = "I2C";

#define I2C_PORT I2C_NUM_0

esp_err_t i2c_init(void) {
    i2c_config_t config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_SCL,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_FREQ_HZ,
    };

    ESP_RETURN_ON_ERROR(i2c_param_config(I2C_PORT, &config), TAG, "Config failed");
    ESP_RETURN_ON_ERROR(i2c_driver_install(I2C_PORT, I2C_MODE_MASTER, 0, 0, 0), TAG, "Driver install failed");

    ESP_LOGI(TAG, "I2C initialized");
    return ESP_OK;
}

esp_err_t i2c_write_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    RETURN_ON_ERROR_I2C(i2c_master_start(cmd), TAG, "Start failed", cmd);
    RETURN_ON_ERROR_I2C(i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true), TAG, "Write addr failed", cmd);
    RETURN_ON_ERROR_I2C(i2c_master_write_byte(cmd, reg_addr, true), TAG, "Write reg failed", cmd);
    RETURN_ON_ERROR_I2C(i2c_master_write_byte(cmd, data, true), TAG, "Write data failed", cmd);
    RETURN_ON_ERROR_I2C(i2c_master_stop(cmd), TAG, "Stop failed", cmd);

    RETURN_ON_ERROR_I2C(i2c_master_cmd_begin(I2C_PORT, cmd, WAIT_TIME), TAG, "Command failed", cmd);
    i2c_cmd_link_delete(cmd);

    return ESP_OK;
}

esp_err_t i2c_read_16bit(uint8_t dev_addr, uint8_t reg_addr, uint16_t *data) {
    uint8_t buffer[2]; //16-bit-> 8*2 bytes

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    // Write register address
    RETURN_ON_ERROR_I2C(i2c_master_start(cmd), TAG, "Start failed", cmd);
    RETURN_ON_ERROR_I2C(i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true), TAG, "Write addr failed", cmd);
    RETURN_ON_ERROR_I2C(i2c_master_write_byte(cmd, reg_addr, true), TAG, "Write reg failed", cmd);

    RETURN_ON_ERROR_I2C(i2c_master_start(cmd), TAG, "Restart failed", cmd);
    RETURN_ON_ERROR_I2C(i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_READ, true), TAG, "Read addr failed", cmd);
    // Read two bytes from the device
    RETURN_ON_ERROR_I2C(i2c_master_read_byte(cmd, &buffer[0], I2C_MASTER_ACK), TAG, "Read byte 1 failed", cmd);
    RETURN_ON_ERROR_I2C(i2c_master_read_byte(cmd, &buffer[1], I2C_MASTER_NACK), TAG, "Read byte 2 failed", cmd);
    
    RETURN_ON_ERROR_I2C(i2c_master_stop(cmd), TAG, "Stop failed", cmd);
    RETURN_ON_ERROR_I2C(i2c_master_cmd_begin(I2C_PORT, cmd, WAIT_TIME), TAG, "Command failed", cmd);

    i2c_cmd_link_delete(cmd);

    *data = combine_bytes(buffer[0], buffer[1]);
    return ESP_OK;
}


esp_err_t i2c_read_8bit(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    RETURN_ON_ERROR_I2C(i2c_master_start(cmd), TAG, "Start failed", cmd);
    RETURN_ON_ERROR_I2C(i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true), TAG, "Write addr failed", cmd);
    RETURN_ON_ERROR_I2C(i2c_master_write_byte(cmd, reg_addr, true), TAG, "Write reg failed", cmd);

    RETURN_ON_ERROR_I2C(i2c_master_start(cmd), TAG, "Restart failed", cmd);
    RETURN_ON_ERROR_I2C(i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_READ, true), TAG, "Read addr failed", cmd);
    RETURN_ON_ERROR_I2C(i2c_master_read_byte(cmd, data, I2C_MASTER_NACK), TAG, "Read byte failed", cmd);

    RETURN_ON_ERROR_I2C(i2c_master_stop(cmd), TAG, "Stop failed", cmd);
    RETURN_ON_ERROR_I2C(i2c_master_cmd_begin(I2C_PORT, cmd, WAIT_TIME), TAG, "Command failed", cmd);

    i2c_cmd_link_delete(cmd);
    return ESP_OK;
}

//Helper functions

uint16_t combine_bytes(uint8_t low, uint8_t high) {
    return ((uint16_t)high << 8) | low;
}

