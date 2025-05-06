from machine import Pin, PWM
import utime

# Configuración de pines
in1 = Pin(2, Pin.OUT)
in2 = Pin(3, Pin.OUT)
en = PWM(Pin(4))
sensor = Pin(5, Pin.IN, Pin.PULL_DOWN)

# Parámetros ajustados
en.freq(1000)
in1.high()
in2.low()

step_size = 20
step_time = 700  # ms
sample_interval = 4  # 4ms como requisito
pulses_per_rev = 20  # 20 tics por vuelta (encoder óptico)

# Variables
pwm_sequence = list(range(0, 101, step_size)) + list(range(100 - step_size, -1, -step_size))
rpm_data = []
pulse_count = 0

def count_pulse(pin):
    global pulse_count
    pulse_count += 1

sensor.irq(trigger=Pin.IRQ_RISING, handler=count_pulse)

def set_pwm(percent):
    en.duty_u16(int(percent * 65535 / 100))

# Bucle principal optimizado
current_step = 0
last_step_time = utime.ticks_ms()
last_sample_time = utime.ticks_ms()
start_time = utime.ticks_ms()

# Espera inicial para estabilización
utime.sleep_ms(500)

while current_step < len(pwm_sequence):
    now = utime.ticks_ms()
    
    # Muestreo cada 4ms exactos
    if utime.ticks_diff(now, last_sample_time) >= sample_interval:
        dt = utime.ticks_diff(now, last_sample_time) / 1000  # segundos
        
        # Cálculo preciso de RPM con 20 tics/vuelta
        rpm = (pulse_count / pulses_per_rev) / dt * 60 if dt > 0 else 0
        
        # Guardar datos crudos
        rpm_data.append((
            utime.ticks_diff(now, start_time),  # delta
            pwm_sequence[current_step],        # pwm
            int(rpm)                          # rpm cruda
        ))
        
        pulse_count = 0
        last_sample_time = now
    
    # Cambio de PWM
    if utime.ticks_diff(now, last_step_time) >= step_time:
        set_pwm(pwm_sequence[current_step])
        current_step += 1
        last_step_time = now

# Guardar en archivo CSV
with open("curvita.csv", "w") as f:
    f.write("delta;pwm;rpm\n")
    for row in rpm_data:
        f.write("{};{};{}\n".format(*row))

print("Datos guardados en curvita.csv")