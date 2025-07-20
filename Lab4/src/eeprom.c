/**
 * @file eeprom.c
 * @brief Implementación de funciones para el manejo de una EEPROM externa mediante I2C.
 *
 * Este archivo contiene la lógica para inicializar la EEPROM, leer y escribir datos
 * en ella (específicamente valores flotantes), y mantener un índice de capturas.
 * También permite visualizar y borrar los datos guardados.
 */

#include "eeprom.h"
#include "hardware/i2c.h"
#include <string.h>
#include <stdio.h>

/// Definición de la instancia de I2C utilizada
#define EEPROM_I2C i2c0

/// Pin de datos (SDA)
#define SDA_PIN 4

/// Pin de reloj (SCL)
#define SCL_PIN 5

/// Dirección I2C de los bloques utilizados para guardar variables
#define BLOQUE_VAR1 0x51
#define BLOQUE_VAR2 0x52
#define BLOQUE_VAR3 0x53

/// Offset en la EEPROM donde inician los datos (los primeros bytes se reservan para el índice)
#define OFFSET_DATOS 4

/// Máximo número de capturas que se pueden guardar (índice 0 está reservado)
#define MAX_CAPTURAS 63


/**
 * @brief Inicializa la interfaz I2C para comunicación con la EEPROM.
 */
void eeprom_init() {
    i2c_init(EEPROM_I2C, 100 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
}

/**
 * @brief Escribe un byte en una dirección de memoria específica de la EEPROM.
 * 
 * @param dev_addr Dirección del dispositivo EEPROM.
 * @param mem_addr Dirección de memoria dentro del dispositivo.
 * @param data Byte a escribir.
 */
void eeprom_write_byte(uint8_t dev_addr, uint8_t mem_addr, uint8_t data) {
    uint8_t buf[2] = {mem_addr, data};
    i2c_write_blocking(EEPROM_I2C, dev_addr, buf, 2, false);
    sleep_ms(5);
}

/**
 * @brief Lee un byte desde una dirección de memoria específica de la EEPROM.
 * 
 * @param dev_addr Dirección del dispositivo EEPROM.
 * @param mem_addr Dirección de memoria desde la que se desea leer.
 * @return Byte leído.
 */
uint8_t eeprom_read_byte(uint8_t dev_addr, uint8_t mem_addr) {
    i2c_write_blocking(EEPROM_I2C, dev_addr, &mem_addr, 1, true);
    uint8_t data;
    i2c_read_blocking(EEPROM_I2C, dev_addr, &data, 1, false);
    return data;
}

/**
 * @brief Escribe un valor flotante (4 bytes) en una posición específica de la EEPROM.
 *
 * @param block_addr Dirección del bloque donde escribir.
 * @param index Índice de captura dentro del bloque.
 * @param value Valor flotante a guardar.
 */
void eeprom_write_float(uint8_t block_addr, uint8_t index, float value) {
    uint8_t addr = OFFSET_DATOS + index * 4;
    uint8_t *data = (uint8_t*)&value;
    for (int i = 0; i < 4; i++) {
        eeprom_write_byte(block_addr, addr + i, data[i]);
    }
}

/**
 * @brief Lee un valor flotante desde una posición específica de la EEPROM.
 *
 * @param block_addr Dirección del bloque donde leer.
 * @param index Índice de captura dentro del bloque.
 * @return Valor flotante leído.
 */
float eeprom_read_float(uint8_t block_addr, uint8_t index) {
    uint8_t addr = OFFSET_DATOS + index * 4;
    float value;
    uint8_t *data = (uint8_t*)&value;
    for (int i = 0; i < 4; i++) {
        data[i] = eeprom_read_byte(block_addr, addr + i);
    }
    return value;
}

/**
 * @brief Obtiene el índice actual de captura guardado en la EEPROM.
 *
 * @return Índice actual (1-based). Si es 0xFF o inválido, debe inicializarse.
 */
uint8_t eeprom_get_index() {
    return eeprom_read_byte(BLOQUE_VAR1, 0);
}

/**
 * @brief Establece el índice de captura en las tres memorias EEPROM.
 *
 * @param index Nuevo índice de captura.
 */
void eeprom_set_index(uint8_t index) {
    eeprom_write_byte(BLOQUE_VAR1, 0, index);
    eeprom_write_byte(BLOQUE_VAR2, 0, index);
    eeprom_write_byte(BLOQUE_VAR3, 0, index);
}

/**
 * @brief Guarda una captura compuesta por tres valores flotantes (por ejemplo: dBFS, latitud y longitud).
 *
 * @param v1 Primer valor a guardar.
 * @param v2 Segundo valor a guardar.
 * @param v3 Tercer valor a guardar.
 */
void eeprom_guardar_captura(float v1, float v2, float v3) {
    uint8_t idx = eeprom_get_index();
    if (idx == 0xFF || idx > MAX_CAPTURAS) idx = 1;

    eeprom_write_float(BLOQUE_VAR1, idx - 1, v1);
    eeprom_write_float(BLOQUE_VAR2, idx - 1, v2);
    eeprom_write_float(BLOQUE_VAR3, idx - 1, v3);

    eeprom_set_index(idx + 1);
}

/**
 * @brief Muestra por consola todas las capturas almacenadas.
 */
void eeprom_ver_datos() {
    uint8_t idx = eeprom_get_index();
    if (idx == 0xFF || idx == 0) {
        printf("No hay datos guardados.\n");
        return;
    }

    for (uint8_t i = 0; i < idx - 1; i++) {
        float v1 = eeprom_read_float(BLOQUE_VAR1, i);
        float v2 = eeprom_read_float(BLOQUE_VAR2, i);
        float v3 = eeprom_read_float(BLOQUE_VAR3, i);
        printf("Captura %d: %.6f\t%.6f\t%.6f\n", i + 1, v1, v2, v3);
    }
}

/**
 * @brief Borra todos los datos de la EEPROM y reinicia el índice de captura.
 */
void eeprom_flush() {
    for (uint8_t i = 0; i < MAX_CAPTURAS; i++) {
        eeprom_write_float(BLOQUE_VAR1, i, 0.0f);
        eeprom_write_float(BLOQUE_VAR2, i, 0.0f);
        eeprom_write_float(BLOQUE_VAR3, i, 0.0f);
    }

    eeprom_set_index(1); // Reiniciar el índice a 1
    printf("Memoria EEPROM limpiada correctamente.\n");
}
