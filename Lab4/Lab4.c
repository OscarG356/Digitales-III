#include <stdio.h>
#include <math.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/sync.h"
#include "hardware/adc.h"
#include "include/adc_audio.h"
#include "include/nmea_parser.h"

#define UART_ID uart1
#define BAUD_RATE 9600
#define UART_TX_PIN 8
#define UART_RX_PIN 9
#define BUF_SIZE 256

#define BUTTON_PIN 6
#define TIMEOUT_MS 5000      // Timeout GPS: 5 segundos
#define ADC_INTERVAL_US 2500 // 2.5 ms entre muestras (400 Hz)
#define NUM_SAMPLES 4000     // 10 segundos * 400 Hz

volatile bool tomar_datos = false;
volatile uint32_t last_press_ms = 0;

// Callback del botón
void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == BUTTON_PIN && (events & GPIO_IRQ_EDGE_RISE)) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_press_ms > 200) {
            tomar_datos = true;
            last_press_ms = now;
        }
    }
}

int main() {
    stdio_init_all();
    init_adc();

    // Inicializar botón
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_down(BUTTON_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    // Inicializar UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Esperar conexión USB
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    printf("USB conectado. Iniciando sistema...\n");

    char line[BUF_SIZE];
    int index = 0;
    gps_data_t gps;
    float samples[NUM_SAMPLES];

    while (true) {
        if (tomar_datos) {
            tomar_datos = false;
            printf("\n📡 Esperando datos válidos del GPS (timeout 5s)...\n");

            uint64_t start_time = to_ms_since_boot(get_absolute_time());
            bool found_valid_fix = false;

            // Esperar GPS válido
            while ((to_ms_since_boot(get_absolute_time()) - start_time) < TIMEOUT_MS) {
                if (uart_is_readable(UART_ID)) {
                    char c = uart_getc(UART_ID);
                    if (c == '\n' || index >= BUF_SIZE - 1) {
                        line[index] = '\0';
                        index = 0;

                        if (line[0] == '$' && nmea_parse_line(line, &gps) && gps.valid_fix) {
                            found_valid_fix = true;
                            break;
                        }
                    } else if (c != '\r') {
                        line[index++] = c;
                    }
                }
            }

            if (found_valid_fix) {
                // Imprimir ubicación
                printf("\n🛰️ Datos GPS válidos:\n");
                printf("Latitud: %.6f° %c\n", gps.latitude, gps.lat_dir);
                printf("Longitud: %.6f° %c\n", gps.longitude, gps.lon_dir);
                printf("Fecha: %02d/%02d/%04d\n", gps.day, gps.month, gps.year);
                printf("Hora: %02d:%02d:%02d\n", gps.hour, gps.minute, gps.second);
                printf("Satélites: %d | Altitud: %.2f m | Fix: %d\n",
                       gps.satellites, gps.altitude, gps.fix_quality);

                // Tomar muestras ADC por 10 segundos
                printf("\n🎙️ Midiendo audio por 10 segundos...\n");
                for (uint32_t i = 0; i < NUM_SAMPLES; i++) {
                    samples[i] = read_adc_voltage();
                    sleep_us(ADC_INTERVAL_US);
                }

                float rms = calculate_rms(samples, NUM_SAMPLES);
                float dbfs = calculate_dbfs(rms);
                printf("🔊 Nivel de audio: %.2f dBFS\n", dbfs);

            } else {
                printf("\n🚫 No se obtuvo señal GPS válida en 5 segundos. No se midió audio.\n");
            }
        }

        __wfi(); // Esperar interrupción del botón
    }
}
