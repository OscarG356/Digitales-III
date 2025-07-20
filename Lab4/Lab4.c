/**
 * @file Lab4.c
 * @brief Programa principal para la captura de datos GPS y nivel de ruido utilizando un microcontrolador Raspberry Pi Pico.
 *
 * Este programa implementa un sistema embebido que adquiere datos de ubicación geográfica mediante un módulo GPS
 * y mide niveles de ruido ambiental usando un sensor de audio conectado al ADC. Los datos se almacenan
 * en una memoria EEPROM externa, y el sistema se controla por medio de una máquina de estados.
 *
 * El flujo general incluye la inicialización del sistema, espera de señal GPS (PPS), captura de datos
 * GPS y audio, almacenamiento en memoria, e interacción por interfaz serial.
 *
 * @author Imar y Oscar
 * @date 2025
 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/adc.h"
#include "hardware/sync.h"

#include "include/nmea_parser.h"
#include "include/adc_audio.h"
#include "include/led_status.h"
#include "include/eeprom.h"

/// UART y Pines GPS
#define UART_ID uart1
#define BAUD_RATE 9600
#define UART_TX_PIN 8
#define UART_RX_PIN 9
#define BUF_SIZE 256

/// Pines de sincronización
#define PPS_PIN 7
#define BUTTON_PIN 6

/// Variables globales compartidas entre interrupciones
volatile bool gps_lock = false;              ///< Señal de sincronización GPS válida (PPS).
volatile bool boton_presionado = false;      ///< Bandera de botón físico presionado.
volatile uint32_t last_boton_ms = 0;         ///< Último tiempo de presión de botón.

/// Máquina de estados principal
typedef enum {
    ESTADO_INICIAL,
    ESPERANDO_GPS,
    ESPERANDO_BOTON,
    CAPTURANDO_DATOS,
    INTERFAZ_SERIAL,
    ESTADO_ERROR
} estado_t;

/**
 * @brief Callback global para manejar interrupciones de GPIO.
 *
 * Este callback se activa cuando se detecta un flanco de subida en los pines configurados
 * (botón o PPS). Actualiza las variables globales correspondientes.
 *
 * @param gpio Número del pin GPIO que generó la interrupción.
 * @param events Eventos asociados al GPIO (e.g., flanco de subida).
 */
void gpio_global_callback(uint gpio, uint32_t events);

/**
 * @brief Inicializa la UART para comunicación con el módulo GPS.
 *
 * Configura los pines TX y RX para UART y establece la velocidad de comunicación.
 */
void init_uart_gps();

/**
 * @brief Inicializa los GPIOs para el botón y la señal PPS.
 *
 * Configura los pines como entradas, habilita resistencias pull-down y registra
 * las interrupciones correspondientes.
 */
void init_botones_pps();

/**
 * @brief Captura datos GPS y de audio.
 *
 * Este método espera datos válidos del módulo GPS y luego captura muestras de audio
 * utilizando el ADC. Los datos capturados se almacenan en la EEPROM.
 *
 * @return true si la captura fue exitosa, false si hubo un error o se presionó el botón.
 */
bool capturar_datos();

/**
 * @brief Función principal del programa.
 *
 * Implementa una máquina de estados para manejar las diferentes etapas del sistema:
 * inicialización, espera de señal GPS, espera de botón, captura de datos, interfaz serial y manejo de errores.
 *
 * @return int Código de retorno del programa (0 para éxito).
 */
int main();


// --- Implementación ---

void gpio_global_callback(uint gpio, uint32_t events) {
    if (gpio == BUTTON_PIN && (events & GPIO_IRQ_EDGE_RISE)) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_boton_ms > 200) {
            boton_presionado = true;
            last_boton_ms = now;
        }
    }

    if (gpio == PPS_PIN && (events & GPIO_IRQ_EDGE_RISE)) {
        gps_lock = true;
    }
}

void init_uart_gps() {
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
}

void init_botones_pps() {
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_down(BUTTON_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_RISE, true, &gpio_global_callback);

    gpio_init(PPS_PIN);
    gpio_set_dir(PPS_PIN, GPIO_IN);
    gpio_pull_down(PPS_PIN);
    gpio_set_irq_enabled(PPS_PIN, GPIO_IRQ_EDGE_RISE, true); 
}

