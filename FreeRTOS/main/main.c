#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void task1()
{
    while (1)
    {
        printf("Read temp\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void task2()
{
    while (1)
    {
        printf("Read temp\n");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void app_main(void)
{
    task1();
    task2();
}