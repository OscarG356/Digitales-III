/**
 * @file adc_audio.c
 * @brief Implementación de funciones para captura y análisis de señal de audio usando ADC.
 * 
 * Este archivo contiene funciones para inicializar el ADC de la Raspberry Pi Pico,
 * capturar muestras de voltaje desde un pin analógico, calcular el valor RMS de la señal
 * y estimar el nivel de señal en dBFS (decibelios relativos al valor máximo posible del sistema).
 */

#include "include/adc_audio.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include <math.h>
#include <stdio.h>

/**
 * @brief Inicializa el ADC y configura el pin de entrada.
 * 
 * Se selecciona el canal ADC0 correspondiente al pin GPIO26.
 */
void init_adc() {
    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(0); // ADC0
}

/**
 * @brief Lee una muestra del ADC y la convierte a voltaje en voltios.
 * 
 * @return Valor de voltaje entre 0.0 y 3.3 V.
 */
float read_adc_voltage() {
    uint16_t raw = adc_read();
    return (raw / 4095.0f) * 3.3f; // 12 bits, 3.3V ref
}

/**
 * @brief Calcula el valor RMS (raíz cuadrática media) de un arreglo de muestras.
 * 
 * Primero remueve el valor DC (offset promedio), y luego calcula el RMS sobre la parte AC.
 * 
 * @param samples Arreglo de muestras en voltios.
 * @param num_samples Número de muestras en el arreglo.
 * @return Valor RMS de la señal.
 */
float calculate_rms(const float *samples, uint32_t num_samples) {
    float sum = 0;
    for (uint16_t i = 0; i < num_samples; i++) {
        sum += samples[i];
    }
    float offset = sum / num_samples;

    float sq_sum = 0;
    for (uint16_t i = 0; i < num_samples; i++) {
        float ac = samples[i] - offset;
        sq_sum += ac * ac;
    }
    return sqrtf(sq_sum / num_samples);
}

/**
 * @brief Calcula el nivel de señal en dBFS (decibelios en escala Full Scale).
 * 
 * Usa el valor RMS y lo compara con el máximo RMS posible para una señal senoidal de 3.3Vpp.
 * 
 * @param rms Valor RMS de la señal.
 * @return Nivel de señal en dBFS. El valor máximo es 0 dBFS.
 */
float calculate_dbfs(float rms) {
    const float max_rms = (3.3f / 2.0f) / 1.4142f; // ≈ 1.165V
    if (rms < 0.001f) return -100; // Evitar -inf
    return 20 * log10f(rms / max_rms);
}
