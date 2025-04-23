#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>


#define LED_PIN 2

void app_main(void)
{
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);    
    uint32_t isOn = 0;
    while (1)
    {
        isOn = !isOn;
        gpio_set_level(LED_PIN, isOn);
        printf("LED is: %lu \n", isOn);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        
    }
}

// int led_pins[] = {2, 4, 5, 16}; // Common onboard LED pins for ESP32 dev boards

// void app_main() {
//     for (int i = 0; i < sizeof(led_pins)/sizeof(int); i++) {
//         gpio_reset_pin(led_pins[i]);
//         gpio_set_direction(led_pins[i], GPIO_MODE_OUTPUT);
//         gpio_set_level(led_pins[i], 1);
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//         gpio_set_level(led_pins[i], 0);
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }