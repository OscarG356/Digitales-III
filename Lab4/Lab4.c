#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/sync.h"
#include "hardware/adc.h"
#include "pico/time.h"
#include "include/adc_audio.h"


#define ADC_PIN 26       // GPIO26 = ADC0
#define LED_PIN 25       // LED onboard
#define BUTTON_PIN 6     // Botón

volatile bool medir = false;
volatile uint32_t last_press_ms = 0; // Último tiempo válido de pulsación

// Callback de interrupción del botón
void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == BUTTON_PIN && (events & GPIO_IRQ_EDGE_RISE)) {
        uint32_t now = to_ms_since_boot(get_absolute_time());

        if (now - last_press_ms > 200) { // 200 ms antirrebote
            medir = true;
            last_press_ms = now;
        }
    }
}

int main() {
    stdio_init_all();
    init_adc();

    // Inicializar LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, true);

    // Inicializar botón
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_down(BUTTON_PIN);

    // Habilitar interrupción con callback
    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    float samples[NUM_SAMPLES];

    while (true) {
        __wfi(); // Esperar interrupción

        if (medir) {
            medir = false;

            // Tomar muestras de audio
            for (unsigned long i = 0; i < NUM_SAMPLES; i++) {
                samples[i] = read_adc_voltage();
                sleep_us(500);
            }

            float rms = calculate_rms(samples, NUM_SAMPLES);
            float dbfs = calculate_dbfs(rms);

            printf("Nivel de audio: %.2f dBFS\n", dbfs);
        }
    }
}
