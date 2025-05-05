#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "driver/ledc.h"

// Define GPIOs
#define GPIO_OUTPUT_IO_0    18
#define GPIO_OUTPUT_IO_1    19
#define GPIO_INPUT_IO_0     22
#define GPIO_INPUT_IO_1     23
#define GPIO_OUTPUT_PIN_SEL ((1ULL<<GPIO_OUTPUT_IO_0) | (1ULL<<GPIO_OUTPUT_IO_1))
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1))
#define ESP_INTR_FLAG_DEFAULT 0

#define LEDC_LS_TIMER          LEDC_TIMER_1
#define LEDC_LS_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_LS_CH2_GPIO       4
#define LEDC_LS_CH2_CHANNEL    LEDC_CHANNEL_2

static QueueHandle_t gpio_evt_queue = NULL;

void gpio_setup_output(void)
{
    gpio_config_t io_conf;
    // Output pins configuration
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
}

void gpio_setup_input(void)
{
    gpio_config_t io_conf;
    // Input pins configuration
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
}

void LEDC_setup(void)
{
    // First configure timer
    ledc_timer_config_t timer_conf = {
        .speed_mode       = LEDC_LS_MODE,
        .timer_num        = LEDC_LS_TIMER,
        .duty_resolution  = LEDC_TIMER_13_BIT, 
        .freq_hz          = 5000,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_conf);

    // Then configure channel
    ledc_channel_config_t LEDC_conf = {
        .gpio_num       = LEDC_LS_CH2_GPIO,
        .speed_mode     = LEDC_LS_MODE,
        .channel        = LEDC_LS_CH2_CHANNEL,
        .intr_type      = LEDC_INTR_DISABLE, 
        .timer_sel      = LEDC_LS_TIMER,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&LEDC_conf);

    ledc_fade_func_install(0);
}

// void update_LED_duty(int duty)
// {
//     ledc_set_duty(LEDC_LS_MODE, LEDC_LS_CH2_CHANNEL, duty);
//     ledc_update_duty(LEDC_LS_MODE, LEDC_LS_CH2_CHANNEL);
// }

// void fade_LED(int cnt)
// {
//     if (cnt % 2){
//         //odd -> fade up
//         int duty = 0;
//         while (duty <= 8000)
//         {
//             update_LED_duty(duty);
//             duty += 250;
//             vTaskDelay(pdMS_TO_TICKS(30));
//         }
//     } else {
//         //even -> fade down
//         int duty = 8000;
//         while (duty >= 0)
//         {
//             update_LED_duty(duty);
//             duty -= 250;
//             vTaskDelay(pdMS_TO_TICKS(30));
//         }
//     }
// }

void fade_LED_with_LEDC(int cnt)
{
    if (cnt % 2){
        //odd -> fade up
        ledc_set_fade_with_time(LEDC_LS_MODE, LEDC_LS_CH2_CHANNEL, 4000, 1000);
        ledc_fade_start(LEDC_LS_MODE, LEDC_LS_CH2_CHANNEL, 0);
    } else {
        //even -> fade down
        ledc_set_fade_with_time(LEDC_LS_MODE, LEDC_LS_CH2_CHANNEL, 0, 1000);
        ledc_fade_start(LEDC_LS_MODE, LEDC_LS_CH2_CHANNEL, 0);
    }
}

// ISR handler
static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

// Task that handles button presses
static void gpio_task_example(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {

            //Debounce
            gpio_isr_handler_remove(io_num);
            do
            {
                vTaskDelay(20 / portTICK_PERIOD_MS);
            } while (gpio_get_level(io_num) == 0);
            

            int64_t time_now = esp_timer_get_time();
            printf("GPIO[%ld] intr, val: %d at time: %lld us\n", io_num, gpio_get_level(io_num), time_now);

            if (io_num == GPIO_INPUT_IO_0)
            {
                //Light LED 0.1s
                gpio_set_level(GPIO_OUTPUT_IO_0, 1);
                vTaskDelay(100 / portTICK_PERIOD_MS);
                gpio_set_level(GPIO_OUTPUT_IO_0, 0);
            }

            if (io_num == GPIO_INPUT_IO_1)
            {
                //Light LED 0.1s
                gpio_set_level(GPIO_OUTPUT_IO_1, 1);
                vTaskDelay(100 / portTICK_PERIOD_MS);
                gpio_set_level(GPIO_OUTPUT_IO_1, 0);
            }

            gpio_isr_handler_add(io_num, gpio_isr_handler, (void *)io_num);
        }
    }
}

static void gpio_setup_interrupts_and_task()
{
    // Create queue
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

    // Start gpio task
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

    // Install ISR service and add handlers
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);
    gpio_isr_handler_add(GPIO_INPUT_IO_1, gpio_isr_handler, (void*) GPIO_INPUT_IO_1);
}

void app_main(void)
{
    gpio_setup_output();
    gpio_setup_input();
    gpio_setup_interrupts_and_task();
    LEDC_setup();

    int cnt = 0;
    while (1) {
        printf("cnt: %d\n", cnt++);
        //vTaskDelay(1000 / portTICK_PERIOD_MS);
        fade_LED_with_LEDC(cnt);
    }
}