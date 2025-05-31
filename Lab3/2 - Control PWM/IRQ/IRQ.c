#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include <stdio.h>

#define ENA_PIN 11
#define IN1_PIN 12
#define IN2_PIN 13
#define ENCODER_PIN 28
#define PULSOS_POR_REV 20

#define PWM_WRAP 10000
#define PWM_FREQ_DIV 4.0f

volatile uint32_t pulsos = 0;
uint slice;

/** @brief Inicializa los pines del motor y PWM */
void motor_init() {
    gpio_init(IN1_PIN);
    gpio_init(IN2_PIN);
    gpio_set_dir(IN1_PIN, GPIO_OUT);
    gpio_set_dir(IN2_PIN, GPIO_OUT);
    gpio_put(IN1_PIN, 1);
    gpio_put(IN2_PIN, 0);

    gpio_set_function(ENA_PIN, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(ENA_PIN);

    pwm_set_wrap(slice, PWM_WRAP);
    pwm_set_clkdiv(slice, PWM_FREQ_DIV);
    pwm_set_enabled(slice, true);
    pwm_set_chan_level(slice, pwm_gpio_to_channel(ENA_PIN), 0);
}

/** @brief Cambia el duty cycle del PWM */
void set_pwm(uint8_t duty) {
    if (duty > 100) duty = 100;
    uint level = (PWM_WRAP * duty) / 100;
    pwm_set_chan_level(slice, pwm_gpio_to_channel(ENA_PIN), level);
}

/** @brief Calcula la RPM a partir del número de pulsos */
float calcular_rpm(uint32_t pulsos, float intervalo_s) {
    return (pulsos / (float)PULSOS_POR_REV) / intervalo_s * 60.0f;
}

/** @brief IRQ handler para el encoder */
void encoder_irq(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_RISE) {
        pulsos++;
    }
}

int main() {
    stdio_init_all();
    gpio_init(ENCODER_PIN);
    gpio_set_dir(ENCODER_PIN, GPIO_IN);
    gpio_pull_up(ENCODER_PIN);

    // Configura IRQ en flanco de subida
    gpio_set_irq_enabled_with_callback(ENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &encoder_irq);

    motor_init();

    char buffer[16];
    int pwm_val = 0;

    absolute_time_t t_inicio = get_absolute_time();

    while (true) {
        // Lectura no bloqueante de PWM por consola
        int c = getchar_timeout_us(0);
        if (c != PICO_ERROR_TIMEOUT) {
            int idx = 0;
            buffer[idx++] = (char)c;
            while (idx < sizeof(buffer) - 1) {
                c = getchar_timeout_us(0);
                if (c == '\n' || c == '\r' || c == PICO_ERROR_TIMEOUT) break;
                buffer[idx++] = (char)c;
            }
            buffer[idx] = '\0';
            if (sscanf(buffer, "%d", &pwm_val) == 1) {
                set_pwm((uint8_t)pwm_val);
                printf("PWM ajustado a %d%%\n", pwm_val);
            }
        }

        // Imprimir cada segundo
        absolute_time_t t_actual = get_absolute_time();
        if (absolute_time_diff_us(t_inicio, t_actual) >= 1000000) { // 1 segundo = 1,000,000 us
            float intervalo = absolute_time_diff_us(t_inicio, t_actual) / 1e6;
            uint32_t pulsos_copia = pulsos;
            pulsos = 0;
            printf("[IRQ] RPM = %.2f\n", calcular_rpm(pulsos_copia, intervalo));
            t_inicio = t_actual;
        }
    }
}
