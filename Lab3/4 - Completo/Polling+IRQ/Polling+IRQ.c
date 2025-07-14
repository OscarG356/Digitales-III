/**
 * @file motor_control.c
 * @brief Control de motor DC con medición de RPM usando encoder en Raspberry Pi Pico.
 *
 * Este programa proporciona un control flexible de un motor DC, permitiendo
 * la interacción a través de una interfaz serial (consola). La medición de
 * las Revoluciones Por Minuto (RPM) del motor se realiza mediante un encoder
 * rotatorio, cuyo conteo de pulsos se gestiona a través de interrupciones para
 * mayor precisión.
 *
 * @section features Características Principales:
 * - **Control por Comandos de Consola**: El usuario puede interactuar con el
 * sistema enviando comandos específicos a través de la comunicación serial (UART).
 * - **"START <step>"**: Inicia el modo de "Curva de Reacción". En este modo,
 * el sistema realiza un barrido automático del ciclo de trabajo PWM (subiendo
 * y luego bajando) para caracterizar la respuesta dinámica del motor. El parámetro
 * `<step>` define el incremento/decremento de PWM en cada paso del barrido.
 * - **"PWM <valor>"**: Ajusta el ciclo de trabajo PWM del motor a un valor fijo
 * en modo de control abierto. El `<valor>` debe ser un porcentaje (0-100).
 * - **Medición de RPM en Tiempo Real**: Se mide la velocidad del motor en RPM
 * periódicamente, tanto en el modo de curva de reacción (para registro de datos)
 * como en el modo PWM fijo (para monitoreo continuo).
 * - **Conteo de Pulsos por Interrupción**: Los pulsos del encoder se cuentan
 * utilizando una rutina de interrupción, lo que asegura un conteo preciso y no
 * bloqueante, independientemente de la carga de trabajo del bucle principal.
 *
 * @section hardware_info Información de Hardware:
 * - **Microcontrolador**: Raspberry Pi Pico.
 * - **Controlador del Motor**: Puente H (ej. L298N) conectado a los pines #ENA_PIN,
 * #IN1_PIN y #IN2_PIN para control de velocidad (PWM) y dirección.
 * - **Encoder Rotatorio**: Conectado al pin #ENCODER_PIN para feedback de velocidad.
 *
 * @author [Your Name/Team Name Here]
 * @date June 2, 2025
 */

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/irq.h" // Se incluye para la gestión de interrupciones
#include <stdio.h>
#include <string.h>

/// @name Definiciones de Pines
/// @{
#define ENA_PIN 11              /**< @brief Pin GPIO para la señal PWM de habilitación del motor (Enable A del puente H). */
#define IN1_PIN 12              /**< @brief Pin GPIO de control IN1 para la dirección del motor en el puente H. */
#define IN2_PIN 13              /**< @brief Pin GPIO de control IN2 para la dirección del motor en el puente H. */
#define ENCODER_PIN 28          /**< @brief Pin GPIO conectado a la salida de un canal del encoder para el conteo de pulsos. */
/// @}

/// @name Constantes del Encoder
/// @{
#define PULSOS_POR_REV 20       /**< @brief Número de pulsos que genera el encoder por cada revolución completa del eje del motor. */
/// @}

/// @name Constantes de Configuración PWM
/// @{
#define PWM_WRAP 10000          /**< @brief Valor de envolvente (TOP) para el contador del PWM. Define la resolución del PWM. Un valor de 10000 con un divisor de 4.0f y un reloj de 125MHz da una frecuencia de ~3.125 kHz. */
#define PWM_FREQ_DIV 4.0f       /**< @brief Divisor de frecuencia para el reloj del PWM (cl_div). Un divisor menor resulta en una frecuencia de PWM más alta. */
/// @}

/// @name Constantes de Operación y Muestreo
/// @{
#define MUETREO_MS 4            /**< @brief Intervalo de tiempo en milisegundos para la toma de muestras de RPM y su almacenamiento en el buffer. */
#define PASO_PWM_MS 2000        /**< @brief Duración en milisegundos que se mantiene cada nivel de PWM durante el barrido de la curva de reacción. */
#define BUFFER_MAX 10000        /**< @brief Tamaño máximo del buffer #buffer_datos para almacenar los registros de la curva de reacción. */
/// @}

