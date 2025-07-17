#ifndef EEPROM_H
#define EEPROM_H

#include "pico/stdlib.h"

void eeprom_init();
void eeprom_write_float(uint8_t block_addr, uint8_t index, float value);
float eeprom_read_float(uint8_t block_addr, uint8_t index);
uint8_t eeprom_get_index();
void eeprom_set_index(uint8_t index);
void eeprom_guardar_captura(float v1, float v2, float v3);
void eeprom_ver_datos();
void eeprom_flush();


#endif
