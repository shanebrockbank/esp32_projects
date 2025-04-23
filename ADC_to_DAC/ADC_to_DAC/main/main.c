#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "driver/adc.h"
#include "driver/dac.h"

//Define Pins
#define ADC_CHANNEL ADC1_CHANNEL_0 //Pin 36
#define DAC_CHANNEL DAC_CHANNEL_1 //Pin 25

#define NUM_SAMPLES 16

void setup(void){
    //ADC
    adc1_config_width(ADC_WIDTH_BIT_12);  // 12-bit resolution
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_11);  // Allow reading up to ~3.3V

    //DAC
    dac_output_enable(DAC_CHANNEL);  // Enable DAC output on GPIO25

    // Just a message to know it's set up
    printf("ADC and DAC initialized.\n");
}

int voltage_average(int samples[], int *i, int *sum, int ADC1_raw)
{
    *sum -= samples[*i];
    *sum += ADC1_raw;
    samples[*i] = ADC1_raw;
    *i = (*i + 1) % NUM_SAMPLES;
    return *sum / NUM_SAMPLES;
}
    


void app_main(void)
{
    setup();
    int sum = 0;
    int ADC1_raw = 0;
    int i = 0;
    int samples[NUM_SAMPLES];
    int ADC1_average = 0;

    for (int j = 0; j < NUM_SAMPLES; j++) {
        samples[j] = 0;
    }

    while (1)
    {
        
        ADC1_raw = adc1_get_raw(ADC_CHANNEL);
        ADC1_average = voltage_average(samples, &i, &sum, ADC1_raw);

        float ADC1_volts = ((float)ADC1_average/4095.0) * 3.3;

        printf("Raw: %4d | Avg: %4d | Index: %2d | Volts: %.2f\n", ADC1_raw, ADC1_average, i, ADC1_volts);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    //main loop
    //read in values from ADC
    //Perform voltage divider calc with known resitor 

}