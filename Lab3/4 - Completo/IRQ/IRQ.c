/**
 * @file motor_control.c
 * @brief Control de un motor DC con lectura de RPM mediante encoder en Raspberry Pi Pico.
 *
 * Este programa ofrece dos modos principales de operación:
 * - **Modo CURVA**: Realiza un barrido automático del ciclo de trabajo PWM del motor (subiendo y luego bajando)
 * mientras captura periódicamente las RPM del motor y el PWM aplicado. Al finalizar el barrido,
 * imprime todos los datos recopilados (tiempo, PWM, RPM) por la interfaz serial.
 * - **Modo PWM**: Mantiene un valor de PWM fijo y configurable por el usuario. En este modo,
 * el sistema imprime las RPM actuales del motor a intervalos regulares.
 *
 * @section hardware Hardware utilizado
 * - Motor DC controlado a través de un puente H (ej. L298N) conectado a los pines #ENA_PIN, #IN1_PIN, #IN2_PIN.
 * - Encoder rotatorio conectado al pin #ENCODER_PIN para la lectura de RPM.
 * - Raspberry Pi Pico como microcontrolador.
 *
 */

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include <string.h>

/// @name Definiciones de Pines
/// @{
/// Pin para la señal PWM del motor (Enable del L298)
#define ENA_PIN 11
/// Pin de dirección IN1 del L298
#define IN1_PIN 12
/// Pin de dirección IN2 del L298
#define IN2_PIN 13
/// Pin de entrada del encoder rotatorio
#define ENCODER_PIN 28
/// @}

/// @name Constantes del Encoder
/// @{
/// Pulsos generados por el encoder por cada revolución completa del motor.
#define PULSOS_POR_REV 20
/// @}

/// @name Constantes del PWM
/// @{
/// Valor máximo del contador para un ciclo PWM completo, define la resolución del PWM.
#define PWM_WRAP 10000
/// Divisor de frecuencia del reloj del PWM. Un valor menor resulta en una frecuencia de PWM más alta.
#define PWM_FREQ_DIV 4.0f
/// @}

/// @name Constantes de Operación
/// @{
/// Paso de incremento/decremento del PWM en el modo CURVA.
#define STEP_PWM 20
/// Valor máximo de PWM permitido (100% del ciclo de trabajo).
#define MAX_PWM 100
/// Periodo de muestreo en milisegundos para la lectura de RPM y almacenamiento de datos.
#define MUETREO_MS 4
/// Periodo en milisegundos para cambiar el valor del PWM en el modo CURVA.
#define PASO_PWM_MS 2000
/// Tamaño máximo del buffer para almacenar los registros de la curva de respuesta.
#define BUFFER_MAX 10000
/// @}

/**
 * @brief Estructura para almacenar los registros de datos de la curva de respuesta.
 *
 * Cada instancia de esta estructura representa una única muestra de datos
 * capturada durante la ejecución del modo CURVA.
 */
typedef struct {
    uint32_t tiempo_ms; ///< @brief Tiempo transcurrido desde el inicio de la prueba en milisegundos.
    uint8_t pwm;        ///< @brief Valor del ciclo de trabajo PWM aplicado en ese instante (0-100%).
    float rpm;          ///< @brief Velocidad del motor medida en Revoluciones Por Minuto (RPM).
} Registro;

/**
 * @brief Enumeración de los posibles estados de operación del sistema.
 *
 * Define el modo actual en el que se encuentra el controlador del motor.
 */
typedef enum {
    ESTADO_IDLE,    ///< @brief El sistema está inactivo, esperando comandos por la interfaz serial.
    ESTADO_CURVA,   ///< @brief El sistema está ejecutando un barrido automático de PWM y capturando datos.
    ESTADO_PWM      ///< @brief El sistema mantiene un PWM fijo y reporta las RPM periódicamente.
} Estado;

/// @name Variables Globales Volátiles
/// @{
volatile uint32_t pulsos = 0;   ///< @brief Contador global de pulsos detectados por el encoder.
                                /**< Se incrementa mediante una interrupción cada vez que el encoder genera un pulso.
                                 * Es volátil para asegurar que el compilador no optimice su acceso en el bucle principal
                                 * y en la rutina de interrupción. */
/// @}

/// @name Variables Globales
/// @{
uint slice;                     ///< @brief El número de 'slice' PWM asignado al pin #ENA_PIN.
                                /**< Se utiliza para configurar y controlar el canal PWM. */
Registro buffer[BUFFER_MAX];    ///< @brief Buffer circular o lineal para almacenar los datos de tipo #Registro.
                                /**< Utilizado principalmente en el modo #ESTADO_CURVA para guardar las mediciones. */
uint32_t idx = 0;               ///< @brief Índice actual para la escritura en el #buffer.
                                /**< Indica la próxima posición disponible para almacenar un nuevo #Registro. */
