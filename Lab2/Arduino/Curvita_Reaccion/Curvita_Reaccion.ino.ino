// Pines del L298
const int in1Pin = 2;
const int in2Pin = 3;
const int enAPin = 4;

// Encoder
const int encoderPin = 5;
const int pulsesPerRevolution = 20;

// Estructura para almacenar muestras
struct Sample {
  uint32_t delta;  // Tiempo desde inicio (en ms)
  uint8_t pwm;     // PWM aplicado
  uint16_t rpm;    // Velocidad medida
};

const int maxSamples = 7000;
Sample buffer[maxSamples];
int bufferIndex = 0;

volatile unsigned int pulseCount = 0;

// Temporizadores
unsigned long lastSampleTime = 0;
unsigned long lastStepTime = 0;
unsigned long startTime;

const unsigned long sampleInterval = 4;       // Cada 4 ms
const unsigned long stepInterval = 2000;      // Cada 2 s cambia PWM

// PWM
int pwmSteps[] = {0, 51, 102, 153, 204, 255, 204, 153, 102, 51, 0};
int stepIndex = 0;
bool finished = false;

void countPulse() {
  pulseCount++;
}

void setup() {
  Serial.begin(115200);

  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(enAPin, OUTPUT);
  digitalWrite(in1Pin, HIGH); // Dirección fija
  digitalWrite(in2Pin, LOW);

  analogWriteFreq(1000); // PWM de 1kHz
  analogWrite(enAPin, pwmSteps[stepIndex]); // PWM inicial

  pinMode(encoderPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderPin), countPulse, RISING);

  startTime = millis();
  lastSampleTime = startTime;
  lastStepTime = startTime;
}

void loop() {
  unsigned long currentTime = millis();

  if (!finished) {
    // --- Toma de muestras cada 4 ms ---
    if (currentTime - lastSampleTime >= sampleInterval) {
      noInterrupts();
      unsigned int count = pulseCount;
      pulseCount = 0;
      interrupts();

      float rpm = (count * 60.0) / pulsesPerRevolution;

      if (bufferIndex < maxSamples) {
        Sample s;
        s.delta = currentTime - startTime;
        s.pwm = pwmSteps[stepIndex];
        s.rpm = rpm;
        buffer[bufferIndex++] = s;
      }

      lastSampleTime = currentTime;
    }

    // --- Cambiar PWM cada 2 segundos ---
    if (currentTime - lastStepTime >= stepInterval) {
      stepIndex++;
      if (stepIndex >= sizeof(pwmSteps)/sizeof(pwmSteps[0])) {
        analogWrite(enAPin, 0); // Apaga el motor
        finished = true;
      } else {
        analogWrite(enAPin, pwmSteps[stepIndex]);
        lastStepTime = currentTime;
      }
    }
  } else {
    // --- Enviar CSV ---
    Serial.println("delta;pwm;rpm");
    for (int i = 0; i < bufferIndex; i++) {
      Serial.print(buffer[i].delta);
      Serial.print(";");
      Serial.print(buffer[i].pwm);
      Serial.print(";");
      Serial.println(buffer[i].rpm);
    }
    Serial.println("✅ CSV enviado.");
    while (true); // Detener el programa
  }
}
