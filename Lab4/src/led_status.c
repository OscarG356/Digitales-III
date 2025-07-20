/**
 * @file led_status.c
 * @brief Implementación de funciones para controlar LEDs de estado en la Raspberry Pi Pico.
 */

#include "include/led_status.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/time.h"

/**
 * @brief Inicializa los pines GPIO asociados a los LEDs de estado.
 * 
 * Configura los pines como salida y los apaga.
 */
void leds_init(void) {
    gpio_init(LED_ROJO);    gpio_set_dir(LED_ROJO, GPIO_OUT);    led_off(LED_ROJO);
    gpio_init(LED_NARANJA); gpio_set_dir(LED_NARANJA, GPIO_OUT); led_off(LED_NARANJA);
    gpio_init(LED_VERDE);   gpio_set_dir(LED_VERDE, GPIO_OUT);   led_off(LED_VERDE);
}

/**
 * @brief Enciende el LED conectado al GPIO especificado.
 * @param gpio Número de pin GPIO al que está conectado el LED.
 */
void led_on(uint32_t gpio) {
    gpio_put(gpio, 1);
}

/**
 * @brief Apaga el LED conectado al GPIO especificado.
 * @param gpio Número de pin GPIO al que está conectado el LED.
 */
void led_off(uint32_t gpio) {
    gpio_put(gpio, 0);
}

/**
 * @brief Alterna el estado del LED conectado al GPIO especificado.
 * @param gpio Número de pin GPIO al que está conectado el LED.
 */
void led_toggle(uint32_t gpio) {
    gpio_put(gpio, !gpio_get(gpio));
}

/**
 * @brief Hace parpadear un LED a una frecuencia dada durante cierto tiempo.
 * 
 * @param gpio Número de pin GPIO del LED.
 * @param hz Frecuencia de parpadeo en Hz.
 * @param duration_ms Duración total del parpadeo en milisegundos.
 */
void led_blink(uint32_t gpio, int hz, int duration_ms) {
    int periodo = 1000 / hz;
    int ciclos = duration_ms / periodo;
    for (int i = 0; i < ciclos; i++) {
        led_toggle(gpio);
        sleep_ms(periodo / 2);
    }
    led_off(gpio);
}

/**
 * @brief Muestra un estado "OK", encendiendo el LED verde y apagando los demás.
 */
void led_show_ok(void) {
    led_off(LED_ROJO);
    led_off(LED_NARANJA);
    led_on(LED_VERDE);
}

/**
 * @brief Muestra un estado de error, encendiendo el LED rojo y apagando los demás.
 */
void led_show_error(void) {
    led_off(LED_VERDE);
    led_off(LED_NARANJA);
    led_on(LED_ROJO);
}

/**
 * @brief Muestra un parpadeo de captura, haciendo parpadear el LED naranja a 2 Hz durante 3 segundos.
 */
void led_blink_capture(void) {
    led_blink(LED_NARANJA, 2, 3000);
}