/// @}

/**
 * @brief Función de callback para la interrupción del encoder.
 *
 * Esta función es invocada automáticamente cada vez que se detecta
 * un flanco de subida (rising edge) en el pin del encoder (#ENCODER_PIN).
 * Simplemente incrementa el contador global de pulsos.
 *
 * @param gpio El número del pin GPIO que generó la interrupción.
 * @param events El tipo de evento que desencadenó la interrupción (ej. GPIO_IRQ_EDGE_RISE).
 * @pre El pin del encoder debe estar configurado como entrada y la interrupción debe estar habilitada.
 * @post El contador global `pulsos` se incrementa en uno.
 */
void encoder_irq_callback(uint gpio, uint32_t events) {
    if (gpio == ENCODER_PIN) {
        pulsos++;
    }
}

/**
 * @brief Inicializa los pines del motor y la configuración del PWM.
 *
 * Configura los pines #IN1_PIN e #IN2_PIN como salidas para controlar la dirección del motor.
 * Configura el pin #ENA_PIN como una salida PWM, estableciendo su envolvente (#PWM_WRAP),
 * el divisor de frecuencia (#PWM_FREQ_DIV), y habilitando el PWM.
 *
 * @pre Los pines #ENA_PIN, #IN1_PIN y #IN2_PIN deben estar correctamente definidos.
 * @post Los pines del motor están configurados y el subsistema PWM está listo para operar.
 */
