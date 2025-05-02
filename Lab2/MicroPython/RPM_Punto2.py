from machine import Pin, PWM, Timer
import utime

# Configuración de pines
motor_pwm = PWM(Pin(15))
motor_pwm.freq(1000)

sensor_pin = Pin(14, Pin.IN)
led = Pin(25, Pin.OUT)

# Variables globales
current_pwm = 0
rpm = 0
last_capture_time = utime.ticks_ms()
pulse_count = 0
state = 'IDLE'
capture_data = []
step_size = 20
direction = 1  # 1: subida, -1: bajada
last_monitor_time = 0

# Configurar interrupción para sensor
def on_pulse(pin):
    global pulse_count
    pulse_count += 1

sensor_pin.irq(trigger=Pin.IRQ_RISING, handler=on_pulse)

# Timer para calcular RPM
def calculate_rpm(timer):
    global rpm, pulse_count
    count = pulse_count
    pulse_count = 0
    # Suponiendo 1 pulso por vuelta y medición cada 250 ms
    rpm = (count * 60 * 4)  # 4 veces por segundo

rpm_timer = Timer()
rpm_timer.init(freq=4, mode=Timer.PERIODIC, callback=calculate_rpm)

# Ajustar PWM del motor
def set_pwm(duty):
    global current_pwm
    if duty < 0:
        duty = 0
    elif duty > 100:
        duty = 100
    current_pwm = duty
    motor_pwm.duty_u16(int(duty * 65535 / 100))

# Comando START
def start_capture(step):
    global state, capture_data, direction
    capture_data = []
    direction = 1
    state = 'CAPTURE'
    set_pwm(0)

# Comando PWM
def manual_pwm(value):
    global state
    set_pwm(value)
    state = 'MANUAL'

# Captura periódica cada 4 ms
def capture_loop(timer):
    global state, current_pwm, rpm, capture_data, step_size, direction
    if state == 'CAPTURE':
        timestamp = utime.ticks_ms()
        capture_data.append((timestamp, current_pwm, rpm))

capture_timer = Timer()
capture_timer.init(freq=250, mode=Timer.PERIODIC, callback=capture_loop)

# Proceso principal
def monitor_loop():
    global state, current_pwm, last_monitor_time, direction
    next_step_time = utime.ticks_ms() + 2000

    while True:
        # Modo MANUAL: imprimir valores cada 500 ms
        if state == 'MANUAL':
            now = utime.ticks_ms()
            if utime.ticks_diff(now, last_monitor_time) > 500:
                print(f"{now}, {current_pwm}, {rpm}")
                last_monitor_time = now

        # Modo CAPTURE: manejar escalones de PWM
        elif state == 'CAPTURE':
            now = utime.ticks_ms()
            if utime.ticks_diff(now, next_step_time) >= 0:
                if direction == 1:
                    if current_pwm + step_size < 100:
                        set_pwm(current_pwm + step_size)
                        next_step_time = now + 2000
                    else:
                        direction = -1
                        next_step_time = now + 2000
                elif direction == -1:
                    if current_pwm - step_size >= 0:
                        set_pwm(current_pwm - step_size)
                        next_step_time = now + 2000
                    else:
                        # Fin de la captura
                        set_pwm(0)
                        state = 'IDLE'
                        print("timestamp,PWM,RPM")
                        for entry in capture_data:
                            print(f"{entry[0]},{entry[1]},{entry[2]}")
                        print("END")

        # Leer comandos desde consola
        if uart.any():
            cmd = uart.readline().decode().strip()
            if cmd.startswith("START"):
                try:
                    step_size = int(cmd.split()[1])
                    if step_size <= 0 or step_size >= 100:
                        print("ERROR: Paso inválido")
                    else:
                        start_capture(step_size)
                except:
                    print("ERROR: Comando mal formado")

            elif cmd.startswith("PWM"):
                try:
                    value = int(cmd.split()[1])
                    if value < 0 or value > 100:
                        print("ERROR: PWM fuera de rango")
                    else:
                        manual_pwm(value)
                except:
                    print("ERROR: Comando mal formado")

# UART por USB
import sys
uart = sys.stdin

# Iniciar
print("Sistema listo. Usa 'START <valor>' o 'PWM <valor>'")
monitor_loop()
