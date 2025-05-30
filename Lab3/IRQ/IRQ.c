#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/time.h"

#define ENCODER_PIN 28      // Cambia al pin que uses
#define PULSOS_POR_VUELTA 20

volatile uint32_t pulsos = 0;

void encoder_isr(uint gpio, uint32_t events) {
    pulsos++;
}

int main() {
    stdio_init_all();

    gpio_init(ENCODER_PIN);
    gpio_set_dir(ENCODER_PIN, GPIO_IN);
    gpio_pull_up(ENCODER_PIN);
    gpio_set_irq_enabled_with_callback(ENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &encoder_isr);

    while (true) {
        uint32_t pulsos_inicio = pulsos;
        absolute_time_t start = get_absolute_time();

        // Espera activa durante 1 segundo (1000 ms)
        while (absolute_time_diff_us(start, get_absolute_time()) < 1000000) {
            tight_loop_contents(); // Opcional, para indicar espera activa
        }

        uint32_t pulsos_fin = pulsos;
        uint32_t pulsos_en_intervalo = pulsos_fin - pulsos_inicio;

        float rpm = (pulsos_en_intervalo * 60.0f) / PULSOS_POR_VUELTA;
        printf("Pulsos en el intervalo: %u, RPM: %.2f\n", pulsos_en_intervalo, rpm);
    }
}
