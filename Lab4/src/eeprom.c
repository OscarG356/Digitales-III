#include "eeprom.h"
#include "hardware/i2c.h"
#include <string.h>
#include <stdio.h>

#define EEPROM_I2C i2c0
#define SDA_PIN 4
#define SCL_PIN 5

#define BLOQUE_VAR1 0x51
#define BLOQUE_VAR2 0x52
#define BLOQUE_VAR3 0x53
#define OFFSET_DATOS 4
#define MAX_CAPTURAS 63  // índice 0 reservado

void eeprom_init() {
    i2c_init(EEPROM_I2C, 100 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
}

void eeprom_write_byte(uint8_t dev_addr, uint8_t mem_addr, uint8_t data) {
    uint8_t buf[2] = {mem_addr, data};
    i2c_write_blocking(EEPROM_I2C, dev_addr, buf, 2, false);
    sleep_ms(5);
}

uint8_t eeprom_read_byte(uint8_t dev_addr, uint8_t mem_addr) {
    i2c_write_blocking(EEPROM_I2C, dev_addr, &mem_addr, 1, true);
    uint8_t data;
    i2c_read_blocking(EEPROM_I2C, dev_addr, &data, 1, false);
    return data;
}

void eeprom_write_float(uint8_t block_addr, uint8_t index, float value) {
    uint8_t addr = OFFSET_DATOS + index * 4;
    uint8_t *data = (uint8_t*)&value;
    for (int i = 0; i < 4; i++) {
        eeprom_write_byte(block_addr, addr + i, data[i]);
    }
}

float eeprom_read_float(uint8_t block_addr, uint8_t index) {
    uint8_t addr = OFFSET_DATOS + index * 4;
    float value;
    uint8_t *data = (uint8_t*)&value;
    for (int i = 0; i < 4; i++) {
        data[i] = eeprom_read_byte(block_addr, addr + i);
    }
    return value;
}

uint8_t eeprom_get_index() {
    return eeprom_read_byte(BLOQUE_VAR1, 0);
}

void eeprom_set_index(uint8_t index) {
    eeprom_write_byte(BLOQUE_VAR1, 0, index);
    eeprom_write_byte(BLOQUE_VAR2, 0, index);
    eeprom_write_byte(BLOQUE_VAR3, 0, index);
}

void eeprom_guardar_captura(float v1, float v2, float v3) {
    uint8_t idx = eeprom_get_index();
    if (idx == 0xFF || idx > MAX_CAPTURAS) idx = 1;

    eeprom_write_float(BLOQUE_VAR1, idx - 1, v1);
    eeprom_write_float(BLOQUE_VAR2, idx - 1, v2);
    eeprom_write_float(BLOQUE_VAR3, idx - 1, v3);

    eeprom_set_index(idx + 1);
}

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

void eeprom_flush() {
    for (uint8_t i = 0; i < MAX_CAPTURAS; i++) {
        eeprom_write_float(BLOQUE_VAR1, i, 0.0f);
        eeprom_write_float(BLOQUE_VAR2, i, 0.0f);
        eeprom_write_float(BLOQUE_VAR3, i, 0.0f);
    }

    eeprom_set_index(1); // Reiniciar el índice a 1
    printf("Memoria EEPROM limpiada correctamente.\n");
}