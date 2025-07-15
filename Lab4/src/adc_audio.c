#include "include/adc_audio.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include <math.h>
#include <stdio.h>

void init_adc() {
    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(0); // ADC0
}

float read_adc_voltage() {
    uint16_t raw = adc_read();
    return (raw / 4095.0f) * 3.3f; // 12 bits, 3.3V ref
}

float calculate_rms(const float *samples, uint32_t num_samples) {
    float sum = 0;
    for (uint16_t i = 0; i < num_samples; i++) {
        sum += samples[i];
    }
    float offset = sum / num_samples;

    float sq_sum = 0;
    for (uint16_t i = 0; i < num_samples; i++) {
        float ac = samples[i] - offset;
        sq_sum += ac * ac;
    }
    return sqrtf(sq_sum / num_samples);
}

float calculate_dbfs(float rms) {
    const float max_rms = (3.3f / 2.0f) / 1.4142f; // ≈ 1.165V
    if (rms < 0.001f) return -100; // Evitar -inf
    return 20 * log10f(rms / max_rms);
}
