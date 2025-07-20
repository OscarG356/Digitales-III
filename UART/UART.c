#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"

#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0   // GP0 (no lo usas si solo recibes)
#define UART_RX_PIN 1   // GP1
#define LED_PIN 25      // LED integrado

int main() {
    // Inicializa UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_hw_flow(UART_ID, false, false);

    // Inicializa LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0); // Apagado al inicio

    while (true) {
        if (uart_is_readable(UART_ID)) {
            char c = uart_getc(UART_ID);  // Recibe carácter
            gpio_put(LED_PIN, 1);         // Enciende LED
            sleep_ms(200);                // Lo mantiene prendido un rato
            gpio_put(LED_PIN, 0);         // Apaga LED
        }
    }

    return 0;
}
