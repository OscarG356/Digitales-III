#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "nmea_parser.h"

#define UART_ID uart1
#define BAUD_RATE 9600
#define UART_TX_PIN 8
#define UART_RX_PIN 9
#define BUF_SIZE 256

int main() {
    stdio_init_all();

    // Esperar conexión USB
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    printf("USB conectado. Iniciando GPS...\n");

    // Inicializar UART para GPS
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    char line[BUF_SIZE];
    int index = 0;
    gps_data_t gps;

    while (true) {
        if (uart_is_readable(UART_ID)) {
            char c = uart_getc(UART_ID);

            // Armar línea hasta \n
            if (c == '\n' || index >= BUF_SIZE - 1) {
                line[index] = '\0';
                index = 0;

                if (line[0] == '$') {
                    // Procesar solo tramas GPRMC
                    if (nmea_parse_line(line, &gps)) {
                        printf("\n🛰️ Datos GPS:\n");
                        printf("Latitud: %.6f° %c\n", gps.latitude, gps.lat_dir);
                        printf("Longitud: %.6f° %c\n", gps.longitude, gps.lon_dir);
                        printf("Fecha: %02d/%02d/%04d\n", gps.day, gps.month, gps.year);
                        printf("Hora: %02d:%02d:%02d\n", gps.hour, gps.minute, gps.second);
                        printf("Satélites: %d | Altitud: %.2f m | Fix: %d\n",
                            gps.satellites, gps.altitude, gps.fix_quality);
}
                }
            } else if (c != '\r') {
                line[index++] = c;
            }
        }

        sleep_ms(1);
    }
}
