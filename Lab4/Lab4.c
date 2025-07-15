#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/sync.h"
#include "hardware/adc.h"
#include "include/adc_audio.h"

#define NUM_SAMPLES 2048
#define ADC_PIN 26 // GPIO26 = ADC0
#define LED_PIN 25 // GPIO25 = LED integrado
#define BUTTON_PIN 6 // GPIO6 = Botón


// Callback para manejar la interrupción del botón
void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == BUTTON_PIN && (events & GPIO_IRQ_EDGE_RISE)) {
        // No se necesita lógica adicional aquí, solo despertar el procesador
    }
}

int main() {
    stdio_init_all();
    init_adc();

    // Inicializar el LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Inicializar el botón
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_down(BUTTON_PIN); // Configurar el botón con pulldown

    // Configurar interrupción en el GPIO6
    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    float samples[NUM_SAMPLES];

    while (true) {
        __wfi(); // Reducir consumo mientras espera

        // 1. Tomar muestras
        for (int i = 0; i < NUM_SAMPLES; i++) {
            samples[i] = read_adc_voltage();
            sleep_us(100); // 10 kHz = 100 µs
        }

        // 2. Calcular RMS y dBFS usando funciones de la librería
        float rms = calculate_rms(samples, NUM_SAMPLES);
        float dbfs = calculate_dbfs(rms);

        // 3. Imprimir
        printf("Nivel de audio: %.2f dBFS\n", dbfs);

        // Encender o apagar el LED según el nivel de audio
        if (dbfs > -50) {
            gpio_put(LED_PIN, 1); // Encender el LED
        } else {
            gpio_put(LED_PIN, 0); // Apagar el LED
        }

        sleep_ms(500); // Reducir consumo mientras espera
    }
}