/**
 * @brief Enumeración de los posibles estados de operación del sistema de control.
 *
 * Define los diferentes modos en los que el controlador del motor puede encontrarse,
 * influenciando el comportamiento y la lógica del bucle principal.
 */
typedef enum {
    IDLE,           /**< @brief El sistema está inactivo, esperando comandos por la interfaz serial. */
    CONTROL_PWM,    /**< @brief El ciclo de trabajo PWM del motor se ajusta manualmente a un valor fijo. */
    CURVA_REACCION  /**< @brief El sistema ejecuta un barrido automático de PWM y registra la respuesta del motor. */
} Estado;

/**
 * @brief Estructura para almacenar un registro de datos de la curva de reacción.
 *
 * Cada instancia de esta estructura representa una medición puntual que incluye
 * el tiempo desde el inicio de la prueba, el valor de PWM aplicado, y las RPM medidas.
 */
typedef struct {
    uint32_t tiempo_ms; /**< @brief Tiempo transcurrido desde el inicio de la prueba en milisegundos. */
    uint8_t pwm;        /**< @brief Valor del ciclo de trabajo PWM aplicado en ese instante (0-100%). */
    float rpm;          /**< @brief Velocidad del motor medida en Revoluciones Por Minuto (RPM). */
} Registro;

/// @name Variables Globales
/// @{
volatile uint32_t pulsos = 0;   /**< @brief Contador global de pulsos del encoder.
                                 * Es `volatile` porque es modificado por una rutina de servicio de interrupción (ISR)
                                 * y leído en el bucle principal, asegurando que el compilador no optimice su acceso. */
uint slice;                     /**< @brief Identificador del "slice" PWM utilizado para el pin #ENA_PIN.
                                 * Necesario para configurar y controlar el canal PWM. */
Registro buffer[BUFFER_MAX];    /**< @brief Buffer de almacenamiento para los registros de datos
                                 * (#Registro) durante la ejecución de la curva de reacción. */
uint32_t idx = 0;               /**< @brief Índice actual en el #buffer.
                                 * Indica la próxima posición disponible para almacenar un nuevo #Registro. */
/// @}


/**
 * @brief Inicializa los pines GPIO y el módulo PWM para el control del motor.
 *
 * Configura los pines de dirección (#IN1_PIN, #IN2_PIN) como salidas y los pone
 * en un estado inicial para una dirección de giro (por ejemplo, hacia adelante).
 * Configura el pin #ENA_PIN para operar como una salida PWM, estableciendo el
 * valor de envolvente (#PWM_WRAP), el divisor de frecuencia (#PWM_FREQ_DIV),
 * y habilitando el bloque PWM. El motor se inicializa con un PWM de 0 (detenido).
 *
 * @pre Las constantes #ENA_PIN, #IN1_PIN, #IN2_PIN, #PWM_WRAP, y #PWM_FREQ_DIV deben estar definidas.
 * @post Los pines del motor están configurados y el subsistema PWM está listo para operar.
 */
