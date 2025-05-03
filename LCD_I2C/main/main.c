#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"

#define I2C_MASTER_NUM         I2C_NUM_0
#define I2C_MASTER_SDA_IO      21
#define I2C_MASTER_SCL_IO      22
#define I2C_MASTER_CLK_SPEED   100000

#define LCD_ADDR            0x27
#define I2C_TIMEOUT_MS      1000

// Bit mask positions for PCF8574
#define LCD_BACKLIGHT       0x08
#define LCD_ENABLE          0x04
#define LCD_READ_WRITE      0x02
#define LCD_REGISTER_SELECT 0x01

void i2c_master_init(void)
{
    i2c_config_t i2c_conf = {
        .mode               = I2C_MODE_MASTER,
        .sda_io_num         = I2C_MASTER_SDA_IO,
        .scl_io_num         = I2C_MASTER_SCL_IO,
        .sda_pullup_en      = GPIO_PULLUP_ENABLE,
        .scl_pullup_en      = GPIO_PULLUP_ENABLE,
        .master.clk_speed   = I2C_MASTER_CLK_SPEED
    };
    i2c_param_config(I2C_MASTER_NUM, &i2c_conf);
    i2c_driver_install(I2C_MASTER_NUM, i2c_conf.mode, 0, 0, 0);
}
int i2c_addr_scan(void) {
    for (int addr = 0x03; addr < 0x78; addr++) {
        printf("Checking address: %d\n", addr);
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();

        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        
        esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(100));
        i2c_cmd_link_delete(cmd);
        if (ret == ESP_OK) {
            printf("Device found at: %d\n", addr);
            return addr;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    return 0;
}

void lcd_send_nibble(uint8_t nibble, uint8_t control_bits) {
    uint8_t data = (nibble << 4) | control_bits;

    // Send data with enable pulse
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LCD_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, data | LCD_ENABLE, true);  // EN = 1
    i2c_master_write_byte(cmd, data & ~LCD_ENABLE, true); // EN = 0
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);
    vTaskDelay(pdMS_TO_TICKS(2));  // Allow LCD to process
}

void lcd_send_byte(uint8_t byte, uint8_t mode) {
    lcd_send_nibble(byte >> 4, mode | LCD_BACKLIGHT);
    lcd_send_nibble(byte & 0x0F, mode | LCD_BACKLIGHT);
}

void lcd_command(uint8_t cmd) {
    lcd_send_byte(cmd, 0x00);  // RS = 0
}

void lcd_write_char(char c) {
    lcd_send_byte(c, LCD_REGISTER_SELECT);  // RS = 1
}

void lcd_init() {
    vTaskDelay(pdMS_TO_TICKS(50));  // Wait 50ms after power

    // Send 0x30 3 times (soft reset)
    lcd_send_nibble(0x03, LCD_BACKLIGHT);
    vTaskDelay(pdMS_TO_TICKS(10));
    lcd_send_nibble(0x03, LCD_BACKLIGHT);
    vTaskDelay(pdMS_TO_TICKS(10));
    lcd_send_nibble(0x03, LCD_BACKLIGHT);
    vTaskDelay(pdMS_TO_TICKS(10));

    // Set to 4-bit mode
    lcd_send_nibble(0x02, LCD_BACKLIGHT);
    vTaskDelay(pdMS_TO_TICKS(10));

    lcd_command(0x28); // Function Set: 4-bit, 2-line
    lcd_command(0x08); // Display OFF
    lcd_command(0x01); // Clear Display
    vTaskDelay(pdMS_TO_TICKS(2));
    lcd_command(0x06); // Entry Mode Set
    lcd_command(0x0C); // Display ON, cursor off
}

// Set cursor position (0-indexed)
void lcd_set_cursor(uint8_t col, uint8_t row) {
    const uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    lcd_command(0x80 | (col + row_offsets[row]));
}

// Write a null-terminated string
void lcd_write_string(const char *str) {
    while (*str) {
        lcd_write_char(*str++);
    }
}
    

void app_main(void)
{
    i2c_master_init();
    //i2c_addr_scan();
    lcd_init();

    lcd_set_cursor(0, 0);  // Line 1, position 0
    lcd_write_string("Hello");

    lcd_set_cursor(0, 1);  // Line 2, position 0
    lcd_write_string("ESP32");

}