void motor_init() {
    gpio_init(IN1_PIN);
    gpio_init(IN2_PIN);
    gpio_set_dir(IN1_PIN, GPIO_OUT);
    gpio_set_dir(IN2_PIN, GPIO_OUT);
    gpio_put(IN1_PIN, 1); // Establece una dirección por defecto (hacia adelante)
    gpio_put(IN2_PIN, 0); // Establece una dirección por defecto (hacia adelante)

    gpio_set_function(ENA_PIN, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(ENA_PIN);
    pwm_set_wrap(slice, PWM_WRAP);
    pwm_set_clkdiv(slice, PWM_FREQ_DIV);
    pwm_set_enabled(slice, true);
    // El nivel inicial de PWM se establece en main()
}

/**
 * @brief Inicializa el pin del encoder y configura su interrupción.
 *
 * Configura el pin #ENCODER_PIN como una entrada digital con resistencia pull-up.
 * Habilita las interrupciones en el flanco de subida del encoder y asocia la
 * función #encoder_irq_callback para manejar estas interrupciones.
 *
 * @pre El pin #ENCODER_PIN debe estar definido.
 * @post El encoder está configurado para contar pulsos mediante interrupciones.
 */
void encoder_init() {
    gpio_init(ENCODER_PIN);
    gpio_set_dir(ENCODER_PIN, GPIO_IN);
    gpio_pull_up(ENCODER_PIN);
    // Habilita la interrupción en el flanco de subida y registra el callback
    gpio_set_irq_enabled_with_callback(ENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &encoder_irq_callback);
}

/**
 * @brief Establece el valor del ciclo de trabajo PWM para el motor.
 *
 * Calcula el nivel de PWM requerido basado en un porcentaje de ciclo de trabajo
 * y lo aplica al canal PWM correspondiente. El valor de `duty` se limita
 * automáticamente entre 0 y 100 para evitar desbordamientos.
 *
 * @param duty Porcentaje del ciclo de trabajo PWM deseado (0 a 100).
 * @pre El subsistema PWM debe haber sido inicializado con #motor_init.
 * @post El ciclo de trabajo del PWM del motor se actualiza al valor especificado.
 */
void set_pwm(uint8_t duty) {
    if (duty > 100) duty = 100; // Limita el duty a un máximo de 100%
    uint level = (PWM_WRAP * duty) / 100; // Convierte el porcentaje a un nivel de cuenta PWM
    pwm_set_chan_level(slice, pwm_gpio_to_channel(ENA_PIN), level);
}

/**
 * @brief Calcula las Revoluciones Por Minuto (RPM) del motor.
 *
 * Utiliza el número de pulsos del encoder y el intervalo de tiempo transcurrido
 * para determinar la velocidad actual del motor en RPM.
 *
 * @param pulsos Número de pulsos detectados por el encoder en el `intervalo_s`.
 * @param intervalo_s Duración del intervalo de tiempo en segundos sobre el cual se contaron los pulsos.
 * @return La velocidad calculada del motor en Revoluciones Por Minuto (RPM).
 * @pre La constante #PULSOS_POR_REV debe estar configurada correctamente para el encoder utilizado.
 */
float calcular_rpm(uint32_t pulsos, float intervalo_s) {
    // RPM = (Pulsos / Pulsos_por_Rev) / Tiempo_en_segundos * 60_segundos_por_minuto
    return (pulsos / (float)PULSOS_POR_REV) / intervalo_s * 60.0f;
}

/**
 * @brief Función principal del programa.
 *
 * Inicializa la comunicación serial, el motor y el encoder.
 * Contiene el bucle infinito que gestiona la máquina de estados del sistema,
 * procesa los comandos recibidos por la interfaz serial y realiza las acciones
 * correspondientes a cada estado (IDLE, CURVA, PWM fijo).
 *
 * Los comandos soportados son:
 * - "START": Inicia el modo #ESTADO_CURVA para el barrido automático de PWM.
 * - "PWM <valor>": Inicia el modo #ESTADO_PWM, estableciendo un PWM fijo de `<valor>%`.
 *
 * @return Siempre 0 (el bucle principal es infinito en una aplicación embebida).
 *
 * @section main_flow Flujo principal
 * 1. Inicialización de periféricos (UART, motor, encoder).
 * 2. Bucle infinito (`while(true)`):
 * - **Lectura de comandos**: Intenta leer un comando completo de la entrada serial.
 * - **Procesamiento de comandos**:
 * - Si el comando es "START", cambia al estado #ESTADO_CURVA, resetea variables de control
 * y el buffer de datos.
 * - Si el comando es "PWM <valor>", cambia al estado #ESTADO_PWM, aplica el PWM especificado
 * y lo reporta.
 * - **Máquina de estados**:
 * - **#ESTADO_IDLE**: Permanece inactivo, esperando nuevos comandos.
 * - **#ESTADO_CURVA**:
 * - Cada #MUETREO_MS, calcula las RPM, registra una #Registro en el #buffer.
 * - Cada #PASO_PWM_MS, incrementa o decrementa el PWM (#STEP_PWM).
 * - Al alcanzar #MAX_PWM, invierte la dirección del PWM.
 * - Al llegar a 0% PWM, finaliza la curva, imprime todos los datos del #buffer por serial,
 * y regresa al estado #ESTADO_IDLE.
 * - **#ESTADO_PWM**:
 * - Cada 1 segundo, calcula las RPM y las imprime por serial, junto con el PWM actual.
 */
int main() {
    stdio_init_all();   // Inicializa la comunicación serial por USB
    motor_init();       // Configura los pines del motor y el PWM
    encoder_init();     // Configura el pin del encoder y su interrupción

    Estado estado = ESTADO_IDLE; // El sistema inicia en estado inactivo
    int pwm = 0;                 // Valor actual del PWM
    int direccion = 1;           // Dirección de cambio del PWM en modo curva (1 para subir, -1 para bajar)
    char cmd_buffer[32];         // Buffer para almacenar comandos de la consola

    // Variables de tiempo para controlar intervalos
    absolute_time_t t0;          // Tiempo de inicio de la curva para referencia de tiempo relativo
    absolute_time_t t_muestra;   // Último tiempo de toma de muestra de RPM
    absolute_time_t t_paso;      // Último tiempo de cambio de PWM en modo curva
    absolute_time_t t_inicio;    // Tiempo de inicio para mediciones en modo PWM fijo (para cálculo de RPM)

    // Inicializa las variables de tiempo para evitar valores basura antes del primer uso
    t0 = t_muestra = t_paso = t_inicio = get_absolute_time();

    set_pwm(0); // Asegura que el motor esté detenido al iniciar

    while (true) {
        // --- Lectura de comandos por consola (UART) ---
        // Intenta leer un carácter de la entrada serial sin bloquear (timeout de 0 us)
        int c = getchar_timeout_us(0);
        if (c != PICO_ERROR_TIMEOUT) {
            int idx_cmd = 0;
            cmd_buffer[idx_cmd++] = (char)c; // Guarda el primer carácter
            // Lee el resto del comando hasta encontrar un salto de línea, retorno de carro, o timeout
            while (idx_cmd < sizeof(cmd_buffer) - 1) {
                c = getchar_timeout_us(0);
                if (c == '\n' || c == '\r' || c == PICO_ERROR_TIMEOUT) break;
                cmd_buffer[idx_cmd++] = (char)c;
            }
            cmd_buffer[idx_cmd] = '\0'; // Null-terminate el string del comando

            // --- Procesamiento de comandos ---
            if (strncmp(cmd_buffer, "START", 5) == 0) {
                // Comando "START": Inicia el modo CURVA
                estado = ESTADO_CURVA;
                idx = 0;         // Resetea el índice del buffer
                pwm = 0;         // Inicia el PWM en 0
                direccion = 1;   // Inicia la curva subiendo el PWM
                // Reinicia todos los contadores de tiempo para el inicio de la curva
                t0 = t_muestra = t_paso = get_absolute_time();
                set_pwm(pwm); // Aplica el PWM inicial
                printf("Modo CURVA iniciado\n");

            } else if (strncmp(cmd_buffer, "PWM", 3) == 0) {
                // Comando "PWM <valor>": Inicia el modo PWM abierto
                int pwm_val = 0;
                // Intenta parsear el valor numérico después de "PWM"
                if (sscanf(cmd_buffer + 3, "%d", &pwm_val) == 1) {
                    estado = ESTADO_PWM;        // Cambia al estado de PWM fijo
                    set_pwm((uint8_t)pwm_val);  // Aplica el PWM especificado
                    // Reinicia el contador de pulsos y el tiempo de inicio para la lectura de RPM en este modo
                    __atomic_store_n(&pulsos, 0, __ATOMIC_RELAXED); // Reset `pulsos` atomically
                    t_inicio = get_absolute_time();
                    printf("Modo PWM abierto, PWM=%d%%\n", pwm_val);
                }
            }
        }

        // --- Máquina de Estados ---
        switch (estado) {
            case ESTADO_IDLE:
                // El sistema está en espera, no realiza acciones periódicas.
                break;

            case ESTADO_CURVA: {
                // Captura periódica de RPM y cambio de PWM automático
                absolute_time_t ahora = get_absolute_time(); // Tiempo actual

                // Diferencias de tiempo desde la última muestra y último cambio de paso
                int64_t delta_muestra = absolute_time_diff_us(t_muestra, ahora);
                int64_t delta_paso = absolute_time_diff_us(t_paso, ahora);

                // Lógica de muestreo de datos para la curva
                if (delta_muestra >= MUETREO_MS * 1000 && idx < BUFFER_MAX) {
                    // Calcula el tiempo transcurrido desde el inicio de la curva en milisegundos
                    float t_ms = absolute_time_diff_us(t0, ahora) / 1000.0f;
                    // Toma una copia atómica de pulsos para el cálculo y luego lo resetea
                    uint32_t pulsos_copia = __atomic_exchange_n(&pulsos, 0, __ATOMIC_RELAXED);
                    // Calcula RPM usando los pulsos acumulados en el intervalo de muestreo
                    float rpm = calcular_rpm(pulsos_copia, MUETREO_MS / 1000.0f);
                    // Guarda la muestra en el buffer
                    buffer[idx++] = (Registro){.tiempo_ms = (uint32_t)t_ms, .pwm = (uint8_t)pwm, .rpm = rpm};
                    t_muestra = ahora; // Actualiza el tiempo de la última muestra
                }

                // Lógica de cambio de PWM en la curva
                if (delta_paso >= PASO_PWM_MS * 1000) {
                    pwm += direccion * STEP_PWM; // Incrementa/decrementa el PWM

                    if (pwm > MAX_PWM) {
                        pwm = MAX_PWM;    // Limita el PWM al máximo
                        direccion = -1;   // Invierte la dirección para empezar a bajar
                    } else if (pwm < 0) {
                        // La curva ha llegado a su fin (PWM < 0, lo que significa que bajó de 0)
                        set_pwm(0); // Asegura que el motor se detenga
                        printf("Curva terminada. Exportando datos...\n");
                        printf("Tiempo_ms,PWM,RPM\n"); // Encabezado de la tabla

                        // Imprime todos los datos recolectados en el buffer
                        for (uint32_t i = 0; i < idx; i++) {
                            printf("%lu,%d,%.2f\n", buffer[i].tiempo_ms, buffer[i].pwm, buffer[i].rpm);
                        }
                        estado = ESTADO_IDLE; // Vuelve al estado inactivo
                        break; // Sale del switch para evitar aplicar PWM o procesar más en este ciclo
                    }
                    set_pwm(pwm);   // Aplica el nuevo nivel de PWM
                    t_paso = ahora; // Actualiza el tiempo del último cambio de PWM
                }
                break;
            }

            case ESTADO_PWM: {
                // Muestra la RPM cada segundo con PWM constante
                absolute_time_t t_actual = get_absolute_time(); // Tiempo actual

                // Si ha pasado al menos 1 segundo desde la última impresión de RPM
                if (absolute_time_diff_us(t_inicio, t_actual) >= 1000000) {
                    float intervalo = absolute_time_diff_us(t_inicio, t_actual) / 1e6; // Intervalo en segundos
                    // Toma una copia atómica de pulsos para el cálculo y luego lo resetea
                    uint32_t pulsos_copia = __atomic_exchange_n(&pulsos, 0, __ATOMIC_RELAXED);
                    printf("[PWM] RPM = %.2f\n", calcular_rpm(pulsos_copia, intervalo));
                    t_inicio = t_actual; // Actualiza el tiempo de la última impresión
                }
                break;
            }
        }
    }
}