void motor_init() {
    gpio_init(IN1_PIN);
    gpio_init(IN2_PIN);
    gpio_set_dir(IN1_PIN, GPIO_OUT);
    gpio_set_dir(IN2_PIN, GPIO_OUT);
    gpio_put(IN1_PIN, 1);   /**< @brief Configura dirección del motor (ej. hacia adelante) */
    gpio_put(IN2_PIN, 0);

    gpio_set_function(ENA_PIN, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(ENA_PIN);
    pwm_set_wrap(slice, PWM_WRAP);
    pwm_set_clkdiv(slice, PWM_FREQ_DIV);
    pwm_set_enabled(slice, true);
    pwm_set_chan_level(slice, pwm_gpio_to_channel(ENA_PIN), 0); // Inicializa PWM en 0
}

/**
 * @brief Ajusta el ciclo de trabajo PWM del motor.
 *
 * Convierte un porcentaje de ciclo de trabajo a un nivel de cuenta PWM
 * (basado en #PWM_WRAP) y lo aplica al canal PWM configurado.
 * El valor de `duty` se limita internamente entre 0 y 100%.
 *
 * @param duty Ciclo de trabajo deseado en porcentaje (0-100).
 * @pre El módulo PWM debe haber sido inicializado previamente con #motor_init.
 * @post El ciclo de trabajo del PWM del motor se actualiza al valor especificado.
 */
void set_pwm(uint8_t duty) {
    if (duty > 100) duty = 100; // Limita el duty a un máximo de 100%
    uint level = (PWM_WRAP * duty) / 100; // Calcula el nivel de cuenta para el PWM
    pwm_set_chan_level(slice, pwm_gpio_to_channel(ENA_PIN), level);
}

/**
 * @brief Calcula las Revoluciones Por Minuto (RPM) del motor.
 *
 * Utiliza el número de pulsos de encoder contados y el intervalo de tiempo
 * durante el cual se contaron para calcular la velocidad angular del motor en RPM.
 *
 * @param pulsos Número de pulsos detectados por el encoder en el `intervalo_s`.
 * @param intervalo_s Duración del intervalo de tiempo en segundos (flotante).
 * @return La velocidad calculada del motor en Revoluciones Por Minuto (RPM).
 * @pre La constante #PULSOS_POR_REV debe estar correctamente definida para el encoder utilizado.
 */
float calcular_rpm(uint32_t pulsos, float intervalo_s) {
    // RPM = (Pulsos / Pulsos_por_Revolución) / Tiempo_en_segundos * 60_segundos_por_minuto
    return (pulsos / (float)PULSOS_POR_REV) / intervalo_s * 60.0f;
}

/**
 * @brief Función de callback para la interrupción del pin del encoder.
 *
 * Esta función es invocada automáticamente por el sistema de interrupciones
 * cada vez que se detecta un flanco de subida en el pin #ENCODER_PIN.
 * Su única tarea es incrementar el contador global `pulsos`.
 *
 * @param gpio El número del pin GPIO que generó la interrupción.
 * @param events El tipo de evento de interrupción que ocurrió (ej. GPIO_IRQ_EDGE_RISE).
 * @pre El pin del encoder debe estar configurado como entrada con pull-up
 * y la interrupción habilitada para flancos de subida.
 * @post El contador global `pulsos` se incrementa en uno.
 * @note Esta función debe ser lo más corta y eficiente posible para no bloquear otras interrupciones.
 */
void encoder_callback(uint gpio, uint32_t events) {
    if (gpio == ENCODER_PIN) {
        pulsos++;
    }
}

/**
 * @brief Función principal del programa.
 *
 * Inicializa la comunicación serial, configura el pin del encoder con interrupciones,
 * e inicializa el motor. Contiene el bucle infinito principal que implementa
 * una máquina de estados para gestionar el comportamiento del sistema,
 * procesando comandos por consola y realizando las mediciones y control del motor.
 *
 * @return Siempre 0 (en un sistema embebido, el bucle principal es infinito).
 *
 * @section main_operations Operaciones Principales:
 * - **Inicialización**: Configura `stdio` para comunicación serial, el pin del encoder
 * con una interrupción en el flanco de subida que invoca a #encoder_callback, y el motor.
 * - **Bucle Infinito**:
 * - Calcula diferencias de tiempo para controlar intervalos de muestreo y cambio de PWM.
 * - **Lectura de Comandos**: Intenta leer comandos (`"START <step>"`, `"PWM <valor>"`)
 * de la entrada serial de forma no bloqueante.
 * - **Procesamiento de Comandos**: Analiza el comando recibido y cambia el
 * #Estado del sistema, reiniciando variables relevantes según el nuevo estado.
 * - **Máquina de Estados**:
 * - **#IDLE**: El sistema está en espera, no realiza acciones periódicas.
 * - **#CONTROL_PWM**: Imprime las RPM actuales y el PWM aplicado cada segundo.
 * El PWM se mantiene fijo según el último comando "PWM".
 * - **#CURVA_REACCION**:
 * - **Muestreo de Datos**: Cada #MUETREO_MS, calcula las RPM (basado en `pulsos`
 * desde la última muestra) y registra un #Registro en el #buffer. Resetea `pulsos`.
 * - **Barrido de PWM**: Cada #PASO_PWM_MS, ajusta el PWM:
 * - Incrementa el PWM por `step_up` hasta alcanzar 100%.
 * - Al alcanzar 100%, invierte la `direccion` y comienza a decrementar el PWM por `step_down`.
 * - Al alcanzar 0% PWM (o menos), la curva se considera completada. Imprime todos los
 * datos almacenados en el #buffer en formato CSV y regresa al estado #IDLE.
 */
int main() {
    stdio_init_all(); // Inicializa la comunicación serial (USB CDC)

    // Configuración del pin del encoder y su interrupción
    gpio_init(ENCODER_PIN);
    gpio_set_dir(ENCODER_PIN, GPIO_IN);
    gpio_pull_up(ENCODER_PIN); // Habilita la resistencia pull-up para el pin del encoder
    // Configura la interrupción en el flanco de subida y asocia la función de callback
    gpio_set_irq_enabled_with_callback(ENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &encoder_callback);

    motor_init(); // Inicializa los pines del motor y el subsistema PWM

    // Variables de tiempo para controlar los intervalos de las operaciones
    absolute_time_t t0 = get_absolute_time();       // Tiempo de referencia para el inicio de la curva
    absolute_time_t t_muestra = t0;                 // Último tiempo en que se tomó una muestra de RPM
    absolute_time_t t_paso = t0;                    // Último tiempo en que se cambió el nivel de PWM en la curva
    absolute_time_t t_print = t0;                   // Último tiempo en que se imprimieron las RPM en modo CONTROL_PWM

    Estado estado_actual = IDLE;    // Estado inicial del sistema
    int pwm = 0;                    // Variable que almacena el valor de PWM actual (0-100%)
    int step_up = 20, step_down = 20; // Incrementos para el PWM en la curva (se actualizan con el comando START)
    int direccion = 1;              // Dirección de cambio del PWM (1: subir, -1: bajar)

    set_pwm(0); // Asegura que el motor esté detenido al inicio

    char comando[32]; // Buffer para almacenar el comando recibido por la consola

    while (true) {
        absolute_time_t ahora = get_absolute_time(); // Tiempo actual en el bucle
        // Cálculos de las diferencias de tiempo desde las últimas operaciones
        int64_t delta_muestra = absolute_time_diff_us(t_muestra, ahora);
        int64_t delta_paso = absolute_time_diff_us(t_paso, ahora);
        int64_t delta_print = absolute_time_diff_us(t_print, ahora);

        // --- Lectura de comandos por consola (no bloqueante) ---
        int c = getchar_timeout_us(0); // Intenta leer un carácter, con timeout de 0 microsegundos
        if (c != PICO_ERROR_TIMEOUT) {
            int idx_cmd = 0;
            comando[idx_cmd++] = (char)c; // Guarda el primer carácter
            // Lee el resto del comando hasta un fin de línea, retorno de carro, o timeout
            while (idx_cmd < sizeof(comando) - 1) {
                c = getchar_timeout_us(0);
                if (c == '\n' || c == '\r' || c == PICO_ERROR_TIMEOUT) break;
                comando[idx_cmd++] = (char)c;
            }
            comando[idx_cmd] = '\0'; // Asegura que la cadena esté terminada en nulo

            // --- Procesamiento de comandos ---
            if (strncmp(comando, "START", 5) == 0) {
                int step;
                sscanf(comando, "START %d", &step); // Extrae el valor del paso del comando
                step_up = step_down = step; // El paso de subida y bajada son iguales
                printf("Inicio curva reacción | Step: %d\n", step);
                estado_actual = CURVA_REACCION; // Cambia al estado de curva de reacción
                pwm = 0;                        // Reinicia el PWM a 0
                direccion = 1;                  // La curva inicia subiendo el PWM
                set_pwm(pwm);                   // Aplica el PWM inicial (0)
                t0 = ahora;                     // Reinicia el tiempo de referencia para la curva
                t_paso = ahora;                 // Reinicia el tiempo del último cambio de PWM
                idx = 0;                        // Resetea el índice del buffer de datos
                // Reiniciar el contador de pulsos global atómicamente antes de empezar una nueva medición
                __atomic_store_n(&pulsos, 0, __ATOMIC_RELAXED);
            } else if (strncmp(comando, "PWM", 3) == 0) {
                sscanf(comando, "PWM %d", &pwm); // Extrae el valor de PWM del comando
                if (pwm < 0) pwm = 0;            // Limita el PWM mínimo a 0
                if (pwm > 100) pwm = 100;        // Limita el PWM máximo a 100
                set_pwm(pwm);                    // Aplica el PWM manual
                printf("PWM ajustado a %d%%\n", pwm);
                estado_actual = CONTROL_PWM;     // Cambia al estado de control manual de PWM
                // Reiniciar el contador de pulsos global atómicamente para una nueva medición en este modo
                __atomic_store_n(&pulsos, 0, __ATOMIC_RELAXED);
                t_print = ahora; // Reinicia el tiempo de la última impresión para el modo manual
            }
        }

        // --- Máquina de estados: Lógica de control basada en el estado actual ---
        switch (estado_actual) {
            case IDLE:
                // El sistema está en espera, no realiza ninguna acción periódica.
                break;

            case CONTROL_PWM:
                // En el modo de control manual de PWM, imprime las RPM cada segundo.
                if (delta_print >= 1000000) { // Si ha pasado 1 segundo (1,000,000 microsegundos)
                    // Tomar una copia atómica de pulsos y luego resetearla
                    uint32_t pulsos_copia = __atomic_exchange_n(&pulsos, 0, __ATOMIC_RELAXED);
                    // Calcular RPM usando los pulsos acumulados en el intervalo de impresión
                    float rpm_actual = calcular_rpm(pulsos_copia, delta_print / 1e6);
                    printf("[PWM manual] RPM = %.2f | PWM = %d%%\n", rpm_actual, pwm);
                    t_print = ahora; // Actualiza el tiempo de la última impresión
                }
                break;

            case CURVA_REACCION:
                // --- Lógica de muestreo de RPM y registro de datos para la curva ---
                if (delta_muestra >= MUETREO_MS * 1000 && idx < BUFFER_MAX) {
                    // Si ha pasado el tiempo de muestreo y hay espacio en el buffer
                    float t_ms = absolute_time_diff_us(t0, ahora) / 1000.0f; // Tiempo actual en ms desde el inicio de la curva
                    // Tomar una copia atómica de pulsos y luego resetearla
                    uint32_t pulsos_copia = __atomic_exchange_n(&pulsos, 0, __ATOMIC_RELAXED);
                    // Calcular RPM para el intervalo de muestreo
                    float rpm = calcular_rpm(pulsos_copia, MUETREO_MS / 1000.0f);
                    // Almacenar la muestra en el buffer
                    buffer[idx++] = (Registro){.tiempo_ms = (uint32_t)t_ms, .pwm = (uint8_t)pwm, .rpm = rpm};
                    t_muestra = ahora; // Actualiza el tiempo de la última muestra
                }

                // --- Lógica de cambio gradual del PWM para el barrido de la curva ---
                if (delta_paso >= PASO_PWM_MS * 1000) { // Si ha pasado el tiempo definido para un paso de PWM
                    pwm += direccion * (direccion > 0 ? step_up : step_down); // Ajusta el PWM

                    if (pwm > 100) {
                        pwm = 100;      // Limita el PWM al 100%
                        direccion = -1; // Invierte la dirección para comenzar a bajar el PWM
                    } else if (pwm < 0) {
                        // Si el PWM ha bajado a 0 (o menos), la curva ha terminado
                        set_pwm(0);     // Detiene el motor
                        printf("Curva reacción completada.\n");
                        printf("Tiempo_ms,PWM,RPM\n"); // Encabezado para la salida CSV

                        // Imprime todos los datos registrados durante la curva
                        for (uint32_t i = 0; i < idx; i++) {
                            printf("%lu,%d,%.2f\n", buffer[i].tiempo_ms, buffer[i].pwm, buffer[i].rpm);
                        }
                        estado_actual = IDLE; // Vuelve al estado inactivo
                        break; // Sale del switch para evitar procesar más en este ciclo si la curva terminó
                    }
                    set_pwm(pwm);   // Aplica el nuevo nivel de PWM al motor
                    t_paso = ahora; // Actualiza el tiempo del último cambio de PWM
                }
                break;
        }
    }
}