from machine import Timer
import time
import random

# Estructura de datos
class Sample:
    def __init__(self, delta, pwm, rpm):
        self.delta = delta
        self.pwm = pwm
        self.rpm = rpm

# Configuración
MAX_SAMPLES = 1000
buffer = []
start_time = time.ticks_ms()
csv_file = "/datos.csv"  # Archivo a guardar

def adquirir_dato(timer):
    global buffer

    if len(buffer) < MAX_SAMPLES:
        delta = time.ticks_diff(time.ticks_ms(), start_time)
        pwm = random.randint(0, 255)
        rpm = random.randint(1000, 4000)
        buffer.append(Sample(delta, pwm, rpm))

    elif len(buffer) == MAX_SAMPLES:
        guardar_csv()
        buffer.append(None)  # Marcar para no volver a guardar

def guardar_csv():
    try:
        with open(csv_file, "w") as f:
            f.write("delta;pwm;rpm\n")
            for s in buffer:
                f.write(f"{s.delta};{s.pwm};{s.rpm}\n")
        print("✅ CSV guardado en la flash como 'datos.csv'")
    except Exception as e:
        print("❌ Error guardando CSV:", e)

# Timer para tomar datos cada 5 ms (200 Hz)
tim = Timer()
tim.init(freq=200, mode=Timer.PERIODIC, callback=adquirir_dato)
