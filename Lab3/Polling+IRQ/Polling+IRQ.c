/**
 * @file rpm_polling_interrupt.c
 * @brief Medición de RPM usando polling + interrupciones en Raspberry Pi Pico.
 */

#include "pico/stdlib.h"
#include <stdio.h>

#define ENCODER_PIN 28
#define SAMPLE_TIME_MS 1000

volatile int pulse_count = 0;

/**
 * @brief Manejador de interrupción del encoder.
 */
void encoder_callback(uint gpio, uint32_t events) {
    if (gpio == ENCODER_PIN && events & GPIO_IRQ_EDGE_RISE) {
        pulse_count++;
    }
}

/**
 * @brief Función principal.
 */
int main() {
    stdio_init_all();
    gpio_init(ENCODER_PIN);
    gpio_set_dir(ENCODER_PIN, GPIO_IN);
    gpio_set_irq_enabled_with_callback(ENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &encoder_callback);

    while (true) {
        int previous_count = pulse_count;
        sleep_ms(SAMPLE_TIME_MS);
        int current_count = pulse_count;
        int delta = current_count - previous_count;
        float rpm = delta * 60.0f;
        printf("Pulsos: %d, RPM (polling + interrupt): %.2f\n", delta, rpm);
    }
}