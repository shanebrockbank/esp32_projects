#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <esp_timer.h>

#define GPIO_OUTPUT_0 18
#define GPIO_OUTPUT_1 19
#define GPIO_OUTPUT_PIN_SEL ((1<<GPIO_OUTPUT_0) | (1<<GPIO_OUTPUT_1))

#define GPIO_INPUT_0 5
#define GPIO_INPUT_1 23
#define GPIO_INPUT_PIN_SEL ((1<<GPIO_INPUT_0) | (1<<GPIO_INPUT_1))

volatile bool BUTTON_1_FLAG = false;

void GPIO_output_setup(void)
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

void GPIO_input_setup(void)
{
    gpio_config_t io_conf;
    //interrupt on falling edge
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    //bit mask of the pins
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode    
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    //set isr to falling edge because pull up resistor 
    //gpio_set_intr_type(GPIO_INPUT_0, GPIO_INTR_NEGEDGE);
}

void GPIO_setup(void)
{
    printf("Setting up GPIO...\n");
    GPIO_output_setup();
    GPIO_input_setup();
    
}

static void IRAM_ATTR isr_1(void* arg)
{
    BUTTON_1_FLAG = true;
}

void ISR_setup(void)
{
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_INPUT_1, isr_1, NULL);
}

void app_main(void)
{
    GPIO_setup();
    ISR_setup();

    while(1)
    {
        if (BUTTON_1_FLAG)
        {
            BUTTON_1_FLAG = false;
            printf("Button Pressed!\n");
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}