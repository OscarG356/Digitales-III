/**
 * @file motor_control.c
 * @brief Control de un motor DC con lectura de encoder y captura de curva de reacción.
 *
 * Este programa permite:
 * - Ajustar el PWM manualmente con el comando "PWM X"
 * - Ejecutar una curva de reacción automática con el comando "START X"
 * - Medir RPM cada 4 ms y guardar los datos en un buffer
 * - Imprimir la curva al finalizar
 *
 * Hardware utilizado:
 * - Motor controlado con L298 (ENA, IN1, IN2)
 * - Encoder conectado al pin 28
 * - Raspberry Pi Pico
 */

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include <string.h>

/// Pin del PWM (ENA del L298)
#define ENA_PIN 11
/// Pin IN1 del L298
#define IN1_PIN 12
/// Pin IN2 del L298
#define IN2_PIN 13
/// Pin de entrada del encoder
#define ENCODER_PIN 28
/// Pulsos por revolución del encoder
#define PULSOS_POR_REV 20

/// Valor de envolvente para el PWM
#define PWM_WRAP 10000
/// Divisor de frecuencia del clock del PWM
#define PWM_FREQ_DIV 4.0f

/// Tiempo de muestreo en milisegundos
#define MUETREO_MS 4
/// Duración de cada paso de PWM en curva de reacción
#define PASO_PWM_MS 2000
/// Máximo número de muestras del buffer
#define BUFFER_MAX 10000

/**
 * @brief Estados posibles del sistema.
 */
typedef enum { IDLE, CONTROL_PWM, CURVA_REACCION } Estado;

/**
 * @brief Estructura para almacenar una muestra de la curva de reacción.
 */
typedef struct {
    uint32_t tiempo_ms; ///< Tiempo desde inicio de prueba en milisegundos
    uint8_t pwm;        ///< Nivel de PWM aplicado (0-100%)
    float rpm;          ///< RPM medida en ese instante
} Registro;

volatile uint32_t pulsos = 0; ///< Contador global de pulsos del encoder
uint slice;                   ///< PWM slice utilizado
Registro buffer[BUFFER_MAX]; ///< Buffer de almacenamiento de muestras
uint32_t idx = 0;             ///< Índice actual en el buffer

/**
 * @brief Inicializa los pines del motor y el PWM.
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
    pwm_set_chan_level(slice, pwm_gpio_to_channel(ENA_PIN), 0);
}

/**
 * @brief Establece el nivel de PWM al motor.
 * @param duty Ciclo de trabajo entre 0 y 100 (%)
 */
void set_pwm(uint8_t duty) {
    if (duty > 100) duty = 100;
    uint level = (PWM_WRAP * duty) / 100;
    pwm_set_chan_level(slice, pwm_gpio_to_channel(ENA_PIN), level);
}

/**
 * @brief Calcula las RPM a partir de pulsos y tiempo.
 * @param pulsos Número de pulsos medidos
 * @param intervalo_s Tiempo transcurrido en segundos
 * @return RPM calculadas
 */
float calcular_rpm(uint32_t pulsos, float intervalo_s) {
    return (pulsos / (float)PULSOS_POR_REV) / intervalo_s * 60.0f;
}

/**
 * @brief Función principal. Ejecuta el control del motor y captura de datos.
 */
// [...] Código ya documentado con Doxygen arriba

int main() {
    stdio_init_all();  // Inicializa la comunicación serial (USB)
    
    // Configura el pin del encoder como entrada con resistencia pull-up
    gpio_init(ENCODER_PIN);
    gpio_set_dir(ENCODER_PIN, GPIO_IN);
    gpio_pull_up(ENCODER_PIN);

    motor_init();  // Inicializa el motor y su señal PWM

    bool estado_anterior = gpio_get(ENCODER_PIN);  // Estado previo del encoder para detectar flanco
    absolute_time_t t0 = get_absolute_time();      // Tiempo de referencia inicial
    absolute_time_t t_muestra = t0;
    absolute_time_t t_paso = t0;
    absolute_time_t t_print = t0;

    Estado estado_actual = IDLE;
    int pwm = 0;
    int step_up = 20, step_down = 20;  // Incrementos de PWM para subida y bajada
    int direccion = 1;  // Dirección actual de incremento del PWM (1: sube, -1: baja)

    set_pwm(0);  // Inicia con PWM en 0
    char comando[32];  // Buffer para comandos por serial

    while (true) {
        // --- Conteo de pulsos del encoder (detección de flanco ascendente) ---
        bool estado_actual_encoder = gpio_get(ENCODER_PIN);
        if (!estado_anterior && estado_actual_encoder) pulsos++;  // Detecta flanco de subida
        estado_anterior = estado_actual_encoder;

        // Tiempos actuales para verificar intervalos
        absolute_time_t ahora = get_absolute_time();
        int64_t delta_muestra = absolute_time_diff_us(t_muestra, ahora);
        int64_t delta_paso = absolute_time_diff_us(t_paso, ahora);
        int64_t delta_print = absolute_time_diff_us(t_print, ahora);

        // --- Manejo de comandos por UART ---
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

            // --- Procesamiento del comando START ---
            if (strncmp(comando, "START", 5) == 0) {
                sscanf(comando, "START %d", &step_up);
                step_down = step_up;
                printf("Inicio curva reacción | Step: %d\n", step_up);
                estado_actual = CURVA_REACCION;
                pwm = 0;
                direccion = 1;
                set_pwm(pwm);
                t0 = ahora;
                t_paso = ahora;
                idx = 0;  // Resetea el índice del buffer de datos
            }

            // --- Procesamiento del comando PWM ---
            else if (strncmp(comando, "PWM", 3) == 0) {
                sscanf(comando, "PWM %d", &pwm);
                if (pwm < 0) pwm = 0;
                if (pwm > 100) pwm = 100;
                set_pwm(pwm);
                printf("PWM ajustado a %d%%\n", pwm);
                estado_actual = CONTROL_PWM;
            }
        }

        // --- Máquina de estados ---
        switch (estado_actual) {
            case IDLE:
                break;

            case CONTROL_PWM:
                if (delta_print >= 1000000) {  // Cada 1 segundo
                    float rpm_actual = calcular_rpm(pulsos, delta_print / 1e6);
                    printf("[PWM manual] RPM = %.2f | PWM = %d%%\n", rpm_actual, pwm);
                    pulsos = 0;
                    t_print = ahora;
                }
                break;

            case CURVA_REACCION:
                // --- Registro periódico de datos ---
                if (delta_muestra >= MUETREO_MS * 1000 && idx < BUFFER_MAX) {
                    float t_ms = absolute_time_diff_us(t0, ahora) / 1000.0f;
                    float rpm = calcular_rpm(pulsos, MUETREO_MS / 1000.0f);
                    buffer[idx++] = (Registro){.tiempo_ms = (uint32_t)t_ms, .pwm = pwm, .rpm = rpm};
                    pulsos = 0;
                    t_muestra = ahora;
                }

                // --- Cambio de PWM cada cierto tiempo ---
                if (delta_paso >= PASO_PWM_MS * 1000) {
                    pwm += direccion * (direccion > 0 ? step_up : step_down);
                    
                    if (pwm > 100) {
                        pwm = 100;
                        direccion = -1;  // Invierte la dirección para comenzar a bajar
                    } else if (pwm < 0) {
                        set_pwm(0);  // Detiene el motor
                        printf("Curva reacción completada.\n");
                        printf("Tiempo_ms,PWM,RPM\n");

                        // --- Imprime resultados de la curva ---
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
