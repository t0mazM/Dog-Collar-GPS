#include <i2c.h>

static const char *TAG = "I2C"; // Used for logging

I2CPORT I2CPORT_1 = { 
    .number = 0,
    .sda_io_num = I2C_SDA,
    .scl_io_num = I2C_SCL,
};


esp_err_t init_i2c_port(I2CPORT* port) {
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = port->sda_io_num,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = port->scl_io_num,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_FREQ_HZ,
    };
    ESP_RETURN_ON_ERROR(i2c_param_config(port->number, &i2c_config), TAG, "i2c init fail");
    ESP_RETURN_ON_ERROR(i2c_driver_install(port->number, I2C_MODE_MASTER, 0, 0, 0), TAG, "i2c init fail");
    return ESP_OK;
}

void init_i2c() {
    esp_err_t ret = init_i2c_port(&I2CPORT_1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C init failed: %s", esp_err_to_name(ret));
        abort();  
    }

    ESP_LOGI(TAG, "I2C initialized successfully");
}


esp_err_t write_to_i2c(i2c_port_t i2c_port_num, uint8_t device_addr, uint8_t device_register, uint8_t data){
    /*
    Establishes connection to i2c device and writes data to specified register
    */
 
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    RETURN_ON_ERROR_I2C(i2c_master_start(cmd), TAG, "i2c start fail", cmd);

    //We send ack bit (true), also make room for it with <<1
    RETURN_ON_ERROR_I2C(i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_WRITE, true), TAG, "i2c write byte fail", cmd);

    //We write data to the register
    RETURN_ON_ERROR_I2C(i2c_master_write_byte(cmd, device_register, true), TAG, "i2c write register fail", cmd);
    RETURN_ON_ERROR_I2C(i2c_master_write_byte(cmd, data, true), TAG, "i2c write data fail", cmd);
    RETURN_ON_ERROR_I2C(i2c_master_stop(cmd), TAG, "i2c stop fail", cmd);

    // Execute the I2C command
    RETURN_ON_ERROR_I2C(i2c_master_cmd_begin(i2c_port_num, cmd, WAIT_TIME), TAG, "i2c cmd begin fail", cmd);

    i2c_cmd_link_delete(cmd);
    return ESP_OK;
}

esp_err_t read_from_i2c(i2c_port_t i2c_port_num, uint8_t device_addr, uint8_t device_register, uint8_t* data) {
 
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    // Start i2c to write
    RETURN_ON_ERROR_I2C(i2c_master_start(cmd), TAG, "i2c start fail", cmd);

    // Specify which register to read from
    RETURN_ON_ERROR_I2C(i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_WRITE, true), TAG, "i2c write byte fail", cmd);
    RETURN_ON_ERROR_I2C(i2c_master_write_byte(cmd, device_register, true), TAG, "i2c write register fail", cmd);

    // Start a new i2c  to read
    RETURN_ON_ERROR_I2C(i2c_master_start(cmd), TAG, "i2c start fail", cmd);

    RETURN_ON_ERROR_I2C(i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_READ, true), TAG, "i2c write byte fail", cmd);

    // Read from the device register
    RETURN_ON_ERROR_I2C(i2c_master_read_byte(cmd, data, I2C_MASTER_LAST_NACK), TAG, "i2c read byte fail", cmd);

    i2c_master_stop(cmd);
    RETURN_ON_ERROR_I2C(i2c_master_cmd_begin(i2c_port_num, cmd, WAIT_TIME), TAG, "i2c cmd begin fail", cmd);

    i2c_cmd_link_delete(cmd);
    return ESP_OK;
}


