#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/sync.h"
#include "pico/time.h"

#define BUTTON_PIN 6
#define LED_PIN 25 // Pin del LED integrado en la Raspberry Pi Pico

volatile bool button_pressed = false;
volatile uint32_t last_interrupt_time = 0; // Tiempo de la última interrupción

// Callback que se llama cuando se genera una interrupción
void gpio_callback(uint gpio, uint32_t events) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    if (gpio == BUTTON_PIN && (events & GPIO_IRQ_EDGE_RISE)) {
        // Antirrebote: Ignorar interrupciones rápidas
        if (current_time - last_interrupt_time > 200) { // 200 ms de margen
            button_pressed = true;
            last_interrupt_time = current_time;
        }
    }
}

int main() {
    stdio_init_all();

    // Inicializar el LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1); // Encender el LED

    // Inicializar el botón
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_down(BUTTON_PIN); // Configurar el botón con pulldown

    // Configurar interrupción por flanco de bajada
    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    while (true) {
        __wfi(); // Espera hasta que ocurra una interrupción

        if (button_pressed) {
            printf("Botón presionado, mensaje enviado.\n");
            button_pressed = false;
        }
    }
}