#ifndef LED_STATUS_H
#define LED_STATUS_H

#include <stdint.h>
#include <stdbool.h>

#define LED_ROJO    19
#define LED_NARANJA 20
#define LED_VERDE   21

// Inicializa los pines de los 3 LEDs
void leds_init(void);

// Encender, apagar y alternar LEDs
void led_on(uint32_t gpio);
void led_off(uint32_t gpio);
void led_toggle(uint32_t gpio);

// Parpadeo con frecuencia y duración
void led_blink(uint32_t gpio, int hz, int duration_ms);

// Estados comunes
void led_show_ok(void);      // Enciende verde
void led_show_error(void);   // Enciende rojo
void led_blink_capture(void); // Parpadea naranja a 2 Hz por 3 s

#endif
