// Pines del L298
const int in1Pin = 2;
const int in2Pin = 3;
const int enAPin = 4;

// Encoder
const int encoderPin = 5;
const int pulsesPerRevolution = 20;

// Estructura para almacenar muestras
struct Sample {
  uint32_t delta;  // Tiempo transcurrido desde el inicio (en ms)
  uint8_t pwm;     // Valor PWM (0-255)
  uint16_t rpm;    // RPM calculada
};

const int maxSamples = 9;  // Tamaño del buffer
Sample buffer[maxSamples];
int bufferIndex = 0;

volatile unsigned int pulseCount = 0;
float rpm = 0;

// Temporizadores
unsigned long lastRPMTime = 0;
unsigned long lastPWMTime = 0;

// Intervalos
const unsigned long rpmInterval = 1000;   // Calcular RPM cada 1s
const unsigned long pwmInterval = 1000;   // Cambiar PWM cada 1s

int duty = 0;

unsigned long startTime;

void countPulse() {
  pulseCount++;
}

void setup() {
  Serial.begin(115200);
  startTime = millis();

  // Motor
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(enAPin, OUTPUT);

  digitalWrite(in1Pin, HIGH);  // Dirección fija
  digitalWrite(in2Pin, LOW);

  analogWriteFreq(1000);  // Frecuencia PWM 1kHz

  // Encoder
  pinMode(encoderPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderPin), countPulse, RISING);

  lastRPMTime = millis();
  lastPWMTime = millis();
}

void loop() {
  unsigned long currentTime = millis();

  // --- Actualizar PWM cada pwmInterval ---
  if (currentTime - lastPWMTime >= pwmInterval) {
    analogWrite(enAPin, duty);
    duty += 25;
    if (duty > 250) duty = 0;
    lastPWMTime = currentTime;
  }

  // --- Calcular RPM cada rpmInterval ---
  if (currentTime - lastRPMTime >= rpmInterval) {
    noInterrupts();
    unsigned int count = pulseCount;
    pulseCount = 0;
    interrupts();

    rpm = (count * 60.0) / pulsesPerRevolution;
    lastRPMTime = currentTime;

    // Guardamos los datos en el buffer
    if (bufferIndex < maxSamples) {
      Sample s;
      s.delta = millis() - startTime;  // Tiempo transcurrido desde el inicio
      s.pwm = duty;                    // Valor de PWM
      s.rpm = rpm;                     // RPM calculada

      buffer[bufferIndex++] = s;  // Almacenamos la muestra en el buffer
    }

    // --- Cuando el buffer esté lleno, mandar los datos en formato CSV ---
    if (bufferIndex >= maxSamples) {
      Serial.println("delta;pwm;rpm");  // Encabezado CSV

      // Enviar todos los datos en formato CSV
      for (int i = 0; i < maxSamples; i++) {
        Serial.print(buffer[i].delta);
        Serial.print(";");
        Serial.print(buffer[i].pwm);
        Serial.print(";");
        Serial.println(buffer[i].rpm);
      }

      Serial.println("✅ CSV enviado.");

      // Resetear el buffer para empezar una nueva iteración
      bufferIndex = 0;  // Reiniciar el índice del buffer
      startTime = millis();  // Reiniciar el tiempo de inicio
    }
  }
}
