#include "include/led_status.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/time.h"

void leds_init(void) {
    gpio_init(LED_ROJO);    gpio_set_dir(LED_ROJO, GPIO_OUT);    led_off(LED_ROJO);
    gpio_init(LED_NARANJA); gpio_set_dir(LED_NARANJA, GPIO_OUT); led_off(LED_NARANJA);
    gpio_init(LED_VERDE);   gpio_set_dir(LED_VERDE, GPIO_OUT);   led_off(LED_VERDE);
}

void led_on(uint32_t gpio) {
    gpio_put(gpio, 1);
}

void led_off(uint32_t gpio) {
    gpio_put(gpio, 0);
}

void led_toggle(uint32_t gpio) {
    gpio_put(gpio, !gpio_get(gpio));
}

void led_blink(uint32_t gpio, int hz, int duration_ms) {
    int periodo = 1000 / hz;
    int ciclos = duration_ms / periodo;
    for (int i = 0; i < ciclos; i++) {
        led_toggle(gpio);
        sleep_ms(periodo / 2);
    }
    led_off(gpio);
}

// --- Estados comunes ---
void led_show_ok(void) {
    led_off(LED_ROJO);
    led_off(LED_NARANJA);
    led_on(LED_VERDE);
}

void led_show_error(void) {
    led_off(LED_VERDE);
    led_off(LED_NARANJA);
    led_on(LED_ROJO);
}

void led_blink_capture(void) {
    led_blink(LED_NARANJA, 2, 3000);
}
