#ifndef ADC_AUDIO_H
#define ADC_AUDIO_H

#include <stdint.h>

/// Número total de muestras a capturar del ADC.
#define NUM_SAMPLES 20000

/// Pin GPIO utilizado para la entrada del ADC.
#define ADC_PIN 26

/**
 * @brief Inicializa el módulo ADC de la Raspberry Pi Pico.
 *
 * Configura el ADC para que lea desde el pin definido por `ADC_PIN`.
 */
void init_adc();

/**
 * @brief Lee una muestra del ADC y la convierte a voltios.
 *
 * @return Valor de voltaje correspondiente a la lectura ADC (0.0V a 3.3V).
 */
float read_adc_voltage();

/**
 * @brief Calcula el valor RMS de un conjunto de muestras.
 *
 * @param samples Arreglo de muestras en voltios.
 * @param num_samples Número de muestras en el arreglo.
 * @return Valor RMS de las muestras.
 */
float calculate_rms(const float *samples, uint32_t num_samples);

/**
 * @brief Convierte un valor RMS en voltios al nivel de audio en dBFS.
 *
 * @param rms Valor RMS de la señal.
 * @return Nivel de audio en decibelios Full Scale (dBFS).
 */
float calculate_dbfs(float rms);

#endif // ADC_AUDIO_H
