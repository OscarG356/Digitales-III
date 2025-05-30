#include <stdio.h>
#include "pico/stdlib.h"

#define ENCODER_PIN 2
#define SAMPLE_TIME_MS 1000  // Intervalo para calcular RPM (1 segundo)

// Función para esperar flanco de subida por polling
bool wait_rising_edge(uint gpio) {
    while (gpio_get(gpio));        // Espera que baje
    while (!gpio_get(gpio));       // Espera que suba
    return true;
}

int main() {
    stdio_init_all();
    gpio_init(ENCODER_PIN);
    gpio_set_dir(ENCODER_PIN, GPIO_IN);
    gpio_pull_down(ENCODER_PIN);  // Asegura lectura estable si hay ruido

    while (true) {
        uint32_t start_time = to_ms_since_boot(get_absolute_time());
        uint32_t pulse_count = 0;

        // Cuenta pulsos por 1 segundo (SAMPLE_TIME_MS)
        while (to_ms_since_boot(get_absolute_time()) - start_time < SAMPLE_TIME_MS) {
            if (wait_rising_edge(ENCODER_PIN)) {
                pulse_count++;
            }
        }

        // Supón 1 pulso = 1 vuelta. Si hay más, ajusta:
        float rpm = (pulse_count * 60000.0f) / SAMPLE_TIME_MS;
        printf("RPM (polling): %.2f\n", rpm);
    }

    return 0;
}
