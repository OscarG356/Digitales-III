#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include <stdio.h>

#define ENA_PIN 11
#define IN1_PIN 12
#define IN2_PIN 13
#define ENCODER_PIN 28
#define PULSOS_POR_REV 20

#define PWM_WRAP 10000
#define PWM_FREQ_DIV 4.0f
#define MAX_MUESTRAS 2000

typedef struct {
    uint32_t timestamp_ms;
    uint8_t pwm;
    float rpm;
} Muestra;

Muestra buffer[MAX_MUESTRAS];
volatile int indice_muestra = 0;
volatile uint32_t pulsos = 0;

uint slice;
absolute_time_t tiempo_inicio;

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

// Función para muestreo a 250 Hz (cada 4 ms)
bool muestrear(struct repeating_timer *t) {
    static absolute_time_t ultimo = {0};
    absolute_time_t ahora = get_absolute_time();
    float intervalo = to_ms_since_boot(ahora) - to_ms_since_boot(ultimo);
    ultimo = ahora;

    if (indice_muestra < MAX_MUESTRAS) {
        float rpm = calcular_rpm(pulsos, intervalo / 1000.0f);
        buffer[indice_muestra].timestamp_ms = to_ms_since_boot(ahora) - to_ms_since_boot(tiempo_inicio);
        buffer[indice_muestra].pwm = pwm_get_chan_level(slice, pwm_gpio_to_channel(ENA_PIN)) * 100 / PWM_WRAP;
        buffer[indice_muestra].rpm = rpm;
        indice_muestra++;
    }
    pulsos = 0;
    return true;
}

void guardar_csv() {
    FILE *f = fopen("reaccion.csv", "w");
    if (!f) return;
    fprintf(f, "tiempo_ms,pwm,rpm\n");
    for (int i = 0; i < indice_muestra; i++) {
        fprintf(f, "%lu,%u,%.2f\n", buffer[i].timestamp_ms, buffer[i].pwm, buffer[i].rpm);
    }
    fclose(f);
}

int main() {
    stdio_init_all();

    gpio_init(ENCODER_PIN);
    gpio_set_dir(ENCODER_PIN, GPIO_IN);
    gpio_pull_up(ENCODER_PIN);

    motor_init();

    tiempo_inicio = get_absolute_time();

    // Polling simple de flanco
    bool anterior = gpio_get(ENCODER_PIN);

    // Timer de muestreo
    struct repeating_timer timer;
    add_repeating_timer_us(-4000, muestrear, NULL, &timer); // 4 ms

    // Escalones de PWM: 0→100% y luego 100→0%
    for (int pwm = 0; pwm <= 100; pwm += 20) {
        set_pwm(pwm);
        sleep_ms(2000);
    }
    for (int pwm = 100; pwm >= 0; pwm -= 20) {
        set_pwm(pwm);
        sleep_ms(2000);
    }

    // Apagar motor y detener timer
    set_pwm(0);
    cancel_repeating_timer(&timer);

    // Guardar datos
    guardar_csv();

    while (true) {
        tight_loop_contents(); // Esperar indefinidamente
    }
}