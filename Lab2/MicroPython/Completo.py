"""
@file Completo.py
@brief Sistema de caracterización de motores DC con interfaz serial
@author Imar Jimenez y Oscar Gutierrez
@date 5/05/2025

Este programa permite caracterizar motores DC mediante:
1. Control manual de PWM con lectura de RPM
2. Captura automática de curvas de respuesta
Los datos se guardan en archivo CSV para posterior análisis.
"""

from machine import Pin, PWM 
from time import ticks_ms, ticks_diff
import sys
import select

# ------------------ CONFIGURACIÓN DE PINES ------------------
## @var in1
# @brief Pin de dirección 1 del puente H
in1 = Pin(2, Pin.OUT)

## @var in2
# @brief Pin de dirección 2 del puente H
in2 = Pin(3, Pin.OUT)

## @var enA
# @brief Pin PWM para control de velocidad
enA = PWM(Pin(4))

## @var encoder_pin
# @brief Pin de entrada del encoder óptico
encoder_pin = Pin(5, Pin.IN, Pin.PULL_UP)

# ------------------ PARÁMETROS DEL SISTEMA ------------------
## @var pulses_per_revolution
# @brief Pulsos por revolución del encoder (ajustar según especificaciones)
pulses_per_revolution = 20

## @var sample_interval
# @brief Intervalo de muestreo en ms (250Hz @ 4ms)
sample_interval = 4

## @var step_interval
# @brief Intervalo entre cambios de PWM durante captura (ms)
step_interval = 2000

## @var print_interval
# @brief Intervalo de reporte en modo manual (ms)
print_interval = 500

# ------------------ VARIABLES GLOBALES ------------------
## @var pulse_count
# @brief Contador de pulsos del encoder (actualizado por interrupción)
pulse_count = 0

## @var current_state
# @brief Estado actual del sistema (IDLE, MANUAL_PWM, CAPTURING)
current_state = 0  # Inicia en IDLE

## @var current_pwm
# @brief Valor actual de PWM (0-100%)
current_pwm = 0

## @var csv_file
# @brief Objeto archivo para guardar datos
csv_file = None

# ------------------ CONSTANTES ------------------
IDLE = 0        # @brief Estado inactivo
MANUAL_PWM = 1  # @brief Modo control manual
CAPTURING = 2   # @brief Modo captura de datos

def count_pulse(pin):
    """@brief Interrupción para conteo de pulsos del encoder
    @param pin Pin que generó la interrupción
    """
    global pulse_count
    pulse_count += 1

def process_command(cmd):
    """@brief Procesa comandos recibidos por serial
    @param cmd Cadena con el comando recibido
    
    Comandos disponibles:
    - START <paso>: Inicia captura con incrementos especificados
    - PWM <valor>: Establece valor PWM manualmente
    """
    global pwm_step, current_pwm, descending, current_state
    global start_time, last_step_time, csv_file

    cmd = cmd.strip().upper()

    if cmd.startswith("START"):
        try:
            pwm_step = int(cmd.split()[1])
        except:
            pwm_step = 20

        pwm_step = min(max(pwm_step, 1), 100)
        current_pwm = 0
        descending = False
        enA.duty_u16(0)
        start_time = ticks_ms()
        last_step_time = ticks_ms()
        try:
            csv_file = open("curvita.csv", "w")
            csv_file.write("delta;pwm;rpm\n")
        except Exception as e:
            print(f"Error al abrir archivo: {e}")
            return
        current_state = CAPTURING
        print("Captura iniciada...")

    elif cmd.startswith("PWM"):
        try:
            pwm_val = int(cmd.split()[1])
        except:
            pwm_val = 0

        pwm_val = min(max(pwm_val, 0), 100)
        current_pwm = pwm_val
        enA.duty_u16(int(current_pwm * 65535 / 100))
        current_state = MANUAL_PWM
        print("Modo manual activado")

# ------------------ CONFIGURACIÓN INICIAL ------------------
# Configurar interrupción del encoder
encoder_pin.irq(trigger=Pin.IRQ_RISING, handler=count_pulse)

# Establecer dirección del motor
in1.value(1)
in2.value(0)
enA.duty_u16(0)
enA.freq(1000)  # Frecuencia PWM de 1kHz

# ------------------ BUCLE PRINCIPAL ------------------
input_line = ""
descending = False
just_changed_pwm = False
pwm_change_time = 0
last_sample_time = last_step_time = last_print_time = 0

while True:
    # Lectura no bloqueante de comandos
    while sys.stdin in select.select([sys.stdin], [], [], 0)[0]:
        c = sys.stdin.read(1)
        if c == '\n':
            process_command(input_line)
            input_line = ""
        else:
            input_line += c

    current_time = ticks_ms()

    # Muestreo periódico de RPM
    if ticks_diff(current_time, last_sample_time) >= sample_interval:
        last_sample_time = current_time
        count = pulse_count
        pulse_count = 0
        rpm = (count * 60000) / (pulses_per_revolution * sample_interval)

        if current_state == CAPTURING:
            delta = ticks_diff(current_time, start_time)

            if just_changed_pwm and ticks_diff(current_time, pwm_change_time) < 100:
                continue
            just_changed_pwm = False

            try:
                csv_file.write(f"{delta};{current_pwm};{int(rpm)}\n")
            except:
                print("Error al escribir en archivo")
                current_state = IDLE
                if csv_file:
                    csv_file.close()

        elif current_state == MANUAL_PWM and ticks_diff(current_time, last_print_time) >= print_interval:
            last_print_time = current_time
            print(f"PWM: {current_pwm}% | RPM: {int(rpm)}")

    # Secuencia de pasos durante captura
    if current_state == CAPTURING and ticks_diff(current_time, last_step_time) >= step_interval:
        last_step_time = current_time

        if not descending:
            current_pwm = min(current_pwm + pwm_step, 100)
            if current_pwm >= 100:
                descending = True
        else:
            current_pwm = max(current_pwm - pwm_step, 0)
            if current_pwm <= 0:
                current_pwm = 0
                enA.duty_u16(0)
                current_state = IDLE
                if csv_file:
                    csv_file.close()
                print("Captura finalizada")
                continue

        enA.duty_u16(int(current_pwm * 65535 / 100))
        just_changed_pwm = True
        pwm_change_time = current_time