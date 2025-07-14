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
typedef enum {
    IDLE,           ///< El sistema está inactivo, esperando comandos.
    CONTROL_PWM,    ///< El PWM se controla manualmente a través de comandos seriales.
    CURVA_REACCION  ///< Ejecutando la rutina de curva de reacción automática.
} Estado;

/**
 * @brief Estructura para almacenar una muestra de la curva de reacción.
 */
typedef struct {
    uint32_t tiempo_ms; ///< Tiempo desde inicio de prueba en milisegundos
    uint8_t pwm;        ///< Nivel de PWM aplicado (0-100%)
    float rpm;          ///< RPM medida en ese instante
} Registro;

volatile uint32_t pulsos = 0; ///< Contador global de pulsos del encoder.
uint slice;                   ///< PWM slice utilizado para controlar el motor.
Registro buffer[BUFFER_MAX]; ///< Buffer de almacenamiento de muestras de la curva de reacción.
uint32_t idx = 0;             ///< Índice actual en el buffer de almacenamiento de muestras.

/**
 * @brief Inicializa los pines del motor y el PWM.
 *
 * Configura los pines IN1 e IN2 del L298 como salidas digitales y el pin ENA
 * como salida PWM. Establece la dirección inicial del motor y configura
 * los parámetros del PWM (envolvente, divisor de reloj) y lo habilita.
 */
void motor_init() {
    gpio_init(IN1_PIN);
    gpio_init(IN2_PIN);
    gpio_set_dir(IN1_PIN, GPIO_OUT);
    gpio_set_dir(IN2_PIN, GPIO_OUT);
    // Establece la dirección inicial del motor (ej. hacia adelante)
    gpio_put(IN1_PIN, 1);
    gpio_put(IN2_PIN, 0);

    gpio_set_function(ENA_PIN, GPIO_FUNC_PWM); // Configura el pin ENA para la función PWM
    slice = pwm_gpio_to_slice_num(ENA_PIN);    // Obtiene el slice PWM asociado al pin ENA

    pwm_set_wrap(slice, PWM_WRAP);          // Establece el valor máximo del contador PWM (envolvente)
    pwm_set_clkdiv(slice, PWM_FREQ_DIV);    // Establece el divisor de frecuencia del reloj PWM
    pwm_set_enabled(slice, true);           // Habilita el slice PWM
    pwm_set_chan_level(slice, pwm_gpio_to_channel(ENA_PIN), 0); // Inicia con el PWM en 0
}

/**
 * @brief Establece el nivel de PWM al motor.
 * @param duty Ciclo de trabajo entre 0 y 100 (%).
 *
 * Esta función calcula el nivel del canal PWM a partir del ciclo de trabajo
 * deseado y lo aplica al pin ENA. El valor de duty se limita entre 0 y 100.
 */
void set_pwm(uint8_t duty) {
    if (duty > 100) duty = 100; // Asegura que el duty cycle no exceda 100%
    uint level = (PWM_WRAP * duty) / 100; // Calcula el nivel del PWM basado en el duty cycle y el PWM_WRAP
    pwm_set_chan_level(slice, pwm_gpio_to_channel(ENA_PIN), level); // Aplica el nivel PWM
}

/**
 * @brief Calcula las RPM a partir de pulsos y tiempo.
 * @param pulsos Número de pulsos medidos del encoder.
 * @param intervalo_s Tiempo transcurrido en segundos durante el cual se midieron los pulsos.
 * @return RPM calculadas (revoluciones por minuto).
 *
 * La fórmula utilizada es: RPM = (Pulsos / PULSOS_POR_REV) / intervalo_s * 60.0f
 */
float calcular_rpm(uint32_t pulsos, float intervalo_s) {
    // Evitar división por cero si el intervalo es muy pequeño
    if (intervalo_s == 0) return 0.0f;
    return (pulsos / (float)PULSOS_POR_REV) / intervalo_s * 60.0f;
}

