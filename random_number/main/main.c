#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_random.h"
#include "math.h"
#include <inttypes.h>

#define TAG "Dice Roll"

int dice_roll()
{
    int random = esp_random();
    int positiveNumber = abs(random); //get postive
    int diceNumber = (positiveNumber % 6) + 1; //get 1-6
    return diceNumber;
}

void app_main(void)
{
    while(1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS); //1000 ticks / ms = 1s
        ESP_LOGI(TAG, "Random Number: %d", dice_roll());
    }
}