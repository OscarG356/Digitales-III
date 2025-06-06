/**
 * @file motor_control.c
 * @brief Control de un motor DC con lectura de RPM mediante encoder en Raspberry Pi Pico.
 *
 * Modos disponibles:
 * - Modo CURVA: barrido automático de PWM y captura de curva de respuesta.
 * - Modo PWM: PWM fijo con lectura periódica de RPM.
 */

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include <string.h>

/// Pin para señal PWM del motor
#define ENA_PIN 11
/// Pines de dirección del motor
#define IN1_PIN 12
#define IN2_PIN 13
/// Pin de entrada del encoder
#define ENCODER_PIN 28
/// Pulsos por revolución del encoder
#define PULSOS_POR_REV 20

/// Resolución del PWM
#define PWM_WRAP 10000
/// Divisor de frecuencia del PWM
#define PWM_FREQ_DIV 4.0f

/// Paso de incremento de PWM en modo curva
#define STEP_PWM 20
/// PWM máximo permitido
#define MAX_PWM 100
/// Periodo de muestreo en milisegundos
#define MUETREO_MS 4
/// Periodo de cambio de PWM en modo curva
#define PASO_PWM_MS 2000
/// Máximo de registros en el buffer
#define BUFFER_MAX 10000

/**
 * @brief Estructura para almacenar los registros de datos.
 */
typedef struct {
    uint32_t tiempo_ms; ///< Tiempo desde inicio en milisegundos
    uint8_t pwm;        ///< Valor del PWM aplicado
    float rpm;          ///< RPM medida
} Registro;

/**
 * @brief Estados posibles del sistema.
 */
typedef enum {
    ESTADO_IDLE,  ///< Espera de comandos
    ESTADO_CURVA, ///< Barrido automático de PWM
    ESTADO_PWM    ///< PWM fijo
} Estado;

volatile uint32_t pulsos = 0; ///< Contador de pulsos del encoder
uint slice;                   ///< PWM slice asociado al pin ENA
Registro buffer[BUFFER_MAX]; ///< Buffer de almacenamiento de datos
uint32_t idx = 0;             ///< Índice actual del buffer

/**
 * @brief Callback de interrupción del encoder.
 * @param gpio Pin que generó la interrupción.
 * @param events Tipo de evento de interrupción.
 */
void encoder_irq_callback(uint gpio, uint32_t events) {
    if (gpio == ENCODER_PIN) {
        pulsos++;
    }
}

/**
 * @brief Inicializa el motor y el PWM.
 */
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

/**
 * @brief Inicializa el encoder y su interrupción.
 */
void encoder_init() {
    gpio_init(ENCODER_PIN);
    gpio_set_dir(ENCODER_PIN, GPIO_IN);
    gpio_pull_up(ENCODER_PIN);
    gpio_set_irq_enabled_with_callback(ENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &encoder_irq_callback);
}

/**
 * @brief Establece el valor de PWM al motor.
 * @param duty Porcentaje de PWM (0 a 100)
 */
void set_pwm(uint8_t duty) {
    if (duty > 100) duty = 100;
    uint level = (PWM_WRAP * duty) / 100;
    pwm_set_chan_level(slice, pwm_gpio_to_channel(ENA_PIN), level);
}

/**
 * @brief Calcula la RPM a partir de la cantidad de pulsos y el intervalo de tiempo.
 * @param pulsos Número de pulsos detectados.
 * @param intervalo_s Intervalo de tiempo en segundos.
 * @return RPM calculada.
 */
float calcular_rpm(uint32_t pulsos, float intervalo_s) {
    return (pulsos / (float)PULSOS_POR_REV) / intervalo_s * 60.0f;
}

/**
 * @brief Función principal. Lee comandos por consola y ejecuta el control del motor.
 */
int main() {
    stdio_init_all();
    motor_init();
    encoder_init();

    Estado estado = ESTADO_IDLE;
    int pwm = 0;
    int direccion = 1;
    char cmd_buffer[32];

    absolute_time_t t0, t_muestra, t_paso, t_inicio;
    t0 = t_muestra = t_paso = t_inicio = get_absolute_time();

    set_pwm(0);

    while (true) {
        // Lectura de comandos por consola (START o PWM <valor>)
        int c = getchar_timeout_us(0);
        if (c != PICO_ERROR_TIMEOUT) {
            int idx_cmd = 0;
            cmd_buffer[idx_cmd++] = (char)c;
            while (idx_cmd < sizeof(cmd_buffer) - 1) {
                c = getchar_timeout_us(0);
                if (c == '\n' || c == '\r' || c == PICO_ERROR_TIMEOUT) break;
                cmd_buffer[idx_cmd++] = (char)c;
            }
            cmd_buffer[idx_cmd] = '\0';

            if (strncmp(cmd_buffer, "START", 5) == 0) {
                // Iniciar modo CURVA
                estado = ESTADO_CURVA;
                idx = 0;
                pwm = 0;
                direccion = 1;
                t0 = t_muestra = t_paso = get_absolute_time();
                set_pwm(pwm);
                printf("Modo CURVA iniciado\n");

            } else if (strncmp(cmd_buffer, "PWM", 3) == 0) {
                // Iniciar modo PWM abierto
                int pwm_val = 0;
                if (sscanf(cmd_buffer + 3, "%d", &pwm_val) == 1) {
                    estado = ESTADO_PWM;
                    set_pwm((uint8_t)pwm_val);
                    printf("Modo PWM abierto, PWM=%d%%\n", pwm_val);
                }
            }
        }

        switch (estado) {
            case ESTADO_IDLE:
                // Estado de espera
                break;

            case ESTADO_CURVA: {
                // Captura periódica de RPM y cambio de PWM automático
                absolute_time_t ahora = get_absolute_time();
                int64_t delta_muestra = absolute_time_diff_us(t_muestra, ahora);
                int64_t delta_paso = absolute_time_diff_us(t_paso, ahora);

                if (delta_muestra >= MUETREO_MS * 1000 && idx < BUFFER_MAX) {
                    float t_ms = absolute_time_diff_us(t0, ahora) / 1000.0f;
                    float rpm = (pulsos / (float)PULSOS_POR_REV) / (MUETREO_MS / 1000.0f) * 60.0f;
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
                        // Fin de la curva
                        set_pwm(0);
                        printf("Curva terminada. Exportando datos...\n");
                        printf("Tiempo_ms,PWM,RPM\n");
                        for (uint32_t i = 0; i < idx; i++) {
                            printf("%lu,%d,%.2f\n", buffer[i].tiempo_ms, buffer[i].pwm, buffer[i].rpm);
                        }
                        estado = ESTADO_IDLE;
                        break;
                    }
                    set_pwm(pwm);
                    t_paso = ahora;
                }
                break;
            }

            case ESTADO_PWM: {
                // Muestra la RPM cada segundo con PWM constante
                absolute_time_t t_actual = get_absolute_time();
                if (absolute_time_diff_us(t_inicio, t_actual) >= 1000000) {
                    float intervalo = absolute_time_diff_us(t_inicio, t_actual) / 1e6;
                    uint32_t pulsos_copia = pulsos;
                    pulsos = 0;
                    printf("[PWM] RPM = %.2f\n", calcular_rpm(pulsos_copia, intervalo));
                    t_inicio = t_actual;
                }
                break;
            }
        }
    }
}
