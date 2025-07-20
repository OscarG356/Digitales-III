/**
 * @file eeprom.h
 * @brief Interfaz para manejo de almacenamiento EEPROM simulado en la Raspberry Pi Pico.
 *
 * Este módulo permite almacenar y recuperar valores de tipo float de una memoria EEPROM
 * simulada usando la flash de la Raspberry Pi Pico.
 */

#ifndef EEPROM_H
#define EEPROM_H

#include "pico/stdlib.h"

/**
 * @brief Inicializa la EEPROM simulada.
 *
 * Debe llamarse al inicio para configurar correctamente el acceso a memoria.
 */
void eeprom_init();

/**
 * @brief Escribe un valor flotante en una posición específica.
 *
 * @param block_addr Dirección base (bloque) donde se escribirá.
 * @param index Índice dentro del bloque.
 * @param value Valor flotante a almacenar.
 */
void eeprom_write_float(uint8_t block_addr, uint8_t index, float value);

/**
 * @brief Lee un valor flotante desde una posición específica.
 *
 * @param block_addr Dirección base (bloque) desde donde se leerá.
 * @param index Índice dentro del bloque.
 * @return Valor flotante almacenado.
 */
float eeprom_read_float(uint8_t block_addr, uint8_t index);

/**
 * @brief Obtiene el índice actual de escritura.
 *
 * @return Índice usado para la siguiente escritura.
 */
uint8_t eeprom_get_index();

/**
 * @brief Establece el índice de escritura.
 *
 * @param index Nuevo valor del índice.
 */
void eeprom_set_index(uint8_t index);

/**
 * @brief Guarda una captura de 3 valores flotantes consecutivos en la EEPROM.
 *
 * @param v1 Primer valor.
 * @param v2 Segundo valor.
 * @param v3 Tercer valor.
 */
void eeprom_guardar_captura(float v1, float v2, float v3);

/**
 * @brief Imprime por consola todos los datos almacenados.
 *
 * Muestra los valores guardados en la EEPROM en orden.
 */
void eeprom_ver_datos();

/**
 * @brief Fuerza la escritura de la EEPROM en flash.
 *
 * Asegura que todos los datos pendientes se almacenen de forma permanente.
 */
void eeprom_flush();

#endif
