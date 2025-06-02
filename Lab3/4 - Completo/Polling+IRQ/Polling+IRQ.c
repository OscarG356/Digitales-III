#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include <stdio.h>
#include <string.h>

#define ENA_PIN 11
#define IN1_PIN 12
#define IN2_PIN 13
#define ENCODER_PIN 28
#define PULSOS_POR_REV 20

#define PWM_WRAP 10000
#define PWM_FREQ_DIV 4.0f

#define MUETREO_MS 4
#define PASO_PWM_MS 2000
#define BUFFER_MAX 10000

typedef enum { IDLE, CONTROL_PWM, CURVA_REACCION } Estado;

typedef struct {
    uint32_t tiempo_ms;
    uint8_t pwm;
    float rpm;
} Registro;

volatile uint32_t pulsos = 0;
uint slice;
Registro buffer[BUFFER_MAX];
uint32_t idx = 0;

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

void set_pwm(uint8_t duty) {
    if (duty > 100) duty = 100;
    uint level = (PWM_WRAP * duty) / 100;
    pwm_set_chan_level(slice, pwm_gpio_to_channel(ENA_PIN), level);
}

float calcular_rpm(uint32_t pulsos, float intervalo_s) {
    return (pulsos / (float)PULSOS_POR_REV) / intervalo_s * 60.0f;
}

// *** INTERRUPCIÓN: cuenta pulsos del encoder ***
void encoder_callback(uint gpio, uint32_t events) {
    if (gpio == ENCODER_PIN) pulsos++;
}

int main() {
    stdio_init_all();
    gpio_init(ENCODER_PIN);
    gpio_set_dir(ENCODER_PIN, GPIO_IN);
    gpio_pull_up(ENCODER_PIN);
    gpio_set_irq_enabled_with_callback(ENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &encoder_callback);  // *INTERRUPCIÓN*

    motor_init();

    absolute_time_t t0 = get_absolute_time();
    absolute_time_t t_muestra = t0;
    absolute_time_t t_paso = t0;
    absolute_time_t t_print = t0;

    Estado estado_actual = IDLE;
    int pwm = 0;
    int step_up = 20, step_down = 20;
    int direccion = 1;

    set_pwm(0);
    char comando[32];

    while (true) {
        absolute_time_t ahora = get_absolute_time();
        int64_t delta_muestra = absolute_time_diff_us(t_muestra, ahora);
        int64_t delta_paso = absolute_time_diff_us(t_paso, ahora);
        int64_t delta_print = absolute_time_diff_us(t_print, ahora);

        // *** POLLING: recepción de comando desde consola ***
        int c = getchar_timeout_us(0);
        if (c != PICO_ERROR_TIMEOUT) {
            int idx_cmd = 0;
            comando[idx_cmd++] = (char)c;
            while (idx_cmd < sizeof(comando) - 1) {
                c = getchar_timeout_us(0);
                if (c == '\n' || c == '\r' || c == PICO_ERROR_TIMEOUT) break;
                comando[idx_cmd++] = (char)c;
            }
            comando[idx_cmd] = '\0';

            if (strncmp(comando, "START", 5) == 0) {
                int step;
                sscanf(comando, "START %d", &step);
                step_up = step_down = step;
                printf("Inicio curva reacción | Step: %d\n", step);
                estado_actual = CURVA_REACCION;
                pwm = 0;
                direccion = 1;
                set_pwm(pwm);
                t0 = ahora;
                t_paso = ahora;
                idx = 0;
            } else if (strncmp(comando, "PWM", 3) == 0) {
                sscanf(comando, "PWM %d", &pwm);
                if (pwm < 0) pwm = 0;
                if (pwm > 100) pwm = 100;
                set_pwm(pwm);
                printf("PWM ajustado a %d%%\n", pwm);
                estado_actual = CONTROL_PWM;
            }
        }

        // *** POLLING: control por estados ***
        switch (estado_actual) {
            case IDLE:
                break;

            case CONTROL_PWM:
                if (delta_print >= 1000000) {
                    float rpm_actual = calcular_rpm(pulsos, delta_print / 1e6);
                    printf("[PWM manual] RPM = %.2f | PWM = %d%%\n", rpm_actual, pwm);
                    pulsos = 0;
                    t_print = ahora;
                }
                break;

            case CURVA_REACCION:
                if (delta_muestra >= MUETREO_MS * 1000 && idx < BUFFER_MAX) {
                    float t_ms = absolute_time_diff_us(t0, ahora) / 1000.0f;
                    float rpm = calcular_rpm(pulsos, MUETREO_MS / 1000.0f);
                    buffer[idx++] = (Registro){.tiempo_ms = (uint32_t)t_ms, .pwm = pwm, .rpm = rpm};
                    pulsos = 0;
                    t_muestra = ahora;
                }
                if (delta_paso >= PASO_PWM_MS * 1000) {
                    pwm += direccion * (direccion > 0 ? step_up : step_down);
                    if (pwm > 100) {
                        pwm = 100;
                        direccion = -1;
                    } else if (pwm < 0) {
                        set_pwm(0);
                        printf("Curva reacción completada.\n");
                        printf("Tiempo_ms,PWM,RPM\n");
                        for (uint32_t i = 0; i < idx; i++)
                            printf("%lu,%d,%.2f\n", buffer[i].tiempo_ms, buffer[i].pwm, buffer[i].rpm);
                        estado_actual = IDLE;
                        break;
                    }
                    set_pwm(pwm);
                    t_paso = ahora;
                }
                break;
        }
    }
}