#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "driver/adc.h"
#include "driver/dac.h"

// Define ADC and DAC pins
#define ADC_CHANNEL ADC1_CHANNEL_0  // GPIO36
#define DAC_CHANNEL DAC_CHANNEL_1   // GPIO25

#define NUM_SAMPLES 4  // Number of samples to average for smoothing

// Function to initialize ADC and DAC peripherals
void setup(void) {
    // Configure ADC: 12-bit resolution, attenuation for 0–3.3V range
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_11);

    // Enable DAC output
    dac_output_enable(DAC_CHANNEL);

    printf("ADC and DAC initialized.\n");
}

// Moving average filter for noise reduction
int voltage_average(int samples[], int *i, int *sum, int ADC1_raw) {
    *sum -= samples[*i];
    *sum += ADC1_raw;
    samples[*i] = ADC1_raw;
    *i = (*i + 1) % NUM_SAMPLES;
    return *sum / NUM_SAMPLES;
}

// Convert raw ADC reading to voltage (in volts)
float calc_input_volts(int ADC1_average) {
    return ((float)ADC1_average / 4095.0f) * 3.3f;
}

// Convert voltage to DAC output value (0–255), with basic clamping
int calc_output_volts(float ADC1_volts) {
    int DAC_out = (int)((ADC1_volts / 3.3f) * 255.0f);

    // Manual offset adjustment based on expected lighting conditions
    DAC_out += 100;

    // Clamp DAC output to valid 8-bit range
    if (DAC_out > 255) DAC_out = 255;
    else if (DAC_out < 0) DAC_out = 0;

    return DAC_out;
}

void app_main(void) {
    setup();

    // State variables
    int sum = 0, i = 0;
    int ADC1_raw = 0, ADC1_average = 0;
    float ADC1_volts = 0.0f;
    int DAC_out = 0;
    int samples[NUM_SAMPLES] = {0};

    while (1) {
        // Read raw ADC value
        ADC1_raw = adc1_get_raw(ADC_CHANNEL);

        // Smooth with moving average
        ADC1_average = voltage_average(samples, &i, &sum, ADC1_raw);

        // Convert to volts
        ADC1_volts = calc_input_volts(ADC1_average);

        // Map input voltage to DAC output
        DAC_out = calc_output_volts(ADC1_volts);

        // Output to DAC
        dac_output_voltage(DAC_CHANNEL, DAC_out);

        // Debug info
        printf("Raw: %4d | Avg: %4d | Index: %2d | ADC_Volts: %.2f | DAC_Out: %4d\n",
               ADC1_raw, ADC1_average, i, ADC1_volts, DAC_out);

        // Delay for 100 ms
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