bool capturar_datos() {
    char line[BUF_SIZE];
    int index = 0;
    gps_data_t gps;
    bool gps_valido = false;

    printf("\n📡 Esperando datos GPS válidos...\n");
    uint64_t start = to_ms_since_boot(get_absolute_time());

    while (to_ms_since_boot(get_absolute_time()) - start < 5000) {
        if (uart_is_readable(UART_ID)) {
            char c = uart_getc(UART_ID);
            if (c == '\n' || index >= BUF_SIZE - 1) {
                line[index] = '\0';
                index = 0;
                if (line[0] == '$' && nmea_parse_line(line, &gps) && gps.valid_fix) {
                    gps_valido = true;
                    break;
                }
            } else if (c != '\r') {
                line[index++] = c;
            }
        }
        if (boton_presionado) return false;
    }

    if (!gps_valido) return false;

    printf("✅ GPS OK: %.6f %c, %.6f %c\n", gps.latitude, gps.lat_dir, gps.longitude, gps.lon_dir);

    float samples[NUM_SAMPLES];
    for (int i = 0; i < NUM_SAMPLES; i++) {
        if (boton_presionado) return false;
        samples[i] = read_adc_voltage();
        sleep_us(500);  // ~2kHz
    }

    float rms = calculate_rms(samples, NUM_SAMPLES);
    float dbfs = calculate_dbfs(rms);
    printf("🎤 Nivel de ruido: %.2f dBFS\n", dbfs);

    eeprom_guardar_captura(dbfs, gps.latitude, gps.longitude);

    return true;
}

int main() {
    stdio_init_all();
    init_adc();
    leds_init();
    init_uart_gps();
    init_botones_pps();
    eeprom_init();

    estado_t estado = ESTADO_INICIAL;

    while (1) {
        switch (estado) {
            case ESTADO_INICIAL:
                printf("🔁 Inicializando...\n");
                led_off(LED_VERDE);
                led_off(LED_NARANJA);
                led_off(LED_ROJO);

                char comando[8] = {0};
                int i = 0;
                while (i < sizeof(comando) - 1) {
                    int c = getchar();
                    if (c == '\n' || c == '\r') {
                        comando[i] = '\0';
                        break;
                    }
                    comando[i++] = (char)c;
                }

                if (strcmp(comando, "serial") == 0) {
                    estado = INTERFAZ_SERIAL;
                } else if (strcmp(comando, "gps") == 0) {
                    estado = ESPERANDO_GPS;
                } else {
                    printf("❌ Comando no reconocido. Usa 'gps' o 'serial'.\n");
                    sleep_ms(1000);
                }
                break;

            case ESPERANDO_GPS:
                printf("⏳ Esperando señal GPS (PPS)...\n");
                sleep_ms(500);
                __wfi();
                if (gps_lock) {
                    led_show_ok();
                    estado = ESPERANDO_BOTON;
                }
                break;

            case ESPERANDO_BOTON:
                printf("📴 Esperando botón...\n");
                sleep_ms(500);
                __wfi();
                if (!gps_lock) {
                    estado = ESTADO_ERROR;
                    break;
                }
                if (boton_presionado) {
                    boton_presionado = false;
                    estado = CAPTURANDO_DATOS;
                }
                break;

            case CAPTURANDO_DATOS:
                led_off(LED_VERDE);
                led_on(LED_NARANJA);
                boton_presionado = false;
                if (!gps_lock) {
                    estado = ESTADO_ERROR;
                } else if (!capturar_datos()) {
                    led_show_error();
                    sleep_ms(3000);
                    estado = ESPERANDO_BOTON;
                } else {
                    led_blink_capture();  // Parpadea 2 Hz, 3s
                    estado = ESPERANDO_GPS;
                }
                led_off(LED_NARANJA);
                break;

            case INTERFAZ_SERIAL:
                printf("🔌 Interfaz serial activa...\n");
                led_on(LED_VERDE);
                led_on(LED_NARANJA);
                led_on(LED_ROJO);

                while (1) {
                    char comando[16] = {0};
                    int i = 0;
                    while (i < sizeof(comando) - 1) {
                        int c = getchar_timeout_us(0);
                        if (c == PICO_ERROR_TIMEOUT) {
                            sleep_ms(100);
                            continue;
                        }
                        if (c == '\r' || c == '\n') {
                            comando[i] = '\0';
                            break;
                        }
                        comando[i++] = (char)c;
                    }

                    if (strcmp(comando, "q") == 0) {
                        led_off(LED_VERDE);
                        led_off(LED_NARANJA);
                        led_off(LED_ROJO);
                        estado = ESPERANDO_GPS;
                        break;
                    } else if (strcmp(comando, "dump") == 0) {
                        eeprom_ver_datos();
                    } else if (strcmp(comando, "delete") == 0) {
                        eeprom_flush();
                    } else {
                        printf("Comando no reconocido. Usa 'dump', 'delete', o 'q'.\n");
                    }
                }
                break;

            case ESTADO_ERROR:
                printf("❌ Error en el sistema\n");
                led_show_error();
                sleep_ms(3000);
                estado = ESPERANDO_GPS;
                led_off(LED_ROJO);
                break;
        }
    }
}
