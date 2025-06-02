#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include <stdio.h>
#include <string.h>

#define ENA_PIN 11
#define IN1_PIN 12
#define IN2_PIN 13
#define ENCODER_PIN 28
#define PULSOS_POR_REV 20

#define PWM_WRAP 10000
#define PWM_FREQ_DIV 4.0f

#define STEP_PWM 20
#define MAX_PWM 100
#define MUETREO_MS 4
#define PASO_PWM_MS 2000
#define BUFFER_MAX 10000  // depende de pasos y muestreo


typedef struct {
    uint32_t tiempo_ms;
    uint8_t pwm;
    float rpm;
} Registro;

volatile uint32_t pulsos = 0;
uint slice;

Registro buffer[BUFFER_MAX];
uint32_t idx = 0;

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


int main() {
    stdio_init_all();
    gpio_init(ENCODER_PIN);
    gpio_set_dir(ENCODER_PIN, GPIO_IN);
    gpio_pull_up(ENCODER_PIN);

    motor_init();

    bool estado_anterior = gpio_get(ENCODER_PIN);
    absolute_time_t t0 = get_absolute_time();
    absolute_time_t t_muestra = t0;
    absolute_time_t t_paso = t0;

    int pwm = 0;
    int direccion = 1;

    set_pwm(pwm);

    while (true) {
        bool estado_actual = gpio_get(ENCODER_PIN);
        if (!estado_anterior && estado_actual) {
            pulsos++;
        }
        estado_anterior = estado_actual;

        absolute_time_t ahora = get_absolute_time();
        int64_t delta_muestra = absolute_time_diff_us(t_muestra, ahora);
        int64_t delta_paso = absolute_time_diff_us(t_paso, ahora);

        if (delta_muestra >= MUETREO_MS * 1000 && idx < BUFFER_MAX) {
            float t_ms = absolute_time_diff_us(t0, ahora) / 1000.0f;
            float rpm = calcular_rpm(pulsos, MUETREO_MS / 1000.0f);
            buffer[idx++] = (Registro){.tiempo_ms = (uint32_t)t_ms, .pwm = pwm, .rpm = rpm};
            pulsos = 0;
            t_muestra = ahora;
        }

        if (delta_paso >= PASO_PWM_MS * 1000) {
            pwm += direccion * STEP_PWM;
            if (pwm > MAX_PWM) {
                pwm = MAX_PWM;
                direccion = -1;
            } else if (pwm < 0) {
                break;  // Fin del ciclo de prueba
            }
            set_pwm(pwm);
            t_paso = ahora;
        }
    }

    set_pwm(0);  // Apagar motor

    // Enviar datos por serial en formato CSV
    printf("Tiempo_ms,PWM,RPM\n");
    for (uint32_t i = 0; i < idx; i++) {
        printf("%lu,%d,%.2f\n", buffer[i].tiempo_ms, buffer[i].pwm, buffer[i].rpm);
    }

    while (true);  // Detener
}