/**
 * @brief Función principal del programa.
 *
 * Inicializa la comunicación serial, configura el pin del encoder,
 * inicializa el motor y el PWM. Implementa un bucle infinito que gestiona
 * la lectura del encoder, el procesamiento de comandos seriales y una
 * máquina de estados para controlar el motor (modo manual o curva de reacción).
 * En el modo de curva de reacción, registra datos de tiempo, PWM y RPM en un buffer
 * para su posterior impresión.
 *
 * @return Siempre 0 (aunque el bucle `while(true)` impide su retorno).
 */
int main() {
    stdio_init_all();  // Inicializa la comunicación serial (USB)

    // Configura el pin del encoder como entrada con resistencia pull-up
    gpio_init(ENCODER_PIN);
    gpio_set_dir(ENCODER_PIN, GPIO_IN);
    gpio_pull_up(ENCODER_PIN);

    motor_init();  // Inicializa el motor y su señal PWM

    bool estado_anterior = gpio_get(ENCODER_PIN);   // Estado previo del encoder para detectar flanco
    absolute_time_t t0 = get_absolute_time();       // Tiempo de referencia inicial para la curva de reacción
    absolute_time_t t_muestra = t0;                 // Tiempo para la próxima muestra de RPM
    absolute_time_t t_paso = t0;                    // Tiempo para el próximo cambio de PWM en la curva
    absolute_time_t t_print = t0;                   // Tiempo para la próxima impresión en modo manual

    Estado estado_actual = IDLE; // Estado inicial del sistema
    int pwm = 0;                 // Nivel actual de PWM
    int step_up = 20, step_down = 20; // Incrementos de PWM para subida y bajada en la curva
    int direccion = 1;           // Dirección actual de incremento del PWM (1: sube, -1: baja)

    set_pwm(0);  // Inicia con PWM en 0
    char comando[32]; // Buffer para comandos por serial

    while (true) {
        // --- Conteo de pulsos del encoder (detección de flanco ascendente) ---
        bool estado_actual_encoder = gpio_get(ENCODER_PIN);
        if (!estado_anterior && estado_actual_encoder) {
            pulsos++; // Detecta flanco de subida e incrementa el contador de pulsos
        }
        estado_anterior = estado_actual_encoder; // Actualiza el estado anterior del encoder

        // Tiempos actuales para verificar intervalos
        absolute_time_t ahora = get_absolute_time();
        int64_t delta_muestra = absolute_time_diff_us(t_muestra, ahora);
        int64_t delta_paso = absolute_time_diff_us(t_paso, ahora);
        int64_t delta_print = absolute_time_diff_us(t_print, ahora);

        // --- Manejo de comandos por UART ---
        int c = getchar_timeout_us(0); // Intenta leer un carácter de la UART sin bloqueo
        if (c != PICO_ERROR_TIMEOUT) { // Si se recibió un carácter
            int idx_cmd = 0;
            comando[idx_cmd++] = (char)c; // Almacena el primer carácter
            // Lee el resto del comando hasta un salto de línea, retorno de carro o timeout
            while (idx_cmd < sizeof(comando) - 1) {
                c = getchar_timeout_us(0);
                if (c == '\n' || c == '\r' || c == PICO_ERROR_TIMEOUT) break;
                comando[idx_cmd++] = (char)c;
            }
            comando[idx_cmd] = '\0'; // Nul-termina el comando

            // --- Procesamiento del comando START ---
            if (strncmp(comando, "START", 5) == 0) {
                sscanf(comando, "START %d", &step_up); // Extrae el valor del paso
                step_down = step_up; // El paso de bajada es igual al de subida
                printf("Inicio curva reacción | Step: %d\n", step_up);
                estado_actual = CURVA_REACCION; // Cambia al estado de curva de reacción
                pwm = 0;                     // Reinicia PWM a 0
                direccion = 1;               // Comienza subiendo el PWM
                set_pwm(pwm);                // Aplica el PWM inicial
                t0 = ahora;                  // Reinicia el tiempo de referencia para la curva
                t_paso = ahora;              // Reinicia el tiempo para el cambio de paso
                idx = 0;                     // Resetea el índice del buffer de datos
            }
            // --- Procesamiento del comando PWM ---
            else if (strncmp(comando, "PWM", 3) == 0) {
                sscanf(comando, "PWM %d", &pwm); // Extrae el valor del PWM
                if (pwm < 0) pwm = 0;           // Limita el PWM a un mínimo de 0
                if (pwm > 100) pwm = 100;       // Limita el PWM a un máximo de 100
                set_pwm(pwm);                   // Aplica el PWM
                printf("PWM ajustado a %d%%\n", pwm);
                estado_actual = CONTROL_PWM;    // Cambia al estado de control manual de PWM
            }
        }

        // --- Máquina de estados ---
        switch (estado_actual) {
            case IDLE:
                // No hace nada, esperando comandos
                break;

            case CONTROL_PWM:
                // Imprime las RPM cada 1 segundo en modo de control manual
                if (delta_print >= 1000000) { // Cada 1 segundo (1,000,000 microsegundos)
                    float rpm_actual = calcular_rpm(pulsos, delta_print / 1e6); // Calcula RPM
                    printf("[PWM manual] RPM = %.2f | PWM = %d%%\n", rpm_actual, pwm);
                    pulsos = 0;        // Reinicia el contador de pulsos
                    t_print = ahora;   // Actualiza el tiempo de la última impresión
                }
                break;

            case CURVA_REACCION:
                // --- Registro periódico de datos ---
                // Registra datos cada MUETREO_MS si el buffer no está lleno
                if (delta_muestra >= MUETREO_MS * 1000 && idx < BUFFER_MAX) {
                    float t_ms = absolute_time_diff_us(t0, ahora) / 1000.0f; // Tiempo desde el inicio de la curva
                    float rpm = calcular_rpm(pulsos, MUETREO_MS / 1000.0f);   // Calcula RPM para el intervalo de muestreo
                    // Almacena la muestra en el buffer
                    buffer[idx++] = (Registro){.tiempo_ms = (uint32_t)t_ms, .pwm = (uint8_t)pwm, .rpm = rpm};
                    pulsos = 0;        // Reinicia el contador de pulsos
                    t_muestra = ahora; // Actualiza el tiempo de la última muestra
                }

                // --- Cambio de PWM cada cierto tiempo ---
                // Ajusta el PWM cada PASO_PWM_MS
                if (delta_paso >= PASO_PWM_MS * 1000) {
                    pwm += direccion * (direccion > 0 ? step_up : step_down); // Incrementa/decrementa el PWM
                    
                    if (pwm >= 100) {
                        pwm = 100;         // Asegura que el PWM no exceda 100
                        direccion = -1;    // Invierte la dirección para comenzar a bajar
                    } else if (pwm <= 0) {
                        set_pwm(0);        // Detiene el motor (PWM a 0)
                        printf("Curva reacción completada.\n");
                        printf("Tiempo_ms,PWM,RPM\n"); // Encabezado para la salida CSV

                        // --- Imprime resultados de la curva ---
                        // Itera sobre el buffer e imprime los datos registrados
                        for (uint32_t i = 0; i < idx; i++) {
                            printf("%lu,%d,%.2f\n", buffer[i].tiempo_ms, buffer[i].pwm, buffer[i].rpm);
                        }
                        estado_actual = IDLE; // Vuelve al estado inactivo
                        break; // Sale del switch case para evitar más procesamiento en este ciclo
                    }

                    set_pwm(pwm);      // Aplica el nuevo valor de PWM
                    t_paso = ahora;    // Actualiza el tiempo del último cambio de paso
                }
                break;
        }
    }
    return 0; // Este retorno es teóricamente inalcanzable debido al bucle while(true)
}