/**
 * @file led_status.h
 * @brief Control de LEDs para indicar estado del sistema.
 */

#ifndef LED_STATUS_H
#define LED_STATUS_H

#include <stdint.h>
#include <stdbool.h>

#define LED_ROJO    19  /**< GPIO para LED rojo */
#define LED_NARANJA 20  /**< GPIO para LED naranja */
#define LED_VERDE   21  /**< GPIO para LED verde */

/**
 * @brief Inicializa los GPIOs de los LEDs.
 */
void leds_init(void);

/**
 * @brief Enciende un LED específico.
 * @param gpio Número de GPIO del LED.
 */
void led_on(uint32_t gpio);

/**
 * @brief Apaga un LED específico.
 * @param gpio Número de GPIO del LED.
 */
void led_off(uint32_t gpio);

/**
 * @brief Cambia el estado (toggle) de un LED.
 * @param gpio Número de GPIO del LED.
 */
void led_toggle(uint32_t gpio);

/**
 * @brief Hace parpadear un LED a una frecuencia durante un tiempo.
 * @param gpio Número de GPIO del LED.
 * @param hz Frecuencia de parpadeo en Hz.
 * @param duration_ms Duración total del parpadeo en milisegundos.
 */
void led_blink(uint32_t gpio, int hz, int duration_ms);

/**
 * @brief Muestra estado de OK (enciende LED verde).
 */
void led_show_ok(void);

/**
 * @brief Muestra estado de error (enciende LED rojo).
 */
void led_show_error(void);

/**
 * @brief Parpadea LED naranja a 2 Hz por 3 segundos para indicar captura.
 */
void led_blink_capture(void);

#endif
