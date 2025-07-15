#ifndef ADC_AUDIO_H
#define ADC_AUDIO_H

#include <stdint.h>

#define NUM_SAMPLES 20000
#define ADC_PIN 26

// Inicializar el ADC
void init_adc();

// Leer muestra ADC y convertir a voltios
float read_adc_voltage();

// Calcular RMS de las muestras
float calculate_rms(const float *samples, uint32_t num_samples);

// Calcular nivel de audio en dBFS
float calculate_dbfs(float rms);

#endif // ADC_AUDIO_H
