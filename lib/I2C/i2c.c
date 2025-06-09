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

    init_i2c_port(&I2CPORT_1);
}


esp_err_t write_to_i2c(i2c_port_t i2c_port_num, uint8_t device_addr, uint8_t device_register, uint8_t data){
    /*
    Establishes connection to i2c device and writes data to specified register
    */
 
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_RETURN_ON_ERROR(i2c_master_start(cmd), TAG, "i2c start fail");

    //We send ack bit (true), also make room for it with <<1
    ESP_RETURN_ON_ERROR(i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_WRITE, true), TAG, "i2c write byte fail");

    //We write data to the register 
    ESP_RETURN_ON_ERROR(i2c_master_write_byte(cmd, device_register, true), TAG, "i2c write register fail");
    ESP_RETURN_ON_ERROR(i2c_master_write_byte(cmd, data, true), TAG, "i2c write data fail");
    ESP_RETURN_ON_ERROR(i2c_master_stop(cmd), TAG, "i2c stop fail");

    // Execute the I2C command
    ESP_RETURN_ON_ERROR(i2c_master_cmd_begin(i2c_port_num, cmd, WAIT_TIME), TAG, "i2c cmd begin fail");

    i2c_cmd_link_delete(cmd);
    return ESP_OK;
}

esp_err_t read_from_i2c(i2c_port_t i2c_port_num, uint8_t device_addr, uint8_t device_register, uint8_t* data) {
 
    esp_err_t error = ESP_OK;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    // Start i2c to write
    error = i2c_master_start(cmd);
        if (error != ESP_OK)  {i2c_cmd_link_delete(cmd); return error; }

    // Specify which register to read from
    error = i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_WRITE, true);
        if (error != ESP_OK)  {i2c_cmd_link_delete(cmd); return error; }
    error = i2c_master_write_byte(cmd, device_register, true);
        if (error != ESP_OK)  {i2c_cmd_link_delete(cmd); return error; }

    // Start a new i2c  to read
    error = i2c_master_start(cmd);
        if (error != ESP_OK)  {i2c_cmd_link_delete(cmd); return error; }

    error = i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_READ, true);
        if (error != ESP_OK)  {i2c_cmd_link_delete(cmd); return error; }

    // Read from the device register
    error = i2c_master_read_byte(cmd, data, I2C_MASTER_LAST_NACK);
        if (error != ESP_OK)  {i2c_cmd_link_delete(cmd); return error; }

    i2c_master_stop(cmd);
        if (error != ESP_OK)  {i2c_cmd_link_delete(cmd); return error; }

    // Execute the I2C command
    error = i2c_master_cmd_begin(i2c_port_num, cmd, WAIT_TIME);
        if (error != ESP_OK)  {i2c_cmd_link_delete(cmd); return error; }

    i2c_cmd_link_delete(cmd);
    return ESP_OK;
}


